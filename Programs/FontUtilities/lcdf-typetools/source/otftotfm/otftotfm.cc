/* otftotfm.cc -- driver for translating OpenType fonts to TeX metrics
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
#ifdef WIN32
# define _USE_MATH_DEFINES
#endif
#include <efont/psres.hh>
#include <efont/t1rw.hh>
#include <efont/t1font.hh>
#include <efont/t1item.hh>
#include <efont/t1bounds.hh>
#include <efont/otfcmap.hh>
#include <efont/otfname.hh>
#include <efont/otfgsub.hh>
#include <efont/ttfkern.hh>
#include "glyphfilter.hh"
#include "metrics.hh"
#include "dvipsencoding.hh"
#include "automatic.hh"
#include "secondary.hh"
#include "kpseinterface.h"
#include "util.hh"
#include "otftotfm.hh"
#include <lcdf/md5.h>
#include <lcdf/clp.h>
#include <lcdf/error.hh>
#include <lcdf/hashmap.hh>
#include <lcdf/straccum.hh>
#include <efont/cff.hh>
#include <efont/otf.hh>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <algorithm>
#include <math.h>
/* M_PI isn't defined in C99 */
#ifndef M_PI
# define M_PI 3.14159265358979323846264338327
#endif
#ifdef HAVE_CTIME
# include <time.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

using namespace Efont;

#define VERSION_OPT             301
#define HELP_OPT                302
#define QUERY_SCRIPTS_OPT       303
#define QUERY_FEATURES_OPT      304
#define KPATHSEA_DEBUG_OPT      305

#define SCRIPT_OPT              311
#define FEATURE_OPT             312
#define ENCODING_OPT            313
#define LITERAL_ENCODING_OPT    314
#define EXTEND_OPT              315
#define SLANT_OPT               316
#define LETTERSPACE_OPT         317
#define LIGKERN_OPT             318
#define CODINGSCHEME_OPT        319
#define UNICODING_OPT           320
#define BOUNDARY_CHAR_OPT       321
#define DESIGN_SIZE_OPT         322
#define MINIMUM_KERN_OPT        323
#define ALTSELECTOR_CHAR_OPT    324
#define INCLUDE_ALTERNATES_OPT  325
#define EXCLUDE_ALTERNATES_OPT  326
#define CLEAR_ALTERNATES_OPT    327
#define ALTSELECTOR_FEATURE_OPT 328
#define DEFAULT_LIGKERN_OPT     329
#define NO_ECOMMAND_OPT         330
#define LETTER_FEATURE_OPT      331
#define INCLUDE_SUBS_OPT        332
#define EXCLUDE_SUBS_OPT        333
#define CLEAR_SUBS_OPT          334
#define SUBS_FILTER_OPT         335
#define ALTERNATES_FILTER_OPT   336
#define SPACE_FACTOR_OPT        337
#define MATH_SPACING_OPT        338
#define POSITION_OPT            339
#define WARN_MISSING_OPT        340
#define BASE_ENCODINGS_OPT      341
#define FIXED_PITCH_OPT         342
#define ITALIC_ANGLE_OPT        343
#define PROPORTIONAL_WIDTH_OPT  344
#define X_HEIGHT_OPT            345

#define AUTOMATIC_OPT           350
#define FONT_NAME_OPT           351
#define QUIET_OPT               352
#define GLYPHLIST_OPT           353
#define VENDOR_OPT              354
#define TYPEFACE_OPT            355
#define NOCREATE_OPT            356
#define VERBOSE_OPT             357
#define FORCE_OPT               358

#define VIRTUAL_OPT             360
#define PL_OPT                  361
#define TFM_OPT                 362
#define MAP_FILE_OPT            363
#define OUTPUT_ENCODING_OPT     364

#define DIR_OPTS                380
#define ENCODING_DIR_OPT        (DIR_OPTS + O_ENCODING)
#define TFM_DIR_OPT             (DIR_OPTS + O_TFM)
#define PL_DIR_OPT              (DIR_OPTS + O_PL)
#define VF_DIR_OPT              (DIR_OPTS + O_VF)
#define VPL_DIR_OPT             (DIR_OPTS + O_VPL)
#define TYPE1_DIR_OPT           (DIR_OPTS + O_TYPE1)
#define TYPE42_DIR_OPT          (DIR_OPTS + O_TYPE42)
#define TRUETYPE_DIR_OPT        (DIR_OPTS + O_TRUETYPE)
#define DIR_OPT                 (DIR_OPTS + NUMODIR)

#define NO_OUTPUT_OPTS          400
#define NO_ENCODING_OPT         (NO_OUTPUT_OPTS + G_ENCODING)
#define NO_TYPE1_OPT            (NO_OUTPUT_OPTS + G_TYPE1)
#define NO_DOTLESSJ_OPT         (NO_OUTPUT_OPTS + G_DOTLESSJ)
#define NO_UPDMAP_OPT           (NO_OUTPUT_OPTS + G_UPDMAP)
#define UPDMAP_SYS_OPT          (NO_OUTPUT_OPTS + G_UPDMAP_USER)

#define YES_OUTPUT_OPTS         2000
#define TRUETYPE_OPT            (YES_OUTPUT_OPTS + G_TRUETYPE)
#define TYPE42_OPT              (YES_OUTPUT_OPTS + G_TYPE42)
#define UPDMAP_USER_OPT         (YES_OUTPUT_OPTS + G_UPDMAP_USER)

#define CHAR_OPTTYPE            (Clp_ValFirstUser)

static Clp_Option options[] = {

    { "script", 's', SCRIPT_OPT, Clp_ValString, 0 },
    { "feature", 'f', FEATURE_OPT, Clp_ValString, 0 },
    { "letter-feature", 0, LETTER_FEATURE_OPT, Clp_ValString, 0 },
    { "lf", 0, LETTER_FEATURE_OPT, Clp_ValString, 0 },
    { "include-substitutions", 0, INCLUDE_SUBS_OPT, Clp_ValString, 0 },
    { "exclude-substitutions", 0, EXCLUDE_SUBS_OPT, Clp_ValString, 0 },
    { "clear-substitutions", 0, CLEAR_SUBS_OPT, 0, 0 },
    { "substitution-filter", 0, SUBS_FILTER_OPT, Clp_ValString, 0 },
    { "subs-filter", 0, SUBS_FILTER_OPT, Clp_ValString, 0 },
    { "encoding", 'e', ENCODING_OPT, Clp_ValString, 0 },
    { "literal-encoding", 0, LITERAL_ENCODING_OPT, Clp_ValString, 0 },
    { "base-encodings", 0, BASE_ENCODINGS_OPT, Clp_ValString, 0 },
    { "extend", 'E', EXTEND_OPT, Clp_ValDouble, 0 },
    { "slant", 'S', SLANT_OPT, Clp_ValDouble, 0 },
    { "letterspacing", 'L', LETTERSPACE_OPT, Clp_ValInt, 0 },
    { "letterspace", 'L', LETTERSPACE_OPT, Clp_ValInt, 0 },
    { "min-kern", 'k', MINIMUM_KERN_OPT, Clp_ValDouble, 0 },
    { "minimum-kern", 'k', MINIMUM_KERN_OPT, Clp_ValDouble, 0 },
    { "kern-precision", 'k', MINIMUM_KERN_OPT, Clp_ValDouble, 0 },
    { "ligkern", 0, LIGKERN_OPT, Clp_ValString, 0 },
    { "position", 0, POSITION_OPT, Clp_ValString, 0 },
    { "warn-missing", 0, WARN_MISSING_OPT, 0, Clp_Negate },
    { "no-encoding-commands", 0, NO_ECOMMAND_OPT, 0, 0 },
    { "default-ligkern", 0, DEFAULT_LIGKERN_OPT, 0, Clp_Negate },
    { "unicoding", 0, UNICODING_OPT, Clp_ValString, 0 },
    { "coding-scheme", 0, CODINGSCHEME_OPT, Clp_ValString, 0 },
    { "boundary-char", 0, BOUNDARY_CHAR_OPT, CHAR_OPTTYPE, 0 },
    { "altselector", 0, ALTSELECTOR_CHAR_OPT, CHAR_OPTTYPE, Clp_PreferredMatch },
    { "altselector-char", 0, ALTSELECTOR_CHAR_OPT, CHAR_OPTTYPE, 0 },
    { "altselector-feature", 0, ALTSELECTOR_FEATURE_OPT, Clp_ValString, 0 },
    { "design-size", 0, DESIGN_SIZE_OPT, Clp_ValDouble, 0 },
    { "include-alternates", 0, INCLUDE_ALTERNATES_OPT, Clp_ValString, 0 },
    { "exclude-alternates", 0, EXCLUDE_ALTERNATES_OPT, Clp_ValString, 0 },
    { "clear-alternates", 0, CLEAR_ALTERNATES_OPT, 0, 0 },
    { "alternates-filter", 0, ALTERNATES_FILTER_OPT, Clp_ValString, 0 },
    { "space-factor", 0, SPACE_FACTOR_OPT, Clp_ValDouble, 0 },
    { "math-spacing", 0, MATH_SPACING_OPT, CHAR_OPTTYPE, Clp_Negate | Clp_Optional },
    { "fixed-pitch", 0, FIXED_PITCH_OPT, 0, Clp_Negate },
    { "fixed-width", 0, FIXED_PITCH_OPT, 0, Clp_Negate },
    { "proportional-width", 0, PROPORTIONAL_WIDTH_OPT, 0, Clp_Negate },
    { "italic-angle", 0, ITALIC_ANGLE_OPT, Clp_ValDouble, 0 },
    { "x-height", 0, X_HEIGHT_OPT, Clp_ValString, 0 },

    { "pl", 'p', PL_OPT, 0, Clp_Negate },
    { "tfm", 't', TFM_OPT, 0, Clp_Negate }, // not in documentation
    { "virtual", 0, VIRTUAL_OPT, 0, Clp_Negate },
    { "no-encoding", 0, NO_ENCODING_OPT, 0, 0 },
    { "no-type1", 0, NO_TYPE1_OPT, 0, 0 },
    { "no-dotlessj", 0, NO_DOTLESSJ_OPT, 0, 0 },
    { "no-updmap", 0, NO_UPDMAP_OPT, 0, 0 },
    { "updmap-sys", 0, UPDMAP_SYS_OPT, 0, 0 },
    { "updmap-user", 0, UPDMAP_USER_OPT, 0, 0 },
    { "truetype", 0, TRUETYPE_OPT, 0, Clp_Negate },
    { "type42", 0, TYPE42_OPT, 0, Clp_Negate },
    { "map-file", 0, MAP_FILE_OPT, Clp_ValString, Clp_Negate },
    { "output-encoding", 0, OUTPUT_ENCODING_OPT, Clp_ValString, Clp_Optional },

    { "automatic", 'a', AUTOMATIC_OPT, 0, Clp_Negate },
    { "name", 'n', FONT_NAME_OPT, Clp_ValString, 0 },
    { "vendor", 'v', VENDOR_OPT, Clp_ValString, 0 },
    { "typeface", 0, TYPEFACE_OPT, Clp_ValString, 0 },

    { "directory", 0, DIR_OPT, Clp_ValString, 0 },
    { "encoding-directory", 0, ENCODING_DIR_OPT, Clp_ValString, 0 },
    { "pl-directory", 0, PL_DIR_OPT, Clp_ValString, 0 },
    { "tfm-directory", 0, TFM_DIR_OPT, Clp_ValString, 0 },
    { "vpl-directory", 0, VPL_DIR_OPT, Clp_ValString, 0 },
    { "vf-directory", 0, VF_DIR_OPT, Clp_ValString, 0 },
    { "type1-directory", 0, TYPE1_DIR_OPT, Clp_ValString, 0 },
    { "type42-directory", 0, TYPE42_DIR_OPT, Clp_ValString, 0 },
    { "truetype-directory", 0, TRUETYPE_DIR_OPT, Clp_ValString, 0 },

    { "quiet", 'q', QUIET_OPT, 0, Clp_Negate },
    { "glyphlist", 0, GLYPHLIST_OPT, Clp_ValString, 0 },
    { "no-create", 0, NOCREATE_OPT, 0, 0 },
    { "force", 0, FORCE_OPT, 0, Clp_Negate },
    { "verbose", 'V', VERBOSE_OPT, 0, Clp_Negate },
    { "kpathsea-debug", 0, KPATHSEA_DEBUG_OPT, Clp_ValInt, 0 },

    { "help", 'h', HELP_OPT, 0, 0 },
    { "version", 0, VERSION_OPT, 0, 0 },

    { "query-features", 0, QUERY_FEATURES_OPT, 0, 0 },
    { "qf", 0, QUERY_FEATURES_OPT, 0, 0 },
    { "query-scripts", 0, QUERY_SCRIPTS_OPT, 0, 0 },
    { "qs", 0, QUERY_SCRIPTS_OPT, 0, 0 },

};

