% writecff.w
%
% Copyright 2006-2010 Taco Hoekwater <taco@@luatex.org>
%
% This file is part of LuaTeX.
%
% LuaTeX is free software; you can redistribute it and/or modify it under
% the terms of the GNU General Public License as published by the Free
% Software Foundation; either version 2 of the License, or (at your
% option) any later version.
%
% LuaTeX is distributed in the hope that it will be useful, but WITHOUT
% ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
% FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
% License for more details.
%
% You should have received a copy of the GNU General Public License along
% with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

@ @c


#include "ptexlib.h"
#include "lua/luatex-api.h"
#include "font/writecff.h"

extern int cidset;

@ @c
#define get_offset(s,n) get_unsigned(s, (n))
#define get_card8(a)  (card8)(a->stream[a->offset++])
#define get_card16(a) (card16)(get_unsigned(a,2))

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

@ @c

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


@ Only read header part but not body
@c
cff_index *cff_get_index_header(cff_font * cff)
{
    cff_index *idx;
    card16 i, count;

    idx = xcalloc(1, sizeof(cff_index));

    idx->count = count = get_card16(cff);
    if (count > 0) {
        idx->offsize = get_card8(cff);
        if (idx->offsize < 1 || idx->offsize > 4)
            normal_error("cff","invalid offsize data");

        idx->offset =
            xmalloc((unsigned) (((unsigned) count + 1) * sizeof(l_offset)));
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



@ @c
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
            normal_error("cff","invalid offsize data");

        idx->offset =
            xmalloc((unsigned) (((unsigned) count + 1) * sizeof(l_offset)));
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


@ @c
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

@ @c
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

@ @c
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


@ @c
void cff_release_index(cff_index * idx)
{
    if (idx) {
        xfree(idx->data);
        xfree(idx->offset);
        xfree(idx);
    }
}

@ @c
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


@ @c
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

@ @c
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

@ @c
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


@ @c
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

@ @c
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


@ @c
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
    memmove(idx->data, name, strlen(name));     /* no trailing |'\0'| */

    return (long) (5 + strlen(name));
}

long cff_put_header(cff_font * cff, card8 * dest, long destlen)
{
    if (destlen < 4)
        normal_error("cff","not enough space available");

    *(dest++) = cff->header_major;
    *(dest++) = cff->header_minor;
    *(dest++) = 4;              /* Additional data in between header and
                                 * Name INDEX ignored.
                                 */
    /* We will set all offset (0) to four-byte integer. */
    *(dest++) = 4;
    cff->header_offsize = 4;

    return 4;
}

@ @c
#define CFF_PARSE_OK                0
#define CFF_CFF_ERROR_PARSE_CFF_ERROR      -1
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

@

Operand stack:
 only numbers are stored (as double)

Operand types:

number : double (integer or real)
boolean: stored as a number
SID    : stored as a number
array  : array of numbers
delta  : array of numbers

@c
#define CFF_DICT_STACK_LIMIT 64
static int stack_top = 0;
static double arg_stack[CFF_DICT_STACK_LIMIT];

@
CFF DICT encoding:
TODO: default values

@c
#define CFF_LAST_DICT_OP1 22
#define CFF_LAST_DICT_OP2 39
#define CFF_LAST_DICT_OP (CFF_LAST_DICT_OP1 + CFF_LAST_DICT_OP2)

static struct {
    const char *opname;
    int argtype;
} dict_operator[CFF_LAST_DICT_OP] = {
    {
    "version", CFF_TYPE_SID}, {
    "Notice", CFF_TYPE_SID}, {
    "FullName", CFF_TYPE_SID}, {
    "FamilyName", CFF_TYPE_SID}, {
    "Weight", CFF_TYPE_SID}, {
    "FontBBox", CFF_TYPE_ARRAY}, {
    "BlueValues", CFF_TYPE_DELTA}, {
    "OtherBlues", CFF_TYPE_DELTA}, {
    "FamilyBlues", CFF_TYPE_DELTA}, {
    "FamilyOtherBlues", CFF_TYPE_DELTA}, {
    "StdHW", CFF_TYPE_NUMBER}, {
    "StdVW", CFF_TYPE_NUMBER}, {
    NULL, -1},                  /* first byte of two-byte operator */
        /* Top */
    {
    "UniqueID", CFF_TYPE_NUMBER}, {
    "XUID", CFF_TYPE_ARRAY}, {
    "charset", CFF_TYPE_OFFSET}, {
    "Encoding", CFF_TYPE_OFFSET}, {
    "CharStrings", CFF_TYPE_OFFSET}, {
    "Private", CFF_TYPE_SZOFF}, /* two numbers (size and offset) */
        /* Private */
    {
    "Subrs", CFF_TYPE_OFFSET}, {
    "defaultWidthX", CFF_TYPE_NUMBER}, {
    "nominalWidthX", CFF_TYPE_NUMBER},
        /* Operator 2 */
    {
    "Copyright", CFF_TYPE_SID}, {
    "IsFixedPitch", CFF_TYPE_BOOLEAN}, {
    "ItalicAngle", CFF_TYPE_NUMBER}, {
    "UnderlinePosition", CFF_TYPE_NUMBER}, {
    "UnderlineThickness", CFF_TYPE_NUMBER}, {
    "PaintType", CFF_TYPE_NUMBER}, {
    "CharstringType", CFF_TYPE_NUMBER}, {
    "FontMatrix", CFF_TYPE_ARRAY}, {
    "StrokeWidth", CFF_TYPE_NUMBER}, {
    "BlueScale", CFF_TYPE_NUMBER}, {
    "BlueShift", CFF_TYPE_NUMBER}, {
    "BlueFuzz", CFF_TYPE_NUMBER}, {
    "StemSnapH", CFF_TYPE_DELTA}, {
    "StemSnapV", CFF_TYPE_DELTA}, {
    "ForceBold", CFF_TYPE_BOOLEAN}, {
    NULL, -1}, {
    NULL, -1}, {
    "LanguageGroup", CFF_TYPE_NUMBER}, {
    "ExpansionFactor", CFF_TYPE_NUMBER}, {
    "InitialRandomSeed", CFF_TYPE_NUMBER}, {
    "SyntheticBase", CFF_TYPE_NUMBER}, {
    "PostScript", CFF_TYPE_SID}, {
    "BaseFontName", CFF_TYPE_SID}, {
    "BaseFontBlend", CFF_TYPE_DELTA},   /* MMaster ? */
    {
    NULL, -1}, {
    NULL, -1}, {
    NULL, -1}, {
    NULL, -1}, {
    NULL, -1}, {
    NULL, -1},
        /* CID-Keyed font */
    {
    "ROS", CFF_TYPE_ROS},       /* SID SID number */
    {
    "CIDFontVersion", CFF_TYPE_NUMBER}, {
    "CIDFontRevision", CFF_TYPE_NUMBER}, {
    "CIDFontType", CFF_TYPE_NUMBER}, {
    "CIDCount", CFF_TYPE_NUMBER}, {
    "UIDBase", CFF_TYPE_NUMBER}, {
    "FDArray", CFF_TYPE_OFFSET}, {
    "FDSelect", CFF_TYPE_OFFSET}, {
"FontName", CFF_TYPE_SID},};

@ Parse DICT data
@c
static double get_integer(card8 ** data, card8 * endptr, int *status)
{
    long result = 0;
    card8 b0, b1, b2;

    b0 = *(*data)++;
    if (b0 == 28 && *data < endptr - 2) {       /* shortint */
        b1 = *(*data)++;
        b2 = *(*data)++;
        result = b1 * 256 + b2;
        if (result > 0x7fffL)
            result -= 0x10000L;
    } else if (b0 == 29 && *data < endptr - 4) {        /* longint */
        int i;
        result = *(*data)++;
        if (result > 0x7f)
            result -= 0x100;
        for (i = 0; i < 3; i++) {
            result = result * 256 + (**data);
            *data += 1;
        }
    } else if (b0 >= 32 && b0 <= 246) { /* int (1) */
        result = b0 - 139;
    } else if (b0 >= 247 && b0 <= 250) {        /* int (2) */
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

@ Simply uses strtod
@c
static double get_real(card8 ** data, card8 * endptr, int *status)
{
    double result = 0.0;
    int nibble = 0, pos = 0;
    int len = 0, fail = 0;

    if (**data != 30 || *data >= endptr - 1) {
        *status = CFF_CFF_ERROR_PARSE_CFF_ERROR;
        return 0.0;
    }

    *data += 1;                 /* skip first byte (30) */

    pos = 0;
    while ((!fail) && len < WORK_BUFFER_SIZE - 2 && *data < endptr) {
        /* get nibble */
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
        } else if (nibble == 0x0b || nibble == 0x0c) {  /* E, E- */
            work_buffer[len++] = 'e';
            if (nibble == 0x0c)
                work_buffer[len++] = '-';
        } else if (nibble == 0x0e) {    /* `-' */
            work_buffer[len++] = '-';
        } else if (nibble == 0x0d) {    /* skip */
            /* do nothing */
        } else if (nibble == 0x0f) {    /* end */
            work_buffer[len++] = '\0';
            if (((pos % 2) == 0) && (**data != 0xff)) {
                fail = 1;
            }
            break;
        } else {                /* invalid */
            fail = 1;
        }
        pos++;
    }

    /* returned values */
    if (fail || nibble != 0x0f) {
        *status = CFF_CFF_ERROR_PARSE_CFF_ERROR;
    } else {
        char *s;
	/* strtod sets errno for  OVERFLOW and _maybe_ UNDERFLOW */
        /* but not for an invalid  conversion (as for example  if we try to convert "foo" in a double )*/
        /* At least in glib sets errno also for UNDERFLOW */
        /* We don't save/restore the prev. errno */
        errno=0;
        result = strtod(work_buffer, &s);
        if ( (result==0.0 && work_buffer==s) || errno ) {
              /* conversion is not possible */
             *status = CFF_CFF_ERROR_PARSE_CFF_ERROR;
         }
    }
    return result;
}

@ operators
@c
static void add_dict(cff_dict * dict,
                     card8 ** data, card8 * endptr, int *status)
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
        /* not zeroed! */
        dict->entries =
            xrealloc(dict->entries,
                     (unsigned) ((unsigned) dict->max *
                                 sizeof(cff_dict_entry)));
    }

    (dict->entries)[dict->count].id = id;
    (dict->entries)[dict->count].key = dict_operator[id].opname;
    if (argtype == CFF_TYPE_NUMBER ||
        argtype == CFF_TYPE_BOOLEAN ||
        argtype == CFF_TYPE_SID || argtype == CFF_TYPE_OFFSET) {
        /* check for underflow here, as exactly one operand is expected */
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
        /* just ignore operator if there were no operands provided;
           don't treat this as underflow (e.g. StemSnapV in TemporaLGCUni-Italic.otf) */
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
                /* reset FontMatrix to [0.001 * * 0.001 * *],
                   fix mantis bug \# 0000200 (acroread "feature") */
                (dict->entries)[dict->count].values[0] = 0.001;
                (dict->entries)[dict->count].values[3] = 0.001;
            }
            dict->count += 1;
        }
    }

    *data += 1;

    return;
}


