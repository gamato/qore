/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
  QoreTypeInfo.h

  Qore Programming Language

  Copyright (C) 2003 - 2017 Qore Technologies, s.r.o.

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.

  Note that the Qore library is released under a choice of three open-source
  licenses: MIT (as above), LGPL 2+, or GPL 2+; see README-LICENSE for more
  information.
*/

#ifndef _QORE_QORETYPEINFO_H

#define _QORE_QORETYPEINFO_H

#include <map>
#include <vector>
#include <utility>
#include <functional>

#define NO_TYPE_INFO "any"

// adds external types to global type map
DLLLOCAL void add_to_type_map(qore_type_t t, const QoreTypeInfo* typeInfo);
DLLLOCAL bool builtinTypeHasDefaultValue(qore_type_t t);
// returns the default value for any type >= 0 and < NT_OBJECT
DLLLOCAL AbstractQoreNode* getDefaultValueForBuiltinValueType(qore_type_t t);

DLLLOCAL void concatClass(std::string& str, const char* cn);

enum q_typespec_t : unsigned char {
   QTS_TYPE = 0,
   QTS_CLASS = 1,
   QTS_HASHDECL = 2,
};

class QoreTypeSpec {
public:
   DLLLOCAL QoreTypeSpec(qore_type_t t) : typespec(QTS_TYPE) {
      u.t = t;
   }

   DLLLOCAL QoreTypeSpec(const QoreClass* qc) : typespec(QTS_CLASS) {
      u.qc = qc;
   }

   DLLLOCAL QoreTypeSpec(const TypedHashDecl* hd) : typespec(QTS_HASHDECL) {
      u.hd = hd;
   }

   DLLLOCAL q_typespec_t getTypeSpec() const {
      return typespec;
   }

   DLLLOCAL qore_type_t getType() const {
      switch (typespec) {
         case QTS_TYPE: return u.t;
         case QTS_CLASS: return NT_OBJECT;
         case QTS_HASHDECL: return NT_HASH;
      }
   }

   DLLLOCAL const QoreClass* getClass() const {
      return typespec == QTS_CLASS ? u.qc : nullptr;
   }

   DLLLOCAL const TypedHashDecl* getHashDecl() const {
      return typespec == QTS_HASHDECL ? u.hd : nullptr;
   }

   DLLLOCAL qore_type_result_e matchType(qore_type_t t) const {
      if (typespec == QTS_CLASS)
         return t == NT_OBJECT ? QTI_IDENT : QTI_NOT_EQUAL;
      else if (typespec == QTS_HASHDECL)
         return t == NT_HASH ? QTI_IDENT : QTI_NOT_EQUAL;
      if (u.t == NT_ALL)
         return QTI_AMBIGUOUS;
      return u.t == t ? QTI_IDENT : QTI_NOT_EQUAL;
   }

   // this is the "expecting" type, t is the type to match
   // ex: this = class, t = NT_OBJECT, result = AMBIGU`OUS
   // ex: this = NT_OBJECT, t = class, result = IDENT
   DLLLOCAL qore_type_result_e match(const QoreTypeSpec& t) const;

   // this is the "expecting" type, t is the type to match
   // ex: this = class, t = NT_OBJECT, result = AMBIGU`OUS
   // ex: this = NT_OBJECT, t = class, result = IDENT
   DLLLOCAL qore_type_result_e match(const QoreTypeSpec& t, bool& may_not_match) const;

   DLLLOCAL bool runtimeTestMatch(const QoreValue& n, bool& priv_error) const;

   DLLLOCAL bool operator==(const QoreTypeSpec& other) const;
   DLLLOCAL bool operator!=(const QoreTypeSpec& other) const;

private:
   union {
      qore_type_t t;
      const QoreClass* qc;
      const TypedHashDecl* hd;
   } u;
   q_typespec_t typespec;
};

struct QoreReturnSpec {
   const QoreTypeSpec spec;
   bool exact = false;

   DLLLOCAL QoreReturnSpec(const QoreTypeSpec&& spec, bool exact = false) : spec(spec), exact(exact) {
   }
};

typedef std::vector<QoreReturnSpec> q_return_vec_t;

typedef std::function<void (QoreValue&, ExceptionSink*)> q_type_map_t;

static q_type_map_t null_to_nothing = [] (QoreValue& n, ExceptionSink* xsink) { n.assignNothing(); };

struct QoreAcceptSpec {
   const QoreTypeSpec spec;
   const q_type_map_t map;
   bool exact = false;

   DLLLOCAL QoreAcceptSpec(const QoreTypeSpec&& spec, const q_type_map_t&& map, bool exact = false) : spec(spec), map(map), exact(exact) {
   }
};
typedef std::vector<QoreAcceptSpec> q_accept_vec_t;

template <typename T>
DLLLOCAL bool typespec_vec_compare(const T& a, const T& b);

DLLLOCAL bool accept_vec_compare(const q_accept_vec_t& a, const q_accept_vec_t& b);
DLLLOCAL bool return_vec_compare(const q_return_vec_t& a, const q_return_vec_t& b);

class QoreTypeInfo {
protected:
   QoreString tname;

public:
   const q_accept_vec_t accept_vec;
   const q_return_vec_t return_vec;

   DLLLOCAL QoreTypeInfo(const char* name, const q_accept_vec_t&& a_vec, const q_return_vec_t&& r_vec) : tname(name), accept_vec(a_vec), return_vec(r_vec) {
   }

   DLLLOCAL virtual ~QoreTypeInfo() = default;

   // static version of method, checking for null pointer
   DLLLOCAL static qore_type_t getSingleType(const QoreTypeInfo* ti) {
      return ti && hasType(ti) ? ti->getSingleType() : NT_ALL;
   }

   // static version of method, checking for null pointer
   DLLLOCAL static bool parseAcceptsReturns(const QoreTypeInfo* ti, qore_type_t t) {
      return ti && hasType(ti) ? ti->parseAcceptsReturns(t) : true;
   }

   // static version of method, checking for null pointer
   DLLLOCAL static qore_type_result_e parseReturns(const QoreTypeInfo* ti, QoreTypeSpec t) {
      return ti && hasType(ti) ? ti->parseReturns(t) : QTI_AMBIGUOUS;
   }

   // static version of method, checking for null pointer
   // returns true if this type only returns the type given
   DLLLOCAL static bool isType(const QoreTypeInfo* ti, QoreTypeSpec t) {
      return ti ? ti->isType(t) : false;
   }

   // static version of method, checking for null pointer
   DLLLOCAL static qore_type_result_e runtimeAcceptsValue(const QoreTypeInfo* ti, const QoreValue n) {
      return ti && hasType(ti) ? ti->runtimeAcceptsValue(n) : QTI_AMBIGUOUS;
   }

   // static version of method, checking for null pointer
   DLLLOCAL static qore_type_result_e parseAccepts(const QoreTypeInfo* first, const QoreTypeInfo* second) {
      bool may_not_match = true;
      return parseAccepts(first, second, may_not_match);
   }

   // static version of method, checking for null pointer
   DLLLOCAL static qore_type_result_e parseAccepts(const QoreTypeInfo* first, const QoreTypeInfo* second, bool& may_not_match) {
      if (!hasType(first) || !hasType(second))
         return QTI_AMBIGUOUS;
      if (first == second)
         return QTI_IDENT;
      return first->parseAccepts(second, may_not_match);
   }

   // static version of method, checking for null pointer
   DLLLOCAL static bool returnsSingle(const QoreTypeInfo* ti) {
      return ti && hasType(ti) ? ti->return_vec.size() == 1 : false;
   }

   // static version of method, checking for null pointer
   DLLLOCAL static bool acceptsSingle(const QoreTypeInfo* ti) {
      return ti && hasType(ti) ? ti->accept_vec.size() == 1 : false;
   }

