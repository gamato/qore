/*
 QC_QFileDialog.h
 
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

#ifndef _QORE_QT_QC_QFILEDIALOG_H

#define _QORE_QT_QC_QFILEDIALOG_H

#include <QFileDialog>
#include "QoreAbstractQDialog.h"
#include "qore-qt-events.h"

DLLLOCAL extern qore_classid_t CID_QFILEDIALOG;
DLLLOCAL extern class QoreClass *QC_QFileDialog;

DLLLOCAL QoreNamespace *initQFileDialogNS(QoreClass *);
DLLLOCAL void initQFileDialogStaticFunctions();

class myQFileDialog : public QFileDialog, public QoreQDialogExtension
{
#define QOREQTYPE QFileDialog
#define MYQOREQTYPE myQFileDialog
#include "qore-qt-metacode.h"
#include "qore-qt-qdialog-methods.h"
#undef MYQOREQTYPE
#undef QOREQTYPE

   public:
      DLLLOCAL myQFileDialog(QoreObject *obj, QWidget* parent, Qt::WindowFlags flags) : QFileDialog(parent, flags), QoreQDialogExtension(obj->getClass())
      {
         init(obj);
      }
      DLLLOCAL myQFileDialog(QoreObject *obj, QWidget* parent = 0, const QString& caption = QString(), const QString& directory = QString(), const QString& filter = QString()) : QFileDialog(parent, caption, directory, filter), QoreQDialogExtension(obj->getClass())
      {
         init(obj);
      }
};

class QoreQFileDialog : public QoreAbstractQDialog
{
   public:
      QPointer<myQFileDialog> qobj;

      DLLLOCAL QoreQFileDialog(QoreObject *obj, QWidget* parent, Qt::WindowFlags flags) : qobj(new myQFileDialog(obj, parent, flags))
      {
      }
      DLLLOCAL QoreQFileDialog(QoreObject *obj, QWidget* parent = 0, const QString& caption = QString(), const QString& directory = QString(), const QString& filter = QString()) : qobj(new myQFileDialog(obj, parent, caption, directory, filter))
      {
      }
      DLLLOCAL virtual class QObject *getQObject() const
      {
         return static_cast<QObject *>(&(*qobj));
      }
      DLLLOCAL virtual class QWidget *getQWidget() const
      {
         return static_cast<QWidget *>(&(*qobj));
      }
      DLLLOCAL virtual QPaintDevice *getQPaintDevice() const
      {
         return static_cast<QPaintDevice *>(&(*qobj));
      }
      DLLLOCAL virtual class QDialog *getQDialog() const
      {
         return static_cast<QDialog *>(&(*qobj));
      }
      QORE_VIRTUAL_QDIALOG_METHODS
};

#endif // _QORE_QT_QC_QFILEDIALOG_H