@ All operands are treated as number or array of numbers.
  Private: two numbers, size and offset
  ROS    : three numbers, SID, SID, and a number

@c
cff_dict *cff_dict_unpack(card8 * data, card8 * endptr)
{
    cff_dict *dict;
    int status = CFF_PARSE_OK;

    stack_top = 0;

    dict = cff_new_dict();
    while (data < endptr && status == CFF_PARSE_OK) {
        if (*data < 22) {       /* operator */
            add_dict(dict, &data, endptr, &status);
        } else if (*data == 30) {       /* real - First byte of a sequence (variable) */
            if (stack_top < CFF_DICT_STACK_LIMIT) {
                arg_stack[stack_top] = get_real(&data, endptr, &status);
                stack_top++;
            } else {
                status = CFF_CFF_ERROR_STACK_OVERFLOW;
            }
        } else if (*data == 255 || (*data >= 22 && *data <= 27)) {      /* reserved */
            data++;
        } else {                /* everything else are integer */
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

@ @c
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

@ @c
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

@ @c
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

@ @c
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
    } else {
        if (cff->private[0] == NULL ||
            !cff_dict_known(cff->private[0], "Subrs")) {
            (cff->subrs)[0] = NULL;
        } else {
            offset = (long) cff_dict_get(cff->topdict, "Private", 1);
            offset += (long) cff_dict_get(cff->private[0], "Subrs", 0);
            cff->offset = (l_offset) offset;
            (cff->subrs)[0] = cff_get_index(cff);
            len += cff_index_size((cff->subrs)[0]);
        }
    }

    return len;
}


@ @c
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

    /* must exist */
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


@ @c
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
                (size = (long) cff_dict_get(cff->fdarray[i], "Private", 0))
                > 0) {
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


@ @c
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
    cff->header_offsize = get_card8(cff);
    if (cff->header_offsize < 1 || cff->header_offsize > 4) {
        normal_warning("cff","invalid offsize data");
        cff_close(cff);
        return NULL;
    }
    if (cff->header_major > 1) {
        formatted_warning("cff","major version %u not supported", cff->header_major);
        cff_close(cff);
        return NULL;
    }
    cff->offset = cff->header_hdr_size;

    /* Name INDEX */
    idx = cff_get_index(cff);
    if (n > idx->count - 1) {
        normal_warning("cff","invalid fontset index number");
        cff_close(cff);
        return NULL;
    }

    cff->name = idx;

    cff->fontname = cff_get_name(cff);

    /* Top DICT INDEX */
    idx = cff_get_index(cff);
    if (n > idx->count - 1) {
        normal_warning("cff","top DICT not exist");
        cff_close(cff);
        return NULL;
    }
    cff->topdict = cff_dict_unpack(idx->data + idx->offset[n] - 1,
                                   idx->data + idx->offset[n + 1] - 1);
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

    /* String INDEX */
    cff->string = cff_get_index(cff);

    /* offset to GSubr */
    cff->gsubr_offset = cff->offset;

    /* Number of glyphs */
    offset = (long) cff_dict_get(cff->topdict, "CharStrings", 0);
    cff->offset = (l_offset) offset;
    cff->num_glyphs = get_card16(cff);

    /* Check for font type */
    if (cff_dict_known(cff->topdict, "ROS")) {
        cff->flag |= FONTTYPE_CIDFONT;
    } else {
        cff->flag |= FONTTYPE_FONT;
    }

    /* Check for encoding */
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

    cff->offset = cff->gsubr_offset;    /* seek back to GSubr */

    return cff;
}

@* write a cff for opentype.

