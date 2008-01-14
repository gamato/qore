/*
  QT_date.cc

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
#include <qore/intern/QT_date.h>

class QoreNode *date_DefaultValue()
{
   ZeroDate->ref();
   return ZeroDate;
}

class QoreNode *date_Copy(const QoreNode *l, class ExceptionSink *xsink)
{
   return new QoreNode(new DateTime(*(l->val.date_time)));
}

void date_DeleteContents(class QoreNode *n)
{
   delete n->val.date_time;
}
