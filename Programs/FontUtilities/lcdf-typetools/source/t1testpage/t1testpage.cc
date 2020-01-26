/* t1testpage.cc -- driver for generating Type 1 fonts' test pages
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
#include <efont/t1interp.hh>
#include <efont/t1bounds.hh>
#include <lcdf/clp.h>
#include <lcdf/error.hh>
#include <lcdf/globmatch.hh>
#include <lcdf/transform.hh>
#include <algorithm>
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
#define OUTPUT_OPT	303
#define GLYPH_OPT	304
#define SMOKE_OPT	305

const Clp_Option options[] = {
    { "help", 'h', HELP_OPT, 0, 0 },
    { "output", 'o', OUTPUT_OPT, Clp_ValString, 0 },
    { "version", 0, VERSION_OPT, 0, 0 },
    { "glyph", 'g', GLYPH_OPT, Clp_ValString, 0 },
    { "smoke", 's', SMOKE_OPT, 0, Clp_Negate }
};


static const char *program_name;
static PermString::Initializer initializer;
static HashMap<PermString, int> glyph_order(-1);


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
%<T1testpage%> creates a PostScript proof document for the specified Type 1\n\
font file and writes it to the standard output. The proof shows every\n\
glyph in the font, including its glyph name and encoding.\n\
\n\
Usage: %s [OPTION]... [FONT...]\n\
\n\
Each FONT is the name of a PFA or PFB font file. If omitted, t1testpage will\n\
read a font file from the standard input.\n\
\n\
Options:\n\
  -g, --glyph=GLYPH            Limit output to one or more GLYPHs.\n\
  -s, --smoke                  Print smoke proofs, one character per page.\n\
  -o, --output=FILE            Write output to FILE instead of standard out.\n\
  -h, --help                   Print this message and exit.\n\
      --version                Print version number and exit.\n\
\n\
Report bugs to <ekohler@gmail.com>.\n", program_name);
}


// OUTPUTTER

namespace {

class Testpager { public:
    Testpager(FILE *f)
	: _f(f), _pageno(1) {
    }
    virtual ~Testpager() {
	fclose(_f);
    }
    virtual void prolog(const Vector<Type1Font *> &fonts) = 0;
    void newpage() {
	fprintf(_f, "%%%%Page: %d %d\n", _pageno, _pageno);
	++_pageno;
    }
    virtual void font(Type1Font *font, const Vector<PermString>& glyph_names) = 0;
    virtual void epilog() {
	fprintf(_f, "%%%%EOF\n");
    }
  protected:
    FILE *_f;
    int _pageno;
};

class GridTestpager : public Testpager { public:
    GridTestpager(FILE *f)
	: Testpager(f) {
    }
    void prolog(const Vector<Type1Font *> &fonts);
    void font(Type1Font *font, const Vector<PermString>& glyph_names);
};

class SmokeTestpager : public Testpager { public:
    SmokeTestpager(FILE *f)
	: Testpager(f) {
    }
    void prolog(const Vector<Type1Font *> &fonts);
    void font(Type1Font *font, const Vector<PermString>& glyph_names);
};

}


// MAIN

static Type1Font *
do_file(const char *filename, PsresDatabase *psres, ErrorHandler *errh)
{
    FILE *f;
    if (!filename || strcmp(filename, "-") == 0) {
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
	errh->fatal("%s: %s", filename, strerror(errno));

    Type1Reader *reader;
    int c = getc(f);
    ungetc(c, f);
    if (c == EOF)
	errh->fatal("%s: empty file", filename);
    if (c == 128)
	reader = new Type1PFBReader(f);
    else
	reader = new Type1PFAReader(f);

    Type1Font *font = new Type1Font(*reader);

    delete reader;
    return font;
}

void
GridTestpager::prolog(const Vector<Type1Font *> &fonts)
{
    fprintf(_f, "%%!PS-Adobe-3.0\n\
%%%%LanguageLevel: 2\n\
%%%%DocumentMedia: Plain 612 792 white ( )\n\
%%%%BeginProlog\n");
    fprintf(_f, "/magicstr 1 string def\n\
/magicbox { %% row col char name encoding  magicbox  -\n\
  5 3 roll 54 mul 36 add exch 54 mul neg 682 add moveto currentpoint\n\
  .8 setgray 54 0 rlineto 0 54 rlineto -54 0 rlineto closepath stroke\n\
  0 setgray moveto\n\
  gsave /Helvetica 7 selectfont 3 1.5 rmoveto show grestore\n\
  gsave /Helvetica 7 selectfont 3 45.5 rmoveto show grestore\n\
  magicstr 0 3 -1 roll put\n\
  magicstr stringwidth pop 54 sub -2 div 16 rmoveto magicstr show\n\
} bind def\n");
    Type1PFAWriter w(_f);
    for (Vector<Type1Font *>::const_iterator it = fonts.begin();
	 it != fonts.end(); ++it)
	(*it)->write(w);
    fprintf(_f, "%%%%EndProlog\n");
}

void
GridTestpager::font(Type1Font* font, const Vector<PermString>& glyph_names)
{
    HashMap<PermString, int> encodings(-1);
    if (Type1Encoding *encoding = font->type1_encoding())
	for (int i = 255; i >= 0; i--)
	    if (encoding->elt(i))
		encodings.insert(encoding->elt(i), i);

    int per_row = 10;
    int nrows = 13;
    int per_page = nrows * per_row;

    int page = 0, gi = -1;
    for (Vector<PermString>::const_iterator it = glyph_names.begin();
	 it != glyph_names.end(); ++it) {
	// allow font that doesn't have all glyphs
	if (!font->glyph(*it))
	    continue;

	++gi;
	if (gi % per_page == 0) {
	    if (page)
		fprintf(_f, "showpage restore\n");
	    ++page;
	    newpage();
	    fprintf(_f, "save\n");
	    // make new font
	    fprintf(_f, "/%s findfont dup length dict begin\n\
 { 1 index /FID ne {def} {pop pop} ifelse } forall\n /Encoding [",
		    font->font_name().c_str());
	    int gx = 0;
	    for (Vector<PermString>::const_iterator xit = it;
		 xit != glyph_names.end() && gx < per_page; ++xit)
		if (font->glyph(*xit)) {
		    ++gx;
		    fprintf(_f, " /%s", xit->c_str());
		    if (gx % 10 == 9)
			fprintf(_f, "\n");
		}
	    fprintf(_f, " ] def\n currentdict end /X exch definefont pop\n\
/Helvetica-Bold 16 selectfont 36 742 moveto (%s) show\n\
/X 24 selectfont\n", font->font_name().c_str());
	}

	int row = (gi % per_page) / per_row;
	int col = gi % per_row;

	fprintf(_f, "%d %d %d (%s)", row, col, gi % per_page, it->c_str());
	if (encodings[*it] >= 0) {
	    int e = encodings[*it];
	    if (e == '\\')
		fprintf(_f, " ('\\\\\\\\')");
	    else if (e == '\'')
		fprintf(_f, " ('\\\\'')");
	    else if (e == '(' || e == ')')
		fprintf(_f, " ('\\%c')", e);
	    else if (e >= 32 && e < 127)
		fprintf(_f, " ('%c')", e);
	    else
		fprintf(_f, " ('\\\\%03o')", e);
	} else
	    fprintf(_f, " ()");
	fprintf(_f, " magicbox\n");
    }

    if (page)
	fprintf(_f, "showpage restore\n");
}



/*****
 * SMOKE PROOFS
 **/

