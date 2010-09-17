/*
 ManagedDatasource.cpp
 
 Qore Programming Language
 
 Copyright 2003 - 2010 Qore Technologies, sro
 
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

#include <qore/Qore.h>
#include <qore/intern/ManagedDatasource.h>

#include <stdlib.h>
#include <string.h>

ManagedDatasource::ManagedDatasource(DBIDriver *ndsl) : 
   Datasource(ndsl), counter(0), tid(-1), waiting(0), sql_waiting(0), 
   tl_timeout_ms(DEFAULT_TL_TIMEOUT) {
}

ManagedDatasource::~ManagedDatasource() {
}

void ManagedDatasource::cleanup(ExceptionSink *xsink) {
   AutoLocker al(&ds_lock);
   // wait for any in-progress action to complete
   wait_for_sql();
   if (isInTransaction()) {
      xsink->raiseException("DATASOURCE-TRANSACTION-EXCEPTION", "TID %d terminated while in a transaction; transaction will be automatically rolled back and the lock released", gettid());
      Datasource::rollback(xsink);
      setTransactionStatus(false);
      // force-exit the transaction lock
      forceReleaseLock();
   }
}

void ManagedDatasource::destructor(ExceptionSink *xsink) {
   AutoLocker al(&ds_lock);
   // closeUnlocked will throw an exception if a transaction is in progress (and release the lock)
   closeUnlocked(xsink);
   counter = -1;
}

void ManagedDatasource::deref(ExceptionSink *xsink) {
   if (ROdereference()) {
      close(xsink);
      delete this;
   }
}

// this function is only called by remove_thread_resource()
// during a call, meaning that the reference count cannot reach 0,
// meaning that the close method will never be run here
void ManagedDatasource::deref() {
#ifdef DEBUG
   assert(!ROdereference());
#else
   ROdereference();
#endif
}

int ManagedDatasource::grabLockIntern() {
   int ctid = gettid();
   
   if (tid == ctid)
      return 0;

   while (tid != -1) {
      ++waiting;
      if (tl_timeout_ms) {
	 int rc = cTransaction.wait(&ds_lock, tl_timeout_ms);
	 --waiting;
	 if (!rc)
	    continue;
	 printd(5, "ManagedDatasource %08p timed out after %dms waiting for tid %d to release lock\n", this, tl_timeout_ms, tid);
	 return -1;
      }
      else
	 cTransaction.wait(&ds_lock);
      --waiting;
   }
   
   tid = ctid;
   
   return 0;   
}

int ManagedDatasource::grabLock(ExceptionSink *xsink) {
   if (grabLockIntern() < 0) {
      endDBActionIntern();
      const char *un = getUsername();
      const char *db = getDBName();
      xsink->raiseException("TRANSACTION-LOCK-TIMEOUT", "timed out on datasource '%s@%s' after waiting %d millisecond%s on transaction lock held by TID %d", 
			    un ? un : "<n/a>", db ? db : "<n/a>", tl_timeout_ms, 
			    tl_timeout_ms == 1 ? "" : "s", tid); //tGate.getLockTID());
      return -1;
   }
   return 0;
}

void ManagedDatasource::releaseLockIntern() {
   assert(tid == gettid());
   tid = -1;
   if (waiting)
      cTransaction.signal();
}

void ManagedDatasource::releaseLock() {
   AutoLocker al(ds_lock);
   releaseLockIntern();
}

void ManagedDatasource::forceReleaseLock() {
   tid = -1;
   if (waiting)
      cTransaction.signal();
}

ManagedDatasource *ManagedDatasource::copy() {
   ManagedDatasource *nds = new ManagedDatasource(const_cast<DBIDriver *>(getDriver()));   
   nds->setPendingConnectionValues(static_cast<Datasource *>(this));
   return nds;
}

// must be holding ds_lock
int ManagedDatasource::wait_for_sql(ExceptionSink *xsink) {
   // object has been deleted in another thread
   if (counter < 0) {
      xsink->raiseException("DATASOURCE-ERROR", "The Datasource object has already been deleted");
      return -1;
   }
   wait_for_sql();
   return 0;
}

// must be holding ds_lock
void ManagedDatasource::wait_for_sql() {
   while (counter > 0) {
      ++sql_waiting;
      cSQL.wait(&ds_lock);
      --sql_waiting;
   }
   // in case there are other calls waiting
   cSQL.signal();
}

int ManagedDatasource::startDBAction(ExceptionSink *xsink, bool need_transaction_lock, bool *new_transaction) {
   AutoLocker al(&ds_lock);

   // save previous trans lock status
   bool had_lock = (tid == gettid());

   // first grab the transaction lock
   if (need_transaction_lock && grabLock(xsink))
      return -1;

#ifdef DEBUG
   //printd(0, "ManagedDatasource::startDBAction() ntl=%d tid=%d\n", need_transaction_lock, tid);

   if (need_transaction_lock)
      assert(tid == gettid());
#endif

   // ensure no SQL is in progress on the connection
   if (wait_for_sql(xsink)) {
      // release transaction lock if necessary
      if (need_transaction_lock && !had_lock)
	 releaseLockIntern();
      return -1;
   }

   // open the datasource if necessary
   if (!isOpen() && (Datasource::open(xsink) || *xsink)) {
      // release transaction lock if necessary
      if (need_transaction_lock && !had_lock)
	 releaseLockIntern();
      return -1;
   }

   // set new_transaction flag if grabbing the transaction lock for the first time
   if (need_transaction_lock && new_transaction && !had_lock)
      *new_transaction = true;

   // flag for SQL in progress
   counter = 1;
   return 0;
}

bool ManagedDatasource::endDBActionIntern(char cmd, bool new_transaction) {
   if (cmd == DAH_ACQUIRE) {
      // save thread resource if we just started a transaction
      if (new_transaction) {
	 // save thread resource if we just started a transaction
	 if (isInTransaction())
	    set_thread_resource(this);
	 else
	    releaseLockIntern();
      }
   }
   else if (cmd) {
      assert(cmd == DAH_RELEASE);
      
      // transaction is complete, remove the transaction thread resource
      if (!new_transaction)
	 remove_thread_resource(this);
      
      releaseLockIntern();
   }

   counter = 0;
   if (sql_waiting)
      cSQL.signal();

   return tid == gettid();
}

bool ManagedDatasource::endDBAction(char cmd, bool new_transaction) {
   AutoLocker al(&ds_lock);
   return endDBActionIntern(cmd, new_transaction);
}

void ManagedDatasource::setTransactionLockTimeout(int t_ms) {
   tl_timeout_ms = t_ms;
}

int ManagedDatasource::getTransactionLockTimeout() const {
   return tl_timeout_ms;
}

void ManagedDatasource::setAutoCommit(bool ac) {
   AutoLocker al(&ds_lock);
   wait_for_sql();
   Datasource::setAutoCommit(ac);
}

AbstractQoreNode *ManagedDatasource::select(const QoreString *query_str, const QoreListNode *args, ExceptionSink *xsink) {
   DatasourceActionHelper dbah(*this, xsink);
   if (!dbah)
      return 0;

   return Datasource::select(query_str, args, xsink);
}

// FIXME: should be a native DBI driver method
AbstractQoreNode *ManagedDatasource::selectRow(const QoreString *sql, const QoreListNode *args, ExceptionSink *xsink) {
   AbstractQoreNode *rv;

   {
      DatasourceActionHelper dbah(*this, xsink);
      if (!dbah)
	 return 0;

      rv = Datasource::selectRows(sql, args, xsink);
   }

   // return only hash of first row, if any
   if (rv && rv->getType() == NT_LIST) {
      QoreListNode *l = reinterpret_cast<QoreListNode *>(rv);
      AbstractQoreNode *h = l->shift();
      rv->deref(xsink);
      rv = h;
   }
   
   return rv;
}

AbstractQoreNode *ManagedDatasource::selectRows(const QoreString *query_str, const QoreListNode *args, ExceptionSink *xsink) {
   DatasourceActionHelper dbah(*this, xsink);
   if (!dbah)
      return 0;

   return Datasource::selectRows(query_str, args, xsink);
}

AbstractQoreNode *ManagedDatasource::exec(const QoreString *query_str, const QoreListNode *args, ExceptionSink *xsink) {
   DatasourceActionHelper dbah(*this, xsink, getAutoCommit() ? DAH_NONE : DAH_ACQUIRE);
   if (!dbah)
      return 0;

   //printd(0, "ManagedDatasource::exec() st=%d tid=%d\n", start_transaction, tid);

   return Datasource::exec(query_str, args, xsink);
}

bool ManagedDatasource::beginTransaction(ExceptionSink *xsink) {
   DatasourceActionHelper dbah(*this, xsink, getAutoCommit() ? DAH_NONE : DAH_ACQUIRE);
   if (!dbah)
      return false;

   Datasource::beginTransaction(xsink);
   //printd(0, "ManagedDatasource::beginTransaction() this=%08p isInTransaction()=%d\n", this, isInTransaction());

   return dbah.newTransaction();
}

int ManagedDatasource::commit(ExceptionSink *xsink) {
   DatasourceActionHelper dbah(*this, xsink, getAutoCommit() ? DAH_NONE : DAH_RELEASE);
   if (!dbah)
      return -1;

   return Datasource::commit(xsink);
}

int ManagedDatasource::rollback(ExceptionSink *xsink) {
   DatasourceActionHelper dbah(*this, xsink, getAutoCommit() ? DAH_NONE : DAH_RELEASE);
   if (!dbah)
      return -1;

   return Datasource::rollback(xsink);
}

int ManagedDatasource::open(ExceptionSink *xsink) {
   AutoLocker al(&ds_lock);
   if (wait_for_sql(xsink))
      return -1;
   return Datasource::open(xsink);
}

// returns 0 for OK, -1 for exception
int ManagedDatasource::closeUnlocked(ExceptionSink *xsink) {
   int rc = 0;

   // wait for any in-progress action to complete
   wait_for_sql();
   if (isOpen()) {
      if (isInTransaction()) {
	 if (!wasConnectionAborted()) {
	    // FIXME: check for statement
	    xsink->raiseException("DATASOURCE-TRANSACTION-EXCEPTION", "Datasource closed while in a transaction; transaction will be automatically rolled back and the lock released");
	    Datasource::rollback(xsink);
	 }
	 remove_thread_resource(this);
	 setTransactionStatus(false);
	 // force-exit the transaction lock
	 forceReleaseLock();
	 rc = -1;
      }

      Datasource::close();
   }
   
   return rc;
}

int ManagedDatasource::close(ExceptionSink *xsink) {
   AutoLocker al(&ds_lock);
   return closeUnlocked(xsink);
}

// forces a close and open to reset a database connection
void ManagedDatasource::reset(ExceptionSink *xsink) {
   AutoLocker al(&ds_lock);
   closeUnlocked(xsink);
   // open the connection
   Datasource::open(xsink);
}

void ManagedDatasource::setPendingUsername(const char *u) {
   AutoLocker al(&ds_lock);
   Datasource::setPendingUsername(u);
}

void ManagedDatasource::setPendingPassword(const char *p) {
   AutoLocker al(&ds_lock);
   Datasource::setPendingPassword(p);
}

void ManagedDatasource::setPendingDBName(const char *d) {
   AutoLocker al(&ds_lock);
   Datasource::setPendingDBName(d);
}

void ManagedDatasource::setPendingDBEncoding(const char *c) {
   AutoLocker al(&ds_lock);
   Datasource::setPendingDBEncoding(c);
}

void ManagedDatasource::setPendingHostName(const char *h) {
   AutoLocker al(&ds_lock);
   Datasource::setPendingHostName(h);
}

void ManagedDatasource::setPendingPort(int port) {
   AutoLocker al(&ds_lock);
   Datasource::setPendingPort(port);
}

QoreStringNode *ManagedDatasource::getPendingUsername() const {
   AutoLocker al(&ds_lock);
   return Datasource::getPendingUsername();
}

QoreStringNode *ManagedDatasource::getPendingPassword() const {
   AutoLocker al(&ds_lock);
   return Datasource::getPendingPassword();
}

QoreStringNode *ManagedDatasource::getPendingDBName() const {
   AutoLocker al(&ds_lock);
   return Datasource::getPendingDBName();
}

QoreStringNode *ManagedDatasource::getPendingDBEncoding() const {
   AutoLocker al(&ds_lock);
   return Datasource::getPendingDBEncoding();
}

QoreStringNode *ManagedDatasource::getPendingHostName() const {
   AutoLocker al(&ds_lock);
   return Datasource::getPendingHostName();
}

int ManagedDatasource::getPendingPort() const {
   AutoLocker al(&ds_lock);
   return Datasource::getPendingPort();
}

AbstractQoreNode *ManagedDatasource::getServerVersion(ExceptionSink *xsink) {
   DatasourceActionHelper dbah(*this, xsink);
   if (!dbah)
      return 0;

   return Datasource::getServerVersion(xsink);
}

AbstractQoreNode *ManagedDatasource::getClientVersion(ExceptionSink *xsink) const {
   return Datasource::getClientVersion(xsink);
}
