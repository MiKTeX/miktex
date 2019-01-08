/*

Copyright 2006-2010 Taco Hoekwater <taco@luatex.org>

This file is part of LuaTeX.

LuaTeX is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.

LuaTeX is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU General Public License along
with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

#include "ptexlib.h"
#include "lua/luatex-api.h"
#include "font/writecff.h"

extern int cidset;

#define get_offset(s,n) get_unsigned(s, (n))
#define get_card8(a)  (card8)(a->stream[a->offset++])
#define get_card16(a) (card16)(get_unsigned(a,2))
#define get_card32(a) (get_unsigned(a,4))

#undef b0
#undef b1
#undef b2
#undef b3

#define WORK_BUFFER_SIZE 1024

static char work_buffer[WORK_BUFFER_SIZE];

static unsigned long get_unsigned(cff_font * cff, int n)
{
    unsigned long v = 0;
    while (n-- > 0)
        v = v * 256 + get_card8(cff);
    return v;
}

const char *const cff_stdstr[CFF_STDSTR_MAX] = {
    ".notdef", "space", "exclam", "quotedbl", "numbersign",
    "dollar", "percent", "ampersand", "quoteright", "parenleft",
    "parenright", "asterisk", "plus", "comma", "hyphen",
    "period", "slash", "zero", "one", "two",
    "three", "four", "five", "six", "seven",
    "eight", "nine", "colon", "semicolon", "less",
    "equal", "greater", "question", "at", "A",
    "B", "C", "D", "E", "F",
    "G", "H", "I", "J", "K",
    "L", "M", "N", "O", "P",
    "Q", "R", "S", "T", "U",
    "V", "W", "X", "Y", "Z",
    "bracketleft", "backslash", "bracketright", "asciicircum", "underscore",
    "quoteleft", "a", "b", "c", "d",
    "e", "f", "g", "h", "i",
    "j", "k", "l", "m", "n",
    "o", "p", "q", "r", "s",
    "t", "u", "v", "w", "x",
    "y", "z", "braceleft", "bar", "braceright",
    "asciitilde", "exclamdown", "cent", "sterling", "fraction",
    "yen", "florin", "section", "currency", "quotesingle",
    "quotedblleft", "guillemotleft", "guilsinglleft", "guilsinglright", "fi",
    "fl", "endash", "dagger", "daggerdbl", "periodcentered",
    "paragraph", "bullet", "quotesinglbase", "quotedblbase", "quotedblright",
    "guillemotright", "ellipsis", "perthousand", "questiondown", "grave",
    "acute", "circumflex", "tilde", "macron", "breve",
    "dotaccent", "dieresis", "ring", "cedilla", "hungarumlaut",
    "ogonek", "caron", "emdash", "AE", "ordfeminine",
    "Lslash", "Oslash", "OE", "ordmasculine", "ae",
    "dotlessi", "lslash", "oslash", "oe", "germandbls",
    "onesuperior", "logicalnot", "mu", "trademark", "Eth",
    "onehalf", "plusminus", "Thorn", "onequarter", "divide",
    "brokenbar", "degree", "thorn", "threequarters", "twosuperior",
    "registered", "minus", "eth", "multiply", "threesuperior",
    "copyright", "Aacute", "Acircumflex", "Adieresis", "Agrave",
    "Aring", "Atilde", "Ccedilla", "Eacute", "Ecircumflex",
    "Edieresis", "Egrave", "Iacute", "Icircumflex", "Idieresis",
    "Igrave", "Ntilde", "Oacute", "Ocircumflex", "Odieresis",
    "Ograve", "Otilde", "Scaron", "Uacute", "Ucircumflex",
    "Udieresis", "Ugrave", "Yacute", "Ydieresis", "Zcaron",
    "aacute", "acircumflex", "adieresis", "agrave", "aring",
    "atilde", "ccedilla", "eacute", "ecircumflex", "edieresis",
    "egrave", "iacute", "icircumflex", "idieresis", "igrave",
    "ntilde", "oacute", "ocircumflex", "odieresis", "ograve",
    "otilde", "scaron", "uacute", "ucircumflex", "udieresis",
    "ugrave", "yacute", "ydieresis", "zcaron", "exclamsmall",
    "Hungarumlautsmall", "dollaroldstyle", "dollarsuperior", "ampersandsmall",
    "Acutesmall",
    "parenleftsuperior", "parenrightsuperior", "twodotenleader",
    "onedotenleader", "zerooldstyle",
    "oneoldstyle", "twooldstyle", "threeoldstyle", "fouroldstyle",
    "fiveoldstyle",
    "sixoldstyle", "sevenoldstyle", "eightoldstyle", "nineoldstyle",
    "commasuperior",
    "threequartersemdash", "periodsuperior", "questionsmall", "asuperior",
    "bsuperior",
    "centsuperior", "dsuperior", "esuperior", "isuperior", "lsuperior",
    "msuperior", "nsuperior", "osuperior", "rsuperior", "ssuperior",
    "tsuperior", "ff", "ffi", "ffl", "parenleftinferior",
    "parenrightinferior", "Circumflexsmall", "hyphensuperior", "Gravesmall",
    "Asmall",
    "Bsmall", "Csmall", "Dsmall", "Esmall", "Fsmall",
    "Gsmall", "Hsmall", "Ismall", "Jsmall", "Ksmall",
    "Lsmall", "Msmall", "Nsmall", "Osmall", "Psmall",
    "Qsmall", "Rsmall", "Ssmall", "Tsmall", "Usmall",
    "Vsmall", "Wsmall", "Xsmall", "Ysmall", "Zsmall",
    "colonmonetary", "onefitted", "rupiah", "Tildesmall", "exclamdownsmall",
    "centoldstyle", "Lslashsmall", "Scaronsmall", "Zcaronsmall",
    "Dieresissmall",
    "Brevesmall", "Caronsmall", "Dotaccentsmall", "Macronsmall", "figuredash",
    "hypheninferior", "Ogoneksmall", "Ringsmall", "Cedillasmall",
    "questiondownsmall",
    "oneeighth", "threeeighths", "fiveeighths", "seveneighths", "onethird",
    "twothirds", "zerosuperior", "foursuperior", "fivesuperior", "sixsuperior",
    "sevensuperior", "eightsuperior", "ninesuperior", "zeroinferior",
    "oneinferior",
    "twoinferior", "threeinferior", "fourinferior", "fiveinferior",
    "sixinferior",
    "seveninferior", "eightinferior", "nineinferior", "centinferior",
    "dollarinferior",
    "periodinferior", "commainferior", "Agravesmall", "Aacutesmall",
    "Acircumflexsmall",
    "Atildesmall", "Adieresissmall", "Aringsmall", "AEsmall", "Ccedillasmall",
    "Egravesmall", "Eacutesmall", "Ecircumflexsmall", "Edieresissmall",
    "Igravesmall",
    "Iacutesmall", "Icircumflexsmall", "Idieresissmall", "Ethsmall",
    "Ntildesmall",
    "Ogravesmall", "Oacutesmall", "Ocircumflexsmall", "Otildesmall",
    "Odieresissmall",
    "OEsmall", "Oslashsmall", "Ugravesmall", "Uacutesmall", "Ucircumflexsmall",
    "Udieresissmall", "Yacutesmall", "Thornsmall", "Ydieresissmall",
    "001.000", "001.001", "001.002", "001.003",
    "Black", "Bold", "Book", "Light", "Medium", "Regular", "Roman", "Semibold"
};

/*tex Only read the header part and forget about the body */

cff_index *cff_get_index_header(cff_font * cff)
{
    cff_index *idx;
    card16 i, count;
    idx = xcalloc(1, sizeof(cff_index));
    if (cff->header_major == 2) {
        idx->count = count = get_card32(cff);
    } else {
        idx->count = count = get_card16(cff);
    }
    if (count > 0) {
        idx->offsize = get_card8(cff);
        if (idx->offsize < 1 || idx->offsize > 4)
            normal_error("cff","invalid offsize data (1)");
        idx->offset = xmalloc((unsigned) (((unsigned) count + 1) * sizeof(l_offset)));
        for (i = 0; i <count + 1 ; i++) {
            (idx->offset)[i] = get_offset(cff, idx->offsize);
            if (i == USHRT_MAX)
                break;
        }
        if (idx->offset[0] != 1)
            normal_error("cff","invalid index data");
        idx->data = NULL;
    } else {
        idx->offsize = 0;
        idx->offset = NULL;
        idx->data = NULL;
    }
    return idx;
}

cff_index *cff_get_index(cff_font * cff)
{
    cff_index *idx;
    card16 i, count;
    size_t length;
    idx = xcalloc(1, sizeof(cff_index));
    idx->count = count = get_card16(cff);
    if (count > 0) {
        idx->offsize = get_card8(cff);
        if (idx->offsize < 1 || idx->offsize > 4)
            normal_error("cff","invalid offsize data (2)");
        idx->offset = xmalloc((unsigned) (((unsigned) count + 1) * sizeof(l_offset)));
        for (i = 0; i < count + 1; i++) {
            idx->offset[i] = get_offset(cff, idx->offsize);
        }
        if (idx->offset[0] != 1)
            normal_error("cff","invalid index offset data");
        length = (size_t) (idx->offset[count] - idx->offset[0]);
        idx->data = xmalloc((unsigned) length * sizeof(card8));
        memcpy(idx->data, &cff->stream[cff->offset], length);
        cff->offset += length;
    } else {
        idx->offsize = 0;
        idx->offset = NULL;
        idx->data = NULL;
    }
    return idx;
}

static cff_index *cff_empty_index(cff_font * cff)
{
    cff_index *idx;
    idx = xcalloc(1, sizeof(cff_index));
    idx->count = 0;
    idx->offsize = 0;
    idx->offset = NULL;
    idx->data = NULL;
    return idx;
}

static cff_index *cff_get_index2(cff_font * cff)
{
    /*tex We fake a dict array. */
    cff_index *idx;
    size_t length;
    idx = xcalloc(1, sizeof(cff_index));
    length = (size_t) cff->header_offsize;
    idx->offsize = 2;
    idx->count = 1;
    idx->offset = xmalloc((unsigned) (((unsigned) 2) * sizeof(l_offset)));
    idx->offset[0] = 1;
    idx->offset[1] = length + 1;
    idx->data = xmalloc((unsigned) length * sizeof(card8));
    memcpy(idx->data, &cff->stream[cff->offset], length );
    cff->offset += length ;
    return idx;
}

long cff_pack_index(cff_index * idx, card8 * dest, long destlen)
{
    long len = 0;
    unsigned long datalen;
    card16 i;
    if (idx->count < 1) {
        if (destlen < 2)
            normal_error("cff","not enough space available");
        memset(dest, 0, 2);
        return 2;
    }
    len = cff_index_size(idx);
    datalen = idx->offset[idx->count] - 1;
    if (destlen < len)
        normal_error("cff","not enough space available");
    *(dest++) = (card8) ((idx->count >> 8) & 0xff);
    *(dest++) = (card8) (idx->count & 0xff);
    if (datalen < 0xffUL) {
        idx->offsize = 1;
        *(dest++) = 1;
        for (i = 0; i <= idx->count; i++) {
            *(dest++) = (card8) (idx->offset[i] & 0xff);
        }
    } else if (datalen < 0xffffUL) {
        idx->offsize = 2;
        *(dest++) = 2;
        for (i = 0; i <= idx->count; i++) {
            *(dest++) = (card8) ((idx->offset[i] >> 8) & 0xff);
            *(dest++) = (card8) (idx->offset[i] & 0xff);
        }
    } else if (datalen < 0xffffffUL) {
        idx->offsize = 3;
        *(dest++) = 3;
        for (i = 0; i <= idx->count; i++) {
            *(dest++) = (card8) ((idx->offset[i] >> 16) & 0xff);
            *(dest++) = (card8) ((idx->offset[i] >> 8) & 0xff);
            *(dest++) = (card8) (idx->offset[i] & 0xff);
        }
    } else {
        idx->offsize = 4;
        *(dest++) = 4;
        for (i = 0; i <= idx->count; i++) {
            *(dest++) = (card8) ((idx->offset[i] >> 24) & 0xff);
            *(dest++) = (card8) ((idx->offset[i] >> 16) & 0xff);
            *(dest++) = (card8) ((idx->offset[i] >> 8) & 0xff);
            *(dest++) = (card8) (idx->offset[i] & 0xff);
        }
    }
    memmove(dest, idx->data, idx->offset[idx->count] - 1);
    return len;
}

long cff_index_size(cff_index * idx)
{
    if (idx->count > 0) {
        l_offset datalen;
        datalen = idx->offset[idx->count] - 1;
        if (datalen < 0xffUL) {
            idx->offsize = 1;
        } else if (datalen < 0xffffUL) {
            idx->offsize = 2;
        } else if (datalen < 0xffffffUL) {
            idx->offsize = 3;
        } else {
            idx->offsize = 4;
        }
        return (3 + (idx->offsize) * (idx->count + 1) + (long) datalen);
    } else {
        return 2;
    }
}

cff_index *cff_new_index(card16 count)
{
    cff_index *idx;
    idx = xcalloc(1, sizeof(cff_index));
    idx->count = count;
    idx->offsize = 0;
    if (count > 0) {
        idx->offset = xcalloc((unsigned) (count + 1), sizeof(l_offset));
        (idx->offset)[0] = 1;
    } else {
        idx->offset = NULL;
    }
    idx->data = NULL;
    return idx;
}

void cff_release_index(cff_index * idx)
{
    if (idx) {
        xfree(idx->data);
        xfree(idx->offset);
        xfree(idx);
    }
}

void cff_release_dict(cff_dict * dict)
{
    if (dict) {
        if (dict->entries) {
            int i;
            for (i = 0; i < dict->count; i++) {
                xfree((dict->entries)[i].values);
            }
            xfree(dict->entries);
        }
        xfree(dict);
    }
}

void cff_release_encoding(cff_encoding * encoding)
{
    if (encoding) {
        switch (encoding->format & (~0x80)) {
        case 0:
            xfree(encoding->data.codes);
            break;
        case 1:
            xfree(encoding->data.range1);
            break;
        default:
            normal_error("cff","unknown encoding format");
        }
        if (encoding->format & 0x80)
            xfree(encoding->supp);
        xfree(encoding);
    }
}

void cff_release_charsets(cff_charsets * charset)
{
    if (charset) {
        switch (charset->format) {
        case 0:
            xfree(charset->data.glyphs);
            break;
        case 1:
            xfree(charset->data.range1);
            break;
        case 2:
            xfree(charset->data.range2);
            break;
        default:
            break;
        }
        xfree(charset);
    }
}

void cff_release_fdselect(cff_fdselect * fdselect)
{
    if (fdselect) {
        if (fdselect->format == 0) {
            xfree(fdselect->data.fds);
        } else if (fdselect->format == 3) {
            xfree(fdselect->data.ranges);
        }
        xfree(fdselect);
    }
}

