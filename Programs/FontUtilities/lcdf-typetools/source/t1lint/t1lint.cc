/* t1lint.cc -- driver for checking Type 1 fonts for validity
 *
 * Copyright (c) 1999-2019 Eddie Kohler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version. This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <efont/psres.hh>
#include <efont/t1rw.hh>
#include <efont/t1font.hh>
#include <efont/t1item.hh>
#include <efont/t1mm.hh>
#include "cscheck.hh"
#include <lcdf/clp.h>
#include <lcdf/error.hh>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#ifdef HAVE_CTIME
# include <time.h>
#endif
#if defined(_MSDOS) || defined(_WIN32)
# include <fcntl.h>
# include <io.h>
#endif

using namespace Efont;

#define VERSION_OPT	301
#define HELP_OPT	302
#define QUIET_OPT	303

const Clp_Option options[] = {
  { "help", 'h', HELP_OPT, 0, 0 },
  { "quiet", 'q', QUIET_OPT, 0, Clp_Negate },
  { "version", 0, VERSION_OPT, 0, 0 },
};


static const char *program_name;


void
usage_error(ErrorHandler *errh, const char *error_message, ...)
{
  va_list val;
  va_start(val, error_message);
  if (!error_message)
    errh->message("Usage: %s [OPTION]... FONT", program_name);
  else
    errh->xmessage(ErrorHandler::e_error, error_message, val);
  errh->message("Type %s --help for more information.", program_name);
  exit(1);
}

void
usage()
{
    FileErrorHandler uerrh(stdout);
    uerrh.message("\
%<T1lint%> checks a PostScript Type 1 font for correctness. Any errors or\n\
divergences from the specification are reported to standard error. Nothing is\n\
printed for correct fonts. The command exits with status 0 if there are no\n\
errors, and status 1 otherwise.\n\
\n\
Usage: %s [OPTION]... FONT [...]\n\
\n\
Each FONT is the name of a PFA or PFB font file.\n\
\n\
Options:\n\
  -h, --help                   Print this message and exit.\n\
  -q, --quiet                  Do not report errors to standard error.\n\
      --version                Print version number and exit.\n\
\n\
Report bugs to <ekohler@gmail.com>.\n", program_name);
}


/*****
 * CHECKS
 **/

static bool
get_num_array(Type1Font *font, int dictionary, const char *name,
	      Vector<double> &v, ErrorHandler *errh, bool mandatory = false)
{
    if (Type1Definition *d = font->dict(dictionary, name)) {
	if (d->value_numvec(v))
	    return true;
	errh->error("%s not an array of numbers", name);
	v.clear();
    } else if (mandatory)
	errh->error("%s not defined", name);
    return false;
}

static bool
get_integer(Type1Font *font, int dictionary, const char *name,
	    int &v, ErrorHandler *errh, bool mandatory = false)
{
    Type1Definition *d = font->dict(dictionary, name);
    double scratch;
    if (d && d->value_int(v))
	return true;
    else if (d && d->value_num(scratch)) {
	errh->warning("%s not an integer", name);
	v = (int)scratch;
	return true;
    } else if (d)
	errh->error("%s not a number", name);
    else if (mandatory)
	errh->error("%s not defined", name);
    return false;
}

// BLUES

static void
check_blue_array(Vector<double> &blues, const char *name, double BlueScale,
		 ErrorHandler *errh)
{
  if (blues.size() % 2 != 0) {
    errh->error("%s has an odd number of entries", name);
    blues.push_back(blues.back());
  }

  for (int i = 0; i < blues.size(); i++)
    if (blues[i] != (double)((int)blues[i])) {
      errh->warning("some %s entries are not integers", name);
      break;
    }

  double max_diff = 1 / BlueScale;
  for (int i = 0; i < blues.size(); i += 2) {
    if (blues[i] > blues[i+1])
      errh->error("%s zone %d in the wrong order", name, i/2);
    else if (blues[i+1] - blues[i] >= max_diff)
	errh->error("%s zone %d too large in relation to BlueScale (size %g, max %g [%g])", name, i/2, blues[i+1] - blues[i], max_diff, BlueScale);
  }
}

