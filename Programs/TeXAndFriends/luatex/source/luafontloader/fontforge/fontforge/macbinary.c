/* Copyright (C) 2000-2008 by George Williams */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.

 * The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "fontforgevw.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ustring.h>
#include "ttf.h"
#include "psfont.h"
#ifdef LUA_FF_LIB
#  undef __Mac
#endif
#if __Mac
#  include <ctype.h>
#  include </Developer/Headers/FlatCarbon/Files.h>
#else
#  include <utype.h>
#  undef __Mac
#  define __Mac 0
#endif

const int mac_dpi = 72;
/* I had always assumed that the mac still believed in 72dpi screens, but I */
/*  see that in geneva under OS/9, the pointsize does not match the pixel */
/*  size of the font. But the dpi is not constant (and the differences */
/*  excede those supplied by rounding errors) varying between 96 and 84dpi */

/* A Mac Resource fork */
/*  http://developer.apple.com/techpubs/mac/MoreToolbox/MoreToolbox-9.html */
/*    begins with a 16 byte header containing: */
/*	resource start offset */
/*	map start offset */
/*	resource length */
/*	map length */
/*    then 256-16 bytes of zeros */
/*    the resource section consists of (many) */
/*	4 byte length count */
/*	resource data	*/
/*    the map section contains */
/*	A copy of the 16 byte header */
/*	a 4 byte mac internal value (I hope) */
/*	another 4 bytes of mac internal values (I hope) */
/*	a 2 byte offset from the start of the map section to the list of resource types */
/*	a 2 byte offset from the start of the map section to the list of resource names */
/*	The resource type list consists of */
/*	    a 2 byte count of the number of resource types (-1) */
/*	    (many copies of) */
/*		a 4 byte resource type ('FOND' for example) */
/*		a 2 byte count of the number of resources of this type (-1) */
/*		a 2 byte offset from the type list start to the resource table */
/*	    a resource table looks like */
/*		a 2 byte offset from the resource name table to a pascal */
/*			string containing this resource's name (or 0xffff for none) */
/*		1 byte of resource flags */
/*		3 bytes of offset from the resource section to the length & */
/*			data of this instance of the resource type */
/*		4 bytes of 0 */
/*	The resource name section consists of */
/*	    a bunch of pascal strings (ie. preceded by a length byte) */

/* The POST resource isn't noticeably documented, it's pretty much a */
/*  straight copy of the pfb file cut up into 0x800 byte chunks. */
/*  (each section of the pfb file has it's own set of chunks, the last may be smaller than 0x800) */
/* The NFNT resource http://developer.apple.com/techpubs/mac/Text/Text-250.html */
/* The FOND resource http://developer.apple.com/techpubs/mac/Text/Text-269.html */
/* The sfnt resource is basically a copy of the ttf file */

/* A MacBinary file */
/*  http://www.lazerware.com/formats/macbinary.html */
/*    begins with a 128 byte header */
/*	(which specifies lengths for data/resource forks) */
/*	(and contains mac type/creator data) */
/*	(and other stuff) */
/*	(and finally a crc checksum) */
/*    is followed by the data section (padded to a mult of 128 bytes) */
/*    is followed by the resource section (padded to a mult of 128 bytes) */

/* ******************************** Creation ******************************** */


struct resource {
    uint32 pos;
    uint8 flags;
    uint16 id;
    char *name;
    uint32 nameloc;
    uint32 nameptloc;
};

struct resourcetype {
    uint32 tag;
    struct resource *res;
    uint32 resloc;
};

struct macbinaryheader {
    char *macfilename;
    char *binfilename;          /* if macfilename is null and this is set we will figure out macfilename by removing .bin */
    uint32 type;
    uint32 creator;
};


enum psstyle_flags { psf_bold = 1, psf_italic = 2, psf_outline = 4,
    psf_shadow = 0x8, psf_condense = 0x10, psf_extend = 0x20
};

uint16 _MacStyleCode(char *styles, SplineFont * sf, uint16 * psstylecode)
{
    unsigned short stylecode = 0, psstyle = 0;

    if (strstrmatch(styles, "Bold") || strstrmatch(styles, "Demi") ||
        strstrmatch(styles, "Heav") || strstrmatch(styles, "Blac") ||
/* A few fonts have German/French styles in their names */
        strstrmatch(styles, "Fett") || strstrmatch(styles, "Gras")) {
        stylecode = sf_bold;
        psstyle = psf_bold;
    } else if (sf != NULL && sf->weight != NULL &&
               (strstrmatch(sf->weight, "Bold")
                || strstrmatch(sf->weight, "Demi")
                || strstrmatch(sf->weight, "Heav")
                || strstrmatch(sf->weight, "Blac")
                || strstrmatch(sf->weight, "Fett")
                || strstrmatch(sf->weight, "Gras"))) {
        stylecode = sf_bold;
        psstyle = psf_bold;
    }
    /* URW uses four leter abbreviations of Italic and Oblique */
    /* Somebody else uses two letter abbrevs */
    if ((sf != NULL && sf->italicangle != 0) ||
        strstrmatch(styles, "Ital") ||
        strstrmatch(styles, "Obli") ||
        strstrmatch(styles, "Slanted") ||
        strstrmatch(styles, "Kurs") || strstr(styles, "It")) {
        stylecode |= sf_italic;
        psstyle |= psf_italic;
    }
    if (strstrmatch(styles, "Underline")) {
        stylecode |= sf_underline;
    }
    if (strstrmatch(styles, "Outl")) {
        stylecode |= sf_outline;
        psstyle |= psf_outline;
    }
    if (strstr(styles, "Shadow") != NULL) {
        stylecode |= sf_shadow;
        psstyle |= psf_shadow;
    }
    if (strstrmatch(styles, "Cond") || strstr(styles, "Cn") ||
        strstrmatch(styles, "Narrow")) {
        stylecode |= sf_condense;
        psstyle |= psf_condense;
    }
    if (strstrmatch(styles, "Exte") || strstr(styles, "Ex")) {
        stylecode |= sf_extend;
        psstyle |= psf_extend;
    }
    if ((psstyle & psf_extend) && (psstyle & psf_condense)) {
        if (sf != NULL)
            LogError(_
                     ("Warning: %s(%s) is both extended and condensed. That's impossible.\n"),
                     sf->fontname, sf->origname);
        else
            LogError(_
                     ("Warning: Both extended and condensed. That's impossible.\n"));
        psstyle &= ~psf_extend;
        stylecode &= ~sf_extend;
    }
    if (psstylecode != NULL)
        *psstylecode = psstyle;
    return (stylecode);
}



/* ******************************** Reading ********************************* */