inline StringAccum&
operator<<(StringAccum& sa, const Point& p)
{
    return sa << p.x << ' ' << p.y;
}

class Smoker : public CharstringInterp { public:

    Smoker(const Transform&);

    void act_line(int, const Point &, const Point &);
    void act_curve(int, const Point &, const Point &, const Point &, const Point &);
    void act_closepath(int);

    String char_postscript()		{ return _char_sa.take_string(); }
    String points_postscript()		{ return _points_sa.take_string(); }

    bool run(const CharstringContext&);

  private:

    Transform _xform;
    StringAccum _char_sa;
    StringAccum _points_sa;
    Point _char_cp;

    inline void maybe_move(const Point&);

};

Smoker::Smoker(const Transform& xform)
    : _xform(xform)
{
}

void
Smoker::maybe_move(const Point& p)
{
    if (_char_cp.x < -100000 || _char_cp != p) {
	_char_sa << (p * _xform) << " moveto\n";
	_points_sa << (p * _xform) << " pA\n";
    }
}

void
Smoker::act_line(int cmd, const Point& p0, const Point& p1)
{
    Point xp1 = p1 * _xform;

    maybe_move(p0);
    _char_sa << xp1 << " lineto\n";
    if (cmd == Charstring::cRlineto
	|| cmd == Charstring::cHlineto
	|| cmd == Charstring::cVlineto)
	_points_sa << xp1 << " pA\n";
    _char_cp = p1;
}

