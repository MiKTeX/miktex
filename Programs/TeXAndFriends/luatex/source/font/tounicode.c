/*

Copyright 2006 Han The Thanh, <thanh@pdftex.org>
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

#define isXdigit(c) (isdigit(c) || ('A' <= (c) && (c) <= 'F'))

/*tex

    |UNIC_STRING| is a string allocated by |def_tounicode| while
    |UNI_EXTRA_STRING| is one allocated by |set_glyph_unicode|.

*/

#define UNI_UNDEF        -1
#define UNI_STRING       -2
#define UNI_EXTRA_STRING -3

static struct avl_table *glyph_unicode_tree = NULL;

static int comp_glyph_unicode_entry(const void *pa, const void *pb, void *p)
{
    (void) p;
    return strcmp(((const glyph_unicode_entry *) pa)->name, ((const glyph_unicode_entry *) pb)->name);
}

static glyph_unicode_entry *new_glyph_unicode_entry(void)
{
    glyph_unicode_entry *e;
    e = xtalloc(1, glyph_unicode_entry);
    e->name = NULL;
    e->code = UNI_UNDEF;
    e->unicode_seq = NULL;
    return e;
}

static void destroy_glyph_unicode_entry(void *pa, void *pb)
{
    glyph_unicode_entry *e = (glyph_unicode_entry *) pa;
    (void) pb;
    xfree(e->name);
    if (e->code == UNI_STRING) {
        xfree(e->unicode_seq);
    }
}

void glyph_unicode_free(void)
{
    if (glyph_unicode_tree != NULL)
        avl_destroy(glyph_unicode_tree, destroy_glyph_unicode_entry);
}

void def_tounicode(str_number glyph, str_number unistr)
{
    char buf[SMALL_BUF_SIZE], *p, *ph;
    char buf2[SMALL_BUF_SIZE], *q;
    /*tex |0| is invalid, |1| an \UNICODE\ value and |2| a string */
    int valid_unistr;
    int i, l;
    glyph_unicode_entry *gu, t;
    void **aa;
    p = makecstring(glyph);
    assert(strlen(p) < SMALL_BUF_SIZE);
    strcpy(buf, p);
    free(p);
    p = makecstring(unistr);
    ph = p;
    /*tex Ignore leading spaces. */
    while (*p == ' ')
        p++;
    l = (int) strlen(p);
    /*tex Ignore traling spaces. */
    while (l > 0 && p[l - 1] == ' ')
        l--;
    /*tex A \UNICODE\ value is the most common case. */
    valid_unistr = 1;
    for (i = 0; i < l; i++) {
        /*tex If a space occurs we treat this entry as a string. */
        if (p[i] == ' ')
            valid_unistr = 2;
        else if (!isXdigit((unsigned char)p[i])) {
            valid_unistr = 0;
            break;
        }
    }
    if (l == 0 || valid_unistr == 0 || strlen(buf) == 0 || strcmp(buf, notdef) == 0) {
        formatted_warning("tounicode", "invalid parameter(s): %s -> %s", buf, p);
        return;
    }
    if (glyph_unicode_tree == NULL) {
        glyph_unicode_tree = avl_create(comp_glyph_unicode_entry, NULL, &avl_xallocator);
    }
    t.name = buf;
    /*tex Allow overriding existing entries. */
    if ((gu = (glyph_unicode_entry *) avl_find(glyph_unicode_tree, &t)) != NULL) {
        if (gu->code == UNI_STRING) {
            xfree(gu->unicode_seq);
        }
    } else {
        /*tex Make a new entry. */
        gu = new_glyph_unicode_entry();
        gu->name = xstrdup(buf);
    }
    if (valid_unistr == 2) {
        /*tex A string can have space(s) that we ignore by copying |p| to |buf2|. */
        for (q = buf2; *p != 0; p++)
            if (*p != ' ')
                *q++ = *p;
        *q = 0;
        gu->code = UNI_STRING;
        gu->unicode_seq = xstrdup(buf2);
    } else {
        i = sscanf(p, "%lX", &(gu->code));
    }
    aa = avl_probe(glyph_unicode_tree, gu);
    assert(aa != NULL);
    free(ph);
}