static SplineFont *SearchPostscriptResources(FILE * f, long rlistpos,
                                             int subcnt, long rdata_pos,
                                             long name_list, int flags)
{
    long here = ftell(f);
    long *offsets, lenpos;
    int rname = -1, tmp;
    int ch1, ch2;
    int len, type, i, j, rlen;
    unsigned short id, *rsrcids;
    /* I don't pretend to understand the rational behind the format of a */
    /*  postscript font. It appears to be split up into chunks where the */
    /*  maximum chunk size is 0x800, each section (ascii, binary, ascii, eof) */
    /*  has its own set of chunks (ie chunks don't cross sections) */
    char *buffer = NULL;
    int max = 0;
    FILE *pfb;
    FontDict *fd;
    SplineFont *sf;
    (void) name_list;
    fseek(f, rlistpos, SEEK_SET);
    rsrcids = gcalloc(subcnt, sizeof(short));
    offsets = gcalloc(subcnt, sizeof(long));
    for (i = 0; i < subcnt; ++i) {
        rsrcids[i] = getushort(f);
        tmp = (short) getushort(f);
        if (rname == -1)
            rname = tmp;
        /* flags = */ getc(f);
        ch1 = getc(f);
        ch2 = getc(f);
        offsets[i] = rdata_pos + ((ch1 << 16) | (ch2 << 8) | getc(f));
        /* mbz = */ getlong(f);
    }

    pfb = tmpfile();
    if (pfb == NULL) {
        LogError(_("Can't open temporary file for postscript output\n"));
        fseek(f, here, SEEK_SET);
        free(offsets);
        return (NULL);
    }

    putc(0x80, pfb);
    putc(1, pfb);
    lenpos = ftell(pfb);
    putc(0, pfb);
    putc(0, pfb);
    putc(0, pfb);
    putc(0, pfb);
    len = 0;
    type = 1;
    id = 501;
    for (i = 0; i < subcnt; ++i) {
        for (j = 0; j < subcnt; ++j)
            if (rsrcids[j] == id)
                break;
        if (j == subcnt) {
            LogError(_("Missing POST resource %u\n"), id);
            break;
        }
        id = id + 1;
        fseek(f, offsets[j], SEEK_SET);
        rlen = getlong(f);
        ch1 = getc(f);
        ch2 = getc(f);
        rlen -= 2;              /* those two bytes don't count as real data */
        if (ch1 == type)
            len += rlen;
        else {
            long hold = ftell(pfb);
            fseek(pfb, lenpos, SEEK_SET);
            putc(len >> 24, pfb);
            putc((len >> 16) & 0xff, pfb);
            putc((len >> 8) & 0xff, pfb);
            putc(len & 0xff, pfb);
            fseek(pfb, hold, SEEK_SET);
            if (ch1 == 5)       /* end of font mark */
                break;
            putc(0x80, pfb);
            putc(ch1, pfb);
            lenpos = ftell(pfb);
            putc(0, pfb);
            putc(0, pfb);
            putc(0, pfb);
            putc(0, pfb);
            type = ch1;
            len = rlen;
        }
        if (rlen > max) {
            free(buffer);
            max = rlen;
            if (max < 0x800)
                max = 0x800;
            buffer = galloc(max);
            if (buffer == NULL) {
                LogError(_("Out of memory\n"));
                exit(1);
            }
        }
        if(fread(buffer, 1, rlen, f) != ((size_t)rlen) ) {
	  LogError(_("Unable to read %u bytes for buffer\n"),rlen);
          exit(1);
         }
        fwrite(buffer, 1, rlen, pfb);
    }
    free(buffer);
    free(offsets);
    free(rsrcids);
    putc(0x80, pfb);
    putc(3, pfb);
    fseek(pfb, lenpos, SEEK_SET);
    putc(len >> 24, pfb);
    putc((len >> 16) & 0xff, pfb);
    putc((len >> 8) & 0xff, pfb);
    putc(len & 0xff, pfb);
    fseek(f, here, SEEK_SET);
    rewind(pfb);
    if (flags & ttf_onlynames)
        return ((SplineFont *) _NamesReadPostscript(pfb));      /* This closes the font for us */

    fd = _ReadPSFont(pfb);
    sf = NULL;
    if (fd != NULL) {
        sf = SplineFontFromPSFont(fd);
        PSFontFree(fd);
        /* There is no FOND in a postscript file, so we can't read any kerning */
    }
    fclose(pfb);
    return (sf);
}

static SplineFont *SearchTtfResources(FILE * f, long rlistpos, int subcnt,
                                      long rdata_pos, long name_list,
                                      char *filename, int flags,
                                      enum openflags openflags)
{
    long here, start = ftell(f);
    long roff;
    int rname = -1;
    int ch1, ch2;
    int len, i, rlen, ilen;
    /* The sfnt resource is just a copy of the ttf file */
    char *buffer = NULL;
    int max = 0;
    FILE *ttf;
    SplineFont *sf;
    int which = 0;
    char **names;
    char *pt, *lparen, *rparen;
    char *chosenname = NULL;
    (void) name_list;
    fseek(f, rlistpos, SEEK_SET);
    if (subcnt > 1 || (flags & ttf_onlynames)) {
        names = gcalloc(subcnt + 1, sizeof(char *));
        for (i = 0; i < subcnt; ++i) {
            /* resource id = */ getushort(f);
            /* rname = (short) */ getushort(f);
            /* flags = */ getc(f);
            ch1 = getc(f);
            ch2 = getc(f);
            roff = rdata_pos + ((ch1 << 16) | (ch2 << 8) | getc(f));
            /* mbz = */ getlong(f);
            here = ftell(f);
            names[i] = TTFGetFontName(f, roff + 4, roff + 4);
            if (names[i] == NULL) {
                char buffer[32];
                sprintf(buffer, "Nameless%d", i);
                names[i] = copy(buffer);
            }
            fseek(f, here, SEEK_SET);
        }
        if (flags & ttf_onlynames) {
            return ((SplineFont *) names);
        }
        if ((pt = strrchr(filename, '/')) == NULL)
            pt = filename;
        /* Someone gave me a font "Nafees Nastaleeq(Updated).ttf" and complained */
        /*  that ff wouldn't open it */
        /* Now someone will complain about "Nafees(Updated).ttc(fo(ob)ar)" */
        if ((lparen = strrchr(pt, '(')) != NULL &&
            (rparen = strrchr(lparen, ')')) != NULL && rparen[1] == '\0') {
            char *find = copy(lparen + 1);
            pt = strchr(find, ')');
            if (pt != NULL)
                *pt = '\0';
            for (which = subcnt - 1; which >= 0; --which)
                if (strcmp(names[which], find) == 0)
                    break;
            if (which == -1) {
                char *end;
                which = strtol(find, &end, 10);
                if (*end != '\0')
                    which = -1;
            }
            if (which == -1) {
                char *fn = copy(filename);
                fn[lparen - filename] = '\0';
                ff_post_error(_("Not in Collection"), _("%s is not in %.100s"),
                              find, fn);
                free(fn);
            }
            free(find);
        } else
            which = 0;
        if (lparen == NULL && which != -1)
            chosenname = copy(names[which]);
        for (i = 0; i < subcnt; ++i)
            free(names[i]);
        free(names);
        fseek(f, rlistpos, SEEK_SET);
    }