void cff_close(cff_font * cff)
{
    card16 i;
    if (cff) {
        xfree(cff->fontname);
        if (cff->name)
            cff_release_index(cff->name);
        if (cff->topdict)
            cff_release_dict(cff->topdict);
        if (cff->string)
            cff_release_index(cff->string);
        if (cff->gsubr)
            cff_release_index(cff->gsubr);
        if (cff->encoding)
            cff_release_encoding(cff->encoding);
        if (cff->charsets)
            cff_release_charsets(cff->charsets);
        if (cff->fdselect)
            cff_release_fdselect(cff->fdselect);
        if (cff->cstrings)
            cff_release_index(cff->cstrings);
        if (cff->fdarray) {
            for (i = 0; i < cff->num_fds; i++) {
                if (cff->fdarray[i])
                    cff_release_dict(cff->fdarray[i]);
            }
            xfree(cff->fdarray);
        }
        if (cff->private) {
            for (i = 0; i < cff->num_fds; i++) {
                if (cff->private[i])
                    cff_release_dict(cff->private[i]);
            }
            xfree(cff->private);
        }
        if (cff->subrs) {
            for (i = 0; i < cff->num_fds; i++) {
                if (cff->subrs[i])
                    cff_release_index(cff->subrs[i]);
            }
            xfree(cff->subrs);
        }
        if (cff->_string)
            cff_release_index(cff->_string);
        xfree(cff);
    }
    return;
}

char *cff_get_name(cff_font * cff)
{
    char *fontname;
    l_offset len;
    cff_index *idx;
    idx = cff->name;
    len = idx->offset[cff->index + 1] - idx->offset[cff->index];
    fontname = xmalloc((unsigned) (len + 1) * sizeof(char));
    memcpy(fontname, idx->data + idx->offset[cff->index] - 1, len);
    fontname[len] = '\0';
    return fontname;
}

long cff_set_name(cff_font * cff, char *name)
{
    cff_index *idx;
    if (strlen(name) > 127)
        normal_error("cff","FontName string length too large");
    if (cff->name)
        cff_release_index(cff->name);
    cff->name = idx = xcalloc(1, sizeof(cff_index));
    idx->count = 1;
    idx->offsize = 1;
    idx->offset = xmalloc(2 * sizeof(l_offset));
    (idx->offset)[0] = 1;
    (idx->offset)[1] = strlen(name) + 1;
    idx->data = xmalloc((unsigned) strlen(name) * sizeof(card8));
    /*tex No trailing |\0| */
    memmove(idx->data, name, strlen(name));
    return (long) (5 + strlen(name));
}

long cff_put_header(cff_font * cff, card8 * dest, long destlen)
{
    if (destlen < 4)
        normal_error("cff","not enough space available");
    /*tex cff->header_major */
    *(dest++) = 1;
    *(dest++) = cff->header_minor;
    *(dest++) = 4;
    /*tex
        Additional data in between header and Name INDEX is ignored. We will set
        all offset (0) to a four-byte integer.
    */
    *(dest++) = 4;
    cff->header_offsize = 4;
    return 4;
}

#define CFF_PARSE_OK                    0
#define CFF_CFF_ERROR_PARSE_CFF_ERROR  -1
#define CFF_CFF_ERROR_STACK_OVERFLOW   -2
#define CFF_CFF_ERROR_STACK_UNDERFLOW  -3
#define CFF_CFF_ERROR_STACK_RANGECHECK -4

#define DICT_ENTRY_MAX 16

cff_dict *cff_new_dict(void)
{
    cff_dict *dict;
    dict = xcalloc(1, sizeof(cff_dict));
    dict->max = DICT_ENTRY_MAX;
    dict->count = 0;
    dict->entries = xcalloc((unsigned) dict->max, sizeof(cff_dict_entry));
    return dict;
}

/*tex

    Operand stack: only numbers are stored (as double). Operand types are:

    \startitemize
    \startitem number:  double (integer or real) \stopitem
    \startitem boolean: stored as a number       \stopitem
    \startitem SID:     stored as a number       \stopitem
    \startitem array:   array of numbers         \stopitem
    \startitem delta:   array of numbers         \stopitem
    \stopitemize

*/

#define CFF_DICT_STACK_LIMIT 64
static int stack_top = 0;
static double arg_stack[CFF_DICT_STACK_LIMIT];

/* The CFF DICT encoding: */

#define CFF_LAST_DICT_OP1 26
#define CFF_LAST_DICT_OP2 39
#define CFF_LAST_DICT_OP (CFF_LAST_DICT_OP1 + CFF_LAST_DICT_OP2)

static struct {
    const char *opname;
    int argtype;
} dict_operator[CFF_LAST_DICT_OP] = {
    { "version", CFF_TYPE_SID },
    { "Notice", CFF_TYPE_SID },
    { "FullName", CFF_TYPE_SID },
    { "FamilyName", CFF_TYPE_SID },
    { "Weight", CFF_TYPE_SID },
    { "FontBBox", CFF_TYPE_ARRAY },
    { "BlueValues", CFF_TYPE_DELTA },
    { "OtherBlues", CFF_TYPE_DELTA },
    { "FamilyBlues", CFF_TYPE_DELTA },
    { "FamilyOtherBlues", CFF_TYPE_DELTA },
    { "StdHW", CFF_TYPE_NUMBER },
    { "StdVW", CFF_TYPE_NUMBER },
    { NULL, -1 },
    { "UniqueID", CFF_TYPE_NUMBER },
    { "XUID", CFF_TYPE_ARRAY },
    { "charset", CFF_TYPE_OFFSET },
    { "Encoding", CFF_TYPE_OFFSET },
    { "CharStrings", CFF_TYPE_OFFSET },
    { "Private", CFF_TYPE_SZOFF },
    { "Subrs", CFF_TYPE_OFFSET },
    { "defaultWidthX", CFF_TYPE_NUMBER },
    { "nominalWidthX", CFF_TYPE_NUMBER },
    { NULL, -1 },
    { NULL, -1 },
    /*tex two CFF2 instructions */
    { "vstore", CFF_TYPE_OFFSET },
    { "maxstack", CFF_TYPE_NUMBER },
    /*tex Here we start with operator 2 of 12. */
    { "Copyright", CFF_TYPE_SID },
    { "IsFixedPitch", CFF_TYPE_BOOLEAN },
    { "ItalicAngle", CFF_TYPE_NUMBER },
    { "UnderlinePosition", CFF_TYPE_NUMBER },
    { "UnderlineThickness", CFF_TYPE_NUMBER },
    { "PaintType", CFF_TYPE_NUMBER },
    { "CharstringType", CFF_TYPE_NUMBER },
    { "FontMatrix", CFF_TYPE_ARRAY },
    { "StrokeWidth", CFF_TYPE_NUMBER },
    { "BlueScale", CFF_TYPE_NUMBER },
    { "BlueShift", CFF_TYPE_NUMBER },
    { "BlueFuzz", CFF_TYPE_NUMBER },
    { "StemSnapH", CFF_TYPE_DELTA },
    { "StemSnapV", CFF_TYPE_DELTA },
    { "ForceBold", CFF_TYPE_BOOLEAN },
    { NULL, -1 },
    { NULL, -1 },
    { "LanguageGroup", CFF_TYPE_NUMBER },
    { "ExpansionFactor", CFF_TYPE_NUMBER },
    { "InitialRandomSeed", CFF_TYPE_NUMBER },
    { "SyntheticBase", CFF_TYPE_NUMBER },
    { "PostScript", CFF_TYPE_SID },
    { "BaseFontName", CFF_TYPE_SID },
    { "BaseFontBlend", CFF_TYPE_DELTA },
    { NULL, -1 },
    { NULL, -1 },
    { NULL, -1 },
    { NULL, -1 },
    { NULL, -1 },
    { NULL, -1 },
    { "ROS", CFF_TYPE_ROS },
    { "CIDFontVersion", CFF_TYPE_NUMBER },
    { "CIDFontRevision", CFF_TYPE_NUMBER },
    { "CIDFontType", CFF_TYPE_NUMBER },
    { "CIDCount", CFF_TYPE_NUMBER },
    { "UIDBase", CFF_TYPE_NUMBER },
    { "FDArray", CFF_TYPE_OFFSET },
    { "FDSelect", CFF_TYPE_OFFSET },
    { "FontName", CFF_TYPE_SID }
};

/*tex Parse DICT data */

static double get_integer(card8 ** data, card8 * endptr, int *status)
{
    long result = 0;
    card8 b0, b1, b2;

    b0 = *(*data)++;
    if (b0 == 28 && *data < endptr - 2) {
        /*tex shortint */
        b1 = *(*data)++;
        b2 = *(*data)++;
        result = b1 * 256 + b2;
        if (result > 0x7fffL)
            result -= 0x10000L;
    } else if (b0 == 29 && *data < endptr - 4) {
        /*tex longint */
        int i;
        result = *(*data)++;
        if (result > 0x7f)
            result -= 0x100;
        for (i = 0; i < 3; i++) {
            result = result * 256 + (**data);
            *data += 1;
        }
    } else if (b0 >= 32 && b0 <= 246) {
        /*tex int (1) */
        result = b0 - 139;
    } else if (b0 >= 247 && b0 <= 250) {
        /*tex int (2) */
        b1 = *(*data)++;
        result = (b0 - 247) * 256 + b1 + 108;
    } else if (b0 >= 251 && b0 <= 254) {
        b1 = *(*data)++;
        result = -(b0 - 251) * 256 - b1 - 108;
    } else {
        *status = CFF_CFF_ERROR_PARSE_CFF_ERROR;
    }
    return (double) result;
}

/*tex Simply uses |strtod|: */

static double get_real(card8 ** data, card8 * endptr, int *status)
{
    double result = 0.0;
    int nibble = 0, pos = 0;
    int len = 0, fail = 0;

    if (**data != 30 || *data >= endptr - 1) {
        *status = CFF_CFF_ERROR_PARSE_CFF_ERROR;
        return 0.0;
    }
    /*tex Skip the first byte (30): */
    *data += 1;
    pos = 0;
    while ((!fail) && len < WORK_BUFFER_SIZE - 2 && *data < endptr) {
        /*tex Get a nibble. */
        if (pos % 2) {
            nibble = **data & 0x0f;
            *data += 1;
        } else {
            nibble = (**data >> 4) & 0x0f;
        }
        if (nibble >= 0x00 && nibble <= 0x09) {
            work_buffer[len++] = (char) (nibble + '0');
        } else if (nibble == 0x0a) {    /* . */
            work_buffer[len++] = '.';
        } else if (nibble == 0x0b || nibble == 0x0c) {
            /*tex E, E- */
            work_buffer[len++] = 'e';
            if (nibble == 0x0c)
                work_buffer[len++] = '-';
        } else if (nibble == 0x0e) {
            /*tex the minus */
            work_buffer[len++] = '-';
        } else if (nibble == 0x0d) {
            /*tex do nothing */
        } else if (nibble == 0x0f) {
            /*tex we're done */
            work_buffer[len++] = '\0';
            if (((pos % 2) == 0) && (**data != 0xff)) {
                fail = 1;
            }
            break;
        } else {
            /*tex invalid */
            fail = 1;
        }
        pos++;
    }
    /*tex the returned values */
    if (fail || nibble != 0x0f) {
        *status = CFF_CFF_ERROR_PARSE_CFF_ERROR;
    } else {
        char *s;
        errno=0;
        result = strtod(work_buffer, &s);
        if ((result==0.0 && work_buffer==s) || errno) {
              /*tex Conversion is not possible. */
             *status = CFF_CFF_ERROR_PARSE_CFF_ERROR;
         }
    }
    return result;
}

/*tex Operators */

static void add_dict(cff_dict * dict, card8 ** data, card8 * endptr, int *status)
{
    int id, argtype, t;
    id = **data;
    if (id == 0x0c) {
        *data += 1;
        if (*data >= endptr ||
            (id = **data + CFF_LAST_DICT_OP1) >= CFF_LAST_DICT_OP) {
            *status = CFF_CFF_ERROR_PARSE_CFF_ERROR;
            return;
        }
    } else if (id >= CFF_LAST_DICT_OP1) {
        *status = CFF_CFF_ERROR_PARSE_CFF_ERROR;
        return;
    }
    argtype = dict_operator[id].argtype;
    if (dict_operator[id].opname == NULL || argtype < 0) {
        *status = CFF_CFF_ERROR_PARSE_CFF_ERROR;
        return;
    }
    if (dict->count >= dict->max) {
        dict->max += DICT_ENTRY_MAX;
        /*tex Not zeroed! */
        dict->entries = xrealloc(dict->entries, (unsigned) ((unsigned) dict->max * sizeof(cff_dict_entry)));
    }
    (dict->entries)[dict->count].id = id;
    (dict->entries)[dict->count].key = dict_operator[id].opname;
    if (argtype == CFF_TYPE_NUMBER ||
        argtype == CFF_TYPE_BOOLEAN ||
        argtype == CFF_TYPE_SID || argtype == CFF_TYPE_OFFSET) {
        /*tex Check for underflow here, as exactly one operand is expected. */
        if (stack_top < 1) {
            *status = CFF_CFF_ERROR_STACK_UNDERFLOW;
            return;
        }
        stack_top--;
        (dict->entries)[dict->count].count = 1;
        (dict->entries)[dict->count].values = xcalloc(1, sizeof(double));
        (dict->entries)[dict->count].values[0] = arg_stack[stack_top];
        dict->count += 1;
    } else {
        /*tex
            Just ignore operator if there were no operands provided. Don't treat
            this as underflow, e.g. |StemSnapV| in |TemporaLGCUni-Italic.otf|.
        */
        if ((t = stack_top) > 0) {
            (dict->entries)[dict->count].count = stack_top;
            (dict->entries)[dict->count].values =
                xmalloc((unsigned) ((unsigned) stack_top * sizeof(double)));
            while (stack_top > 0) {
                stack_top--;
                (dict->entries)[dict->count].values[stack_top] =
                    arg_stack[stack_top];
            }
            if (t > 3 && strcmp(dict_operator[id].opname, "FontMatrix") == 0) {
                (dict->entries)[dict->count].values[0] = 0.001;
                (dict->entries)[dict->count].values[3] = 0.001;
            }
            dict->count += 1;
        }
    }
    *data += 1;
    return;
}

/*tex

    All operands are treated as number or array of numbers.

    \startitemize
    \startitem |Private|: two numbers, size and offset     \stopitem
    \startitem |ROS|: hree numbers, SID, SID, and a number \stopitem
    \stopitemize

*/