void
Smoker::act_curve(int, const Point &p0, const Point &p1, const Point &p2, const Point &p3)
{
    Point xp1 = p1 * _xform;
    Point xp2 = p2 * _xform;
    Point xp3 = p3 * _xform;

    maybe_move(p0);
    _char_sa << xp1 << ' ' << xp2 << ' ' << xp3 << " curveto\n";
    _points_sa << xp1 << " pC " << xp2 << " pC " << xp3 << " pA\n";
    _char_cp = p3;
}

void
Smoker::act_closepath(int)
{
    _char_sa << "closepath\n";
}

bool
Smoker::run(const CharstringContext& g)
{
    _char_sa.clear();
    _points_sa.clear();
    _char_cp.x = -200000;
    return CharstringInterp::interpret(g);
}

#define LEFT_BOUND 72.
#define RIGHT_BOUND (7.5 * 72)
#define BOTTOM_BOUND 72.
#define TOP_BOUND (7.5 * 72)

static Transform
bounds2xform(CharstringBounds& bounds, bool expand = false)
{
    if (expand) {
	Point vec = bounds.bb_top_right() - bounds.bb_bottom_left();
	bounds.act_line(0, bounds.bb_bottom_left() - 0.1 * vec, bounds.bb_top_right() + 0.1 * vec);
    }
    bounds.act_line(0, Point(0, 0), bounds.width());
    bounds.act_line(0, Point(0, 0), Point(1, 1));

    double true_width = std::max(bounds.bb_right(), 0.) - std::min(bounds.bb_left(), 0.);
    double true_height = std::max(bounds.bb_top(), 0.) - std::min(bounds.bb_bottom(), 0.);
    double x_scale = (RIGHT_BOUND - LEFT_BOUND) / true_width;
    double y_scale = (TOP_BOUND - BOTTOM_BOUND) / true_height;
    double scale = std::min(x_scale, y_scale);
    double origin_x = LEFT_BOUND;
    if (bounds.bb_left() < 0)
	origin_x += -bounds.bb_left() * scale;
    double origin_y = BOTTOM_BOUND;
    if (bounds.bb_bottom() < 0)
	origin_y += -bounds.bb_bottom() * scale;
    return Transform(scale, 0, 0, scale, origin_x, origin_y);
}

static const char* const bounds_glyphs[] = {
    "A", "Eacute", "Ecircumflex", "l", "g", "p", "q", "j", "J", "emdash", 0
};

void
SmokeTestpager::prolog(const Vector<Type1Font *> &)
{
    fprintf(_f, "%%!PS-Adobe-3.0\n%%%%LanguageLevel: 2\n%%%%BeginProlog\n");
    fprintf(_f, "/pA { %% x y  pA  -\n\
  moveto\n\
  -2 -2 rmoveto 4 0 rlineto 0 4 rlineto -4 0 rlineto closepath\n\
  gsave 1 setgray stroke grestore 0 setgray fill\n\
} bind def\n\
/pC { %% x y  pC  -\n\
  2 copy moveto 2 0 rmoveto 2 0 360 arc\n\
  gsave 1 setgray stroke grestore 0.1 setgray fill\n\
} bind def\n\
");
    fprintf(_f, "%%%%EndProlog\n");
}