    for (i = 0; i < subcnt; ++i) {
        /* resource id = */ getushort(f);
        rname = (short) getushort(f);
        /* flags = */ getc(f);
        ch1 = getc(f);
        ch2 = getc(f);
        roff = rdata_pos + ((ch1 << 16) | (ch2 << 8) | getc(f));
        /* mbz = */ getlong(f);
        if (i != which)
            continue;
        here = ftell(f);

        ttf = tmpfile();
        if (ttf == NULL) {
            LogError(_("Can't open temporary file for truetype output.\n"));
            continue;
        }

        fseek(f, roff, SEEK_SET);
        ilen = rlen = getlong(f);
        if (rlen > 16 * 1024)
            ilen = 16 * 1024;
        if (ilen > max) {
            free(buffer);
            max = ilen;
            if (max < 0x800)
                max = 0x800;
            buffer = malloc(max);
        }
        for (len = 0; len < rlen;) {
            int temp = ilen;
            if (rlen - len < ilen)
                temp = rlen - len;
            temp = fread(buffer, 1, temp, f);
            if (temp == EOF)
                break;
            fwrite(buffer, 1, temp, ttf);
            len += temp;
        }
        rewind(ttf);
        sf = _SFReadTTF(ttf, flags, openflags, NULL, NULL);
        fclose(ttf);
        if (sf != NULL) {
            free(buffer);
            fseek(f, start, SEEK_SET);
            if (sf->chosenname == NULL)
                sf->chosenname = chosenname;
            return (sf);
        }
        fseek(f, here, SEEK_SET);
    }
    free(chosenname);
    free(buffer);
    fseek(f, start, SEEK_SET);
    return (NULL);
}

typedef struct fond {
    char *fondname;
    int first, last;
    int assoc_cnt;
    struct assoc {
        short size, style, id;
    } *assoc;
    /* size==0 => scalable */
    /* style>>8 is the bit depth (0=>1, 1=>2, 2=>4, 3=>8) */
    /* search order for ID is sfnt, NFNT, FONT */
    int stylewidthcnt;
    struct stylewidths {
        short style;
        short *widthtab;        /* 4.12 fixed number with the width specified as a fraction of an em */
    } *stylewidths;
    int stylekerncnt;
    struct stylekerns {
        short style;
        int kernpairs;
        struct kerns {
            unsigned char ch1, ch2;
            short offset;       /* 4.12 */
        } *kerns;
    } *stylekerns;
    char *psnames[48];
    struct fond *next;
} FOND;

struct MacFontRec {
    short fontType;
    short firstChar;
    short lastChar;
    short widthMax;
    short kernMax;              /* bb learing */
    short Descent;              /* maximum negative distance below baseline */
    short fRectWidth;           /* bounding box width */
    short fRectHeight;          /* bounding box height */
    unsigned short *offsetWidths;       /* offset to start of offset/width table */
    /* 0xffff => undefined, else high byte is offset in locTable, */
    /*  low byte is width */
    short ascent;
    short descent;
    short leading;
    short rowWords;             /* shorts per row */
    unsigned short *fontImage;  /* rowWords*fRectHeight */
    /* Images for all characters plus one extra for undefined */
    unsigned short *locs;       /* lastchar-firstchar+3 words */
    /* Horizontal offset to start of n'th character. Note: applies */
    /*  to each row. Missing characters have same loc as following */
};

static void FondListFree(FOND * list)
{
    FOND *next;
    int i;

    while (list != NULL) {
        next = list->next;
        free(list->assoc);
        for (i = 0; i < list->stylewidthcnt; ++i)
            free(list->stylewidths[i].widthtab);
        free(list->stylewidths);
        for (i = 0; i < list->stylekerncnt; ++i)
            free(list->stylekerns[i].kerns);
        free(list->stylekerns);
        for (i = 0; i < 48; ++i)
            free(list->psnames[i]);
        free(list);
        list = next;
    }
}

