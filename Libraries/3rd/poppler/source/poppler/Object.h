//========================================================================
//
// Object.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2007 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2008 Kees Cook <kees@outflux.net>
// Copyright (C) 2008, 2010, 2017 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Jakub Wilk <jwilk@jwilk.net>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 Adrian Perez de Castro <aperez@igalia.com>
// Copyright (C) 2016 Jakub Alba <jakubalba@gmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef OBJECT_H
#define OBJECT_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <set>
#include <stdio.h>
#include <string.h>
#include "goo/gtypes.h"
#include "goo/gmem.h"
#include "goo/GooString.h"
#include "goo/GooLikely.h"
#include "Error.h"

#define OBJECT_TYPE_CHECK(wanted_type) \
    if (unlikely(type != wanted_type)) { \
        error(errInternal, 0, "Call to Object where the object was type {0:d}, " \
                 "not the expected type {1:d}", type, wanted_type); \
        abort(); \
    }

#define OBJECT_2TYPES_CHECK(wanted_type1, wanted_type2) \
    if (unlikely(type != wanted_type1) && unlikely(type != wanted_type2)) { \
        error(errInternal, 0, "Call to Object where the object was type {0:d}, " \
	      "not the expected type {1:d} or {2:d}", type, wanted_type1, wanted_type2); \
        abort(); \
    }

#define OBJECT_3TYPES_CHECK(wanted_type1, wanted_type2, wanted_type3)	\
    if (unlikely(type != wanted_type1) && unlikely(type != wanted_type2) && unlikely(type != wanted_type3)) { \
        error(errInternal, 0, "Call to Object where the object was type {0:d}, " \
	      "not the expected type {1:d}, {2:d} or {3:d}", type, wanted_type1, wanted_type2, wanted_type3); \
        abort(); \
    }

#define CHECK_NOT_DEAD \
  if (unlikely(type == objDead)) { \
        error(errInternal, 0, "Call to dead object"); \
        abort(); \
    }

class XRef;
class Array;
class Dict;
class Stream;

//------------------------------------------------------------------------
// Ref
//------------------------------------------------------------------------

struct Ref {
  int num;			// object number
  int gen;			// generation number
};

//------------------------------------------------------------------------
// object types
//------------------------------------------------------------------------

enum ObjType {
  // simple objects
  objBool,			// boolean
  objInt,			// integer
  objReal,			// real
  objString,			// string
  objName,			// name
  objNull,			// null

  // complex objects
  objArray,			// array
  objDict,			// dictionary
  objStream,			// stream
  objRef,			// indirect reference

  // special objects
  objCmd,			// command name
  objError,			// error return from Lexer
  objEOF,			// end of file return from Lexer
  objNone,			// uninitialized object

  // poppler-only objects
  objInt64,			// integer with at least 64-bits
  objDead			// and object after shallowCopy
};

#define numObjTypes 16		// total number of object types

//------------------------------------------------------------------------
// Object
//------------------------------------------------------------------------

#ifdef DEBUG_MEM
#define initObj(t) free(); zeroUnion(); ++numAlloc[type = t]
#else
#define initObj(t) free(); zeroUnion(); type = t
#endif

#ifdef DEBUG_MEM
#define constructObj(t) ++numAlloc[type = t]
#else
#define constructObj(t) type = t
#endif

class Object {
public:
  // clear the anonymous union as best we can -- clear at least a pointer
  void zeroUnion() { this->cString = NULL; }

  // Default constructor.
  Object():
    type(objNone) { zeroUnion(); }
  ~Object();

  explicit Object(GBool boolnA)
    { constructObj(objBool); booln = boolnA; }
  explicit Object(int intgA)
    { constructObj(objInt); intg = intgA; }
  explicit Object(ObjType typeA)
    { constructObj(typeA); }
  explicit Object(double realA)
    { constructObj(objReal); real = realA; }
  explicit Object(GooString *stringA)
    { constructObj(objString); string = stringA; }
  Object(ObjType typeA, const char *stringA)
    { constructObj(typeA); cString = copyString(stringA); }
  explicit Object(long long int64gA)
    { constructObj(objInt64); int64g = int64gA; }
  explicit Object(Array *arrayA)
    { constructObj(objArray); array = arrayA; }
  explicit Object(Dict *dictA)
    { constructObj(objDict); dict = dictA; }
  explicit Object(Stream *streamA)
    { constructObj(objStream); stream = streamA; }
  Object(int numA, int genA)
    { constructObj(objRef); ref.num = numA; ref.gen = genA; }
  template<typename T> Object(T) = delete;

  Object(Object&& other);
  Object& operator=(Object&& other);

  Object &operator=(const Object &other) = delete;
  Object(const Object &other) = delete;

  // Set object to null.
  void setToNull() { initObj(objNull); }

  // Copy this to obj
  Object copy() const;

  // If object is a Ref, fetch and return the referenced object.
  // Otherwise, return a copy of the object.
  Object fetch(XRef *xref, int recursion = 0) const;

