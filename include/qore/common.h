/*
  common.h

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

#ifndef _QORE_COMMON_H

#define _QORE_COMMON_H

#include <qore/config.h>

#include <string.h>

#ifdef _MSC_VER
  #ifdef BUILDING_DLL
    #define DLLEXPORT __declspec(dllexport)
  #else
    #define DLLEXPORT __declspec(dllimport)
  #endif
  #define DLLLOCAL
#else
  #ifdef HAVE_GCC_VISIBILITY
    #define DLLEXPORT __attribute__ ((visibility("default")))
    #define DLLLOCAL __attribute__ ((visibility("hidden")))
  #else
    #define DLLEXPORT
    #define DLLLOCAL
  #endif
#endif

class ltstr
{
  public:
   bool operator()(char* s1, char* s2) const
   {
      return strcmp(s1, s2) < 0;
   }
};

class ltchar
{
  public:
   bool operator()(char s1, char s2) const
   {
      return s1 < s2;
   }
};

class cltstr
{
  public:
   bool operator()(const char* s1, const char* s2) const
   {
      return strcmp(s1, s2) < 0;
   }
};

#include <set>
typedef std::set<char *, ltstr> strset_t;

typedef long long int64;
typedef char *lvh_t;

typedef class QoreNode *(*q_func_t)(class QoreNode *, class ExceptionSink *);
typedef class QoreNode *(*q_method_t)(class Object *, void *, class QoreNode *, class ExceptionSink *);
typedef void (*q_constructor_t)(class Object *, class QoreNode *, class ExceptionSink *);
typedef void (*q_destructor_t)(class Object *, void *, class ExceptionSink *);
typedef void (*q_copy_t)(class Object *, class Object *, void *, class ExceptionSink *);

#include <stdlib.h>

#ifndef HAVE_ATOLL
#ifdef HAVE_STRTOIMAX
#include <inttypes.h>
static inline long long atoll(char *str)
{
   return strtoimax(str, NULL, 10);
}
#else
static inline long long atoll(char *str)
{
   long long i;
   sscanf(str, "%lld", &i);
   return i;
}
#endif
#endif

#if !defined(HAVE_STRTOLL) && defined(HAVE_STRTOIMAX)
#include <inttypes.h>
#define strtoll strtoimax
#endif

#endif // _QORE_COMMON_H