/* There's probably only one fond in the file, but there could be more so be */
/*  prepared... */
/* I want the fond: */
/*  to get the fractional widths for the SWIDTH entry on bdf */
/*  to get the font name */
/*  to get the font association tables */
/*  to get the style flags */
/* http://developer.apple.com/techpubs/mac/Text/Text-269.html */
static FOND *BuildFondList(FILE * f, long rlistpos, int subcnt, long rdata_pos,
                           long name_list, int flags)
{
    long here, start = ftell(f);
    long offset;
    int rname = -1;
    char name[300];
    int ch1, ch2;
    int i, j, k, cnt, isfixed;
    FOND *head = NULL, *cur;
    long widoff, kernoff, styleoff;

    fseek(f, rlistpos, SEEK_SET);
    for (i = 0; i < subcnt; ++i) {
        /* resource id = */ getushort(f);
        rname = (short) getushort(f);
        /* flags = */ getc(f);
        ch1 = getc(f);
        ch2 = getc(f);
        offset = rdata_pos + ((ch1 << 16) | (ch2 << 8) | getc(f));
        /* mbz = */ getlong(f);
        here = ftell(f);

        cur = gcalloc(1, sizeof(FOND));
        cur->next = head;
        head = cur;

        if (rname != -1) {
            fseek(f, name_list + rname, SEEK_SET);
            ch1 = getc(f);
            if( fread(name, 1, ch1, f) != ((size_t)ch1)) {
	      LogError(_("Unable to read %u bytes for name, but going on.\n"),ch1);
            }
            name[ch1] = '\0';
            cur->fondname = copy(name);
        }

        offset += 4;
        fseek(f, offset, SEEK_SET);
        isfixed = getushort(f) & 0x8000 ? 1 : 0;
        /* family id = */ getushort(f);
        cur->first = getushort(f);
        cur->last = getushort(f);
/* on a 1 point font... */
        /* ascent = */ getushort(f);
        /* descent = (short) */ getushort(f);
        /* leading = */ getushort(f);
        /* widmax = */ getushort(f);
        if ((widoff = getlong(f)) != 0)
            widoff += offset;
        if ((kernoff = getlong(f)) != 0)
            kernoff += offset;
        if ((styleoff = getlong(f)) != 0)
            styleoff += offset;
        for (j = 0; j < 9; ++j)
            getushort(f);
        /* internal & undefined, for international scripts = */ getlong(f);
        /* version = */ getushort(f);
        cur->assoc_cnt = getushort(f) + 1;
        cur->assoc = gcalloc(cur->assoc_cnt, sizeof(struct assoc));
        for (j = 0; j < cur->assoc_cnt; ++j) {
            cur->assoc[j].size = getushort(f);
            cur->assoc[j].style = getushort(f);
            cur->assoc[j].id = getushort(f);
        }
        if (widoff != 0) {
            fseek(f, widoff, SEEK_SET);
            cnt = getushort(f) + 1;
            cur->stylewidthcnt = cnt;
            cur->stylewidths = gcalloc(cnt, sizeof(struct stylewidths));
            for (j = 0; j < cnt; ++j) {
                cur->stylewidths[j].style = getushort(f);
                cur->stylewidths[j].widthtab =
                    galloc((cur->last - cur->first + 3) * sizeof(short));
                for (k = cur->first; k <= cur->last + 2; ++k)
                    cur->stylewidths[j].widthtab[k] = getushort(f);
            }
        }
        if (kernoff != 0 && (flags & ttf_onlykerns)) {
            fseek(f, kernoff, SEEK_SET);
            cnt = getushort(f) + 1;
            cur->stylekerncnt = cnt;
            cur->stylekerns = gcalloc(cnt, sizeof(struct stylekerns));
            for (j = 0; j < cnt; ++j) {
                cur->stylekerns[j].style = getushort(f);
                cur->stylekerns[j].kernpairs = getushort(f);
                cur->stylekerns[j].kerns =
                    galloc(cur->stylekerns[j].kernpairs * sizeof(struct kerns));
                for (k = 0; k < cur->stylekerns[j].kernpairs; ++k) {
                    cur->stylekerns[j].kerns[k].ch1 = getc(f);
                    cur->stylekerns[j].kerns[k].ch2 = getc(f);
                    cur->stylekerns[j].kerns[k].offset = getushort(f);
                }
            }
        }
        if (styleoff != 0) {
            uint8 stringoffsets[48];
            int strcnt, stringlen, format;
            char **strings, *pt;
            fseek(f, styleoff, SEEK_SET);
            /* class = */ getushort(f);
            /* glyph encoding offset = */ getlong(f);
            /* reserved = */ getlong(f);
            for (j = 0; j < 48; ++j)
                stringoffsets[j] = getc(f);
            strcnt = getushort(f);
            strings = galloc(strcnt * sizeof(char *));
            for (j = 0; j < strcnt; ++j) {
                stringlen = getc(f);
                strings[j] = galloc(stringlen + 2);
                strings[j][0] = stringlen;
                strings[j][stringlen + 1] = '\0';
                for (k = 0; k < stringlen; ++k)
                    strings[j][k + 1] = getc(f);
            }
            for (j = 0; j < 48; ++j) {
                for (k = j - 1; k >= 0; --k)
                    if (stringoffsets[j] == stringoffsets[k])
                        break;
                if (k != -1)
                    continue;   /* this style doesn't exist */
                format = stringoffsets[j] - 1;
                stringlen = strings[0][0];
                if (format != 0)
                    for (k = 0; k < strings[format][0]; ++k)
                        stringlen += strings[strings[format][k + 1] - 1][0];
                pt = cur->psnames[j] = galloc(stringlen + 1);
                strcpy(pt, strings[0] + 1);
                pt += strings[0][0];
                if (format != 0)
                    for (k = 0; k < strings[format][0]; ++k) {
                        strcpy(pt, strings[strings[format][k + 1] - 1] + 1);
                        pt += strings[strings[format][k + 1] - 1][0];
                    }
                *pt = '\0';
            }
            for (j = 0; j < strcnt; ++j)
                free(strings[j]);
            free(strings);
        }
        fseek(f, here, SEEK_SET);
    }
    fseek(f, start, SEEK_SET);
    return (head);
}

static char *BuildName(char *family, int style)
{
    char buffer[350] = "";

    strncpy(buffer, family, 200);
    if (style != 0)
        strcat(buffer, "-");
    if (style & sf_bold)
        strcat(buffer, "Bold");
    if (style & sf_italic)
        strcat(buffer, "Italic");
    if (style & sf_underline)
        strcat(buffer, "Underline");
    if (style & sf_outline)
        strcat(buffer, "Outline");
    if (style & sf_shadow)
        strcat(buffer, "Shadow");
    if (style & sf_condense)
        strcat(buffer, "Condensed");
    if (style & sf_extend)
        strcat(buffer, "Extended");
    return (copy(buffer));
}

static int GuessStyle(char *fontname, int *styles, int style_cnt)
{
    int which, style;
    char *stylenames = _GetModifiers(fontname, NULL, NULL);

    style = _MacStyleCode(stylenames, NULL, NULL);
    for (which = style_cnt; which >= 0; --which)
        if (styles[which] == style)
            return (which);

    return (-1);
}

