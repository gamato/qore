/* 
   QC_Mutex.h

   Qore Programming Language

   Copyright (C) 2003, 2004, 2005, 2006, 2007 David Nichols

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

#ifndef _QORE_CLASS_MUTEX

#define _QORE_CLASS_MUTEX

#include <qore/Qore.h>
#include <qore/AbstractSmartLock.h>

DLLEXPORT extern int CID_MUTEX;

DLLLOCAL class QoreClass *initMutexClass();

class Mutex : public AbstractPrivateData, public AbstractSmartLock
{
private:
   int tid, waiting;
   
public:
   DLLLOCAL Mutex() : tid(-1), waiting(0) {}
   //DLLLOCAL virtual ~Mutex() {}
   DLLLOCAL int lock(class ExceptionSink *xsink)
   {
      int mtid = gettid();
      class VLock *nvl = getVLock();
      AutoLocker al(&asl_lock);
      if (tid != -1)
      {
	 if (tid == mtid)
	 {
	    xsink->raiseException("LOCK-ERROR", "TID %d called Mutex::lock() twice without a Mutex::unlock()", tid);
	    return -1;
	 }
	 while (tid != -1)
	 {
	    waiting++;
	    int rc =  nvl->waitOn((AbstractSmartLock *)this, vl, mtid, xsink);
	    waiting--;
	    if (rc)
	       return -1;
	 }
      }
      tid = mtid;
      vl = nvl;
      nvl->push(this);
      return 0;
   }
   DLLLOCAL int unlock(class ExceptionSink *xsink)
   {
      int mtid = gettid();
      AutoLocker al(&asl_lock);
      if (tid == -1)
      {
	 xsink->raiseException("LOCK-ERROR", "TID %d called Mutex::unlock() while the lock was already unlocked", mtid);
	 return -1;
      }
      if (tid != mtid)
      {
	 xsink->raiseException("LOCK-ERROR", "TID %d called Mutex::unlock() while the lock is held by tid %d", mtid, tid);
	 return -1;
      }
#ifdef DEBUG
      AbstractSmartLock *g = vl->pop();
      assert(g == this);
#else
      vl->pop();
#endif
      
      tid = -1;
      vl = NULL;
      if (waiting)
	 asl_cond.signal();

      return 0;
   }
   DLLLOCAL int trylock()
   {
      AutoLocker al(&asl_lock);
      if (tid != -1)
	 return -1;
      tid = gettid();
      vl = getVLock();
      return 0;
   }
   DLLLOCAL int release()
   {
      AutoLocker al(&asl_lock);
      if (tid == -1)
	 return -1;
      tid = -1;
      vl = NULL;
      if (waiting)
	 asl_cond.signal();
      return 0;
   }
   DLLLOCAL int verify_lock_tid(char *meth, class ExceptionSink *xsink)
   {
      AutoLocker al(&asl_lock);
      int mtid = gettid();
      if (tid == mtid)
	 return 0;
      if (tid == -1)
	 xsink->raiseException("LOCK-ERROR", "%s() with unlocked lock argument", meth);
      else
	 xsink->raiseException("LOCK-ERROR", "TID called %s with lock argument held by TID %d", mtid, tid);
      return -1;
   }  
};

#endif // _QORE_CLASS_MUTEX