cff_dict *cff_dict_unpack(card8 * data, card8 * endptr)
{
    cff_dict *dict;
    int status = CFF_PARSE_OK;
    stack_top = 0;
    dict = cff_new_dict();
    while (data < endptr && status == CFF_PARSE_OK) {
        if (*data < CFF_LAST_DICT_OP1) {
            /*tex Some operator. */
            add_dict(dict, &data, endptr, &status);
        } else if (*data == 30) {
            /*tex First byte of a sequence (variable). */
            if (stack_top < CFF_DICT_STACK_LIMIT) {
                arg_stack[stack_top] = get_real(&data, endptr, &status);
                stack_top++;
            } else {
                status = CFF_CFF_ERROR_STACK_OVERFLOW;
            }
        } else if (*data == 255 || (*data >= CFF_LAST_DICT_OP1 && *data <= 27)) {
            /*tex Reserved. */
            data++;
        } else {
            /*tex Everything else is an integer. */
            if (stack_top < CFF_DICT_STACK_LIMIT) {
                arg_stack[stack_top] = get_integer(&data, endptr, &status);
                stack_top++;
            } else {
                status = CFF_CFF_ERROR_STACK_OVERFLOW;
            }
        }
    }
    if (status != CFF_PARSE_OK) {
        formatted_error("cff","parsing DICT failed (error=%d)", status);
    } else if (stack_top != 0) {
        normal_warning("cff","garbage in DICT data");
        stack_top = 0;
    }
    return dict;
}

int cff_dict_known(cff_dict * dict, const char *key)
{
    int i;
    for (i = 0; i < dict->count; i++) {
        if (key && strcmp(key, (dict->entries)[i].key) == 0
            && (dict->entries)[i].count > 0)
            return 1;
    }
    return 0;
}

double cff_dict_get(cff_dict * dict, const char *key, int idx)
{
    double value = 0.0;
    int i;
    assert(key && dict);
    for (i = 0; i < dict->count; i++) {
        if (strcmp(key, (dict->entries)[i].key) == 0) {
           if ((dict->entries)[i].count > idx)
                value = (dict->entries)[i].values[idx];
            else
                normal_error("cff","invalid index number");
            break;
        }
    }
    if (i == dict->count)
        formatted_error("cff","DICT entry '%s' not found", key);
    return value;
}

card8 cff_fdselect_lookup(cff_font * cff, card16 gid)
{
    card8 fd = 0xff;
    cff_fdselect *fdsel;
    if (cff->fdselect == NULL)
        normal_error("cff","FDSelect not available");
    fdsel = cff->fdselect;
    if (gid >= cff->num_glyphs)
        normal_error("cff","invalid glyph index");
    switch (fdsel->format) {
        case 0:
            fd = fdsel->data.fds[gid];
            break;
        case 3:
            {
                if (gid == 0) {
                    fd = (fdsel->data).ranges[0].fd;
                } else {
                    card16 i;
                    for (i = 1; i < (fdsel->num_entries); i++) {
                        if (gid < (fdsel->data).ranges[i].first)
                            break;
                    }
                    fd = (fdsel->data).ranges[i - 1].fd;
                }
            }
            break;
        default:
            normal_error("cff","invalid FDSelect format");
            break;
    }
    if (fd >= cff->num_fds)
        normal_error("cff","invalid Font DICT index");
    return fd;
}

long cff_read_subrs(cff_font * cff)
{
    long len = 0;
    long offset;
    int i;
    if ((cff->flag & FONTTYPE_CIDFONT) && cff->fdselect == NULL) {
        cff_read_fdselect(cff);
    }
    if ((cff->flag & FONTTYPE_CIDFONT) && cff->fdarray == NULL) {
        cff_read_fdarray(cff);
    }
    if (cff->private == NULL)
        cff_read_private(cff);
    if (cff->gsubr == NULL) {
        cff->offset = cff->gsubr_offset;
        cff->gsubr = cff_get_index(cff);
    }
    cff->subrs = xcalloc(cff->num_fds, sizeof(cff_index *));
    if (cff->flag & FONTTYPE_CIDFONT) {
        for (i = 0; i < cff->num_fds; i++) {
            if (cff->private[i] == NULL ||
                !cff_dict_known(cff->private[i], "Subrs")) {
                (cff->subrs)[i] = NULL;
            } else {
                offset = (long) cff_dict_get(cff->fdarray[i], "Private", 1);
                offset += (long) cff_dict_get(cff->private[i], "Subrs", 0);
                cff->offset = (l_offset) offset;
                (cff->subrs)[i] = cff_get_index(cff);
                len += cff_index_size((cff->subrs)[i]);
            }
        }
    } else if (cff->private[0] == NULL || !cff_dict_known(cff->private[0], "Subrs")) {
        (cff->subrs)[0] = NULL;
    } else {
        offset = (long) cff_dict_get(cff->topdict, "Private", 1);
        offset += (long) cff_dict_get(cff->private[0], "Subrs", 0);
        cff->offset = (l_offset) offset;
        (cff->subrs)[0] = cff_get_index(cff);
        len += cff_index_size((cff->subrs)[0]);
    }
    return len;
}

long cff_read_fdarray(cff_font * cff)
{
    long len = 0;
    cff_index *idx;
    long offset, size;
    card16 i;
    if (cff->topdict == NULL)
        normal_error("cff","top DICT not found");
    if (!(cff->flag & FONTTYPE_CIDFONT))
        return 0;
    offset = (long) cff_dict_get(cff->topdict, "FDArray", 0);
    cff->offset = (l_offset) offset;
    idx = cff_get_index(cff);
    cff->num_fds = (card8) idx->count;
    cff->fdarray = xmalloc((unsigned) (idx->count * sizeof(cff_dict *)));
    for (i = 0; i < idx->count; i++) {
        card8 *data = idx->data + (idx->offset)[i] - 1;
        size = (long) ((idx->offset)[i + 1] - (idx->offset)[i]);
        if (size > 0) {
            (cff->fdarray)[i] = cff_dict_unpack(data, data + size);
        } else {
            (cff->fdarray)[i] = NULL;
        }
    }
    len = cff_index_size(idx);
    cff_release_index(idx);
    return len;
}

long cff_read_private(cff_font * cff)
{
    long len = 0;
    card8 *data;
    long offset, size;
    if (cff->flag & FONTTYPE_CIDFONT) {
        int i;
        if (cff->fdarray == NULL)
            cff_read_fdarray(cff);
        cff->private = xmalloc((unsigned) (cff->num_fds * sizeof(cff_dict *)));
        for (i = 0; i < cff->num_fds; i++) {
            if (cff->fdarray[i] != NULL &&
                    cff_dict_known(cff->fdarray[i], "Private") &&
                    (size = (long) cff_dict_get(cff->fdarray[i], "Private", 0)) > 0) {
                offset = (long) cff_dict_get(cff->fdarray[i], "Private", 1);
                cff->offset = (l_offset) offset;
                data = xmalloc((unsigned) size * sizeof(card8));
                memcpy(data, &cff->stream[cff->offset], (size_t) size);
                cff->offset = (l_offset) size;
                (cff->private)[i] = cff_dict_unpack(data, data + size);
                xfree(data);
                len += size;
            } else {
                (cff->private)[i] = NULL;
            }
        }
    } else {
        cff->num_fds = 1;
        cff->private = xmalloc(sizeof(cff_dict *));
        if (cff_dict_known(cff->topdict, "Private") &&
                (size = (long) cff_dict_get(cff->topdict, "Private", 0)) > 0) {
            offset = (long) cff_dict_get(cff->topdict, "Private", 1);
            cff->offset = (l_offset) offset;
            data = xmalloc((unsigned) size * sizeof(card8));
            memcpy(data, &cff->stream[cff->offset], (size_t) size);
            cff->offset = (l_offset) size;
            cff->private[0] = cff_dict_unpack(data, data + size);
            xfree(data);
            len += size;
        } else {
            (cff->private)[0] = NULL;
            len = 0;
        }
    }
    return len;
}

cff_font *read_cff(unsigned char *buf, long buflength, int n)
{
    cff_font *cff;
    cff_index *idx;
    long offset;
    cff = xcalloc(1, sizeof(cff_font));
    cff->stream = buf;
    cff->stream_size = (l_offset) buflength;
    cff->index = n;
    cff->header_major = get_card8(cff);
    cff->header_minor = get_card8(cff);
    cff->header_hdr_size = get_card8(cff);
    if (cff->header_major == 2) {
        /*tex We have only one top dictionary. */
        cff->header_offsize = get_card16(cff);
    } else {
        cff->header_offsize = get_card8(cff);
        if (cff->header_offsize < 1 || cff->header_offsize > 4) {
            normal_warning("cff","invalid offsize data (4)");
            cff_close(cff);
            return NULL;
        }
    }
    if (cff->header_major > 2) {
        formatted_warning("cff","major version %u not supported", cff->header_major);
        cff_close(cff);
        return NULL;
    }
    cff->offset = cff->header_hdr_size;
    /*tex The name index. */
    if (cff->header_major == 2) {
        cff->name = cff_empty_index(cff);
    } else {
        idx = cff_get_index(cff);
        if (n > idx->count - 1) {
            normal_warning("cff","invalid fontset index number");
            cff_close(cff);
            return NULL;
        }
        cff->name = idx;
        cff->fontname = cff_get_name(cff);
    }
    /*tex The top dict index. */
    if (cff->header_major == 2) {
        /*tex we fake an index (just one entry) */
        idx = cff_get_index2(cff);
    } else {
        idx = cff_get_index(cff);
    }
    if (n > idx->count - 1) {
        normal_warning("cff","top DICT not exist");
        cff_close(cff);
        return NULL;
    }
    cff->topdict = cff_dict_unpack(idx->data + idx->offset[n] - 1, idx->data + idx->offset[n + 1] - 1);
    if (!cff->topdict) {
        normal_warning("cff","parsing top DICT data failed");
        cff_close(cff);
        return NULL;
    }
    cff_release_index(idx);
    if (cff_dict_known(cff->topdict, "CharstringType") &&
        cff_dict_get(cff->topdict, "CharstringType", 0) != 2) {
        normal_warning("cff","only type 2 charstrings supported");
        cff_close(cff);
        return NULL;
    }
    if (cff_dict_known(cff->topdict, "SyntheticBase")) {
        normal_warning("cff","synthetic font not supported");
        cff_close(cff);
        return NULL;
    }
    /*tex The string index. */
    if (cff->header_major == 2) {
        /*tex do nothing */
    } else {
        cff->string = cff_get_index(cff);
    }
    /*tex The offset to subroutines. */
    cff->gsubr_offset = cff->offset;
    /*tex The number of glyphs. */
    offset = (long) cff_dict_get(cff->topdict, "CharStrings", 0);
    cff->offset = (l_offset) offset;
    cff->num_glyphs = get_card16(cff);
    /*tex Check for font type. */
    if (cff_dict_known(cff->topdict, "ROS")) {
        cff->flag |= FONTTYPE_CIDFONT;
    } else {
        cff->flag |= FONTTYPE_FONT;
    }
    /*tex Check for the encoding. */
    if (cff_dict_known(cff->topdict, "Encoding")) {
        offset = (long) cff_dict_get(cff->topdict, "Encoding", 0);
        if (offset == 0) {      /* predefined */
            cff->flag |= ENCODING_STANDARD;
        } else if (offset == 1) {
            cff->flag |= ENCODING_EXPERT;
        }
    } else {
        cff->flag |= ENCODING_STANDARD;
    }
    cff->offset = cff->gsubr_offset;
    return cff;
}

/*tex Write CFF data for an \OPENTYPE\ font. We need to pack dictionary data. */

static long pack_integer(card8 * dest, long destlen, long value)
{
    long len = 0;
    if (value >= -107 && value <= 107) {
        if (destlen < 1)
            normal_error("cff","buffer overflow (1)");
        dest[0] = (card8) ((value + 139) & 0xff);
        len = 1;
    } else if (value >= 108 && value <= 1131) {
        if (destlen < 2)
            normal_error("cff","buffer overflow (2)");
        value = (long) 0xf700u + value - 108;
        dest[0] = (card8) ((value >> 8) & 0xff);
        dest[1] = (card8) (value & 0xff);
        len = 2;
    } else if (value >= -1131 && value <= -108) {
        if (destlen < 2)
            normal_error("cff","buffer overflow (3)");
        value = (long) 0xfb00u - value - 108;
        dest[0] = (card8) ((value >> 8) & 0xff);
        dest[1] = (card8) (value & 0xff);
        len = 2;
    } else if (value >= -32768 && value <= 32767) {
        /*tex shortint */
        if (destlen < 3)
            normal_error("cff","buffer overflow (4)");
        dest[0] = 28;
        dest[1] = (card8) ((value >> 8) & 0xff);
        dest[2] = (card8) (value & 0xff);
        len = 3;
    } else {
        /*tex longint */
        if (destlen < 5)
            normal_error("cff","buffer overflow (5)");
        dest[0] = 29;
        dest[1] = (card8) ((value >> 24) & 0xff);
        dest[2] = (card8) ((value >> 16) & 0xff);
        dest[3] = (card8) ((value >> 8) & 0xff);
        dest[4] = (card8) (value & 0xff);
        len = 5;
    }
    return len;
}

static long pack_real(card8 * dest, long destlen, double value)
{
    long e;
    int i = 0, pos = 2;
    int res;
#define CFF_REAL_MAX_LEN 17
    if (destlen < 2)
        normal_error("cff","buffer overflow (6)");
    dest[0] = 30;
    if (value == 0.0) {
        dest[1] = 0x0f;
        return 2;
    }
    if (value < 0.0) {
        dest[1] = 0xe0;
        value *= -1.0;
        pos++;
    }
    e = 0;
    if (value >= 10.0) {
        while (value >= 10.0) {
            value /= 10.0;
            e++;
        }
    } else if (value < 1.0) {
        while (value < 1.0) {
            value *= 10.0;
            e--;
        }
    }
    res = sprintf(work_buffer, "%1.14g", value);
    if (res<0)
        normal_error("cff","invalid conversion");
    if (res>CFF_REAL_MAX_LEN)
        res=CFF_REAL_MAX_LEN;
    for (i = 0; i < res; i++) {
        unsigned char ch = 0;
        if (work_buffer[i] == '\0') {
            /*tex In fact |res| should prevent this. */
            break;
        } else if (work_buffer[i] == '.') {
            ch = 0x0a;
        } else if (work_buffer[i] >= '0' && work_buffer[i] <= '9') {
            ch = (unsigned char) (work_buffer[i] - '0');
        } else {
            normal_error("cff","invalid character");
        }
        if (destlen < pos / 2 + 1)
            normal_error("cff","buffer overflow (7)");

        if (pos % 2) {
            dest[pos / 2] = (card8) (dest[pos / 2] + ch);
        } else {
            dest[pos / 2] = (card8) (ch << 4);
        }
        pos++;
    }
    if (e > 0) {
        if (pos % 2) {
            dest[pos / 2] = (card8) (dest[pos / 2] + 0x0b);
        } else {
            if (destlen < pos / 2 + 1)
                normal_error("cff","buffer overflow (8)");
            dest[pos / 2] = (card8) (0xb0);
        }
        pos++;
    } else if (e < 0) {
        if (pos % 2) {
            dest[pos / 2] = (card8) (dest[pos / 2] + 0x0c);
        } else {
            if (destlen < pos / 2 + 1)
                normal_error("cff","buffer overflow (9)");
            dest[pos / 2] = (card8) (0xc0);
        }
        e *= -1;
        pos++;
    }
    if (e != 0) {
        sprintf(work_buffer, "%ld", e);
        for (i = 0; i < CFF_REAL_MAX_LEN; i++) {
            unsigned char ch = 0;
            if (work_buffer[i] == '\0') {
                break;
            } else if (work_buffer[i] == '.') {
                ch = 0x0a;
            } else if (work_buffer[i] >= '0' && work_buffer[i] <= '9') {
                ch = (unsigned char) (work_buffer[i] - '0');
            } else {
                normal_error("cff","invalid character");
            }
            if (destlen < pos / 2 + 1)
                normal_error("cff","buffer overflow (10)");
            if (pos % 2) {
                dest[pos / 2] = (card8) (dest[pos / 2] + ch);
            } else {
                dest[pos / 2] = (card8) (ch << 4);
            }
            pos++;
        }
    }
    if (pos % 2) {
        dest[pos / 2] = (card8) (dest[pos / 2] + 0x0f);
        pos++;
    } else {
        if (destlen < pos / 2 + 1)
            normal_error("cff","buffer overflow (11)");
        dest[pos / 2] = (card8) (0xff);
        pos += 2;
    }
    return pos / 2;
}