static FOND *PickFOND(FOND * fondlist, char *filename, char **name, int *style)
{
    int i, j;
    FOND *test;
    uint8 stylesused[96];
    char **names;
    FOND **fonds = NULL, *fond = NULL;
    int *styles = NULL;
    int cnt, which;
    char *pt, *lparen;
    char *find = NULL;

    if ((pt = strrchr(filename, '/')) != NULL)
        pt = filename;
    if ((lparen = strchr(filename, '(')) != NULL && strchr(lparen, ')') != NULL) {
        find = copy(lparen + 1);
        pt = strchr(find, ')');
        if (pt != NULL)
            *pt = '\0';
        for (test = fondlist; test != NULL; test = test->next) {
            for (i = 0; i < 48; ++i)
                if (test->psnames[i] != NULL
                    && strcmp(find, test->psnames[i]) == 0) {
                    *style = (i & 3) | ((i & ~3) << 1); /* PS styles skip underline bit */
                    *name = copy(test->psnames[i]);
                    return (test);
                }
        }
    }

    /* The file may contain multiple families, and each family may contain */
    /*  multiple styles (and each style may contain multiple sizes, but that's */
    /*  not an issue for us here) */
    names = NULL;
    for (i = 0; i < 2; ++i) {
        cnt = 0;
        for (test = fondlist; test != NULL; test = test->next)
            if (test->fondname != NULL) {
                memset(stylesused, 0, sizeof(stylesused));
                for (j = 0; j < test->assoc_cnt; ++j) {
                    if (test->assoc[j].size != 0
                        && !stylesused[test->assoc[j].style]) {
                        stylesused[test->assoc[j].style] = true;
                        if (names != NULL) {
                            names[cnt] =
                                BuildName(test->fondname, test->assoc[j].style);
                            styles[cnt] = test->assoc[j].style;
                            fonds[cnt] = test;
                        }
                        ++cnt;
                    }
                }
            }
        if (names == NULL) {
            names = gcalloc(cnt + 1, sizeof(char *));
            fonds = galloc(cnt * sizeof(FOND *));
            styles = galloc(cnt * sizeof(int));
        }
    }

    if (find != NULL) {
        for (which = cnt - 1; which >= 0; --which)
            if (strcmp(names[which], find) == 0)
                break;
        if (which == -1 && strstrmatch(find, test->fondname) != NULL)
            which = GuessStyle(find, styles, cnt);
        if (which == -1) {
            char *fn = copy(filename);
            fn[lparen - filename] = '\0';
            ff_post_error(_("Not in Collection"), _("%s is not in %.100s"),
                          find, fn);
            free(fn);
        }
        free(find);
    } else
        which = 0;

    if (which != -1) {
        fond = fonds[which];
        *name = copy(names[which]);
        *style = styles[which];
    }
    for (i = 0; i < cnt; ++i)
        free(names[i]);
    free(names);
    free(fonds);
    free(styles);
    if (which == -1)
        return (NULL);

    return (fond);
}


/* Look for kerning info and merge it into the currently existing font "into" */
static SplineFont *FindFamilyStyleKerns(SplineFont * into, EncMap * map,
                                        FOND * fondlist, char *filename)
{
    char *name;
    int style;
    FOND *fond;
    int i, j;
    int ch1, ch2, offset;
    KernPair *kp;
    SplineChar *sc1, *sc2;

    fond = PickFOND(fondlist, filename, &name, &style);
    if (fond == NULL || into == NULL)
        return (NULL);
    for (i = 0; i < fond->stylekerncnt; ++i)
        if (fond->stylekerns[i].style == style)
            break;
    if (i == fond->stylekerncnt) {
        LogError(_("No kerning table for %s\n"), name);
        free(name);
        return (NULL);
    }
    for (j = 0; j < fond->stylekerns[i].kernpairs; ++j) {
        ch1 = fond->stylekerns[i].kerns[j].ch1;
        ch2 = fond->stylekerns[i].kerns[j].ch2;
        offset =
            (fond->stylekerns[i].kerns[j].offset *
             (into->ascent + into->descent) + (1 << 11)) >> 12;
        sc1 = SFMakeChar(into, map, ch1);
        sc2 = SFMakeChar(into, map, ch2);
        for (kp = sc1->kerns; kp != NULL; kp = kp->next)
            if (kp->sc == sc2)
                break;
        if (kp == NULL) {
            uint32 script;
            kp = chunkalloc(sizeof(KernPair));
            kp->sc = sc2;
            kp->next = sc1->kerns;
            sc1->kerns = kp;
            script = SCScriptFromUnicode(sc1);
            if (script == DEFAULT_SCRIPT)
                script = SCScriptFromUnicode(sc2);
            kp->subtable =
                SFSubTableFindOrMake(sc1->parent, CHR('k', 'e', 'r', 'n'),
                                     script, gpos_pair);
        }
        kp->off = offset;
    }
    return (into);
}

/* Look for a bare truetype font in a binhex/macbinary wrapper */
static SplineFont *MightBeTrueType(FILE * binary, int32 pos, int32 dlen,
                                   int flags, enum openflags openflags)
{
    FILE *temp ;
    char *buffer ;
    int len;
    SplineFont *sf;

    if (flags & ttf_onlynames) {
        char **ret;
        char *temp = TTFGetFontName(binary, pos, pos);
        if (temp == NULL)
            return (NULL);
        ret = galloc(2 * sizeof(char *));
        ret[0] = temp;
        ret[1] = NULL;
        return ((SplineFont *) ret);
    }
    temp = tmpfile();
    buffer = galloc(8192);


    fseek(binary, pos, SEEK_SET);
    while (dlen > 0) {
        len = dlen > 8192 ? 8192 : dlen;
        len = fread(buffer, 1, dlen > 8192 ? 8192 : dlen, binary);
        if (len == 0)
            break;
        fwrite(buffer, 1, len, temp);
        dlen -= len;
    }
    rewind(temp);
    sf = _SFReadTTF(temp, flags, openflags, NULL, NULL);
    fclose(temp);
    free(buffer);
    return (sf);
}