static void
check_blue_overlap(Vector<double> &bl1, const char *name1,
		   Vector<double> &bl2, const char *name2, int BlueFuzz,
		   ErrorHandler *errh)
{
  int min_fuzz = 2*BlueFuzz + 1;
  for (int i = 2; i < bl1.size(); i += 2) {
    int thresh = (&bl1 == &bl2 ? i : bl2.size());
    for (int j = 0; j < thresh; j += 2) {
      if ((bl2[j] >= bl1[i] && bl2[j] <= bl1[i+1])
	  || (bl2[j+1] >= bl1[i] && bl2[j+1] <= bl1[i+1]))
	errh->error("%s zone %d and %s zone %d overlap",
		    name1, i/2, name2, j/2);
      else if ((bl2[j] >= bl1[i+1] && bl2[j] < bl1[i+1]+min_fuzz)
	       || (bl2[j+1] <= bl1[i] && bl2[j+1]+min_fuzz > bl1[i]))
	errh->error("%s zone %d and %s zone %d overlap within BlueFuzz",
		    name1, i/2, name2, j/2);
    }
  }
}

static void
check_blues(Type1Font *font, ErrorHandler *errh)
{
  Type1Definition *d;

  // BlueScale
  double BlueScale;
  bool ok = false;
  if ((d = font->p_dict("BlueScale"))) {
    if (!d->value_num(BlueScale))
      errh->error("BlueScale not a real number");
    else if (BlueScale <= 0)
      errh->error("BlueScale less than or equal to 0");
    else {
      if (BlueScale > 0.5)
	errh->error("suspiciously large BlueScale (%g)", BlueScale);
      ok = true;
    }
  }
  if (!ok) BlueScale = 0.039625;

  // BlueShift
  int BlueShift = -1;
  if (get_integer(font, Type1Font::dP, "BlueShift", BlueShift, errh)
      && BlueShift < 0)
    errh->error("BlueShift less than 0");
  if (BlueShift < 0)
    BlueShift = 7;

  // BlueFuzz
  int BlueFuzz = -1;
  if (get_integer(font, Type1Font::dP, "BlueFuzz", BlueFuzz, errh)
      && BlueFuzz < 0)
    errh->error("BlueFuzz less than 0");
  if (BlueFuzz < 0)
    BlueFuzz = 1;
  else if (BlueFuzz > 10)
    errh->warning("suspiciously large BlueFuzz (%d)", BlueFuzz);

  // BlueValues
  Vector<double> BlueValues, OtherBlues;
  get_num_array(font, Type1Font::dP, "BlueValues", BlueValues, errh, true);
  get_num_array(font, Type1Font::dP, "OtherBlues", OtherBlues, errh);

  check_blue_array(BlueValues, "BlueValues", BlueScale, errh);
  if (BlueValues.size() > 14)
    errh->error("too many zones in BlueValues (max 7)");
  check_blue_array(OtherBlues, "OtherBlues", BlueScale, errh);
  if (OtherBlues.size() > 10)
    errh->error("too many zones in OtherBlues (max 5)");

  check_blue_overlap(BlueValues, "BlueValues", BlueValues, "BlueValues", BlueFuzz, errh);
  check_blue_overlap(OtherBlues, "OtherBlues", OtherBlues, "OtherBlues", BlueFuzz, errh);
  check_blue_overlap(BlueValues, "BlueValues", OtherBlues, "OtherBlues", BlueFuzz, errh);

  // FamilyBlues
  Vector<double> FamilyBlues, FamilyOtherBlues;
  get_num_array(font, Type1Font::dP, "FamilyBlues", FamilyBlues, errh);
  get_num_array(font, Type1Font::dP, "FamilyOtherBlues", FamilyOtherBlues, errh);

  check_blue_array(FamilyBlues, "FamilyBlues", BlueScale, errh);
  if (FamilyBlues.size() > 14)
    errh->error("too many zones in FamilyBlues (max 7)");
  check_blue_array(FamilyOtherBlues, "FamilyOtherBlues", BlueScale, errh);
  if (FamilyOtherBlues.size() > 10)
    errh->error("too many zones in FamilyOtherBlues (max 5)");

  check_blue_overlap(FamilyBlues, "FamilyBlues", FamilyBlues, "FamilyBlues", BlueFuzz, errh);
  check_blue_overlap(FamilyOtherBlues, "FamilyOtherBlues", FamilyOtherBlues, "FamilyOtherBlues", BlueFuzz, errh);
  check_blue_overlap(FamilyBlues, "FamilyBlues", FamilyOtherBlues, "FamilyOtherBlues", BlueFuzz, errh);
}