@ Pack DICT data
@c
static long pack_integer(card8 * dest, long destlen, long value)
{
    long len = 0;

    if (value >= -107 && value <= 107) {
        if (destlen < 1)
            normal_error("cff","buffer overflow");
        dest[0] = (card8) ((value + 139) & 0xff);
        len = 1;
    } else if (value >= 108 && value <= 1131) {
        if (destlen < 2)
            normal_error("cff","buffer overflow");
        value = (long) 0xf700u + value - 108;
        dest[0] = (card8) ((value >> 8) & 0xff);
        dest[1] = (card8) (value & 0xff);
        len = 2;
    } else if (value >= -1131 && value <= -108) {
        if (destlen < 2)
            normal_error("cff","buffer overflow");
        value = (long) 0xfb00u - value - 108;
        dest[0] = (card8) ((value >> 8) & 0xff);
        dest[1] = (card8) (value & 0xff);
        len = 2;
    } else if (value >= -32768 && value <= 32767) {     /* shortint */
        if (destlen < 3)
            normal_error("cff","buffer overflow");
        dest[0] = 28;
        dest[1] = (card8) ((value >> 8) & 0xff);
        dest[2] = (card8) (value & 0xff);
        len = 3;
    } else {                    /* longint */
        if (destlen < 5)
            normal_error("cff","buffer overflow");
        dest[0] = 29;
        dest[1] = (card8) ((value >> 24) & 0xff);
        dest[2] = (card8) ((value >> 16) & 0xff);
        dest[3] = (card8) ((value >> 8) & 0xff);
        dest[4] = (card8) (value & 0xff);
        len = 5;
    }
    return len;
}

@ @c
static long pack_real(card8 * dest, long destlen, double value)
{
    long e;
    int i = 0, pos = 2;
    int res;
#define CFF_REAL_MAX_LEN 17

    if (destlen < 2)
        normal_error("cff","buffer overflow");

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

    res=sprintf(work_buffer, "%1.14g", value);
    if (res<0) normal_error("cff","invalid conversion");
    if (res>CFF_REAL_MAX_LEN) res=CFF_REAL_MAX_LEN;

    for (i = 0; i < res; i++) {
        unsigned char ch = 0;

        if (work_buffer[i] == '\0') {
	  /* res should prevent this.  */
	  /* normal_error("cff","cannot happen"); */
            break;
        } else if (work_buffer[i] == '.') {
            ch = 0x0a;
        } else if (work_buffer[i] >= '0' && work_buffer[i] <= '9') {
            ch = (unsigned char) (work_buffer[i] - '0');
        } else {
            normal_error("cff","invalid character");
        }

        if (destlen < pos / 2 + 1)
            normal_error("cff","buffer overflow");

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
                normal_error("cff","buffer overflow");
            dest[pos / 2] = (card8) (0xb0);
        }
        pos++;
    } else if (e < 0) {
        if (pos % 2) {
            dest[pos / 2] = (card8) (dest[pos / 2] + 0x0c);
        } else {
            if (destlen < pos / 2 + 1)
                normal_error("cff","buffer overflow");
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
                normal_error("cff","buffer overflow");

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
            normal_error("cff","buffer overflow");
        dest[pos / 2] = (card8) (0xff);
        pos += 2;
    }

    return pos / 2;
}

@ @c
static long cff_dict_put_number(double value,
                                card8 * dest, long destlen, int type)
{
    long len = 0;
    double nearint;

    nearint = floor(value + 0.5);
    /* set offset to longint */
    if (type == CFF_TYPE_OFFSET) {
        long lvalue;

        lvalue = (long) value;
        if (destlen < 5)
            normal_error("cff","buffer overflow");
        dest[0] = 29;
        dest[1] = (card8) ((lvalue >> 24) & 0xff);
        dest[2] = (card8) ((lvalue >> 16) & 0xff);
        dest[3] = (card8) ((lvalue >> 8) & 0xff);
        dest[4] = (card8) (lvalue & 0xff);
        len = 5;
    } else if (value > CFF_INT_MAX || value < CFF_INT_MIN || (fabs(value - nearint) > 1.0e-5)) {        /* real */
        len = pack_real(dest, destlen, value);
    } else {                    /* integer */
        len = pack_integer(dest, destlen, (long) nearint);
    }

    return len;
}

@ @c
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
            len += cff_dict_put_number(de->values[i],
                                       dest + len, destlen - len, type);
        }
        if (id >= 0 && id < CFF_LAST_DICT_OP1) {
            if (len + 1 > destlen)
                normal_error("cff","buffer overflow");
            dest[len++] = (card8) id;
        } else if (id >= 0 && id < CFF_LAST_DICT_OP) {
            if (len + 2 > destlen)
                normal_error("cff","buffer overflow");
            dest[len++] = 12;
            dest[len++] = (card8) (id - CFF_LAST_DICT_OP1);
        } else {
            normal_error("cff","invalid DICT operator ID");
        }
    }

    return len;
}

@ @c
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


@ @c
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
            xrealloc(dict->entries,
                     (unsigned) ((unsigned) dict->max *
                                 sizeof(cff_dict_entry)));
    }

    (dict->entries)[dict->count].id = id;
    (dict->entries)[dict->count].key = dict_operator[id].opname;
    (dict->entries)[dict->count].count = count;
    if (count > 0) {
        (dict->entries)[dict->count].values =
            xcalloc((unsigned) count, sizeof(double));
    } else {
        (dict->entries)[dict->count].values = NULL;
    }
    dict->count += 1;

    return;
}


@ @c
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

@ @c
void cff_dict_set(cff_dict * dict, const char *key, int idx, double value)
{
    int i;

    assert(dict && key);

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


@ Strings
@c
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

@ @c
long cff_get_sid(cff_font * cff, const char *str)
{
    card16 i;

    if (!cff || !str)
        return -1;

    /* I search String INDEX first. */
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


@ @c
void cff_update_string(cff_font * cff)
{
    if (cff == NULL)
        normal_error("cff","CFF font not opened");

    if (cff->string)
        cff_release_index(cff->string);
    cff->string = cff->_string;
    cff->_string = NULL;
}


@ @c
s_SID cff_add_string(cff_font * cff, const char *str)
{
    card16 idx;
    cff_index *strings;
    l_offset offset, size;

    if (cff == NULL)
        normal_error("cff","CFF font not opened");

    if (cff->_string == NULL)
        cff->_string = cff_new_index(0);
    strings = cff->_string;

    for (idx = 0; idx < strings->count; idx++) {
        size = strings->offset[idx + 1] - strings->offset[idx];
        offset = strings->offset[idx];
        if (size == strlen(str) &&
            !memcmp(strings->data + offset - 1, str, strlen(str)))
            return (s_SID) (idx + CFF_STDSTR_MAX);
    }

    for (idx = 0; idx < CFF_STDSTR_MAX; idx++) {
        if (cff_stdstr[idx] && !strcmp(cff_stdstr[idx], str))
            return idx;
    }
    offset = (strings->count > 0) ? strings->offset[strings->count] : 1;
    strings->offset =
        xrealloc(strings->offset,
                 (unsigned) (((unsigned) strings->count +
                              2) * sizeof(l_offset)));
    if (strings->count == 0)
        strings->offset[0] = 1;
    idx = strings->count;
    strings->count = (card16) (strings->count + 1);
    strings->offset[strings->count] = offset + strlen(str);
    strings->data =
        xrealloc(strings->data,
                 (unsigned) ((offset + strlen(str) - 1) * sizeof(card8)));
    memcpy(strings->data + offset - 1, str, strlen(str));

    return (s_SID) (idx + CFF_STDSTR_MAX);
}


@ @c
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
                (dict->entries)[i].values[0] = cff_add_string(cff, str);
                xfree(str);
            } else if (dict_operator[id].argtype == CFF_TYPE_ROS) {
                str = cff_get_string(cff, (s_SID) (dict->entries)[i].values[0]);
                (dict->entries)[i].values[0] = cff_add_string(cff, str);
                xfree(str);
                str = cff_get_string(cff, (s_SID) (dict->entries)[i].values[1]);
                (dict->entries)[i].values[1] = cff_add_string(cff, str);
                xfree(str);
            }

        }
    }
}