static SplineFont *IsResourceFork(FILE * f, long offset, char *filename,
                                  int flags, enum openflags openflags,
                                  SplineFont * into, EncMap * map)
{
    /* If it is a good resource fork then the first 16 bytes are repeated */
    /*  at the location specified in bytes 4-7 */
    /* We include an offset because if we are looking at a mac binary file */
    /*  the resource fork will actually start somewhere in the middle of the */
    /*  file, not at the beginning */
    unsigned char buffer[16], buffer2[16];
    long rdata_pos, map_pos, type_list, name_list, rpos;
    int32 rdata_len, map_len;
    uint32 nfnt_pos, font_pos, fond_pos;
    unsigned long tag;
    int i, cnt, subcnt, nfnt_subcnt = 0, font_subcnt = 0, fond_subcnt = 0;
    SplineFont *sf;
    FOND *fondlist = NULL;
    fond_pos = 0;
    fseek(f, offset, SEEK_SET);
    if (fread(buffer, 1, 16, f) != 16)
        return (NULL);
    rdata_pos =
        offset +
        ((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]);
    map_pos =
        offset +
        ((buffer[4] << 24) | (buffer[5] << 16) | (buffer[6] << 8) | buffer[7]);
    rdata_len =
        ((buffer[8] << 24) | (buffer[9] << 16) | (buffer[10] << 8) |
         buffer[11]);
    map_len =
        ((buffer[12] << 24) | (buffer[13] << 16) | (buffer[14] << 8) |
         buffer[15]);
    if (rdata_pos + rdata_len != map_pos || rdata_len == 0)
        return (NULL);
    fseek(f, map_pos, SEEK_SET);
    buffer2[15] = buffer[15] + 1;       /* make it be different */
    if (fread(buffer2, 1, 16, f) != 16)
        return (NULL);

/* Apple's data fork resources appear to have a bunch of zeroes here instead */
/*  of a copy of the first 16 bytes */
    for (i = 0; i < 16; ++i)
        if (buffer2[i] != 0)
            break;
    if (i != 16) {
        for (i = 0; i < 16; ++i)
            if (buffer[i] != buffer2[i])
                return (NULL);
    }
    getlong(f);                 /* skip the handle to the next resource map */
    getushort(f);               /* skip the file resource number */
    getushort(f);               /* skip the attributes */
    type_list = map_pos + getushort(f);
    name_list = map_pos + getushort(f);

    fseek(f, type_list, SEEK_SET);
    cnt = getushort(f) + 1;
    for (i = 0; i < cnt; ++i) {
        tag = getlong(f);
        /* printf( "%c%c%c%c\n", tag>>24, (tag>>16)&0xff, (tag>>8)&0xff, tag&0xff ); */
        subcnt = getushort(f) + 1;
        rpos = type_list + getushort(f);
        sf = NULL;
        if (tag == CHR('P', 'O', 'S', 'T') && !(flags & (ttf_onlystrikes | ttf_onlykerns)))     /* No FOND */
            sf = SearchPostscriptResources(f, rpos, subcnt, rdata_pos,
                                           name_list, flags);
        else if (tag == CHR('s', 'f', 'n', 't') && !(flags & ttf_onlykerns))
            sf = SearchTtfResources(f, rpos, subcnt, rdata_pos, name_list,
                                    filename, flags, openflags);
        else if (tag == CHR('N', 'F', 'N', 'T')) {
            nfnt_pos = rpos;
            nfnt_subcnt = subcnt;
        } else if (tag == CHR('F', 'O', 'N', 'T')) {
            font_pos = rpos;
            font_subcnt = subcnt;
        } else if (tag == CHR('F', 'O', 'N', 'D')) {
            fond_pos = rpos;
            fond_subcnt = subcnt;
        }
        if (sf != NULL)
            return (sf);
    }
    if (flags & ttf_onlynames)  /* Not interested in bitmap resources here */
        return (NULL);

    if (flags & ttf_onlykerns) {        /* For kerns */
        if (fond_subcnt != 0)
            fondlist =
                BuildFondList(f, fond_pos, fond_subcnt, rdata_pos, name_list,
                              flags);
        into = FindFamilyStyleKerns(into, map, fondlist, filename);
        FondListFree(fondlist);
        return (into);
    }
    /* Ok. If no outline font, try for a bitmap */
    if (nfnt_subcnt == 0) {
        nfnt_pos = font_pos;
        nfnt_subcnt = font_subcnt;
    }
    return ((SplineFont *) - 1);        /* It's a valid resource file, but just has no fonts */
}


static SplineFont *IsResourceInBinary(FILE * f, char *filename, int flags,
                                      enum openflags openflags,
                                      SplineFont * into, EncMap * map)
{
    unsigned char header[128];
    unsigned long offset, dlen, rlen;

    if (fread(header, 1, 128, f) != 128)
        return (NULL);
    if (header[0] != 0 || header[74] != 0 || header[82] != 0 || header[1] <= 0
        || header[1] > 33 || header[63] != 0 || header[2 + header[1]] != 0)
        return (NULL);
    dlen =
        ((header[0x53] << 24) | (header[0x54] << 16) | (header[0x55] << 8) |
         header[0x56]);
    rlen =
        ((header[0x57] << 24) | (header[0x58] << 16) | (header[0x59] << 8) |
         header[0x5a]);
    /* 128 bytes for header, then the dlen is padded to a 128 byte boundary */
    offset = 128 + ((dlen + 127) & ~127);
/* Look for a bare truetype font in a binhex/macbinary wrapper */
    if (dlen != 0 && rlen <= dlen) {
        int pos = ftell(f);
        if (fread(header, 1, 4, f) != ((size_t)4)) {
           LogError(_("Unable to read 4 bytes for header, but going on.\n"));
        }
        header[5] = '\0';
        if (strcmp((char *) header, "OTTO") == 0
            || strcmp((char *) header, "true") == 0
            || strcmp((char *) header, "ttcf") == 0 || (header[0] == 0
                                                        && header[1] == 1
                                                        && header[2] == 0
                                                        && header[3] == 0))
            return (MightBeTrueType(f, pos, dlen, flags, openflags));
    }
    return (IsResourceFork(f, offset, filename, flags, openflags, into, map));
}

static int lastch = 0, repeat = 0;
static void outchr(FILE * binary, int ch)
{
    int i;

    if (repeat) {
        if (ch == 0) {
            /* no repeat, output a literal 0x90 (the repeat flag) */
            lastch = 0x90;
            putc(lastch, binary);
        } else {
            for (i = 1; i < ch; ++i)
                putc(lastch, binary);
        }
        repeat = 0;
    } else if (ch == 0x90) {
        repeat = 1;
    } else {
        putc(ch, binary);
        lastch = ch;
    }
}

static SplineFont *IsResourceInHex(FILE * f, char *filename, int flags,
                                   enum openflags openflags, SplineFont * into,
                                   EncMap * map)
{
    /* convert file from 6bit to 8bit */
    /* interesting data is enclosed between two colons */
    FILE *binary = tmpfile();
    char *sixbit =
        "!\"#$%&'()*+,-012345689@ABCDEFGHIJKLMNPQRSTUVXYZ[`abcdefhijklmpqr";
    int ch, val, cnt, i, dlen, rlen;
    unsigned char header[20];
    char *pt;
    SplineFont *ret;

    if (binary == NULL) {
        LogError(_("can't create temporary file\n"));
        return (NULL);
    }

    lastch = repeat = 0;
    while ((ch = getc(f)) != ':');      /* There may be comments before file start */
    cnt = val = 0;
    while ((ch = getc(f)) != ':') {
        if (isspace(ch))
            continue;
        for (pt = sixbit; *pt != ch && *pt != '\0'; ++pt);
        if (*pt == '\0') {
            fclose(binary);
            return (NULL);
        }
        val = (val << 6) | (pt - sixbit);
        if (++cnt == 4) {
            outchr(binary, (val >> 16) & 0xff);
            outchr(binary, (val >> 8) & 0xff);
            outchr(binary, val & 0xff);
            val = cnt = 0;
        }
    }
    if (cnt != 0) {
        if (cnt == 1)
            outchr(binary, val << 2);
        else if (cnt == 2) {
            val <<= 4;
            outchr(binary, (val >> 8) & 0xff);
            outchr(binary, val & 0xff);
        } else if (cnt == 3) {
            val <<= 6;
            outchr(binary, (val >> 16) & 0xff);
            outchr(binary, (val >> 8) & 0xff);
            outchr(binary, val & 0xff);
        }
    }

    rewind(binary);
    ch = getc(binary);          /* Name length */
    /* skip name */
    for (i = 0; i < ch; ++i)
        getc(binary);
    if (getc(binary) != '\0') {
        fclose(binary);
        return (NULL);
    }
    if ( fread(header, 1, 20, binary) != 20 ) {
        LogError(_("Can't read 20 bytes for header\n"));
        return (NULL);
    }
    dlen =
        (header[10] << 24) | (header[11] << 16) | (header[12] << 8) |
        header[13];
    rlen =
        (header[14] << 24) | (header[15] << 16) | (header[16] << 8) |
        header[17];
/* Look for a bare truetype font in a binhex/macbinary wrapper */
    if (dlen != 0 && rlen < dlen) {
        int pos = ftell(binary);
        if (fread(header, 1, 4, binary) != 4) {
	  LogError(_("Can't read 4 bytes for header\n"));
	  return (NULL);
	}
        header[5] = '\0';
        if (strcmp((char *) header, "OTTO") == 0
            || strcmp((char *) header, "true") == 0
            || strcmp((char *) header, "ttcf") == 0 || (header[0] == 0
                                                        && header[1] == 1
                                                        && header[2] == 0
                                                        && header[3] == 0)) {
            ret = MightBeTrueType(binary, pos, dlen, flags, openflags);
            fclose(binary);
            return (ret);
        }
    }
    if (rlen == 0) {
        fclose(binary);
        return (NULL);
    }

    ret =
        IsResourceFork(binary, ftell(binary) + dlen + 2, filename, flags,
                       openflags, into, map);

    fclose(binary);
    return (ret);
}