   // static version of method, checking for null pointer
   DLLLOCAL static const QoreClass* getUniqueReturnClass(const QoreTypeInfo* ti) {
      if (!ti || ti->return_vec.size() > 1 || !hasType(ti))
         return nullptr;
      return ti->return_vec[0].spec.getClass();
   }

   // static version of method, checking for null pointer
   DLLLOCAL static const TypedHashDecl* getUniqueReturnHashDecl(const QoreTypeInfo* ti) {
      if (!ti || ti->return_vec.size() > 1 || !hasType(ti))
         return nullptr;
      return ti->return_vec[0].spec.getHashDecl();
   }

   // static version of method, checking for null pointer
   DLLLOCAL static bool hasType(const QoreTypeInfo* ti) {
      return !ti || ti->accept_vec[0].spec.getType() == NT_ALL ? false : true;
   }

   // static version of method, checking for null pointer
   DLLLOCAL static bool needsScan(const QoreTypeInfo* ti) {
      return ti && hasType(ti) ? ti->needsScanImpl() : true;
   }

   // static version of method, checking for null pointer
   DLLLOCAL static const char* getName(const QoreTypeInfo* ti) {
      return ti && hasType(ti) ? ti->tname.c_str() : NO_TYPE_INFO;
   }

   // static version of method, checking for null pointer
   DLLLOCAL static void getThisType(const QoreTypeInfo* ti, QoreString& str) {
      if (ti)
         ti->getThisTypeImpl(str);
      else
         str.sprintf("no value");
   }

   // static version of method, checking for null pointer
   DLLLOCAL static void acceptInputParam(const QoreTypeInfo* ti, int param_num, const char* param_name, QoreValue& n, ExceptionSink* xsink) {
      if (ti)
         ti->acceptInputIntern(false, param_num, param_name, n, xsink);
   }

   // static version of method, checking for null pointer
   DLLLOCAL static void acceptInputMember(const QoreTypeInfo* ti, const char* member_name, QoreValue& n, ExceptionSink* xsink) {
      if (ti)
         ti->acceptInputIntern(true, -1, member_name, n, xsink);
   }

   // static version of method, checking for null pointer
   DLLLOCAL static void acceptInputKey(const QoreTypeInfo* ti, const char* member_name, QoreValue& n, ExceptionSink* xsink) {
      if (ti)
         ti->acceptInputIntern(false, -1, member_name, n, xsink);
   }

   // static version of method, checking for null pointer
   DLLLOCAL static void acceptAssignment(const QoreTypeInfo* ti, const char* text, QoreValue& n, ExceptionSink* xsink) {
      assert(text && text[0] == '<');
      if (ti)
         ti->acceptInputIntern(false, -1, text, n, xsink);
   }

   // static version of method, checking for null pointer
   DLLLOCAL static bool hasDefaultValue(const QoreTypeInfo* ti) {
      return ti ? ti->hasDefaultValueImpl() : false;
   }

   // static version of method, checking for null pointer
   DLLLOCAL static QoreValue getDefaultQoreValue(const QoreTypeInfo* ti) {
      return ti ? ti->getDefaultQoreValueImpl() : QoreValue();
   }

   // static version of method, checking for null pointer
   DLLLOCAL static bool mayRequireFilter(const QoreTypeInfo* ti, const QoreValue& n) {
      return ti ? ti->mayRequireFilter(n) : false;
   }

   // static version of method, checking for null pointer
   DLLLOCAL static bool equal(const QoreTypeInfo* a, const QoreTypeInfo* b) {
      bool hta = hasType(a);
      bool htb = hasType(b);
      if (hta && htb)
         return (a == b) ? true : accept_vec_compare(a->accept_vec, b->accept_vec) && return_vec_compare(a->return_vec, b->return_vec);
      return hta || htb ? false : true;
   }

   // static version of method, checking for null pointer
   DLLLOCAL static bool isInputIdentical(const QoreTypeInfo* a, const QoreTypeInfo* b) {
      bool hta = hasType(a);
      bool htb = hasType(b);
      if (hta && htb)
         return (a == b) ? true : accept_vec_compare(a->accept_vec, b->accept_vec);
      return hta || htb ? false : true;
   }

   // static version of method, checking for null pointer
   DLLLOCAL static bool isOutputIdentical(const QoreTypeInfo* a, const QoreTypeInfo* b) {
      bool hta = hasType(a);
      bool htb = hasType(b);
      if (hta && htb)
         return a == b ? true : return_vec_compare(a->return_vec, b->return_vec);
      return hta || htb ? false : true;
   }

   // if second's return type is compatible with first's return type
   // static version of method, checking for null pointer
   DLLLOCAL static bool isOutputCompatible(const QoreTypeInfo* first, const QoreTypeInfo* second) {
      if (hasType(first) && hasType(second))
         return first->isOutputCompatible(second);
      return true;
   }

   // static version of method, checking for null pointer
   DLLLOCAL static bool canConvertToScalar(const QoreTypeInfo* ti) {
      return ti ? ti->canConvertToScalarImpl() : true;
   }

   // static version of method, checking for null pointer
   DLLLOCAL static void checkDoNonNumericWarning(const QoreTypeInfo* ti, const QoreProgramLocation& loc, const char* preface) {
      if (ti && !ti->canConvertToScalarImpl())
         ti->doNonNumericWarning(loc, preface);
   }

   // static version of method, checking for null pointer
   DLLLOCAL static void checkDoNonBooleanWarning(const QoreTypeInfo* ti, const QoreProgramLocation& loc, const char* preface) {
      if (ti && !ti->canConvertToScalarImpl())
         ti->doNonBooleanWarning(loc, preface);
   }

   // static version of method, checking for null pointer
   DLLLOCAL static void checkDoNonStringWarning(const QoreTypeInfo* ti, const QoreProgramLocation& loc, const char* preface) {
      if (ti && !ti->canConvertToScalarImpl())
         ti->doNonStringWarning(loc, preface);
   }

   // static version of method, checking for null pointer
   DLLLOCAL static void concatName(const QoreTypeInfo* ti, std::string& str) {
      if (ti && hasType(ti))
         str.append(ti->tname.c_str());
      else
         str.append(NO_TYPE_INFO);
   }

   DLLLOCAL int doAcceptError(bool priv_error, bool obj, int param_num, const char* param_name, const QoreValue& n, ExceptionSink* xsink) const {
      if (priv_error) {
         if (obj)
            doObjectPrivateClassException(param_name, xsink);
         else
            doPrivateClassException(param_num + 1, param_name, xsink);
      }
      else {
         if (obj)
            doObjectHashDeclTypeException(param_name, n, xsink);
         else
            doTypeException(param_num + 1, param_name, n, xsink);
      }
      return -1;
   }

   DLLLOCAL int doTypeException(int param_num, const char* param_name, const QoreValue& n, ExceptionSink* xsink) const {
      // xsink may be null in case parse exceptions have been disabled in the QoreProgram object
      // for example if there was a "requires" error
      if (!xsink)
         return -1;

      QoreStringNode* desc = new QoreStringNode;
      QoreTypeInfo::ptext(*desc, param_num, param_name);
      desc->concat("expects ");
      getThisTypeImpl(*desc);
      desc->concat(", but got ");
      QoreTypeInfo::getNodeType(*desc, n);
      desc->concat(" instead");
      xsink->raiseException("RUNTIME-TYPE-ERROR", desc);
      return -1;
   }

   DLLLOCAL void acceptInputIntern(bool obj, int param_num, const char* param_name, QoreValue& n, ExceptionSink* xsink) const {
      for (auto& t : accept_vec) {
         bool priv_error = false;
         if (t.spec.runtimeTestMatch(n, priv_error)) {
            assert(!priv_error);
            if (t.map)
               t.map(n, xsink);
            return;
         }
         if (priv_error) {
            doAcceptError(true, obj, param_num, param_name, n, xsink);
            return;
         }
      }
      doAcceptError(false, obj, param_num, param_name, n, xsink);
   }