static long cff_dict_put_number(double value, card8 * dest, long destlen, int type)
{
    long len = 0;
    double nearint;
    nearint = floor(value + 0.5);
    if (type == CFF_TYPE_OFFSET) {
        long lvalue;
        lvalue = (long) value;
        if (destlen < 5)
            normal_error("cff","buffer overflow (12)");
        dest[0] = 29;
        dest[1] = (card8) ((lvalue >> 24) & 0xff);
        dest[2] = (card8) ((lvalue >> 16) & 0xff);
        dest[3] = (card8) ((lvalue >> 8) & 0xff);
        dest[4] = (card8) (lvalue & 0xff);
        len = 5;
    } else if (value > CFF_INT_MAX || value < CFF_INT_MIN || (fabs(value - nearint) > 1.0e-5)) {
        /*tex A real */
        len = pack_real(dest, destlen, value);
    } else {
        /*tex An integer */
        len = pack_integer(dest, destlen, (long) nearint);
    }
    return len;
}

static long put_dict_entry(cff_dict_entry * de, card8 * dest, long destlen)
{
    long len = 0;
    int i, type, id;
    if (de->count > 0) {
        id = de->id;
        if (dict_operator[id].argtype == CFF_TYPE_OFFSET ||
            dict_operator[id].argtype == CFF_TYPE_SZOFF) {
            type = CFF_TYPE_OFFSET;
        } else {
            type = CFF_TYPE_NUMBER;
        }
        for (i = 0; i < de->count; i++) {
            len += cff_dict_put_number(de->values[i], dest + len, destlen - len, type);
        }
        if (id >= 0 && id < CFF_LAST_DICT_OP1) {
            if (len + 1 > destlen)
                normal_error("cff","buffer overflow (13)");
            dest[len++] = (card8) id;
        } else if (id >= 0 && id < CFF_LAST_DICT_OP) {
            if (len + 2 > destlen)
                normal_error("cff","buffer overflow (14)");
            dest[len++] = 12;
            dest[len++] = (card8) (id - CFF_LAST_DICT_OP1);
        } else {
            normal_error("cff","invalid DICT operator ID");
        }
    }
    return len;
}

long cff_dict_pack(cff_dict * dict, card8 * dest, long destlen)
{
    long len = 0;
    int i;
    for (i = 0; i < dict->count; i++) {
        if (!strcmp(dict->entries[i].key, "ROS")) {
            len += put_dict_entry(&dict->entries[i], dest, destlen);
            break;
        }
    }
    for (i = 0; i < dict->count; i++) {
        if (strcmp(dict->entries[i].key, "ROS")) {
            len += put_dict_entry(&dict->entries[i], dest + len, destlen - len);
        }
    }
    return len;
}

void cff_dict_add(cff_dict * dict, const char *key, int count)
{
    int id, i;
    for (id = 0; id < CFF_LAST_DICT_OP; id++) {
        if (key && dict_operator[id].opname &&
            strcmp(dict_operator[id].opname, key) == 0)
            break;
    }
    if (id == CFF_LAST_DICT_OP)
        normal_error("cff","unknown DICT operator");
    for (i = 0; i < dict->count; i++) {
        if ((dict->entries)[i].id == id) {
            if ((dict->entries)[i].count != count)
                normal_error("cff","inconsistent DICT argument number");
            return;
        }
    }
    if (dict->count + 1 >= dict->max) {
        dict->max += 8;
        dict->entries =
            xrealloc(dict->entries, (unsigned) ((unsigned) dict->max * sizeof(cff_dict_entry)));
    }
    (dict->entries)[dict->count].id = id;
    (dict->entries)[dict->count].key = dict_operator[id].opname;
    (dict->entries)[dict->count].count = count;
    if (count > 0) {
        (dict->entries)[dict->count].values = xcalloc((unsigned) count, sizeof(double));
    } else {
        (dict->entries)[dict->count].values = NULL;
    }
    dict->count += 1;
    return;
}

void cff_dict_remove(cff_dict * dict, const char *key)
{
    int i;
    for (i = 0; i < dict->count; i++) {
        if (key && strcmp(key, (dict->entries)[i].key) == 0) {
            (dict->entries)[i].count = 0;
            xfree((dict->entries)[i].values);
        }
    }
}

void cff_dict_set(cff_dict * dict, const char *key, int idx, double value)
{
    int i;
    for (i = 0; i < dict->count; i++) {
        if (strcmp(key, (dict->entries)[i].key) == 0) {
            if ((dict->entries)[i].count > idx)
                (dict->entries)[i].values[idx] = value;
            else
                normal_error("cff","invalid index number");
            break;
        }
    }
    if (i == dict->count)
        formatted_error("cff","DICT entry '%s' not found", key);
}


/*tex Strings */

char *cff_get_string(cff_font * cff, s_SID id)
{
    char *result = NULL;
    size_t len;
    if (id < CFF_STDSTR_MAX) {
        len = strlen(cff_stdstr[id]);
        result = xmalloc((unsigned) (len + 1) * sizeof(char));
        memcpy(result, cff_stdstr[id], len);
        result[len] = '\0';
    } else if (cff && cff->string) {
        cff_index *strings = cff->string;
        id = (s_SID) (id - CFF_STDSTR_MAX);
        if (id < strings->count) {
            len = (strings->offset)[id + 1] - (strings->offset)[id];
            result = xmalloc((unsigned) (len + 1) * sizeof(char));
            memmove(result, strings->data + (strings->offset)[id] - 1, len);
            result[len] = '\0';
        }
    }
    return result;
}

long cff_get_sid(cff_font * cff, const char *str)
{
    card16 i;
    if (!cff || !str)
        return -1;
    /*tex We search the string index first. */
    if (cff && cff->string) {
        cff_index *idx = cff->string;
        for (i = 0; i < idx->count; i++) {
            if (strlen(str) == (idx->offset)[i + 1] - (idx->offset)[i] &&
                !memcmp(str, (idx->data) + (idx->offset)[i] - 1, strlen(str)))
                return (i + CFF_STDSTR_MAX);
        }
    }
    for (i = 0; i < CFF_STDSTR_MAX; i++) {
        if (!strcmp(str, cff_stdstr[i]))
            return i;
    }
    return -1;
}

void cff_update_string(cff_font * cff)
{
    if (cff == NULL)
        normal_error("cff","CFF font not opened");
    if (cff->string)
        cff_release_index(cff->string);
    cff->string = cff->_string;
    cff->_string = NULL;
}

s_SID cff_add_string(cff_font * cff, const char *str)
{
    card16 idx;
    cff_index *strings;
    l_offset offset, size;
    if (cff == NULL) {
        normal_error("cff","CFF font not opened");
    }
    if (cff->_string == NULL) {
        cff->_string = cff_new_index(0);
    }
    strings = cff->_string;
    for (idx = 0; idx < strings->count; idx++) {
        size = strings->offset[idx + 1] - strings->offset[idx];
        offset = strings->offset[idx];
        if (size == strlen(str) && !memcmp(strings->data + offset - 1, str, strlen(str))) {
            return (s_SID) (idx + CFF_STDSTR_MAX);
        }
    }
    for (idx = 0; idx < CFF_STDSTR_MAX; idx++) {
        if (cff_stdstr[idx] && !strcmp(cff_stdstr[idx], str)) {
            return idx;
        }
    }
    offset = (strings->count > 0) ? strings->offset[strings->count] : 1;
    strings->offset = xrealloc(strings->offset, (unsigned) (((unsigned) strings->count + 2) * sizeof(l_offset)));
    if (strings->count == 0)
        strings->offset[0] = 1;
    idx = strings->count;
    strings->count = (card16) (strings->count + 1);
    strings->offset[strings->count] = offset + strlen(str);
    strings->data = xrealloc(strings->data, (unsigned) ((offset + strlen(str) - 1) * sizeof(card8)));
    memcpy(strings->data + offset - 1, str, strlen(str));
    return (s_SID) (idx + CFF_STDSTR_MAX);
}

void cff_dict_update(cff_dict * dict, cff_font * cff)
{
    int i;
    for (i = 0; i < dict->count; i++) {
        if ((dict->entries)[i].count > 0) {
            char *str;
            int id;
            id = (dict->entries)[i].id;
            if (dict_operator[id].argtype == CFF_TYPE_SID) {
                str = cff_get_string(cff, (s_SID) (dict->entries)[i].values[0]);
                if (str != NULL) {
                    (dict->entries)[i].values[0] = cff_add_string(cff, str);
                    xfree(str);
                }
            } else if (dict_operator[id].argtype == CFF_TYPE_ROS) {
                str = cff_get_string(cff, (s_SID) (dict->entries)[i].values[0]);
                if (str != NULL) {
                    (dict->entries)[i].values[0] = cff_add_string(cff, str);
                    xfree(str);
                }
                str = cff_get_string(cff, (s_SID) (dict->entries)[i].values[1]);
                if (str != NULL) {
                    (dict->entries)[i].values[1] = cff_add_string(cff, str);
                    xfree(str);
                }
            }
        }
    }
}

/*tex The charsets. */

long cff_read_charsets(cff_font * cff)
{
    cff_charsets *charset;
    long offset, length;
    card16 count, i;
    if (cff->topdict == NULL)
        normal_error("cff","top DICT not available");
    if (!cff_dict_known(cff->topdict, "charset")) {
        cff->flag |= CHARSETS_ISOADOBE;
        cff->charsets = NULL;
        return 0;
    }
    offset = (long) cff_dict_get(cff->topdict, "charset", 0);
    if (offset == 0) {
        /*tex predefined */
        cff->flag |= CHARSETS_ISOADOBE;
        cff->charsets = NULL;
        return 0;
    } else if (offset == 1) {
        cff->flag |= CHARSETS_EXPERT;
        cff->charsets = NULL;
        return 0;
    } else if (offset == 2) {
        cff->flag |= CHARSETS_EXPSUB;
        cff->charsets = NULL;
        return 0;
    }
    cff->offset = (l_offset) offset;
    cff->charsets = charset = xcalloc(1, sizeof(cff_charsets));
    charset->format = get_card8(cff);
    charset->num_entries = 0;
    count = (card16) (cff->num_glyphs - 1);
    length = 1;
    /*tex Not well documented. */
    switch (charset->format) {
        case 0:
            charset->num_entries = (card16) (cff->num_glyphs - 1);  /* no .notdef */
            charset->data.glyphs =
                xmalloc((unsigned) (charset->num_entries * sizeof(s_SID)));
            length += (charset->num_entries) * 2;
            for (i = 0; i < (charset->num_entries); i++) {
                charset->data.glyphs[i] = get_card16(cff);
            }
            count = 0;
            break;
        case 1:
            {
                cff_range1 *ranges = NULL;
                while (count > 0 && charset->num_entries < cff->num_glyphs) {
                    ranges =
                        xrealloc(ranges,
                                 (unsigned) (((unsigned) charset->num_entries +
                                              1) * sizeof(cff_range1)));
                    ranges[charset->num_entries].first = get_card16(cff);
                    ranges[charset->num_entries].n_left = get_card8(cff);
                    count = (card16) (count - ranges[charset->num_entries].n_left + 1);     /* no-overrap */
                    charset->num_entries++;
                    charset->data.range1 = ranges;
                }
                length += (charset->num_entries) * 3;
            }
            break;
        case 2:
            {
                cff_range2 *ranges = NULL;
                while (count > 0 && charset->num_entries < cff->num_glyphs) {
                    ranges =
                        xrealloc(ranges,
                                 (unsigned) (((unsigned) charset->num_entries +
                                              1) * sizeof(cff_range2)));
                    ranges[charset->num_entries].first = get_card16(cff);
                    ranges[charset->num_entries].n_left = get_card16(cff);
                    count = (card16) (count - (ranges[charset->num_entries].n_left + 1));   /* non-overrapping */
                    charset->num_entries++;
                }
                charset->data.range2 = ranges;
                length += (charset->num_entries) * 4;
            }
            break;
        default:
            xfree(charset);
            normal_error("cff","unknown charset format");
            break;
    }
    if (count > 0) {
        normal_warning("cff","charset data possibly broken (too many glyphs)");
    }
    return length;
}

long cff_pack_charsets(cff_font * cff, card8 * dest, long destlen)
{
    long len = 0;
    card16 i;
    cff_charsets *charset;
    if (cff->flag & HAVE_STANDARD_CHARSETS || cff->charsets == NULL)
        return 0;
    if (destlen < 1)
        normal_error("cff","buffer overflow (15)");
    charset = cff->charsets;
    dest[len++] = charset->format;
    switch (charset->format) {
        case 0:
            if (destlen < len + (charset->num_entries) * 2)
                normal_error("cff","buffer overflow (16)");
            for (i = 0; i < (charset->num_entries); i++) {
                s_SID sid = (charset->data).glyphs[i];      /* or CID */
                dest[len++] = (card8) ((sid >> 8) & 0xff);
                dest[len++] = (card8) (sid & 0xff);
            }
            break;
        case 1:
            {
                if (destlen < len + (charset->num_entries) * 3)
                    normal_error("cff","buffer overflow (17)");
                for (i = 0; i < (charset->num_entries); i++) {
                    dest[len++] = (card8) (((charset->data).range1[i].first >> 8) & 0xff);
                    dest[len++] = (card8) ((charset->data).range1[i].first & 0xff);
                    dest[len++] = (card8) ((charset->data).range1[i].n_left);
                }
            }
            break;
        case 2:
            {
                if (destlen < len + (charset->num_entries) * 4)
                    normal_error("cff","buffer overflow (18)");
                for (i = 0; i < (charset->num_entries); i++) {
                    dest[len++] = (card8) (((charset->data).range2[i].first >> 8) & 0xff);
                    dest[len++] = (card8) ((charset->data).range2[i].first & 0xff);
                    dest[len++] = (card8) (((charset->data).range2[i].n_left >> 8) & 0xff);
                    dest[len++] = (card8) ((charset->data).range2[i].n_left & 0xff);
                }
            }
            break;
        default:
            normal_error("cff","unknown charset format");
            break;
    }
    return len;
}