static SplineFont *IsResourceInFile(char *filename, int flags,
                                    enum openflags openflags, SplineFont * into,
                                    EncMap * map)
{
    FILE *f;
    char *spt, *pt;
    SplineFont *sf;
    char *temp = filename, *lparen;

    if ((pt = strrchr(filename, '/')) == NULL)
        pt = filename;
    if ((lparen = strchr(pt, '(')) != NULL && strchr(lparen, ')') != NULL) {
        temp = copy(filename);
        temp[lparen - filename] = '\0';
    }
    f = fopen(temp, "rb");
    if (temp != filename)
        free(temp);
    if (f == NULL)
        return (NULL);
    spt = strrchr(filename, '/');
    if (spt == NULL)
        spt = filename;
    pt = strrchr(spt, '.');
    if (pt != NULL && (pt[1] == 'b' || pt[1] == 'B')
        && (pt[2] == 'i' || pt[2] == 'I') && (pt[3] == 'n' || pt[3] == 'N')
        && (pt[4] == '\0' || pt[4] == '(')) {
        if ((sf = IsResourceInBinary(f, filename, flags, openflags, into, map))) {
            fclose(f);
            return (sf);
        }
    } else if (pt != NULL && (pt[1] == 'h' || pt[1] == 'H')
               && (pt[2] == 'q' || pt[2] == 'Q') && (pt[3] == 'x'
                                                     || pt[3] == 'X')
               && (pt[4] == '\0' || pt[4] == '(')) {
        if ((sf = IsResourceInHex(f, filename, flags, openflags, into, map))) {
            fclose(f);
            return (sf);
        }
    }

    sf = IsResourceFork(f, 0, filename, flags, openflags, into, map);
    fclose(f);
#if __Mac
    if (sf == NULL)
        sf = HasResourceFork(filename, flags, openflags, into, map);
#endif
    return (sf);
}

static SplineFont *FindResourceFile(char *filename, int flags,
                                    enum openflags openflags, SplineFont * into,
                                    EncMap * map)
{
    char *spt, *pt, *dpt;
    char buffer[1400];
    SplineFont *sf;

    if ((sf = IsResourceInFile(filename, flags, openflags, into, map)))
        return (sf);

    /* Well, look in the resource fork directory (if it exists), the resource */
    /*  fork is placed there in a seperate file on (some) non-Mac disks */
    strcpy(buffer, filename);
    spt = strrchr(buffer, '/');
    if (spt == NULL) {
        spt = buffer;
        pt = filename;
    } else {
        ++spt;
        pt = filename + (spt - buffer);
    }
    strcpy(spt, "resource.frk/");
    strcat(spt, pt);
    if ((sf = IsResourceInFile(buffer, flags, openflags, into, map)))
        return (sf);

    /* however the resource fork does not appear to do long names properly */
    /*  names are always lower case 8.3, do some simple things to check */
    spt = strrchr(buffer, '/') + 1;
    for (pt = spt; *pt; ++pt)
        if (isupper(*pt))
            *pt = tolower(*pt);
    dpt = strchr(spt, '.');
    if (dpt == NULL)
        dpt = spt + strlen(spt);
    if (dpt - spt > 8 || strlen(dpt) > 4) {
        char exten[8];
        strncpy(exten, dpt, 7);
        exten[4] = '\0';        /* it includes the dot */
        if (dpt - spt > 6)
            dpt = spt + 6;
        *dpt++ = '~';
        *dpt++ = '1';
        strcpy(dpt, exten);
    }
    return (IsResourceInFile(buffer, flags, openflags, into, map));
}


static char *createtmpfile(char *filename)
{
    char *p, *tempname;
    p = strrchr(filename,'/');
    if (p != NULL) {
	filename = p+1;
    }
    assert(strlen(filename)>=5);
    tempname = malloc(strlen(filename)+2);
    if (tempname == NULL) {
	LogError(_("Out of memory\n"));
	exit(1);
    }
    strcpy(tempname,filename);
    strcpy(tempname+strlen(tempname)-5,"XXXXXX"); /* dfont -> XXXXXX */

#ifdef HAVE_MKSTEMP
    {
      int i = mkstemp(tempname);
      if (i) {
        close(i);
      }
    }
#else
    mktemp(tempname);
#endif
    return tempname;
}