   DLLLOCAL void doNonNumericWarning(const QoreProgramLocation& loc, const char* preface) const;
   DLLLOCAL void doNonBooleanWarning(const QoreProgramLocation& loc, const char* preface) const;
   DLLLOCAL void doNonStringWarning(const QoreProgramLocation& loc, const char* preface) const;

protected:
   DLLLOCAL int doObjectPrivateClassException(const char* param_name, ExceptionSink* xsink) const {
      assert(xsink);
      QoreStringNode* desc = new QoreStringNode;
      desc->sprintf("member '%s' expects ", param_name);
      getThisTypeImpl(*desc);
      desc->concat(", but got an object where this class is privately inherited instead");
      xsink->raiseException("RUNTIME-TYPE-ERROR", desc);
      return -1;
   }

   DLLLOCAL int doPrivateClassException(int param_num, const char* param_name, ExceptionSink* xsink) const {
      // xsink may be null in case that parse exceptions have been disabled in the QoreProgram object
      // for example if there was a "requires" error
      if (!xsink)
         return -1;

      QoreStringNode* desc = new QoreStringNode;
      QoreTypeInfo::ptext(*desc, param_num, param_name);
      desc->concat("expects ");
      getThisTypeImpl(*desc);
      desc->concat(", but got an object where this class is privately inherited instead");
      xsink->raiseException("RUNTIME-TYPE-ERROR", desc);
      return -1;
   }

   DLLLOCAL int doObjectHashDeclTypeException(const char* param_name, const QoreValue& n, ExceptionSink* xsink) const {
      assert(xsink);
      QoreStringNode* desc = new QoreStringNode;
      desc->sprintf("member '%s' expects ", param_name);
      getThisTypeImpl(*desc);
      desc->concat(", but got ");
      QoreTypeInfo::getNodeType(*desc, n);
      desc->concat(" instead");
      xsink->raiseException("RUNTIME-TYPE-ERROR", desc);
      return -1;
   }

   DLLLOCAL qore_type_t getSingleType() const {
      if (accept_vec.size() == 1 && return_vec.size() == 1) {
         qore_type_t qt = accept_vec[0].spec.getType();
         if (qt == return_vec[0].spec.getType())
            return qt;
      }
      return NT_ALL;
   }

   DLLLOCAL bool parseAcceptsReturns(qore_type_t t) const {
      bool ok = false;
      for (auto& i : accept_vec) {
         if (i.spec.matchType(t) != QTI_NOT_EQUAL) {
            ok = true;
            break;
         }
      }
      if (!ok)
         return false;
      for (auto& i : return_vec) {
         if (i.spec.matchType(t) != t)
            return true;
      }
      return false;
   }

   DLLLOCAL qore_type_result_e parseReturns(QoreTypeSpec t) const {
      if (return_vec.size() == 1)
         return t.match(return_vec[0].spec);
      for (auto& i : return_vec) {
         qore_type_result_e rv = t.match(i.spec);
         if (rv != QTI_NOT_EQUAL)
            return i.exact ? QTI_IDENT : QTI_AMBIGUOUS;
      }
      return QTI_NOT_EQUAL;
   }

   // returns true if this type only returns the type given
   DLLLOCAL bool isType(QoreTypeSpec t) const {
      if (return_vec.size() > 1)
         return false;
      return t.match(return_vec[0].spec) == QTI_IDENT;
   }

   // returns true if the type matches an accept type with a filter (type only checked)
   DLLLOCAL bool mayRequireFilter(const QoreValue& n) const {
      for (auto& at : accept_vec) {
         if (at.map && at.spec.matchType(n.getType()) != QTI_NOT_EQUAL)
            return true;
      }
      return false;
   }

   // if the argument's return type is compatible with "this"'s return type
   DLLLOCAL bool isOutputCompatible(const QoreTypeInfo* typeInfo) const {
      for (auto& rt : typeInfo->return_vec) {
         for (auto& at : accept_vec) {
            if (at.spec.match(rt.spec))
               return true;
         }
      }
      return false;
   }

   DLLLOCAL qore_type_result_e parseAccepts(const QoreTypeInfo* typeInfo, bool& may_not_match) const {
      if (typeInfo->return_vec.size() > accept_vec.size()) {
         may_not_match = true;
      }

      bool ok = false;
      for (auto& rt : typeInfo->return_vec) {
         bool t_no_match = true;
         for (auto& at : accept_vec) {
            qore_type_result_e res = at.spec.match(rt.spec, may_not_match);
            switch (res) {
               case QTI_IDENT:
                  if (at.exact && rt.exact)
                     return QTI_IDENT;
                  // fall down to next case
               case QTI_AMBIGUOUS:
                  if (t_no_match) {
                     t_no_match = false;
                     if (!ok) {
                        ok = true;
                        if (may_not_match)
                           return QTI_AMBIGUOUS;
                     }
                  }
                  // fall down to default
               default:
                  break;
            }
         }
         if (t_no_match) {
            if (!may_not_match) {
               may_not_match = true;
               if (ok)
                  return QTI_AMBIGUOUS;
            }
         }
      }
      if (ok)
         return typeInfo->return_vec.size() != accept_vec.size() ? QTI_AMBIGUOUS : QTI_IDENT;
      may_not_match = false;
      return QTI_NOT_EQUAL;
   }

   DLLLOCAL qore_type_result_e runtimeAcceptsValue(const QoreValue& n) const;

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return true;
   }

   DLLLOCAL virtual void getThisTypeImpl(QoreString& str) const {
      str.sprintf("type '%s'", tname.c_str());
   }

   DLLLOCAL virtual bool hasDefaultValueImpl() const {
      return false;
   }

   DLLLOCAL virtual QoreValue getDefaultQoreValueImpl() const {
      return QoreValue();
   }

   // returns true if there is no type or if the type can be converted to a scalar (numeric, bool, int, string, etc) value, false true if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const = 0;

   DLLLOCAL static void getNodeType(QoreString& str, const QoreValue& n) {
      qore_type_t nt = n.getType();
      if (nt == NT_NOTHING) {
         str.concat("no value");
         return;
      }
      if (nt != NT_OBJECT) {
         str.sprintf("type '%s'", n.getTypeName());
         return;
      }
      str.sprintf("an object of class '%s'", n.get<const QoreObject>()->getClassName());
   }

   DLLLOCAL static void ptext(QoreString& str, int param_num, const char* param_name) {
      if (!param_num && param_name && param_name[0] == '<') {
         str.concat(param_name);
         str.concat(' ');
         return;
      }
      if (param_name && param_name[0] == '<') {
         str.concat(param_name);
         str.concat(' ');
      }
      str.concat("parameter ");
      if (param_num) {
         str.sprintf("%d ", param_num);
         if (param_name && param_name[0] != '<')
            str.sprintf("('%s') ", param_name);
      }
      else
         str.sprintf("'%s' ", param_name);
   }
};

class QoreParseTypeInfo;
typedef std::vector<QoreParseTypeInfo*> parse_type_vec_t;

// this is basically just a wrapper around NamedScope
class QoreParseTypeInfo {
protected:
   bool or_nothing;
   std::string tname;

   DLLLOCAL QoreParseTypeInfo(const NamedScope* n_cscope) : or_nothing(false), cscope(n_cscope->copy()) {
      setName();
   }

   DLLLOCAL void setName() {
      if (or_nothing)
         tname = "*";
      tname += cscope->getIdentifier();
      if (!subtypes.empty()) {
         tname += '<';
         tname += subtypes[0]->getName();
         for (unsigned i = 1; i < subtypes.size(); ++i) {
            tname += ", ";
            tname += subtypes[i]->getName();
         }
         tname += '>';
      }
   }

public:
   NamedScope* cscope; // namespace scope for class
   parse_type_vec_t subtypes;

