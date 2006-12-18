/*
 IfStatement.cc
 
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
#include <qore/common.h>
#include <qore/IfStatement.h>
#include <qore/Statement.h>
#include <qore/Variable.h>
#include <qore/QoreNode.h>

IfStatement::IfStatement(class QoreNode *c, class StatementBlock *i, class StatementBlock *e)
{
   cond = c;
   if_code = i;
   else_code = e;
   lvars = NULL;
}

IfStatement::~IfStatement()
{
   cond->deref(NULL);
   if (if_code)
      delete if_code;
   if (else_code)
      delete else_code;
   if (lvars)
      delete lvars;
}

// only executed by Statement::exec()
int IfStatement::exec(class QoreNode **return_value, class ExceptionSink *xsink)
{
   int i, rc = 0;
   
   tracein("IfStatement::exec()");
   // instantiate local variables
   for (i = 0; i < lvars->num_lvars; i++)
      instantiateLVar(lvars->ids[i], NULL);
   
   if (cond->boolEval(xsink))
   {
      if (!xsink->isEvent() && if_code)
	 rc = if_code->exec(return_value, xsink);
   }
   else if (else_code)
      rc = else_code->exec(return_value, xsink);
   
   // uninstantiate local variables
   for (i = 0; i < lvars->num_lvars; i++)
      uninstantiateLVar(xsink);
   traceout("IfStatement::exec()");
   return rc;
}

void IfStatement::parseInit(lvh_t oflag, int pflag)
{
   int i, lvids = 0;
   
   lvids += process_node(&cond, oflag, pflag);
   if (if_code)
      if_code->parseInit(oflag, pflag);
   if (else_code)
      else_code->parseInit(oflag, pflag);
   // save local variables
   lvars = new LVList(lvids);
   for (i = 0; i < lvids; i++)
      lvars->ids[i] = pop_local_var();
}
