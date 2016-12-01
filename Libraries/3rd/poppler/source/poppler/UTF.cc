//========================================================================
//
// UTF.h
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2008 Koji Otani <sho@bbr.jp>
// Copyright (C) 2012 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2012 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2016 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2016 Jason Crain <jason@aquaticape.us>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include "goo/gmem.h"
#include "PDFDocEncoding.h"
#include "UTF.h"
#include <algorithm>

bool UnicodeIsValid(Unicode ucs4)
{
  return (ucs4 < 0x110000) &&
    ((ucs4 & 0xfffff800) != 0xd800) &&
    (ucs4 < 0xfdd0 || ucs4 > 0xfdef) &&
    ((ucs4 & 0xfffe) != 0xfffe);
}

int UTF16toUCS4(const Unicode *utf16, int utf16Len, Unicode **ucs4)
{
  int i, n, len;
  Unicode *u;

  // count characters
  len = 0;
  for (i = 0; i < utf16Len; i++) {
    if (utf16[i] >= 0xd800 && utf16[i] < 0xdc00 && i + 1 < utf16Len &&
        utf16[i+1] >= 0xdc00 && utf16[i+1] < 0xe000) {
      i++; /* surrogate pair */
    }
    len++;
  }
  if (ucs4 == NULL)
    return len;

  u = (Unicode*)gmallocn(len, sizeof(Unicode));
  n = 0;
  // convert string
  for (i = 0; i < utf16Len; i++) {
    if (utf16[i] >= 0xd800 && utf16[i] < 0xdc00) { /* surrogate pair */
      if (i + 1 < utf16Len && utf16[i+1] >= 0xdc00 && utf16[i+1] < 0xe000) {
	/* next code is a low surrogate */
	u[n] = (((utf16[i] & 0x3ff) << 10) | (utf16[i+1] & 0x3ff)) + 0x10000;
	++i;
      } else {
	/* missing low surrogate
	   replace it with REPLACEMENT CHARACTER (U+FFFD) */
	u[n] = 0xfffd;
      }
    } else if (utf16[i] >= 0xdc00 && utf16[i] < 0xe000) {
      /* invalid low surrogate
	 replace it with REPLACEMENT CHARACTER (U+FFFD) */
      u[n] = 0xfffd;
    } else {
      u[n] = utf16[i];
    }
    if (!UnicodeIsValid(u[n])) {
      u[n] = 0xfffd;
    }
    n++;
  }
  *ucs4 = u;
  return len;
}

int TextStringToUCS4(GooString *textStr, Unicode **ucs4)
{
  int i, len;
  const char *s;
  Unicode *u;

  len = textStr->getLength();
  s = textStr->getCString();
  if (len == 0) {
    *ucs4 = 0;
    return 0;
  }

  if (textStr->hasUnicodeMarker()) {
    Unicode *utf16;
    len = len/2 - 1;
    if (len > 0) {
      utf16 = new Unicode[len];
      for (i = 0 ; i < len; i++) {
        utf16[i] = (s[2 + i*2] & 0xff) << 8 | (s[3 + i*2] & 0xff);
      }
      len = UTF16toUCS4(utf16, len, &u);
      delete[] utf16;
    } else {
      u = NULL;
    }
  } else {
    u = (Unicode*)gmallocn(len, sizeof(Unicode));
    for (i = 0 ; i < len; i++) {
      u[i] = pdfDocEncoding[s[i] & 0xff];
    }
  }
  *ucs4 = u;
  return len;
}

bool UnicodeIsWhitespace(Unicode ucs4)
{
  static Unicode const spaces[] = { 0x0009, 0x000A, 0x000B, 0x000C, 0x000D,
    0x0020, 0x0085, 0x00A0, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005,
    0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x2028, 0x2029, 0x202F, 0x205F,
    0x3000 };
  Unicode const *end = spaces + sizeof(spaces) / sizeof(spaces[0]);
  Unicode const *i = std::lower_bound(spaces, end, ucs4);
  return (i != end && *i == ucs4);
}