   DLLLOCAL QoreParseTypeInfo(char* n_cscope, bool n_or_nothing = false) : or_nothing(n_or_nothing), cscope(new NamedScope(n_cscope)) {
      setName();
      assert(strcmp(n_cscope, "any"));

      //printd(5, "QoreParseTypeInfo::QoreParseTypeInfo() %s\n", tname.c_str());
   }

   DLLLOCAL QoreParseTypeInfo(char* n_cscope, bool n_or_nothing, parse_type_vec_t&& subtypes) : or_nothing(n_or_nothing), cscope(new NamedScope(n_cscope)), subtypes(subtypes) {
      setName();

      //printd(5, "QoreParseTypeInfo::QoreParseTypeInfo() %s\n", tname.c_str());
   }

   DLLLOCAL QoreParseTypeInfo(const QoreParseTypeInfo& old) : or_nothing(old.or_nothing), tname(old.tname), cscope(old.cscope ? new NamedScope(*old.cscope) : nullptr) {
      // copy subtypes
      for (const auto& i : old.subtypes)
         subtypes.push_back(new QoreParseTypeInfo(*i));
   }

   DLLLOCAL ~QoreParseTypeInfo() {
      delete cscope;
      for (auto& i : subtypes)
         delete i;
   }

   // static version of method, checking for null pointer
   DLLLOCAL static bool parseStageOneIdenticalWithParsed(const QoreParseTypeInfo* pti, const QoreTypeInfo* typeInfo, bool& recheck) {
      if (pti && typeInfo)
         return pti->parseStageOneIdenticalWithParsed(typeInfo, recheck);
      else if (pti)
         return false;
      else if (typeInfo)
         return false;
      else
         return true;
   }

   // static version of method, checking for null pointer
   DLLLOCAL static bool parseStageOneIdentical(const QoreParseTypeInfo* pti, const QoreParseTypeInfo* typeInfo) {
      if (pti && typeInfo)
         return pti->parseStageOneIdentical(typeInfo);
      else
         return !(pti || typeInfo);
   }

   // static version of method, checking for null pointer
   DLLLOCAL static const QoreTypeInfo* resolveAndDelete(QoreParseTypeInfo* pti, const QoreProgramLocation& loc) {
      return pti ? pti->resolveAndDelete(loc) : nullptr;
   }

   // static version of method, checking for null pointer
   DLLLOCAL static const QoreTypeInfo* resolve(QoreParseTypeInfo* pti, const QoreProgramLocation& loc) {
      return pti ? pti->resolve(loc) : nullptr;
   }

#ifdef DEBUG
   DLLLOCAL const char* getCID() const { return cscope ? cscope->getIdentifier() : "n/a"; }

   // static version of method, checking for null pointer
   DLLLOCAL static const char* getCID(const QoreParseTypeInfo* pti) { return pti ? pti->getCID() : "n/a"; }
#endif

   DLLLOCAL QoreParseTypeInfo* copy() const {
      return new QoreParseTypeInfo(cscope);
   }

   // static version of method, checking for null pointer
   DLLLOCAL static const char* getName(const QoreParseTypeInfo* pti) {
      return pti ? pti->getName() : NO_TYPE_INFO;
   }

   // static version of method, checking for null pointer
   DLLLOCAL static void concatName(const QoreParseTypeInfo* pti, std::string& str) {
      if (pti)
         pti->concatName(str);
      else
         str.append(NO_TYPE_INFO);
   }

private:
   // prototype (expecting type) should be "this"
   // returns true if the prototype does not expect any type or the types are compatible,
   // false if otherwise
   DLLLOCAL bool parseStageOneEqual(const QoreParseTypeInfo* typeInfo) const {
      return !strcmp(cscope->getIdentifier(), typeInfo->cscope->getIdentifier());
   }

   // used when parsing user code to find duplicate signatures
   DLLLOCAL bool parseStageOneIdenticalWithParsed(const QoreTypeInfo* typeInfo, bool& recheck) const {
      if (!typeInfo)
         return false;

      const QoreClass* qc = QoreTypeInfo::getUniqueReturnClass(typeInfo);
      if (!qc)
         return false;

      // both have class info
      if (!strcmp(cscope->getIdentifier(), qc->getName()))
         return recheck = true;
      else
         return false;
   }

   // used when parsing user code to find duplicate signatures
   DLLLOCAL bool parseStageOneIdentical(const QoreParseTypeInfo* typeInfo) const {
      if (!typeInfo)
         return false;

      return !strcmp(cscope->ostr, typeInfo->cscope->ostr);
   }

   DLLLOCAL const QoreTypeInfo* resolve(const QoreProgramLocation& loc) const;
   // resolves the current type to an QoreTypeInfo pointer and deletes itself
   DLLLOCAL const QoreTypeInfo* resolveAndDelete(const QoreProgramLocation& loc);
   DLLLOCAL const QoreTypeInfo* resolveSubtype(const QoreProgramLocation& loc) const;

   DLLLOCAL const char* getName() const {
      return tname.c_str();
   }

   DLLLOCAL void concatName(std::string& str) const {
      concatClass(str, cscope->getIdentifier());
   }

   DLLLOCAL static const QoreTypeInfo* resolveClass(const QoreProgramLocation& loc, const NamedScope& cscope, bool or_nothing);
};

class QoreAnyTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreAnyTypeInfo() : QoreTypeInfo("any", q_accept_vec_t {{NT_ALL, nullptr}}, q_return_vec_t {{NT_ALL}}) {
   }

protected:
   DLLLOCAL virtual void getThisTypeImpl(QoreString& str) const {
      str.concat("no value");
   }

   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }
};

class QoreClassTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreClassTypeInfo(const QoreClass* qc, const char* name) : QoreTypeInfo(name, q_accept_vec_t {{qc, nullptr, true}}, q_return_vec_t {{qc, true}}) {
   }

protected:
   DLLLOCAL QoreClassTypeInfo(const char* name, const q_accept_vec_t&& a_vec, const q_return_vec_t&& r_vec) : QoreTypeInfo(name, std::move(a_vec), std::move(r_vec)) {
   }

   DLLLOCAL virtual void getThisTypeImpl(QoreString& str) const {
      str.sprintf("an object of class '%s'", accept_vec[0].spec.getClass()->getName());
   }

   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return false;
   }
};

class QoreClassOrNothingTypeInfo : public QoreClassTypeInfo {
public:
   DLLLOCAL QoreClassOrNothingTypeInfo(const QoreClass* qc, const char* name) : QoreClassTypeInfo(name,
      q_accept_vec_t {
         {qc, nullptr},
         {NT_NOTHING, nullptr},
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) { n.assignNothing(); }},
      }, q_return_vec_t {{qc}, {NT_NOTHING}}) {
      tname.prepend("*");
   }

protected:
   DLLLOCAL virtual void getThisTypeImpl(QoreString& str) const {
      str.sprintf("an object of class '%s' or no value (NOTHING)", accept_vec[0].spec.getClass()->getName());
   }

   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return false;
   }
};

class QoreHashDeclTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreHashDeclTypeInfo(const TypedHashDecl* hd, const char* name) : QoreTypeInfo(name, q_accept_vec_t {{hd, nullptr, true}}, q_return_vec_t {{hd, true}}) {
   }

protected:
   DLLLOCAL QoreHashDeclTypeInfo(const char* name, const q_accept_vec_t&& a_vec, const q_return_vec_t&& r_vec) : QoreTypeInfo(name, std::move(a_vec), std::move(r_vec)) {
   }

   DLLLOCAL virtual void getThisTypeImpl(QoreString& str) const {
      str.sprintf("hash<%s>", accept_vec[0].spec.getHashDecl()->getName());
   }

   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return false;
   }
};