static const char * const default_ligkerns = "\
space l =: lslash ; space L =: Lslash ; \
question quoteleft =: questiondown ; \
exclam quoteleft =: exclamdown ; \
hyphen hyphen =: endash ; endash hyphen =: emdash ; \
quoteleft quoteleft =: quotedblleft ; \
quoteright quoteright =: quotedblright ;";

struct BaseEncoding {
    String font_name;
    String secondary;
    DvipsEncoding encoding;
};

static const char *program_name;
static String current_time;
static StringAccum invocation;

static PermString::Initializer perm_initializer;
static PermString dot_notdef(".notdef");

static Vector<Efont::OpenType::Tag> interesting_scripts;
static Vector<Efont::OpenType::Tag> interesting_features;
static Vector<Efont::OpenType::Tag> altselector_features;
static GlyphFilter null_filter;
static HashMap<Efont::OpenType::Tag, GlyphFilter*> feature_filters(0);
static HashMap<Efont::OpenType::Tag, GlyphFilter*> altselector_feature_filters(0);

static String font_name;
static String encoding_file;
static Vector<BaseEncoding *> base_encodings;
static double extend;
static double slant;
int letterspace;
static double design_size;
static double minimum_kern = 2.0;
static double space_factor = 1.0;
static bool math_spacing = false;
static int skew_char = -1;
static bool override_is_fixed_pitch = false;
static bool is_fixed_pitch;
static bool override_italic_angle = false;
static double italic_angle;
static int override_x_height = FontInfo::x_height_auto;
static double x_height;

static String out_encoding_file;
static String out_encoding_name;

unsigned output_flags = G_ENCODING | G_METRICS | G_VMETRICS | G_PSFONTSMAP | G_TYPE1 | G_DOTLESSJ | G_UPDMAP | G_TRUETYPE;

bool automatic = false;
bool verbose = false;
bool no_create = false;
bool quiet = false;
bool force = false;

static String otf_data;


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
%<Otftotfm%> generates TeX font metrics files from an OpenType font (PostScript\n\
flavor only), including ligatures, kerns, and some positionings. Supply\n\
%<-s SCRIPT[.LANG]%> options to specify a language system, %<-f FEAT%> options to\n\
turn on optional OpenType features, and a %<-e ENC%> option to specify a base\n\
encoding. Output files are written to the current directory (but see\n\
%<--automatic%> and the %<directory%> options).\n\
\n\
Usage: %s [-a] [OPTIONS] OTFFILE FONTNAME\n\n",
           program_name);
    uerrh.message("\
Font feature and transformation options:\n\
  -s, --script=SCRIPT[.LANG]   Use features for script SCRIPT[.LANG] [latn].\n\
  -f, --feature=FEAT           Activate feature FEAT.\n\
  --lf, --letter-feature=FEAT  Activate feature FEAT for letters.\n\
      --subs-filter=PAT        Substitute only characters matching PAT.\n\
      --include-subs=PAT       Same, but cumulative.\n\
      --exclude-subs=PAT       Don%,t substitute characters matching PAT.\n\
      --clear-subs             Clear included/excluded substitutions.\n\
  -E, --extend=F               Widen characters by a factor of F.\n\
  -S, --slant=AMT              Oblique characters by AMT, generally <<1.\n\
  -L, --letterspacing=AMT      Letterspace each character by AMT units.\n\
      --math-spacing[=SKEWCH]  Use letterspacing appropriate for math.\n\
  -k, --min-kern=N             Omit kerns with absolute value < N [2.0].\n\
      --space-factor=F         Scale wordspace by a factor of F.\n\
      --design-size=SIZE       Set font design size to SIZE.\n\
      --fixed-width            Set fixed width (no space stretch).\n\
      --italic-angle=ANGLE     Set font italic angle (for positioning accents).\n\
      --x-height=AMT           Set x-height to AMT units.\n\
\n");
    uerrh.message("\
Encoding options:\n\
  -e, --encoding=FILE          Use DVIPS encoding FILE as a base encoding.\n\
      --boundary-char=CHAR     Set the boundary character to CHAR.\n\
      --altselector-char=CHAR  Set the alternate selector character to CHAR.\n\
      --altselector-feature=F  Activate feature F for --altselector-char.\n\
      --alternates-filter=PAT  Include only alternate characters matching PAT.\n\
      --include-alternates=PAT Same as --alternates-filter, but cumulative.\n\
      --exclude-alternates=PAT Ignore alternate characters matching PAT.\n\
      --clear-alternates       Clear included/excluded alternates.\n\
      --ligkern=COMMAND        Add a ligature or kern.\n\
      --position=COMMAND       Add a POSITION command.\n\
      --unicoding=COMMAND      Add a UNICODING command.\n\
      --no-encoding-commands   Ignore encoding file%,s LIGKERN/UNICODINGs.\n\
      --no-default-ligkern     Don%,t include default LIGKERNs.\n\
      --coding-scheme=SCHEME   Set the output coding scheme to SCHEME.\n\
      --warn-missing           Warn about characters not supported by font.\n\
      --literal-encoding=FILE  Use DVIPS encoding FILE verbatim.\n\
      --base-encodings=FILE    Output can refer to base fonts named in FILE.\n\
\n");
    uerrh.message("\
Automatic mode options:\n\
  -a, --automatic              Install in a TeX Directory Structure.\n\
  -v, --vendor=NAME            Set font vendor for TDS [lcdftools].\n\
      --typeface=NAME          Set typeface name for TDS [<font family>].\n\
      --no-type1               Do not generate Type 1 fonts.\n\
      --no-dotlessj            Do not generate dotless-j fonts.\n\
      --no-truetype            Do not install TrueType-flavored input fonts.\n\
      --no-updmap              Do not run updmap.\n\
      --updmap-user            Run `updmap-user` instead of `updmap-sys`.\n\
\n\
Output options:\n\
  -n, --name=NAME              Generated font name is NAME.\n\
  -p, --pl                     Output human-readable PL/VPLs, not TFM/VFs.\n\
      --no-virtual             Do not generate VFs or VPLs.\n\
      --no-encoding            Do not generate an encoding file.\n\
      --no-map                 Do not generate a psfonts.map line.\n\
      --output-encoding[=FILE] Only generate an encoding file.\n\
\n");
    uerrh.message("\
File location options:\n\
      --tfm-directory=DIR      Put TFM files in DIR [.|automatic].\n\
      --pl-directory=DIR       Put PL files in DIR [.|automatic].\n\
      --vf-directory=DIR       Put VF files in DIR [.|automatic].\n\
      --vpl-directory=DIR      Put VPL files in DIR [.|automatic].\n\
      --encoding-directory=DIR Put encoding files in DIR [.|automatic].\n\
      --type1-directory=DIR    Put Type 1 fonts in DIR [automatic].\n\
      --truetype-directory=DIR Put TrueType fonts in DIR [automatic].\n\
      --directory=DIR          Put output in DIR [.|automatic].\n\
      --map-file=FILE          Update FILE with psfonts.map information [-].\n\
\n\
Other options:\n\
      --glyphlist=FILE         Use FILE to map Adobe glyph names to Unicode.\n\
  -V, --verbose                Print progress information to standard error.\n\
      --no-create              Print messages, don't modify any files.\n\
      --force                  Generate files even if versions already exist.\n"
#if HAVE_KPATHSEA
"      --kpathsea-debug=MASK    Set path searching debug flags to MASK.\n"
#endif
"  -h, --help                   Print this message and exit.\n\
  -q, --quiet                  Do not generate any error messages.\n\
      --version                Print version number and exit.\n\
\n\
Report bugs to <ekohler@gmail.com>.\n");
}

#ifndef WIN32
extern "C" {
static void
sigchld_handler(int s)
{
#if !HAVE_SIGACTION
    signal(s, sigchld_handler);
#else
    (void) s;
#endif
}
}

static void
handle_sigchld()
{
#ifdef SIGCHLD
    int sigchld = SIGCHLD;
#else
    int sigchld = SIGCLD;
#endif
#if HAVE_SIGACTION
    struct sigaction sa;
    sigaction(sigchld, 0, &sa);
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = 0;
    sigaction(sigchld, &sa, 0);
#else
    signal(sigchld, sigchld_handler);
#endif
}
#endif


// MAIN

String
suffix_font_name(const String &font_name, const String &suffix)
{
    const char *begin = font_name.begin(), *end = font_name.end();
    while (end > begin && isdigit((unsigned char) end[-1]))
        --end;
    if (end < font_name.end() && end > begin && end[-1] != '-' && end[-1] != '+')
        end = font_name.end();
    else
        while (end > begin && (end[-1] == '-' || end[-1] == '+'))
            --end;
    if (end == begin)
        end = font_name.end();
    return font_name.substring(begin, end) + suffix + font_name.substring(end, font_name.end());
}

static inline String
make_base_font_name(const String &font_name)
{
    return suffix_font_name(font_name, "--base");
}

static double
get_design_size(const FontInfo &finfo)
{
    try {
        String gpos_table = finfo.otf->table("GPOS");
        if (!gpos_table)
            throw OpenType::Error();

        ErrorHandler *errh = ErrorHandler::silent_handler();
        OpenType::Gpos gpos(gpos_table, errh);

        // extract 'size' feature(s)
        int required_fid;
        Vector<int> fids;
        for (const OpenType::Tag *t = interesting_scripts.begin(); t < interesting_scripts.end(); t += 2)
            gpos.script_list().features(t[0], t[1], required_fid, fids, 0, false);

        int size_fid = gpos.feature_list().find(OpenType::Tag("size"), fids);
        if (size_fid < 0)
            throw OpenType::Error();

        // old Adobe fonts implement an old, incorrect idea
        // of what the FeatureParams offset means.
        OpenType::Name name(finfo.otf->table("name"), errh);
        OpenType::Data size_data = gpos.feature_list().size_params(size_fid, name, errh);
        if (!size_data.length())
            throw OpenType::Error();

        double result = size_data.u16(0) / 10.;
        // check for insane design sizes
        if (result < 1 || result > 1000)
            throw OpenType::Error();

        // return a number in 'pt', not 'bp'
        return result * 72.27 / 72.;

    } catch (OpenType::Error) {
        return 10.0;
    }
}

static const char * const digit_names[] = {
    "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"
};

static inline const char *
lig_context_str(int ctx)
{
    return (ctx == 0 ? "LIG" : (ctx < 0 ? "/LIG" : "LIG/"));
}

static double max_printed_real;

