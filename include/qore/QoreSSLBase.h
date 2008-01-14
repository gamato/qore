/*
  QoreSSLBase.h
  
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

#ifndef _QORE_QORESSLBASE_H

#define _QORE_QORESSLBASE_H

#include <openssl/ssl.h>
#include <openssl/x509v3.h>

class QoreSSLBase
{
  public:
   static class QoreHash *X509_NAME_to_hash(X509_NAME *n);
   static class DateTime *ASN1_TIME_to_DateTime(ASN1_STRING *t);
   static class QoreStringNode *ASN1_OBJECT_to_QoreStringNode(ASN1_OBJECT *o);
};

#endif // _QORE_CLASS_SSLBASE_H
