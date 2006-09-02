/*
  ql_time.cc

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
#include <qore/ql_time.h>
#include <qore/QoreNode.h>
#include <qore/support.h>
#include <qore/QoreString.h>
#include <qore/DateTime.h>
#include <qore/params.h>
#include <qore/QoreLib.h>
#include <qore/BuiltinFunctionList.h>

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

// define qore_gettime() for various platforms to get time in nanoseconds
#ifdef HAVE_CLOCK_GETTIME
// define qore_gettime() for POSIX platforms
typedef struct timespec qore_timespec_t;
static inline void qore_gettime(qore_timespec_t *tp)
{
   clock_gettime(CLOCK_REALTIME, tp);
}
#else
// use gettimeofday() to get microsecond resolution and multiply by 1000
struct qore_timespec_t
{
      unsigned tv_sec;
      unsigned tv_nsec;
};
static inline void qore_gettime(qore_timespec_t *tp)
{
   struct timeval tv;
   gettimeofday(&tv, NULL);
   tp->tv_sec = tv.tv_sec;
   tp->tv_nsec = tv.tv_usec * 1000;
}
#endif

static class QoreNode *f_now(class QoreNode *params, ExceptionSink *xsink)
{
   time_t ct;

   ct = time(NULL);

   struct tm tms;
   class DateTime *dt = new DateTime(q_localtime(&ct, &tms));
   //printf("f_now() %d\n", ct);

   return new QoreNode(dt);
}

static class QoreNode *f_format_date(class QoreNode *params, ExceptionSink *xsink)
{
   class QoreNode *temp, *p0, *p1;

   if (!(p0 = test_param(params, NT_STRING, 0)) ||
       !(p1 = get_param(params, 1)))
      return NULL;

   tracein("format_date()");
   // second argument is converted to a date if necessary
   if (p1->type != NT_DATE)
      temp = p1->convert(NT_DATE);
   else
      temp = p1;

   class QoreString *rv = temp->val.date_time->format(p0->val.String->getBuffer());
   
   if (temp != p1)
      temp->deref(xsink);

   printd(5, "format_date() returning \"%s\"\n", rv->getBuffer());
   traceout("format_date()");

   return new QoreNode(rv);
}

static class QoreNode *f_localtime(class QoreNode *params, ExceptionSink *xsink)
{
   class QoreNode *p0;
   time_t t;

   if (!(p0 = get_param(params, 0)))
      return NULL;

   t = p0->getAsInt();

   struct tm tms;
   class DateTime *dt = new DateTime(q_localtime(&t, &tms));

   return new QoreNode(dt);
}

static class QoreNode *f_gmtime(class QoreNode *params, ExceptionSink *xsink)
{
   class QoreNode *p0;
   time_t t;

   if (!(p0 = get_param(params, 0)))
      return NULL;

   t = p0->getAsInt();

   struct tm tms;
   class DateTime *dt = new DateTime(q_gmtime(&t, &tms));

   return new QoreNode(dt);
}

static class QoreNode *f_mktime(class QoreNode *params, ExceptionSink *xsink)
{
   class QoreNode *temp, *p0;
   struct tm nt;
   time_t t;

   if (!(p0 = get_param(params, 0)))
      return NULL;
   if (p0->type == NT_DATE)
      temp = p0;
   else
      temp = p0->convert(NT_DATE);

   temp->val.date_time->getTM(&nt);

   t = mktime(&nt);

   if (temp != p0)
      temp->deref(xsink);

   return new QoreNode(NT_INT, t);
}

#ifdef HAVE_TIMEGM
static class QoreNode *f_timegm(class QoreNode *params, ExceptionSink *xsink)
{
   class QoreNode *temp, *p0;
   struct tm nt;
   time_t t;

   if (!(p0 = get_param(params, 0)))
      return NULL;
   if (p0->type != NT_DATE)
      temp = p0->convert(NT_DATE);
   else
      temp = p0;

   temp->val.date_time->getTM(&nt);
   t = timegm(&nt);

   if (temp != p0)
      temp->deref(xsink);

   return new QoreNode(NT_INT, t);
}
#endif

static class QoreNode *f_years(class QoreNode *params, ExceptionSink *xsink)
{
   int y;
   class QoreNode *p0 = get_param(params, 0);
   if (!p0)
      y = 0;
   else
      y = p0->getAsInt();
   
   class DateTime *dt = new DateTime();
   dt->year = y;
   dt->relative = 1;
   return new QoreNode(dt);
}

static class QoreNode *f_months(class QoreNode *params, ExceptionSink *xsink)
{
   int m;
   class QoreNode *p0 = get_param(params, 0);
   if (!p0)
      m = 0;
   else
      m = p0->getAsInt();
   
   class DateTime *dt = new DateTime();
   dt->month = m;
   dt->relative = 1;
   return new QoreNode(dt);
}

static class QoreNode *f_days(class QoreNode *params, ExceptionSink *xsink)
{
   int d;
   class QoreNode *p0 = get_param(params, 0);
   if (!p0)
      d = 0;
   else
      d = p0->getAsInt();
   
   class DateTime *dt = new DateTime();
   dt->day = d;
   dt->relative = 1;
   return new QoreNode(dt);
}

static class QoreNode *f_hours(class QoreNode *params, ExceptionSink *xsink)
{
   int h;
   class QoreNode *p0 = get_param(params, 0);
   if (!p0)
      h = 0;
   else
      h = p0->getAsInt();
   
   class DateTime *dt = new DateTime();
   dt->hour = h;
   dt->relative = 1;
   return new QoreNode(dt);
}

static class QoreNode *f_minutes(class QoreNode *params, ExceptionSink *xsink)
{
   int m;
   class QoreNode *p0 = get_param(params, 0);
   if (!p0)
      m = 0;
   else
      m = p0->getAsInt();
   
   class DateTime *dt = new DateTime();
   dt->minute = m;
   dt->relative = 1;
   return new QoreNode(dt);
}

static class QoreNode *f_seconds(class QoreNode *params, ExceptionSink *xsink)
{
   int s;
   class QoreNode *p0 = get_param(params, 0);
   if (!p0)
      s = 0;
   else
      s = p0->getAsInt();
   
   class DateTime *dt = new DateTime();
   dt->second = s;
   dt->relative = 1;
   return new QoreNode(dt);
}

static class QoreNode *f_milliseconds(class QoreNode *params, ExceptionSink *xsink)
{
   int m;
   class QoreNode *p0 = get_param(params, 0);
   if (!p0)
      m = 0;
   else
      m = p0->getAsInt();
   
   class DateTime *dt = new DateTime();
   dt->millisecond = m;
   dt->relative = 1;
   return new QoreNode(dt);
}

// returns an integer corresponding to the year value in the date
static class QoreNode *f_get_years(class QoreNode *params, ExceptionSink *xsink)
{
   class QoreNode *p0 = test_param(params, NT_DATE, 0);
   if (!p0)
      return NULL;
   
   return new QoreNode((int64)p0->val.date_time->year);
}

// returns an integer corresponding to the month value in the date
static class QoreNode *f_get_months(class QoreNode *params, ExceptionSink *xsink)
{
   class QoreNode *p0 = test_param(params, NT_DATE, 0);
   if (!p0)
      return NULL;
   
   return new QoreNode((int64)p0->val.date_time->month);
}

// returns an integer corresponding to the day value in the date
static class QoreNode *f_get_days(class QoreNode *params, ExceptionSink *xsink)
{
   class QoreNode *p0 = test_param(params, NT_DATE, 0);
   if (!p0)
      return NULL;
   
   return new QoreNode((int64)p0->val.date_time->day);
}

// returns an integer corresponding to the hour value in the date
static class QoreNode *f_get_hours(class QoreNode *params, ExceptionSink *xsink)
{
   class QoreNode *p0 = test_param(params, NT_DATE, 0);
   if (!p0)
      return NULL;
   
   return new QoreNode((int64)p0->val.date_time->hour);
}

// returns an integer corresponding to the minute value in the date
static class QoreNode *f_get_minutes(class QoreNode *params, ExceptionSink *xsink)
{
   class QoreNode *p0 = test_param(params, NT_DATE, 0);
   if (!p0)
      return NULL;
   
   return new QoreNode((int64)p0->val.date_time->minute);
}

// returns an integer corresponding to the second value in the date
static class QoreNode *f_get_seconds(class QoreNode *params, ExceptionSink *xsink)
{
   class QoreNode *p0 = test_param(params, NT_DATE, 0);
   if (!p0)
      return NULL;
   
   return new QoreNode((int64)p0->val.date_time->second);
}

// returns an integer corresponding to the millisecond value in the date
static class QoreNode *f_get_milliseconds(class QoreNode *params, ExceptionSink *xsink)
{
   class QoreNode *p0 = test_param(params, NT_DATE, 0);
   if (!p0)
      return NULL;
   
   return new QoreNode((int64)p0->val.date_time->millisecond);
}

// returns an integer corresponding to the number of the day in the year
static class QoreNode *f_getDayNumber(class QoreNode *params, ExceptionSink *xsink)
{
   class QoreNode *p0 = test_param(params, NT_DATE, 0);
   if (!p0)
      return NULL;
   
   return new QoreNode((int64)p0->val.date_time->getDayNumber());
}

// returns an integer corresponding to the number of the day in the year
static class QoreNode *f_getDayOfWeek(class QoreNode *params, ExceptionSink *xsink)
{
   class QoreNode *p0 = test_param(params, NT_DATE, 0);
   if (!p0)
      return NULL;
   
   return new QoreNode((int64)p0->val.date_time->getDayOfWeek());
}

// returns a hash giving the ISO-8601 values for the year and calendar week for the date passed
static class QoreNode *f_getISOWeekHash(class QoreNode *params, ExceptionSink *xsink)
{
   class QoreNode *p0 = test_param(params, NT_DATE, 0);
   if (!p0)
      return NULL;

   int year, week, day;
   p0->val.date_time->getISOWeek(year, week, day);
   class Hash *h = new Hash();
   h->setKeyValue("year", new QoreNode((int64)year), NULL);
   h->setKeyValue("week", new QoreNode((int64)week), NULL);
   h->setKeyValue("day", new QoreNode((int64)day), NULL);
   
   return new QoreNode(h);
}

// returns a string corresponding to the ISO-8601 year and calendar week for the date passed
static class QoreNode *f_getISOWeekString(class QoreNode *params, ExceptionSink *xsink)
{
   class QoreNode *p0 = test_param(params, NT_DATE, 0);
   if (!p0)
      return NULL;
   
   int year, week, day;
   p0->val.date_time->getISOWeek(year, week, day);
   class QoreString *str = new QoreString();
   str->sprintf("%04d-W%02d-%d", year, week, day);

   return new QoreNode(str);
}

// returns a date corresponding to the ISO-8601 calendar week information passed
// args: year, week #, [day #]
// note that ISO-8601 week days go from 1 - 7 = Mon - Sun
static class QoreNode *f_getDateFromISOWeek(class QoreNode *params, ExceptionSink *xsink)
{
   class QoreNode *pt = get_param(params, 0);
   int year = pt ? pt->getAsInt() : 0;

   pt = get_param(params, 1);
   int week = pt ? pt->getAsInt() : 0;

   // day number defaults to 1 = Monday, start of the week (7 = Sun)
   pt = get_param(params, 2);
   int day = pt ? pt->getAsInt() : 1;

   class DateTime *dt = DateTime::getDateFromISOWeek(year, week, day, xsink);
   if (!dt)
      return NULL;
   return new QoreNode(dt);
}

/**
 * f_clock_getmillis(class QoreNode *params, ExceptionSink *xsink)
 *
 * returns the current system clock time value as milliseconds
 * 20051114 aargon
 * updates for Darwin
 * 20051116 david_nichols
 */