  // Type checking.
  ObjType getType() const { CHECK_NOT_DEAD; return type; }
  GBool isBool() const { CHECK_NOT_DEAD; return type == objBool; }
  GBool isInt() const { CHECK_NOT_DEAD; return type == objInt; }
  GBool isReal() const { CHECK_NOT_DEAD; return type == objReal; }
  GBool isNum() const { CHECK_NOT_DEAD; return type == objInt || type == objReal || type == objInt64; }
  GBool isString() const { CHECK_NOT_DEAD; return type == objString; }
  GBool isName() const { CHECK_NOT_DEAD; return type == objName; }
  GBool isNull() const { CHECK_NOT_DEAD; return type == objNull; }
  GBool isArray() const { CHECK_NOT_DEAD; return type == objArray; }
  GBool isDict() const { CHECK_NOT_DEAD; return type == objDict; }
  GBool isStream() const { CHECK_NOT_DEAD; return type == objStream; }
  GBool isRef() const { CHECK_NOT_DEAD; return type == objRef; }
  GBool isCmd() const { CHECK_NOT_DEAD; return type == objCmd; }
  GBool isError() const { CHECK_NOT_DEAD; return type == objError; }
  GBool isEOF() const { CHECK_NOT_DEAD; return type == objEOF; }
  GBool isNone() const { CHECK_NOT_DEAD; return type == objNone; }
  GBool isInt64() const { CHECK_NOT_DEAD; return type == objInt64; }
  GBool isIntOrInt64() const { CHECK_NOT_DEAD; return type == objInt || type == objInt64; }

  // Special type checking.
  GBool isName(const char *nameA) const
    { return type == objName && !strcmp(cString, nameA); }
  GBool isDict(const char *dictType) const;
  GBool isStream(char *dictType) const;
  GBool isCmd(const char *cmdA) const
    { return type == objCmd && !strcmp(cString, cmdA); }

  // Accessors.
  GBool getBool() const { OBJECT_TYPE_CHECK(objBool); return booln; }
  int getInt() const { OBJECT_TYPE_CHECK(objInt); return intg; }
  double getReal() const { OBJECT_TYPE_CHECK(objReal); return real; }

  // Note: integers larger than 2^53 can not be exactly represented by a double.
  // Where the exact value of integers up to 2^63 is required, use isInt64()/getInt64().
  double getNum() const { OBJECT_3TYPES_CHECK(objInt, objInt64, objReal);
    return type == objInt ? (double)intg : type == objInt64 ? (double)int64g : real; }
  double getNum(bool *ok) const {
    if (unlikely(type != objInt && type != objInt64 && type != objReal)) {
      *ok = false;
      return 0.;
    }
    return type == objInt ? (double)intg : type == objInt64 ? (double)int64g : real;
  }
  GooString *getString() const { OBJECT_TYPE_CHECK(objString); return string; }
  // After takeString() the only method that should be called for the object is free()
  // because the object it's not expected to have a NULL string.
  GooString *takeString() {
    OBJECT_TYPE_CHECK(objString); GooString *s = string; string = NULL; return s; }
  char *getName() const { OBJECT_TYPE_CHECK(objName); return cString; }
  Array *getArray() const { OBJECT_TYPE_CHECK(objArray); return array; }
  Dict *getDict() const { OBJECT_TYPE_CHECK(objDict); return dict; }
  Stream *getStream() const { OBJECT_TYPE_CHECK(objStream); return stream; }
  Ref getRef() const { OBJECT_TYPE_CHECK(objRef); return ref; }
  int getRefNum() const { OBJECT_TYPE_CHECK(objRef); return ref.num; }
  int getRefGen() const { OBJECT_TYPE_CHECK(objRef); return ref.gen; }
  char *getCmd() const { OBJECT_TYPE_CHECK(objCmd); return cString; }
  long long getInt64() const { OBJECT_TYPE_CHECK(objInt64); return int64g; }
  long long getIntOrInt64() const { OBJECT_2TYPES_CHECK(objInt, objInt64);
    return type == objInt ? intg : int64g; }

  // Array accessors.
  int arrayGetLength() const;
  void arrayAdd(Object &&elem);
  void arrayRemove(int i);
  Object arrayGet(int i, int recursion) const;
  Object arrayGetNF(int i) const;

  // Dict accessors.
  int dictGetLength() const;
  void dictAdd(char *key, Object &&val);
  void dictSet(const char *key, Object &&val);
  void dictRemove(const char *key);
  GBool dictIs(const char *dictType) const;
  Object dictLookup(const char *key, int recursion = 0) const;
  Object dictLookupNF(const char *key) const;
  char *dictGetKey(int i) const;
  Object dictGetVal(int i) const;
  Object dictGetValNF(int i) const;

  // Stream accessors.
  GBool streamIs(char *dictType) const;
  void streamReset();
  void streamClose();
  int streamGetChar() const;
  int streamGetChars(int nChars, Guchar *buffer) const;
  int streamLookChar() const;
  char *streamGetLine(char *buf, int size) const;
  Goffset streamGetPos() const;
  void streamSetPos(Goffset pos, int dir = 0);
  Dict *streamGetDict() const;

