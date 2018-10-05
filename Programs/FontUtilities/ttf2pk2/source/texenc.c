/*
 *   texenc.c
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>     /* for definition of NULL */

#include "ttf2tfm.h"
#include "texenc.h"


encoding staticencoding =
{
  "TeX text",
  {"Gamma", "Delta", "Theta", "Lambda",
   "Xi", "Pi", "Sigma", "Upsilon",
   "Phi", "Psi", "Omega", "arrowup",
   "arrowdown", "quotesingle", "exclamdown", "questiondown",

   "dotlessi", "dotlessj", "grave", "acute",
   "caron", "breve", "macron", "ring",
   "cedilla", "germandbls", "ae", "oe",
   "oslash", "AE", "OE", "Oslash",

   "space", "exclam", "quotedbl", "numbersign",
   "dollar", "percent", "ampersand", "quoteright",
   "parenleft", "parenright", "asterisk", "plus",
   "comma", "hyphen", "period", "slash",

   "zero", "one", "two", "three",
   "four", "five", "six", "seven",
   "eight", "nine", "colon", "semicolon",
   "less", "equal", "greater", "question",

   "at", "A", "B", "C",
   "D", "E", "F", "G",
   "H", "I", "J", "K",
   "L", "M", "N", "O",

   "P", "Q", "R", "S",
   "T", "U", "V", "W",
   "X", "Y", "Z", "bracketleft",
   "backslash", "bracketright", "circumflex", "underscore",

   "quoteleft", "a", "b", "c",
   "d", "e", "f", "g",
   "h", "i", "j", "k",
   "l", "m", "n", "o",

   "p", "q", "r", "s",
   "t", "u", "v", "w",
   "x", "y", "z", "braceleft",
   "bar", "braceright", "tilde", "dieresis",

   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",

   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",
   
   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",

   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",

   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",

   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",

   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",

   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",
   ".notdef", ".notdef", ".notdef", ".notdef",
  }
};


/*
 *   It's easier to put this in static storage and parse it as we go
 *   than to build the structures ourselves.
 *
 *   The semicolons in the LIGKERN lines must be left isolated.
 */

const char *staticligkern[] =
{
  "% LIGKERN space l =: lslash ; space L =: Lslash ;",
  "% LIGKERN question quoteleft =: questiondown ;",
  "% LIGKERN exclam quoteleft =: exclamdown ;",
  "% LIGKERN hyphen hyphen =: endash ; endash hyphen =: emdash ;",
  "% LIGKERN quoteleft quoteleft =: quotedblleft ;",
  "% LIGKERN quoteright quoteright =: quotedblright ;",
  "% LIGKERN space {} * ; * {} space ; zero {} * ; * {} zero ;",
  "% LIGKERN one {} * ; * {} one ; two {} * ; * {} two ;",
  "% LIGKERN three {} * ; * {} three ; four {} * ; * {} four ;",
  "% LIGKERN five {} * ; * {} five ; six {} * ; * {} six ;",
  "% LIGKERN seven {} * ; * {} seven ; eight {} * ; * {} eight ;",
  "% LIGKERN nine {} * ; * {} nine ;",

  /*
   *   Kern accented characters the same way as their base.
   */

  "% LIGKERN Aacute <> A ; aacute <> a ;",
  "% LIGKERN Acircumflex <> A ; acircumflex <> a ;",
  "% LIGKERN Adieresis <> A ; adieresis <> a ;",
  "% LIGKERN Agrave <> A ; agrave <> a ;",
  "% LIGKERN Aring <> A ; aring <> a ;",
  "% LIGKERN Atilde <> A ; atilde <> a ;",
  "% LIGKERN Ccedilla <> C ; ccedilla <> c ;",
  "% LIGKERN Eacute <> E ; eacute <> e ;",
  "% LIGKERN Ecircumflex <> E ; ecircumflex <> e ;",
  "% LIGKERN Edieresis <> E ; edieresis <> e ;",
  "% LIGKERN Egrave <> E ; egrave <> e ;",
  "% LIGKERN Iacute <> I ; iacute <> i ;",
  "% LIGKERN Icircumflex <> I ; icircumflex <> i ;",
  "% LIGKERN Idieresis <> I ; idieresis <> i ;",
  "% LIGKERN Igrave <> I ; igrave <> i ;",
  "% LIGKERN Ntilde <> N ; ntilde <> n ;",
  "% LIGKERN Oacute <> O ; oacute <> o ;",
  "% LIGKERN Ocircumflex <> O ; ocircumflex <> o ;",
  "% LIGKERN Odieresis <> O ; odieresis <> o ;",
  "% LIGKERN Ograve <> O ; ograve <> o ;",
  "% LIGKERN Oslash <> O ; oslash <> o ;",
  "% LIGKERN Otilde <> O ; otilde <> o ;",
  "% LIGKERN Scaron <> S ; scaron <> s ;",
  "% LIGKERN Uacute <> U ; uacute <> u ;",
  "% LIGKERN Ucircumflex <> U ; ucircumflex <> u ;",
  "% LIGKERN Udieresis <> U ; udieresis <> u ;",
  "% LIGKERN Ugrave <> U ; ugrave <> u ;",
  "% LIGKERN Yacute <> Y ; yacute <> y ;",
  "% LIGKERN Ydieresis <> Y ; ydieresis <> y ;",
  "% LIGKERN Zcaron <> Z ; zcaron <> z ;",

  /* lig commands for default ligatures */

  "% LIGKERN f i =: fi ; f l =: fl ; f f =: ff ; ff i =: ffi ;",
  "% LIGKERN ff l =: ffl ;",
  NULL
};

/*
 *   The above layout corresponds to TeX Typewriter Type and is compatible
 *   with TeX Text because the position of ligatures is immaterial.
 */


/*
 *   These are the eight ligature ops, in VPL terms and in METAFONT terms.
 */

const char *vplligops[] =
{
  "LIG", "/LIG", "/LIG>", "LIG/", "LIG/>", "/LIG/", "/LIG/>", "/LIG/>>", 0
};

const char *encligops[] =
{
  "=:", "|=:", "|=:>", "=:|", "=:|>", "|=:|", "|=:|>", "|=:|>>", 0
};


/*
 *   For TeX we want to compute a character height that works properly
 *   with accents.  The following list of accents doesn't need to be
 *   complete.
 */

/*
 *   We only do this if the xheight has a reasonable value (>50).
 */

const char *accents[] = {"acute", "tilde", "caron", "dieresis", NULL};

char default_codingscheme[] = "Unspecified";


/* end */