namespace {
struct Printer {
    Printer(FILE* f, unsigned design_units, unsigned units_per_em)
        : f_(f), du_((double) design_units / units_per_em),
          round_(design_units == 1000) {
    }
    inline double transform(double value) const;
    void print_transformed(const char* prefix, double value) const;
    void print(const char* prefix, double value) const;
    String render(double value) const;
    FILE* f_;
    double du_;
    bool round_;
};

inline double Printer::transform(double value) const {
    value *= du_;
    if (round_)
        value = ceil(value);
    return value;
}

void Printer::print_transformed(const char* prefix, double value) const {
    if (round_ || value == 0 || (value > 0.01 && value - floor(value) < 0.01))
        fprintf(f_, "%s R %g)\n", prefix, value);
    else
        fprintf(f_, "%s R %.4f)\n", prefix, value);
    max_printed_real = std::max(max_printed_real, fabs(value));
}

void Printer::print(const char* prefix, double value) const {
    print_transformed(prefix, transform(value));
}

String Printer::render(double value) const {
    value = transform(value);
    if (round_ || value == 0 || (value > 0.01 && value - floor(value) < 0.01))
        return String(value);
    else {
        char buf[128];
        sprintf(buf, "%.4f", value);
        return String(buf);
    }
}
} // namespace

double
font_slant(const FontInfo &finfo)
{
    double val = finfo.italic_angle();
    return -tan(val * M_PI / 180.0) + slant;
}

static void
output_pl(Metrics &metrics, const String &ps_name, int boundary_char,
          const FontInfo &finfo, bool vpl,
          const String &filename, ErrorHandler *errh)
{
    // create file
    if (no_create) {
        errh->message("would create %s", filename.c_str());
        return;
    }

    if (verbose)
        errh->message("creating %s", filename.c_str());
    FILE *f = fopen(filename.c_str(), "w");
    if (!f) {
        errh->error("%s: %s", filename.c_str(), strerror(errno));
        return;
    }

    // XXX check DESIGNSIZE and DESIGNUNITS for correctness

    fprintf(f, "(COMMENT Created by '%s'%s)\n", invocation.c_str(), current_time.c_str());

    // calculate a TeX FAMILY name using afm2tfm's algorithm
    String family_name = String("TeX-") + ps_name;
    if (family_name.length() > 19)
        family_name = family_name.substring(0, 9) + family_name.substring(-10);
    fprintf(f, "(FAMILY %s)\n", family_name.c_str());

    if (metrics.coding_scheme())
        fprintf(f, "(CODINGSCHEME %.39s)\n", String(metrics.coding_scheme()).c_str());
    int design_units = metrics.design_units();

    if (design_size <= 0)
        design_size = get_design_size(finfo);
    max_printed_real = 0;

    fprintf(f, "(DESIGNSIZE R %.1f)\n"
            "(DESIGNUNITS R %d.0)\n"
            "(COMMENT DESIGNSIZE (1 em) IS IN POINTS)\n"
            "(COMMENT OTHER DIMENSIONS ARE MULTIPLES OF DESIGNSIZE/%d)\n"
            "(FONTDIMEN\n", design_size, design_units, design_units);

    // figure out font dimensions
    Transform font_xform;
    if (extend)
        font_xform.scale(extend, 1);
    if (slant)
        font_xform.shear(slant);
    double bounds[4], width;
    Printer pr(f, design_units, metrics.units_per_em());

    double actual_slant = font_slant(finfo);
    if (actual_slant)
        fprintf(f, "   (SLANT R %g)\n", actual_slant);

    if (char_bounds(bounds, width, finfo, font_xform, ' ')) {
        // advance space width by letterspacing, scale by space_factor
        double space_width = (width + (vpl ? letterspace : 0)) * space_factor;
        pr.print("   (SPACE", space_width);
        if (finfo.is_fixed_pitch()) {
            // fixed-pitch: no space stretch or shrink
            pr.print("   (STRETCH", 0);
            pr.print("   (SHRINK", 0);
            pr.print("   (EXTRASPACE", space_width);
        } else {
            pr.print("   (STRETCH", space_width / 2.);
            pr.print("   (SHRINK", space_width / 3.);
            pr.print("   (EXTRASPACE", space_width / 6.);
        }
    }

    double x_height = finfo.x_height(font_xform);
    if (x_height < finfo.units_per_em())
        pr.print("   (XHEIGHT", x_height);

    pr.print("   (QUAD", finfo.units_per_em());
    fprintf(f, "   )\n");

    if (boundary_char >= 0)
        fprintf(f, "(BOUNDARYCHAR D %d)\n", boundary_char);

    // figure out font mapping
    int mapped_font0 = 0;
    Vector<int> font_mapping;
    if (vpl) {
        int vpl_first_font = (metrics.need_base() ? 0 : 1);
        font_mapping.assign(metrics.n_mapped_fonts(), 0);
        if (vpl_first_font == 1 && font_mapping.size() == 1)
            font_mapping.push_back(0);
        // how many times is each font used?
        Vector<Setting> settings;
        for (int i = 0; i < 256; i++)
            if (metrics.setting(i, settings)) {
                int font_number = 0;
                for (const Setting *s = settings.begin(); s < settings.end(); s++)
                    if (s->op == Setting::SHOW)
                        font_mapping[font_number]++;
                    else if (s->op == Setting::FONT)
                        font_number = (int) s->x;
            }
        // make sure the most-used font is number 0
        mapped_font0 = std::max_element(font_mapping.begin(), font_mapping.end()) - font_mapping.begin();
        // prepare the mapping
        for (int i = vpl_first_font; i < font_mapping.size(); i++)
            font_mapping[i] = i - vpl_first_font;
        font_mapping[mapped_font0] = 0;
        font_mapping[vpl_first_font] = mapped_font0 - vpl_first_font;
        if (vpl_first_font != 0)
            font_mapping[0] = font_mapping.size() - 1;
        // write MAPFONT
        for (int i = 0; i < metrics.n_mapped_fonts() - vpl_first_font; i++) {
            int j = std::find(font_mapping.begin(), font_mapping.end(), i) - font_mapping.begin();
            String name = metrics.mapped_font_name(j);
            if (!name)
                name = make_base_font_name(font_name);
            fprintf(f, "(MAPFONT D %d\n   (FONTNAME %s)\n   (FONTDSIZE R %.1f)\n   )\n", i, name.c_str(), design_size);
        }
    } else
        for (int i = 0; i < metrics.n_mapped_fonts(); i++)
            font_mapping.push_back(i);

    // figure out the proper names and numbers for glyphs
    Vector<String> glyph_ids;
    Vector<String> glyph_comments(257, String());
    Vector<String> glyph_base_comments(257, String());
    for (int i = 0; i < metrics.encoding_size(); i++) {
        if (metrics.glyph(i)) {
            PermString name = metrics.code_name(i), expected_name;
            if (i >= '0' && i <= '9')
                expected_name = digit_names[i - '0'];
            else if ((i >= 'A' && i <= 'Z') || (i >= 'a' && i <= 'z'))
                expected_name = PermString((char)i);
            String glyph_comment;
            if (name != expected_name)
                glyph_comment = " (COMMENT " + String(name) + ")";

            int base = metrics.base_code(i);
            if (base >= 0 && base < 256)
                glyph_base_comments[base] = glyph_comment;

            if (i >= 256)
                continue;

            char* expected_name_end;
            if (expected_name
                && (name == expected_name
                    || (name.length() == 7
                        && memcmp(name.data(), "uni00", 5) == 0
                        && strtol(name.c_str() + 3, &expected_name_end, 16) == i
                        && *expected_name_end == 0)))
                glyph_ids.push_back("C " + String((char)i));
            else
                glyph_ids.push_back("D " + String(i));

            glyph_comments[i] = glyph_base_comments[i] = glyph_comment;

        } else if (i < 256)
            glyph_ids.push_back("D " + String(i));
    }
    // finally, BOUNDARYCHAR
    glyph_ids.push_back("BOUNDARYCHAR");

    // LIGTABLE
    fprintf(f, "(LIGTABLE\n");
    Vector<int> lig_code2, lig_outcode, lig_context, kern_code2, kern_amt;
    // don't print KRN x after printing LIG x
    uint32_t used[8];
    bool any_ligs = false;
    StringAccum omitted_clig_sa;
    for (int i = 0; i <= 256; i++)
        if (metrics.glyph(i) && minimum_kern < 10000) {
            int any_lig = metrics.ligatures(i, lig_code2, lig_outcode, lig_context);
            int any_kern = metrics.kerns(i, kern_code2, kern_amt);
            if (any_lig || any_kern) {
                StringAccum kern_sa;
                memset(used, 0, sizeof(used));
                for (int j = 0; j < lig_code2.size(); j++) {
                    if (lig_outcode[j] < 257) {
                        kern_sa << "   (" << lig_context_str(lig_context[j])
                                << ' ' << glyph_ids[lig_code2[j]]
                                << ' ' << glyph_ids[lig_outcode[j]]
                                << ')' << glyph_comments[lig_code2[j]]
                                << glyph_comments[lig_outcode[j]] << '\n';
                        used[lig_code2[j] >> 5] |= (1 << (lig_code2[j] & 0x1F));
                    } else {
                        omitted_clig_sa << "(COMMENT omitted "
                                << lig_context_str(lig_context[j])
                                << ' ' << metrics.code_name(i)
                                << ' ' << metrics.code_name(lig_code2[j])
                                << ' ' << metrics.code_name(lig_outcode[j])
                                << ")\n";
                    }
                }
                for (Vector<int>::const_iterator k2 = kern_code2.begin(); k2 < kern_code2.end(); k2++)
                    if (!(used[*k2 >> 5] & (1 << (*k2 & 0x1F)))) {
                        double this_kern = kern_amt[k2 - kern_code2.begin()];
                        if (fabs(this_kern) >= minimum_kern)
                            kern_sa << "   (KRN " << glyph_ids[*k2]
                                    << " R " << pr.render(this_kern)
                                    << ')' << glyph_comments[*k2] << '\n';
                    }
                if (kern_sa) {
                    if (any_ligs)
                        fprintf(f, "\n");
                    fprintf(f, "   (LABEL %s)%s\n%s   (STOP)\n", glyph_ids[i].c_str(), glyph_comments[i].c_str(), kern_sa.c_str());
                    any_ligs = true;
                }
            }
        }
    fprintf(f, "   )\n");
    if (omitted_clig_sa)
        fprintf(f, "%s\n", omitted_clig_sa.c_str());

    // CHARACTERs
    Vector<Setting> settings;
    StringAccum sa;
    Vector<Point> push_stack;

    for (int i = 0; i < 256; i++)
        if (metrics.setting(i, settings)) {
            fprintf(f, "(CHARACTER %s%s\n", glyph_ids[i].c_str(), glyph_comments[i].c_str());

            // unparse settings into DVI commands
            sa.clear();
            push_stack.clear();
            CharstringBounds boundser(font_xform);
            int program_number = mapped_font0;
            const CharstringProgram *program = finfo.program();
            for (const Setting *s = settings.begin(); s < settings.end(); s++)
                switch (s->op) {

                  case Setting::SHOW:
                    if (vpl || program == finfo.program())
                        boundser.char_bounds(program->glyph_context(s->y));
                    // 3.Aug.2004 -- reported by Marco Kuhlmann: Don't use
                    // glyph_ids[] array when looking at a different font.
                    if (program_number == 0)
                        sa << "      (SETCHAR " << glyph_ids[s->x] << ')' << glyph_base_comments[s->x] << "\n";
                    else
                        sa << "      (SETCHAR D " << s->x << ")\n";
                    break;

                  case Setting::MOVE: {
                      int x = 0, y = 0;
                      while (s+1 < settings.end() && s[1].op == Setting::MOVE)
                          x += s->x, y += s->y, s++;
                      if (vpl)
                          boundser.translate(s->x + x, s->y + y);
                      if (s->x + x)
                          sa << "      (MOVERIGHT R " << pr.render(s->x + x) << ")\n";
                      if (s->y + y)
                          sa << "      (MOVEUP R " << pr.render(s->y + y) << ")\n";
                      break;
                  }

                  case Setting::RULE:
                    if (vpl) {
                        boundser.mark(Point(0, 0));
                        boundser.mark(Point(s->x, s->y));
                        boundser.translate(s->x, 0);
                    }
                    sa << "      (SETRULE R " << pr.render(s->y) << " R " << pr.render(s->x) << ")\n";
                    break;

                  case Setting::FONT:
                    if ((int) s->x != program_number) {
                        program = metrics.mapped_font((int) s->x);
                        program_number = (int) s->x;
                        sa << "      (SELECTFONT D " << font_mapping[program_number] << ")\n";
                    }
                    break;

                  case Setting::PUSH:
                    push_stack.push_back(boundser.transform(Point(0, 0)));
                    sa << "      (PUSH)\n";
                    break;

                  case Setting::POP: {
                      assert(push_stack.size());
                      Point p = push_stack.back() - boundser.transform(Point(0, 0));
                      if (vpl)
                          boundser.translate(p.x, p.y);
                      push_stack.pop_back();
                      sa << "      (POP)\n";
                      break;
                  }

                  case Setting::SPECIAL: {
                      bool needhex = false;
                      for (const char *str = s->s.begin(); str < s->s.end() && !needhex; str++)
                          if (*str < ' ' || *str > '~' || *str == '(' || *str == ')')
                              needhex = true;
                      if (needhex) {
                          sa << "      (SPECIALHEX ";
                          for (const char *str = s->s.begin(); str < s->s.end(); str++) {
                              static const char hexdig[] = "0123456789ABCDEF";
                              int val = (unsigned char) *str;
                              sa << hexdig[val >> 4] << hexdig[val & 0xF];
                          }
                          sa << ")\n";
                      } else
                          sa << "      (SPECIAL " << s->s << ")\n";
                      break;
                  }

                }

            assert(push_stack.size() == 0);

            // output information
            boundser.output(bounds, width);
            pr.print("   (CHARWD", width);
            if (bounds[3] > 0)
                pr.print("   (CHARHT", bounds[3]);
            if (bounds[1] < 0)
                pr.print("   (CHARDP", -bounds[1]);
            if (bounds[2] > width)
                pr.print_transformed("   (CHARIC", pr.transform(bounds[2]) - pr.transform(width));
            if (vpl && (settings.size() > 1 || settings[0].op != Setting::SHOW))
                fprintf(f, "   (MAP\n%s      )\n", sa.c_str());
            fprintf(f, "   )\n");
        }

    // at last, close the file
    fclose(f);

    // Did we print a number too big for TeX to handle?  If so, try again.
    if (max_printed_real >= 2047) {
        if (metrics.design_units() <= 1)
            errh->fatal("This font appears to be broken.  It has characters so big that the PL format\ncannot represent them.");
        metrics.set_design_units(metrics.design_units() > 200 ? metrics.design_units() - 250 : 1);
        if (verbose)
            errh->message("the font%,s metrics overflow the limits of PL files\n(reducing DESIGNUNITS to %d and trying again)", metrics.design_units());
        output_pl(metrics, ps_name, boundary_char, finfo, vpl, filename, errh);
    }
}

