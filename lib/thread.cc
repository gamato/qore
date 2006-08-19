/*
  thread.cc

  POSIX thread library for Qore

  Qore Programming Language

  Copyright (C) 2003, 2004, 2005, 2006 David Nichols

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <qore/config.h>
#include <qore/support.h>
#include <qore/common.h>
#include <qore/thread.h>
#include <qore/QoreNode.h>
#include <qore/QoreType.h>
#include <qore/List.h>
#include <qore/Object.h>
#include <qore/Operator.h>
#include <qore/QoreClass.h>
#include <qore/Variable.h>
#include <qore/QoreProgram.h>
#include <qore/Namespace.h>
#include <qore/LockedObject.h>

// to register object types
#include <qore/QC_Queue.h>
#include <qore/QC_Mutex.h>
#include <qore/QC_Condition.h>
#include <qore/QC_RWLock.h>
#include <qore/QC_Gate.h>
#include <qore/QC_Sequence.h>
#include <qore/QC_Counter.h>
#include <qore/QC_RMutex.h>

#include <pthread.h>
#include <sys/time.h>

// FIXME: move to config.h or something like that
// not more than this number of threads can be running at the same time
#define MAX_THREADS 0x1000

// new thread entry positions will be allocated in blocks of this size
#define THREAD_LIST_BLOCK 0x80

#ifdef DEBUG
bool threads_initialized = false;
#endif

class Operator *OP_BACKGROUND;

LockedObject lThreadList;

ThreadCleanupList tclist;
ThreadResourceList trlist;

// default thread creation attribute
static pthread_attr_t ta_default;

// recursive mutex attribute
pthread_mutexattr_t ma_recursive;

static int      max_thread_list = 0;
static int      current_tid = 0;
ThreadEntry    *thread_list = NULL;
pthread_key_t   thread_data_key;

#ifndef HAVE_GETHOSTBYNAME_R
class LockedObject lck_gethostbyname;
#endif

#ifndef HAVE_GETHOSTBYADDR_R
class LockedObject lck_gethostbyaddr;
#endif

// total number of threads running
int num_threads = 0;

tid_node *tid_head = NULL, *tid_tail = NULL;

class BGThreadParams {
   public:
      class Object *obj;
      class Object *callobj;
      class QoreNode *fc;
      class QoreProgram *pgm;
      int tid;
      int line;
      char *file;
      bool method_reference;

      inline BGThreadParams(class QoreNode *f, int t, class ExceptionSink *xsink)
      { 
	 tid = t;
	 fc = f;
	 pgm = getProgram();
	 line = get_pgm_counter();
	 file = get_pgm_file();

	 obj = NULL;
	 // get and reference the current stack object, if any, for the new call stack
	 callobj = getStackObject();

	 if (callobj && fc->type == NT_FUNCTION_CALL && fc->val.fcall->type == FC_SELF)
	 {
	    // we reference the object so it won't go out of scope while the thread is running
	    obj = callobj;
	    obj->ref();
	 }
	 else if (fc->type == NT_TREE && fc->val.tree.op == OP_OBJECT_FUNC_REF)
	 {
	    // evaluate object
	    class QoreNode *n = fc->val.tree.left->eval(xsink);
	    if (!n || xsink->isEvent())
	       return;
	    
	    fc->val.tree.left->deref(xsink);
	    fc->val.tree.left = n;
	    if (n->type == NT_OBJECT)
	    {
	       obj = n->val.object;
	       obj->ref();
	    }
	 }
 
	 if (callobj)
	    callobj->tRef();

	 // increment the program's thread counter
	 pgm->tcount.inc();
      }
      inline ~BGThreadParams()
      {
	 // decrement program's thread count
	 pgm->tcount.dec();
      }
      void cleanup(class ExceptionSink *xsink)
      {
	 if (fc) fc->deref(xsink);
	 derefObj(xsink);
	 derefCallObj();
      }
      inline void derefCallObj()
      {
	 // dereference call object if present
	 if (callobj)
	 {
	    callobj->tDeref();
	    callobj = NULL;
	 }
      }
      inline void derefObj(class ExceptionSink *xsink)
      {
	 if (obj)
	 {
	    obj->dereference(xsink);
	    obj = NULL;
	 }
      }
      inline class QoreNode *exec(class ExceptionSink *xsink)
      {
	 class QoreNode *rv = fc->eval(xsink);
	 fc->deref(xsink);
	 fc = NULL;
	 return rv;
      }
};

inline ThreadResourceList::~ThreadResourceList()
{
#ifdef DEBUG
   if (head)
      run_time_error("ThreadResourceList %08x not empty, head = %08x", this, head);
#endif
}

inline class ThreadResourceNode *ThreadResourceList::find(void *key)
{
   class ThreadResourceNode *w = head;
   while (w)
   {
      if (w->key == key)
	 return w;
      w = w->next;
   }
   return NULL;
}

void ThreadResourceList::setIntern(class ThreadResourceNode *n)
{
   //printd(5, "TRL::setIntern(key=%08x, func=%08x)\n", n->key, n->func);
   n->next = head;
   if (head)
      head->prev = n;
   head = n;
}

void ThreadResourceList::set(void *key, qtrdest_t func)
{
   class ThreadResourceNode *n = new ThreadResourceNode(key, func);
   lock();
   setIntern(n);
   unlock();
}

inline int ThreadResourceList::setOnce(void *key, qtrdest_t func)
{
   int rc = 0;
   lock();
   if (find(key))
      rc = -1;
   else
      setIntern(new ThreadResourceNode(key, func));
   unlock();
   return rc;
}

inline void ThreadResourceList::removeIntern(class ThreadResourceNode *w)
{
   //printd(5, "removeIntern(%08x) starting (head=%08x)\n", w, head);
   if (w->prev)
      w->prev->next = w->next;
   else
      head = w->next;
   if (w->next)
      w->next->prev = w->prev;
   //printd(5, "removeIntern(%08x) done (head=%08x)\n", w, head);
}

void ThreadResourceList::purgeTID(int tid, class ExceptionSink *xsink)
{
   // we put all the nodes in a temporary list and then run them from there
   class ThreadResourceList trl;

   lock();
   class ThreadResourceNode *w = head;
   while (w)
   {
      //printd(5, "TRL::purgeTID(%d) w->tid=%d, w->key=%08x, w->next=%08x\n", tid,w->tid, w->key, w->next);
      if (w->tid == tid)
      {
	 class ThreadResourceNode *n = w->next;
	 removeIntern(w);
	 w->prev = NULL;
	 trl.setIntern(w);
	 w = n;
      }
      else
	 w = w->next;
   }   
   unlock();

   if (trl.head)
   {
      class ThreadResourceNode *w = trl.head;
      while (w)
      {
	 w->call(xsink);
	 w = w->next;
      }
      // erase all nodes in temporary list
      w = trl.head;
      while (w)
      {
	 class ThreadResourceNode *n = w->next;
	 delete w;
	 w = n;
      }
#ifdef DEBUG
      trl.head = NULL;
#endif
   }

   //printd(5, "TRL::purgeTID() done\n");
}

void ThreadResourceList::remove(void *key)
{
   //printd(5, "TRL::remove(key=%08x)\n", key);
   lock();
   class ThreadResourceNode *w = find(key);
   if (w)
   {
      removeIntern(w);
      delete w;
   }
   unlock();
}

inline ThreadCleanupList::ThreadCleanupList()
{
   //printf("ThreadCleanupList::ThreadCleanupList() head=NULL\n");

   head = NULL;
}

inline ThreadCleanupList::~ThreadCleanupList()
{
   //printf("ThreadCleanupList::~ThreadCleanupList() head=%08x\n", head);

   while (head)
   {
      class ThreadCleanupNode *w = head->next;
      delete head;
      head = w;
   }
}

void ThreadCleanupList::push(qtdest_t func, void *arg)
{
   class ThreadCleanupNode *w = new ThreadCleanupNode;
   w->next = head;
   w->func = func;
   w->arg = arg;
   head = w;
   //printf("TCL::push() this=%08x, &head=%08x, head=%08x, head->next=%08x\n", this, &head, head, head->next);
}

void ThreadCleanupList::pop(int exec)
{
   //printf("TCL::pop() this=%08x, &head=%08x, head=%08x\n", this, &head, head);
   // NOTE: if exit() is called, then somehow head = NULL !!!
   // I can't explain it, but that's why the if statement is there... :-(
   if (head)
   {
      if (exec)
	 head->func(head->arg);
      class ThreadCleanupNode *w = head->next;
      delete head;
      head = w;
   }
}

static inline void grow_thread_list()
{
   int start = max_thread_list;
   max_thread_list += THREAD_LIST_BLOCK;
   if (max_thread_list > MAX_THREADS)
      max_thread_list = MAX_THREADS;
   thread_list = (ThreadEntry *)realloc(thread_list, sizeof(ThreadEntry) * max_thread_list);
   // zero out new entries
   for (int i = start; i < max_thread_list; i++)
      thread_list[i].ptid = 0L;
}

// returns tid allocated for thread
static int get_thread_entry()
{
   int tid;

   lThreadList.lock();
   if (current_tid == max_thread_list)
   {
      if (max_thread_list < MAX_THREADS)
      {
	 grow_thread_list();
	 tid = current_tid++;
      }
      else
      {
	 int i;
	 // scan thread_list for free entry
	 for (i = 0; i < max_thread_list; i++)
	 {
	    if (!thread_list[i].ptid)
	    {
	       tid = i;
	       break;
	    }
	 }
	 if (i == max_thread_list)
	    tid = -1;
      }
   }
   else
      tid = current_tid++;
   
   if (tid != -1)
   {
      thread_list[tid].ptid = (pthread_t)-1L;
      thread_list[tid].tidnode = new tid_node(tid);
      thread_list[tid].callStack = NULL;

      num_threads++;
   }
   lThreadList.unlock();
   return tid;
}

static void deregister_thread(int tid)
{
   // NOTE: cannot safely call printd here, because normally the thread_data has been deleted
   lThreadList.lock();

   thread_list[tid].cleanup();
   num_threads--;

   lThreadList.unlock();
}

static inline void delete_thread_data()
{
   delete (ThreadData *)pthread_getspecific(thread_data_key);
}

static void *op_background_thread(void *vtp)
{
   class BGThreadParams *btp = (BGThreadParams *)vtp;
    
   // register thread
   register_thread(btp->tid, pthread_self(), btp->pgm);
   printd(5, "op_background_thread() started");

   if (!thread_list[btp->tid].callStack)
      printf("TID %d: callstack = NULL\n", btp->tid);
   // create thread-local data for this thread in the program object
   btp->pgm->startThread();
   // set program counter for new thread
   update_pgm_counter_pgm_file(btp->line, btp->file);

   // push this call on the thread stack
   pushCall("background operator", CT_NEWTHREAD, btp->callobj);

   // dereference call object if present
   btp->derefCallObj();

   class ExceptionSink xsink;

   // run thread expression
   class QoreNode *rv = btp->exec(&xsink);

   // if there is an object, we dereference the extra reference here
   btp->derefObj(&xsink);

   // pop the call from the stack
   popCall(&xsink);

   // dereference any return value from the background expression
   if (rv)
      rv->deref(&xsink);

   // delete any thread data
   btp->pgm->endThread(&xsink);
   
   // cleanup thread resources
   trlist.purgeTID(btp->tid, &xsink);

   xsink.handleExceptions();

   printd(4, "thread terminating");

   // delete internal thread data structure
   delete_thread_data();

   // deregister_thread
   deregister_thread(btp->tid);

   // run any cleanup functions
   tclist.exec();

   delete btp;

   pthread_exit(NULL);
   return NULL;
}

static class QoreNode *op_background(class QoreNode *left, class QoreNode *right, ExceptionSink *xsink)
{
   if (!left)
      return NULL;

   //printd(2, "op_background() before crlr left = %08x\n", left);
   QoreNode *nl = copy_and_resolve_lvar_refs(left, xsink);
   //printd(2, "op_background() after crlr nl = %08x\n", nl);
   if (xsink->isEvent())
   {
      if (nl) nl->deref(xsink);
      return NULL;
   }
   if (!nl)
      return NULL;

   // now we are ready to create the new thread

   // get thread entry
   //printd(2, "calling get_thread_entry()\n");
   int tid = get_thread_entry();
   //printd(2, "got %d()\n", tid);

   // if can't start thread, then throw exception
   if (tid == -1)
   {
      if (nl) nl->deref(xsink);
      xsink->raiseException("THREAD-CREATION-FAILURE", "thread list is full with %d threads", max_thread_list);
      return NULL;
   }

   //printd(2, "creating BGThreadParams(%08x, %d)\n", nl, tid);
   BGThreadParams *tp = new BGThreadParams(nl, tid, xsink);
   if (xsink->isEvent())
   {
      if (nl) nl->deref(xsink);
      deregister_thread(tid);
      return NULL;
   }
   //printd(2, "tp = %08x\n", tp);
   // create thread
   pthread_t ptid;
   int rc;
   //printd(2, "calling pthread_create(%08x, %08x, %08x, %08x)\n", &ptid, &ta_default, op_background_thread, tp);
   if ((rc = pthread_create(&ptid, &ta_default, op_background_thread, tp)))
   {
      tp->cleanup(xsink);
      delete tp;

      deregister_thread(tid);
      xsink->raiseException("THREAD-CREATION-FAILURE", "could not create thread: %s", strerror(errno));
      return NULL;
   }
   printd(5, "pthread_create() new thread TID %d, pthread_create() returned %d\n", tid, rc);

   printd(5, "create_thread() created thread with TID %d\n", ptid);
   return new QoreNode((int64)tid);
}

void init_qore_threads()
{
   tracein("qore_init_threads()");

   // init thread list
   grow_thread_list();

   // init thread data key
   pthread_key_create(&thread_data_key, NULL); //thread_data_cleanup);

   // setup parent thread data
   register_thread(get_thread_entry(), pthread_self(), NULL);

   // register "background" Operator.handler
   OP_BACKGROUND = oplist.add(new Operator(1, "background", "run in background thread", 0, true));
   OP_BACKGROUND->addFunction(NT_ALL, NT_NONE, op_background);

   // initialize default thread creation attribute
   pthread_attr_init(&ta_default);
   pthread_attr_setdetachstate(&ta_default, PTHREAD_CREATE_DETACHED);

   // initialize recursive mutex attribute
   pthread_mutexattr_init(&ma_recursive);
   pthread_mutexattr_settype(&ma_recursive, PTHREAD_MUTEX_RECURSIVE);

#ifdef DEBUG
   // mark threading as active
   threads_initialized = true;
#endif

   traceout("qore_init_threads()");
}

class Namespace *get_thread_ns()
{
   // create Qore::Thread namespace
   class Namespace *Thread = new Namespace("Thread");

   Thread->addSystemClass(initQueueClass());
   Thread->addSystemClass(initMutexClass());
   Thread->addSystemClass(initRMutexClass());
   Thread->addSystemClass(initConditionClass());
   Thread->addSystemClass(initRWLockClass());
   Thread->addSystemClass(initGateClass());
   Thread->addSystemClass(initSequenceClass());
   Thread->addSystemClass(initCounterClass());

   return Thread;
}

void delete_qore_threads()
{
#ifdef DEBUG
   // mark threading as inactive
   threads_initialized = false;
#endif
   tracein("delete_qore_threads()");

   ExceptionSink xsink;
   trlist.purgeTID(0, &xsink);
   xsink.handleExceptions();

   pthread_mutexattr_destroy(&ma_recursive);

   //printd(2, "calling pthread_attr_destroy(%08x)\n", &ta_default);
   pthread_attr_destroy(&ta_default);
   //printd(2, "returned from pthread_attr_destroy(%08x)\n", &ta_default);

   delete_thread_data();

   thread_list[0].cleanup();

   // delete key
   pthread_key_delete(thread_data_key);

   // delete thread list
   free(thread_list);

   max_thread_list = 0;

   traceout("delete_qore_threads()");
}

List *get_thread_list()
{
   List *l = new List();
   lThreadList.lock();
   tid_node *w = tid_head;
   while (w)
   {
      l->push(new QoreNode(NT_INT, w->tid));
      w = w->next;
   }
   lThreadList.unlock();
   return l;
}

Hash *getAllCallStacks()
{
   Hash *h = new Hash();
   QoreString str;
   lThreadList.lock();
   tid_node *w = tid_head;
   while (w)
   {
      // get call stack
      if (thread_list[w->tid].callStack)
      {
	 List *l = thread_list[w->tid].callStack->getCallStack();
	 if (l->size())
	 {
	    // make hash entry
	    str.terminate(0);
	    str.sprintf("%d", w->tid);
	    h->setKeyValue(&str, new QoreNode(l), NULL);
	 }
	 else
	    delete l;
      }
      w = w->next;
   }   
   lThreadList.unlock();
   return h;
}