  // Output.
  const char *getTypeName() const;
  void print(FILE *f = stdout) const;

  // Memory testing.
  static void memCheck(FILE *f);

private:
  friend class Array; // Needs free and initNullAfterMalloc
  friend class Dict; // Needs free and initNullAfterMalloc
  friend class XRef; // Needs free and initNullAfterMalloc

  // Free object contents.
  void free();

  // Only use if are mallocing Objects
  void initNullAfterMalloc() { constructObj(objNull); }

  ObjType type;			// object type
  union {			// value for each type:
    GBool booln;		//   boolean
    int intg;			//   integer
    long long int64g;           //   64-bit integer
    double real;		//   real
    GooString *string;		//   string
    char *cString;		//   name or command, depending on objType
    Array *array;		//   array
    Dict *dict;			//   dictionary
    Stream *stream;		//   stream
    Ref ref;			//   indirect reference
  };

#ifdef DEBUG_MEM
  static int			// number of each type of object
    numAlloc[numObjTypes];	//   currently allocated
#endif
};

//------------------------------------------------------------------------
// Array accessors.
//------------------------------------------------------------------------

#include "Array.h"

inline int Object::arrayGetLength() const
  { OBJECT_TYPE_CHECK(objArray); return array->getLength(); }

inline void Object::arrayAdd(Object &&elem)
  { OBJECT_TYPE_CHECK(objArray); array->add(std::move(elem)); }

inline void Object::arrayRemove(int i)
  { OBJECT_TYPE_CHECK(objArray); array->remove(i); }

inline Object Object::arrayGet(int i, int recursion = 0) const
  { OBJECT_TYPE_CHECK(objArray); return array->get(i, recursion); }

inline Object Object::arrayGetNF(int i) const
  { OBJECT_TYPE_CHECK(objArray); return array->getNF(i); }

//------------------------------------------------------------------------
// Dict accessors.
//------------------------------------------------------------------------

#include "Dict.h"

inline int Object::dictGetLength() const
  { OBJECT_TYPE_CHECK(objDict); return dict->getLength(); }

inline void Object::dictAdd(char *key, Object &&val)
  { OBJECT_TYPE_CHECK(objDict); dict->add(key, std::move(val)); }

inline void Object::dictSet(const char *key, Object &&val)
  { OBJECT_TYPE_CHECK(objDict); dict->set(key, std::move(val)); }

inline void Object::dictRemove(const char *key)
  { OBJECT_TYPE_CHECK(objDict); dict->remove(key); }

inline GBool Object::dictIs(const char *dictType) const
  { OBJECT_TYPE_CHECK(objDict); return dict->is(dictType); }

inline GBool Object::isDict(const char *dictType) const
  { return type == objDict && dictIs(dictType); }

inline Object Object::dictLookup(const char *key, int recursion) const
  { OBJECT_TYPE_CHECK(objDict); return dict->lookup(key, recursion); }

inline Object Object::dictLookupNF(const char *key) const
  { OBJECT_TYPE_CHECK(objDict); return dict->lookupNF(key); }

inline char *Object::dictGetKey(int i) const
  { OBJECT_TYPE_CHECK(objDict); return dict->getKey(i); }

inline Object Object::dictGetVal(int i) const
  { OBJECT_TYPE_CHECK(objDict); return dict->getVal(i); }

inline Object Object::dictGetValNF(int i) const
  { OBJECT_TYPE_CHECK(objDict); return dict->getValNF(i); }

//------------------------------------------------------------------------
// Stream accessors.
//------------------------------------------------------------------------

#include "Stream.h"

inline GBool Object::streamIs(char *dictType) const
  { OBJECT_TYPE_CHECK(objStream); return stream->getDict()->is(dictType); }

inline GBool Object::isStream(char *dictType) const
  { return type == objStream && streamIs(dictType); }

inline void Object::streamReset()
  { OBJECT_TYPE_CHECK(objStream); stream->reset(); }

inline void Object::streamClose()
  { OBJECT_TYPE_CHECK(objStream); stream->close(); }

inline int Object::streamGetChar() const
  { OBJECT_TYPE_CHECK(objStream); return stream->getChar(); }

inline int Object::streamGetChars(int nChars, Guchar *buffer) const
  { OBJECT_TYPE_CHECK(objStream); return stream->doGetChars(nChars, buffer); }

inline int Object::streamLookChar() const
  { OBJECT_TYPE_CHECK(objStream); return stream->lookChar(); }

inline char *Object::streamGetLine(char *buf, int size) const
  { OBJECT_TYPE_CHECK(objStream); return stream->getLine(buf, size); }

inline Goffset Object::streamGetPos() const
  { OBJECT_TYPE_CHECK(objStream); return stream->getPos(); }

inline void Object::streamSetPos(Goffset pos, int dir)
  { OBJECT_TYPE_CHECK(objStream); stream->setPos(pos, dir); }

inline Dict *Object::streamGetDict() const
  { OBJECT_TYPE_CHECK(objStream); return stream->getDict(); }

#endif