struct Lookup {
    bool used;
    bool required;
    Vector<OpenType::Tag> features;
    GlyphFilter* filter;
    Lookup()                    : used(false), required(false), filter(0) { }
};

static void
find_lookups(const OpenType::ScriptList& scripts, const OpenType::FeatureList& features, Vector<Lookup>& lookups, ErrorHandler* errh)
{
    Vector<int> fids, lookupids;
    int required;

    // go over all scripts
    for (int i = 0; i < interesting_scripts.size(); i += 2) {
        OpenType::Tag script = interesting_scripts[i];
        OpenType::Tag langsys = interesting_scripts[i+1];

        // collect features applying to this script
        scripts.features(script, langsys, required, fids, errh);

        // only use the selected features
        features.filter(fids, interesting_features);

        // mark features as having been used
        for (int j = (required < 0 ? 0 : -1); j < fids.size(); j++) {
            int fid = (j < 0 ? required : fids[j]);
            OpenType::Tag ftag = features.tag(fid);
            if (features.lookups(fid, lookupids, errh) < 0)
                lookupids.clear();
            for (int k = 0; k < lookupids.size(); k++) {
                int l = lookupids[k];
                if (l < 0 || l >= lookups.size())
                    errh->error("lookup for %<%s%> feature out of range", OpenType::Tag::langsys_text(script, langsys).c_str());
                else {
                    lookups[l].used = true;
                    lookups[l].features.push_back(ftag);
                    if (j < 0)
                        lookups[l].required = true;
                }
            }
        }
    }

    // now check for compatible glyph filters
    for (Lookup* l = lookups.begin(); l < lookups.end(); l++)
        if (l->used && !l->required) {
            l->filter = feature_filters[l->features[0]];
            for (OpenType::Tag* ft = l->features.begin() + 1; ft < l->features.end(); ft++)
                if (!l->filter->check_eq(*feature_filters[*ft])) {
                    errh->error("%<%s%> and %<%s%> features share a lookup, but have different filters", l->features[0].text().c_str(), ft->text().c_str());
                    break;
                }
        }
}

static int
write_encoding_file(String &filename, const String &encoding_name,
                    StringAccum &contents, ErrorHandler *errh)
{
    FILE *f;
    int ok_retval = (access(filename.c_str(), R_OK) >= 0 ? 0 : 1);

    if (no_create) {
        errh->message((ok_retval ? "would create encoding file %s" : "would update encoding file %s"), filename.c_str());
        return ok_retval;
    } else if (verbose)
        errh->message((ok_retval ? "creating encoding file %s" : "updating encoding file %s"), filename.c_str());

    int fd = open(filename.c_str(), O_RDWR | O_CREAT, 0666);
    if (fd < 0)
        return errh->error("%s: %s", filename.c_str(), strerror(errno));
    f = fdopen(fd, "r+");
    // NB: also change update_autofont_map if you change this code

#if defined(F_SETLKW) && defined(HAVE_FTRUNCATE)
    {
        struct flock lock;
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = 0;
        lock.l_len = 0;
        int result;
        while ((result = fcntl(fd, F_SETLKW, &lock)) < 0 && errno == EINTR)
            /* try again */;
        if (result < 0) {
            result = errno;
            fclose(f);
            return errh->error("locking %s: %s", filename.c_str(), strerror(result));
        }
    }
#endif

    // read old data from encoding file
    StringAccum sa;
    int amt;
    do {
        if (char *x = sa.reserve(8192)) {
            amt = fread(x, 1, 8192, f);
            sa.adjust_length(amt);
        } else
            amt = 0;
    } while (amt != 0);
    if (!feof(f) || ferror(f))
        return errh->error("%s: %s", filename.c_str(), strerror(errno));
    String old_encodings = sa.take_string();
    bool created = (!old_encodings);

    // append old encodings
    int pos1 = old_encodings.find_left("\n%%");
    while (pos1 >= 0 && pos1 < old_encodings.length()) {
        int pos2 = old_encodings.find_left("\n%%", pos1 + 1);
        if (pos2 < 0)
            pos2 = old_encodings.length();
        if (old_encodings.substring(pos1 + 3, encoding_name.length()) == encoding_name) {
            // encoding already exists, don't change it
            fclose(f);
            if (verbose)
                errh->message("%s unchanged", filename.c_str());
            return 0;
        } else
            contents << old_encodings.substring(pos1, pos2 - pos1);
        pos1 = pos2;
    }

    // rewind file
#ifdef HAVE_FTRUNCATE
    rewind(f);
    if (ftruncate(fd, 0) < 0)
#endif
    {
        fclose(f);
        f = fopen(filename.c_str(), "w");
        fd = fileno(f);
    }

    ignore_result(fwrite(contents.data(), 1, contents.length(), f));

    fclose(f);

    // inform about the new file if necessary
    if (created)
        update_odir(O_ENCODING, filename, errh);
    return 0;
}

static bool
output_encoding(const Metrics &metrics,
                const Vector<PermString> &glyph_names,
                ErrorHandler *errh)
{
    static const char * const hex_digits = "0123456789ABCDEF";

    // collect encoding data
    Vector<Metrics::Glyph> glyphs;
    if (!metrics.base_glyphs(glyphs, 256))
        return false;

    StringAccum sa;
    for (int i = 0; i < 256; i++) {
        if ((i & 0xF) == 0)
            sa << (i ? "\n%" : "%") << hex_digits[(i >> 4) & 0xF] << '0' << '\n' << ' ';
        else if ((i & 0x7) == 0)
            sa << '\n' << ' ';
        int g = glyphs[i];
        if (g > 0 && g < glyph_names.size())
            sa << ' ' << '/' << glyph_names[g];
        else
            sa << " /.notdef";
    }
    sa << '\n';

    // digest encoding data
    MD5_CONTEXT md5;
    md5_init(&md5);
    md5_update(&md5, (const unsigned char *)sa.data(), sa.length());
    char text_digest[MD5_TEXT_DIGEST_SIZE + 1];
    md5_final_text(text_digest, &md5);

    // name encoding using digest
    out_encoding_name = "AutoEnc_" + String(text_digest);

    // create encoding filename
    bool output_encoding_only = (bool) out_encoding_file;
    if (!out_encoding_file)
        out_encoding_file = getodir(O_ENCODING, errh) + String("/a_") + String(text_digest).substring(0, 6) + ".enc";

    // exit if we're not responsible for generating an encoding
    if (!(output_flags & G_ENCODING))
        return true;

    // put encoding block in a StringAccum
    // 3.Jun.2003: stick command line definition at the end of the encoding,
    // where it won't confuse idiotic ps2pk
    StringAccum contents;
    if (!output_encoding_only)
        contents << "% THIS FILE WAS AUTOMATICALLY GENERATED -- DO NOT EDIT\n\n\
%%" << out_encoding_name << "\n";
    contents << "% Encoding created by otftotfm" << current_time << "\n\
% Command line follows encoding\n";

    // the encoding itself
    contents << '/' << out_encoding_name << " [\n" << sa << "] def\n";

    // write banner -- unfortunately this takes some doing
    String banner = String("Command line: '") + String(invocation.data(), invocation.length()) + String("'");
    char *buf = banner.mutable_data();
    // get rid of crap characters
    for (int i = 0; i < banner.length(); i++)
        if (buf[i] < ' ' || buf[i] > 0176) {
            if (buf[i] == '\n' || buf[i] == '\r')
                buf[i] = ' ';
            else
                buf[i] = '.';
        }
    // break lines at 80 characters -- it would be nice if this were in a
    // library
    while (banner.length() > 0) {
        int pos = banner.find_left(' '), last_pos = pos;
        while (pos < 75 && pos >= 0) {
            last_pos = pos;
            pos = banner.find_left(' ', pos + 1);
        }
        if (last_pos < 0 || (pos < 0 && banner.length() < 75))
            last_pos = banner.length();
        contents << "% " << banner.substring(0, last_pos) << '\n';
        banner = banner.substring(last_pos + 1);
    }

    // open encoding file
    if (out_encoding_file == "-")
        ignore_result(fwrite(contents.data(), 1, contents.length(), stdout));
    else if (write_encoding_file(out_encoding_file, out_encoding_name, contents, errh) == 1)
        update_odir(O_ENCODING, out_encoding_file, errh);
    return true;
}

