/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
  QoreSQLStatement.h

  Qore Programming Language

  Copyright (C) 2006 - 2010 Qore Technologies, sro
  
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

#ifndef _QORE_QORESQLSTATEMENT_H
#define _QORE_QORESQLSTATEMENT_H

#include <qore/intern/sql_statement_private.h>

class DatasourceStatementHelper;

#define STMT_IDLE      0
#define STMT_PREPARED  1
#define STMT_EXECED    2
#define STMT_DELETED   3

class QoreSQLStatement : public AbstractPrivateData, public SQLStatement {
protected:
   DLLLOCAL static const char *stmt_statuses[];

   // helper object for acquiring a Datasource pointer
   DatasourceStatementHelper *dsh;
   // status
   unsigned char status;

   DLLLOCAL static int invalidException(ExceptionSink *xsink) {
      xsink->raiseException("SQLSTATMENT-ERROR", "TID %d attempted to acquire already deleted SQLStatement object", gettid());
      return -1;
   }

   DLLLOCAL int checkStatus(int stat, const char *action, ExceptionSink *xsink) {
      if (status == STMT_DELETED)
	 return invalidException(xsink);

      if (stat != status) {
	 if (stat == STMT_IDLE)
	    return closeIntern(xsink);

         xsink->raiseException("SQLSTATMENT-ERROR", "SQLStatement::%s() called expecting status '%s', but statement has status '%s'", action, stmt_statuses[stat], stmt_statuses[status]);
         return -1;
      }

      return 0;
   }

   DLLLOCAL int closeIntern(ExceptionSink *xsink);

public:
   DLLLOCAL QoreSQLStatement() : dsh(0), status(STMT_IDLE) {
   }

   DLLLOCAL ~QoreSQLStatement();

   DLLLOCAL int init(DatasourceStatementHelper *n_dsh, ExceptionSink *xsink);

   DLLLOCAL virtual void deref(ExceptionSink *xsink);

   DLLLOCAL int prepare(QoreString &str, ExceptionSink *xsink);

   DLLLOCAL int bind(QoreListNode &l, ExceptionSink *xsink);

   DLLLOCAL int exec(ExceptionSink *xsink);

   DLLLOCAL int close(ExceptionSink *xsink);

   DLLLOCAL bool next(ExceptionSink *xsink);

   DLLLOCAL QoreListNode *fetchRow(ExceptionSink *xsink);

   DLLLOCAL bool active() const;
};

#endif