static long check_unicode_value(char *s, boolean multiple_value)
{
    int l = (int) strlen(s);
    int i;
    /*tex Anything that is not |UNI_UNDEF| will do: */
    long code = 0;
    if (l == 0)
        return UNI_UNDEF;
    if (multiple_value && l % 4 != 0)
        return UNI_UNDEF;
    if (!multiple_value && !(4 <= l && l <= 6))
        return UNI_UNDEF;
    for (i = 0; i < l; i++) {
        if (!isXdigit((unsigned char)s[i]))
            return UNI_UNDEF;
        if (multiple_value) {
            if (i % 4 == 3) {
                if (sscanf(s + i - 3, "%4lX", &code) != 1)
                    return UNI_UNDEF;
                if (!((0x0000 <= code && code <= 0xD7FF) ||
                      (0xE000 <= code && code <= 0xFFFF)))
                    return UNI_UNDEF;
            }
        } else {                /* single value */
            if (i == l - 1) {
                if (sscanf(s, "%lX", &code) != 1)
                    return UNI_UNDEF;
                if (!((0x0000 <= code && code <= 0xD7FF) ||
                      (0xE000 <= code && code <= 0x10FFFF)))
                    return UNI_UNDEF;
            }
        }
    }
    return code;
}

/*

    This function set proper values to |*gp| based on |s|; in case it returns
    |gp->code == UNI_EXTRA_STRING| then the caller is responsible for freeing
    |gp->unicode_seq| too.

*/

static void set_glyph_unicode(char *s, glyph_unicode_entry * gp)
{
    char buf[SMALL_BUF_SIZE], buf2[SMALL_BUF_SIZE], *p;
    long code;
    boolean last_component;
    glyph_unicode_entry tmp, *ptmp;
    /*tex Skip dummy entries. */
    if (s == NULL || s == notdef)
        return;
    /*tex Strip everything after the first dot. */
    p = strchr(s, '.');
    if (p != NULL) {
        *buf = 0;
        strncat(buf, s, (size_t) (p - s));
        s = buf;
    }
    if (strlen(s) == 0)
        return;
    /*tex Check for case of multiple components separated by |_|. */
    p = strchr(s, '_');
    if (p != NULL) {
        assert(strlen(s) < sizeof(buf));
        if (s != buf) {
            strcpy(buf, s);
            p = strchr(buf, '_');
            s = buf;
        }
        *buf2 = 0;
        last_component = false;
        for (;;) {
            *p = 0;
            tmp.code = UNI_UNDEF;
            set_glyph_unicode(s, &tmp);
            switch (tmp.code) {
            case UNI_UNDEF:
                /*tex Not found, do nothing. */
                break;
            case UNI_STRING:
                /*tex |s| matched an entry with string value in the database. */
                assert(tmp.unicode_seq != NULL);
                assert(strlen(buf2) + strlen(tmp.unicode_seq) < sizeof(buf2));
                strcat(buf2, tmp.unicode_seq);
                break;
            case UNI_EXTRA_STRING:
                /*tex |s| is a multiple value of form "uniXXXX" */
                assert(strlen(buf2) + strlen(tmp.unicode_seq) < sizeof(buf2));
                strcat(buf2, tmp.unicode_seq);
                xfree(tmp.unicode_seq);
                break;
            default:
                /*tex
                    |s| matched an entry with numeric value in the database, or a
                    value derived from |uXXXX|.
                */
                assert(tmp.code >= 0);
                strcat(buf2, utf16be_str(tmp.code));
            }
            if (last_component)
                break;
            s = p + 1;
            p = strchr(s, '_');
            if (p == NULL) {
                p = strend(s);
                last_component = true;
            }
        }
        gp->code = UNI_EXTRA_STRING;
        gp->unicode_seq = xstrdup(buf2);
        return;
    }
    /*tex Lookup glyph name in the database. */
    tmp.name = s;
    tmp.code = UNI_UNDEF;
    ptmp = (glyph_unicode_entry *) avl_find(glyph_unicode_tree, &tmp);
    if (ptmp != NULL) {
        gp->code = ptmp->code;
        gp->unicode_seq = ptmp->unicode_seq;
        return;
    }
    /*tex Check for case of |uniXXXX|, multiple 4-hex-digit values allowed. */
    if (str_prefix(s, "uni")) {
        p = s + strlen("uni");
        code = check_unicode_value(p, true);
        if (code != UNI_UNDEF) {
            if (strlen(p) == 4) {
                /*tex Single value: */
                gp->code = code;
            } else {
                /*tex Multiple value: */
                gp->code = UNI_EXTRA_STRING;
                gp->unicode_seq = xstrdup(p);
            }
        }
        /*tex Since the last case cannot happen: */
        return;
    }
    /*tex Check for case of |uXXXX|, a single value up to 6 hex digits. */
    if (str_prefix(s, "u")) {
        p = s + strlen("u");
        code = check_unicode_value(p, false);
        if (code != UNI_UNDEF) {
            assert(code >= 0);
            gp->code = code;
        }
    }
}