static void
output_tfm(Metrics &metrics, const String &ps_name, int boundary_char,
           const FontInfo &finfo, String tfm_filename, String vf_filename,
           String pl_filename, ErrorHandler *errh)
{
    bool had_pl_filename = !pl_filename.empty();
    bool vpl = vf_filename;

    if (!pl_filename) {
        if (no_create) {
            errh->message("would write %s to temporary file", (vpl ? "VPL" : "PL"));
            pl_filename = "<temporary>";
        } else {
            int pl_fd = temporary_file(pl_filename, errh);
            if (pl_fd < 0)
                return;
            output_pl(metrics, ps_name, boundary_char, finfo, vpl, pl_filename, errh);
            close(pl_fd);
        }
    }

    StringAccum command;
    if (vpl)
        command << "vptovf " << shell_quote(pl_filename) << ' ' << shell_quote(vf_filename) << ' ' << shell_quote(tfm_filename) << " 2>&1";
    else
        command << "pltotf " << shell_quote(pl_filename) << ' ' << shell_quote(tfm_filename) << " 2>&1";

    FILE* cmdfile = mypopen(command.c_str(), "r", errh);
    int status;
    if (cmdfile) {
        StringAccum results;
        while (!feof(cmdfile)) {
            char* buf = results.reserve(BUFSIZ);
            results.adjust_length(fread(buf, 1, BUFSIZ, cmdfile));
        }
        // compensate for shitty vptovf/pltotf messages
        char* by_units, *last_line;
        while (!results.empty()
               && (by_units = strstr((char*) results.c_str(), "  units"))
               && (last_line = strrchr((char*) results.c_str(), '\n'))
               && results.end() - last_line > 1
               && isdigit((unsigned char) last_line[1])) {
            char* start_number = last_line + 1;
            char* end_number = last_line + 2;
            while (end_number < results.end()
                   && isdigit((unsigned char) *end_number))
                ++end_number;
            if (end_number < results.end() && *end_number == '.') {
                char* dot = end_number;
                ++end_number;
                while (end_number < results.end()
                       && end_number < dot + 8
                       && isdigit((unsigned char) *end_number))
                    ++end_number;
            }
            String number(start_number, end_number);
            char* by_units_dest = by_units + 1 + (end_number - start_number);
            memmove(by_units_dest, by_units + 1, start_number - (by_units + 1));
            memmove(by_units + 1, number.begin(), number.length());
        }
        if (!results.empty())
            fwrite(results.begin(), 1, results.length(), stderr);
        status = pclose(cmdfile);
    } else
        status = -1;

    if (!no_create && !had_pl_filename)
        unlink(pl_filename.c_str());

    if (status != 0)
        errh->fatal("%s execution failed", (vpl ? "vptovf" : "pltotf"));
    else {
        update_odir(O_TFM, tfm_filename, errh);
        if (vpl)
            update_odir(O_VF, vf_filename, errh);
    }
}

void
output_metrics(Metrics &metrics, const String &ps_name, int boundary_char,
               const FontInfo &finfo,
               const String &encoding_name, const String &encoding_file,
               const String &font_name,
               String (*dvips_include)(const String &ps_name, const FontInfo &, ErrorHandler *),
               ErrorHandler *errh)
{
    String base_font_name = font_name;
    bool need_virtual = metrics.need_virtual(257);
    if (need_virtual) {
        if (output_flags & G_VMETRICS)
            base_font_name = make_base_font_name(font_name);
        else if (output_flags & G_METRICS)
            errh->warning("features require virtual fonts");
    }

    // output virtual metrics
    if (!(output_flags & G_VMETRICS))
        /* do nothing */;
    else if (!need_virtual) {
        if (automatic) {
            // erase old virtual font
            String vf = getodir(O_VF, errh) + "/" + font_name + ".vf";
            if (no_create)
                errh->message("would remove potential VF file %<%s%>", vf.c_str());
            else {
                if (verbose)
                    errh->message("removing potential VF file %<%s%>", vf.c_str());
                if (unlink(vf.c_str()) < 0 && errno != ENOENT)
                    errh->error("removing %s: %s", vf.c_str(), strerror(errno));
            }
        }
    } else {
        String vplfile;
        if (output_flags & G_ASCII) {
            vplfile = getodir(O_VPL, errh) + "/" + font_name + ".vpl";
            output_pl(metrics, ps_name, boundary_char, finfo, true, vplfile, errh);
            update_odir(O_VPL, vplfile, errh);
        }
        if (output_flags & G_BINARY) {
            String tfm = getodir(O_TFM, errh) + "/" + font_name + ".tfm";
            String vf = getodir(O_VF, errh) + "/" + font_name + ".vf";
            output_tfm(metrics, ps_name, boundary_char, finfo, tfm, vf, vplfile, errh);
        }
    }

    // quit if no base needed
    metrics.make_base(257);
    if (!metrics.need_base())
        return;

    // output metrics
    double save_minimum_kern = minimum_kern;
    if (need_virtual)
        minimum_kern = 100000;
    if (output_flags & G_METRICS) {
        String plfile;
        if (output_flags & G_ASCII) {
            plfile = getodir(O_PL, errh) + "/" + base_font_name + ".pl";
            output_pl(metrics, ps_name, boundary_char, finfo, false, plfile, errh);
            update_odir(O_PL, plfile, errh);
        }
        if (output_flags & G_BINARY) {
            String tfm = getodir(O_TFM, errh) + "/" + base_font_name + ".tfm";
            output_tfm(metrics, ps_name, boundary_char, finfo, tfm, String(), plfile, errh);
        }
    }
    minimum_kern = save_minimum_kern;

    // print DVIPS map line
    if (errh->nerrors() == 0 && (output_flags & G_PSFONTSMAP)) {
        StringAccum sa;
        sa << base_font_name << ' ' << ps_name << " \"";
        if (extend)
            sa << extend << " ExtendFont ";
        if (slant)
            sa << slant << " SlantFont ";
        if (encoding_name)
            sa << encoding_name << " ReEncodeFont\" <[" << pathname_filename(encoding_file);
        else
            sa << "\"";
        sa << ' ' << dvips_include(ps_name, finfo, errh) << '\n';
        update_autofont_map(base_font_name, sa.take_string(), errh);
        // if virtual font, remove any map line for base font name
        if (base_font_name != font_name)
            update_autofont_map(font_name, "", errh);
    }
}


enum { F_GSUB_TRY = 1, F_GSUB_PART = 2, F_GSUB_ALL = 4,
       F_GPOS_TRY = 8, F_GPOS_PART = 16, F_GPOS_ALL = 32,
       X_UNUSED = 0, X_BOTH_NONE = 1, X_GSUB_NONE = 2,
       X_GSUB_PART = 3, X_GPOS_NONE = 4, X_GPOS_PART = 5, X_COUNT };

static const char * const x_messages[] = {
    "% ignored, not supported by font",
    "% ignored, too complex for me",
    "complex substitutions from % ignored",
    "some complex substitutions from % ignored",
    "complex positionings from % ignored",
    "some complex positionings from % ignored",
};

static void
report_underused_features(const HashMap<uint32_t, int> &feature_usage, ErrorHandler *errh)
{
    Vector<String> x[X_COUNT];
    for (int i = 0; i < interesting_features.size(); i++) {
        OpenType::Tag f = interesting_features[i];
        int fu = feature_usage[f.value()];
        String ftext = errh->format("%<%s%>", f.text().c_str());
        if (fu == 0)
            x[X_UNUSED].push_back(ftext);
        else if ((fu & (F_GSUB_TRY | F_GPOS_TRY)) == fu)
            x[X_BOTH_NONE].push_back(ftext);
        else {
            if (fu & F_GSUB_TRY) {
                if ((fu & (F_GSUB_PART | F_GSUB_ALL)) == 0)
                    x[X_GSUB_NONE].push_back(ftext);
                else if (fu & F_GSUB_PART)
                    x[X_GSUB_PART].push_back(ftext);
            }
            if (fu & F_GPOS_TRY) {
                if ((fu & (F_GPOS_PART | F_GPOS_ALL)) == 0)
                    x[X_GPOS_NONE].push_back(ftext);
                else if (fu & F_GPOS_PART)
                    x[X_GPOS_PART].push_back(ftext);
            }
        }
    }

    for (int i = 0; i < X_COUNT; i++)
        if (x[i].size())
            goto found;
    return;

  found:
    for (int i = 0; i < X_COUNT; i++)
        if (x[i].size()) {
            StringAccum sa;
            const char* msg_pct = strchr(x_messages[i], '%');
            sa.append(x_messages[i], msg_pct - x_messages[i]);
            const char* sep = (x[i].size() > 2 ? ", " : " ");
            for (const String* a = x[i].begin(); a < x[i].end() - 1; a++)
                sa << *a << sep;
            sa << (x[i].size() > 1 ? "and " : "") << x[i].back()
               << (x[i].size() > 1 ? " features" : " feature") << (msg_pct+1);
            sa.append_break_lines(sa.take_string(), 58);
            errh->warning("%s", sa.c_str());
        }
}

static String otf_filename;

static String
main_dvips_map(const String &ps_name, const FontInfo &finfo, ErrorHandler *errh)
{
    if (String fn = installed_type1(otf_filename, ps_name, (output_flags & G_TYPE1) != 0, errh))
        return "<" + pathname_filename(fn);
    if (!finfo.cff) {
        String ttf_fn, t42_fn;
        ttf_fn = installed_truetype(otf_filename, (output_flags & G_TRUETYPE) != 0, errh);
        t42_fn = installed_type42(otf_filename, ps_name, (output_flags & G_TYPE42) != 0, errh);
        if (t42_fn && (!ttf_fn || (output_flags & G_TYPE42) != 0))
            return "<" + pathname_filename(t42_fn);
        else if (ttf_fn)
            return "<" + pathname_filename(ttf_fn);
    }
    return "<" + pathname_filename(otf_filename);
}

static void
do_gsub(Metrics& metrics, const OpenType::Font& otf,
        DvipsEncoding& dvipsenc, bool dvipsenc_literal,
        HashMap<uint32_t, int>& feature_usage,
        const Vector<PermString>& glyph_names, ErrorHandler* errh)
{
    // find activated GSUB features
    OpenType::Gsub gsub(otf.table("GSUB"), &otf, errh);
    Vector<Lookup> lookups(gsub.nlookups(), Lookup());
    find_lookups(gsub.script_list(), gsub.feature_list(), lookups, errh);

    // find all characters that might result
    Vector<bool> used(glyph_names.size(), false);
    for (Metrics::Code c = 0; c < metrics.encoding_size(); ++c) {
        Metrics::Glyph g = metrics.glyph(c);
        if (g >= 0 && g < used.size())
            used[g] = true;
    }
    for (int i = 0; i < lookups.size(); ++i)
        if (lookups[i].used) {
            OpenType::GsubLookup l = gsub.lookup(i);
            l.mark_out_glyphs(gsub, used);
        }
    OpenType::Coverage used_coverage(used);

    // apply activated GSUB features
    Vector<OpenType::Substitution> subs;
    for (int i = 0; i < lookups.size(); i++)
        if (lookups[i].used) {
            OpenType::GsubLookup l = gsub.lookup(i);
            subs.clear();
            bool understood = l.unparse_automatics(gsub, subs, used_coverage);

            // check for -ffina, which should apply only at the ends of words,
            // and -finit, which should apply only at the beginnings.
            OpenType::Tag feature = (lookups[i].features.size() == 1 ? lookups[i].features[0] : OpenType::Tag());
            if (feature == OpenType::Tag("fina") || feature == OpenType::Tag("fin2") || feature == OpenType::Tag("fin3")) {
                if (dvipsenc.boundary_char() < 0)
                    errh->warning("%<-ffina%> requires a boundary character\n(The input encoding didn%,t specify a boundary character, but\nI need one to implement %<-ffina%> features correctly.  Try\nthe %<--boundary-char%> option.)");
                else {
                    int bg = metrics.boundary_glyph();
                    for (int j = 0; j < subs.size(); j++)
                        subs[j].add_outer_right(bg);
                }
            } else if (feature == OpenType::Tag("init")) {
                int bg = metrics.boundary_glyph();
                for (int j = 0; j < subs.size(); j++)
                    subs[j].add_outer_left(bg);
            }

            //for (int subno = 0; subno < subs.size(); subno++) fprintf(stderr, "%5d\t%s\n", i, subs[subno].unparse().c_str());

            // figure out which glyph filter to use
            int nunderstood = metrics.apply(subs, !dvipsenc_literal, i, *lookups[i].filter, glyph_names);

            // mark as used
            int d = (understood && nunderstood == subs.size() ? F_GSUB_ALL : (nunderstood ? F_GSUB_PART : 0)) + F_GSUB_TRY;
            for (int j = 0; j < lookups[i].features.size(); j++)
                feature_usage.find_force(lookups[i].features[j].value()) |= d;
        }

    // apply alternate selectors
    if (metrics.altselectors() && !dvipsenc_literal) {
        // do lookups
        altselector_features.swap(interesting_features);
        altselector_feature_filters.swap(feature_filters);
        Vector<Lookup> alt_lookups(gsub.nlookups(), Lookup());
        find_lookups(gsub.script_list(), gsub.feature_list(), alt_lookups, ErrorHandler::silent_handler());
        Vector<OpenType::Substitution> alt_subs;
        for (int i = 0; i < alt_lookups.size(); i++)
            if (alt_lookups[i].used) {
                OpenType::GsubLookup l = gsub.lookup(i);
                alt_subs.clear();
                (void) l.unparse_automatics(gsub, alt_subs, used_coverage);
                metrics.apply_alternates(alt_subs, i, *alt_lookups[i].filter, glyph_names);
            }
        altselector_features.swap(interesting_features);
        altselector_feature_filters.swap(feature_filters);
    }
}