// STEMS

void
check_stem_snap(Vector<double> &stem_snap, double main, bool is_v,
		ErrorHandler *errh)
{
  const char *name = (is_v ? "V" : "H");
  if (stem_snap.size() > 12)
    errh->error("StemSnap%s has more than 12 entries", name);
  for (int i = 0; i < stem_snap.size() - 1; i++)
    if (stem_snap[i] >= stem_snap[i+1]) {
      errh->error("StemSnap%s is not sorted in increasing order", name);
      break;
    }
  for (int i = 0; i < stem_snap.size(); i++)
    if (stem_snap[i] == main)
      return;
  if (main >= 0)
    errh->warning("Std%sW not in StemSnap%s array", name, name);
}

void
check_stems(Type1Font *font, ErrorHandler *errh)
{
  Vector<double> StdHW, StdVW, StemSnapH, StemSnapV;
  if (get_num_array(font, Type1Font::dP, "StdHW", StdHW, errh)) {
    if (StdHW.size() != 1)
      errh->error("StdHW has %d entries (exactly one required)", StdHW.size());
    if (StdHW.size() > 0 && StdHW[0] <= 0)
      errh->error("StdHW entry less than or equal to 0");
  }
  if (get_num_array(font, Type1Font::dP, "StdVW", StdVW, errh)) {
    if (StdVW.size() != 1)
      errh->error("StdVW has %d entries (exactly one required)", StdVW.size());
    if (StdVW.size() > 0 && StdVW[0] <= 0)
      errh->error("StdVW entry less than or equal to 0");
  }

  if (get_num_array(font, Type1Font::dP, "StemSnapH", StemSnapH, errh))
    check_stem_snap(StemSnapH, StdHW.size() ? StdHW[0] : -1000, false, errh);
  if (get_num_array(font, Type1Font::dP, "StemSnapV", StemSnapV, errh))
    check_stem_snap(StemSnapV, StdVW.size() ? StdVW[0] : -1000, true, errh);
}


// MAIN