/*
static void set_cid_glyph_unicode(long index, glyph_unicode_entry * gp, internal_font_number f)
{
    char *s;
    if (font_tounicode(f)) {
        if ((s = get_charinfo_tounicode(char_info(f, (int) index))) != NULL) {
            gp->code = UNI_EXTRA_STRING;
            gp->unicode_seq = xstrdup(s);
        } else {
            // No fall back as we're providing them ourselves.
        }
    } else {
        // Fall back
        gp->code = index;
    }
}
*/

int write_tounicode(PDF pdf, char **glyph_names, char *name)
{
    char buf[SMALL_BUF_SIZE], *p;
    static char builtin_suffix[] = "-builtin";
    short range_size[257];
    glyph_unicode_entry gtab[257];
    int objnum;
    int i, j;
    int bfchar_count, bfrange_count, subrange_count;
    assert(strlen(name) + strlen(builtin_suffix) < SMALL_BUF_SIZE);
    if (glyph_unicode_tree == NULL) {
        pdf->gen_tounicode = 0;
        return 0;
    }
    strcpy(buf, name);
    if ((p = strrchr(buf, '.')) != NULL && strcmp(p, ".enc") == 0) {
        /*tex
            Strip |.enc| from encoding name.
        */
        *p = 0;
    } else {
        /*
            The suffix |.enc| is not present so this is a builtin encoding so the
            name becomes e.g. |cmr10-builtin|.
        */
        strcat(buf, builtin_suffix);
    }
    objnum = pdf_create_obj(pdf, obj_type_others, 0);
    pdf_begin_obj(pdf, objnum, OBJSTM_NEVER);
    pdf_begin_dict(pdf);
    pdf_dict_add_streaminfo(pdf);
    pdf_end_dict(pdf);
    pdf_begin_stream(pdf);
    pdf_printf(pdf,
        "%%!PS-Adobe-3.0 Resource-CMap\n"
        "%%%%DocumentNeededResources: ProcSet (CIDInit)\n"
        "%%%%IncludeResource: ProcSet (CIDInit)\n"
        "%%%%BeginResource: CMap (TeX-%s-0)\n"
        "%%%%Title: (TeX-%s-0 TeX %s 0)\n"
        "%%%%Version: 1.000\n"
        "%%%%EndComments\n"
        "/CIDInit /ProcSet findresource begin\n"
        "12 dict begin\n"
        "begincmap\n"
        "/CIDSystemInfo\n"
        "<< /Registry (TeX)\n"
        "/Ordering (%s)\n"
        "/Supplement 0\n"
        ">> def\n"
        "/CMapName /TeX-%s-0 def\n"
        "/CMapType 2 def\n"
        "1 begincodespacerange\n"
        "<00> <FF>\n" "endcodespacerange\n",
        buf, buf, buf, buf, buf);
    /*tex Set gtab: */
    for (i = 0; i < 256; ++i) {
        gtab[i].code = UNI_UNDEF;
        set_glyph_unicode(glyph_names[i], &gtab[i]);
    }
    gtab[256].code = UNI_UNDEF;
    /*tex Set |range_size|: */
    for (i = 0; i < 256;) {
        if (gtab[i].code == UNI_STRING || gtab[i].code == UNI_EXTRA_STRING) {
            /*tex Single entry: */
            range_size[i] = 1;
            i++;
        } else if (gtab[i].code == UNI_UNDEF) {
            /*tex No entry: */
            range_size[i] = 0;
            i++;
        } else {
            /*tex |gtab[i].code >= 0| */
            j = i;
            while (i < 256 && gtab[i + 1].code >= 0 && gtab[i].code + 1 == gtab[i + 1].code)
                i++;
            /*tex
                At this point |i| is the last entry of the subrange so we move |i| to
                the next entry.
            */
            i++;
            range_size[j] = (short) (i - j);
        }
    }
    /*tex Calculate |bfrange_count| and |bfchar_count|. */
    bfrange_count = 0;
    bfchar_count = 0;
    for (i = 0; i < 256;) {
        if (range_size[i] == 1) {
            bfchar_count++;
            i++;
        } else if (range_size[i] > 1) {
            bfrange_count++;
            i += range_size[i];
        } else
            i++;
    }
    /*tex Write out |bfrange|. */
    i = 0;
  write_bfrange:
    if (bfrange_count > 100)
        subrange_count = 100;
    else
        subrange_count = bfrange_count;
    bfrange_count -= subrange_count;
    pdf_printf(pdf, "%i beginbfrange\n", subrange_count);
    for (j = 0; j < subrange_count; j++) {
        while (range_size[i] <= 1 && i < 256)
            i++;
        assert(i < 256);
        pdf_printf(pdf, "<%02X> <%02X> <%s>\n", i, i + range_size[i] - 1,
                   utf16be_str(gtab[i].code));
        i += range_size[i];
    }
    pdf_printf(pdf, "endbfrange\n");
    if (bfrange_count > 0)
        goto write_bfrange;
    /*tex Write out |bfchar|. */
    i = 0;
  write_bfchar:
    if (bfchar_count > 100)
        subrange_count = 100;
    else
        subrange_count = bfchar_count;
    bfchar_count -= subrange_count;
    pdf_printf(pdf, "%i beginbfchar\n", subrange_count);
    for (j = 0; j < subrange_count; j++) {
        while (i < 256) {
            if (range_size[i] > 1)
                i += range_size[i];
            else if (range_size[i] == 0)
                i++;
            else
                break;
        }
        assert(i < 256 && gtab[i].code != UNI_UNDEF);
        if (gtab[i].code == UNI_STRING || gtab[i].code == UNI_EXTRA_STRING) {
            assert(gtab[i].unicode_seq != NULL);
            pdf_printf(pdf, "<%02X> <%s>\n", i, gtab[i].unicode_seq);
        } else
            pdf_printf(pdf, "<%02X> <%s>\n", i, utf16be_str(gtab[i].code));
        i++;
    }
    pdf_printf(pdf, "endbfchar\n");
    if (bfchar_count > 0)
        goto write_bfchar;
    /*tex Free strings allocated by |set_glyph_unicode|. */
    for (i = 0; i < 256; ++i) {
        if (gtab[i].code == UNI_EXTRA_STRING)
            xfree(gtab[i].unicode_seq);
    }
    pdf_printf(pdf,
        "endcmap\n"
        "CMapName currentdict /CMap defineresource pop\n"
        "end\n"
        "end\n"
        "%%%%EndResource\n"
        "%%%%EOF\n");
    pdf_end_stream(pdf);
    pdf_end_obj(pdf);
    return objnum;
}