/*tex

    Here we decode and encode Type 2 charstring. All local/global subroutine
    calls in a given charstring is replace by the content of subroutine
    charstrings. We do this because some PostScript RIP may have problems with
    sparse subroutine array. Workaround for this is to re-order subroutine array
    so that no gap appears in the subroutine array, or put dummy charstrings that
    contains only `return' in the gap. However, re-ordering of subroutine is
    rather difficult for Type 2 charstrings due to the bias which depends on the
    total number of subroutines. Replacing callgsubr/callsubr calls with the
    content of the corresponding subroutine charstring may be more efficient than
    putting dummy subroutines in the case of subsetted font. Adobe distiller
    seems doing same thing.

    And also note that subroutine numbers within subroutines can depend on the
    content of operand stack as follows:

    \startyping
    \.{  ... l m callsubr << subr \#(m+bias): n add callsubr >> ...}
    \stoptyping

    I've not implemented the `random' operator which generates a pseudo-random
    number in the range (0, 1] and push them into argument stack. How
    pseudo-random sequences are generated is not documented in the Type 2
    charstring spec.

*/

#define CS_TYPE2_DEBUG_STR   "Type2 Charstring Parser"
#define CS_TYPE2_DEBUG       5

#define CS_BUFFER_CFF_ERROR -3
#define CS_STACK_CFF_ERROR  -2
#define CS_PARSE_CFF_ERROR  -1
#define CS_PARSE_OK          0
#define CS_PARSE_END         1
#define CS_SUBR_RETURN       2
#define CS_CHAR_END          3

static int status = CS_PARSE_CFF_ERROR;

#define DST_NEED(a,b) {if ((a) < (b)) { status = CS_BUFFER_CFF_ERROR ; return ; }}
#define SRC_NEED(a,b) {if ((a) < (b)) { status = CS_PARSE_CFF_ERROR  ; return ; }}
#define NEED(a,b)     {if ((a) < (b)) { status = CS_STACK_CFF_ERROR  ; return ; }}

/*tex The hintmask and cntrmask need the number of stem zones. */

static int num_stems = 0;
static int phase = 0;

/*tex Subroutine nesting.
*/
static int cs2_nest = 0;

/*tex The advance width. */

static int have_width = 0;
static double width = 0.0;

/*tex

    Standard Encoding Accented Characters: Optional four arguments for endchar.
    See, CFF spec., p.35. This is obsolete feature and is no longer supported.

    \starttyping
    static double seac[4] = { 0.0, 0.0, 0.0, 0.0 }; // gone
    \stoptyping

*/

/*tex Operand stack and Transient array */

static int cs2_stack_top = 0;
static double cs2_arg_stack[CS_ARG_STACK_MAX];
static double trn_array[CS_TRANS_ARRAY_MAX];

/*tex

    Type 2 CharString encoding, first the 1 byte operators:

*/

/*      RESERVED       0 */
#define cs_hstem       1
/*      RESERVED       2 */
#define cs_vstem       3
#define cs_vmoveto     4
#define cs_rlineto     5
#define cs_hlineto     6
#define cs_vlineto     7
#define cs_rrcurveto   8
/*      cs_closepath   9  */
#define cs_callsubr   10
#define cs_return     11
#define cs_escape     12
/*      cs_hsbw       13 */
#define cs_endchar    14
#define cs_setvsindex 15
#define cs_blend      16
/*      RESERVED      17 */
#define cs_hstemhm    18
#define cs_hintmask   19
#define cs_cntrmask   20
#define cs_rmoveto    21
#define cs_hmoveto    22
#define cs_vstemhm    23
#define cs_rcurveline 24
#define cs_rlinecurve 25
#define cs_vvcurveto  26
#define cs_hhcurveto  27
/*      SHORTINT      28 */
#define cs_callgsubr  29
#define cs_vhcurveto  30
#define cs_hvcurveto  31

/*tex

    Next the two byte CharString operators:

*/

#define cs_dotsection       0
/*      cs_vstem3           1 */
/*      cs_hstem3           2 */
#define cs_and              3
#define cs_or               4
#define cs_not              5
/*      cs_seac             6 */
/*      cs_sbw              7 */
/*      RESERVED            8 */
#define cs_abs              9
#define cs_add             10
#define cs_sub             11
#define cs_div             12
/*      RESERVED           13 */
#define cs_neg             14
#define cs_eq              15
/*      cs_callothersubr   16 */
/*      cs_pop|            17 */
#define cs_drop            18
/*      RESERVED           19 */
#define cs_put             20
#define cs_get             21
#define cs_ifelse          22
#define cs_random          23
#define cs_mul             24
/*      RESERVED           25 */
#define cs_sqrt            26
#define cs_dup             27
#define cs_exch            28
#define cs_index           29
#define cs_roll            30
/*      cs_setcurrentpoint 31 */
/*      RESERVED           32 */
/*      RESERVED           33 */
#define cs_hflex           34
#define cs_flex            35
#define cs_hflex1          36
#define cs_flex1           37

/*tex  |clear_stack| put all operands sotred in operand stack to dest. */

static void clear_stack(card8 ** dest, card8 * limit)
{
    int i;
    for (i = 0; i < cs2_stack_top; i++) {
        double value;
        long ivalue;
        value = cs2_arg_stack[i];
        /*tex The nearest integer value. */
        ivalue = (long) floor(value + 0.5);
        if (value >= 0x8000L || value <= (-0x8000L - 1)) {
            /*tex
                This number cannot be represented as a single operand. We must
                use |a b mul ...| or |a c div| to represent large values.
            */
            normal_error("cff","argument value too large (this is bug)");
        } else if (fabs(value - (double) ivalue) > 3.0e-5) {
            /*tex A 16.16-bit signed fixed value  */
            DST_NEED(limit, *dest + 5);
            *(*dest)++ = 255;
            /*tex The mantissa. */
            ivalue = (long) floor(value);
            *(*dest)++ = (card8) ((ivalue >> 8) & 0xff);
            *(*dest)++ = (card8) (ivalue & 0xff);
            /*tex The fraction. */
            ivalue = (long) ((value - (double) ivalue) * 0x10000l);
            *(*dest)++ = (card8) ((ivalue >> 8) & 0xff);
            *(*dest)++ = (card8) (ivalue & 0xff);
            /*tex Everything else is integer. */
        } else if (ivalue >= -107 && ivalue <= 107) {
            DST_NEED(limit, *dest + 1);
            *(*dest)++ = (card8) (ivalue + 139);
        } else if (ivalue >= 108 && ivalue <= 1131) {
            DST_NEED(limit, *dest + 2);
            ivalue = (long) 0xf700u + ivalue - 108;
            *(*dest)++ = (card8) ((ivalue >> 8) & 0xff);
            *(*dest)++ = (card8) (ivalue & 0xff);
        } else if (ivalue >= -1131 && ivalue <= -108) {
            DST_NEED(limit, *dest + 2);
            ivalue = (long) 0xfb00u - ivalue - 108;
            *(*dest)++ = (card8) ((ivalue >> 8) & 0xff);
            *(*dest)++ = (card8) (ivalue & 0xff);
        } else if (ivalue >= -32768 && ivalue <= 32767) {
            /*tex A shortint. */
            DST_NEED(limit, *dest + 3);
            *(*dest)++ = 28;
            *(*dest)++ = (card8) ((ivalue >> 8) & 0xff);
            *(*dest)++ = (card8) ((ivalue) & 0xff);
        } else {
            normal_error("cff","unexpected error");
        }
    }
    /*tex Clear the stack. */
    cs2_stack_top = 0;
    return;
}

/*tex
    Single byte operators: Path construction, Operator for finishing a path, Hint
    operators. Phases:

    \starttabulate
    \NC \type{0} \NC inital state \NC \NR
    \NC \type{1} \NC hint declaration, first stack-clearing operator appeared \NC \NR
    \NC \type{2} \NC in path construction \NC \NR
    \stoptabulate

*/

static void do_operator1(card8 ** dest, card8 * limit, card8 ** data, card8 * endptr)
{
    card8 op = **data;

    *data += 1;

    switch (op) {
        case cs_hstemhm:
        case cs_vstemhm:
            /*tex  A charstring may have a hintmask if the above operator has been seen. */
        case cs_hstem:
        case cs_vstem:
            if (phase == 0 && (cs2_stack_top % 2)) {
                have_width = 1;
                width = cs2_arg_stack[0];
            }
            num_stems += cs2_stack_top / 2;
            clear_stack(dest, limit);
            DST_NEED(limit, *dest + 1);
            *(*dest)++ = op;
            phase = 1;
            break;
        case cs_hintmask:
        case cs_cntrmask:
            if (phase < 2) {
                if (phase == 0 && (cs2_stack_top % 2)) {
                    have_width = 1;
                    width = cs2_arg_stack[0];
                }
                num_stems += cs2_stack_top / 2;
            }
            clear_stack(dest, limit);
            DST_NEED(limit, *dest + 1);
            *(*dest)++ = op;
            if (num_stems > 0) {
                int masklen = (num_stems + 7) / 8;
                DST_NEED(limit, *dest + masklen);
                SRC_NEED(endptr, *data + masklen);
                memmove(*dest, *data, (size_t) masklen);
                *data += masklen;
                *dest += masklen;
            }
            phase = 2;
            break;
        case cs_rmoveto:
            if (phase == 0 && (cs2_stack_top % 2)) {
                have_width = 1;
                width = cs2_arg_stack[0];
            }
            clear_stack(dest, limit);
            DST_NEED(limit, *dest + 1);
            *(*dest)++ = op;
            phase = 2;
            break;
        case cs_hmoveto:
        case cs_vmoveto:
            if (phase == 0 && (cs2_stack_top % 2) == 0) {
                have_width = 1;
                width = cs2_arg_stack[0];
            }
            clear_stack(dest, limit);
            DST_NEED(limit, *dest + 1);
            *(*dest)++ = op;
            phase = 2;
            break;
        case cs_endchar:
            if (cs2_stack_top == 1) {
                have_width = 1;
                width = cs2_arg_stack[0];
                clear_stack(dest, limit);
            } else if (cs2_stack_top == 4 || cs2_stack_top == 5) {
                normal_warning("cff","'seac' character deprecated in type 2 charstring");
                status = CS_PARSE_CFF_ERROR;
                return;
            } else if (cs2_stack_top > 0) {
                normal_warning("cff","operand stack not empty");
            }
            DST_NEED(limit, *dest + 1);
            *(*dest)++ = op;
            status = CS_CHAR_END;
            break;
            /*tex The above operators are candidate for first stack clearing operator. */
        case cs_setvsindex:
            /*
                vsindex = cs2_arg_stack[cs2_stack_top-1];
                cs2_stack_top -= 1;
            */
            normal_warning("cff2","unsupported setvindex operator");
            status = CS_PARSE_CFF_ERROR;
            break;
        case cs_blend:
            /*
                blends = cs2_arg_stack[cs2_stack_top-1];
                cs2_stack_top -= 1;
                cs2_stack_top -= blends * regions ;
            */
            normal_warning("cff2","unsupported blend operator");
            status = CS_PARSE_CFF_ERROR;
            break;
        case cs_rlineto:
        case cs_hlineto:
        case cs_vlineto:
        case cs_rrcurveto:
        case cs_rcurveline:
        case cs_rlinecurve:
        case cs_vvcurveto:
        case cs_hhcurveto:
        case cs_vhcurveto:
        case cs_hvcurveto:
            if (phase < 2) {
                normal_warning("cff","broken type 2 charstring");
                status = CS_PARSE_CFF_ERROR;
                return;
            }
            clear_stack(dest, limit);
            DST_NEED(limit, *dest + 1);
            *(*dest)++ = op;
            break;
            /*tex All the operotors above are stack clearing. */
        case cs_return:
            normal_error("cff","unexpected return");
        case cs_callgsubr:
            normal_error("cff","unexpected callgsubr");
        case cs_callsubr:
            normal_error("cff","unexpected callsubr");
            break;
        default:
            formatted_warning("cff","%s: unknown charstring operator: 0x%02x", CS_TYPE2_DEBUG_STR, op);
            status = CS_PARSE_CFF_ERROR;
            break;
    }
    return;
}

/*tex

    Double byte operators: Flex, arithmetic, conditional, and storage operators.
    The following operators are not supported: random but How random ?

*/