void
SmokeTestpager::font(Type1Font* font, const Vector<PermString>& glyph_names)
{
    HashMap<PermString, int> encodings(-1);
    if (Type1Encoding *encoding = font->type1_encoding())
	for (int i = 255; i >= 0; i--)
	    encodings.insert(encoding->elt(i), i);

    // First, calculate font bounds, and from there, the transform.
    CharstringBounds bounds;
    for (const char* const* bg = bounds_glyphs; *bg; bg++)
	if (CharstringContext cc = font->glyph_context(*bg))
	    bounds.char_bounds(cc, false);
    if (bounds.bb_left() == bounds.bb_right())
	for (int gi = 0; gi < glyph_names.size(); gi++)
	    bounds.char_bounds(font->glyph_context(glyph_names[gi]), false);
    Transform font_xform = bounds2xform(bounds, true);

    for (int gi = 0; gi < glyph_names.size(); gi++) {
	// allow font that doesn't have all glyphs
	if (!font->glyph(glyph_names[gi]))
	    continue;

	newpage();
	fprintf(_f, "save\n");

	CharstringContext cc = font->glyph_context(glyph_names[gi]);
	bounds.clear();
	bounds.char_bounds(cc, false);

	Transform xform(font_xform);
	if (bounds.bb_known()) {
	    Point lb = bounds.bb_bottom_left() * xform;
	    Point rt = bounds.bb_top_right() * xform;
	    Point wd = bounds.width() * xform;
	    if (lb.x < LEFT_BOUND || lb.y < BOTTOM_BOUND
		|| rt.x > RIGHT_BOUND || rt.y > TOP_BOUND
		|| wd.x < LEFT_BOUND || wd.x > RIGHT_BOUND
		|| wd.y < BOTTOM_BOUND || wd.y > TOP_BOUND)
		xform = bounds2xform(bounds);
	}

	Smoker smoker(xform);
	smoker.run(font->glyph_context(glyph_names[gi]));
	fprintf(_f, "%g %g moveto 0 %g rlineto %g %g moveto 0 %g rlineto %g %g moveto %g 0 rlineto 0 setgray stroke\n",
		(Point(0, 0) * xform).x, BOTTOM_BOUND - 36, TOP_BOUND - BOTTOM_BOUND + 72,
		(bounds.width() * xform).x, BOTTOM_BOUND - 36, TOP_BOUND - BOTTOM_BOUND + 72,
		LEFT_BOUND - 36, (Point(0, 0) * xform).y, RIGHT_BOUND - LEFT_BOUND + 72);

	fprintf(_f, "%s\n0.5 setgray fill\n\n1 setlinewidth\n%s\nshowpage\nrestore\n", smoker.char_postscript().c_str(), smoker.points_postscript().c_str());
    }
}


/*****
 * MAIN PROGRAM
 **/

