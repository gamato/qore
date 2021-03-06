/*
  QoreTypeInfo.cpp

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

#include <qore/Qore.h>
#include <qore/QoreRWLock.h>
#include "qore/intern/qore_program_private.h"
#include "qore/intern/QoreNamespaceIntern.h"
#include "qore/intern/qore_number_private.h"
#include "qore/intern/qore_program_private.h"
#include "qore/intern/QoreClassIntern.h"
#include "qore/intern/typed_hash_decl_private.h"
#include "qore/intern/qore_list_private.h"
#include "qore/intern/QoreHashNodeIntern.h"

const QoreAnyTypeInfo staticAnyTypeInfo;
const QoreAutoTypeInfo staticAutoTypeInfo;

const QoreBigIntTypeInfo staticBigIntTypeInfo;
const QoreBigIntOrNothingTypeInfo staticBigIntOrNothingTypeInfo;

const QoreStringTypeInfo staticStringTypeInfo;
const QoreStringOrNothingTypeInfo staticStringOrNothingTypeInfo;

const QoreBoolTypeInfo staticBoolTypeInfo;
const QoreBoolOrNothingTypeInfo staticBoolOrNothingTypeInfo;

const QoreBinaryTypeInfo staticBinaryTypeInfo;
const QoreBinaryOrNothingTypeInfo staticBinaryOrNothingTypeInfo;

const QoreObjectTypeInfo staticObjectTypeInfo;
const QoreObjectOrNothingTypeInfo staticObjectOrNothingTypeInfo;

const QoreDateTypeInfo staticDateTypeInfo;
const QoreDateOrNothingTypeInfo staticDateOrNothingTypeInfo;

const QoreHashTypeInfo staticHashTypeInfo;
const QoreHashOrNothingTypeInfo staticHashOrNothingTypeInfo;

const QoreAutoHashTypeInfo staticAutoHashTypeInfo;
const QoreAutoHashOrNothingTypeInfo staticAutoHashOrNothingTypeInfo;

const QoreListTypeInfo staticListTypeInfo;
const QoreListOrNothingTypeInfo staticListOrNothingTypeInfo;

const QoreAutoListTypeInfo staticAutoListTypeInfo;
const QoreAutoListOrNothingTypeInfo staticAutoListOrNothingTypeInfo;

const QoreNothingTypeInfo staticNothingTypeInfo;

const QoreNullTypeInfo staticNullTypeInfo;
const QoreNullOrNothingTypeInfo staticNullOrNothingTypeInfo;

const QoreClosureTypeInfo staticClosureTypeInfo;
const QoreClosureOrNothingTypeInfo staticClosureOrNothingTypeInfo;

const QoreCallReferenceTypeInfo staticCallReferenceTypeInfo;
const QoreCallReferenceOrNothingTypeInfo staticCallReferenceOrNothingTypeInfo;

const QoreReferenceTypeInfo staticReferenceTypeInfo;
const QoreReferenceOrNothingTypeInfo staticReferenceOrNothingTypeInfo;

const QoreNumberTypeInfo staticNumberTypeInfo;
const QoreNumberOrNothingTypeInfo staticNumberOrNothingTypeInfo;

const QoreFloatTypeInfo staticFloatTypeInfo;
const QoreFloatOrNothingTypeInfo staticFloatOrNothingTypeInfo;

const QoreCodeTypeInfo staticCodeTypeInfo;
const QoreCodeOrNothingTypeInfo staticCodeOrNothingTypeInfo;

const QoreDataTypeInfo staticDataTypeInfo;
const QoreDataOrNothingTypeInfo staticDataOrNothingTypeInfo;

const QoreSoftBigIntTypeInfo staticSoftBigIntTypeInfo;
const QoreSoftBigIntOrNothingTypeInfo staticSoftBigIntOrNothingTypeInfo;

const QoreSoftFloatTypeInfo staticSoftFloatTypeInfo;
const QoreSoftFloatOrNothingTypeInfo staticSoftFloatOrNothingTypeInfo;

const QoreSoftNumberTypeInfo staticSoftNumberTypeInfo;
const QoreSoftNumberOrNothingTypeInfo staticSoftNumberOrNothingTypeInfo;

const QoreSoftBoolTypeInfo staticSoftBoolTypeInfo;
const QoreSoftBoolOrNothingTypeInfo staticSoftBoolOrNothingTypeInfo;

const QoreSoftStringTypeInfo staticSoftStringTypeInfo;
const QoreSoftStringOrNothingTypeInfo staticSoftStringOrNothingTypeInfo;

const QoreSoftDateTypeInfo staticSoftDateTypeInfo;
const QoreSoftDateOrNothingTypeInfo staticSoftDateOrNothingTypeInfo;

const QoreSoftListTypeInfo staticSoftListTypeInfo;
const QoreSoftListOrNothingTypeInfo staticSoftListOrNothingTypeInfo;

const QoreSoftAutoListTypeInfo staticSoftAutoListTypeInfo;
const QoreSoftAutoListOrNothingTypeInfo staticSoftAutoListOrNothingTypeInfo;

const QoreTimeoutTypeInfo staticTimeoutTypeInfo;
const QoreTimeoutOrNothingTypeInfo staticTimeoutOrNothingTypeInfo;

const QoreIntOrFloatTypeInfo staticIntOrFloatTypeInfo;

const QoreIntFloatOrNumberTypeInfo staticIntFloatOrNumberTypeInfo;

const QoreFloatOrNumberTypeInfo staticFloatOrNumberTypeInfo;

const QoreTypeInfo* anyTypeInfo = &staticAnyTypeInfo,
   *autoTypeInfo = &staticAutoTypeInfo,
   *bigIntTypeInfo = &staticBigIntTypeInfo,
   *floatTypeInfo = &staticFloatTypeInfo,
   *boolTypeInfo = &staticBoolTypeInfo,
   *stringTypeInfo = &staticStringTypeInfo,
   *binaryTypeInfo = &staticBinaryTypeInfo,
   *dateTypeInfo = &staticDateTypeInfo,
   *objectTypeInfo = &staticObjectTypeInfo,
   *hashTypeInfo = &staticHashTypeInfo,
   *autoHashTypeInfo = &staticAutoHashTypeInfo,
   *listTypeInfo = &staticListTypeInfo,
   *autoListTypeInfo = &staticAutoListTypeInfo,
   *nothingTypeInfo = &staticNothingTypeInfo,
   *nullTypeInfo = &staticNullTypeInfo,
   *numberTypeInfo = &staticNumberTypeInfo,
   *runTimeClosureTypeInfo = &staticClosureTypeInfo,
   *callReferenceTypeInfo = &staticCallReferenceTypeInfo,
   *referenceTypeInfo = &staticReferenceTypeInfo,
   *codeTypeInfo = &staticCodeTypeInfo,
   *softBigIntTypeInfo = &staticSoftBigIntTypeInfo,
   *softFloatTypeInfo = &staticSoftFloatTypeInfo,
   *softNumberTypeInfo = &staticSoftNumberTypeInfo,
   *softBoolTypeInfo = &staticSoftBoolTypeInfo,
   *softStringTypeInfo = &staticSoftStringTypeInfo,
   *softDateTypeInfo = &staticSoftDateTypeInfo,
   *softListTypeInfo = &staticSoftListTypeInfo,
   *softAutoListTypeInfo = &staticSoftAutoListTypeInfo,
   *dataTypeInfo = &staticDataTypeInfo,
   *timeoutTypeInfo = &staticTimeoutTypeInfo,
   *bigIntOrFloatTypeInfo = &staticIntOrFloatTypeInfo,
   *bigIntFloatOrNumberTypeInfo = &staticIntFloatOrNumberTypeInfo,
   *floatOrNumberTypeInfo = &staticFloatOrNumberTypeInfo,

   *bigIntOrNothingTypeInfo = &staticBigIntOrNothingTypeInfo,
   *floatOrNothingTypeInfo = &staticFloatOrNothingTypeInfo,
   *numberOrNothingTypeInfo = &staticNumberOrNothingTypeInfo,
   *stringOrNothingTypeInfo = &staticStringOrNothingTypeInfo,
   *boolOrNothingTypeInfo = &staticBoolOrNothingTypeInfo,
   *binaryOrNothingTypeInfo = &staticBinaryOrNothingTypeInfo,
   *objectOrNothingTypeInfo = &staticObjectOrNothingTypeInfo,
   *dateOrNothingTypeInfo = &staticDateOrNothingTypeInfo,
   *hashOrNothingTypeInfo = &staticHashOrNothingTypeInfo,
   *autoHashOrNothingTypeInfo = &staticAutoHashOrNothingTypeInfo,
   *listOrNothingTypeInfo = &staticListOrNothingTypeInfo,
   *autoListOrNothingTypeInfo = &staticAutoListOrNothingTypeInfo,
   *nullOrNothingTypeInfo = &staticNullOrNothingTypeInfo,
   *codeOrNothingTypeInfo = &staticCodeOrNothingTypeInfo,
   *dataOrNothingTypeInfo = &staticDataOrNothingTypeInfo,
   *referenceOrNothingTypeInfo = &staticReferenceOrNothingTypeInfo,

   *softBigIntOrNothingTypeInfo = &staticSoftBigIntOrNothingTypeInfo,
   *softFloatOrNothingTypeInfo = &staticSoftFloatOrNothingTypeInfo,
   *softNumberOrNothingTypeInfo = &staticSoftNumberOrNothingTypeInfo,
   *softBoolOrNothingTypeInfo = &staticSoftBoolOrNothingTypeInfo,
   *softStringOrNothingTypeInfo = &staticSoftStringOrNothingTypeInfo,
   *softDateOrNothingTypeInfo = &staticSoftDateOrNothingTypeInfo,
   *softListOrNothingTypeInfo = &staticSoftListOrNothingTypeInfo,
   *softAutoListOrNothingTypeInfo = &staticSoftAutoListOrNothingTypeInfo,
   *timeoutOrNothingTypeInfo = &staticTimeoutOrNothingTypeInfo;

QoreListNode* emptyList;
QoreHashNode* emptyHash;
QoreStringNode* NullString;
DateTimeNode* ZeroDate, * OneDate;
QoreBigIntNode* Zero;
QoreFloatNode* ZeroFloat;
QoreNumberNode* ZeroNumber, * NaNumber, * InfinityNumber, * piNumber;

// map from types to default values
typedef std::map<qore_type_t, AbstractQoreNode* > def_val_map_t;
static def_val_map_t def_val_map;

// map from names used when parsing to types
typedef std::map<const char* , const QoreTypeInfo* , ltstr> str_typeinfo_map_t;
static str_typeinfo_map_t str_typeinfo_map;
static str_typeinfo_map_t str_ornothingtypeinfo_map;

// map from types to type info
typedef std::map<qore_type_t, const QoreTypeInfo* > type_typeinfo_map_t;
static type_typeinfo_map_t type_typeinfo_map;
static type_typeinfo_map_t type_ornothingtypeinfo_map;

// global external type map
static type_typeinfo_map_t extern_type_info_map;

// map from types to names
typedef std::map<qore_type_t, const char* > type_str_map_t;
static type_str_map_t type_str_map;

// map from simple types to "or nothing" types
typedef std::map<const QoreTypeInfo*, const QoreTypeInfo*> typeinfo_map_t;
static typeinfo_map_t typeinfo_map;

static QoreThreadLock ctl; // complex type lock

typedef std::map<const QoreTypeInfo*, QoreTypeInfo*> tmap_t;
tmap_t ch_map,          // complex hash map
   chon_map,            // complex hash or nothing map
   cl_map,              // complex list map
   clon_map,            // complex list or nothing map
   cr_map,              // complex reference map
   cron_map,            // complex reference or nothing map
   csl_map,             // complex softlist map
   cslon_map;           // complex softlist or nothing map

// rwlock for global type map
static QoreRWLock extern_type_info_map_lock;

static void do_maps(qore_type_t t, const char* name, const QoreTypeInfo* typeInfo, const QoreTypeInfo* orNothingTypeInfo) {
   str_typeinfo_map[name]          = typeInfo;
   str_ornothingtypeinfo_map[name] = orNothingTypeInfo;
   type_typeinfo_map[t]            = typeInfo;
   type_ornothingtypeinfo_map[t]   = orNothingTypeInfo;
   type_str_map[t]                 = name;
   typeinfo_map[typeInfo]          = orNothingTypeInfo;
}

// at least the NullString must be created after the default character encoding is set
void init_qore_types() {
   // initialize global default values
   NullString     = new QoreStringNode;
   ZeroDate       = DateTimeNode::makeAbsolute(0, 0, 0);
   OneDate        = DateTimeNode::makeAbsolute(0, 0, 0, 0, 0, 1);
   Zero           = new QoreBigIntNode;
   ZeroFloat      = new QoreFloatNode;
   ZeroNumber     = new QoreNumberNode;
   NaNumber       = qore_number_private::getNaNumber();
   InfinityNumber = qore_number_private::getInfinity();
   piNumber       = qore_number_private::getPi();

   emptyList      = new QoreListNode;
   emptyHash      = new QoreHashNode;

   def_val_map[NT_INT]     = Zero->refSelf();
   def_val_map[NT_STRING]  = NullString->refSelf();
   def_val_map[NT_BOOLEAN] = &False;
   def_val_map[NT_DATE]    = ZeroDate->refSelf();
   def_val_map[NT_FLOAT]   = ZeroFloat->refSelf();
   def_val_map[NT_NUMBER]  = ZeroNumber->refSelf();
   def_val_map[NT_LIST]    = emptyList->refSelf();
   def_val_map[NT_HASH]    = emptyHash->refSelf();
   def_val_map[NT_BINARY]  = new BinaryNode;
   def_val_map[NT_NULL]    = &Null;
   def_val_map[NT_NOTHING] = &Nothing;

   do_maps(NT_INT,         "int", bigIntTypeInfo, bigIntOrNothingTypeInfo);
   do_maps(NT_STRING,      "string", stringTypeInfo, stringOrNothingTypeInfo);
   do_maps(NT_BOOLEAN,     "bool", boolTypeInfo, boolOrNothingTypeInfo);
   do_maps(NT_FLOAT,       "float", floatTypeInfo, floatOrNothingTypeInfo);
   do_maps(NT_NUMBER,      "number", numberTypeInfo, numberOrNothingTypeInfo);
   do_maps(NT_BINARY,      "binary", binaryTypeInfo, binaryOrNothingTypeInfo);
   do_maps(NT_LIST,        "list", listTypeInfo, listOrNothingTypeInfo);
   do_maps(NT_HASH,        "hash", hashTypeInfo, hashOrNothingTypeInfo);
   do_maps(NT_OBJECT,      "object", objectTypeInfo, objectOrNothingTypeInfo);
   do_maps(NT_ALL,         "any", anyTypeInfo, anyTypeInfo);
   do_maps(NT_ALL,         "auto", autoTypeInfo, autoTypeInfo);
   do_maps(NT_DATE,        "date", dateTypeInfo, dateOrNothingTypeInfo);
   do_maps(NT_CODE,        "code", codeTypeInfo, codeOrNothingTypeInfo);
   do_maps(NT_DATA,        "data", dataTypeInfo, dataOrNothingTypeInfo);
   do_maps(NT_REFERENCE,   "reference", referenceTypeInfo, referenceOrNothingTypeInfo);
   do_maps(NT_NULL,        "null", nullTypeInfo, nullOrNothingTypeInfo);
   do_maps(NT_NOTHING,     "nothing", nothingTypeInfo, nothingTypeInfo);

   do_maps(NT_SOFTINT,     "softint", softBigIntTypeInfo, softBigIntOrNothingTypeInfo);
   do_maps(NT_SOFTFLOAT,   "softfloat", softFloatTypeInfo, softFloatOrNothingTypeInfo);
   do_maps(NT_SOFTNUMBER,  "softnumber", softNumberTypeInfo, softNumberOrNothingTypeInfo);
   do_maps(NT_SOFTBOOLEAN, "softbool", softBoolTypeInfo, softBoolOrNothingTypeInfo);
   do_maps(NT_SOFTSTRING,  "softstring", softStringTypeInfo, softStringOrNothingTypeInfo);
   do_maps(NT_SOFTDATE,    "softdate", softDateTypeInfo, softDateOrNothingTypeInfo);
   do_maps(NT_SOFTLIST,    "softlist", softListTypeInfo, softListOrNothingTypeInfo);

   do_maps(NT_TIMEOUT,     "timeout", timeoutTypeInfo, timeoutOrNothingTypeInfo);

   // map the closure and callref strings to codeTypeInfo to ensure that these
   // types are always interchangeable
   do_maps(NT_RUNTIME_CLOSURE, "closure", codeTypeInfo, codeOrNothingTypeInfo);
   do_maps(NT_FUNCREF, "callref", codeTypeInfo, codeOrNothingTypeInfo);
}

void delete_qore_types() {
   // dereference all values from default value map
   for (def_val_map_t::iterator i = def_val_map.begin(), e = def_val_map.end(); i != e; ++i)
      i->second->deref(nullptr);

   // dereference global default values
   NullString->deref();
   piNumber->deref();
   InfinityNumber->deref();
   NaNumber->deref();
   ZeroNumber->deref();
   ZeroFloat->deref();
   Zero->deref();
   OneDate->deref();
   ZeroDate->deref();
   emptyList->deref(nullptr);
   emptyHash->deref(nullptr);

   // delete stored type information
   for (auto& i : ch_map)
      delete i.second;
   for (auto& i : chon_map)
      delete i.second;
   for (auto& i : cl_map)
      delete i.second;
   for (auto& i : clon_map)
      delete i.second;
   for (auto& i : cr_map)
      delete i.second;
   for (auto& i : cron_map)
      delete i.second;
   for (auto& i : csl_map)
      delete i.second;
   for (auto& i : cslon_map)
      delete i.second;
}

void add_to_type_map(qore_type_t t, const QoreTypeInfo* typeInfo) {
   QoreAutoRWWriteLocker al(extern_type_info_map_lock);
   assert(extern_type_info_map.find(t) == extern_type_info_map.end());
   extern_type_info_map[t] = typeInfo;
}

const QoreTypeInfo* get_or_nothing_type_check(const QoreTypeInfo* typeInfo) {
   return QoreTypeInfo::parseAcceptsReturns(typeInfo, NT_NOTHING) ? typeInfo : get_or_nothing_type(typeInfo);
}

const QoreTypeInfo* get_or_nothing_type(const QoreTypeInfo* typeInfo) {
   assert(!QoreTypeInfo::parseAcceptsReturns(typeInfo, NT_NOTHING));

   typeinfo_map_t::iterator i = typeinfo_map.find(typeInfo);
   if (i != typeinfo_map.end())
      return i->second;

   // see if we have a complex type
   {
      const TypedHashDecl* hd = QoreTypeInfo::getUniqueReturnHashDecl(typeInfo);
      if (hd)
         return hd->getTypeInfo(true);
   }

   {
      const QoreTypeInfo* ti = QoreTypeInfo::getUniqueReturnComplexHash(typeInfo);
      if (ti)
         return qore_program_private::get(*getProgram())->getComplexHashOrNothingType(ti);
   }

   {
      const QoreTypeInfo* ti = QoreTypeInfo::getUniqueReturnComplexSoftList(typeInfo);
      if (ti)
         return qore_program_private::get(*getProgram())->getComplexSoftListOrNothingType(ti);
   }

   {
      const QoreTypeInfo* ti = QoreTypeInfo::getUniqueReturnComplexList(typeInfo);
      if (ti)
         return qore_program_private::get(*getProgram())->getComplexListOrNothingType(ti);
   }

   {
      const QoreTypeInfo* ti = QoreTypeInfo::getUniqueReturnComplexReference(typeInfo);
      if (ti)
         return qore_program_private::get(*getProgram())->getComplexReferenceOrNothingType(ti);
   }

   return nullptr;
}

const QoreTypeInfo* qore_get_complex_hash_type(const QoreTypeInfo* vti) {
   AutoLocker al(ctl);

   tmap_t::iterator i = ch_map.lower_bound(vti);
   if (i != ch_map.end() && i->first == vti)
      return i->second;

   QoreComplexHashTypeInfo* ti = new QoreComplexHashTypeInfo(vti);
   ch_map.insert(i, tmap_t::value_type(vti, ti));
   return ti;
}

const QoreTypeInfo* qore_get_complex_hash_or_nothing_type(const QoreTypeInfo* vti) {
   AutoLocker al(ctl);

   tmap_t::iterator i = chon_map.lower_bound(vti);
   if (i != chon_map.end() && i->first == vti)
      return i->second;

   QoreComplexHashOrNothingTypeInfo* ti = new QoreComplexHashOrNothingTypeInfo(vti);
   chon_map.insert(i, tmap_t::value_type(vti, ti));
   return ti;
}

const QoreTypeInfo* qore_get_complex_list_type(const QoreTypeInfo* vti) {
   AutoLocker al(ctl);

   tmap_t::iterator i = cl_map.lower_bound(vti);
   if (i != cl_map.end() && i->first == vti)
      return i->second;

   QoreComplexListTypeInfo* ti = new QoreComplexListTypeInfo(vti);
   cl_map.insert(i, tmap_t::value_type(vti, ti));
   return ti;
}

const QoreTypeInfo* qore_get_complex_list_or_nothing_type(const QoreTypeInfo* vti) {
   AutoLocker al(ctl);

   tmap_t::iterator i = clon_map.lower_bound(vti);
   if (i != clon_map.end() && i->first == vti)
      return i->second;

   QoreComplexListOrNothingTypeInfo* ti = new QoreComplexListOrNothingTypeInfo(vti);
   clon_map.insert(i, tmap_t::value_type(vti, ti));
   return ti;
}

const QoreTypeInfo* qore_get_complex_softlist_type(const QoreTypeInfo* vti) {
   AutoLocker al(ctl);

   tmap_t::iterator i = csl_map.lower_bound(vti);
   if (i != csl_map.end() && i->first == vti)
      return i->second;

   QoreComplexSoftListTypeInfo* ti = new QoreComplexSoftListTypeInfo(vti);
   csl_map.insert(i, tmap_t::value_type(vti, ti));
   return ti;
}

const QoreTypeInfo* qore_get_complex_softlist_or_nothing_type(const QoreTypeInfo* vti) {
   AutoLocker al(ctl);

   tmap_t::iterator i = cslon_map.lower_bound(vti);
   if (i != cslon_map.end() && i->first == vti)
      return i->second;

   QoreComplexSoftListOrNothingTypeInfo* ti = new QoreComplexSoftListOrNothingTypeInfo(vti);
   cslon_map.insert(i, tmap_t::value_type(vti, ti));
   return ti;
}

const QoreTypeInfo* qore_get_complex_reference_type(const QoreTypeInfo* vti) {
   AutoLocker al(ctl);

   tmap_t::iterator i = cr_map.lower_bound(vti);
   if (i != cr_map.end() && i->first == vti)
      return i->second;

   QoreComplexReferenceTypeInfo* ti = new QoreComplexReferenceTypeInfo(vti);
   cr_map.insert(i, tmap_t::value_type(vti, ti));
   return ti;
}

const QoreTypeInfo* qore_get_complex_reference_or_nothing_type(const QoreTypeInfo* vti) {
   AutoLocker al(ctl);

   tmap_t::iterator i = cron_map.lower_bound(vti);
   if (i != cron_map.end() && i->first == vti)
      return i->second;

   QoreComplexReferenceOrNothingTypeInfo* ti = new QoreComplexReferenceOrNothingTypeInfo(vti);
   cron_map.insert(i, tmap_t::value_type(vti, ti));
   return ti;
}

static const QoreTypeInfo* getExternalTypeInfoForType(qore_type_t t) {
   QoreAutoRWReadLocker al(extern_type_info_map_lock);
   type_typeinfo_map_t::iterator i = extern_type_info_map.find(t);
   return (i == extern_type_info_map.end() ? nullptr : i->second);
}

const QoreTypeInfo* getTypeInfoForType(qore_type_t t) {
   type_typeinfo_map_t::iterator i = type_typeinfo_map.find(t);
   return i != type_typeinfo_map.end() ? i->second : getExternalTypeInfoForType(t);
}

const QoreTypeInfo* getTypeInfoForValue(const AbstractQoreNode* n) {
   qore_type_t t = get_node_type(n);
   switch (t) {
      case NT_OBJECT:
         return static_cast<const QoreObject*>(n)->getClass()->getTypeInfo();
      case NT_HASH:
         return static_cast<const QoreHashNode*>(n)->getTypeInfo();
      case NT_LIST:
         return static_cast<const QoreListNode*>(n)->getTypeInfo();
      case NT_REFERENCE:
         return static_cast<const ReferenceNode*>(n)->getTypeInfo();
      default:
         break;
   }
   return getTypeInfoForType(t);
}

AbstractQoreNode* getDefaultValueForBuiltinValueType(qore_type_t t) {
   def_val_map_t::iterator i = def_val_map.find(t);
   assert(i != def_val_map.end());
   return i->second->refSelf();
}

bool builtinTypeHasDefaultValue(qore_type_t t) {
   return def_val_map.find(t) != def_val_map.end();
}

const QoreTypeInfo* getBuiltinUserTypeInfo(const char* str) {
   str_typeinfo_map_t::iterator i = str_typeinfo_map.find(str);
   if (i == str_typeinfo_map.end())
      return nullptr;

   const QoreTypeInfo* rv = i->second;
   // return type "any" for reference types if PO_BROKEN_REFERENCES is set
   if (rv == referenceTypeInfo && (getProgram()->getParseOptions64() & PO_BROKEN_REFERENCES))
      rv = anyTypeInfo;
   return rv;
}

const QoreTypeInfo* getBuiltinUserOrNothingTypeInfo(const char* str) {
   str_typeinfo_map_t::iterator i = str_ornothingtypeinfo_map.find(str);
   if (i == str_ornothingtypeinfo_map.end())
      return nullptr;

   const QoreTypeInfo* rv = i->second;
   // return type "any" for reference types if PO_BROKEN_REFERENCES is set
   if (rv == referenceOrNothingTypeInfo && (getProgram()->getParseOptions64() & PO_BROKEN_REFERENCES))
      rv = anyTypeInfo;

   return rv;
}

const char* getBuiltinTypeName(qore_type_t type) {
   type_str_map_t::iterator i = type_str_map.find(type);
   if (i != type_str_map.end())
      return i->second;

   const QoreTypeInfo* typeInfo = getExternalTypeInfoForType(type);
   if (typeInfo)
      return QoreTypeInfo::getName(typeInfo);
   return "<unknown type>";
}

static qore_type_result_e match_type(const QoreTypeInfo* this_type, const QoreTypeInfo* that_type, bool& may_not_match, bool& may_need_filter) {
   //printd(5, "QoreTypeSpec::match() '%s' <- '%s'\n", QoreTypeInfo::getName(u.ti), QoreTypeInfo::getName(t.u.ti));
   qore_type_result_e res = QoreTypeInfo::parseAccepts(this_type, that_type, may_not_match, may_need_filter);
   if (may_not_match)
      return QTI_NOT_EQUAL;
   // even if types are 100% compatible, if they are not equal, then we perform type folding
   if (res == QTI_IDENT && !may_need_filter && !QoreTypeInfo::equal(this_type, that_type)) {
      may_need_filter = true;
      res = QTI_AMBIGUOUS;
   }
   return res;
}

qore_type_result_e QoreTypeSpec::match(const QoreTypeSpec& t, bool& may_not_match, bool& may_need_filter) const {
   //printd(5, "QoreTypeSpec::match() typespec: %d t.typespec: %d\n", (int)typespec, (int)t.typespec);
   switch (typespec) {
      case QTS_CLASS: {
         switch (t.typespec) {
            case QTS_CLASS:
               return qore_class_private::get(*t.u.qc)->parseCheckCompatibleClass(*qore_class_private::get(*u.qc), may_not_match);
            default: {
               // NOTE: with %strict-types, anything with may_not_match = true must return QTI_NOT_EQUAL
               qore_type_t tt = t.getType();
               if (tt == NT_ALL || tt == NT_OBJECT) {
                  may_not_match = true;
                  return QTI_AMBIGUOUS;
               }
               return QTI_NOT_EQUAL;
            }
         }
         return QTI_NOT_EQUAL;
      }
      case QTS_HASHDECL: {
         switch (t.typespec) {
            case QTS_HASHDECL:
               return typed_hash_decl_private::get(*t.u.hd)->parseEqual(*typed_hash_decl_private::get(*u.hd)) ? QTI_IDENT : QTI_NOT_EQUAL;
            default: {
               return QTI_NOT_EQUAL;
            }
         }
         return QTI_NOT_EQUAL;
      }
      case QTS_COMPLEXHASH: {
         //printd(5, "QoreTypeSpec::match() t.typespec: %d '%s'\n", (int)t.typespec, QoreTypeInfo::getName(u.ti));
         switch (t.typespec) {
            case QTS_COMPLEXHASH:
               return match_type(u.ti, t.u.ti, may_not_match, may_need_filter);
            default: {
               return QTI_NOT_EQUAL;
            }
         }
         return QTI_NOT_EQUAL;
      }
      case QTS_COMPLEXSOFTLIST:
      case QTS_COMPLEXLIST: {
         //printd(5, "QoreTypeSpec::match() t.typespec: %d '%s'\n", (int)t.typespec, QoreTypeInfo::getName(u.ti));
         switch (t.typespec) {
            case QTS_COMPLEXSOFTLIST:
            case QTS_COMPLEXLIST:
               return match_type(u.ti, t.u.ti, may_not_match, may_need_filter);
            default: {
               return QTI_NOT_EQUAL;
            }
         }
         return QTI_NOT_EQUAL;
      }
      case QTS_COMPLEXREF: {
         //printd(5, "QoreTypeSpec::match() t.typespec: %d '%s'\n", (int)t.typespec, QoreTypeInfo::getName(u.ti));
         switch (t.typespec) {
            case QTS_COMPLEXREF: {
               //printd(5, "pcr: '%s' '%s' eq: %d ss: %d\n", QoreTypeInfo::getName(t.u.ti), QoreTypeInfo::getName(u.ti), QoreTypeInfo::equal(u.ti, t.u.ti), QoreTypeInfo::outputSuperSetOf(t.u.ti, u.ti));
               if (QoreTypeInfo::equal(u.ti, t.u.ti))
                  return QTI_IDENT;
               return QoreTypeInfo::outputSuperSetOf(t.u.ti, u.ti) ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
            }
            case QTS_TYPE:
               return t.getType() == NT_REFERENCE ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
            default:
               return QTI_NOT_EQUAL;
         }
         return QTI_NOT_EQUAL;
      }
      case QTS_TYPE: {
         qore_type_t ot = t.getType();
         if (u.t == NT_ALL) {
            return QTI_WILDCARD;
         }
         // NOTE: with %strict-types, anything with may_not_match = true must return QTI_NOT_EQUAL
         if (ot == NT_ALL) {
            may_not_match = true;
            return QTI_AMBIGUOUS;
         }
         if (u.t == ot) {
            // check special cases
            if ((u.t == NT_LIST || u.t == NT_HASH) && t.typespec != QTS_TYPE)
               return QTI_NEAR;
            return QTI_IDENT;
         }
         return QTI_NOT_EQUAL;
      }
   }
   return QTI_NOT_EQUAL;
}

bool QoreTypeSpec::acceptInput(ExceptionSink* xsink, const QoreTypeInfo& typeInfo, q_type_map_t map, bool obj, int param_num, const char* param_name, QoreValue& n) const {
   bool priv_error = false;
   bool ok = false;

   switch (typespec) {
      case QTS_CLASS: {
         if (n.getType() == NT_OBJECT) {
            bool priv;
            if (!n.get<const QoreObject>()->getClass()->getClass(*u.qc, priv))
               break;
            if (!priv) {
               ok = true;
               break;
            }
            // check access
            if (qore_class_private::runtimeCheckPrivateClassAccess(*u.qc)) {
               ok = true;
               break;
            }
            priv_error = true;
         }
         break;
      }
      case QTS_HASHDECL: {
         if (n.getType() == NT_HASH) {
            const TypedHashDecl* hd = n.get<const QoreHashNode>()->getHashDecl();
            if (hd && typed_hash_decl_private::get(*hd)->equal(*typed_hash_decl_private::get(*u.hd))) {
               ok = true;
               break;
            }
         }
         break;
      }
      case QTS_COMPLEXHASH: {
         if (n.getType() == NT_HASH) {
            QoreHashNode* h = n.get<QoreHashNode>();
            const QoreTypeInfo* ti = h->getValueTypeInfo();
            if (QoreTypeInfo::equal(u.ti, ti)) {
               ok = true;
               break;
            }

            // try to fold values into our type; value types are not identical;
            // we have to get a new hash
            if (!h->is_unique()) {
               discard(n.assign(h = qore_hash_private::get(*h)->copy(&typeInfo)), xsink);
               if (*xsink)
                  return true;
            }
            else
               qore_hash_private::get(*h)->complexTypeInfo = &typeInfo;

            // now we have to fold the value types into our type
            HashIterator i(h);
            while (i.next()) {
               hash_assignment_priv ha(*qore_hash_private::get(*h), *qhi_priv::get(i)->i);
               QoreValue hn(ha.swap(nullptr));
               u.ti->acceptInputIntern(xsink, obj, param_num, param_name, hn);
               ha.swap(hn.takeNode());
               if (*xsink)
                  return true;
            }

            ok = true;
         }
         break;
      }
      case QTS_COMPLEXSOFTLIST:
      case QTS_COMPLEXLIST: {
         if (n.getType() == NT_LIST) {
            QoreListNode* l = n.get<QoreListNode>();
            const QoreTypeInfo* ti = l->getValueTypeInfo();
            if (QoreTypeInfo::equal(u.ti, ti)) {
               ok = true;
               break;
            }

            // try to fold values into our type; value types are not identical;
            // we have to get a new list
            qore_list_private* lp;
            if (!l->is_unique()) {
               discard(n.assign(l = qore_list_private::get(*l)->copy(&typeInfo)), xsink);
               if (*xsink)
                  return true;
               lp = qore_list_private::get(*l);
            }
            else {
               lp = qore_list_private::get(*l);
               lp->complexTypeInfo = &typeInfo;
            }

            // now we have to fold the value types into our type
            for (size_t i = 0; i < l->size(); ++i) {
               QoreValue ln(lp->takeExists(i));
               u.ti->acceptInputIntern(xsink, obj, param_num, param_name, ln);
               lp->swap(i, ln.takeNode());
               if (*xsink)
                  return true;
            }

            ok = true;
         }
         else if (typespec == QTS_COMPLEXSOFTLIST) {
            QoreValue val = n;
            n.swap(val);
            n.assign(qore_list_private::newComplexListFromValue(&typeInfo, val, xsink));
            ok = true;
         }
         break;
      }
      case QTS_COMPLEXREF: {
         if (n.getType() == NT_REFERENCE) {
            ReferenceNode* r = n.get<ReferenceNode>();
            const QoreTypeInfo* ti = r->getLValueTypeInfo();
            //printd(5, "cr: %p '%s' == %p '%s': %d\n", u.ti, QoreTypeInfo::getName(u.ti), ti, QoreTypeInfo::getName(ti), QoreTypeInfo::isOutputSubset(u.ti, ti));
            // first check types before instantiating reference
            if (QoreTypeInfo::outputSuperSetOf(ti, u.ti)) {
               // do not process if there is no type restriction
               LValueHelper lvh(r, xsink);
               if (lvh) {
                  QoreValue val = lvh.getReferencedValue();
                  if (!val.isNothing()) {
                     lvh.setTypeInfo(u.ti);
                     //printd(5, "ref assign '%s' to '%s'\n", QoreTypeInfo::getName(val.getTypeInfo()), QoreTypeInfo::getName(u.ti));
                     lvh.assign(val, "<reference>");
                  }
                  // we set ok unconditionally here, because any exception thrown above is enough if there is an error
                  ok = true;
               }
            }
         }
         break;
      }
      case QTS_TYPE:
         if (u.t == NT_ALL || u.t == n.getType())
            ok = true;
         break;
   }

   if (ok) {
      assert(!priv_error);
      if (map)
         map(n, xsink);
      return true;
   }

   if (priv_error) {
      typeInfo.doAcceptError(true, obj, param_num, param_name, n, xsink);
      return true;
   }
   return false;
}

bool QoreTypeSpec::operator==(const QoreTypeSpec& other) const {
   if (typespec != other.typespec)
      return false;
   switch (typespec) {
      case QTS_TYPE:
         return u.t == other.u.t;
      case QTS_CLASS:
         return qore_class_private::get(*u.qc)->equal(*qore_class_private::get(*other.u.qc));
      case QTS_HASHDECL:
         return typed_hash_decl_private::get(*u.hd)->equal(*typed_hash_decl_private::get(*other.u.hd));
      case QTS_COMPLEXHASH:
      case QTS_COMPLEXLIST:
      case QTS_COMPLEXSOFTLIST:
      case QTS_COMPLEXREF:
         return QoreTypeInfo::equal(u.ti, other.u.ti);
   }
   return false;
}

bool QoreTypeSpec::operator!=(const QoreTypeSpec& other) const {
   return !(*this == other);
}

qore_type_result_e QoreTypeSpec::runtimeAcceptsValue(const QoreValue& n, bool exact) const {
   qore_type_t ot = n.getType();
   if (ot == NT_OBJECT && typespec == QTS_CLASS) {
      qore_type_result_e rv = qore_class_private::runtimeCheckCompatibleClass(*u.qc, *n.get<const QoreObject>()->getClass());
      if (rv == QTI_NOT_EQUAL)
         return rv;
      return (rv == QTI_IDENT && exact) ? QTI_IDENT : QTI_AMBIGUOUS;
   }
   else if (ot == NT_HASH && typespec == QTS_HASHDECL) {
      const TypedHashDecl* hd = n.get<const QoreHashNode>()->getHashDecl();
      if (hd && typed_hash_decl_private::get(*u.hd)->equal(*typed_hash_decl_private::get(*hd)))
         return exact ? QTI_IDENT : QTI_AMBIGUOUS;
   }
   else if (ot == NT_HASH && typespec == QTS_COMPLEXHASH) {
      const QoreTypeInfo* ti = n.get<const QoreHashNode>()->getValueTypeInfo();
      if (ti && QoreTypeInfo::equal(u.ti, ti))
         return exact ? QTI_IDENT : QTI_AMBIGUOUS;
   }
   else if (ot == NT_LIST && (typespec == QTS_COMPLEXLIST || typespec == QTS_COMPLEXSOFTLIST)) {
      const QoreTypeInfo* ti = n.get<const QoreListNode>()->getValueTypeInfo();
      if (ti && QoreTypeInfo::equal(u.ti, ti))
         return exact ? QTI_IDENT : QTI_AMBIGUOUS;
   }
   else if (typespec == QTS_COMPLEXSOFTLIST) {
      return QTI_AMBIGUOUS;
   }
   else if (ot == NT_REFERENCE && typespec == QTS_COMPLEXREF) {
      const QoreTypeInfo* ti = n.get<const ReferenceNode>()->getLValueTypeInfo();
      //printd(5, "QoreTypeSpec::runtimeAcceptsValue() cr ti: '%s' typeInfo: '%s' eq: %d ss: %d\n", QoreTypeInfo::getName(ti), QoreTypeInfo::getName(u.ti), QoreTypeInfo::equal(u.ti, ti), QoreTypeInfo::outputSuperSetOf(ti, u.ti));
      if (QoreTypeInfo::equal(u.ti, ti))
         return QTI_IDENT;
      if (QoreTypeInfo::outputSuperSetOf(ti, u.ti))
         return exact ? QTI_IDENT : QTI_AMBIGUOUS;
   }
   else {
      // check special cases
      if (u.t == NT_HASH && ot == NT_HASH) {
         const qore_hash_private* h = qore_hash_private::get(*n.get<const QoreHashNode>());
         if (h->hashdecl || h->complexTypeInfo)
            return QTI_NEAR;
         return exact ? QTI_IDENT : QTI_AMBIGUOUS;
      }
      if (u.t == NT_LIST && ot == NT_LIST) {
            const qore_list_private* l = qore_list_private::get(*n.get<const QoreListNode>());
            if (l->complexTypeInfo)
               return QTI_NEAR;
            return exact ? QTI_IDENT : QTI_AMBIGUOUS;
         }

      if (u.t == NT_ALL || u.t == ot)
         return exact ? QTI_IDENT : QTI_AMBIGUOUS;
   }
   return QTI_NOT_EQUAL;
}

qore_type_result_e QoreTypeInfo::runtimeAcceptsValue(const QoreValue& n) const {
   for (auto& t : accept_vec) {
      qore_type_result_e rv = t.spec.runtimeAcceptsValue(n, t.exact);
      if (rv != QTI_NOT_EQUAL)
         return rv;
   }
   return QTI_NOT_EQUAL;
}

void QoreTypeInfo::doNonNumericWarning(const QoreProgramLocation& loc, const char* preface) const {
   QoreStringNode* desc = new QoreStringNode(preface);
   getThisTypeImpl(*desc);
   desc->sprintf(", which does not evaluate to a numeric type, therefore will always evaluate to 0 at runtime");
   qore_program_private::makeParseWarning(getProgram(), loc, QP_WARN_INVALID_OPERATION, "INVALID-OPERATION", desc);
}

void QoreTypeInfo::doNonBooleanWarning(const QoreProgramLocation& loc, const char* preface) const {
   QoreStringNode* desc = new QoreStringNode(preface);
   getThisTypeImpl(*desc);
   desc->sprintf(", which does not evaluate to a numeric or boolean type, therefore will always evaluate to False at runtime");
   qore_program_private::makeParseWarning(getProgram(), loc, QP_WARN_INVALID_OPERATION, "INVALID-OPERATION", desc);
}

void QoreTypeInfo::doNonStringWarning(const QoreProgramLocation& loc, const char* preface) const {
   QoreStringNode* desc = new QoreStringNode(preface);
   getThisTypeImpl(*desc);
   desc->sprintf(", which cannot be converted to a string, therefore will always evaluate to an empty string at runtime");
   qore_program_private::makeParseWarning(getProgram(), loc, QP_WARN_INVALID_OPERATION, "INVALID-OPERATION", desc);
}

void QoreTypeInfo::stripTypeInfo(QoreValue& n, ExceptionSink* xsink) {
   // strips complex typeinfo for an assignment to an untyped lvalue
   switch (n.getType()) {
      case NT_HASH: {
         map_get_plain_hash(n, xsink);
         break;
      }
      case NT_LIST: {
         map_get_plain_list(n, xsink);
         break;
      }
   }
}

template <typename T>
bool typespec_vec_compare(const T& a, const T& b) {
   if (a.size() != b.size())
      return false;
   for (unsigned i = 0; i < a.size(); ++i) {
      if (a[i].spec != b[i].spec)
         return false;
   }
   return true;
}

bool accept_vec_compare(const q_accept_vec_t& a, const q_accept_vec_t& b) {
   return typespec_vec_compare<q_accept_vec_t>(a, b);
}

bool return_vec_compare(const q_return_vec_t& a, const q_return_vec_t& b) {
   return typespec_vec_compare<q_return_vec_t>(a, b);
}

const QoreTypeInfo* QoreParseTypeInfo::resolveSubtype(const QoreProgramLocation& loc) const {
   if (!strcmp(cscope->ostr, "hash")) {
      if (subtypes.size() == 1) {
         if (!strcmp(subtypes[0]->cscope->ostr, "auto"))
            return or_nothing ? autoHashOrNothingTypeInfo : autoHashTypeInfo;
         // resolve hashdecl
         const TypedHashDecl* hd = qore_root_ns_private::get(*getRootNS())->parseFindHashDecl(loc, *subtypes[0]->cscope);
         //printd(5, "QoreParseTypeInfo::resolveSubtype() this: %p '%s' hd: %p '%s' type: %p (pgm: %p)\n", this, getName(), hd, hd ? hd->getName() : "n/a", hd ? hd->getTypeInfo(false) : nullptr, getProgram());
         return hd ? hd->getTypeInfo(or_nothing) : hashTypeInfo;
      }
      if (subtypes.size() == 2) {
         if (strcmp(subtypes[0]->cscope->ostr, "string")) {
            parseException(loc, "PARSE-TYPE-ERROR", "invalid complex hash type '%s'; hash key type must be 'string'; cannot declare a hash with key type '%s'", getName(), subtypes[0]->cscope->ostr);
         }
         else {
            if (!strcmp(subtypes[1]->cscope->ostr, "auto"))
              return or_nothing ? autoHashOrNothingTypeInfo : autoHashTypeInfo;

            // resolve value type
            const QoreTypeInfo* valueType = QoreParseTypeInfo::resolveAny(subtypes[1], loc);
            if (QoreTypeInfo::hasType(valueType)) {
               return !or_nothing
                  ? qore_program_private::get(*getProgram())->getComplexHashType(valueType)
                  : qore_program_private::get(*getProgram())->getComplexHashOrNothingType(valueType);
            }
         }
      }
      else {
         parseException(loc, "PARSE-TYPE-ERROR", "cannot resolve '%s' with %d type arguments; base type 'hash' takes a single hashdecl name as a subtype argument or two type names giving the key and value types", getName(), (int)subtypes.size());
      }
      return or_nothing ? hashOrNothingTypeInfo : hashTypeInfo;
   }
   if (!strcmp(cscope->ostr, "list")) {
      if (subtypes.size() == 1) {
         if (!strcmp(subtypes[0]->cscope->ostr, "auto"))
            return or_nothing ? autoListOrNothingTypeInfo : autoListTypeInfo;
         // resolve value type
         const QoreTypeInfo* valueType = QoreParseTypeInfo::resolveAny(subtypes[0], loc);
         if (QoreTypeInfo::hasType(valueType)) {
            return !or_nothing
            ? qore_program_private::get(*getProgram())->getComplexListType(valueType)
            : qore_program_private::get(*getProgram())->getComplexListOrNothingType(valueType);
         }
      }
      else {
         parseException(loc, "PARSE-TYPE-ERROR", "cannot resolve '%s' with %d type arguments; base type 'list' takes a single type name giving list element value type", getName(), (int)subtypes.size());
      }
      return or_nothing ? listOrNothingTypeInfo : listTypeInfo;
   }
   if (!strcmp(cscope->ostr, "softlist")) {
      if (subtypes.size() == 1) {
         if (!strcmp(subtypes[0]->cscope->ostr, "auto"))
            return or_nothing ? softAutoListOrNothingTypeInfo : softAutoListTypeInfo;
         // resolve value type
         const QoreTypeInfo* valueType = QoreParseTypeInfo::resolveAny(subtypes[0], loc);
         if (QoreTypeInfo::hasType(valueType)) {
            return !or_nothing
            ? qore_program_private::get(*getProgram())->getComplexSoftListType(valueType)
            : qore_program_private::get(*getProgram())->getComplexSoftListOrNothingType(valueType);
         }
      }
      else {
         parseException(loc, "PARSE-TYPE-ERROR", "cannot resolve '%s' with %d type arguments; base type 'softlist' takes a single type name giving list element value type", getName(), (int)subtypes.size());
      }
      return or_nothing ? softListOrNothingTypeInfo : softListTypeInfo;
   }
   if (!strcmp(cscope->ostr, "reference")) {
      if (subtypes.size() == 1) {
         if (!strcmp(subtypes[0]->cscope->ostr, "auto"))
            return or_nothing ? referenceOrNothingTypeInfo : referenceTypeInfo;
         // resolve value type
         const QoreTypeInfo* valueType = QoreParseTypeInfo::resolveAny(subtypes[0], loc);
         if (QoreTypeInfo::hasType(valueType)) {
            return !or_nothing
            ? qore_program_private::get(*getProgram())->getComplexReferenceType(valueType)
            : qore_program_private::get(*getProgram())->getComplexReferenceOrNothingType(valueType);
         }
      }
      else {
         parseException(loc, "PARSE-TYPE-ERROR", "cannot resolve '%s' with %d type arguments; base type 'reference' takes a single type name giving referenced lvalue type", getName(), (int)subtypes.size());
      }
      return or_nothing ? referenceOrNothingTypeInfo : referenceTypeInfo;
   }
   if (!strcmp(cscope->ostr, "object")) {
      if (subtypes.size() != 1) {
         parseException(loc, "PARSE-TYPE-ERROR", "cannot resolve '%s'; base type 'object' takes a single class name as a subtype argument", getName());
         return or_nothing ? objectOrNothingTypeInfo : objectTypeInfo;
      }

      if (!strcmp(subtypes[0]->cscope->ostr, "auto"))
         return or_nothing ? objectOrNothingTypeInfo : objectTypeInfo;

      // resolve class
      return resolveClass(loc, *subtypes[0]->cscope, or_nothing);
   }

   parseException(loc, "PARSE-TYPE-ERROR", "cannot resolve '%s'; type '%s' does not take subtype declarations", getName(), cscope->getIdentifier());
   return autoTypeInfo;
}

const QoreTypeInfo* QoreParseTypeInfo::resolve(const QoreProgramLocation& loc) const {
   if (!subtypes.empty())
      return resolveSubtype(loc);

   return resolveClass(loc, *cscope, or_nothing);
}

const QoreTypeInfo* QoreParseTypeInfo::resolveAny(const QoreProgramLocation& loc) const {
   if (!subtypes.empty())
      return resolveSubtype(loc);

   const QoreTypeInfo* rv = or_nothing ? getBuiltinUserOrNothingTypeInfo(cscope->ostr) : getBuiltinUserTypeInfo(cscope->ostr);
   return rv ? rv : resolveClass(loc, *cscope, or_nothing);
}

const QoreTypeInfo* QoreParseTypeInfo::resolveAndDelete(const QoreProgramLocation& loc) {
   std::unique_ptr<QoreParseTypeInfo> holder(this);
   return resolve(loc);
}

const QoreTypeInfo* QoreParseTypeInfo::resolveClass(const QoreProgramLocation& loc, const NamedScope& cscope, bool or_nothing) {
   // resolve class
   const QoreClass* qc = qore_root_ns_private::parseFindScopedClass(loc, cscope);

   if (qc && or_nothing) {
      const QoreTypeInfo* rv = qc->getOrNothingTypeInfo();
      if (!rv) {
         parse_error(loc, "class %s cannot be typed with '*' as the class's type handler has an input filter and the filter does not accept NOTHING", qc->getName());
         return objectOrNothingTypeInfo;
      }
      return rv;
   }

   // qc maybe NULL when the class is not found
   return qc ? qc->getTypeInfo() : objectTypeInfo;
}

QoreValue QoreHashDeclTypeInfo::getDefaultQoreValueImpl() const {
   return qore_hash_private::newHashDecl(accept_vec[0].spec.getHashDecl());
   //return new QoreHashNode(accept_vec[0].spec.getHashDecl(), xsink);
}

QoreComplexSoftListTypeInfo::QoreComplexSoftListTypeInfo(const QoreTypeInfo* vti) : QoreComplexListTypeInfo(q_accept_vec_t {
      {QoreComplexListTypeSpec(vti), nullptr, true},
      {NT_LIST, [vti] (QoreValue& n, ExceptionSink* xsink) {
            QoreValue val;
            n.swap(val);
            n.assign(qore_list_private::newComplexListFromValue(qore_program_private::get(*getProgram())->getComplexListType(vti), val, xsink));
         }
      },
      {NT_NOTHING, [vti] (QoreValue& n, ExceptionSink* xsink) {
            QoreListNode* l = new QoreListNode(vti);
            n.assign(l);
         }
      },
      {NT_ALL, [vti] (QoreValue& n, ExceptionSink* xsink) {
            QoreValue val;
            n.swap(val);
            n.assign(qore_list_private::newComplexListFromValue(qore_program_private::get(*getProgram())->getComplexListType(vti), val, xsink));
         }
      },
   }, q_return_vec_t {{QoreComplexListTypeSpec(vti), true}}) {
   assert(vti);
   tname.sprintf("softlist<%s>", QoreTypeInfo::getName(vti));
}

QoreComplexSoftListOrNothingTypeInfo::QoreComplexSoftListOrNothingTypeInfo(const QoreTypeInfo* vti) : QoreComplexListOrNothingTypeInfo(q_accept_vec_t {
      {QoreComplexListTypeSpec(vti), nullptr},
      {NT_LIST, [vti] (QoreValue& n, ExceptionSink* xsink) {
            QoreValue val;
            n.swap(val);
            n.assign(qore_list_private::newComplexListFromValue(qore_program_private::get(*getProgram())->getComplexListType(vti), val, xsink));
         }
      },
      {NT_NOTHING, nullptr},
      {NT_NULL, [] (QoreValue& n, ExceptionSink* xsink) { n.assignNothing(); }},
      {NT_ALL, [vti] (QoreValue& n, ExceptionSink* xsink) {
            QoreValue val;
            n.swap(val);
            n.assign(qore_list_private::newComplexListFromValue(qore_program_private::get(*getProgram())->getComplexListType(vti), val, xsink));
         }
      },
      }, q_return_vec_t {{QoreComplexListTypeSpec(vti)}, {NT_NOTHING}}) {
   assert(vti);
   tname.sprintf("*softlist<%s>", QoreTypeInfo::getName(vti));
}

void map_get_plain_hash(QoreValue& n, ExceptionSink* xsink) {
   ReferenceHolder<QoreHashNode> h(n.get<QoreHashNode>(), xsink);
   n.assign(copy_strip_complex_types(*h));

   /*
   QoreHashNode* h = n.get<QoreHashNode>();
   qore_hash_private* ph = qore_hash_private::get(*h);
   //printd(5, "map_get_plain_hash ph: %p hd: %p c: %p refs: %d\n", ph, ph->hashdecl, ph->complexTypeInfo, h->reference_count());
   if (!ph->hashdecl && !ph->complexTypeInfo)
      return;

   if (!h->is_unique()) {
      discard(n.assign(ph->copy(true)), xsink);
      return;
   }
   ph->hashdecl = nullptr;
   ph->complexTypeInfo = nullptr;
   */
}

void map_get_plain_list(QoreValue& n, ExceptionSink* xsink) {
   ReferenceHolder<QoreListNode> l(n.get<QoreListNode>(), xsink);
   n.assign(copy_strip_complex_types(*l));

   /*
   QoreListNode* l = n.get<QoreListNode>();
   qore_list_private* pl = qore_list_private::get(*l);
   //printd(5, "map_get_plain_list pl: %p c: %p refs: %d\n", ph, pl->complexTypeInfo, l->reference_count());
   if (!pl->complexTypeInfo)
      return;

   if (!l->is_unique()) {
      discard(n.assign(pl->copy(true)), xsink);
      return;
   }
   pl->complexTypeInfo = nullptr;
   */
}