static void do_operator2(card8 ** dest, card8 * limit, card8 ** data, card8 * endptr)
{
    card8 op;
    *data += 1;
    SRC_NEED(endptr, *data + 1);
    op = **data;
    *data += 1;
    switch (op) {
        case cs_dotsection:
            normal_warning("cff","Operator 'dotsection' deprecated in type 2 charstring");
            status = CS_PARSE_CFF_ERROR;
            return;
            break;
        case cs_hflex:
        case cs_flex:
        case cs_hflex1:
        case cs_flex1:
            if (phase < 2) {
                formatted_warning("cff","%s: broken type 2 charstring", CS_TYPE2_DEBUG_STR);
                status = CS_PARSE_CFF_ERROR;
                return;
            }
            clear_stack(dest, limit);
            DST_NEED(limit, *dest + 2);
            *(*dest)++ = cs_escape;
            *(*dest)++ = op;
            break;
            /*tex All operators above are stack clearing. */
        case cs_and:
            NEED(cs2_stack_top, 2);
            cs2_stack_top--;
            if (cs2_arg_stack[cs2_stack_top] && cs2_arg_stack[cs2_stack_top - 1]) {
                cs2_arg_stack[cs2_stack_top - 1] = 1.0;
            } else {
                cs2_arg_stack[cs2_stack_top - 1] = 0.0;
            }
            break;
        case cs_or:
            NEED(cs2_stack_top, 2);
            cs2_stack_top--;
            if (cs2_arg_stack[cs2_stack_top] || cs2_arg_stack[cs2_stack_top - 1]) {
                cs2_arg_stack[cs2_stack_top - 1] = 1.0;
            } else {
                cs2_arg_stack[cs2_stack_top - 1] = 0.0;
            }
            break;
        case cs_not:
            NEED(cs2_stack_top, 1);
            if (cs2_arg_stack[cs2_stack_top - 1]) {
                cs2_arg_stack[cs2_stack_top - 1] = 0.0;
            } else {
                cs2_arg_stack[cs2_stack_top - 1] = 1.0;
            }
            break;
        case cs_abs:
            NEED(cs2_stack_top, 1);
            cs2_arg_stack[cs2_stack_top - 1] =
                fabs(cs2_arg_stack[cs2_stack_top - 1]);
            break;
        case cs_add:
            NEED(cs2_stack_top, 2);
            cs2_arg_stack[cs2_stack_top - 2] += cs2_arg_stack[cs2_stack_top - 1];
            cs2_stack_top--;
            break;
        case cs_sub:
            NEED(cs2_stack_top, 2);
            cs2_arg_stack[cs2_stack_top - 2] -= cs2_arg_stack[cs2_stack_top - 1];
            cs2_stack_top--;
            break;
        case cs_div:
            NEED(cs2_stack_top, 2);
            cs2_arg_stack[cs2_stack_top - 2] /= cs2_arg_stack[cs2_stack_top - 1];
            cs2_stack_top--;
            break;
        case cs_neg:
            NEED(cs2_stack_top, 1);
            cs2_arg_stack[cs2_stack_top - 1] *= -1.0;
            break;
        case cs_eq:
            NEED(cs2_stack_top, 2);
            cs2_stack_top--;
            if (cs2_arg_stack[cs2_stack_top] == cs2_arg_stack[cs2_stack_top - 1]) {
                cs2_arg_stack[cs2_stack_top - 1] = 1.0;
            } else {
                cs2_arg_stack[cs2_stack_top - 1] = 0.0;
            }
            break;
        case cs_drop:
            NEED(cs2_stack_top, 1);
            cs2_stack_top--;
            break;
        case cs_put:
            NEED(cs2_stack_top, 2);
            {
                int idx = (int) cs2_arg_stack[--cs2_stack_top];
                NEED(CS_TRANS_ARRAY_MAX, idx);
                trn_array[idx] = cs2_arg_stack[--cs2_stack_top];
            }
            break;
        case cs_get:
            NEED(cs2_stack_top, 1);
            {
                int idx = (int) cs2_arg_stack[cs2_stack_top - 1];
                NEED(CS_TRANS_ARRAY_MAX, idx);
                cs2_arg_stack[cs2_stack_top - 1] = trn_array[idx];
            }
            break;
        case cs_ifelse:
            NEED(cs2_stack_top, 4);
            cs2_stack_top -= 3;
            if (cs2_arg_stack[cs2_stack_top + 1] > cs2_arg_stack[cs2_stack_top + 2]) {
                cs2_arg_stack[cs2_stack_top - 1] = cs2_arg_stack[cs2_stack_top];
            }
            break;
        case cs_mul:
            NEED(cs2_stack_top, 2);
            cs2_arg_stack[cs2_stack_top - 2] =
                cs2_arg_stack[cs2_stack_top - 2] * cs2_arg_stack[cs2_stack_top - 1];
            cs2_stack_top--;
            break;
        case cs_sqrt:
            NEED(cs2_stack_top, 1);
            cs2_arg_stack[cs2_stack_top - 1] =
                sqrt(cs2_arg_stack[cs2_stack_top - 1]);
            break;
        case cs_dup:
            NEED(cs2_stack_top, 1);
            NEED(CS_ARG_STACK_MAX, cs2_stack_top + 1);
            cs2_arg_stack[cs2_stack_top] = cs2_arg_stack[cs2_stack_top - 1];
            cs2_stack_top++;
            break;
        case cs_exch:
            NEED(cs2_stack_top, 2);
            {
                double save = cs2_arg_stack[cs2_stack_top - 2];
                cs2_arg_stack[cs2_stack_top - 2] = cs2_arg_stack[cs2_stack_top - 1];
                cs2_arg_stack[cs2_stack_top - 1] = save;
            }
            break;
        case cs_index:
            NEED(cs2_stack_top, 2);
            {
                int idx = (int) cs2_arg_stack[cs2_stack_top - 1];
                if (idx < 0) {
                    cs2_arg_stack[cs2_stack_top - 1] =
                        cs2_arg_stack[cs2_stack_top - 2];
                } else {
                    NEED(cs2_stack_top, idx + 2);
                    cs2_arg_stack[cs2_stack_top - 1] =
                        cs2_arg_stack[cs2_stack_top - idx - 2];
                }
            }
            break;
        case cs_roll:
            NEED(cs2_stack_top, 2);
            {
                int N, J;
                J = (int) cs2_arg_stack[--cs2_stack_top];
                N = (int) cs2_arg_stack[--cs2_stack_top];
                NEED(cs2_stack_top, N);
                if (J > 0) {
                    J = J % N;
                    while (J-- > 0) {
                        double save = cs2_arg_stack[cs2_stack_top - 1];
                        int i = cs2_stack_top - 1;
                        while (i > cs2_stack_top - N) {
                            cs2_arg_stack[i] = cs2_arg_stack[i - 1];
                            i--;
                        }
                        cs2_arg_stack[i] = save;
                    }
                } else {
                    J = (-J) % N;
                    while (J-- > 0) {
                        double save = cs2_arg_stack[cs2_stack_top - N];
                        int i = cs2_stack_top - N;
                        while (i < cs2_stack_top - 1) {
                            cs2_arg_stack[i] = cs2_arg_stack[i + 1];
                            i++;
                        }
                        cs2_arg_stack[i] = save;
                    }
                }
            }
            break;
        case cs_random:
            formatted_warning("cff","%s: Charstring operator 'random' found.", CS_TYPE2_DEBUG_STR);
            NEED(CS_ARG_STACK_MAX, cs2_stack_top + 1);
            cs2_arg_stack[cs2_stack_top++] = 1.0;
            break;
        default:
            formatted_warning("cff","%s: unknown charstring operator: 0x0c%02x", CS_TYPE2_DEBUG_STR, op);
            status = CS_PARSE_CFF_ERROR;
            break;
    }
    return;
}

/*tex integer: exactly the same as the DICT encoding (except 29) */

static void cs2_get_integer(card8 ** data, card8 * endptr)
{
    long result = 0;
    card8 b0 = **data, b1, b2;
    *data += 1;
    if (b0 == 28) {
        /*tex shortint */
        SRC_NEED(endptr, *data + 2);
        b1 = **data;
        b2 = *(*data + 1);
        result = b1 * 256 + b2;
        if (result > 0x7fff)
            result -= 0x10000L;
        *data += 2;
    } else if (b0 >= 32 && b0 <= 246) {
        /*tex int (1) */
        result = b0 - 139;
    } else if (b0 >= 247 && b0 <= 250) {
        /*tex int (2) */
        SRC_NEED(endptr, *data + 1);
        b1 = **data;
        result = (b0 - 247) * 256 + b1 + 108;
        *data += 1;
    } else if (b0 >= 251 && b0 <= 254) {
        SRC_NEED(endptr, *data + 1);
        b1 = **data;
        result = -(b0 - 251) * 256 - b1 - 108;
        *data += 1;
    } else {
        status = CS_PARSE_CFF_ERROR;
        return;
    }
    NEED(CS_ARG_STACK_MAX, cs2_stack_top + 1);
    cs2_arg_stack[cs2_stack_top++] = (double) result;
    return;
}

/*tex Signed 16.16-bits fixed number for Type 2 charstring encoding. */

static void get_fixed(card8 ** data, card8 * endptr)
{
    long ivalue;
    double rvalue;
    *data += 1;
    SRC_NEED(endptr, *data + 4);
    ivalue = *(*data) * 0x100 + *(*data + 1);
    rvalue = (double) ((ivalue > 0x7fffL) ? (ivalue - 0x10000L) : ivalue);
    ivalue = *(*data + 2) * 0x100 + *(*data + 3);
    rvalue += ((double) ivalue) / 0x10000L;
    NEED(CS_ARG_STACK_MAX, cs2_stack_top + 1);
    cs2_arg_stack[cs2_stack_top++] = rvalue;
    *data += 4;
    return;
}

/*tex

Subroutines: the bias for subroutine number is introduced in type 2
charstrings.

\starttabulate
\NC \type {subr}     \NC set to a pointer to the subroutine charstring \NC \NR
\NC \type {len}      \NC set to the length of subroutine charstring \NC \NR
\NC \type {subr_idx} \NC CFF INDEX data that contains subroutines \NC \NR
\NC \type {id}       \NC biased subroutine number \NC \NR
\stoptabulate

*/

static void get_subr(card8 ** subr, long *len, cff_index * subr_idx, long id)
{
    card16 count;
    if (subr_idx == NULL)
        formatted_error("cff","%s: subroutine called but no subroutine found",CS_TYPE2_DEBUG_STR);
    count = subr_idx->count;
    /*tex addi the bias number */
    if (count < 1240) {
        id += 107;
    } else if (count < 33900) {
        id += 1131;
    } else {
        id += 32768;
    }
    if (id > count)
        formatted_error("cff","%s: invalid subroutine index: %ld (max=%u)", CS_TYPE2_DEBUG_STR, id, count);
    *len = (long) ((subr_idx->offset)[id + 1] - (subr_idx->offset)[id]);
    *subr = subr_idx->data + (subr_idx->offset)[id] - 1;
    return;
}

/*tex

    The Type 2 interpretation of a number encoded in five-bytes (those with an
    initial byte value of 255) differs from how it is interpreted in the Type 1
    format.

*/

static void do_charstring(card8 ** dest, card8 * limit, card8 ** data, card8 * endptr,
    cff_index * gsubr_idx, cff_index * subr_idx, int cff2)
{
    card8 b0 = 0, *subr;
    long len;
    if (cs2_nest > CS_SUBR_NEST_MAX)
        formatted_error("cff","%s: subroutine nested too deeply", CS_TYPE2_DEBUG_STR);
    cs2_nest++;
    while (*data < endptr && status == CS_PARSE_OK) {
        b0 = **data;
        if (b0 == 255) {
            /*tex A 16-bit.16-bit fixed signed number. */
            get_fixed(data, endptr);
        } else if (b0 == cs_return) {
            status = CS_SUBR_RETURN;
        } else if (b0 == cs_callgsubr) {
            if (cs2_stack_top < 1) {
                status = CS_STACK_CFF_ERROR;
            } else {
                cs2_stack_top--;
                get_subr(&subr, &len, gsubr_idx, (long) cs2_arg_stack[cs2_stack_top]);
                if (*dest + len > limit)
                    formatted_error("cff","%s: possible buffer overflow (1)", CS_TYPE2_DEBUG_STR);
                do_charstring(dest, limit, &subr, subr + len, gsubr_idx, subr_idx, cff2);
                *data += 1;
            }
        } else if (b0 == cs_callsubr) {
            if (cs2_stack_top < 1) {
                status = CS_STACK_CFF_ERROR;
            } else {
                cs2_stack_top--;
                get_subr(&subr, &len, subr_idx, (long) cs2_arg_stack[cs2_stack_top]);
                if (limit < *dest + len)
                    formatted_error("cff","%s: possible buffer overflow (2)", CS_TYPE2_DEBUG_STR);
                do_charstring(dest, limit, &subr, subr + len, gsubr_idx, subr_idx, cff2);
                *data += 1;
            }
        } else if (b0 == cs_escape) {
            do_operator2(dest, limit, data, endptr);
        } else if (b0 < 32 && b0 != 28) {
            do_operator1(dest, limit, data, endptr);
        } else if ((b0 <= 22 && b0 >= 27) || b0 == 31) {
            status = CS_PARSE_CFF_ERROR;
        } else {
            cs2_get_integer(data, endptr);
        }
    }
    if (cff2) {
        DST_NEED(limit, *dest + 1);
        ++endptr;
        *(*dest)++ = cs_endchar;
    } else if (status == CS_SUBR_RETURN) {
        status = CS_PARSE_OK;
    } else if (status == CS_CHAR_END && *data < endptr) {
        formatted_warning("cff","%s: garbage after endchar", CS_TYPE2_DEBUG_STR);
    } else if (status < CS_PARSE_OK) {
        formatted_error("cff","%s: parsing charstring failed: (status=%d, stack=%d)", CS_TYPE2_DEBUG_STR, status, cs2_stack_top);
    }
    cs2_nest--;
    return;
}

static void cs_parse_init(void)
{
    status = CS_PARSE_OK;
    cs2_nest = 0;
    phase = 0;
    num_stems = 0;
    cs2_stack_top = 0;
}

/*tex Not just copying \unknown */

static long cs_copy_charstring(card8 * dst, long dstlen, card8 * src, long srclen, cff_index * gsubr,
    cff_index * subr, double default_width, double nominal_width, cs_ginfo * ginfo, int cff2)
{
    card8 *save = dst;

    cs_parse_init();

    width = 0.0;
    have_width = 0;

    /* expand call(g)subrs */
    do_charstring(&dst, dst + dstlen, &src, src + srclen, gsubr, subr, cff2);

    if (ginfo) {
        ginfo->flags = 0;       /* not used */
        if (have_width) {
            ginfo->wx = nominal_width + width;
        } else {
            ginfo->wx = default_width;
        }
    }

    return (long) (dst - save);
}

/*tex CID-Keyed font specific. */

long cff_read_fdselect(cff_font * cff)
{
    cff_fdselect *fdsel;
    long offset, length;
    card16 i;
    if (cff->topdict == NULL)
        normal_error("cff","top DICT not available");
    if (!(cff->flag & FONTTYPE_CIDFONT))
        return 0;
    offset = (long) cff_dict_get(cff->topdict, "FDSelect", 0);
    cff->offset = (l_offset) offset;
    cff->fdselect = fdsel = xcalloc(1, sizeof(cff_fdselect));
    fdsel->format = get_card8(cff);
    length = 1;
    switch (fdsel->format) {
        case 0:
            fdsel->num_entries = cff->num_glyphs;
            (fdsel->data).fds = xmalloc(fdsel->num_entries * sizeof(card8));
            for (i = 0; i < (fdsel->num_entries); i++) {
                (fdsel->data).fds[i] = get_card8(cff);
            }
            length += fdsel->num_entries;
            break;
        case 3:
            {
                cff_range3 *ranges;
                fdsel->num_entries = get_card16(cff);
                fdsel->data.ranges = ranges =
                    xcalloc(fdsel->num_entries, sizeof(cff_range3));
                for (i = 0; i < (fdsel->num_entries); i++) {
                    ranges[i].first = get_card16(cff);
                    ranges[i].fd = get_card8(cff);
                }
                if (ranges[0].first != 0)
                    normal_error("cff","range not starting with 0");
                if (cff->num_glyphs != get_card16(cff))
                    normal_error("cff","sentinel value mismatched with number of glyphs");
                length += (fdsel->num_entries) * 3 + 4;
            }
            break;
        default:
            xfree(fdsel);
            normal_error("cff","unknown FDSelect format");
            break;
    }
    return length;
}

