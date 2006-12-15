/*
  QoreGetOpt.cc

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
#include <qore/QoreGetOpt.h>
#include <qore/Hash.h>
#include <qore/List.h>
#include <qore/Exception.h>
#include <qore/QoreString.h>
#include <qore/QoreNode.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

QoreGetOptNode::QoreGetOptNode(char *n, char so, char *lo, class QoreType *at, int o)
{
   name = n  ? strdup(n) : NULL;
   short_opt = so;
   long_opt = lo ? strdup(lo) : NULL;
   argtype = at;
   option = o;
}

QoreGetOptNode::~QoreGetOptNode()
{
   if (name)
      free(name);
   if (long_opt)
      free(long_opt);
}

QoreGetOpt::QoreGetOpt()
{
}

QoreGetOpt::~QoreGetOpt()
{
   getopt_node_list_t::iterator i;
   while ((i = node_list.begin()) != node_list.end())
   {
      class QoreGetOptNode *n = *i;
      node_list.erase(i);
      delete n;
   }
   long_map.clear();
   short_map.clear();
}

class QoreGetOptNode *QoreGetOpt::find(char *opt) const
{
   getopt_long_map_t::const_iterator i = long_map.find(opt);
   if (i != long_map.end())
      return i->second;
   return NULL;
}

class QoreGetOptNode *QoreGetOpt::find(char opt) const
{
   getopt_short_map_t::const_iterator i = short_map.find(opt);
   if (i != short_map.end())
      return i->second;
   return NULL;
}

int QoreGetOpt::add(char *name, char short_opt, char *long_opt, class QoreType *argtype, int option)
{
   // check input for validity
   if (!name || !name[0])
      return QGO_ERR_NO_NAME;
   if (!short_opt && (!long_opt || !long_opt[0]))
      return QGO_ERR_NO_OPTION;

   //printf("QoreGetOpt::add(%s, %03d (%c), %s, %08p, %d)\n", name, short_opt, short_opt ? short_opt : '-', long_opt, argtype, option);
   // look for duplicate entries
   if (short_opt && find(short_opt))
      return QGO_ERR_DUP_SHORT_OPT;
   if (long_opt && find(long_opt))
      return QGO_ERR_DUP_LONG_OPT;

   class QoreGetOptNode *n = new QoreGetOptNode(name, short_opt, long_opt, argtype, option);
   if (short_opt)
      short_map[short_opt] = n;
   if (long_opt)
      long_map[n->long_opt] = n;
   node_list.push_back(n);
   
   return 0;
}

static void inline addError(class Hash *h, QoreString *err)
{
   //printd(5, "addError() adding: %s\n", err->getBuffer());
   class QoreNode **v = h->getKeyValuePtr("_ERRORS_");
   if (!(*v))
      (*v) = new QoreNode(new List());
   (*v)->val.list->push(new QoreNode(err));
}

// private, static method
class QoreNode *QoreGetOpt::parseDate(char *val)
{
   // check for ISO-8601 or qore date formats 
   // 2006-01-01              (10)
   // 2006-01-01T10:00:00     (19)
   // 2006-01-01T10:00:00.000 (23)
   int len = strlen(val);
   
   if (len >= 10)
   {
      char *c = strchr(val, '-');
      if (c == (val + 4))
      {
	 QoreString str(val, 4);
	 str.concat(val + 5, 2);
	 str.concat(val + 8, 2);

	 // if time component is there
	 if (len >= 19 && (val[10] == 'T' || val[10] == '-'))
	 {
	    str.concat(val + 11, 2);
	    str.concat(val + 14, 2);
	    str.concat(val + 17, 2);
	    if (len == 23)
	       str.concat(val + 19);
	 }
	 return new QoreNode(new DateTime(str.getBuffer()));
      }
      // fall through to default date parse below
   }

   return new QoreNode(new DateTime(val));
}

void QoreGetOpt::doOption(class QoreGetOptNode *n, class Hash *h, char *val)
{
   // get current value
   class QoreNode **cv = h->getKeyValuePtr(n->name);

   // get a value ready
   if (!n->argtype)
   {
      if (*cv)
	 return;
      (*cv) = boolean_true();
      return;
   }

   // handle option values
   if (!val)
   {
      if (n->option & QGO_OPT_ADDITIVE)
	 if (n->argtype == NT_INT)
	 {
	    if (!(*cv))
	       (*cv) = new QoreNode((int64)0);
	    (*cv)->val.intval++;
	 }
	 else
	 {
	    if (!(*cv))
	       (*cv) = new QoreNode((double)0.0);
	    (*cv)->val.floatval++;
	 }
      else if (!*cv)
	 (*cv) = boolean_true();
      return;
   }

   class QoreNode *v;
   if (n->argtype == NT_STRING)
      v = new QoreNode(val);
   else if (n->argtype == NT_INT)
      v = new QoreNode(strtoll(val, NULL, 10));
   else if (n->argtype == NT_FLOAT)
      v = new QoreNode(strtod(val, NULL));
   else if (n->argtype == NT_DATE)
      v = parseDate(val);
   else if (n->argtype == NT_BOOLEAN)
      v = new QoreNode((bool)strtol(val, NULL, 10));
   else // default string
      v = new QoreNode(val);
   
   if (!(n->option & QGO_OPT_LIST_OR_ADD))
   {
      if (*cv)
	 (*cv)->deref(NULL);
      (*cv) = v;
      return;
   }

   if (n->option & QGO_OPT_LIST)
   {
      if (!(*cv))
	 (*cv) = new QoreNode(new List());
      //else printf("cv->type=%s\n", cv->type->name);
      (*cv)->val.list->push(v);
      return;
   }
   
   // additive
   if (*cv) 
   {
      if (n->argtype == NT_INT)
	 (*cv)->val.intval += v->val.intval;
      else  // float
	 (*cv)->val.floatval += v->val.floatval;
      v->deref(NULL);
      return;
   }

   (*cv) = v;
}

char *QoreGetOpt::getNextArgument(class List *l, class Hash *h, int &i, char *lopt, char sopt)
{
   if (i < (l->size() - 1))
   {
      i++;
      class QoreNode *n = l->retrieve_entry(i);
      if (n && n->type == NT_STRING)
	 return n->val.String->getBuffer();
   }
   QoreString *err = new QoreString();
   if (lopt)
      err->sprintf("long option '--%s' requires an argument", lopt);
   else
      err->sprintf("short option '-%c' requires an argument", sopt);
   addError(h, err);
   return NULL;
}

void QoreGetOpt::processLongArg(char *arg, class List *l, class Hash *h, int &i, bool modify)
{
   char *opt;
   char *val;

   // get a copy of the argument
   QoreString vstr(arg);
   arg = vstr.getBuffer();

   // see if there is an assignment character
   char *tok = strchr(arg, '=');
   if (tok)
   {
      (*tok) = '\0';
      opt = arg;
      val = tok + 1;
   }
   else
   {  
      opt = arg;
      val = NULL;
   }
   // find option
   class QoreGetOptNode *w = find(opt);
   if (!w)
   {
      QoreString *err = new QoreString();
      err->sprintf("unknown long option '--%s'", opt);
      addError(h, err);
      return;
   }
   bool do_modify = false;
   // if we need a value and there isn't one, then try to get the next argument in the list
   if (w->argtype && !val && (w->option & QGO_OPT_MANDATORY))
   {
      val = getNextArgument(l, h, i, opt, '\0');
      if (val && modify)
	 do_modify = true;
      if (!val)
	 return;
   }
   doOption(w, h, val);
   if (do_modify)
      l->pop_entry(--i, NULL);
}

int QoreGetOpt::processShortArg(char *arg, class List *l, class Hash *h, int &i, int &j, bool modify)
{
   char opt = (arg + j)[0];
   // find option
   class QoreGetOptNode *w = find(opt);
   if (!w)
   {
      QoreString *err = new QoreString();
      err->sprintf("unknown short option '-%c'", opt);
      addError(h, err);
      return 0;
   }
   bool do_modify = false;
   char *val = NULL;
   if (w->argtype)
   {
      if ((j < (signed)(strlen(arg) - 1))
	  && ((w->option & QGO_OPT_MANDATORY) || ((arg + j + 1)[0] == '=')))
      {
	 val = arg + j + 1;
	 if (*val == '=')
	    val++;
	 j = 0;
      }
      else if (w->option & QGO_OPT_MANDATORY)
      {
	 if (!(val = getNextArgument(l, h, i, NULL, opt)))
	    return 0;
	 if (modify)
	    do_modify = true;
      }
   }
   doOption(w, h, val);
   if (do_modify)
      l->pop_entry(--i, NULL);
   //printd(5, "processShortArg(%c) val=%08p %s returning %d\n", opt, val, val, !j);
   return !j;
}

class Hash *QoreGetOpt::parse(class List *l, bool modify, class ExceptionSink *xsink)
{
   class Hash *h = new Hash();
   for (int i = 0; i < l->size(); i++)
   {
      //printf("QoreGetOpt::parse() %d/%d\n", i, l->size());
      class QoreNode *n = l->retrieve_entry(i);
      if (!n || n->type != NT_STRING)
	 continue;

      char *arg = n->val.String->getBuffer();
      if (arg[0] == '-')
      {
	 if (!arg[1])
	    continue;
	 if (arg[1] == '-')
	 {
	    if (!arg[2])
	       break;
	    processLongArg(arg + 2, l, h, i, modify);
	    if (modify)
	    {
	       //printd(5, "parse() opt=%s size=%d\n", arg, l->size()); 
	       l->pop_entry(i--, NULL);
	       //printd(5, "parse() popped entry, size=%d\n", l->size());
	    }
	    continue;
	 }
	 int len = strlen(arg);
	 for (int j = 1; j < len; j++)
	    if (processShortArg(arg, l, h, i, j, modify))
	       break;
	 l->pop_entry(i--, NULL);
      }
   }
   //printd(5, "QoreGetOpt::parse() returning h=%08p (size %d)\n", h, h->size());
   return h;
}
