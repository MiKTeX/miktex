/* t1dotlessj.cc -- driver for creating dotlessj characters from Type 1 fonts
 *
 * Copyright (c) 2003-2019 Eddie Kohler
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
#include <efont/t1csgen.hh>
#include <efont/t1bounds.hh>
#include <efont/t1unparser.hh>
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
#if defined(MIKTEX)
#define exit(x) throw(x)
#endif

// also see otftotfm/automatic.cc
enum { EXIT_NORMAL = 0, EXIT_DOTLESSJ_EXISTS = 1, EXIT_J_NODOT = 2,
       EXIT_NO_J = 3, EXIT_ERROR = 4 };

using namespace Efont;

#define VERSION_OPT	301
#define HELP_OPT	302
#define QUIET_OPT	303
#define PFA_OPT		304
#define PFB_OPT		305
#define OUTPUT_OPT	306
#define NAME_OPT	307

const Clp_Option options[] = {
    { "help", 'h', HELP_OPT, 0, 0 },
    { "name", 'n', NAME_OPT, Clp_ValString, 0 },
    { "output", 'o', OUTPUT_OPT, Clp_ValString, 0 },
    { "pfa", 'a', PFA_OPT, 0, 0 },
    { "pfb", 'b', PFB_OPT, 0, 0 },
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
	errh->message("Usage: %s [OPTIONS] [FONTFILE [OUTPUTFILE]]", program_name);
    else
	errh->xmessage(ErrorHandler::e_error, error_message, val);
    errh->message("Type %s --help for more information.", program_name);
    exit(EXIT_ERROR);
}

void
usage()
{
    FileErrorHandler uerrh(stdout);
    uerrh.message("\
%<T1dotlessj%> reads a PostScript Type 1 font, derives a new PostScript Type 1\n\
font containing just a dotlessj character (by chopping the dot from the j),\n\
and writes it to the standard output.\n\
\n\
Usage: %s [OPTIONS] [FONTFILE [OUTPUTFILE]]\n\
\n\
Options:\n\
  -a, --pfa                    Output PFA font.\n\
  -b, --pfb                    Output PFB font. This is the default.\n\
  -o, --output=FILE            Write output to FILE instead of standard output.\n\
  -n, --name=NAME              Set output font%,s PostScript name.\n\
  -h, --help                   Print this message and exit.\n\
  -q, --quiet                  Do not report errors to standard error.\n\
      --version                Print version number and exit.\n\
\n\
Report bugs to <ekohler@gmail.com>.\n", program_name);
}


// Sectioner

class Sectioner : public Type1CharstringGenInterp { public:

    Sectioner(int precision);

    void act_line(int, const Point &, const Point &);
    void act_curve(int, const Point &, const Point &, const Point &, const Point &);
    void act_closepath(int);
    void act_flex(int, const Point &, const Point &, const Point &, const Point &, const Point &, const Point &, const Point &, double);

    void run(const CharstringContext &g);
    void undot(PermString, ErrorHandler *);
    Type1Charstring gen(Type1Font *);

  private:

    CharstringBounds _boundser;

    Vector<String> _sections;
    Vector<int> _bounds;

    void append_bounds();

};

Sectioner::Sectioner(int precision)
    : Type1CharstringGenInterp(precision)
{
    set_direct_hint_replacement(true);
}

void
Sectioner::act_line(int cmd, const Point &p0, const Point &p1)
{
    Type1CharstringGenInterp::act_line(cmd, p0, p1);
    _boundser.act_line(cmd, p0, p1);
}

void
Sectioner::act_curve(int cmd, const Point &p0, const Point &p1, const Point &p2, const Point &p3)
{
    Type1CharstringGenInterp::act_curve(cmd, p0, p1, p2, p3);
    _boundser.act_curve(cmd, p0, p1, p2, p3);
}

void
Sectioner::act_flex(int cmd, const Point &p0, const Point &p1, const Point &p2, const Point &p3_4, const Point &p5, const Point &p6, const Point &p7, double flex_depth)
{
    Type1CharstringGenInterp::act_flex(cmd, p0, p1, p2, p3_4, p5, p6, p7, flex_depth);
    _boundser.act_flex(cmd, p0, p1, p2, p3_4, p5, p6, p7, flex_depth);
}

void Sectioner::append_bounds() {
    double bb[5];
    _boundser.output(bb, bb[4]);
    _bounds.push_back((int) floor(bb[0]));
    _bounds.push_back((int) floor(bb[1]));
    _bounds.push_back((int) ceil(bb[2]));
    _bounds.push_back((int) ceil(bb[3]));
}

void
Sectioner::act_closepath(int cmd)
{
    Type1CharstringGenInterp::act_closepath(cmd);
    Type1Charstring result;
    Type1CharstringGenInterp::intermediate_output(result);
    _sections.push_back(result.data_string());
    append_bounds();
    _boundser.clear();
}

void
Sectioner::run(const CharstringContext &g)
{
    _boundser.clear();
    Type1Charstring last_section;
    Type1CharstringGenInterp::run(g, last_section);
    _sections.push_back(last_section.data_string());
    append_bounds();
}

void
Sectioner::undot(PermString font_name, ErrorHandler *errh)
{
    //for (String *s = _sections.begin(); s < _sections.end(); s++)
    //    fprintf(stderr, "%d  %s\n", s - _sections.begin(), CharstringUnparser::unparse(Type1Charstring(*s)).c_str());

    if (_sections.size() < 3)
	errh->fatal("<%d>%s: %<j%> is already dotless", -EXIT_J_NODOT, font_name.c_str());

    int topmost = -1;
    for (int i = 0; i < _sections.size() - 1; i++)
	if (topmost < 0 || _bounds[i*4 + 1] > _bounds[topmost*4 + 1])
	    topmost = i;

    // check if any sections are below this
    for (int i = 0; i < _sections.size() - 1; i++)
	if (_bounds[i*4 + 1] < _bounds[topmost*4 + 1])
	    goto found_below;
    errh->fatal("<%d>%s: %<j%> is already dotless", -EXIT_J_NODOT, font_name.c_str());

  found_below:
    _sections[topmost] = String();
}

Type1Charstring
Sectioner::gen(Type1Font *font)
{
    StringAccum sa;
    for (String *s = _sections.begin(); s < _sections.end(); s++)
	sa << *s;
    Type1Charstring in(sa.take_string()), out;
    Type1CharstringGenInterp gen(precision());
    gen.set_hint_replacement_storage(font);
    gen.run(CharstringContext(program(), &in), out);
    return out;
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
	errh->fatal("<%d>%s: %s", -EXIT_ERROR, filename, strerror(errno));

    Type1Reader *reader;
    int c = getc(f);
    ungetc(c, f);
    if (c == EOF)
	errh->fatal("<%d>%s: empty file", -EXIT_ERROR, filename);
    if (c == 128)
	reader = new Type1PFBReader(f);
    else
	reader = new Type1PFAReader(f);

    Type1Font *font = new Type1Font(*reader);
    if (!font->ok())
	errh->fatal("<%d>%s: no glyphs in font", -EXIT_ERROR, filename);

    delete reader;
    return font;
}

int
#if defined(MIKTEX)
Main(int argc, char** argv)
#else
main(int argc, char *argv[])
#endif
{
    PsresDatabase *psres = new PsresDatabase;
    psres->add_psres_path(getenv("PSRESOURCEPATH"), 0, false);

    Clp_Parser *clp =
	Clp_NewParser(argc, (const char * const *)argv, sizeof(options) / sizeof(options[0]), options);
    program_name = Clp_ProgramName(clp);

    ErrorHandler *errh = ErrorHandler::static_initialize(new FileErrorHandler(stderr));
    const char *input_file = 0;
    FILE *outputf = 0;
    const char *private_use_dotlessj = "uniF6BE";
    bool binary = true;
    const char *font_name = 0;

    while (1) {
	int opt = Clp_Next(clp);
	switch (opt) {

	  case QUIET_OPT:
	    if (clp->negated)
		errh = ErrorHandler::default_handler();
	    else
		errh = new SilentErrorHandler;
	    break;

	  case NAME_OPT:
	    font_name = clp->vstr;
	    break;

	  case PFA_OPT:
	    binary = false;
	    break;

	  case PFB_OPT:
	    binary = true;
	    break;

	  case OUTPUT_OPT:
	  output_file:
	    if (outputf)
		usage_error(errh, "output file specified twice");
	    if (strcmp(clp->vstr, "-") == 0)
		outputf = stdout;
	    else if (!(outputf = fopen(clp->vstr, "wb")))
		errh->fatal("<%d>%s: %s", -EXIT_ERROR, clp->vstr, strerror(errno));
	    break;

	  case VERSION_OPT:
	    printf("t1dotlessj (LCDF typetools) %s\n", VERSION);
	    printf("Copyright (C) 2003-2019 Eddie Kohler\n\
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
	    if (input_file && outputf)
		usage_error(errh, "too many arguments");
	    else if (input_file)
		goto output_file;
	    else
		input_file = clp->vstr;
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
    Type1Font *font = do_file(input_file, psres, errh);
    if (!input_file || strcmp(input_file, "-") == 0)
	input_file = "<stdin>";

    // check for existing dotlessj
    if (font->glyph("dotlessj"))
	errh->fatal("<%d>%s: already has a %<dotlessj%> glyph", -EXIT_DOTLESSJ_EXISTS, font->font_name().c_str());
    else if (font->glyph("uni0237"))
	errh->fatal("<%d>%s: already has a dotlessj glyph at %<uni0237%>", -EXIT_DOTLESSJ_EXISTS, font->font_name().c_str());
    else if (font->glyph("u0237"))
	errh->fatal("<%d>%s: already has a dotlessj glyph at %<u0237%>", -EXIT_DOTLESSJ_EXISTS, font->font_name().c_str());
    else if (private_use_dotlessj && font->glyph(private_use_dotlessj))
	errh->fatal("<%d>%s: already has a dotlessj glyph at %<%s%>", -EXIT_DOTLESSJ_EXISTS, font->font_name().c_str(), private_use_dotlessj);

    // check for j
    Type1Charstring *j_cs = font->glyph("j");
    if (!j_cs)
	j_cs = font->glyph("uni006A");
    if (!j_cs)
	j_cs = font->glyph("u006A");
    if (!j_cs)
	errh->fatal("<%d>%s: has no %<j%> glyph to make dotless", -EXIT_NO_J, font->font_name().c_str());

    // make new font
    String actual_font_name = (font_name ? String(font_name) : font->font_name() + String("LCDFJ"));
    if (actual_font_name.length() > 29 && !font_name) {
	errh->warning("derived font name %<%s%> longer than 29 characters", actual_font_name.c_str());
	errh->message("(Use the %<--name%> option to supply your own name.)");
    }

    Vector<double> xuid_extension;
    xuid_extension.push_back(0x00237237);
    Type1Font *dotless_font = Type1Font::skeleton_make_copy(font, actual_font_name, &xuid_extension);
    dotless_font->skeleton_common_subrs();

    // copy space and .notdef
    if (Type1Charstring *notdef = font->glyph(".notdef"))
	dotless_font->add_glyph(Type1Subr::make_glyph(".notdef", *notdef, " |-"));
    if (Type1Charstring *space = font->glyph("space")) {
	dotless_font->add_glyph(Type1Subr::make_glyph("space", *space, " |-"));
	dotless_font->type1_encoding()->put(' ', "space");
    }

    // create dotless j
    Sectioner sec(5);
    sec.run(CharstringContext(font, j_cs));
    sec.undot(font->font_name(), errh);
    Type1Subr *dotlessj = Type1Subr::make_glyph("uni0237", sec.gen(dotless_font), " |-");
    dotless_font->add_glyph(dotlessj);
    //fprintf(stderr, "!  %s\n", CharstringUnparser::unparse(dotlessj->t1cs()).c_str());

    // encode dotless j
    dotless_font->type1_encoding()->clear();
    dotless_font->type1_encoding()->put('j', "uni0237");

    // write it to output
    if (!outputf)
	outputf = stdout;
    if (binary) {
#if defined(_MSDOS) || defined(_WIN32)
	_setmode(_fileno(outputf), _O_BINARY);
#endif
	Type1PFBWriter w(outputf);
	dotless_font->write(w);
    } else {
	Type1PFAWriter w(outputf);
	dotless_font->write(w);
    }

    return (errh->nerrors() == 0 ? EXIT_NORMAL : EXIT_ERROR);
}