long cff_pack_fdselect(cff_font * cff, card8 * dest, long destlen)
{
    cff_fdselect *fdsel;
    long len = 0;
    card16 i;
    if (cff->fdselect == NULL)
        return 0;
    if (destlen < 1)
        normal_error("cff","buffer overflow (23)");
    fdsel = cff->fdselect;
    dest[len++] = fdsel->format;
    switch (fdsel->format) {
        case 0:
            if (fdsel->num_entries != cff->num_glyphs)
                normal_error("cff","invalid data");
            if (destlen < len + fdsel->num_entries)
                normal_error("cff","buffer overflow (24)");
            for (i = 0; i < fdsel->num_entries; i++) {
                dest[len++] = (fdsel->data).fds[i];
            }
            break;
        case 3:
            {
                if (destlen < len + 2)
                    normal_error("cff","buffer overflow (25)");
                len += 2;
                for (i = 0; i < (fdsel->num_entries); i++) {
                    if (destlen < len + 3)
                        normal_error("cff","buffer overflow (26)");
                    dest[len++] =
                        (card8) (((fdsel->data).ranges[i].first >> 8) & 0xff);
                    dest[len++] = (card8) ((fdsel->data).ranges[i].first & 0xff);
                    dest[len++] = (card8) ((fdsel->data).ranges[i].fd);
                }
                if (destlen < len + 2)
                    normal_error("cff","buffer overflow (27)");
                dest[len++] = (card8) ((cff->num_glyphs >> 8) & 0xff);
                dest[len++] = (card8) (cff->num_glyphs & 0xff);
                dest[1] = (card8) (((len / 3 - 1) >> 8) & 0xff);
                dest[2] = (card8) ((len / 3 - 1) & 0xff);
            }
            break;
        default:
            normal_error("cff","unknown FDSelect format");
            break;
    }
    return len;
}

/*tex Create an instance of embeddable font. */

static void write_fontfile(PDF pdf, cff_font * cffont, char *fullname)
{
    cff_index *topdict, *fdarray, *private;
    unsigned char *dest;
    long destlen = 0, i, size;
    long offset, topdict_offset, fdarray_offset;
    topdict = cff_new_index(1);
    fdarray = cff_new_index(cffont->num_fds);
    private = cff_new_index(cffont->num_fds);
    cff_dict_remove(cffont->topdict, "UniqueID");
    cff_dict_remove(cffont->topdict, "XUID");
    /*tex A bad font may have this: */
    cff_dict_remove(cffont->topdict, "Private");
    /*tex A bad font may have this: */
    cff_dict_remove(cffont->topdict, "Encoding");
    /*tex This is CFF2 specific: */
    cff_dict_remove(cffont->topdict, "vstore");
    /*tex This is CFF2 specific: */
    cff_dict_remove(cffont->topdict, "maxstack");
    topdict->offset[1] = (l_offset) cff_dict_pack(cffont->topdict, (card8 *) work_buffer, WORK_BUFFER_SIZE) + 1;
    for (i = 0; i < cffont->num_fds; i++) {
        size = 0;
        if (cffont->private && cffont->private[i]) {
            size = cff_dict_pack(cffont->private[i], (card8 *) work_buffer, WORK_BUFFER_SIZE);
            if (size < 1) {
                /*tex |Private| contains only |Subr|: */
                cff_dict_remove(cffont->fdarray[i], "Private");
            }
        }
        (private->offset)[i + 1] = (unsigned long) ((private->offset)[i] + (unsigned) size);
        (fdarray->offset)[i + 1] = (unsigned long) ((fdarray->offset)[i]
            + (unsigned) cff_dict_pack(cffont->fdarray[i], (card8 *) work_buffer, WORK_BUFFER_SIZE));
    }
    /*tex The header size: */
    destlen = 4;
    destlen += cff_set_name(cffont, fullname);
    destlen += cff_index_size(topdict);
    destlen += cff_index_size(cffont->string);
    destlen += cff_index_size(cffont->gsubr);
    /*tex |charset| format 0 */
    destlen += (cffont->charsets->num_entries) * 2 + 1;
    /*tex |fdselect| format 3 */
    destlen += (cffont->fdselect->num_entries) * 3 + 5;
    destlen += cff_index_size(cffont->cstrings);
    destlen += cff_index_size(fdarray);
    /* |Private| is not indexed */
    destlen = (long) (destlen + (long) private->offset[private->count] - 1);
    dest = xcalloc((unsigned) destlen, sizeof(card8));
    offset = 0;
    /*tex |Header| */
    offset += cff_put_header(cffont, dest + offset, destlen - offset);
    /*tex |Name| */
    offset += cff_pack_index(cffont->name, dest + offset, destlen - offset);
    /*tex |Top DICT| */
    topdict_offset = offset;
    offset += cff_index_size(topdict);
    /*tex |Strings| */
    offset += cff_pack_index(cffont->string, dest + offset, destlen - offset);
    /*tex |Global Subrs| */
    offset += cff_pack_index(cffont->gsubr, dest + offset, destlen - offset);
    /*tex |charset| */
    cff_dict_set(cffont->topdict, "charset", 0, (double) offset);
    offset += cff_pack_charsets(cffont, dest + offset, destlen - offset);
    /*tex |FDSelect| */
    cff_dict_set(cffont->topdict, "FDSelect", 0, (double) offset);
    offset += cff_pack_fdselect(cffont, dest + offset, destlen - offset);
    /*tex |CharStrings| */
    cff_dict_set(cffont->topdict, "CharStrings", 0, (double) offset);
    offset += cff_pack_index(cffont->cstrings, dest + offset, cff_index_size(cffont->cstrings));
    cff_release_index(cffont->cstrings);
    /*tex |Charstring|s can consume a lot of memory. */
    cffont->cstrings = NULL;
    /*tex |FDArray| and |Private| */
    cff_dict_set(cffont->topdict, "FDArray", 0, (double) offset);
    fdarray_offset = offset;
    offset += cff_index_size(fdarray);
    fdarray->data = xcalloc((unsigned) (fdarray->offset[fdarray->count] - 1), sizeof(card8));
    for (i = 0; i < cffont->num_fds; i++) {
        size = (long) (private->offset[i + 1] - private->offset[i]);
        if (cffont->private[i] && size > 0) {
            cff_dict_pack(cffont->private[i], dest + offset, size);
            cff_dict_set(cffont->fdarray[i], "Private", 0, (double) size);
            cff_dict_set(cffont->fdarray[i], "Private", 1, (double) offset);
        }
        cff_dict_pack(cffont->fdarray[i], fdarray->data + (fdarray->offset)[i] - 1, (long) (fdarray->offset[fdarray->count] - 1));
        offset += size;
    }
    cff_pack_index(fdarray, dest + fdarray_offset, cff_index_size(fdarray));
    cff_release_index(fdarray);
    cff_release_index(private);
    /*tex Finally the |Top DICT| */
    topdict->data = xcalloc((unsigned) (topdict->offset[topdict->count] - 1), sizeof(card8));
    cff_dict_pack(cffont->topdict, topdict->data, (long) (topdict->offset[topdict->count] - 1));
    cff_pack_index(topdict, dest + topdict_offset, cff_index_size(topdict));
    cff_release_index(topdict);
    for (i = 0; i < offset; i++) {
        strbuf_putchar(pdf->fb, dest[i]);
    }
    xfree(dest);
    return;
}

void write_cff(PDF pdf, cff_font * cffont, fd_entry * fd)
{
    cff_index *charstrings, *cs_idx;
    long charstring_len, max_len;
    long size, offset = 0;
    card8 *data;
    card16 num_glyphs, cs_count1, code, gid, last_cid;
    double nominal_width, default_width;
    char *fontname;
    char *fullname;
    glw_entry *glyph, *found;
    struct avl_traverser t;
    cffont->_string = NULL;
    fontname = xcalloc((unsigned) (1 + strlen(fd->fontname)), 1);
    sprintf(fontname, "%s", fd->fontname);
    fullname = xcalloc((unsigned) (8 + strlen(fd->fontname)), 1);
    sprintf(fullname, "%s+%s", fd->subset_tag, fd->fontname);
    /*tex Finish parsing the CFF. */
    cff_read_private(cffont);
    cff_read_subrs(cffont);
    /*tex The |Width|s. */
    if (cffont->private[0] && cff_dict_known(cffont->private[0], "defaultWidthX")) {
        default_width = (double) cff_dict_get(cffont->private[0], "defaultWidthX", 0);
    } else {
        default_width = CFF_DEFAULTWIDTHX_DEFAULT;
    }
    if (cffont->private[0] && cff_dict_known(cffont->private[0], "nominalWidthX")) {
        nominal_width = (double) cff_dict_get(cffont->private[0], "nominalWidthX", 0);
    } else {
        nominal_width = CFF_NOMINALWIDTHX_DEFAULT;
    }
    num_glyphs = 0;
    last_cid = 0;
    glyph = xtalloc(1, glw_entry);
    /*tex insert |notdef| */
    glyph->id = 0;
    if (avl_find(fd->gl_tree, glyph) == NULL) {
        avl_insert(fd->gl_tree, glyph);
        glyph = xtalloc(1, glw_entry);
    }
    avl_t_init(&t, fd->gl_tree);
    for (found = (glw_entry *) avl_t_first(&t, fd->gl_tree);
         found != NULL; found = (glw_entry *) avl_t_next(&t)) {
        if (found->id > last_cid)
            last_cid = (card16) found->id;
        num_glyphs++;
    }
    {
        cff_fdselect *fdselect;
        fdselect = xcalloc(1, sizeof(cff_fdselect));
        fdselect->format = 3;
        fdselect->num_entries = 1;
        fdselect->data.ranges = xcalloc(1, sizeof(cff_range3));
        fdselect->data.ranges[0].first = 0;
        fdselect->data.ranges[0].fd = 0;
        cffont->fdselect = fdselect;
    }
    {
        cff_charsets *charset;
        charset = xcalloc(1, sizeof(cff_charsets));
        charset->format = 0;
        charset->num_entries = (card16) (num_glyphs - 1);
        charset->data.glyphs = xcalloc(num_glyphs, sizeof(s_SID));
        gid = 0;
        avl_t_init(&t, fd->gl_tree);
        for (found = (glw_entry *) avl_t_first(&t, fd->gl_tree);
             found != NULL; found = (glw_entry *) avl_t_next(&t)) {
            if (found->id != 0) {
                charset->data.glyphs[gid] = (s_SID) found->id;
                gid++;
            }
        }
        cffont->charsets = charset;
        if (cffont->header_major == 2) {
            cff_dict_add(cffont->topdict, "charset", 1);
        }
    }
    cff_dict_add(cffont->topdict, "CIDCount", 1);
    cff_dict_set(cffont->topdict, "CIDCount", 0, last_cid + 1);
    if (cffont->header_major == 2) {
        cff_dict_add(cffont->topdict, "FullName", 1);
        cff_dict_set(cffont->topdict, "FullName", 0, (double) cff_add_string(cffont, fontname));
        cff_dict_add(cffont->topdict, "FontBBox", 4);
        cff_dict_set(cffont->topdict, "FontBBox", 0, fd->font_dim[FONTBBOX1_CODE].val);
        cff_dict_set(cffont->topdict, "FontBBox", 1, fd->font_dim[FONTBBOX2_CODE].val);
        cff_dict_set(cffont->topdict, "FontBBox", 2, fd->font_dim[FONTBBOX3_CODE].val);
        cff_dict_set(cffont->topdict, "FontBBox", 3, fd->font_dim[FONTBBOX4_CODE].val);
    }
    cffont->fdarray = xcalloc(1, sizeof(cff_dict *));
    cffont->fdarray[0] = cff_new_dict();
    cff_dict_add(cffont->fdarray[0], "FontName", 1);
    /*tex fix: skip XXXXXX+ */
    cff_dict_set(cffont->fdarray[0], "FontName", 0, (double) cff_add_string(cffont, fullname));
    cff_dict_add(cffont->fdarray[0], "Private", 2);
    cff_dict_set(cffont->fdarray[0], "Private", 0, 0.0);
    cff_dict_set(cffont->fdarray[0], "Private", 0, 0.0);
    /*tex |FDArray| index offset, not known yet */
    cff_dict_add(cffont->topdict, "FDArray", 1);
    cff_dict_set(cffont->topdict, "FDArray", 0, 0.0);
    /*tex |FDSelect| offset, not known yet */
    cff_dict_add(cffont->topdict, "FDSelect", 1);
    cff_dict_set(cffont->topdict, "FDSelect", 0, 0.0);
    cff_dict_remove(cffont->topdict, "UniqueID");
    cff_dict_remove(cffont->topdict, "XUID");
    cff_dict_remove(cffont->topdict, "Private");
    cff_dict_remove(cffont->topdict, "Encoding");
    cffont->offset = (l_offset) cff_dict_get(cffont->topdict, "CharStrings", 0);
    cs_idx = cff_get_index_header(cffont);
    offset = (long) cffont->offset;
    cs_count1 = cs_idx->count;
    if (cs_count1 < 2) {
        normal_error("cff","no valid charstring data found");
    }
    /*tex Build the new charstrings entry. */
    charstrings = cff_new_index((card16) (cs_count1==USHRT_MAX?cs_count1: cs_count1 + 1));
    max_len = 2 * CS_STR_LEN_MAX;
    charstrings->data = xcalloc((unsigned) max_len, sizeof(card8));
    charstring_len = 0;
    gid = 0;
    data = xcalloc(CS_STR_LEN_MAX, sizeof(card8));
    {
        int i;
        int tex_font = fd->tex_font;
        int streamprovider = 0;
        int callback_id = 0 ;
        if ((tex_font > 0) && (font_streamprovider(tex_font) == 1)) {
            streamprovider = font_streamprovider(tex_font);
            callback_id = callback_defined(glyph_stream_provider_callback);
        }
        for (i = 0; i < cs_count1; i++) {
            code = (card16) i;
            glyph->id = code;
            if ((avl_find(fd->gl_tree,glyph) != NULL)) {
                /*tex This code is the same as below, apart from small details */
                if (callback_id > 0) {
                    lstring * result;
                    run_callback(callback_id, "ddd->L", tex_font, i, streamprovider, &result); /* this call can be sped up */
                    size = (size_t) result->l ;
                    if (size > 0) {
                        if (charstring_len + CS_STR_LEN_MAX >= max_len) {
                            max_len = (long)(charstring_len + 2 * CS_STR_LEN_MAX);
                            charstrings->data = xrealloc(charstrings->data, (unsigned)((unsigned)max_len*sizeof(card8)));
                        }
                        (charstrings->offset)[gid] = (unsigned)(charstring_len + 1);
                        cffont->offset = (l_offset)((unsigned)offset + (cs_idx->offset)[code] - 1);
                        memcpy(charstrings->data+charstring_len,(const char *) result->s,(size_t) size);
                        charstring_len += size;
                        xfree(result);
                    }
                } else {
                    size = (long)(cs_idx->offset[code+1] - cs_idx->offset[code]);
                    if (size > CS_STR_LEN_MAX) {
                        formatted_error("cff","charstring too long: gid=%u, %ld bytes", code, size);
                    }
                    if (charstring_len + CS_STR_LEN_MAX >= max_len) {
                        max_len = (long)(charstring_len + 2 * CS_STR_LEN_MAX);
                        charstrings->data = xrealloc(charstrings->data, (unsigned)((unsigned)max_len*sizeof(card8)));
                    }
                    (charstrings->offset)[gid] = (unsigned)(charstring_len + 1);
                    cffont->offset = (l_offset)((unsigned)offset + (cs_idx->offset)[code] - 1);
                    memcpy(data,&cffont->stream[cffont->offset],(size_t)size);
                    charstring_len += cs_copy_charstring(
                        charstrings->data + charstring_len,
                        max_len - charstring_len,
                        data, size,
                        cffont->gsubr, (cffont->subrs)[0],
                        default_width, nominal_width, NULL,
                        cffont->header_major == 2
                    );
                }
                gid++;
            }
        }
    }
    /*tex
        The |CIDSet| is a table of bits indexed by cid, bytes with high order bit
        first, each (set) bit is a (present) CID.
    */
    if ((! pdf->omit_cidset) && (pdf->major_version == 1)) {
        int cid;
        cidset = pdf_create_obj(pdf, obj_type_others, 0);
        if (cidset != 0) {
            size_t l = (last_cid/8)+1;
            char *stream = xmalloc(l);
            memset(stream, 0, l);
            for (cid = 1; cid <= (long) last_cid; cid++) {
                glyph->id = cid;
                if (avl_find(fd->gl_tree,glyph) != NULL) {
                    stream[(cid / 8)] |= (1 << (7 - (cid % 8)));
                }
            }
            pdf_begin_obj(pdf, cidset, OBJSTM_NEVER);
            pdf_begin_dict(pdf);
            pdf_dict_add_streaminfo(pdf);
            pdf_end_dict(pdf);
            pdf_begin_stream(pdf);
            pdf_out_block(pdf, stream, l);
            pdf_end_stream(pdf);
            pdf_end_obj(pdf);
        }
    }
    /*tex
        This happens if the internal metrics do not agree with the actual disk
        font.
    */
    if (gid < num_glyphs) {
        formatted_warning("cff","embedded subset is smaller than expected: %d instead of %d glyphs", gid, num_glyphs);
        num_glyphs = gid;
    }
    xfree(data);
    cff_release_index(cs_idx);
    (charstrings->offset)[num_glyphs] = (l_offset) (charstring_len + 1);
    charstrings->count = num_glyphs;
    cffont->num_glyphs = num_glyphs;
    cffont->cstrings = charstrings;
    /*tex
        We don't use subroutines at all.
    */
    if (cffont->gsubr)
        cff_release_index(cffont->gsubr);
    cffont->gsubr = cff_new_index(0);
    if (cffont->subrs && cffont->subrs[0])
        cff_release_index(cffont->subrs[0]);
    cffont->subrs[0] = NULL;
    if (cffont->private && (cffont->private)[0]) {
        cff_dict_remove((cffont->private)[0], "Subrs"); /* no Subrs */
    }
    cff_dict_update(cffont->topdict, cffont);
    cff_add_string(cffont, "Adobe");
    cff_add_string(cffont, "Identity");
    if (cffont->header_major == 2) {
        /*tex A crash. */
    } else if (cffont->private && (cffont->private)[0]) {
        cff_dict_update(cffont->private[0], cffont);
    }
    cff_update_string(cffont);
    /* CFF code need to be rewritten */
    cff_dict_add(cffont->topdict, "ROS", 3);
    cff_dict_set(cffont->topdict, "ROS", 0, (double) cff_get_sid(cffont, "Adobe"));
    cff_dict_set(cffont->topdict, "ROS", 1, (double) cff_get_sid(cffont, "Identity"));
    cff_dict_set(cffont->topdict, "ROS", 2, 0.0);
    write_fontfile(pdf, cffont, fullname);
    xfree(fontname);
    xfree(fullname);
    cff_close(cffont);
}