int write_cid_tounicode(PDF pdf, fo_entry * fo, internal_font_number f)
{
    static int range_size[65537];
    static glyph_unicode_entry gtab[65537];
    int objnum;
    int i, j, k, tu;
    int bfchar_count, bfrange_count, subrange_count;
    char *buf;
    char *s;
    buf = xmalloc((unsigned) (strlen(fo->fd->fontname) + 8));
    sprintf(buf, "%s-%s", (fo->fd->subset_tag != NULL ? fo->fd->subset_tag : "UCS"), fo->fd->fontname);
    objnum = pdf_create_obj(pdf, obj_type_others, 0);
    pdf_begin_obj(pdf, objnum, OBJSTM_NEVER);
    pdf_begin_dict(pdf);
    pdf_dict_add_streaminfo(pdf);
    pdf_end_dict(pdf);
    pdf_begin_stream(pdf);
    pdf_printf(pdf,
        "%%!PS-Adobe-3.0 Resource-CMap\n"
        "%%%%DocumentNeededResources: ProcSet (CIDInit)\n"
        "%%%%IncludeResource: ProcSet (CIDInit)\n"
        "%%%%BeginResource: CMap (TeX-%s-0)\n"
        "%%%%Title: (TeX-%s-0 TeX %s 0)\n"
        "%%%%Version: 1.000\n"
        "%%%%EndComments\n"
        "/CIDInit /ProcSet findresource begin\n"
        "12 dict begin\n"
        "begincmap\n"
        "/CIDSystemInfo\n"
        "<< /Registry (TeX)\n"
        "/Ordering (%s)\n"
        "/Supplement 0\n"
        ">> def\n"
        "/CMapName /TeX-Identity-%s def\n"
        "/CMapType 2 def\n"
        "1 begincodespacerange\n"
        "<0000> <FFFF>\n"
        "endcodespacerange\n",
        buf, buf, buf, buf, buf);
    xfree(buf);
    /*tex Set up |gtab|: */
    for (i = 0; i < 65537; ++i) {
        gtab[i].code = UNI_UNDEF;
    }
    for (k = 1; k <= max_font_id(); k++) {
        if (k == f || -f == pdf_font_num(k)) {
            for (i = font_bc(k); i <= font_ec(k); i++) {
                if (quick_char_exists(k, i) && char_used(k, i)) {
                    j = char_index(k, i);
                    if (gtab[j].code == UNI_UNDEF) {
                        /*
                        set_cid_glyph_unicode(i, &gtab[j], f);
                        */
                        tu = 0;
                        /*tex
                                First look in the instance (new). Ff this fails
                                us (context) it will go away or become an option.
                        */
                        if (font_tounicode(k)) {
                            tu = 1; /*tex no fallback to index */
                            if ((s = get_charinfo_tounicode(char_info(k, (int) i))) != NULL) {
                                gtab[j].code = UNI_EXTRA_STRING;
                                gtab[j].unicode_seq = xstrdup(s);
                            }
                        }
                        /*tex
                            Then look in the parent (was default, so one can
                            still be sparse).
                        */
                        if (k != f && gtab[j].code == UNI_UNDEF && font_tounicode(f)) {
                            tu = 1; /*tex no fallback to index */
                            if ((s = get_charinfo_tounicode(char_info(f, (int) i))) != NULL) {
                                gtab[j].code = UNI_EXTRA_STRING;
                                gtab[j].unicode_seq = xstrdup(s);
                            }
                        }
                        if (! tu) {
                            /*tex No tonunicode so we map onto index. */
                            gtab[j].code = i;
                        }
                    }
                }
            }
        }
    }
    /*tex Set |range_size|: */
    for (i = 0; i < 65536;) {
        if (gtab[i].code == UNI_STRING || gtab[i].code == UNI_EXTRA_STRING) {
            /*tex Single entry. */
            range_size[i] = 1;
            i++;
        } else if (gtab[i].code == UNI_UNDEF) {
            /*tex No entry. */
            range_size[i] = 0;
            i++;
        } else {
            /*tex |gtab[i].code >= 0| */
            j = i;
            k = i % 256;
            while (i < 65536 && k<255 && gtab[i + 1].code >= 0 &&
                   gtab[i].code + 1 == gtab[i + 1].code) {
                i++; k++;
            }
            /* tex
                At this point |i| is the last entry of the subrange so we move
                |i| to the next entry
            */
            i++;
            range_size[j] = i - j;
        }
    }
    /*tex Calculate |bfrange_count| and |bfchar_count|. */
    bfrange_count = 0;
    bfchar_count = 0;
    for (i = 0; i < 65536;) {
        if (range_size[i] == 1) {
            bfchar_count++;
            i++;
        } else if (range_size[i] > 1) {
            bfrange_count++;
            i += range_size[i];
        } else
            i++;
    }
    /*tex Write out |bfrange|. */
    i = 0;
  write_bfrange:
    if (bfrange_count > 100)
        subrange_count = 100;
    else
        subrange_count = bfrange_count;
    bfrange_count -= subrange_count;
    pdf_printf(pdf, "%i beginbfrange\n", subrange_count);
    for (j = 0; j < subrange_count; j++) {
        while (range_size[i] <= 1 && i < 65536)
            i++;
        pdf_printf(pdf, "<%04X> <%04X> <%s>\n", i, i + range_size[i] - 1, utf16be_str(gtab[i].code));
        i += range_size[i];
    }
    pdf_printf(pdf, "endbfrange\n");
    if (bfrange_count > 0)
        goto write_bfrange;
    /*tex Write out |bfchar| */
    i = 0;
  write_bfchar:
    if (bfchar_count > 100)
        subrange_count = 100;
    else
        subrange_count = bfchar_count;
    bfchar_count -= subrange_count;
    pdf_printf(pdf, "%i beginbfchar\n", subrange_count);
    for (j = 0; j < subrange_count; j++) {
        while (i < 65536) {
            if (range_size[i] > 1)
                i += range_size[i];
            else if (range_size[i] == 0)
                i++;
            else
                /* |range_size[i] == 1| */
                break;
        }
        assert(i < 65536 && gtab[i].code != UNI_UNDEF);
        if (gtab[i].code == UNI_STRING || gtab[i].code == UNI_EXTRA_STRING) {
            pdf_printf(pdf, "<%04X> <%s>\n", i, gtab[i].unicode_seq);
        } else
            pdf_printf(pdf, "<%04X> <%s>\n", i, utf16be_str(gtab[i].code));
        i++;
    }
    pdf_printf(pdf, "endbfchar\n");
    if (bfchar_count > 0)
        goto write_bfchar;
    /*tex Free strings allocated by |set_glyph_unicode|: */
    for (i = 0; i < 65536; ++i) {
        if (gtab[i].code == UNI_EXTRA_STRING)
            xfree(gtab[i].unicode_seq);
    }
    pdf_printf(pdf,
        "endcmap\n"
        "CMapName currentdict /CMap defineresource pop\n"
        "end\n"
        "end\n"
        "%%%%EndResource\n"
        "%%%%EOF\n");
    pdf_end_stream(pdf);
    pdf_end_obj(pdf);
    return objnum;
}
