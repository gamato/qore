/*
  QoreHash.h

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

#ifndef _QORE_QOREHASH_H

#define _QORE_QOREHASH_H

#include <qore/common.h>
#include <qore/hash_map.h>

#include <stdio.h>
#include <string.h>

// FIXME: use STL list instead
// to maintain the order of inserts
class HashMember {
   public:
      class AbstractQoreNode *node;
      char *key;
      class HashMember *next;
      class HashMember *prev;
};

class HashIterator
{
   private:
      class QoreHash *h;
      class HashMember *ptr;

      // not implemented
      DLLLOCAL HashIterator(const HashIterator&);
      DLLLOCAL HashIterator& operator=(const HashIterator&);

   public:
      DLLEXPORT HashIterator(class QoreHash *h);
      DLLEXPORT HashIterator(class QoreHash &h);
      DLLEXPORT class HashMember *next();
      DLLEXPORT const char *getKey() const;
      // caller owns QoreString returned
      DLLEXPORT class QoreString *getKeyString() const;
      DLLEXPORT class AbstractQoreNode *getValue() const;
      // deletes the key from the hash and returns the value, caller owns the reference
      DLLEXPORT class AbstractQoreNode *takeValueAndDelete();
      // deletes the key from the hash and dereferences the value
      DLLEXPORT void deleteKey(class ExceptionSink *xsink);
      DLLEXPORT class AbstractQoreNode **getValuePtr() const;
      DLLEXPORT class AbstractQoreNode *eval(class ExceptionSink *xsink) const;
      DLLEXPORT bool first() const;
      DLLEXPORT bool last() const;
      //DLLEXPORT void setValue(class AbstractQoreNode *val, class ExceptionSink *xsink);
};

class ConstHashIterator
{
   private:
      const QoreHash *h;
      class HashMember *ptr;

      // not implemented
      DLLLOCAL ConstHashIterator(const HashIterator&);
      DLLLOCAL ConstHashIterator& operator=(const HashIterator&);

   public:
      DLLEXPORT ConstHashIterator(const class QoreHash *h);
      DLLEXPORT ConstHashIterator(const class QoreHash &h);
      DLLEXPORT class HashMember *next();
      DLLEXPORT const char *getKey() const;
      // caller owns QoreString returned
      DLLEXPORT class QoreString *getKeyString() const;
      DLLEXPORT const AbstractQoreNode *getValue() const;
      DLLEXPORT AbstractQoreNode *eval(class ExceptionSink *xsink) const;
      DLLEXPORT bool first() const;
      DLLEXPORT bool last() const;
};

class QoreHash
{
      friend class HashIterator;
      friend class ConstHashIterator;

   private:
      DLLLOCAL class AbstractQoreNode **newKeyValue(const char *key, class AbstractQoreNode *value);
      // does not touch the AbstractQoreNode value
      DLLLOCAL void internDeleteKey(class HashMember *m);
      DLLLOCAL void deref_intern(class ExceptionSink *xsink);
      DLLLOCAL void assimilate_intern(QoreHash *h, class ExceptionSink *xsink);

      // not implemented
      DLLLOCAL QoreHash(const QoreHash&);
      DLLLOCAL QoreHash& operator=(const QoreHash&);

  protected:
      class HashMember *member_list;
      class HashMember *tail;
      int len;
      hm_hm_t hm;
      bool needs_eval;

      int eval_intern(QoreHash *new_hash, class ExceptionSink *xsink) const;
      void copy_intern(QoreHash *new_hash) const;
      DLLEXPORT ~QoreHash();

   public:
      DLLEXPORT QoreHash(bool ne = false);

      DLLEXPORT const char *getFirstKey() const;
      DLLEXPORT const char *getLastKey() const;
      // returns (AbstractQoreNode *)-1 if the key doesn't exist
      DLLEXPORT AbstractQoreNode *getKeyValueExistence(const char *key);
      // returns (AbstractQoreNode *)-1 if the key doesn't exist
      DLLEXPORT const AbstractQoreNode *getKeyValueExistence(const char *key) const;
      // returns (AbstractQoreNode *)-1 if the key doesn't exist
      DLLEXPORT AbstractQoreNode *getKeyValueExistence(const class QoreString *key, class ExceptionSink *xsink);
      // returns (AbstractQoreNode *)-1 if the key doesn't exist
      DLLEXPORT const AbstractQoreNode *getKeyValueExistence(const class QoreString *key, class ExceptionSink *xsink) const;
      DLLEXPORT AbstractQoreNode *getKeyValue(const class QoreString *key, class ExceptionSink *xsink);
      DLLEXPORT const AbstractQoreNode *getKeyValue(const class QoreString *key, class ExceptionSink *xsink) const;
      DLLEXPORT AbstractQoreNode *getKeyValue(const char *key);
      DLLEXPORT const AbstractQoreNode *getKeyValue(const char *key) const;
      DLLEXPORT class QoreHash *copy() const;
      DLLEXPORT class QoreHashNode *copyNode() const;
      DLLEXPORT class AbstractQoreNode **getKeyValuePtr(const class QoreString *key, class ExceptionSink *xsink);
      DLLEXPORT class AbstractQoreNode **getKeyValuePtr(const char *key);
      DLLEXPORT class AbstractQoreNode **getExistingValuePtr(const class QoreString *key, class ExceptionSink *xsink);
      DLLEXPORT class AbstractQoreNode **getExistingValuePtr(const char *key);
      DLLEXPORT void merge(const class QoreHash *h, class ExceptionSink *xsink);
      DLLEXPORT void assimilate(class QoreHash *h, class ExceptionSink *xsink);
      DLLEXPORT class QoreHash *eval(class ExceptionSink *xsink) const;
      // FIXME: change to const QoreString * so encodings can be taken into consideration
      DLLEXPORT class AbstractQoreNode *evalKey(const char *key, class ExceptionSink *xsink) const;
      // FIXME: change to const QoreString * so encodings can be taken into consideration
      DLLEXPORT class AbstractQoreNode *evalKeyExistence(const char *key, class ExceptionSink *xsink) const;
      DLLEXPORT void setKeyValue(const class QoreString *key, class AbstractQoreNode *value, class ExceptionSink *xsink);
      DLLEXPORT void setKeyValue(const char *key, class AbstractQoreNode *value, class ExceptionSink *xsink);
      DLLEXPORT void deleteKey(const class QoreString *key, class ExceptionSink *xsink);
      DLLEXPORT void deleteKey(const char *key, class ExceptionSink *xsink);
      // "takes" the value of the key from the hash and removes the key from the hash and returns the value
      DLLEXPORT class AbstractQoreNode *takeKeyValue(const class QoreString *key, class ExceptionSink *xsink);
      // "takes" the value of the key from the hash and removes the key from the hash and returns the value
      DLLEXPORT class AbstractQoreNode *takeKeyValue(const char *key);
      DLLEXPORT class QoreListNode *getKeys() const;
      DLLEXPORT bool compareSoft(const QoreHash *h, class ExceptionSink *xsink) const;
      DLLEXPORT bool compareHard(const QoreHash *h, class ExceptionSink *xsink) const;
      DLLEXPORT class AbstractQoreNode *evalFirstKeyValue(class ExceptionSink *xsink) const;
      DLLEXPORT void dereference(class ExceptionSink *xsink);
      DLLEXPORT void derefAndDelete(class ExceptionSink *xsink);
      DLLEXPORT int size() const;
      DLLEXPORT bool needsEval() const;
      // get string representation (for %n and %N), foff is for multi-line formatting offset, -1 = no line breaks
      // the ExceptionSink is only needed for QoreObject where a method may be executed
      // returns -1 for exception raised, 0 = OK
      DLLEXPORT int getAsString(QoreString &str, int foff, class ExceptionSink *xsink) const;
      // if del is true, then the returned QoreString * should be deleted, if false, then it must not be
      DLLEXPORT QoreString *getAsString(bool &del, int foff, class ExceptionSink *xsink) const;

      DLLLOCAL void clearNeedsEval();
};

class StackHash : public QoreHash
{
   private:
      class ExceptionSink *xsink;
   
      // none of these operators/methods are implemented - here to make sure they are not used
      DLLLOCAL void *operator new(size_t); 
      DLLLOCAL StackHash();
      DLLLOCAL StackHash(bool i);
      DLLLOCAL void derefAndDelete(class ExceptionSink *xsink);
   
   public:
      DLLEXPORT StackHash(class ExceptionSink *xs)
      {
	 xsink = xs;
      }
      DLLEXPORT ~StackHash()
      {
	 dereference(xsink);
      }
};

class TempQoreHash {
  private:
   QoreHash *h;
   ExceptionSink *xsink;

   DLLLOCAL TempQoreHash(const TempQoreHash&); // not implemented
   DLLLOCAL TempQoreHash& operator=(const TempQoreHash&); // not implemented
   DLLLOCAL void* operator new(size_t); // not implemented, make sure it is not new'ed

  public:
   DLLLOCAL TempQoreHash(QoreHash *nh, ExceptionSink *xs) : h(nh), xsink(xs)
   {
   }
   DLLLOCAL TempQoreHash(ExceptionSink *xs) : h(0), xsink(xs)
   {
   }
   DLLLOCAL ~TempQoreHash()
   {
      if (h)
	 h->derefAndDelete(xsink);
   }
   DLLLOCAL QoreHash *operator->() { return h; }
   DLLLOCAL QoreHash *operator*() { return h; }
   DLLLOCAL void operator=(QoreHash *nv) { if (h) h->derefAndDelete(xsink); h = nv; }
   DLLLOCAL QoreHash *release() 
   {
      QoreHash *rv = h;
      h = 0;
      return rv;
   }
};

#endif // _QORE_HASH_H
