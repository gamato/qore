/*
 DoWhileStatement.cc
 
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

#include <qore/Qore.h>
#include <qore/DoWhileStatement.h>

int DoWhileStatement::execImpl(class QoreNode **return_value, class ExceptionSink *xsink)
{
   int i, rc = 0;
   
   tracein("WhileStatement::execDoWhile()");
   // instantiate local variables
   for (i = 0; i < lvars->num_lvars; i++)
      instantiateLVar(lvars->ids[i], NULL);
   
   do
   {
      if (code && (((rc = code->execImpl(return_value, xsink)) == RC_BREAK) || xsink->isEvent()))
      {
	 rc = 0;
	 break;
      }
      if (rc == RC_RETURN)
	 break;
      else if (rc == RC_CONTINUE)
	 rc = 0;
   } while (cond->boolEval(xsink) && !xsink->isEvent());
   
   // uninstantiate local variables
   for (i = 0; i < lvars->num_lvars; i++)
      uninstantiateLVar(xsink);
   traceout("DoWhileStatement::execImpl()");
   return rc;
}

/* do ... while statements can have variables local to the statement
 * however, it doesn't do much good :-) */
int DoWhileStatement::parseInitImpl(lvh_t oflag, int pflag)
{
   int i, lvids = 0;
   
   if (code)
      code->parseInitImpl(oflag, pflag);
   lvids += process_node(&cond, oflag, pflag);
   
   // save local variables
   lvars = new LVList(lvids);
   for (i = 0; i < lvids; i++)
      lvars->ids[i] = pop_local_var();

   return 0;
}