static char *SearchTtfResourcesFile(FILE * f, long rlistpos, int subcnt,
                                    long rdata_pos, long name_list,
                                    char *filename, char *fontname)
{
    long here;
    long roff;
    int rname = -1;
    int ch1, ch2;
    int len, i, rlen, ilen;
    /* The sfnt resource is just a copy of the ttf file */
    char *buffer = NULL;
    int max = 0;
    FILE *ttf;
    char *sf = NULL;
    int which = 0;
    char **names;
    (void)name_list;
    fseek(f, rlistpos, SEEK_SET);
    if (subcnt > 1) {
        names = gcalloc(subcnt + 1, sizeof(char *));
        for (i = 0; i < subcnt; ++i) {
            /* resource id = */ getushort(f);
            /* rname = (short) */ getushort(f);
            /* flags = */ getc(f);
            ch1 = getc(f);
            ch2 = getc(f);
            roff = rdata_pos + ((ch1 << 16) | (ch2 << 8) | getc(f));
            /* mbz = */ getlong(f);
            here = ftell(f);
            names[i] = TTFGetPSFontName(f, roff + 4, roff + 4);
            if (names[i] == NULL) {
                char buffer[32];
                sprintf(buffer, "Nameless%d", i);
                names[i] = copy(buffer);
            }
            fseek(f, here, SEEK_SET);
        }
        if (1) {
            char *find = fontname;
            for (which = subcnt - 1; which >= 0; --which)
                if (strcmp(names[which], find) == 0)
                    break;
            if (which == -1) {
                char *end;
                which = strtol(find, &end, 10);
                if (*end != '\0')
                    which = -1;
            }
            if (which == -1) {
                ff_post_error(_("Not in Collection"), _("%s is not in %.100s"),
                              find, filename);
            }
        } else {
            which = 0;
        }
        for (i = 0; i < subcnt; ++i)
            free(names[i]);
        free(names);
        fseek(f, rlistpos, SEEK_SET);
    }

    for (i = 0; i < subcnt; ++i) {
        /* resource id = */ getushort(f);
        rname = (short) getushort(f);
        /* flags = */ getc(f);
        ch1 = getc(f);
        ch2 = getc(f);
        roff = rdata_pos + ((ch1 << 16) | (ch2 << 8) | getc(f));
        /* mbz = */ getlong(f);
        if (i != which)
            continue;
        here = ftell(f);

        sf = createtmpfile(filename);
        ttf = fopen(sf, "wb");
        if (ttf == NULL) {
            LogError(_("Can't open temporary file for truetype output.\n"));
            continue;
        }

        fseek(f, roff, SEEK_SET);
        ilen = rlen = getlong(f);
        if (rlen > 16 * 1024)
            ilen = 16 * 1024;
        if (ilen > max) {
            free(buffer);
            max = ilen;
            if (max < 0x800)
                max = 0x800;
            buffer = malloc(max);
        }
        for (len = 0; len < rlen;) {
            int temp = ilen;
            if (rlen - len < ilen)
                temp = rlen - len;
            temp = fread(buffer, 1, temp, f);
            if (temp == EOF)
                break;
            fwrite(buffer, 1, temp, ttf);
            len += temp;
        }
        fclose(ttf);
    }
    free(buffer);
    return sf;
}

static char *IsResourceForkFile(FILE * f, char *filename, char *fontname)
{
    /* If it is a good resource fork then the first 16 bytes are repeated */
    /*  at the location specified in bytes 4-7 */
    /* We include an offset because if we are looking at a mac binary file */
    /*  the resource fork will actually start somewhere in the middle of the */
    /*  file, not at the beginning */
    unsigned char buffer[16], buffer2[16];
    long rdata_pos, map_pos, type_list, name_list, rpos;
    int32 rdata_len, map_len;
    uint32 fond_pos;
    unsigned long tag;
    int i, cnt, subcnt;
    char *sf = NULL;
    fond_pos = 0;
    fseek(f, 0, SEEK_SET);
    if (fread(buffer, 1, 16, f) != 16)
        return (NULL);
    rdata_pos =
        ((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]);
    map_pos =
        ((buffer[4] << 24) | (buffer[5] << 16) | (buffer[6] << 8) | buffer[7]);
    rdata_len =
        ((buffer[8] << 24) | (buffer[9] << 16) | (buffer[10] << 8) |
         buffer[11]);
    map_len =
        ((buffer[12] << 24) | (buffer[13] << 16) | (buffer[14] << 8) |
         buffer[15]);
    if (rdata_pos + rdata_len != map_pos || rdata_len == 0)
        return (NULL);
    fseek(f, map_pos, SEEK_SET);
    buffer2[15] = buffer[15] + 1;       /* make it be different */
    if (fread(buffer2, 1, 16, f) != 16)
        return (NULL);
    for (i = 0; i < 16; ++i)
        if (buffer2[i] != 0)
            break;
    if (i != 16) {
        for (i = 0; i < 16; ++i)
            if (buffer[i] != buffer2[i])
                return (NULL);
    }
    getlong(f);                 /* skip the handle to the next resource map */
    getushort(f);               /* skip the file resource number */
    getushort(f);               /* skip the attributes */
    type_list = map_pos + getushort(f);
    name_list = map_pos + getushort(f);
    fseek(f, type_list, SEEK_SET);
    cnt = getushort(f) + 1;
    for (i = 0; i < cnt; ++i) {
        tag = getlong(f);
        subcnt = getushort(f) + 1;
        rpos = type_list + getushort(f);
        sf = NULL;
        if (tag == CHR('s', 'f', 'n', 't')) {
            sf = SearchTtfResourcesFile(f, rpos, subcnt, rdata_pos, name_list,
                                        filename, fontname);
        }
        if (sf != NULL)
            return (sf);
    }
    return NULL;
}


/* filename "/opt/tex/texmf-fonts/fonts/data/LucidaGrande.dfont", 
   fontname "Lucida Grande Bold"
 */
char *FindResourceTtfFont(char *filename, char *fontname)
{
    char *sf = NULL;
    FILE *f = fopen(filename, "rb");
    if (f == NULL)
        return (NULL);
    sf = IsResourceForkFile(f, filename, fontname);
    fclose(f);
    return sf;
}

SplineFont *SFReadMacBinary(char *filename, int flags, enum openflags openflags)
{
    SplineFont *sf = FindResourceFile(filename, flags, openflags, NULL, NULL);

    if (sf == NULL)
        LogError(_("Couldn't find a font file named %s\n"), filename);
    else if (sf == (SplineFont *) (-1)) {
        LogError(_
                 ("%s is a mac resource file but contains no postscript or truetype fonts\n"),
                 filename);
        sf = NULL;
    }
    return (sf);
}

char **NamesReadMacBinary(char *filename)
{
    return ((char **) FindResourceFile(filename, ttf_onlynames, 0, NULL, NULL));
}

/* should try to optimize this */
SplineFont *SFReadMacBinaryInfo(char *filename, int flags,
                                enum openflags openflags)
{
    SplineFont *sf = FindResourceFile(filename, flags, openflags, NULL, NULL);

    if (sf == NULL)
        LogError(_("Couldn't find a font file named %s\n"), filename);
    else if (sf == (SplineFont *) (-1)) {
        LogError(_
                 ("%s is a mac resource file but contains no postscript or truetype fonts\n"),
                 filename);
        sf = NULL;
    }
    return (sf);
}