@ charsets
@c
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

    if (offset == 0) {          /* predefined */
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

    /* Not sure. Not well documented. */
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
        /* fprintf(stdout, "count=%d\n", count); */
        normal_warning("cff","charset data possibly broken (too many glyphs)");
    }

    return length;
}

@ @c
long cff_pack_charsets(cff_font * cff, card8 * dest, long destlen)
{
    long len = 0;
    card16 i;
    cff_charsets *charset;

    if (cff->flag & HAVE_STANDARD_CHARSETS || cff->charsets == NULL)
        return 0;

    if (destlen < 1)
        normal_error("cff","buffer overflow");

    charset = cff->charsets;

    dest[len++] = charset->format;
    switch (charset->format) {
    case 0:
        if (destlen < len + (charset->num_entries) * 2)
            normal_error("cff","buffer overflow");
        for (i = 0; i < (charset->num_entries); i++) {
            s_SID sid = (charset->data).glyphs[i];      /* or CID */
            dest[len++] = (card8) ((sid >> 8) & 0xff);
            dest[len++] = (card8) (sid & 0xff);
        }
        break;
    case 1:
        {
            if (destlen < len + (charset->num_entries) * 3)
                normal_error("cff","buffer overflow");
            for (i = 0; i < (charset->num_entries); i++) {
                dest[len++] =
                    (card8) (((charset->data).range1[i].first >> 8) & 0xff);
                dest[len++] = (card8) ((charset->data).range1[i].first & 0xff);
                dest[len++] = (card8) ((charset->data).range1[i].n_left);
            }
        }
        break;
    case 2:
        {
            if (destlen < len + (charset->num_entries) * 4)
                normal_error("cff","buffer overflow");
            for (i = 0; i < (charset->num_entries); i++) {
                dest[len++] =
                    (card8) (((charset->data).range2[i].first >> 8) & 0xff);
                dest[len++] = (card8) ((charset->data).range2[i].first & 0xff);
                dest[len++] =
                    (card8) (((charset->data).range2[i].n_left >> 8) & 0xff);
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



@* Type 2 Charstring support.

Decode and encode Type 2 charstring

All local/global subroutine calls in a given charstring is replace by the
content of subroutine charstrings. We do this because some PostScript RIP
may have problems with sparse subroutine array. Workaround for this is to
re-order subroutine array so that no gap appears in the subroutine array,
or put dummy charstrings that contains only `return' in the gap. However,
re-ordering of subroutine is rather difficult for Type 2 charstrings due
to the bias which depends on the total number of subroutines. Replacing
callgsubr/callsubr calls with the content of the corresponding subroutine
charstring may be more efficient than putting dummy subroutines in the
case of subsetted font. Adobe distiller seems doing same thing.

And also note that subroutine numbers within subroutines can depend on the
content of operand stack as follows:

\.{  ... l m callsubr << subr \#(m+bias): n add callsubr >> ...}

I've not implemented the `random' operator which generates a pseudo-random
number in the range (0, 1] and push them into argument stack.
How pseudo-random sequences are generated is not documented in the Type 2
charstring spec..


@c
#define CS_TYPE2_DEBUG_STR "Type2 Charstring Parser"
#define CS_TYPE2_DEBUG     5

/* decoder/encoder status codes */
#define CS_BUFFER_CFF_ERROR -3
#define CS_STACK_CFF_ERROR  -2
#define CS_PARSE_CFF_ERROR  -1
#define CS_PARSE_OK      0
#define CS_PARSE_END     1
#define CS_SUBR_RETURN   2
#define CS_CHAR_END      3

static int status = CS_PARSE_CFF_ERROR;

#define DST_NEED(a,b) {if ((a) < (b)) { status = CS_BUFFER_CFF_ERROR ; return ; }}
#define SRC_NEED(a,b) {if ((a) < (b)) { status = CS_PARSE_CFF_ERROR  ; return ; }}
#define NEED(a,b)     {if ((a) < (b)) { status = CS_STACK_CFF_ERROR  ; return ; }}

/* hintmask and cntrmask need number of stem zones */
static int num_stems = 0;
static int phase = 0;

/* subroutine nesting */
static int cs2_nest = 0;

/* advance width */
static int have_width = 0;
static double width = 0.0;

@
 Standard Encoding Accented Characters:
 Optional four arguments for endchar. See, CFF spec., p.35.
  This is obsolete feature and is no longer supported.
@c
#if 0
/* adx ady bchar achar endchar */
static double seac[4] = { 0.0, 0.0, 0.0, 0.0 };
#endif

@  Operand stack and Transient array
@c
static int cs2_stack_top = 0;
static double cs2_arg_stack[CS_ARG_STACK_MAX];
static double trn_array[CS_TRANS_ARRAY_MAX];

@ Type 2 CharString encoding
@c

/*
 1-byte CharString operaotrs:
 |cs_escape| is first byte of two-byte operator
*/

/*      RESERVED      0 */
#define cs_hstem      1
/*      RESERVED      2 */
#define cs_vstem      3
#define cs_vmoveto    4
#define cs_rlineto    5
#define cs_hlineto    6
#define cs_vlineto    7
#define cs_rrcurveto  8
/*      |cs_closepath|  9  : TYPE1 */
#define cs_callsubr   10
#define cs_return     11
#define cs_escape     12
/*      |cs_hsbw|       13 : TYPE1 */
#define cs_endchar    14
/*      RESERVED      15 */
/*      RESERVED      16 */
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
/*      SHORTINT      28 : first byte of shortint*/
#define cs_callgsubr  29
#define cs_vhcurveto  30
#define cs_hvcurveto  31

/*
 2-byte CharString operaotrs:
 "dotsection" is obsoleted in Type 2 charstring.
 */

#define cs_dotsection 0
/*      |cs_vstem3|     1 : TYPE1 */
/*      |cs_hstem3|     2 : TYPE1 */
#define cs_and        3
#define cs_or         4
#define cs_not        5
/*      |cs_seac|       6 : TYPE1 */
/*      |cs_sbw|        7 : TYPE1 */
/*      RESERVED      8  */
#define cs_abs        9
#define cs_add        10
#define cs_sub        11
#define cs_div        12
/*      RESERVED      13 */
#define cs_neg        14
#define cs_eq         15
/*      |cs_callothersubr| 16 : TYPE1 */
/*      |cs_pop|           17 : TYPE1 */
#define cs_drop       18
/*      RESERVED      19 */
#define cs_put        20
#define cs_get        21
#define cs_ifelse     22
#define cs_random     23
#define cs_mul        24
/*      RESERVED      25 */
#define cs_sqrt       26
#define cs_dup        27
#define cs_exch       28
#define cs_index      29
#define cs_roll       30
/*      |cs_setcurrentpoint| 31 : TYPE1 */
/*      RESERVED      32 */
/*      RESERVED      33 */
#define cs_hflex      34
#define cs_flex       35
#define cs_hflex1     36
#define cs_flex1      37

@
|clear_stack()| put all operands sotred in operand stack to dest.
@c
static void clear_stack(card8 ** dest, card8 * limit)
{
    int i;

    for (i = 0; i < cs2_stack_top; i++) {
        double value;
        long ivalue;
        value = cs2_arg_stack[i];
        /* Nearest integer value */
        ivalue = (long) floor(value + 0.5);
        if (value >= 0x8000L || value <= (-0x8000L - 1)) {
            /*
             This number cannot be represented as a single operand.
             We must use `a b mul ...' or `a c div' to represent large values.
             */
            normal_error("cff","argument value too large (this is bug)");
        } else if (fabs(value - (double) ivalue) > 3.0e-5) {
            /* 16.16-bit signed fixed value  */
            DST_NEED(limit, *dest + 5);
            *(*dest)++ = 255;
            ivalue = (long) floor(value);       /* mantissa */
            *(*dest)++ = (card8) ((ivalue >> 8) & 0xff);
            *(*dest)++ = (card8) (ivalue & 0xff);
            ivalue = (long) ((value - (double) ivalue) * 0x10000l);     /* fraction */
            *(*dest)++ = (card8) ((ivalue >> 8) & 0xff);
            *(*dest)++ = (card8) (ivalue & 0xff);
            /* Everything else are integers. */
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
        } else if (ivalue >= -32768 && ivalue <= 32767) {       /* shortint */
            DST_NEED(limit, *dest + 3);
            *(*dest)++ = 28;
            *(*dest)++ = (card8) ((ivalue >> 8) & 0xff);
            *(*dest)++ = (card8) ((ivalue) & 0xff);
        } else {                /* Shouldn't come here */
            normal_error("cff","unexpected error");
        }
    }

    cs2_stack_top = 0;          /* clear stack */

    return;
}

@
 Single byte operators:
  Path construction, Operator for finishing a path, Hint operators.

 phase:
 \item 0: inital state
 \item 1: hint declaration, first stack-clearing operator appeared
 \item 2: in path construction

@c
static void
do_operator1(card8 ** dest, card8 * limit, card8 ** data, card8 * endptr)
{
    card8 op = **data;

    *data += 1;

    switch (op) {
    case cs_hstemhm:
    case cs_vstemhm:
        /* charstring may have hintmask if above operator have seen */
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
        /* above oprators are candidate for first stack-clearing operator */
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
        /* all operotors above are stack-clearing operator */
        /* no output */
    case cs_return:
    case cs_callgsubr:
    case cs_callsubr:
        normal_error("cff","unexpected call(g)subr/return");
        break;
    default:
        /* no-op ? */
        formatted_warning("cff","%s: unknown charstring operator: 0x%02x", CS_TYPE2_DEBUG_STR, op);
        status = CS_PARSE_CFF_ERROR;
        break;
    }

    return;
}

@
 Double byte operators:
 Flex, arithmetic, conditional, and storage operators.

 Following operators are not supported:
   random: How random ?

@c
static void
do_operator2(card8 ** dest, card8 * limit, card8 ** data, card8 * endptr)
{
    card8 op;

    *data += 1;

    SRC_NEED(endptr, *data + 1);

    op = **data;
    *data += 1;

    switch (op) {
    case cs_dotsection:        /* deprecated */
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
        /* all operator above are stack-clearing */
        /* no output */
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
    case cs_div:               /* doesn't check overflow */
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
        NEED(cs2_stack_top, 2); /* need two arguments at least */
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
        /* no-op ? */
        formatted_warning("cff","%s: unknown charstring operator: 0x0c%02x", CS_TYPE2_DEBUG_STR, op);
        status = CS_PARSE_CFF_ERROR;
        break;
    }

    return;
}

@
 integer:
  exactly the same as the DICT encoding (except 29)
@c
static void cs2_get_integer(card8 ** data, card8 * endptr)
{
    long result = 0;
    card8 b0 = **data, b1, b2;

    *data += 1;

    if (b0 == 28) {             /* shortint */
        SRC_NEED(endptr, *data + 2);
        b1 = **data;
        b2 = *(*data + 1);
        result = b1 * 256 + b2;
        if (result > 0x7fff)
            result -= 0x10000L;
        *data += 2;
    } else if (b0 >= 32 && b0 <= 246) { /* int (1) */
        result = b0 - 139;
    } else if (b0 >= 247 && b0 <= 250) {        /* int (2) */
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

@
Signed 16.16-bits fixed number for Type 2 charstring encoding
@c
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

@
 Subroutines:
  The bias for subroutine number is introduced in type 2 charstrings.

  subr:     set to a pointer to the subroutine charstring.
  len:      set to the length of subroutine charstring.
  |subr_idx|: CFF INDEX data that contains subroutines.
  id:       biased subroutine number.

@c
static void get_subr(card8 ** subr, long *len, cff_index * subr_idx, long id)
{
    card16 count;

    if (subr_idx == NULL)
        formatted_error("cff","%s: subroutine called but no subroutine found",CS_TYPE2_DEBUG_STR);

    count = subr_idx->count;

    /* Adding bias number */
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


@ NOTE:
 The Type 2 interpretation of a number encoded in five-bytes (those with
 an initial byte value of 255) differs from how it is interpreted in the
 Type 1 format.

@c
static void
do_charstring(card8 ** dest, card8 * limit,
              card8 ** data, card8 * endptr,
              cff_index * gsubr_idx, cff_index * subr_idx)
{
    card8 b0 = 0, *subr;
    long len;

    if (cs2_nest > CS_SUBR_NEST_MAX)
        formatted_error("cff","%s: subroutine nested too deeply", CS_TYPE2_DEBUG_STR);

    cs2_nest++;

    while (*data < endptr && status == CS_PARSE_OK) {
        b0 = **data;
        if (b0 == 255) {        /* 16-bit.16-bit fixed signed number */
            get_fixed(data, endptr);
        } else if (b0 == cs_return) {
            status = CS_SUBR_RETURN;
        } else if (b0 == cs_callgsubr) {
            if (cs2_stack_top < 1) {
                status = CS_STACK_CFF_ERROR;
            } else {
                cs2_stack_top--;
                get_subr(&subr, &len, gsubr_idx,
                         (long) cs2_arg_stack[cs2_stack_top]);
                if (*dest + len > limit)
                    formatted_error("cff","%s: possible buffer overflow", CS_TYPE2_DEBUG_STR);
                do_charstring(dest, limit, &subr, subr + len, gsubr_idx, subr_idx);
                *data += 1;
            }
        } else if (b0 == cs_callsubr) {
            if (cs2_stack_top < 1) {
                status = CS_STACK_CFF_ERROR;
            } else {
                cs2_stack_top--;
                get_subr(&subr, &len, subr_idx,
                         (long) cs2_arg_stack[cs2_stack_top]);
                if (limit < *dest + len)
                    formatted_error("cff","%s: possible buffer overflow", CS_TYPE2_DEBUG_STR);
                do_charstring(dest, limit, &subr, subr + len, gsubr_idx, subr_idx);
                *data += 1;
            }
        } else if (b0 == cs_escape) {
            do_operator2(dest, limit, data, endptr);
        } else if (b0 < 32 && b0 != 28) {       /* 19, 20 need mask */
            do_operator1(dest, limit, data, endptr);
        } else if ((b0 <= 22 && b0 >= 27) || b0 == 31) {        /* reserved */
            status = CS_PARSE_CFF_ERROR;        /* not an error ? */
        } else {                /* integer */
            cs2_get_integer(data, endptr);
        }
    }

    if (status == CS_SUBR_RETURN) {
        status = CS_PARSE_OK;
    } else if (status == CS_CHAR_END && *data < endptr) {
        formatted_warning("cff","%s: garbage after endchar", CS_TYPE2_DEBUG_STR);
    } else if (status < CS_PARSE_OK) {  /* error */
        formatted_error("cff","%s: parsing charstring failed: (status=%d, stack=%d)", CS_TYPE2_DEBUG_STR, status, cs2_stack_top);
    }

    cs2_nest--;

    return;
}

@ @c
static void cs_parse_init(void)
{
    status = CS_PARSE_OK;
    cs2_nest = 0;
    phase = 0;
    num_stems = 0;
    cs2_stack_top = 0;
}


@ Not just copying...
@c
long
cs_copy_charstring(card8 * dst, long dstlen,
                   card8 * src, long srclen,
                   cff_index * gsubr, cff_index * subr,
                   double default_width, double nominal_width, cs_ginfo * ginfo)
{
    card8 *save = dst;

    cs_parse_init();

    width = 0.0;
    have_width = 0;

    /* expand call(g)subrs */
    do_charstring(&dst, dst + dstlen, &src, src + srclen, gsubr, subr);

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

@* encodings.

@ Encoding and Charset

Encoding and Charset arrays always begin with GID = 1.

@c
long cff_read_encoding(cff_font * cff)
{
    cff_encoding *encoding;
    long offset, length;
    card8 i;

    if (cff->topdict == NULL) {
        normal_error("cff","top DICT data not found");
    }

    if (!cff_dict_known(cff->topdict, "Encoding")) {
        cff->flag |= ENCODING_STANDARD;
        cff->encoding = NULL;
        return 0;
    }

    offset = (long) cff_dict_get(cff->topdict, "Encoding", 0);
    if (offset == 0) {          /* predefined */
        cff->flag |= ENCODING_STANDARD;
        cff->encoding = NULL;
        return 0;
    } else if (offset == 1) {
        cff->flag |= ENCODING_EXPERT;
        cff->encoding = NULL;
        return 0;
    }

    cff->offset = (l_offset) offset;
    cff->encoding = encoding = xcalloc(1, sizeof(cff_encoding));
    encoding->format = get_card8(cff);
    length = 1;

    switch (encoding->format & (~0x80)) {
    case 0:
        encoding->num_entries = get_card8(cff);
        (encoding->data).codes = xmalloc(encoding->num_entries * sizeof(card8));
        for (i = 0; i < (encoding->num_entries); i++) {
            (encoding->data).codes[i] = get_card8(cff);
        }
        length += encoding->num_entries + 1;
        break;
    case 1:
        {
            cff_range1 *ranges;
            encoding->num_entries = get_card8(cff);
            encoding->data.range1 = ranges
                = xcalloc(encoding->num_entries, sizeof(cff_range1));
            for (i = 0; i < (encoding->num_entries); i++) {
                ranges[i].first = get_card8(cff);
                ranges[i].n_left = get_card8(cff);
            }
            length += (encoding->num_entries) * 2 + 1;
        }
        break;
    default:
        xfree(encoding);
        normal_error("cff","unknown encoding format");
        break;
    }

    /* Supplementary data */
    if ((encoding->format) & 0x80) {
        cff_map *map;
        encoding->num_supps = get_card8(cff);
        encoding->supp = map = xcalloc(encoding->num_supps, sizeof(cff_map));
        for (i = 0; i < (encoding->num_supps); i++) {
            map[i].code = get_card8(cff);
            map[i].glyph = get_card16(cff);     /* SID */
        }
        length += (encoding->num_supps) * 3 + 1;
    } else {
        encoding->num_supps = 0;
        encoding->supp = NULL;
    }

    return length;
}

@ @c
long cff_pack_encoding(cff_font * cff, card8 * dest, long destlen)
{
    long len = 0;
    cff_encoding *encoding;
    card16 i;

    if (cff->flag & HAVE_STANDARD_ENCODING || cff->encoding == NULL)
        return 0;

    if (destlen < 2)
        normal_error("cff","buffer overflow");

    encoding = cff->encoding;

    dest[len++] = encoding->format;
    dest[len++] = encoding->num_entries;
    switch (encoding->format & (~0x80)) {
    case 0:
        if (destlen < len + encoding->num_entries)
            normal_error("cff","buffer overflow");
        for (i = 0; i < (encoding->num_entries); i++) {
            dest[len++] = (encoding->data).codes[i];
        }
        break;
    case 1:
        {
            if (destlen < len + (encoding->num_entries) * 2)
                normal_error("cff","buffer overflow");
            for (i = 0; i < (encoding->num_entries); i++) {
                dest[len++] = (card8) ((encoding->data).range1[i].first & 0xff);
                dest[len++] = (card8) ((encoding->data).range1[i].n_left);
            }
        }
        break;
    default:
        normal_error("cff","unknown encoding format");
        break;
    }

    if ((encoding->format) & 0x80) {
        if (destlen < len + (encoding->num_supps) * 3 + 1)
            normal_error("cff","buffer overflow");
        dest[len++] = encoding->num_supps;
        for (i = 0; i < (encoding->num_supps); i++) {
            dest[len++] = (card8) ((encoding->supp)[i].code);
            dest[len++] = (card8) (((encoding->supp)[i].glyph >> 8) & 0xff);
            dest[len++] = (card8) ((encoding->supp)[i].glyph & 0xff);
        }
    }

    return len;
}

@ CID-Keyed font specific
@c
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


@ @c
long cff_pack_fdselect(cff_font * cff, card8 * dest, long destlen)
{
    cff_fdselect *fdsel;
    long len = 0;
    card16 i;

    if (cff->fdselect == NULL)
        return 0;

    if (destlen < 1)
        normal_error("cff","buffer overflow");

    fdsel = cff->fdselect;

    dest[len++] = fdsel->format;
    switch (fdsel->format) {
    case 0:
        if (fdsel->num_entries != cff->num_glyphs)
            normal_error("cff","invalid data");
        if (destlen < len + fdsel->num_entries)
            normal_error("cff","buffer overflow");
        for (i = 0; i < fdsel->num_entries; i++) {
            dest[len++] = (fdsel->data).fds[i];
        }
        break;
    case 3:
        {
            if (destlen < len + 2)
                normal_error("cff","buffer overflow");
            len += 2;
            for (i = 0; i < (fdsel->num_entries); i++) {
                if (destlen < len + 3)
                    normal_error("cff","buffer overflow");
                dest[len++] =
                    (card8) (((fdsel->data).ranges[i].first >> 8) & 0xff);
                dest[len++] = (card8) ((fdsel->data).ranges[i].first & 0xff);
                dest[len++] = (card8) ((fdsel->data).ranges[i].fd);
            }
            if (destlen < len + 2)
                normal_error("cff","buffer overflow");
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



@ Create an instance of embeddable font.

@c
static void write_fontfile(PDF pdf, cff_font * cffont, char *fullname)
{
    cff_index *topdict, *fdarray, *private;
    unsigned char *dest;
    long destlen = 0, i, size;
    long offset, topdict_offset, fdarray_offset;

    /*  DICT sizes (offset set to long int) */
    topdict = cff_new_index(1);
    fdarray = cff_new_index(cffont->num_fds);
    private = cff_new_index(cffont->num_fds);

    cff_dict_remove(cffont->topdict, "UniqueID");
    cff_dict_remove(cffont->topdict, "XUID");
    cff_dict_remove(cffont->topdict, "Private");        /* some bad font may have */
    cff_dict_remove(cffont->topdict, "Encoding");       /* some bad font may have */

    topdict->offset[1] = (l_offset) cff_dict_pack(cffont->topdict,
                                                  (card8 *) work_buffer,
                                                  WORK_BUFFER_SIZE) + 1;
    for (i = 0; i < cffont->num_fds; i++) {
        size = 0;
        if (cffont->private && cffont->private[i]) {
            size = cff_dict_pack(cffont->private[i],
                                 (card8 *) work_buffer, WORK_BUFFER_SIZE);
            if (size < 1) {     /* Private had contained only Subr */
                cff_dict_remove(cffont->fdarray[i], "Private");
            }
        }
        (private->offset)[i + 1] =
            (unsigned long) ((private->offset)[i] + (unsigned) size);
        (fdarray->offset)[i + 1] =
            (unsigned long) ((fdarray->offset)[i] + (unsigned)
                             cff_dict_pack(cffont->fdarray[i],
                                           (card8 *) work_buffer,
                                           WORK_BUFFER_SIZE));
    }

    destlen = 4;                /* header size */
    destlen += cff_set_name(cffont, fullname);
    destlen += cff_index_size(topdict);
    destlen += cff_index_size(cffont->string);
    destlen += cff_index_size(cffont->gsubr);
    destlen += (cffont->charsets->num_entries) * 2 + 1; /* charset format 0 */
    destlen += (cffont->fdselect->num_entries) * 3 + 5; /* fdselect format 3 */
    destlen += cff_index_size(cffont->cstrings);
    destlen += cff_index_size(fdarray);
    destlen = (long) (destlen + (long) private->offset[private->count] - 1);    /* Private is not INDEX */

    dest = xcalloc((unsigned) destlen, sizeof(card8));

    offset = 0;
    /* Header */
    offset += cff_put_header(cffont, dest + offset, destlen - offset);
    /* Name */
    offset += cff_pack_index(cffont->name, dest + offset, destlen - offset);
    /* Top DICT */
    topdict_offset = offset;
    offset += cff_index_size(topdict);
    /* Strings */
    offset += cff_pack_index(cffont->string, dest + offset, destlen - offset);
    /* Global Subrs */
    offset += cff_pack_index(cffont->gsubr, dest + offset, destlen - offset);

    /* charset */
    cff_dict_set(cffont->topdict, "charset", 0, (double) offset);
    offset += cff_pack_charsets(cffont, dest + offset, destlen - offset);

    /* FDSelect */
    cff_dict_set(cffont->topdict, "FDSelect", 0, (double) offset);
    offset += cff_pack_fdselect(cffont, dest + offset, destlen - offset);

    /* CharStrings */
    cff_dict_set(cffont->topdict, "CharStrings", 0, (double) offset);
    offset += cff_pack_index(cffont->cstrings,
                             dest + offset, cff_index_size(cffont->cstrings));
    cff_release_index(cffont->cstrings);
    cffont->cstrings = NULL;    /* Charstrings cosumes huge memory */

    /* FDArray and Private */
    cff_dict_set(cffont->topdict, "FDArray", 0, (double) offset);
    fdarray_offset = offset;
    offset += cff_index_size(fdarray);

    fdarray->data =
        xcalloc((unsigned) (fdarray->offset[fdarray->count] - 1),
                sizeof(card8));
    for (i = 0; i < cffont->num_fds; i++) {
        size = (long) (private->offset[i + 1] - private->offset[i]);
        if (cffont->private[i] && size > 0) {
            cff_dict_pack(cffont->private[i], dest + offset, size);
            cff_dict_set(cffont->fdarray[i], "Private", 0, (double) size);
            cff_dict_set(cffont->fdarray[i], "Private", 1, (double) offset);
        }
        cff_dict_pack(cffont->fdarray[i],
                      fdarray->data + (fdarray->offset)[i] - 1,
                      (long) (fdarray->offset[fdarray->count] - 1));
        offset += size;
    }

    cff_pack_index(fdarray, dest + fdarray_offset, cff_index_size(fdarray));
    cff_release_index(fdarray);
    cff_release_index(private);

    /* Finally Top DICT */
    topdict->data =
        xcalloc((unsigned) (topdict->offset[topdict->count] - 1),
                sizeof(card8));
    cff_dict_pack(cffont->topdict, topdict->data,
                  (long) (topdict->offset[topdict->count] - 1));
    cff_pack_index(topdict, dest + topdict_offset, cff_index_size(topdict));
    cff_release_index(topdict);

    for (i = 0; i < offset; i++)
        strbuf_putchar(pdf->fb, dest[i]);

    xfree(dest);
    return;
}


@ this block is used a few times
@c
#define DO_COPY_CHARSTRING()                                                   \
  if ((avl_find(fd->gl_tree,glyph) != NULL)) {                                 \
      size = (long)(cs_idx->offset[code+1] - cs_idx->offset[code]);            \
    if (size > CS_STR_LEN_MAX) {                                               \
      formatted_error("cff","charstring too long: gid=%u, %ld bytes", code, size);   \
    }                                                                          \
    if (charstring_len + CS_STR_LEN_MAX >= max_len) {                          \
	max_len = (long)(charstring_len + 2 * CS_STR_LEN_MAX);		\
	charstrings->data = xrealloc(charstrings->data, (unsigned)((unsigned)max_len*sizeof(card8))); \
    }                                                                          \
    (charstrings->offset)[gid] = (unsigned)(charstring_len + 1);	\
    cffont->offset= (l_offset)((unsigned)offset + (cs_idx->offset)[code] - 1);	\
    memcpy(data,&cffont->stream[cffont->offset],(size_t)size);		\
    charstring_len += cs_copy_charstring(charstrings->data + charstring_len,   \
                                         max_len - charstring_len,	\
                                         data, size,                           \
                                         cffont->gsubr, (cffont->subrs)[0],    \
                                         default_width, nominal_width, NULL);  \
    gid++;                                                                     \
  }

@ @c
void write_cff(PDF pdf, cff_font * cffont, fd_entry * fd)
{
    cff_index *charstrings, *cs_idx;

    long charstring_len, max_len;
    long size, offset = 0;

    card8 *data;
    card16 num_glyphs, cs_count1, code, gid, last_cid;

    double nominal_width, default_width;

    char *fullname;

    glw_entry *glyph, *found;
    struct avl_traverser t;

    fullname = xcalloc((unsigned) (8 + strlen(fd->fontname)), 1);
    sprintf(fullname, "%s+%s", fd->subset_tag, fd->fontname);

    /* finish parsing the CFF */
    cff_read_private(cffont);
    cff_read_subrs(cffont);

    /*
     Widths
     */
    if (cffont->private[0] &&
        cff_dict_known(cffont->private[0], "defaultWidthX")) {
        default_width =
            (double) cff_dict_get(cffont->private[0], "defaultWidthX", 0);
    } else {
        default_width = CFF_DEFAULTWIDTHX_DEFAULT;
    }
    if (cffont->private[0] &&
        cff_dict_known(cffont->private[0], "nominalWidthX")) {
        nominal_width =
            (double) cff_dict_get(cffont->private[0], "nominalWidthX", 0);
    } else {
        nominal_width = CFF_NOMINALWIDTHX_DEFAULT;
    }

    num_glyphs = 0;
    last_cid = 0;
    glyph = xtalloc(1, glw_entry);

    /* insert notdef */
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
    }

    cff_dict_add(cffont->topdict, "CIDCount", 1);
    cff_dict_set(cffont->topdict, "CIDCount", 0, last_cid + 1);

    cffont->fdarray = xcalloc(1, sizeof(cff_dict *));
    cffont->fdarray[0] = cff_new_dict();
    cff_dict_add(cffont->fdarray[0], "FontName", 1);
    cff_dict_set(cffont->fdarray[0], "FontName", 0, (double) cff_add_string(cffont, fullname)); /* FIXME: Skip XXXXXX+ */
    cff_dict_add(cffont->fdarray[0], "Private", 2);
    cff_dict_set(cffont->fdarray[0], "Private", 0, 0.0);
    cff_dict_set(cffont->fdarray[0], "Private", 0, 0.0);
    /* FDArray  - index offset, not known yet */
    cff_dict_add(cffont->topdict, "FDArray", 1);
    cff_dict_set(cffont->topdict, "FDArray", 0, 0.0);
    /* FDSelect - offset, not known yet */
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

    /* build the new charstrings entry */
    charstrings = cff_new_index((card16) (cs_count1==USHRT_MAX?cs_count1: cs_count1 + 1));
    max_len = 2 * CS_STR_LEN_MAX;
    charstrings->data = xcalloc((unsigned) max_len, sizeof(card8));
    charstring_len = 0;

    gid = 0;
    data = xcalloc(CS_STR_LEN_MAX, sizeof(card8));

    {
        int i;
        for (i = 0; i < cs_count1; i++) {
            code = (card16) i;
            glyph->id = code;
            DO_COPY_CHARSTRING();
        }
    }

    /* CIDSet: a table of bits indexed by cid, bytes with high order bit first,
       each (set) bit is a (present) CID. */
    if (1) {
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

    /* this happens if the internal metrics do not agree with the actual disk font */
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

    /*
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

    cff_add_string(cffont, "Adobe");
    cff_add_string(cffont, "Identity");

    cff_dict_update(cffont->topdict, cffont);
    cff_dict_update(cffont->private[0], cffont);
    cff_update_string(cffont);

    /* CFF code need to be rewrote... */
    cff_dict_add(cffont->topdict, "ROS", 3);
    cff_dict_set(cffont->topdict, "ROS", 0,
                 (double) cff_get_sid(cffont, "Adobe"));
    cff_dict_set(cffont->topdict, "ROS", 1,
                 (double) cff_get_sid(cffont, "Identity"));
    cff_dict_set(cffont->topdict, "ROS", 2, 0.0);

    write_fontfile(pdf, cffont, fullname);
    xfree(fullname);
    cff_close(cffont);

}

@ @c
#undef ERROR                    /* for mingw */
#define ERROR(a) { perror(a); return 0; }

@ Input : SID or CID (16-bit unsigned int)
 Output: glyph index

@c
card16 cff_charsets_lookup(cff_font * cff, card16 cid)
{
    card16 gid = 0;
    cff_charsets *charset;
    card16 i;

    if (cff->flag & (CHARSETS_ISOADOBE | CHARSETS_EXPERT | CHARSETS_EXPSUB)) {
        ERROR("Predefined CFF charsets not supported yet");
    } else if (cff->charsets == NULL) {
        normal_error("cff","charsets data not available");
    }

    if (cid == 0) {
        return 0;               /* GID 0 (.notdef) */
    }

    charset = cff->charsets;

    gid = 0;
    switch (charset->format) {
    case 0:
        for (i = 0; i < charset->num_entries; i++) {
            if (cid == charset->data.glyphs[i]) {
                gid = (card16) (i + 1);
                return gid;
            }
        }
        break;
    case 1:
        for (i = 0; i < charset->num_entries; i++) {
            if (cid >= charset->data.range1[i].first &&
                cid <=
                charset->data.range1[i].first +
                charset->data.range1[i].n_left) {
                gid = (card16) (gid + cid - charset->data.range1[i].first + 1);
                return gid;
            }
            gid = (card16) (gid + charset->data.range1[i].n_left + 1);
        }
        break;
    case 2:
        for (i = 0; i < charset->num_entries; i++) {
            if (cid >= charset->data.range2[i].first &&
                cid <=
                charset->data.range2[i].first +
                charset->data.range2[i].n_left) {
                gid = (card16) (gid + cid - charset->data.range2[i].first + 1);
                return gid;
            }
            gid = (card16) (gid + charset->data.range2[i].n_left + 1);
        }
        break;
    default:
        normal_error("cff","unknown charset format");
    }

    return 0;                   /* not found */
}


@ @c
#define is_cidfont(a) ((a)->flag & FONTTYPE_CIDFONT)
#define CID_MAX 65535
void write_cid_cff(PDF pdf, cff_font * cffont, fd_entry * fd)
{
    cff_index *charstrings, *cs_idx;

    long charstring_len, max_len;
    long size, offset = 0;

    card8 *data;
    card16 num_glyphs, cs_count1, gid, last_cid;


    int fdsel, prev_fd, cid_count, cid;
    char *fullname;

    glw_entry *glyph;

    unsigned char *CIDToGIDMap = NULL;

    cff_fdselect *fdselect = NULL;
    cff_charsets *charset = NULL;


    if (!is_cidfont(cffont)) {
        perror("Not a CIDfont.");
        return;
    }

    fullname = xcalloc((unsigned) (8 + strlen(fd->fontname)), 1);
    sprintf(fullname, "%s+%s", fd->subset_tag, fd->fontname);

    /* finish parsing the CFF */

    if (cff_dict_known(cffont->topdict, "CIDCount")) {
        cid_count = (card16) cff_dict_get(cffont->topdict, "CIDCount", 0);
    } else {
        cid_count = CFF_CIDCOUNT_DEFAULT;
    }
    cff_read_charsets(cffont);
    CIDToGIDMap = xmalloc((unsigned)
                          ((2 * (unsigned) cid_count) * sizeof(unsigned char)));
    memset(CIDToGIDMap, 0, (size_t) (2 * cid_count));

    glyph = xtalloc(1, glw_entry);
    /* insert notdef */
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
            /*
                gid = (card16) cff_charsets_lookup(cffont, (card16) cid);
            */
            gid = (card16) cid;
            CIDToGIDMap[2 * cid] = (unsigned char) ((gid >> 8) & 0xff);
            CIDToGIDMap[2 * cid + 1] = (unsigned char) (gid & 0xff);
            last_cid = (card16) cid;
            num_glyphs++;
        }
    }
    if (last_cid >= cffont->num_glyphs) {
        formatted_error("cff font","bad glyph index %i",last_cid);
    }

    /* CIDSet: a table of bits indexed by cid, bytes with high order bit first,
       each (set) bit is a (present) CID. */
    if (1) {
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

        gid_org =
            (short unsigned) ((CIDToGIDMap[2 * cid] << 8) |
                              (CIDToGIDMap[2 * cid + 1]));
        size = (long) (cs_idx->offset[gid_org + 1] - cs_idx->offset[gid_org]);
        if (size > CS_STR_LEN_MAX) {
            formatted_error("cff","charstring too long: gid=%u, %ld bytes", cid, size);
        }
        if (charstring_len + CS_STR_LEN_MAX >= max_len) {
            max_len = charstring_len + 2 * CS_STR_LEN_MAX;
            charstrings->data =
                xrealloc(charstrings->data,
                         (unsigned) ((unsigned) max_len * sizeof(card8)));
        }
        (charstrings->offset)[gid] = (l_offset) (charstring_len + 1);
        cffont->offset =
            (l_offset) ((unsigned) offset + (cs_idx->offset)[gid_org] - 1);
        memcpy(data, &cffont->stream[cffont->offset], (size_t) size);
        fdsel = cff_fdselect_lookup(cffont, gid_org);
        charstring_len += cs_copy_charstring(charstrings->data + charstring_len,
                                             max_len - charstring_len,
                                             data, size,
                                             cffont->gsubr,
                                             (cffont->subrs)[fdsel], 0, 0,
                                             NULL);

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

    /*
     * We don't use subroutines at all.
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

@ here is a sneaky trick: fontforge knows how to convert Type1 to CFF, so
I have defined a utility function in luafflib.c that does exactly that.
If it works out ok, I will clean up this code.

@c
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