class QoreHashDeclOrNothingTypeInfo : public QoreHashDeclTypeInfo {
public:
   DLLLOCAL QoreHashDeclOrNothingTypeInfo(const TypedHashDecl* hd, const char* name) : QoreHashDeclTypeInfo(name,
      q_accept_vec_t {
         {hd, nullptr},
         {NT_NOTHING, nullptr},
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) { n.assignNothing(); }},
      }, q_return_vec_t {{hd}, {NT_NOTHING}}) {
      tname.prepend("*");
   }

protected:
   DLLLOCAL virtual void getThisTypeImpl(QoreString& str) const {
      str.sprintf("hash<%s> or no value (NOTHING)", accept_vec[0].spec.getHashDecl()->getName());
   }

   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return false;
   }
};

class QoreBaseTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreBaseTypeInfo(const char* name, qore_type_t t) : QoreTypeInfo(name, q_accept_vec_t {{t, nullptr, true}}, q_return_vec_t {{t, true}}) {
   }

protected:
   DLLLOCAL QoreBaseTypeInfo(const char* name, q_accept_vec_t&& a_vec, q_return_vec_t&& r_vec) : QoreTypeInfo(name, std::move(a_vec), std::move(r_vec)) {
   }
};

class QoreBaseOrNothingTypeInfo : public QoreBaseTypeInfo {
public:
   DLLLOCAL QoreBaseOrNothingTypeInfo(const char* name, qore_type_t t) : QoreBaseTypeInfo(name, q_accept_vec_t {
         {t, nullptr},
         {NT_NOTHING, nullptr},
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) { n.assignNothing(); }},
      },
      q_return_vec_t {{t}, {NT_NOTHING}}) {
   }
};

class QoreBaseConvertTypeInfo : public QoreBaseTypeInfo {
public:
   DLLLOCAL QoreBaseConvertTypeInfo(const char* name, qore_type_t qt) : QoreBaseTypeInfo(name, qt) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }
};

class QoreBaseOrNothingConvertTypeInfo : public QoreBaseOrNothingTypeInfo {
public:
   DLLLOCAL QoreBaseOrNothingConvertTypeInfo(const char* name, qore_type_t qt) : QoreBaseOrNothingTypeInfo(name, qt) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }
};

class QoreBaseNoConvertTypeInfo : public QoreBaseTypeInfo {
public:
   DLLLOCAL QoreBaseNoConvertTypeInfo(const char* name, qore_type_t qt) : QoreBaseTypeInfo(name, qt) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return false;
   }
};

class QoreBaseOrNothingNoConvertTypeInfo : public QoreBaseOrNothingTypeInfo {
public:
   DLLLOCAL QoreBaseOrNothingNoConvertTypeInfo(const char* name, qore_type_t qt) : QoreBaseOrNothingTypeInfo(name, qt) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return false;
   }
};

class QoreBigIntTypeInfo : public QoreBaseConvertTypeInfo {
public:
   DLLLOCAL QoreBigIntTypeInfo() : QoreBaseConvertTypeInfo("int", NT_INT) {
   }

protected:
   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }

   DLLLOCAL virtual bool hasDefaultValueImpl() const {
      return true;
   }

   DLLLOCAL virtual QoreValue getDefaultQoreValueImpl() const {
      return 0LL;
   }
};

class QoreBigIntOrNothingTypeInfo : public QoreBaseOrNothingConvertTypeInfo {
public:
   DLLLOCAL QoreBigIntOrNothingTypeInfo() : QoreBaseOrNothingConvertTypeInfo("*int", NT_INT) {
   }

protected:
   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreStringTypeInfo : public QoreBaseConvertTypeInfo {
public:
   DLLLOCAL QoreStringTypeInfo() : QoreBaseConvertTypeInfo("string", NT_STRING) {
   }

protected:
   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }

   DLLLOCAL virtual bool hasDefaultValueImpl() const {
      return true;
   }

   DLLLOCAL virtual QoreValue getDefaultQoreValueImpl() const {
      return NullString->refSelf();
   }
};

class QoreStringOrNothingTypeInfo : public QoreBaseOrNothingConvertTypeInfo {
public:
   DLLLOCAL QoreStringOrNothingTypeInfo() : QoreBaseOrNothingConvertTypeInfo("*string", NT_STRING) {
   }

protected:
   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreBoolTypeInfo : public QoreBaseConvertTypeInfo {
public:
   DLLLOCAL QoreBoolTypeInfo() : QoreBaseConvertTypeInfo("bool", NT_BOOLEAN) {
   }

protected:
   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }

   DLLLOCAL virtual bool hasDefaultValueImpl() const {
      return true;
   }

   DLLLOCAL virtual QoreValue getDefaultQoreValueImpl() const {
      return false;
   }
};

class QoreBoolOrNothingTypeInfo : public QoreBaseOrNothingConvertTypeInfo {
public:
   DLLLOCAL QoreBoolOrNothingTypeInfo() : QoreBaseOrNothingConvertTypeInfo("*bool", NT_BOOLEAN) {
   }

protected:
   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }

};

class QoreBinaryTypeInfo : public QoreBaseNoConvertTypeInfo {
public:
   DLLLOCAL QoreBinaryTypeInfo() : QoreBaseNoConvertTypeInfo("binary", NT_BINARY) {
   }

protected:
   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }

   DLLLOCAL virtual bool hasDefaultValueImpl() const {
      return true;
   }

   DLLLOCAL virtual QoreValue getDefaultQoreValueImpl() const {
      return new BinaryNode;
   }
};

class QoreBinaryOrNothingTypeInfo : public QoreBaseOrNothingNoConvertTypeInfo {
public:
   DLLLOCAL QoreBinaryOrNothingTypeInfo() : QoreBaseOrNothingNoConvertTypeInfo("*binary", NT_BINARY) {
   }

protected:
   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreObjectTypeInfo : public QoreBaseNoConvertTypeInfo {
public:
   DLLLOCAL QoreObjectTypeInfo() : QoreBaseNoConvertTypeInfo("object", NT_OBJECT) {
   }
};

class QoreObjectOrNothingTypeInfo : public QoreBaseOrNothingNoConvertTypeInfo {
public:
   DLLLOCAL QoreObjectOrNothingTypeInfo() : QoreBaseOrNothingNoConvertTypeInfo("*object", NT_OBJECT) {
   }
};

class QoreDateTypeInfo : public QoreBaseConvertTypeInfo {
public:
   DLLLOCAL QoreDateTypeInfo() : QoreBaseConvertTypeInfo("date", NT_DATE) {
   }

protected:
   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }

   DLLLOCAL virtual bool hasDefaultValueImpl() const {
      return true;
   }

   DLLLOCAL virtual QoreValue getDefaultQoreValueImpl() const {
      return ZeroDate->refSelf();
   }
};

class QoreDateOrNothingTypeInfo : public QoreBaseOrNothingConvertTypeInfo {
public:
   DLLLOCAL QoreDateOrNothingTypeInfo() : QoreBaseOrNothingConvertTypeInfo("*date", NT_DATE) {
   }

protected:
   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreHashTypeInfo : public QoreBaseNoConvertTypeInfo {
public:
   DLLLOCAL QoreHashTypeInfo() : QoreBaseNoConvertTypeInfo("hash", NT_HASH) {
   }

protected:
   DLLLOCAL virtual bool hasDefaultValueImpl() const {
      return true;
   }

   DLLLOCAL virtual QoreValue getDefaultQoreValueImpl() const {
      return emptyHash->hashRefSelf();
   }
};

class QoreHashOrNothingTypeInfo : public QoreBaseOrNothingNoConvertTypeInfo {
public:
   DLLLOCAL QoreHashOrNothingTypeInfo() : QoreBaseOrNothingNoConvertTypeInfo("*hash", NT_HASH) {
   }
};

class QoreListTypeInfo : public QoreBaseNoConvertTypeInfo {
public:
   DLLLOCAL QoreListTypeInfo() : QoreBaseNoConvertTypeInfo("list", NT_LIST) {
   }

protected:
   DLLLOCAL virtual bool hasDefaultValueImpl() const {
      return true;
   }

