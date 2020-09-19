/* t1rawafm.cc -- driver for generating a raw AFM file from a font
 *
 * Copyright (c) 2008-2019 Eddie Kohler
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
#if defined(MIKTEX)
#define exit(x) throw(x)
#endif

using namespace Efont;

#define VERSION_OPT     301
#define HELP_OPT        302
#define OUTPUT_OPT      303
#define SMOKE_OPT       305

const Clp_Option options[] = {
    { "help", 'h', HELP_OPT, 0, 0 },
    { "output", 'o', OUTPUT_OPT, Clp_ValString, 0 },
    { "version", 0, VERSION_OPT, 0, 0 },
};


static const char *program_name;
static PermString::Initializer initializer;


void
usage_error(ErrorHandler *errh, const char *error_message, ...)
{
    va_list val;
    va_start(val, error_message);
    if (!error_message)
        errh->message("Usage: %s [OPTION]... [FONT [OUTPUT]]", program_name);
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
%<T1rawafm%> generates a raw (kernless and ligatureless) AFM file corresponding\n\
to the specified Type 1 font file and writes it to the standard output.\n\
\n\
Usage: %s [OPTION]... [FONT [OUTPUT]]\n\
\n\
FONT is the name of a PFA or PFB font file. If omitted, t1rawafm will read a\n\
font file from the standard input.\n\
\n\
Options:\n\
  -o, --output=FILE            Write output to FILE instead of standard output.\n\
  -h, --help                   Print this message and exit.\n\
      --version                Print version number and exit.\n\
\n\
Report bugs to <ekohler@gmail.com>.\n", program_name);
}


// MAIN

static Type1Font *font;

static void
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

    font = new Type1Font(*reader);
    if (!font->ok())
        errh->fatal("%s: not a Type 1 font", filename);

    delete reader;
}


/*****
 * MAIN PROGRAM
 **/

struct fontinfo_t {
    const char *afm_name;
    int dict;
    const char *dict_name;
    bool as_string;
};

static const fontinfo_t fontinfo_strings[] = {
    { "Version", Type1Font::dFI, "version", true },
    { "Notice", Type1Font::dFI, "Notice", true },
    { "FullName", Type1Font::dFI, "FullName", true },
    { "FamilyName", Type1Font::dFI, "FamilyName", true },
    { "Weight", Type1Font::dFI, "Weight", true },
    { "ItalicAngle", Type1Font::dFI, "ItalicAngle", false },
    { "IsFixedPitch", Type1Font::dFI, "isFixedPitch", false },
    { "UnderlinePosition", Type1Font::dFI, "UnderlinePosition", false },
    { "UnderlineThickness", Type1Font::dFI, "UnderlineThickness", false }
};

static String
strip_newlines(const String &str)
{
    StringAccum sa;
    const char *end = str.end(), *last = str.begin();
    for (const char *s = str.begin(); s != end; ++s)
        if (*s == '\n' || *s == '\r' || *s == '\f' || *s == '\v') {
            sa.append(last, s);
            last = s + 1;
        }
    if (last == str.begin())
        return str;
    else {
        sa.append(last, end);
        return sa.take_string();
    }
}

static void
write_char(FILE *outf, int c, PermString n, Type1Charstring *g, const Transform &font_transform, Type1Font *font)
{
    double bb[4], wx;
    CharstringBounds::bounds(font_transform, CharstringContext(font, g), bb, wx);
    fprintf(outf, "C %d ; WX %d ; N %s ; B %d %d %d %d ;\n",
            c, (int) ceil(wx), n.c_str(),
            (int) floor(bb[0]), (int) floor(bb[1]),
            (int) ceil(bb[2]), (int) ceil(bb[3]));
}