int
click_strcmp(PermString a, PermString b)
{
    const char *ad = a.c_str(), *ae = a.c_str() + a.length();
    const char *bd = b.c_str(), *be = b.c_str() + b.length();

    while (ad < ae && bd < be) {
	if (isdigit((unsigned char) *ad) && isdigit((unsigned char) *bd)) {
	    // compare the two numbers, but don't treat them as numbers in
	    // case of overflow
	    // first, skip initial '0's
	    const char *iad = ad, *ibd = bd;
	    while (ad < ae && *ad == '0')
		ad++;
	    while (bd < be && *bd == '0')
		bd++;
	    int longer_zeros = (ad - iad) - (bd - ibd);
	    // skip to end of number
	    const char *nad = ad, *nbd = bd;
	    while (ad < ae && isdigit((unsigned char) *ad))
		ad++;
	    while (bd < be && isdigit((unsigned char) *bd))
		bd++;
	    // longer number must be larger
	    if ((ad - nad) != (bd - nbd))
		return (ad - nad) - (bd - nbd);
	    // otherwise, compare numbers with the same length
	    for (; nad < ad && nbd < bd; nad++, nbd++)
		if (*nad != *nbd)
		    return *nad - *nbd;
	    // finally, longer string of initial '0's wins
	    if (longer_zeros != 0)
		return longer_zeros;
	} else if (isdigit((unsigned char) *ad))
	    return (isalpha((unsigned char) *bd) ? -1 : 1);
	else if (isdigit((unsigned char) *bd))
	    return (isalpha((unsigned char) *ad) ? 1 : -1);
	else {
	    int d = tolower((unsigned char) *ad) - tolower((unsigned char) *bd);
	    if (d != 0)
		return d;
	    ad++;
	    bd++;
	}
    }

    if ((ae - ad) != (be - bd))
	return (ae - ad) - (be - bd);
    else {
	assert(a.length() == b.length());
	return memcmp(a.c_str(), b.c_str(), a.length());
    }
}

extern "C" {
static int CDECL
glyphcompare(const void *lv, const void *rv)
{
    const PermString* ln = (const PermString*)lv;
    const PermString* rn = (const PermString*)rv;

    // try first without the '.'s
    const char* ldot = strchr(ln->c_str(), '.');
    const char* rdot = strchr(rn->c_str(), '.');
    if (ldot == ln->begin())
	ldot = 0;
    if (rdot == rn->begin())
	rdot = 0;
    if (ldot || rdot) {
	PermString l(ln->begin(), ldot ? ldot : ln->end());
	PermString r(rn->begin(), rdot ? rdot : rn->end());
	int diff = glyphcompare(&l, &r);
	if (diff != 0)
	    return diff;
    }

    int lorder = glyph_order[*ln];
    int rorder = glyph_order[*rn];
    if (lorder >= 0 && rorder >= 0)
	return lorder - rorder;
    else if (lorder >= 0)
	return -1;
    else if (rorder >= 0)
	return 1;
    else
	return click_strcmp(*ln, *rn);
}
}

static bool
glyph_matches(const String& glyph_name, const String* pattern_begin, const String* pattern_end)
{
    if (pattern_begin >= pattern_end)
	return true;
    for (; pattern_begin < pattern_end; pattern_begin++)
	if (glob_match(glyph_name, *pattern_begin))
	    return true;
    return false;
}