static bool
kern_feature_requested()
{
    return std::find(interesting_features.begin(), interesting_features.end(),
                     OpenType::Tag("kern")) != interesting_features.end();
}

static void
do_try_ttf_kern(Metrics& metrics, const OpenType::Font& otf, HashMap<uint32_t, int>& feature_usage, ErrorHandler* errh)
{
    // if no GPOS "kern" lookups and "kern" requested, try "kern" table
    if (!kern_feature_requested())
        return;
    try {
        OpenType::KernTable kern(otf.table("kern"), errh);
        Vector<OpenType::Positioning> poss;
        bool understood = kern.unparse_automatics(poss, errh);
        int nunderstood = metrics.apply(poss);

        // mark as used
        int d = (understood && nunderstood == poss.size() ? F_GPOS_ALL : (nunderstood ? F_GPOS_PART : 0)) + F_GPOS_TRY;
        feature_usage.find_force(OpenType::Tag("kern").value()) |= d;
    } catch (OpenType::BlankTable) {
        // nada
    } catch (OpenType::Error e) {
        errh->warning("kern %<%s%> error, continuing", e.description.c_str());
    }
}

static void
do_gpos(Metrics& metrics, const OpenType::Font& otf, HashMap<uint32_t, int>& feature_usage, ErrorHandler* errh)
{
    OpenType::Gpos gpos(otf.table("GPOS"), errh);
    Vector<Lookup> lookups(gpos.nlookups(), Lookup());
    find_lookups(gpos.script_list(), gpos.feature_list(), lookups, errh);

    // OpenType recommends that if GPOS exists, but the "kern" feature loads
    // no lookups, we use the TrueType "kern" table, if any.
    if (kern_feature_requested()) {
        OpenType::Tag kern_tag("kern");
        for (Lookup *l = lookups.begin(); l != lookups.end(); ++l)
            if (std::find(l->features.begin(), l->features.end(), kern_tag) != l->features.end())
                goto skip_ttf_kern;
        do_try_ttf_kern(metrics, otf, feature_usage, errh);
    skip_ttf_kern: ;
    }

    Vector<OpenType::Positioning> poss;
    for (int i = 0; i < lookups.size(); i++)
        if (lookups[i].used) {
            OpenType::GposLookup l = gpos.lookup(i);
            poss.clear();
            bool understood = l.unparse_automatics(poss, errh);
            int nunderstood = metrics.apply(poss);

            // mark as used
            int d = (understood && nunderstood == poss.size() ? F_GPOS_ALL : (nunderstood ? F_GPOS_PART : 0)) + F_GPOS_TRY;
            for (int j = 0; j < lookups[i].features.size(); j++)
                feature_usage.find_force(lookups[i].features[j].value()) |= d;
        }
}

static void
do_math_spacing(Metrics &metrics, const FontInfo &finfo,
                const DvipsEncoding &dvipsenc)
{
    Transform font_xform;
    if (extend)
        font_xform.scale(extend, 1);
    if (slant)
        font_xform.shear(slant);
    CharstringBounds boundser(font_xform);

    double x_height = finfo.x_height(font_xform);
    double actual_slant = font_slant(finfo);
    int boundary_char = dvipsenc.boundary_char();

    double bounds[4], width;

    for (int code = 0; code < metrics.encoding_size(); code++)
        if (metrics.was_base_glyph(code)
            && code != boundary_char
            && char_bounds(bounds, width, finfo, font_xform, metrics.unicode(code))) {
            int left_sb = (bounds[0] < 0 ? (int) ceil(-bounds[0]) : 0);
            metrics.add_single_positioning(code, left_sb, 0, left_sb);

            if (skew_char >= 0 && code < 256) {
                double sheight = std::max(bounds[3], x_height) - 0.5 * x_height;
                double right_sb = std::max(bounds[2] - width, 0.0);
                double desired = left_sb + 0.5 * width + actual_slant * sheight + 0.25 * right_sb;
                double computed = 0.5 * (left_sb + width + right_sb);
                int skew = (int) (desired - computed);
                metrics.add_kern(code, skew_char, skew);
            }
        }
}

static void
do_file(const String &otf_filename, const OpenType::Font &otf,
        const DvipsEncoding &dvipsenc_in, bool dvipsenc_literal,
        ErrorHandler *errh)
{
    FontInfo finfo(&otf, errh);
    if (!finfo.ok())
        return;
    if (!finfo.cff)
        errh->warning("TrueType-flavored font support is experimental");
    if (override_is_fixed_pitch)
        finfo.set_is_fixed_pitch(is_fixed_pitch);
    if (override_italic_angle)
        finfo.set_italic_angle(italic_angle);
    if (override_x_height != FontInfo::x_height_auto)
        finfo.set_x_height(override_x_height, x_height);

    // save glyph names
    Vector<PermString> glyph_names;
    finfo.glyph_names(glyph_names);
    OpenType::debug_glyph_names = glyph_names;

    // set typeface name from font family name
    {
        String typeface = finfo.family_name();

        // make it reasonable for the shell
        StringAccum sa;
        for (int i = 0; i < typeface.length(); i++)
            if (isalnum((unsigned char) typeface[i]) || typeface[i] == '_' || typeface[i] == '-' || typeface[i] == '.' || typeface[i] == ',' || typeface[i] == '+')
                sa << typeface[i];

        set_typeface(sa.length() ? sa.take_string() : font_name, false);
    }

    // initialize encoding
    DvipsEncoding dvipsenc(dvipsenc_in); // make copy
    Metrics metrics(finfo.program(), finfo.nglyphs());
    // encode boundary glyph at 256; pretend its Unicode value is '\n'
    metrics.encode(256, '\n', metrics.boundary_glyph());
    if (dvipsenc_literal)
        dvipsenc.make_metrics(metrics, finfo, 0, true, errh);
    else {
        T1Secondary secondary(finfo, font_name, otf_filename);
        dvipsenc.make_metrics(metrics, finfo, &secondary, false, errh);
    }

    // maintain statistics about features
    HashMap<uint32_t, int> feature_usage(0);

    // apply activated GSUB features
    try {
        do_gsub(metrics, otf, dvipsenc, dvipsenc_literal, feature_usage, glyph_names, errh);
    } catch (OpenType::BlankTable) {
        // nada
    } catch (OpenType::Error e) {
        errh->warning("GSUB %<%s%> error, continuing", e.description.c_str());
    }

    // apply LIGKERN ligature commands to the result
    dvipsenc.apply_ligkern_lig(metrics, errh);

    // test fake ligature mechanism
    //metrics.add_threeligature('T', 'h', 'e', '0');

    // reencode characters to fit within 8 bytes (+ 1 for the boundary)
    if (!dvipsenc_literal)
        metrics.shrink_encoding(257, dvipsenc_in, errh);

    // apply activated GPOS features
    try {
        do_gpos(metrics, otf, feature_usage, errh);
    } catch (OpenType::BlankTable) {
        do_try_ttf_kern(metrics, otf, feature_usage, errh);
    } catch (OpenType::Error e) {
        errh->warning("GPOS %<%s%> error, continuing", e.description.c_str());
    }

    // apply LIGKERN kerning and POS positioning commands to the result
    dvipsenc.apply_ligkern_kern(metrics, errh);
    dvipsenc.apply_position(metrics, errh);

    // use prespecified raw fonts
    for (BaseEncoding **be = base_encodings.begin(); be != base_encodings.end(); be++)
        if (!(*be)->secondary) {
            Vector<int> mapp;
            (*be)->encoding.make_base_mappings(mapp, finfo);
            metrics.apply_base_encoding((*be)->font_name, (*be)->encoding, mapp);
        }

    // remove extra characters
    metrics.cut_encoding(257);
    //metrics.unparse();
    int boundary_char = dvipsenc.boundary_char();

    // apply letterspacing, if any
    if (letterspace)
        for (int code = 0; code < metrics.encoding_size(); code++)
            if (metrics.was_base_glyph(code) && code != boundary_char) {
                metrics.add_single_positioning(code, letterspace / 2, 0, letterspace);
                if (code < 256) {
                    metrics.add_kern(code, 256, -letterspace / 2);
                    metrics.add_kern(256, code, -letterspace / 2);
                }
            }

    // apply math letterspacing, if any
    if (math_spacing)
        do_math_spacing(metrics, finfo, dvipsenc);

    // reencode right components of boundary_glyph as boundary_char
    if (metrics.reencode_right_ligkern(256, boundary_char) > 0
        && boundary_char < 0) {
        errh->warning("no boundary character, ignoring some ligatures and/or kerns\n");
        errh->message("(You may want to try the --boundary-char option.)");
    }

    // report unused and underused features if any
    report_underused_features(feature_usage, errh);

    // figure out our FONTNAME
    if (!font_name) {
        // derive font name from OpenType font name
        font_name = finfo.postscript_name();
        if (encoding_file) {
            int slash = encoding_file.find_right('/') + 1;
            int dot = encoding_file.find_right('.');
            if (dot < slash)    // includes dot < 0 case
                dot = encoding_file.length();
            font_name += String("--") + encoding_file.substring(slash, dot - slash);
        }
        if (interesting_scripts.size() != 2 || interesting_scripts[0] != OpenType::Tag("latn") || interesting_scripts[1].valid())
            for (int i = 0; i < interesting_scripts.size(); i += 2) {
                font_name += String("--S") + interesting_scripts[i].text();
                if (interesting_scripts[i+1].valid())
                    font_name += String(".") + interesting_scripts[i+1].text();
            }
        for (int i = 0; i < interesting_features.size(); i++)
            if (feature_usage[interesting_features[i].value()])
                font_name += String("--F") + interesting_features[i].text();
    }

    // output encoding
    if (dvipsenc_literal) {
        out_encoding_name = dvipsenc_in.name();
        out_encoding_file = dvipsenc_in.filename();
    } else
        output_encoding(metrics, glyph_names, errh);

    // set up coding scheme
    if (metrics.coding_scheme())
        metrics.set_design_units(1);
    else
        metrics.set_coding_scheme(out_encoding_name);

    // force no type 1
    if (!finfo.cff && (output_flags & G_TYPE1)) {
        errh->warning("assuming --no-type1 since this font is TrueType-flavored");
        output_flags &= ~G_TYPE1;
    }

    // output
    ::otf_filename = otf_filename;
    output_metrics(metrics, finfo.postscript_name(), dvipsenc.boundary_char(),
                   finfo,
                   out_encoding_name, out_encoding_file,
                   font_name, main_dvips_map, errh);
}