   DLLLOCAL virtual QoreValue getDefaultQoreValueImpl() const {
      return emptyList->listRefSelf();
   }
};

class QoreListOrNothingTypeInfo : public QoreBaseOrNothingNoConvertTypeInfo {
public:
   DLLLOCAL QoreListOrNothingTypeInfo() : QoreBaseOrNothingNoConvertTypeInfo("*list", NT_LIST) {
   }
};

class QoreNothingTypeInfo : public QoreBaseNoConvertTypeInfo {
public:
   DLLLOCAL QoreNothingTypeInfo() : QoreBaseNoConvertTypeInfo("nothing", NT_NOTHING) {
   }
};

class QoreNullTypeInfo : public QoreBaseNoConvertTypeInfo {
public:
   DLLLOCAL QoreNullTypeInfo() : QoreBaseNoConvertTypeInfo("null", NT_NULL) {
   }

protected:
   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }

   DLLLOCAL virtual bool hasDefaultValueImpl() const {
      return true;
   }

   DLLLOCAL virtual QoreValue getDefaultQoreValueImpl() const {
      return &Null;
   }
};

class QoreNullOrNothingTypeInfo : public QoreBaseOrNothingNoConvertTypeInfo {
public:
   DLLLOCAL QoreNullOrNothingTypeInfo() : QoreBaseOrNothingNoConvertTypeInfo("*null", NT_NULL) {
   }

protected:
   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreClosureTypeInfo : public QoreBaseNoConvertTypeInfo {
public:
   DLLLOCAL QoreClosureTypeInfo() : QoreBaseNoConvertTypeInfo("closure", NT_RUNTIME_CLOSURE) {
   }
};

class QoreClosureOrNothingTypeInfo : public QoreBaseOrNothingNoConvertTypeInfo {
public:
   DLLLOCAL QoreClosureOrNothingTypeInfo() : QoreBaseOrNothingNoConvertTypeInfo("*closure", NT_RUNTIME_CLOSURE) {
   }
};

class QoreCallReferenceTypeInfo : public QoreBaseNoConvertTypeInfo {
public:
   DLLLOCAL QoreCallReferenceTypeInfo() : QoreBaseNoConvertTypeInfo("callref", NT_FUNCREF) {
   }

protected:
   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreCallReferenceOrNothingTypeInfo : public QoreBaseOrNothingNoConvertTypeInfo {
public:
   DLLLOCAL QoreCallReferenceOrNothingTypeInfo() : QoreBaseOrNothingNoConvertTypeInfo("*callref", NT_FUNCREF) {
   }

protected:
   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreReferenceTypeInfo : public QoreBaseNoConvertTypeInfo {
public:
   DLLLOCAL QoreReferenceTypeInfo() : QoreBaseNoConvertTypeInfo("reference", NT_REFERENCE) {
   }

protected:
   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreReferenceOrNothingTypeInfo : public QoreBaseOrNothingNoConvertTypeInfo {
public:
   DLLLOCAL QoreReferenceOrNothingTypeInfo() : QoreBaseOrNothingNoConvertTypeInfo("*reference", NT_REFERENCE) {
   }

protected:
   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreNumberTypeInfo : public QoreBaseTypeInfo {
public:
   DLLLOCAL QoreNumberTypeInfo() : QoreBaseTypeInfo("number", q_accept_vec_t {
         {NT_NUMBER, nullptr, true},
         {NT_FLOAT, [] (QoreValue& n, ExceptionSink* xsink) { discard(n.assign(new QoreNumberNode(n.getAsFloat())), xsink); }}, {NT_INT, [] (QoreValue& n, ExceptionSink* xsink) { discard(n.assign(new QoreNumberNode(n.getAsBigInt())), xsink); }},
      },
      q_return_vec_t {{NT_NUMBER, true}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }

   DLLLOCAL virtual bool hasDefaultValueImpl() const {
      return true;
   }

   DLLLOCAL virtual QoreValue getDefaultQoreValueImpl() const {
      return ZeroNumber->refSelf();
   }
};

class QoreNumberOrNothingTypeInfo : public QoreBaseTypeInfo {
public:
   DLLLOCAL QoreNumberOrNothingTypeInfo() :
      QoreBaseTypeInfo("*number", q_accept_vec_t {
         {NT_NUMBER, nullptr},
         {NT_FLOAT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new QoreNumberNode(n.getAsFloat())), xsink);
            }
         },
         {NT_INT, [] (QoreValue& n, ExceptionSink* xsink) {
              discard(n.assign(new QoreNumberNode(n.getAsBigInt())), xsink);
            }
         },
         {NT_NOTHING, nullptr},
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) {
               n.assignNothing();
            }
         },
      }, q_return_vec_t {{NT_NUMBER}, {NT_NOTHING}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreFloatTypeInfo : public QoreBaseTypeInfo {
public:
   DLLLOCAL QoreFloatTypeInfo() : QoreBaseTypeInfo("float", q_accept_vec_t {
         {NT_FLOAT, nullptr, true},
         {NT_INT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign((double)n.getAsBigInt()), xsink);
            }
         },
      }, q_return_vec_t {{NT_FLOAT, true}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }

   DLLLOCAL virtual bool hasDefaultValueImpl() const {
      return true;
   }

   DLLLOCAL virtual QoreValue getDefaultQoreValueImpl() const {
      return 0.0;
   }
};

class QoreFloatOrNothingTypeInfo : public QoreBaseTypeInfo {
public:
   DLLLOCAL QoreFloatOrNothingTypeInfo() : QoreBaseTypeInfo("*float", q_accept_vec_t {
         {NT_FLOAT, nullptr},
         {NT_INT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign((double)n.getAsBigInt()), xsink);
            }
         },
         {NT_NOTHING, nullptr},
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) {
               n.assignNothing();
            }
         },
      }, q_return_vec_t {{NT_FLOAT}, {NT_NOTHING}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreCodeTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreCodeTypeInfo() : QoreTypeInfo("code", q_accept_vec_t {
         {NT_RUNTIME_CLOSURE, nullptr},
         {NT_FUNCREF, nullptr},
      }, q_return_vec_t {{NT_RUNTIME_CLOSURE}, {NT_FUNCREF}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return false;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return true;
   }
};

class QoreCodeOrNothingTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreCodeOrNothingTypeInfo() : QoreTypeInfo("*code", q_accept_vec_t {
         {NT_RUNTIME_CLOSURE, nullptr},
         {NT_FUNCREF, nullptr},
         {NT_NOTHING, nullptr},
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) { n.assignNothing(); }},
      }, q_return_vec_t {{NT_RUNTIME_CLOSURE}, {NT_FUNCREF}, {NT_NOTHING}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return false;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return true;
   }
};

class QoreDataTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreDataTypeInfo() : QoreTypeInfo("data", q_accept_vec_t {
         {NT_STRING, nullptr},
         {NT_BINARY, nullptr},
      }, q_return_vec_t {{NT_STRING}, {NT_BINARY}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreDataOrNothingTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreDataOrNothingTypeInfo() : QoreTypeInfo("*data", q_accept_vec_t {
         {NT_STRING, nullptr},
         {NT_BINARY, nullptr},
         {NT_NOTHING, nullptr},
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) { n.assignNothing(); }},
      }, q_return_vec_t {{NT_STRING}, {NT_BINARY}, {NT_NOTHING}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreSoftBigIntTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreSoftBigIntTypeInfo() : QoreTypeInfo("softint", q_accept_vec_t {
         {NT_INT, nullptr, true},
         {NT_FLOAT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBigInt()), xsink);
            }
         },
         {NT_STRING, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBigInt()), xsink);
            }
         },
         {NT_DATE, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBigInt()), xsink);
            }
         },
         {NT_BOOLEAN, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBigInt()), xsink);
            }
         },
         {NT_NUMBER, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBigInt()), xsink);
            }
         },
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(0ll), xsink);
            }
         },
      }, q_return_vec_t {{NT_INT, true}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }

   DLLLOCAL virtual bool hasDefaultValueImpl() const {
      return true;
   }

   DLLLOCAL virtual QoreValue getDefaultQoreValueImpl() const {
      return 0ll;
   }
};

class QoreSoftBigIntOrNothingTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreSoftBigIntOrNothingTypeInfo() : QoreTypeInfo("*softint", q_accept_vec_t {
         {NT_INT, nullptr},
         {NT_FLOAT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBigInt()), xsink);
            }
         },
         {NT_STRING, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBigInt()), xsink);
            }
         },
         {NT_DATE, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBigInt()), xsink);
            }
         },
         {NT_BOOLEAN, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBigInt()), xsink);
            }
         },
         {NT_NUMBER, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBigInt()), xsink);
            }
         },
         {NT_NOTHING, nullptr},
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) { n.assignNothing(); }},
      }, q_return_vec_t {{NT_INT}, {NT_NOTHING}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreSoftFloatTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreSoftFloatTypeInfo() : QoreTypeInfo("softfloat", q_accept_vec_t {
         {NT_FLOAT, nullptr, true},
         {NT_INT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsFloat()), xsink);
            }
         },
         {NT_STRING, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsFloat()), xsink);
            }
         },
         {NT_DATE, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsFloat()), xsink);
            }
         },
         {NT_BOOLEAN, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsFloat()), xsink);
            }
         },
         {NT_NUMBER, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsFloat()), xsink);
            }
         },
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(0.0), xsink);
            }
         },
      }, q_return_vec_t {{NT_FLOAT, true}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }

   DLLLOCAL virtual bool hasDefaultValueImpl() const {
      return true;
   }

   DLLLOCAL virtual QoreValue getDefaultQoreValueImpl() const {
      return 0.0;
   }
};

class QoreSoftFloatOrNothingTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreSoftFloatOrNothingTypeInfo() : QoreTypeInfo("*softfloat", q_accept_vec_t {
         {NT_FLOAT, nullptr},
         {NT_INT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsFloat()), xsink);
            }
         },
         {NT_STRING, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsFloat()), xsink);
            }
         },
         {NT_DATE, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsFloat()), xsink);
            }
         },
         {NT_BOOLEAN, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsFloat()), xsink);
            }
         },
         {NT_NUMBER, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsFloat()), xsink);
            }
         },
         {NT_NOTHING, nullptr},
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) { n.assignNothing(); }},
      }, q_return_vec_t {{NT_FLOAT}, {NT_NOTHING}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreSoftNumberTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreSoftNumberTypeInfo() : QoreTypeInfo("softnumber", q_accept_vec_t {
         {NT_NUMBER, nullptr, true},
         {NT_FLOAT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new QoreNumberNode(n.getAsFloat())), xsink);
            }
         },
         {NT_INT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new QoreNumberNode(n.getAsBigInt())), xsink);
            }
         },
         {NT_STRING, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new QoreNumberNode(n.get<const QoreStringNode>()->c_str())), xsink);
            }
         },
         {NT_DATE, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new QoreNumberNode(n.getAsFloat())), xsink);
            }
         },
         {NT_BOOLEAN, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new QoreNumberNode(n.getAsFloat())), xsink);
            }
         },
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new QoreNumberNode(0.0)), xsink);
            }
         },
      }, q_return_vec_t {{NT_NUMBER, true}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }

   DLLLOCAL virtual bool hasDefaultValueImpl() const {
      return true;
   }

   DLLLOCAL virtual QoreValue getDefaultQoreValueImpl() const {
      return ZeroNumber->refSelf();
   }
};

class QoreSoftNumberOrNothingTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreSoftNumberOrNothingTypeInfo() : QoreTypeInfo("*softnumber", q_accept_vec_t {
         {NT_NUMBER, nullptr},
         {NT_FLOAT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new QoreNumberNode(n.getAsFloat())), xsink);
            }
         },
         {NT_INT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new QoreNumberNode(n.getAsBigInt())), xsink);
            }
         },
         {NT_STRING, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new QoreNumberNode(n.get<const QoreStringNode>()->c_str())), xsink);
            }
         },
         {NT_DATE, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new QoreNumberNode(n.getAsFloat())), xsink);
            }
         },
         {NT_BOOLEAN, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new QoreNumberNode(n.getAsFloat())), xsink);
            }
         },
         {NT_NOTHING, nullptr},
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) { n.assignNothing(); }},
      }, q_return_vec_t {{NT_NUMBER}, {NT_NOTHING}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreSoftBoolTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreSoftBoolTypeInfo() : QoreTypeInfo("softbool", q_accept_vec_t {
         {NT_BOOLEAN, nullptr, true},
         {NT_FLOAT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBool()), xsink);
            }
         },
         {NT_INT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBool()), xsink);
            }
         },
         {NT_STRING, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBool()), xsink);
            }
         },
         {NT_DATE, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBool()), xsink);
            }
         },
         {NT_NUMBER, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBool()), xsink);
            }
         },
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) {
               n.assign(false);
            }
         },
      }, q_return_vec_t {{NT_BOOLEAN, true}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }

   DLLLOCAL virtual bool hasDefaultValueImpl() const {
      return true;
   }

   DLLLOCAL virtual QoreValue getDefaultQoreValueImpl() const {
      return false;
   }
};

class QoreSoftBoolOrNothingTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreSoftBoolOrNothingTypeInfo() : QoreTypeInfo("*softbool", q_accept_vec_t {
         {NT_BOOLEAN, nullptr},
         {NT_FLOAT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBool()), xsink);
            }
         },
         {NT_INT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBool()), xsink);
            }
         },
         {NT_STRING, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBool()), xsink);
            }
         },
         {NT_DATE, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBool()), xsink);
            }
         },
         {NT_NUMBER, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(n.getAsBool()), xsink);
            }
         },
         {NT_NOTHING, nullptr},
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) { n.assignNothing(); }},
      }, q_return_vec_t {{NT_BOOLEAN}, {NT_NOTHING}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreSoftStringTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreSoftStringTypeInfo() : QoreTypeInfo("softstring", q_accept_vec_t {
         {NT_STRING, nullptr, true},
         {NT_BOOLEAN, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new QoreStringNodeMaker(QLLD, n.getAsBigInt())), xsink);
            }
         },
         {NT_FLOAT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(q_fix_decimal(new QoreStringNodeMaker("%.9g", n.getAsFloat()))), xsink);
            }
         },
         {NT_INT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new QoreStringNodeMaker(QLLD, n.getAsBigInt())), xsink);
            }
         },
         {NT_DATE, [] (QoreValue& n, ExceptionSink* xsink) {
               QoreStringNodeValueHelper str(n.getInternalNode());
               discard(n.assign(str.getReferencedValue()), xsink);
            }
         },
         {NT_NUMBER, [] (QoreValue& n, ExceptionSink* xsink) {
               QoreStringNodeValueHelper str(n.getInternalNode());
               discard(n.assign(str.getReferencedValue()), xsink);
            }
         },
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) {
               n.assign(NullString->stringRefSelf());
            }
         },
      }, q_return_vec_t {{NT_STRING, true}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }

   DLLLOCAL virtual bool hasDefaultValueImpl() const {
      return true;
   }

   DLLLOCAL virtual QoreValue getDefaultQoreValueImpl() const {
      return NullString->refSelf();
   }
};

class QoreSoftStringOrNothingTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreSoftStringOrNothingTypeInfo() : QoreTypeInfo("*softstring", q_accept_vec_t {
         {NT_STRING, nullptr},
         {NT_BOOLEAN, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new QoreStringNodeMaker(QLLD, n.getAsBigInt())), xsink);
            }
         },
         {NT_FLOAT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(q_fix_decimal(new QoreStringNodeMaker("%.9g", n.getAsFloat()))), xsink);
            }
         },
         {NT_INT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new QoreStringNodeMaker(QLLD, n.getAsBigInt())), xsink);
            }
         },
         {NT_DATE, [] (QoreValue& n, ExceptionSink* xsink) {
               QoreStringNodeValueHelper str(n.getInternalNode());
               discard(n.assign(str.getReferencedValue()), xsink);
            }
         },
         {NT_NUMBER, [] (QoreValue& n, ExceptionSink* xsink) {
               QoreStringNodeValueHelper str(n.getInternalNode());
               discard(n.assign(str.getReferencedValue()), xsink);
            }
         },
         {NT_NOTHING, nullptr},
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) { n.assignNothing(); }},
      }, q_return_vec_t {{NT_STRING}, {NT_NOTHING}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreSoftDateTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreSoftDateTypeInfo() : QoreTypeInfo("softdate", q_accept_vec_t {
         {NT_DATE, nullptr, true},
         {NT_STRING, [] (QoreValue& n, ExceptionSink* xsink) {
               DateTimeNodeValueHelper dt(n.getInternalNode());
               discard(n.assign(dt.getReferencedValue()), xsink);
            }
         },
         {NT_BOOLEAN, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new DateTimeNode(n.getAsBigInt())), xsink);
            }
         },
         {NT_FLOAT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new DateTimeNode(n.getAsBigInt())), xsink);
            }
         },
         {NT_INT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new DateTimeNode(n.getAsBigInt())), xsink);
            }
         },
         {NT_NUMBER, [] (QoreValue& n, ExceptionSink* xsink) {
               DateTimeNodeValueHelper dt(n.getInternalNode());
               discard(n.assign(dt.getReferencedValue()), xsink);
            }
         },
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) {
               n.assign(new DateTimeNode(0ll));
            }
         },
      }, q_return_vec_t {{NT_DATE, true}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }

   DLLLOCAL virtual bool hasDefaultValueImpl() const {
      return true;
   }

   DLLLOCAL virtual QoreValue getDefaultQoreValueImpl() const {
      return ZeroDate->refSelf();
   }
};

class QoreSoftDateOrNothingTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreSoftDateOrNothingTypeInfo() : QoreTypeInfo("*softdate", q_accept_vec_t {
         {NT_DATE, nullptr},
         {NT_STRING, [] (QoreValue& n, ExceptionSink* xsink) {
               DateTimeNodeValueHelper dt(n.getInternalNode());
               discard(n.assign(dt.getReferencedValue()), xsink);
            }
         },
         {NT_BOOLEAN, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new DateTimeNode(n.getAsBigInt())), xsink);
            }
         },
         {NT_FLOAT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new DateTimeNode(n.getAsBigInt())), xsink);
            }
         },
         {NT_INT, [] (QoreValue& n, ExceptionSink* xsink) {
               discard(n.assign(new DateTimeNode(n.getAsBigInt())), xsink);
            }
         },
         {NT_NUMBER, [] (QoreValue& n, ExceptionSink* xsink) {
               DateTimeNodeValueHelper dt(n.getInternalNode());
               discard(n.assign(dt.getReferencedValue()), xsink);
            }
         },
         {NT_NOTHING, nullptr},
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) { n.assignNothing(); }},
      }, q_return_vec_t {{NT_DATE}, {NT_NOTHING}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreSoftListTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreSoftListTypeInfo() : QoreTypeInfo("softlist", q_accept_vec_t {
         {NT_LIST, nullptr, true},
         {NT_NOTHING, [] (QoreValue& n, ExceptionSink* xsink) {
               QoreListNode* l = new QoreListNode;
               n.assign(l);
            }
         },
         {NT_ALL, [] (QoreValue& n, ExceptionSink* xsink) {
               QoreListNode* l = new QoreListNode;
               l->push(n.takeNode());
               n.assign(l);
            }
         },
      }, q_return_vec_t {{NT_LIST, true}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return false;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return true;
   }

   DLLLOCAL virtual bool hasDefaultValueImpl() const {
      return true;
   }

   DLLLOCAL virtual QoreValue getDefaultQoreValueImpl() const {
      return emptyList->listRefSelf();
   }
};

class QoreSoftListOrNothingTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreSoftListOrNothingTypeInfo() : QoreTypeInfo("*softlist", q_accept_vec_t {
         {NT_LIST, nullptr},
         {NT_NOTHING, nullptr},
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) { n.assignNothing(); }},
         {NT_ALL, [] (QoreValue& n, ExceptionSink* xsink) {
               QoreListNode* l = new QoreListNode;
               l->push(n.takeNode());
               n.assign(l);
            }
         },
      }, q_return_vec_t {{NT_LIST}, {NT_NOTHING}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return false;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return true;
   }
};

class QoreTimeoutTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreTimeoutTypeInfo() : QoreTypeInfo("timeout", q_accept_vec_t {
         {NT_INT, nullptr},
         {NT_DATE, [] (QoreValue& n, ExceptionSink* xsink) {
               int64 ms = n.get<const DateTimeNode>()->getRelativeMilliseconds();
               discard(n.assign(ms), xsink);
            }
         },
      }, q_return_vec_t {{NT_INT, true}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }

   DLLLOCAL virtual bool hasDefaultValueImpl() const {
      return true;
   }

   DLLLOCAL virtual QoreValue getDefaultQoreValueImpl() const {
      return 0ll;
   }
};

class QoreTimeoutOrNothingTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreTimeoutOrNothingTypeInfo() : QoreTypeInfo("*timeout", q_accept_vec_t {
         {NT_INT, nullptr},
         {NT_DATE, [] (QoreValue& n, ExceptionSink* xsink) {
               int64 ms = n.get<const DateTimeNode>()->getRelativeMilliseconds();
               discard(n.assign(ms), xsink);
            }
         },
         {NT_NOTHING, nullptr},
         {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) {
               n.assignNothing();
            }
         },
      }, q_return_vec_t {{NT_INT}, {NT_NOTHING}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreIntOrFloatTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreIntOrFloatTypeInfo() : QoreTypeInfo("int|float", q_accept_vec_t {
         {NT_INT, nullptr},
         {NT_FLOAT, nullptr},
      }, q_return_vec_t {{NT_INT}, {NT_FLOAT}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreIntFloatOrNumberTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreIntFloatOrNumberTypeInfo() : QoreTypeInfo("int|float|number", q_accept_vec_t {
         {NT_INT, nullptr},
         {NT_FLOAT, nullptr},
         {NT_NUMBER, nullptr},
      }, q_return_vec_t {{NT_INT}, {NT_FLOAT}, {NT_NUMBER}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

class QoreFloatOrNumberTypeInfo : public QoreTypeInfo {
public:
   DLLLOCAL QoreFloatOrNumberTypeInfo() : QoreTypeInfo("float|number", q_accept_vec_t {
         {NT_FLOAT, nullptr},
         {NT_NUMBER, nullptr},
      }, q_return_vec_t {{NT_FLOAT}, {NT_NUMBER}}) {
   }

protected:
   // returns true if there is no type or if the type can be converted to a scalar value, false if otherwise
   DLLLOCAL virtual bool canConvertToScalarImpl() const {
      return true;
   }

   // returns true if this type could contain an object or a closure
   DLLLOCAL virtual bool needsScanImpl() const {
      return false;
   }
};

// returns type info for base types
DLLLOCAL const QoreTypeInfo* getTypeInfoForType(qore_type_t t);
// returns type info information for parse types (values)
DLLLOCAL const QoreTypeInfo* getTypeInfoForValue(const AbstractQoreNode* n);

#endif // _QORE_QORETYPEINFO_H