int
main(int argc, char *argv[])
{
    PsresDatabase *psres = new PsresDatabase;
    psres->add_psres_path(getenv("PSRESOURCEPATH"), 0, false);

    Clp_Parser *clp =
	Clp_NewParser(argc, (const char * const *)argv, sizeof(options) / sizeof(options[0]), options);
    program_name = Clp_ProgramName(clp);

    ErrorHandler *errh = ErrorHandler::static_initialize(new FileErrorHandler(stderr));
    const char *output_file = 0;
    Vector<String> glyph_patterns;
    bool smoke = false;
    Vector<Type1Font *> fonts;

    while (1) {
	int opt = Clp_Next(clp);
	switch (opt) {

	  case GLYPH_OPT: {
	      const char* s = clp->vstr, *end = s + strlen(s);
	      while (s < end) {
		  while (s < end && isspace((unsigned char) *s))
		      s++;
		  const char* word = s;
		  while (s < end && !isspace((unsigned char) *s))
		      s++;
		  if (word < s)
		      glyph_patterns.push_back(String(word, s - word));
	      }
	      break;
	  }

	  case SMOKE_OPT:
	    smoke = !clp->negated;
	    break;

	  case OUTPUT_OPT:
	    if (output_file)
		errh->fatal("output file already specified");
	    output_file = clp->vstr;
	    break;

	  case VERSION_OPT:
	    printf("t1testpage (LCDF typetools) %s\n", VERSION);
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
	    fonts.push_back(do_file(clp->vstr, psres, errh));
	    break;

	  case Clp_Done:
	    goto done;

	  case Clp_BadOption:
	    usage_error(errh, 0);
	    break;

	  default:
	    break;

	}
    }

  done:
    if (!fonts.size())
	fonts.push_back(do_file(0, psres, errh));

    FILE *outf;
    if (!output_file || strcmp(output_file, "-") == 0)
	outf = stdout;
    else {
	outf = fopen(output_file, "w");
	if (!outf)
	    errh->fatal("%s: %s", output_file, strerror(errno));
    }

    //font->undo_synthetic();

    // Prepare glyph order table
    int gindex = 0;
    char buf[7] = "Asmall";
    for (int c = 0; c < 26; c++) {
	buf[0] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[c];
	glyph_order.insert(PermString(buf[0]), gindex++);
	glyph_order.insert(PermString("abcdefghijklmnopqrstuvwxyz"[c]), gindex++);
	glyph_order.insert(PermString(buf), gindex++);
    }
    glyph_order.insert("parenleft", gindex++);
    glyph_order.insert("period", gindex++);
    glyph_order.insert("comma", gindex++);
    glyph_order.insert("hyphen", gindex++);
    glyph_order.insert("ampersand", gindex++);
    glyph_order.insert("semicolon", gindex++);
    glyph_order.insert("exclamation", gindex++);
    glyph_order.insert("question", gindex++);
    glyph_order.insert("parenright", gindex++);
    glyph_order.insert("zero", gindex++);
    glyph_order.insert("one", gindex++);
    glyph_order.insert("two", gindex++);
    glyph_order.insert("three", gindex++);
    glyph_order.insert("four", gindex++);
    glyph_order.insert("five", gindex++);
    glyph_order.insert("six", gindex++);
    glyph_order.insert("seven", gindex++);
    glyph_order.insert("eight", gindex++);
    glyph_order.insert("nine", gindex++);
    glyph_order.insert("zerooldstyle", gindex++);
    glyph_order.insert("oneoldstyle", gindex++);
    glyph_order.insert("twooldstyle", gindex++);
    glyph_order.insert("threeoldstyle", gindex++);
    glyph_order.insert("fouroldstyle", gindex++);
    glyph_order.insert("fiveoldstyle", gindex++);
    glyph_order.insert("sixoldstyle", gindex++);
    glyph_order.insert("sevenoldstyle", gindex++);
    glyph_order.insert("eightoldstyle", gindex++);
    glyph_order.insert("nineoldstyle", gindex++);
    glyph_order.insert(".notdef", gindex++);
    glyph_order.insert("space", gindex++);

    // Get glyph names.
    HashMap<PermString, int> glyph_hash(0);
    for (Vector<Type1Font *>::iterator it = fonts.begin(); it != fonts.end();
	 ++it)
	for (int i = 0; i < (*it)->nglyphs(); i++) {
	    if (glyph_matches((*it)->glyph_name(i),
			      glyph_patterns.begin(), glyph_patterns.end()))
		glyph_hash.insert((*it)->glyph_name(i), 1);
	}

    Vector<PermString> glyph_names;
    for (HashMap<PermString, int>::iterator it = glyph_hash.begin();
	 it != glyph_hash.end(); ++it)
	glyph_names.push_back(it.key());
    if (glyph_names.size() == 0)
	errh->fatal("no glyphs to print");
    qsort(glyph_names.begin(), glyph_names.size(), sizeof(PermString), glyphcompare);

    // outputs
    Testpager *tp;
    if (smoke)
	tp = new SmokeTestpager(outf);
    else
	tp = new GridTestpager(outf);

    tp->prolog(fonts);
    for (Vector<Type1Font *>::iterator it = fonts.begin(); it != fonts.end();
	 ++it)
	tp->font(*it, glyph_names);
    tp->epilog();

    delete tp;
    exit(0);
}
