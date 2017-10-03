//========================================================================
//
// Object.cc
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
// Copyright (C) 2008, 2010, 2012, 2017 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stddef.h>
#include "Object.h"
#include "Array.h"
#include "Dict.h"
#include "Error.h"
#include "Stream.h"
#include "XRef.h"

//------------------------------------------------------------------------
// Object
//------------------------------------------------------------------------

static const char *objTypeNames[numObjTypes] = {
  "boolean",
  "integer",
  "real",
  "string",
  "name",
  "null",
  "array",
  "dictionary",
  "stream",
  "ref",
  "cmd",
  "error",
  "eof",
  "none",
  "integer64",
  "dead"
};

#ifdef DEBUG_MEM
int Object::numAlloc[numObjTypes] =
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

Object::Object(Object&& other)
{
  type = other.type;
  real = other.real; // this is the biggest of the union so it's enough
  other.type = objDead;
}

Object& Object::operator=(Object&& other)
{
  free();
  type = other.type;
  real = other.real; // this is the biggest of the union so it's enough
  other.type = objDead;
  return *this;
}

Object::~Object()
{
  free();
}

Object Object::copy() const {
  CHECK_NOT_DEAD;

  Object obj;
  obj.type = type;
  obj.real = real; // this is the biggest of the union so it's enough
  switch (type) {
  case objString:
    obj.string = string->copy();
    break;
  case objName:
    obj.cString = copyString(cString);
    break;
  case objArray:
    array->incRef();
    break;
  case objDict:
    dict->incRef();
    break;
  case objStream:
    stream->incRef();
    break;
  case objCmd:
    obj.cString = copyString(cString);
    break;
  default:
    break;
  }
#ifdef DEBUG_MEM
  ++numAlloc[type];
#endif
  return obj;
}

Object Object::fetch(XRef *xref, int recursion) const {
  CHECK_NOT_DEAD;

  return (type == objRef && xref) ?
         xref->fetch(ref.num, ref.gen, recursion) : copy();
}

void Object::free() {
  switch (type) {
  case objString:
    delete string;
    break;
  case objName:
    gfree(cString);
    break;
  case objArray:
    if (!array->decRef()) {
      delete array;
    }
    break;
  case objDict:
    if (!dict->decRef()) {
      delete dict;
    }
    break;
  case objStream:
    if (!stream->decRef()) {
      delete stream;
    }
    break;
  case objCmd:
    gfree(cString);
    break;
  default:
    break;
  }
#ifdef DEBUG_MEM
  --numAlloc[type];
#endif
  type = objNone;
}

const char *Object::getTypeName() const {
  return objTypeNames[type];
}

void Object::print(FILE *f) const {
  Object obj;
  int i;

  switch (type) {
  case objBool:
    fprintf(f, "%s", booln ? "true" : "false");
    break;
  case objInt:
    fprintf(f, "%d", intg);
    break;
  case objReal:
    fprintf(f, "%g", real);
    break;
  case objString:
    fprintf(f, "(");
    fwrite(string->getCString(), 1, string->getLength(), f);
    fprintf(f, ")");
    break;
  case objName:
    fprintf(f, "/%s", cString);
    break;
  case objNull:
    fprintf(f, "null");
    break;
  case objArray:
    fprintf(f, "[");
    for (i = 0; i < arrayGetLength(); ++i) {
      if (i > 0)
	fprintf(f, " ");
      obj = arrayGetNF(i);
      obj.print(f);
    }
    fprintf(f, "]");
    break;
  case objDict:
    fprintf(f, "<<");
    for (i = 0; i < dictGetLength(); ++i) {
      fprintf(f, " /%s ", dictGetKey(i));
      obj = dictGetValNF(i);
      obj.print(f);
    }
    fprintf(f, " >>");
    break;
  case objStream:
    fprintf(f, "<stream>");
    break;
  case objRef:
    fprintf(f, "%d %d R", ref.num, ref.gen);
    break;
  case objCmd:
    fprintf(f, "%s", cString);
    break;
  case objError:
    fprintf(f, "<error>");
    break;
  case objEOF:
    fprintf(f, "<EOF>");
    break;
  case objNone:
    fprintf(f, "<none>");
    break;
    case objDead:
    fprintf(f, "<dead>");
    break;
  case objInt64:
    fprintf(f, "%lld", int64g);
    break;
  }
}

void Object::memCheck(FILE *f) {
#ifdef DEBUG_MEM
  int i;
  int t;

  t = 0;
  for (i = 0; i < numObjTypes; ++i)
    t += numAlloc[i];
  if (t > 0) {
    fprintf(f, "Allocated objects:\n");
    for (i = 0; i < numObjTypes; ++i) {
      if (numAlloc[i] > 0)
	fprintf(f, "  %-20s: %6d\n", objTypeNames[i], numAlloc[i]);
    }
  }
#else
  (void)f;
#endif
}
