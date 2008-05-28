/*
 qore-qt.h
 
 Qore Programming Language
 
 Copyright 2003 - 2008 David Nichols
 
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

#ifndef _QORE_QORE_QT_H
#define _QORE_QORE_QT_H

#include <QDate>
#include <QDateTime>
#include <QKeySequence>
#include <QBrush>
#include <QObject>
#include <QList>

class QAbstractButton;

#include "BrushStyleNode.h"
#include "PenStyleNode.h"

DLLEXPORT extern AbstractQoreNode *C_Clipboard;

extern int static_argc;
extern char **static_argv;

#include <map>

typedef std::map<int, const char *> qt_enum_map_t;

DLLEXPORT int get_qdate(const AbstractQoreNode *n, QDate &date, class ExceptionSink *xsink);
DLLEXPORT int get_qtime(const AbstractQoreNode *n, QTime &time, class ExceptionSink *xsink);
DLLEXPORT int get_qdatetime(const AbstractQoreNode *n, QDateTime &dt, class ExceptionSink *xsink);
DLLEXPORT int get_qbrush(const AbstractQoreNode *n, QBrush &brush, class ExceptionSink *xsink);
DLLEXPORT int get_qvariant(const AbstractQoreNode *n, QVariant &qv, class ExceptionSink *xsink, bool suppress_exception = false);
DLLEXPORT int get_qbytearray(const AbstractQoreNode *n, QByteArray &qba, class ExceptionSink *xsink, bool suppress_exception = false);
DLLEXPORT int get_qchar(const AbstractQoreNode *n, QChar &c, class ExceptionSink *xsink, bool suppress_exception = false);
DLLEXPORT int get_qstring(const AbstractQoreNode *n, QString &str, class ExceptionSink *xsink, bool suppress_exception = false);
DLLEXPORT int get_qkeysequence(const AbstractQoreNode *n, QKeySequence &qks, class ExceptionSink *xsink, bool suppress_exception = false);

DLLEXPORT QoreObject *return_object(const QoreClass *qclass, AbstractPrivateData *data);
DLLEXPORT QoreObject *return_qabstractbutton(QAbstractButton *button);
DLLEXPORT QoreObject *return_qstyle(const QString &style, class QStyle *qs, ExceptionSink *xsink);
DLLEXPORT QoreObject *return_qobject(QObject *o);
DLLEXPORT QoreObject *return_qstyleoption(const class QStyleOption *qso);
DLLEXPORT QoreObject *return_qevent(class QEvent *event);
DLLEXPORT QoreObject *return_qaction(class QAction *action);
DLLEXPORT QoreObject *return_qwidget(class QWidget *widget, bool managed = true);
DLLEXPORT AbstractQoreNode *return_qvariant(const QVariant &qv);
DLLEXPORT QoreListNode *return_qstringlist(const QStringList &l);

#endif
