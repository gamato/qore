/*
  VarRefNode.h

  Qore Programming Language

  Copyright 2003 - 2009 David Nichols

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

#ifndef _QORE_VARREFNODE_H

#define _QORE_VARREFNODE_H

class VarRefNode : public ParseNode {
   friend class VarRefNodeEvalOptionalRefHolder;

protected:
   char *name;
   qore_var_t type;

   DLLLOCAL ~VarRefNode();

   // evalImpl(): return value requires a deref(xsink)
   DLLLOCAL virtual AbstractQoreNode *evalImpl(ExceptionSink *xsink) const;

   //! optionally evaluates the argument
   /** return value requires a deref(xsink) if needs_deref is true
       @see AbstractQoreNode::eval()
   */
   DLLLOCAL AbstractQoreNode *evalImpl(bool &needs_deref, ExceptionSink *xsink) const;

   DLLLOCAL virtual int64 bigIntEvalImpl(ExceptionSink *xsink) const;
   DLLLOCAL virtual int integerEvalImpl(ExceptionSink *xsink) const;
   DLLLOCAL virtual bool boolEvalImpl(ExceptionSink *xsink) const;
   DLLLOCAL virtual double floatEvalImpl(ExceptionSink *xsink) const;

   DLLLOCAL void resolve(const QoreTypeInfo *typeInfo, const QoreTypeInfo *&outTypeInfo);
   DLLLOCAL AbstractQoreNode *parseInitIntern(LocalVar *oflag, int pflag, int &lvids, const QoreTypeInfo *typeInfo, const QoreTypeInfo *&outTypeInfo);
   
public:
   union var_u {
      class LocalVar *id;   // for local variables
      class Var *var;       // for global variables
   } ref;

   // takes over memory for "n"
   DLLLOCAL VarRefNode(char *n, qore_var_t t);

   DLLLOCAL virtual int getAsString(QoreString &str, int foff, ExceptionSink *xsink) const;
   DLLLOCAL virtual QoreString *getAsString(bool &del, int foff, ExceptionSink *xsink) const;

   // returns the type name as a c string
   DLLLOCAL virtual const char *getTypeName() const;

   // initializes during parsing
   DLLLOCAL virtual AbstractQoreNode *parseInit(LocalVar *oflag, int pflag, int &lvids, const QoreTypeInfo *&typeInfo);

   DLLLOCAL void setValue(AbstractQoreNode *val, ExceptionSink *xsink);
   DLLLOCAL AbstractQoreNode **getValuePtr(AutoVLock *vl, const QoreTypeInfo *&typeInfo, ExceptionSink *xsink) const;
   DLLLOCAL AbstractQoreNode *getValue(AutoVLock *vl, ExceptionSink *xsink) const;
   DLLLOCAL qore_var_t getType() const { return type; }
   DLLLOCAL const char *getName() const { return name; }
   DLLLOCAL void makeLocal() { assert(type == VT_UNRESOLVED); type = VT_LOCAL; }
   DLLLOCAL void makeGlobal() { assert(type == VT_UNRESOLVED); type = VT_GLOBAL; }
   DLLLOCAL virtual const QoreTypeInfo *getTypeInfo() const { return 0; }

   // takes the name - caller owns the memory
   DLLLOCAL char *takeName();
};

class VarRefDeclNode : public VarRefNode {
protected:
   QoreParseTypeInfo typeInfo;

public:
   DLLLOCAL VarRefDeclNode(char *n, qore_var_t t, qore_type_t n_qt) : VarRefNode(n, t), typeInfo(n_qt) {
   }
   // takes over ownership of class_name
   DLLLOCAL VarRefDeclNode(char *n, qore_var_t t, char *class_name) : VarRefNode(n, t), typeInfo(class_name) {
   }
   // initializes during parsing
   DLLLOCAL virtual AbstractQoreNode *parseInit(LocalVar *oflag, int pflag, int &lvids, const QoreTypeInfo *&typeInfo);

   DLLLOCAL virtual const QoreTypeInfo *getTypeInfo() const { 
      return static_cast<const QoreTypeInfo *>(&typeInfo); 
   }
};

class VarRefNodeEvalOptionalRefHolder {
private:
   AbstractQoreNode *val;
   ExceptionSink *xsink;
   bool needs_deref;

   DLLLOCAL void discard_intern() {
      if (needs_deref && val)
	 val->deref(xsink);
   }

   //! this function is not implemented; it is here as a private function in order to prohibit it from being used
   DLLLOCAL VarRefNodeEvalOptionalRefHolder(const VarRefNodeEvalOptionalRefHolder&);

   //! this function is not implemented; it is here as a private function in order to prohibit it from being used
   DLLLOCAL VarRefNodeEvalOptionalRefHolder& operator=(const VarRefNodeEvalOptionalRefHolder&);

   //! this function is not implemented; it is here as a private function in order to prohibit it from being used
   /** this function is not implemented in order to require objects of this type to be allocated on the stack.
    */
   DLLLOCAL void *operator new(size_t);

public:
   //! constructor with a value that will call the class' eval(needs_deref) method
   DLLLOCAL VarRefNodeEvalOptionalRefHolder(const VarRefNode *exp, ExceptionSink *n_xsink) : xsink(n_xsink) {
      if (exp)
	 val = exp->VarRefNode::evalImpl(needs_deref, xsink);
      else {
	 val = 0;
	 needs_deref = false;
      }	    
   }

   //! discards any temporary value evaluated by the constructor or assigned by "assign()"
   DLLLOCAL ~VarRefNodeEvalOptionalRefHolder() {
      discard_intern();
   }
      
   //! discards any temporary value evaluated by the constructor or assigned by "assign()"
   DLLLOCAL void discard() {
      discard_intern();
      needs_deref = false;
      val = 0;
   }

   //! assigns a new value to this holder object
   DLLLOCAL void assign(bool n_needs_deref, AbstractQoreNode *n_val) {
      discard_intern();
      needs_deref = n_needs_deref;
      val = n_val;
   }

   //! returns a referenced value - the caller will own the reference
   DLLLOCAL AbstractQoreNode *getReferencedValue() {
      if (needs_deref)
	 needs_deref = false;
      else if (val)
	 val->ref();
      return val;
   }

   //! returns the object being managed
   DLLLOCAL const AbstractQoreNode *operator->() const { return val; }

   //! returns the object being managed
   DLLLOCAL const AbstractQoreNode *operator*() const { return val; }

   //! returns true if a value is being held
   DLLLOCAL operator bool() const { return val != 0; }
};

#endif