String
installed_metrics_font_name(const String &base_font_name, const String &secondary)
{
    for (BaseEncoding **be = base_encodings.begin(); be != base_encodings.end(); be++)
        if ((*be)->secondary == secondary && ::font_name == base_font_name)
            return (*be)->font_name;
    return String();
}


extern "C" {
static int
clp_parse_char(Clp_Parser *clp, const char *arg, int complain, void *)
{
    if (arg[0] && !arg[1] && !isdigit((unsigned char) arg[0])) {
        clp->val.i = (unsigned char)arg[0];
        return 1;
    } else if (arg[0] == '-' || isdigit((unsigned char) arg[0])) {
        char *end;
        clp->val.i = strtol(arg, &end, 10);
        if (clp->val.i <= 255 && !*end)
            return 1;
    }
    if (complain)
        Clp_OptionError(clp, "'%O' expects a character, not '%s'", arg);
    return 0;
}
}

static void
parse_base_encodings(const String &filename, ErrorHandler *errh)
{
    String str = read_file(filename, errh, true);
    String print_filename = (filename == "-" ? "<stdin>" : filename) + ":";
    int lineno = 1;
    str.c_str();
    const char *s_end = str.end();
    for (const char *s = str.begin(); s != s_end; ) {
        while (s != s_end && isspace((unsigned char) *s) && *s != '\n' && *s != '\r')
            s++;
        // skip comments and blank lines
        if (s != s_end && *s != '%' && *s != '#' && *s != '\n' && *s != '\r') {
            const char *w1 = s;
            while (s != s_end && !isspace((unsigned char) *s))
                s++;
            BaseEncoding *be = new BaseEncoding;
            be->font_name = str.substring(w1, s);
            while (s != s_end && isspace((unsigned char) *s) && *s != '\n' && *s != '\r')
                s++;
            const char *w2 = s;
            while (s != s_end && !isspace((unsigned char) *s))
                s++;
            String efile = str.substring(w2, s);
            while (s != s_end && isspace((unsigned char) *s) && *s != '\n' && *s != '\r')
                s++;
            if (s != s_end && !isspace((unsigned char) *s) && *s != '%' && *s != '#') {
                const char *w3 = s;
                while (s != s_end && !isspace((unsigned char) *s))
                    s++;
                be->secondary = str.substring(w3, s);
            }
            LandmarkErrorHandler lerrh(errh, print_filename + String(lineno));
            int before = lerrh.nerrors();
            if (be->secondary)
                /* encoding ignored */;
            else if (!efile)
                lerrh.error("missing encoding name");
            else if (String path = locate_encoding(efile, errh))
                be->encoding.parse(path, true, true, &lerrh);
            else
                lerrh.error("encoding %<%s%> not found", efile.c_str());
            if (lerrh.nerrors() == before)
                base_encodings.push_back(be);
            else
                delete be;
        }
        while (s != s_end && *s != '\n' && *s != '\r')
            s++;
        if (s != s_end && *s == '\r')
            s++;
        if (s != s_end && *s == '\n')
            s++;
        lineno++;
    }
}