static class QoreNode *f_clock_getmillis(class QoreNode *params, ExceptionSink *xsink) {
   struct timeval tv;
   gettimeofday(&tv, NULL);

   return new QoreNode(((int64)tv.tv_sec*(int64)1000 + tv.tv_usec/1000));
}

/*
 * qore: cock_getnanos()
 * returns the current system clock time value as nanoseconds since Jan 1, 1970
 */
static class QoreNode *f_clock_getnanos(class QoreNode *params, ExceptionSink *xsink) 
{
   qore_timespec_t tp;
   qore_gettime(&tp);

   return new QoreNode(((int64)tp.tv_sec * (int64)1000000000 + tp.tv_nsec)); 
}

/* qore: clock_getmicros()
 * returns the current system clock time value as microseconds since Jan 1, 1970
 */
static class QoreNode *f_clock_getmicros(class QoreNode *params, ExceptionSink *xsink) 
{
   struct timeval tv;
   gettimeofday(&tv, NULL);

   return new QoreNode(((int64)tv.tv_sec * (int64)1000000 + tv.tv_usec));
}

void init_time_functions()
{
   builtinFunctions.add("now", f_now);
   builtinFunctions.add("format_date", f_format_date);
   builtinFunctions.add("localtime", f_localtime);
   builtinFunctions.add("gmtime", f_gmtime);
   builtinFunctions.add("mktime", f_mktime);
#ifdef HAVE_TIMEGM
   builtinFunctions.add("timegm", f_timegm);
#endif

   builtinFunctions.add("years",               f_years);
   builtinFunctions.add("months",              f_months);
   builtinFunctions.add("days",                f_days);
   builtinFunctions.add("hours",               f_hours);
   builtinFunctions.add("minutes",             f_minutes);
   builtinFunctions.add("seconds",             f_seconds);
   builtinFunctions.add("milliseconds",        f_milliseconds);

   builtinFunctions.add("get_years",           f_get_years);
   builtinFunctions.add("get_months",          f_get_months);
   builtinFunctions.add("get_days",            f_get_days);
   builtinFunctions.add("get_hours",           f_get_hours);
   builtinFunctions.add("get_minutes",         f_get_minutes);
   builtinFunctions.add("get_seconds",         f_get_seconds);
   builtinFunctions.add("get_milliseconds",    f_get_milliseconds);

   builtinFunctions.add("getDayNumber",        f_getDayNumber);
   builtinFunctions.add("getDayOfWeek",        f_getDayOfWeek);
   builtinFunctions.add("getISOWeekHash",      f_getISOWeekHash);
   builtinFunctions.add("getISOWeekString",    f_getISOWeekString);
   builtinFunctions.add("getDateFromISOWeek",  f_getDateFromISOWeek);
   
   builtinFunctions.add("clock_getmillis",     f_clock_getmillis);
   builtinFunctions.add("clock_getnanos",      f_clock_getnanos);
   builtinFunctions.add("clock_getmicros",     f_clock_getmicros);
}
