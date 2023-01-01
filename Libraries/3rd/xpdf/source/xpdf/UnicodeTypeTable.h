//========================================================================
//
// UnicodeTypeTable.h
//
// Copyright 2003-2013 Glyph & Cog, LLC
//
//========================================================================

#ifndef UNICODETYPETABLE_H
#define UNICODETYPETABLE_H

#include "gtypes.h"

extern GBool unicodeTypeL(Unicode c);

extern GBool unicodeTypeR(Unicode c);

extern GBool unicodeTypeDigit(Unicode c);

extern GBool unicodeTypeNumSep(Unicode c);

extern GBool unicodeTypeNum(Unicode c);

extern GBool unicodeTypeAlphaNum(Unicode c);

extern GBool unicodeTypeWord(Unicode c);

extern Unicode unicodeToLower(Unicode c);

extern GBool unicodeBracketInfo(Unicode c, GBool *open, Unicode *opposite);

#endif