static void
write_afm(FILE *outf, Type1Font *font)
{
    fprintf(outf, "StartFontMetrics 2.0\n\
Comment Generated by t1rawafm\n");

    for (size_t i = 0; i < sizeof(fontinfo_strings) / sizeof(fontinfo_t); ++i)
        if (Type1Definition *t1d = font->dict(fontinfo_strings[i].dict,
                                              fontinfo_strings[i].dict_name)) {
            if (!fontinfo_strings[i].as_string)
                fprintf(outf, "%s %s\n", fontinfo_strings[i].afm_name, t1d->value().c_str());
            else {
                String s;
                if (t1d->value_string(s))
                    fprintf(outf, "%s %s\n", fontinfo_strings[i].afm_name, strip_newlines(s).c_str());
            }
        }

    Transform font_transform;
    {
        double font_matrix[6];
        font->font_matrix(font_matrix);
        font_transform = Transform(font_matrix);
        font_transform.scale(1000);
    }

    double bb[4], wx;
    if (Type1Charstring *t1cs = font->glyph("H")) {
        CharstringBounds::bounds(font_transform, CharstringContext(font, t1cs), bb, wx);
        if (bb[3])
            fprintf(outf, "CapHeight %d\n", (int) ceil(bb[3]));
    }
    if (Type1Charstring *t1cs = font->glyph("x")) {
        CharstringBounds::bounds(font_transform, CharstringContext(font, t1cs), bb, wx);
        if (bb[3])
            fprintf(outf, "XHeight %d\n", (int) ceil(bb[3]));
    }
    if (Type1Charstring *t1cs = font->glyph("d")) {
        CharstringBounds::bounds(font_transform, CharstringContext(font, t1cs), bb, wx);
        if (bb[3])
            fprintf(outf, "Ascender %d\n", (int) ceil(bb[3]));
    }
    if (Type1Charstring *t1cs = font->glyph("p")) {
        CharstringBounds::bounds(font_transform, CharstringContext(font, t1cs), bb, wx);
        if (bb[1])
            fprintf(outf, "Descender %d\n", (int) floor(bb[1]));
    }

    Vector<double> vd;
    if (Type1Definition *t1d = font->p_dict("StdHW"))
        if (t1d->value_numvec(vd) && vd.size() > 0)
            fprintf(outf, "StdHW %d\n", (int) ceil(vd[0]));
    if (Type1Definition *t1d = font->p_dict("StdVW"))
        if (t1d->value_numvec(vd) && vd.size() > 0)
            fprintf(outf, "StdVW %d\n", (int) ceil(vd[0]));

    double fontbb[4] = { 1000000, 1000000, -1000000, -1000000 };
    for (int i = 0; i < font->nglyphs(); ++i) {
        CharstringBounds::bounds(font_transform, CharstringContext(font, font->glyph(i)), bb, wx);
        fontbb[0] = std::min(fontbb[0], bb[0]);
        fontbb[1] = std::min(fontbb[1], bb[1]);
        fontbb[2] = std::max(fontbb[2], bb[2]);
        fontbb[3] = std::max(fontbb[3], bb[3]);
    }
    fprintf(outf, "FontBBox %d %d %d %d\n",
            (int) floor(fontbb[0]), (int) floor(fontbb[1]),
            (int) ceil(fontbb[2]), (int) ceil(fontbb[3]));

    fprintf(outf, "FontName %s\n", font->font_name().c_str());

    int nglyphs = font->nglyphs();
    PermString dot_notdef(".notdef");
    if (font->glyph(dot_notdef))
        --nglyphs;
    fprintf(outf, "Characters %d\n", nglyphs);

    fprintf(outf, "StartCharMetrics %d\n", nglyphs);
    HashMap<PermString, int> done_yet(0);
    done_yet.insert(dot_notdef, 1);
    if (Type1Encoding *enc = font->type1_encoding()) {
        for (int i = 0; i < 256; ++i) {
            PermString n = enc->elt(i);
            if (!done_yet[n])
                if (Type1Charstring *g = font->glyph(n)) {
                    write_char(outf, i, n, g, font_transform, font);
                    done_yet.insert(n, true);
                }
        }
    }
    for (int i = 0; i < font->nglyphs(); ++i) {
        PermString n = font->glyph_name(i);
        if (!done_yet[n])
            write_char(outf, -1, n, font->glyph(i), font_transform, font);
    }
    fprintf(outf, "EndCharMetrics\n");

    fprintf(outf, "EndFontMetrics\n");
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
    const char *output_file = 0;
    Vector<String> glyph_patterns;

    while (1) {
        int opt = Clp_Next(clp);
        switch (opt) {

        case OUTPUT_OPT:
        output_file:
            if (output_file)
                errh->fatal("output file already specified");
            output_file = clp->vstr;
            break;

          case VERSION_OPT:
            printf("t1rawafm (LCDF typetools) %s\n", VERSION);
            printf("Copyright (C) 2008-2019 Eddie Kohler\n\
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
            if (font)
                goto output_file;
            else
                do_file(clp->vstr, psres, errh);
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
    if (!font)
        do_file(0, psres, errh);

    FILE *outf;
    if (!output_file || strcmp(output_file, "-") == 0)
        outf = stdout;
    else {
        outf = fopen(output_file, "w");
        if (!outf)
            errh->fatal("%s: %s", output_file, strerror(errno));
    }

    write_afm(outf, font);

    exit(0);
}