#define is_cidfont(a) ((a)->flag & FONTTYPE_CIDFONT)
#define CID_MAX 65535

void write_cid_cff(PDF pdf, cff_font * cffont, fd_entry * fd)
{
    cff_index *charstrings, *cs_idx;
    long charstring_len, max_len;
    long size, offset = 0;
    int tex_font = fd->tex_font;
    int streamprovider = 0;
    int callback_id = 0 ;
    card8 *data;
    card16 num_glyphs, cs_count1, gid, last_cid;
    int fdsel, prev_fd, cid_count, cid ;
    char *fullname;
    glw_entry *glyph;
    unsigned char *CIDToGIDMap = NULL;
    cff_fdselect *fdselect = NULL;
    cff_charsets *charset = NULL;
    if (!is_cidfont(cffont)) {
        normal_error("cff","invalid CIDfont");
        return;
    }
    if ((tex_font > 0) && (font_streamprovider(tex_font) == 1)) {
        streamprovider = font_streamprovider(tex_font);
        callback_id = callback_defined(glyph_stream_provider_callback);
    }
    fullname = xcalloc((unsigned) (8 + strlen(fd->fontname)), 1);
    sprintf(fullname, "%s+%s", fd->subset_tag, fd->fontname);
    /*tex Finish parsing the CFF. */
    if (cff_dict_known(cffont->topdict, "CIDCount")) {
        cid_count = (card16) cff_dict_get(cffont->topdict, "CIDCount", 0);
    } else {
        cid_count = CFF_CIDCOUNT_DEFAULT;
    }
    if (cffont->header_major == 2) {
        /*tex hm */
    } else {
        cff_read_charsets(cffont);
    }
    CIDToGIDMap = xmalloc((unsigned) ((2 * (unsigned) cid_count) * sizeof(unsigned char)));
    memset(CIDToGIDMap, 0, (size_t) (2 * cid_count));
    glyph = xtalloc(1, glw_entry);
    /*tex insert |notdef| */
    glyph->id = 0;
    if (avl_find(fd->gl_tree, glyph) == NULL) {
        avl_insert(fd->gl_tree, glyph);
        glyph = xtalloc(1, glw_entry);
    }
    last_cid = 0;
    num_glyphs = 0;
    for (cid = 0; cid <= CID_MAX; cid++) {
        glyph->id = (unsigned) cid;
        if (avl_find(fd->gl_tree, glyph) != NULL) {
            gid = (card16) cid;
            CIDToGIDMap[2 * cid] = (unsigned char) ((gid >> 8) & 0xff);
            CIDToGIDMap[2 * cid + 1] = (unsigned char) (gid & 0xff);
            last_cid = (card16) cid;
            num_glyphs++;
        }
    }
    if (cffont->header_major == 2) {
        /*tex hm */
    } else if (last_cid >= cffont->num_glyphs) {
        formatted_error("cff font","bad glyph index %i",last_cid);
    }
    /*tex
        The |CIDSet| table is a table of bits indexed by cid, bytes with high
        order bit first, each (set) bit is a (present) CID.
    */
    if ((! pdf->omit_cidset) && (pdf->major_version == 1)) {
        cidset = pdf_create_obj(pdf, obj_type_others, 0);
        if (cidset != 0) {
            size_t l = (last_cid / 8) + 1;
            char *stream = xmalloc(l);
            memset(stream, 0, l);
            for (cid = 1; cid <= (long) last_cid; cid++) {
                if (CIDToGIDMap[2 * cid] || CIDToGIDMap[2 * cid + 1]) {
                    stream[(cid / 8)] |= (1 << (7 - (cid % 8)));
                }
            }
            pdf_begin_obj(pdf, cidset, OBJSTM_NEVER);
            pdf_begin_dict(pdf);
            pdf_dict_add_streaminfo(pdf);
            pdf_end_dict(pdf);
            pdf_begin_stream(pdf);
            pdf_out_block(pdf, stream, l);
            pdf_end_stream(pdf);
            pdf_end_obj(pdf);
            xfree(stream);
        }
    }
    cff_read_fdselect(cffont);
    cff_read_fdarray(cffont);
    cff_read_private(cffont);
    cff_read_subrs(cffont);
    cffont->offset = (l_offset) cff_dict_get(cffont->topdict, "CharStrings", 0);
    cs_idx = cff_get_index_header(cffont);
    offset = (long) cffont->offset;
    cs_count1 = cs_idx->count;
    if (cs_count1 < 2) {
        normal_error("cff","no valid charstring data found");
    }
    charset = xcalloc(1, sizeof(cff_charsets));
    charset->format = 0;
    charset->num_entries = 0;
    charset->data.glyphs = xcalloc(num_glyphs, sizeof(s_SID));
    fdselect = xcalloc(1, sizeof(cff_fdselect));
    fdselect->format = 3;
    fdselect->num_entries = 0;
    fdselect->data.ranges = xcalloc(num_glyphs, sizeof(cff_range3));
    charstrings = cff_new_index((card16) (cs_count1==USHRT_MAX?cs_count1: cs_count1 + 1));
    max_len = 2 * CS_STR_LEN_MAX;
    charstrings->data = xcalloc((unsigned) max_len, sizeof(card8));
    charstring_len = 0;
    prev_fd = -1;
    gid = 0;
    data = xcalloc(CS_STR_LEN_MAX, sizeof(card8));
    for (cid = 0; cid <= last_cid; cid++) {
        unsigned short gid_org;
        glyph->id = (unsigned) cid;
        if (avl_find(fd->gl_tree, glyph) == NULL)
            continue;
        gid_org = (short unsigned) ((CIDToGIDMap[2 * cid] << 8) | (CIDToGIDMap[2 * cid + 1]));
        fdsel = cff_fdselect_lookup(cffont, gid_org);
        if (callback_id > 0) {
            /*tex The next blob is not yet tested \unknown\ I need a font. */
            lstring * result;
            run_callback(callback_id, "ddd->L", tex_font, gid_org, streamprovider, &result);
            size = (size_t) result->l ;
            if (size > 0) {
                if (charstring_len + CS_STR_LEN_MAX >= max_len) {
                    max_len = (long)(charstring_len + 2 * CS_STR_LEN_MAX);
                    charstrings->data = xrealloc(charstrings->data, (unsigned)((unsigned)max_len*sizeof(card8)));
                }
                (charstrings->offset)[gid] = (unsigned)(charstring_len + 1);
                cffont->offset = (l_offset)((unsigned)offset + (cs_idx->offset)[gid_org] - 1);
                memcpy(charstrings->data+charstring_len,(const char *) result->s,(size_t)size);
                charstring_len += size;
                xfree(result);
            }
        } else {
            size = (long) (cs_idx->offset[gid_org + 1] - cs_idx->offset[gid_org]);
            if (size > CS_STR_LEN_MAX) {
                formatted_error("cff","charstring too long: gid=%u, %ld bytes", cid, size);
            }
            if (charstring_len + CS_STR_LEN_MAX >= max_len) {
                max_len = charstring_len + 2 * CS_STR_LEN_MAX;
                charstrings->data = xrealloc(charstrings->data, (unsigned) ((unsigned) max_len * sizeof(card8)));
            }
            (charstrings->offset)[gid] = (l_offset) (charstring_len + 1);
            cffont->offset = (l_offset) ((unsigned) offset + (cs_idx->offset)[gid_org] - 1);
            memcpy(data, &cffont->stream[cffont->offset], (size_t) size);
            charstring_len += cs_copy_charstring(
                charstrings->data + charstring_len,
                max_len - charstring_len,
                data, size,
                cffont->gsubr, (cffont->subrs)[fdsel],
                0, 0, NULL,
                cffont->header_major == 2
            );
        }
        if (cid > 0 && gid_org > 0) {
            charset->data.glyphs[charset->num_entries] = (s_SID) cid;
            charset->num_entries++;
        }
        if (fdsel != prev_fd) {
            fdselect->data.ranges[fdselect->num_entries].first = gid;
            fdselect->data.ranges[fdselect->num_entries].fd = (card8) fdsel;
            fdselect->num_entries++;
            prev_fd = fdsel;
        }
        gid++;
    }
    if (gid != num_glyphs)
        formatted_error("cff","unexpected error: %i != %i", gid, num_glyphs);
    xfree(data);
    cff_release_index(cs_idx);
    xfree(CIDToGIDMap);
    (charstrings->offset)[num_glyphs] = (l_offset) (charstring_len + 1);
    charstrings->count = num_glyphs;
    cffont->num_glyphs = num_glyphs;
    cffont->cstrings = charstrings;
    cff_release_charsets(cffont->charsets);
    cffont->charsets = charset;
    cff_release_fdselect(cffont->fdselect);
    cffont->fdselect = fdselect;
    /*tex
        We don't use subroutines at all.
    */
    if (cffont->gsubr)
        cff_release_index(cffont->gsubr);
    cffont->gsubr = cff_new_index(0);
    for (fdsel = 0; fdsel < cffont->num_fds; fdsel++) {
        if (cffont->subrs && cffont->subrs[fdsel]) {
            cff_release_index(cffont->subrs[fdsel]);
            cffont->subrs[fdsel] = NULL;
        }
        if (cffont->private && (cffont->private)[fdsel]) {
            cff_dict_remove((cffont->private)[fdsel], "Subrs"); /* no Subrs */
        }
    }
    write_fontfile(pdf, cffont, fullname);
    xfree(fullname);
    cff_close(cffont);
}

/*tex

    Here is a sneaky trick: fontforge knows how to convert Type1 to CFF, so I
    have defined a utility function in luafflib.c that does exactly that. If it
    works out ok, I will clean up this code.

*/

void writetype1w(PDF pdf, fd_entry * fd)
{
    cff_font *cff;
    int i;
    FILE *fp;
    ff_entry *ff;
    unsigned char *tfm_buffer = NULL;
    int tfm_size = 0;
    ff = check_ff_exist(fd->fm->ff_name, 0);
    fp = fopen(ff->ff_path, "rb");
    cur_file_name = ff->ff_path;
    if (!fp) {
        formatted_error("cff","could not open Type1 font: %s", cur_file_name);
    }
    fclose(fp);
    if (is_subsetted(fd->fm)) {
        report_start_file(filetype_subset,cur_file_name);
    } else {
        report_start_file(filetype_font,cur_file_name);
    }
    (void) ff_createcff(ff->ff_path, &tfm_buffer, &tfm_size);
    if (tfm_size > 0) {
        cff = read_cff(tfm_buffer, tfm_size, 0);
        if (cff != NULL) {
            write_cff(pdf, cff, fd);
        } else {
            for (i = 0; i < tfm_size; i++)
                strbuf_putchar(pdf->fb, tfm_buffer[i]);
        }
        fd->ff_found = 1;
    } else {
        formatted_error("cff","could not understand Type1 font: %s",cur_file_name);
    }
    if (is_subsetted(fd->fm)) {
        report_stop_file(filetype_subset);
    } else {
        report_stop_file(filetype_font);
    }
    cur_file_name = NULL;
}