static void
do_file(const char *filename, PsresDatabase *psres, ErrorHandler *errh,
	ErrorHandler *err_errh)
{
  FILE *f;
  if (strcmp(filename, "-") == 0) {
    f = stdin;
    filename = "<stdin>";
#if defined(_MSDOS) || defined(_WIN32)
    _setmode(_fileno(f), _O_BINARY);
#endif
  } else
    f = fopen(filename, "rb");

  if (!f) {
    // check for PostScript name
    Filename fn = psres->filename_value("FontOutline", filename);
    f = fn.open_read();
  }

  if (!f)
    err_errh->fatal("%s: %s", filename, strerror(errno));

  Type1Reader *reader;
  int c = getc(f);
  ungetc(c, f);
  if (c == EOF)
    err_errh->fatal("%s: empty file", filename);
  if (c == 128)
    reader = new Type1PFBReader(f);
  else
    reader = new Type1PFAReader(f);

  Type1Font *font = new Type1Font(*reader);

  if (font) {
    LandmarkErrorHandler cerrh(errh, filename);

    // check UniqueID values
    int UniqueID = -1, UniqueID2 = -1;
    if (get_integer(font, Type1Font::dF, "UniqueID", UniqueID, errh)
	&& (UniqueID < 0 || UniqueID > 0xFFFFFF))
	cerrh.error("UniqueID not in the range 0-16777215");
    if (get_integer(font, Type1Font::dP, "UniqueID", UniqueID2, errh)
	&& (UniqueID2 < 0 || UniqueID2 > 0xFFFFFF))
	cerrh.error("Private UniqueID not in the range 0-16777215");
    if (UniqueID >= 0 && UniqueID2 >= 0 && UniqueID != UniqueID2)
	cerrh.error("Private UniqueID does not equal font UniqueID");

    // check XUID values
    Vector<double> XUID;
    if (get_num_array(font, Type1Font::dF, "XUID", XUID, errh)) {
	if (XUID.size() == 0)
	    cerrh.error("empty XUID");
	for (int i = 0; i < XUID.size(); i++) {
	    if (floor(XUID[i]) != XUID[i] || XUID[i] < 0 || XUID[i] > 0xFFFFFF) {
		cerrh.error("element of XUID not an integer in the range 0-16777215");
		break;
	    }
	}
    }

    check_blues(font, &cerrh);
    check_stems(font, &cerrh);

    MultipleMasterSpace *mmspace = font->create_mmspace(&cerrh);
    Vector<double> weight_vector;
    if (mmspace)
	weight_vector = mmspace->default_weight_vector();
    int gc = font->nglyphs();
    CharstringChecker cc(weight_vector);

    for (int i = 0; i < gc; i++) {
      ContextErrorHandler derrh
	  (&cerrh, "While interpreting %<%s%>:", font->glyph_name(i).c_str());
      cc.check(font->glyph_context(i), &derrh);
    }

    int ns = font->nsubrs();
    CharstringSubrChecker csc(weight_vector);
    for (int i = 0; i < ns; ++i)
	if (Type1Charstring *cs = font->subr(i)) {
	    ContextErrorHandler derrh(&cerrh, "While interpreting subr %d:", i);
	    CharstringContext cctx(font, cs);
	    csc.check(cctx, &derrh);
	}
  }

  delete font;
  delete reader;
}

int
main(int argc, char *argv[])
{
  PsresDatabase *psres = new PsresDatabase;
  psres->add_psres_path(getenv("PSRESOURCEPATH"), 0, false);

  Clp_Parser *clp =
    Clp_NewParser(argc, (const char * const *)argv, sizeof(options) / sizeof(options[0]), options);
  program_name = Clp_ProgramName(clp);

  ErrorHandler *err_errh = ErrorHandler::static_initialize(new FileErrorHandler(stderr));
  ErrorHandler *out_errh = new FileErrorHandler(stdout);
  ErrorHandler *errh = out_errh;
  int nfiles = 0;

  while (1) {
    int opt = Clp_Next(clp);
    switch (opt) {

     case QUIET_OPT:
       if (clp->negated)
	   errh = out_errh;
       else
	   errh = new SilentErrorHandler;
       break;

     case VERSION_OPT:
      printf("t1lint (LCDF typetools) %s\n", VERSION);
      printf("Copyright (C) 1999-2019 Eddie Kohler\n\
This is free software; see the source for copying conditions.\n\
There is NO warranty, not even for merchantability or fitness for a\n\
particular purpose.\n");
      exit(0);
      break;

     case HELP_OPT:
      usage();
      exit(0);
      break;

     case Clp_NotOption:
      do_file(clp->vstr, psres, errh, err_errh);
      nfiles++;
      break;

     case Clp_Done:
      goto done;

     case Clp_BadOption:
      usage_error(err_errh, 0);
      break;

     default:
      break;

    }
  }

 done:
  if (nfiles == 0)
      do_file("-", psres, errh, err_errh);

  return (errh->nerrors() == 0 ? 0 : 1);
}