int
main(int argc, char *argv[])
{
#ifndef WIN32
    handle_sigchld();
#endif
    Clp_Parser *clp =
        Clp_NewParser(argc, (const char * const *)argv, sizeof(options) / sizeof(options[0]), options);
    Clp_AddType(clp, CHAR_OPTTYPE, 0, clp_parse_char, 0);
    program_name = Clp_ProgramName(clp);
#if HAVE_KPATHSEA
    kpsei_init(argv[0], "lcdftools");
#endif
#ifdef HAVE_CTIME
    {
        time_t t = time(0);
        char *c = ctime(&t);
        current_time = " on " + String(c).substring(0, -1); // get rid of \n
    }
#endif
    for (int i = 0; i < argc; i++)
        invocation << (i ? " " : "") << argv[i];

    ErrorHandler *errh = ErrorHandler::static_initialize(new FileErrorHandler(stderr, String(program_name) + ": "));
    const char *input_file = 0;
    Vector<String> glyphlist_files;
    bool literal_encoding = false;
    bool have_encoding_file = false;
    Vector<String> ligkern;
    Vector<String> pos;
    Vector<String> unicoding;
    Vector<String> base_encoding_files;
    bool no_ecommand = false, default_ligkern = true;
    int warn_missing = -1;
    unsigned specified_output_flags = 0;
    String codingscheme;
    const char* odirs[NUMODIR + 1];
    for (int i = 0; i <= NUMODIR; ++i) {
        odirs[i] = 0;
    }

    GlyphFilter current_substitution_filter;
    GlyphFilter current_alternate_filter;
    GlyphFilter* current_filter_ptr = &null_filter;
    Vector<GlyphFilter*> allocated_filters;

    while (1) {
        int opt = Clp_Next(clp);
        switch (opt) {

          case SCRIPT_OPT: {
              String arg = clp->vstr;
              int period = arg.find_left('.');
              OpenType::Tag scr(period <= 0 ? arg : arg.substring(0, period));
              if (scr.valid() && period > 0) {
                  OpenType::Tag lang(arg.substring(period + 1));
                  if (lang.valid()) {
                      interesting_scripts.push_back(scr);
                      interesting_scripts.push_back(lang);
                  } else
                      usage_error(errh, "bad language tag");
              } else if (scr.valid()) {
                  interesting_scripts.push_back(scr);
                  interesting_scripts.push_back(OpenType::Tag());
              } else
                  usage_error(errh, "bad script tag");
              break;
          }

          case FEATURE_OPT: {
              OpenType::Tag t(clp->vstr);
              if (!t.valid())
                  usage_error(errh, "bad feature tag");
              else if (feature_filters[t])
                  usage_error(errh, "feature %<%s%> included twice", t.text().c_str());
              else {
                  if (!current_filter_ptr) {
                      current_filter_ptr = new GlyphFilter(current_substitution_filter + current_alternate_filter);
                      allocated_filters.push_back(current_filter_ptr);
                  }
                  interesting_features.push_back(t);
                  feature_filters.insert(t, current_filter_ptr);
              }
              break;
          }

          case LETTER_FEATURE_OPT: {
              OpenType::Tag t(clp->vstr);
              if (!t.valid())
                  usage_error(errh, "bad feature tag");
              else if (feature_filters[t])
                  usage_error(errh, "feature %<%s%> included twice", t.text().c_str());
              else {
                  interesting_features.push_back(t);
                  GlyphFilter* gf = new GlyphFilter;
                  gf->add_substitution_filter("<Letter>", false, errh);
                  *gf += current_alternate_filter;
                  feature_filters.insert(t, gf);
              }
              break;
          }

          case SUBS_FILTER_OPT:
            current_substitution_filter = null_filter;
            /* fallthru */
          case EXCLUDE_SUBS_OPT:
          case INCLUDE_SUBS_OPT:
            current_substitution_filter.add_substitution_filter(clp->vstr, opt == EXCLUDE_SUBS_OPT, errh);
            current_filter_ptr = 0;
            break;

          case CLEAR_SUBS_OPT:
            current_substitution_filter = null_filter;
            current_filter_ptr = 0;
            break;

          case ENCODING_OPT:
            if (encoding_file)
                usage_error(errh, "encoding specified twice");
            encoding_file = clp->vstr;
            have_encoding_file = true;
            break;

          case LITERAL_ENCODING_OPT:
            if (encoding_file)
                usage_error(errh, "encoding specified twice");
            encoding_file = clp->vstr;
            have_encoding_file = true;
            literal_encoding = true;
            break;

          case BASE_ENCODINGS_OPT:
            base_encoding_files.push_back(clp->vstr);
            break;

          case EXTEND_OPT:
            if (extend)
                usage_error(errh, "extend value specified twice");
            extend = clp->val.d;
            break;

          case SLANT_OPT:
            if (slant)
                usage_error(errh, "slant value specified twice");
            slant = clp->val.d;
            break;

          case LETTERSPACE_OPT:
            if (letterspace)
                usage_error(errh, "letterspacing value specified twice");
            letterspace = clp->val.i;
            break;

          case SPACE_FACTOR_OPT:
            if (space_factor != 1)
                usage_error(errh, "space factor specified twice");
            space_factor = clp->val.d;
            break;

          case MATH_SPACING_OPT:
            math_spacing = !clp->negated;
            if (math_spacing && clp->have_val) {
                if (clp->val.i < 0 || clp->val.i > 255)
                    usage_error(errh, "--math-spacing skew character must be between 0 and 255");
                skew_char = clp->val.i;
            }
            break;

          case DESIGN_SIZE_OPT:
            if (design_size > 0)
                usage_error(errh, "design size value specified twice");
            else if (clp->val.d <= 0)
                usage_error(errh, "design size must be > 0");
            design_size = clp->val.d;
            break;

          case LIGKERN_OPT:
            ligkern.push_back(clp->vstr);
            break;

          case POSITION_OPT:
            pos.push_back(clp->vstr);
            break;

          case WARN_MISSING_OPT:
            warn_missing = !clp->negated;
            break;

          case NO_ECOMMAND_OPT:
            no_ecommand = true;
            break;

          case DEFAULT_LIGKERN_OPT:
            default_ligkern = !clp->negated;
            break;

          case BOUNDARY_CHAR_OPT:
            ligkern.push_back(String("|| = ") + String(clp->val.i));
            break;

          case ALTSELECTOR_CHAR_OPT:
            ligkern.push_back(String("^^ = ") + String(clp->val.i));
            break;

          case ALTSELECTOR_FEATURE_OPT: {
              OpenType::Tag t(clp->vstr);
              if (!t.valid())
                  usage_error(errh, "bad feature tag");
              else if (altselector_feature_filters[t])
                  usage_error(errh, "altselector feature %<%s%> included twice", t.text().c_str());
              else {
                  if (!current_filter_ptr) {
                      current_filter_ptr = new GlyphFilter(current_substitution_filter + current_alternate_filter);
                      allocated_filters.push_back(current_filter_ptr);
                  }
                  altselector_features.push_back(t);
                  altselector_feature_filters.insert(t, current_filter_ptr);
              }
              break;
          }

          case ALTERNATES_FILTER_OPT:
            current_alternate_filter = null_filter;
            /* fallthru */
          case EXCLUDE_ALTERNATES_OPT:
          case INCLUDE_ALTERNATES_OPT:
            current_alternate_filter.add_alternate_filter(clp->vstr, opt == EXCLUDE_ALTERNATES_OPT, errh);
            current_filter_ptr = 0;
            break;

          case CLEAR_ALTERNATES_OPT:
            current_alternate_filter = null_filter;
            current_filter_ptr = 0;
            break;

          case UNICODING_OPT:
            unicoding.push_back(clp->vstr);
            break;

          case CODINGSCHEME_OPT:
            if (codingscheme)
                usage_error(errh, "coding scheme specified twice");
            codingscheme = clp->vstr;
            if (codingscheme.length() > 39)
                errh->warning("only first 39 characters of coding scheme are significant");
            if (codingscheme.find_left('(') >= 0 || codingscheme.find_left(')') >= 0)
                usage_error(errh, "coding scheme cannot contain parentheses");
            break;

          case AUTOMATIC_OPT:
            automatic = !clp->negated;
            break;

          case VENDOR_OPT:
            if (!set_vendor(clp->vstr))
                usage_error(errh, "vendor name specified twice");
            break;

          case TYPEFACE_OPT:
            if (!set_typeface(clp->vstr, true))
                usage_error(errh, "typeface name specified twice");
            break;

          case VIRTUAL_OPT:
            if (clp->negated)
                output_flags &= ~G_VMETRICS;
            else
                output_flags |= G_VMETRICS;
            specified_output_flags |= G_VMETRICS;
            break;

        case NO_ENCODING_OPT:
        case NO_TYPE1_OPT:
        case NO_DOTLESSJ_OPT:
        case NO_UPDMAP_OPT:
        case UPDMAP_SYS_OPT:
            output_flags &= ~(opt - NO_OUTPUT_OPTS);
            specified_output_flags |= opt - NO_OUTPUT_OPTS;
            break;

        case TRUETYPE_OPT:
        case TYPE42_OPT:
        case UPDMAP_USER_OPT:
            if (!clp->negated)
                output_flags |= (opt - YES_OUTPUT_OPTS);
            else
                output_flags &= ~(opt - YES_OUTPUT_OPTS);
            specified_output_flags |= opt - YES_OUTPUT_OPTS;
            break;

          case OUTPUT_ENCODING_OPT:
            if (out_encoding_file)
                usage_error(errh, "encoding output file specified twice");
            out_encoding_file = (clp->have_val ? clp->vstr : "-");
            output_flags = G_ENCODING;
            specified_output_flags = -1;
            break;

          case MINIMUM_KERN_OPT:
            minimum_kern = clp->val.d;
            break;

          case MAP_FILE_OPT:
            if (clp->negated)
                output_flags &= ~G_PSFONTSMAP;
            else {
                output_flags |= G_PSFONTSMAP;
                if (!set_map_file(clp->vstr))
                    usage_error(errh, "map file specified twice");
            }
            specified_output_flags |= G_PSFONTSMAP;
            break;

        case PL_OPT:
            if (clp->negated)
                output_flags &= ~G_ASCII;
            else
                output_flags |= G_ASCII;
            specified_output_flags |= G_ASCII;
            break;

        case TFM_OPT:
            if (clp->negated)
                output_flags &= ~G_BINARY;
            else
                output_flags |= G_BINARY;
            specified_output_flags |= G_BINARY;
            break;

        case ENCODING_DIR_OPT:
        case TFM_DIR_OPT:
        case PL_DIR_OPT:
        case VF_DIR_OPT:
        case VPL_DIR_OPT:
        case TYPE1_DIR_OPT:
        case TRUETYPE_DIR_OPT:
        case TYPE42_DIR_OPT:
        case DIR_OPT:
            if (!odirs[opt - DIR_OPTS])
                odirs[opt - DIR_OPTS] = clp->vstr;
            else
                usage_error(errh, "%s directory specified twice", odirname(opt - DIR_OPTS));
            break;

          case FONT_NAME_OPT:
          font_name:
            if (font_name)
                usage_error(errh, "font name specified twice");
            font_name = clp->vstr;
            break;

        case FIXED_PITCH_OPT:
            override_is_fixed_pitch = true;
            is_fixed_pitch = !clp->negated;
            break;

        case PROPORTIONAL_WIDTH_OPT:
            override_is_fixed_pitch = true;
            is_fixed_pitch = !!clp->negated;
            break;

        case ITALIC_ANGLE_OPT:
            override_italic_angle = true;
            italic_angle = clp->val.d;
            break;

          case GLYPHLIST_OPT:
            glyphlist_files.push_back(clp->vstr);
            break;

          case QUERY_FEATURES_OPT:
            usage_error(errh, "run %<otfinfo --query-features%> instead");
            break;

          case QUERY_SCRIPTS_OPT:
            usage_error(errh, "run %<otfinfo --query-scripts%> instead");
            break;

          case QUIET_OPT:
            if (clp->negated)
                errh = ErrorHandler::default_handler();
            else
                // 9.Nov.05 -- need a new SilentErrorHandler, because we use
                // the base SilentErrorHandler elsewhere to ignore errors
                errh = new SilentErrorHandler;
            break;

          case VERBOSE_OPT:
            verbose = !clp->negated;
            break;

          case NOCREATE_OPT:
            no_create = clp->negated;
            break;

          case FORCE_OPT:
            force = !clp->negated;
            break;

          case KPATHSEA_DEBUG_OPT:
#if HAVE_KPATHSEA
            kpsei_set_debug_flags(clp->val.u);
#else
            errh->warning("Not compiled with kpathsea!");
#endif
            break;

        case X_HEIGHT_OPT: {
            char* ends;
            if (strcmp(clp->vstr, "auto") == 0)
                override_x_height = FontInfo::x_height_auto;
            else if (strcmp(clp->vstr, "x") == 0)
                override_x_height = FontInfo::x_height_x;
            else if (strcmp(clp->vstr, "font") == 0
                     || strcmp(clp->vstr, "os/2") == 0)
                override_x_height = FontInfo::x_height_os2;
            else if ((x_height = strtod(clp->vstr, &ends)) >= 0
                     && *ends == 0 && *clp->vstr != 0)
                override_x_height = FontInfo::x_height_explicit;
            else
                usage_error(errh, "bad --x-height option");
            break;
        }

          case VERSION_OPT:
            printf("otftotfm (LCDF typetools) %s\n", VERSION);
            printf("Copyright (C) 2002-2019 Eddie Kohler\n\
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
            if (input_file && font_name)
                usage_error(errh, "too many arguments");
            else if (input_file)
                goto font_name;
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
    // check for odd option combinations
    if (warn_missing > 0 && !(output_flags & G_VMETRICS))
        errh->warning("%<--warn-missing%> has no effect with %<--no-virtual%>");
    if (!(specified_output_flags & (G_BINARY | G_ASCII)))
        output_flags |= G_BINARY;

    // set up file names
    if (!input_file)
        usage_error(errh, "no font filename provided");
    if (encoding_file == "-")
        encoding_file = "";

    // set up output directories
    if (odirs[NUMODIR]) {
        for (int i = 0; i < NUMODIR; ++i)
            if (!odirs[i])
                odirs[i] = odirs[NUMODIR];
    }
    for (int i = 0; i < NUMODIR; ++i)
        if (odirs[i])
            setodir(i, odirs[i]);

    // set up feature filters
    if (!altselector_features.size()) {
        if (!current_filter_ptr) {
            current_filter_ptr = new GlyphFilter(current_substitution_filter + current_alternate_filter);
            allocated_filters.push_back(current_filter_ptr);
        }
        altselector_features.push_back(OpenType::Tag("dlig"));
        altselector_feature_filters.insert(OpenType::Tag("dlig"), current_filter_ptr);
        altselector_features.push_back(OpenType::Tag("salt"));
        altselector_feature_filters.insert(OpenType::Tag("salt"), current_filter_ptr);
    } else if (!current_filter_ptr) {
        errh->warning("some filtering options ignored");
        errh->message("(--include-*, --exclude-*, and --*-filter options must occur\nbefore the feature options to which they should apply.)");
    }

    try {
        // read font
        otf_data = read_file(input_file, errh);
        if (errh->nerrors())
            exit(1);

        LandmarkErrorHandler cerrh(errh, printable_filename(input_file));
        BailErrorHandler bail_errh(&cerrh);

        OpenType::Font otf(otf_data, &bail_errh);
        assert(otf.ok());

        // figure out scripts we care about
        if (!interesting_scripts.size()) {
            interesting_scripts.push_back(Efont::OpenType::Tag("latn"));
            interesting_scripts.push_back(Efont::OpenType::Tag());
        }
        std::sort(interesting_features.begin(), interesting_features.end());
        std::sort(altselector_features.begin(), altselector_features.end());

        // find glyphlist
        if (!glyphlist_files.size()) {
#if HAVE_KPATHSEA
            if (String g = kpsei_find_file("glyphlist.txt", KPSEI_FMT_MAP)) {
                glyphlist_files.push_back(g);
                if (verbose)
                    errh->message("glyphlist.txt found with kpathsea at %s", g.c_str());
            } else
#endif
                glyphlist_files.push_back(GLYPHLISTDIR "/glyphlist.txt");
#if HAVE_KPATHSEA
            if (String g = kpsei_find_file("texglyphlist.txt", KPSEI_FMT_MAP)) {
                glyphlist_files.push_back(g);
                if (verbose)
                    errh->message("texglyphlist.txt found with kpathsea at %s", g.c_str());
            } else
#endif
                glyphlist_files.push_back(GLYPHLISTDIR "/texglyphlist.txt");
        }

        // read glyphlist
        for (String *g = glyphlist_files.begin(); g < glyphlist_files.end(); g++)
            if (String s = read_file(*g, errh, true))
                DvipsEncoding::add_glyphlist(s);

        // read base encodings
        for (String *s = base_encoding_files.begin(); s < base_encoding_files.end(); s++)
            parse_base_encodings(*s, errh);

        // read encoding
        DvipsEncoding dvipsenc;
        if (encoding_file) {
            if (String path = locate_encoding(encoding_file, errh))
                dvipsenc.parse(path, no_ecommand, no_ecommand, errh);
            else
                errh->fatal("encoding %<%s%> not found", encoding_file.c_str());
        } else {
            String cff_data(otf.table("CFF"));
            if (!cff_data) {
                errh->error("explicit encoding required for TrueType fonts");
                errh->message("(Use %<-e ENCODING%> to choose an encoding. %<-e texnansx%> often works.)");
                exit(1);
            } else if (!have_encoding_file) {
                errh->warning("no encoding provided");
                errh->message("(Use %<-e ENCODING%> to choose an encoding. %<-e texnansx%> often works,\nor say %<-e -%> to turn off this warning.)");
            }

            // use encoding from font
            Cff cff(cff_data, otf.units_per_em(), &bail_errh);
            Cff::FontParent *font = cff.font(PermString(), &bail_errh);
            assert(cff.ok() && font->ok());
            if (Type1Encoding *t1e = font->type1_encoding()) {
                for (int i = 0; i < 256; i++)
                    dvipsenc.encode(i, (*t1e)[i]);
            } else
                errh->fatal("font has no encoding, specify one explicitly");
        }

        // apply default ligkern commands
        if (default_ligkern)
            dvipsenc.parse_ligkern(default_ligkerns, 0, ErrorHandler::silent_handler());

        // apply command-line ligkern commands and coding scheme
        cerrh.set_landmark("--ligkern command");
        for (int i = 0; i < ligkern.size(); i++)
            dvipsenc.parse_ligkern(ligkern[i], 1, &cerrh);
        cerrh.set_landmark("--position command");
        for (int i = 0; i < pos.size(); i++)
            dvipsenc.parse_position(pos[i], 1, &cerrh);
        cerrh.set_landmark("--unicoding command");
        for (int i = 0; i < unicoding.size(); i++)
            dvipsenc.parse_unicoding(unicoding[i], 1, &cerrh);
        if (codingscheme)
            dvipsenc.set_coding_scheme(codingscheme);
        if (warn_missing >= 0)
            dvipsenc.set_warn_missing(warn_missing);

        do_file(input_file, otf, dvipsenc, literal_encoding, errh);

    } catch (OpenType::Error e) {
        errh->error("unhandled exception %<%s%>", e.description.c_str());
    }

    for (int i = 0; i < allocated_filters.size(); ++i)
        delete allocated_filters[i];
    Clp_DeleteParser(clp);
    return (errh->nerrors() == 0 ? 0 : 1);
}
