% mapfile.w
%
% Copyright 1996-2006 Han The Thanh <thanh@@pdftex.org>
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
#include <math.h>
#include <kpathsea/c-auto.h>
#include <kpathsea/c-memstr.h>
#include <string.h>

#define FM_BUF_SIZE     1024

static FILE *fm_file;

static unsigned char *fm_buffer = NULL;
static int fm_size = 0;
static int fm_curbyte = 0;

#define fm_open(a)      (fm_file = fopen((char *)(a), FOPEN_RBIN_MODE))
#define fm_read_file()  readbinfile(fm_file,&fm_buffer,&fm_size)
#define fm_close()      xfclose(fm_file, cur_file_name)
#define fm_getchar()    fm_buffer[fm_curbyte++]
#define fm_eof()        (fm_curbyte>fm_size)
#define is_cfg_comment(c) \
    (c == 10 || c == '*' || c == '#' || c == ';' || c == '%')

typedef enum { FM_DUPIGNORE, FM_REPLACE, FM_DELETE } updatemode;

typedef struct mitem {
    updatemode mode;            /* FM_DUPIGNORE or FM_REPLACE or FM_DELETE */
    maptype type;               /* map file or map line */
    char *line;                 /* pointer to map file name or map line */
    int lineno;                 /* line number in map file */
} mapitem;
mapitem *mitem = NULL;

#define read_field(r, q, buf) do {  \
    q = buf;                        \
    while (*r != ' ' && *r != '<' && *r != '"' && *r != '\0') \
        *q++ = *r++;                \
    *q = '\0';                      \
    skip_char(r, ' ');                  \
} while (0)

#define set_field(F) do {           \
    if (q > buf)                    \
        fm->F = xstrdup(buf);       \
    if (*r == '\0')                 \
        goto done;                  \
} while (0)

fm_entry *new_fm_entry(void)
{
    fm_entry *fm;
    fm = xtalloc(1, fm_entry);
    fm->tfm_name = NULL;
    fm->sfd_name = NULL;
    fm->ps_name = NULL;
    fm->fd_flags = FD_FLAGS_NOT_SET_IN_MAPLINE;
    fm->ff_name = NULL;
    fm->encname = NULL;
    fm->type = 0;
    fm->slant = 0;
    fm->extend = 1000;
    fm->pid = -1;
    fm->eid = -1;
    fm->subfont = NULL;
    unset_slantset(fm);
    unset_extendset(fm);
    unset_inuse(fm);
    return fm;
}

void delete_fm_entry(fm_entry * fm)
{
    xfree(fm->tfm_name);
    xfree(fm->sfd_name);
    xfree(fm->ps_name);
    xfree(fm->ff_name);
    xfree(fm);
}

static ff_entry *new_ff_entry(void)
{
    ff_entry *ff;
    ff = xtalloc(1, ff_entry);
    ff->ff_name = NULL;
    ff->ff_path = NULL;
    return ff;
}

static void delete_ff_entry(ff_entry * ff)
{
    xfree(ff->ff_name);
    xfree(ff->ff_path);
    xfree(ff);
}

/**********************************************************************/

static struct avl_table *tfm_tree = NULL;
static struct avl_table *ff_tree = NULL;
static struct avl_table *encname_tree = NULL;

/* AVL sort fm_entry into tfm_tree by tfm_name */

static int comp_fm_entry_tfm(const void *pa, const void *pb, void *p)
{
    (void) p;
    return strcmp(((const fm_entry *) pa)->tfm_name,
                  ((const fm_entry *) pb)->tfm_name);
}

/* AVL sort ff_entry into ff_tree by ff_name */

static int comp_ff_entry(const void *pa, const void *pb, void *p)
{
    (void) p;
    return strcmp(((const ff_entry *) pa)->ff_name,
                  ((const ff_entry *) pb)->ff_name);
}

static void create_avl_trees(void)
{
    assert(tfm_tree == NULL);
    tfm_tree = avl_create(comp_fm_entry_tfm, NULL, &avl_xallocator);
    assert(tfm_tree != NULL);
    assert(ff_tree == NULL);
    ff_tree = avl_create(comp_ff_entry, NULL, &avl_xallocator);
    assert(ff_tree != NULL);
    assert(encname_tree == NULL);
    encname_tree = avl_create(comp_string_entry, NULL, &avl_xallocator);
    assert(encname_tree != NULL);
}

int avl_do_entry(fm_entry * fm, int mode)
{
    fm_entry *p;
    void *a;
    void **aa;
    int delete_new = 0;
    if (tfm_tree == NULL)
        create_avl_trees();
    p = (fm_entry *) avl_find(tfm_tree, fm);
    if (p != NULL) {
        switch (mode) {
        case FM_DUPIGNORE:
            formatted_warning("map file", "entry for '%s' already exists, duplicates ignored", fm->tfm_name);
            delete_new = 1;
            break;
        case FM_REPLACE:
        case FM_DELETE:
            if (is_inuse(p)) {
                formatted_warning("map file", "entry for '%s' has been used, replace/delete not allowed", fm->tfm_name);
                delete_new = 1;
            } else {
                a = avl_delete(tfm_tree, p);
                assert(a != NULL);
                delete_fm_entry(p);
            }
            break;
        default:
            assert(0);
        }
    }
    if ((mode == FM_DUPIGNORE || mode == FM_REPLACE) && delete_new == 0) {
        aa = avl_probe(tfm_tree, fm);
        assert(aa != NULL);
    } else
        delete_new = 1;
    return delete_new;
}

/* add the encoding name to an AVL tree. this has nothing to do with writeenc.c */

static char *add_encname(char *s)
{
    char *p;
    void **aa;
    assert(s != NULL);
    assert(encname_tree != NULL);
    if ((p = (char *) avl_find(encname_tree, s)) == NULL) {     /* encoding name not yet registered */
        p = xstrdup(s);
        aa = avl_probe(encname_tree, p);
        assert(aa != NULL);
    }
    return p;
}

/**********************************************************************/
/* consistency check for map entry, with warn flag */

static int check_fm_entry(fm_entry * fm, boolean warn)
{
    int a = 0;
    assert(fm != NULL);

    if (is_fontfile(fm) && !is_included(fm)) {
        if (warn)
            formatted_warning("map file",
                 "ambiguous entry for '%s': font file present but not included, "
                 "will be treated as font file not present", fm->tfm_name);
        xfree(fm->ff_name);
        /* do not set variable |a| as this entry will be still accepted */
    }

    /* if both ps_name and font file are missing, drop this entry */
    if (fm->ps_name == NULL && !is_fontfile(fm)) {
        if (warn)
            formatted_warning("map file", "invalid entry for '%s': both ps_name and font file missing", fm->tfm_name);
        a += 1;
    }

    /* TrueType fonts cannot be reencoded without subsetting */
    if (is_truetype(fm) && is_reencoded(fm) && !is_subsetted(fm)) {
        if (warn)
            formatted_warning("map file", "invalid entry for '%s': only subsetted TrueType font can be reencoded", fm->tfm_name);
        a += 2;
    }

    /* the value of SlantFont and ExtendFont must be reasonable */
    if (fm->slant < FONT_SLANT_MIN || fm->slant > FONT_SLANT_MAX) {
        if (warn)
            formatted_warning("map file", "invalid entry for '%s': value '%g' is to large for SlantFont",
                 fm->tfm_name, fm->slant / 1000.0);
        a += 8;
    }
    if (fm->extend < FONT_EXTEND_MIN || fm->extend > FONT_EXTEND_MAX) {
        if (warn)
            formatted_warning("map file", "invalid entry for '%s': value '%g' is too large for ExtendFont",
                 fm->tfm_name, fm->extend / 1000.0);
        a += 16;
    }

    /* subfonts must be used with subsetted non-reencoded TrueType fonts */
    if (fm->pid != -1 &&
        !(is_truetype(fm) && is_subsetted(fm) && !is_reencoded(fm))) {
        if (warn)
            formatted_warning("map file", "invalid entry for '%s': PidEid can be used only with subsetted non-reencoded TrueType fonts",
                 fm->tfm_name);
        a += 32;
    }

    return a;
}

/**********************************************************************/
/* returns the font number if s is one of the 14 std. font names, -1 otherwise; speed-trimmed. */

int check_std_t1font(char *s)
{
    static const char *std_t1font_names[] = {
        "Courier",              /* 0:7 */
        "Courier-Bold",         /* 1:12 */
        "Courier-Oblique",      /* 2:15 */
        "Courier-BoldOblique",  /* 3:19 */
        "Helvetica",            /* 4:9 */
        "Helvetica-Bold",       /* 5:14 */
        "Helvetica-Oblique",    /* 6:17 */
        "Helvetica-BoldOblique",        /* 7:21 */
        "Symbol",               /* 8:6 */
        "Times-Roman",          /* 9:11 */
        "Times-Bold",           /* 10:10 */
        "Times-Italic",         /* 11:12 */
        "Times-BoldItalic",     /* 12:16 */
        "ZapfDingbats"          /* 13:12 */
    };
    static const int index[] =
        { -1, -1, -1, -1, -1, -1, 8, 0, -1, 4, 10, 9, -1, -1, 5, 2, 12, 6, -1,
        3, -1, 7
    };
    size_t n;
    int k = -1;
    assert(s != NULL);
    n = strlen(s);
    if (n > 21)
        return -1;
    if (n == 12) {              /* three names have length 12 */
        switch (*s) {
        case 'C':
            k = 1;              /* Courier-Bold */
            break;
        case 'T':
            k = 11;             /* Times-Italic */
            break;
        case 'Z':
            k = 13;             /* ZapfDingbats */
            break;
        default:
            return -1;
        }
    } else
        k = index[n];
    if (k > -1 && !strcmp(std_t1font_names[k], s))
        return k;
    return -1;
}

/**********************************************************************/

static void fm_scan_line(void)
{
    int a, b, c, j, u = 0, v = 0;
    char cc;
    float d;
    fm_entry *fm;
    char fm_line[FM_BUF_SIZE], buf[FM_BUF_SIZE];
    char *p, *q, *s;
    char *r = NULL;
    switch (mitem->type) {
    case MAPFILE:
        p = fm_line;
        while (!fm_eof()) {
            if (fm_curbyte == fm_size) {
                fm_curbyte++;
                cc = 10;
            } else {
                cc = (char) fm_getchar();
            }
            append_char_to_buf(cc, p, fm_line, FM_BUF_SIZE);
            if (cc == 10)
                break;
        }
        *(--p) = '\0';
        r = fm_line;
        break;
    case MAPLINE:
        r = mitem->line;        /* work on string from makecstring() */
        break;
    default:
        assert(0);
    }
    if (*r == '\0' || is_cfg_comment(*r))
        return;
    fm = new_fm_entry();
    read_field(r, q, buf);
    set_field(tfm_name);
    if (!isdigit((unsigned char)*r)) {         /* 2nd field ps_name may not start with a digit */
        read_field(r, q, buf);
        set_field(ps_name);
    }
    if (isdigit((unsigned char)*r)) {          /* font descriptor /Flags given? */
        for (s = r; isdigit((unsigned char)*s); s++);
        if (*s == ' ' || *s == '"' || *s == '<' || *s == '\0') {        /* not e. g. 8r.enc */
            fm->fd_flags = atoi(r);
            while (isdigit((unsigned char)*r))
                r++;
        }
    }
    while (1) {                 /* loop through "specials", encoding, font file */
        skip_char(r, ' ');
        switch (*r) {
        case '\0':
            goto done;
        case '"':              /* opening quote */
            r++;
            u = v = 0;
            do {
                skip_char(r, ' ');
                if (sscanf(r, "%f %n", &d, &j) > 0) {
                    s = r + j;  /* jump behind number, eat also blanks, if any */
                    if (*(s - 1) == 'E' || *(s - 1) == 'e')
                        s--;    /* e. g. 0.5ExtendFont: %f = 0.5E */
                    if (str_prefix(s, "SlantFont")) {
                        d *= (float) 1000.0;    /* correct rounding also for neg. numbers */
                        fm->slant = (int) (d > 0 ? d + 0.5 : d - 0.5);
                        set_slantset(fm);
                        r = s + strlen("SlantFont");
                    } else if (str_prefix(s, "ExtendFont")) {
                        d *= (float) 1000.0;
                        fm->extend = (int) (d > 0 ? d + 0.5 : d - 0.5);
                        set_extendset(fm);
                        r = s + strlen("ExtendFont");
                    } else {    /* unknown name */
                        for (r = s; *r != ' ' && *r != '"' && *r != '\0'; r++); /* jump over name */
                        c = *r; /* remember char for temporary end of string */
                        *r = '\0';
                        formatted_warning("map file", "invalid entry for '%s': unknown name '%s' ignored", fm->tfm_name, s);
                        *r = (char) c;
                    }
                } else
                    for (; *r != ' ' && *r != '"' && *r != '\0'; r++);
            }
            while (*r == ' ');
            if (*r == '"')      /* closing quote */
                r++;
            else {
                formatted_warning("map file", "invalid entry for '%s': closing quote missing", fm->tfm_name);
                goto bad_line;
            }
            break;
        case 'P':              /* handle cases for subfonts like 'PidEid=3,1' */
            if (sscanf(r, "PidEid=%i, %i %n", &a, &b, &c) >= 2) {
                fm->pid = (short) a;
                fm->eid = (short) b;
                r += c;
                break;
            }
        default:               /* encoding or font file specification */
            a = b = 0;
            if (*r == '<') {
                a = *r++;
                if (*r == '<' || *r == '[')
                    b = *r++;
            }
            read_field(r, q, buf);
            /* encoding, formats: '8r.enc' or '<8r.enc' or '<[8r.enc' */
            if (strlen(buf) > 4 && strcasecmp(strend(buf) - 4, ".enc") == 0) {
                fm->encname = add_encname(buf);
                u = v = 0;      /* u, v used if intervening blank: "<< foo" */
            } else if (strlen(buf) > 0) {       /* file name given */
                /* font file, formats:
                 * subsetting:    '<cmr10.pfa'
                 * no subsetting: '<<cmr10.pfa'
                 * no embedding:  'cmr10.pfa'
                 */
                if (a == '<' || u == '<') {
                    set_included(fm);
                    if ((a == '<' && b == 0) || (a == 0 && v == 0))
                        set_subsetted(fm);
                    /* otherwise b == '<' (or '[') => no subsetting */
                }
                set_field(ff_name);
                u = v = 0;
            } else {
                u = a;
                v = b;
            }
        }
    }
  done:
    if (fm->ps_name != NULL && (check_std_t1font(fm->ps_name) >= 0))
        set_std_t1font(fm);
    if (is_fontfile(fm) && strlen(fm_fontfile(fm)) > 3) {
        if (strcasecmp(strend(fm_fontfile(fm)) - 4, ".ttf") == 0)
            set_truetype(fm);
        else if (strcasecmp(strend(fm_fontfile(fm)) - 4, ".ttc") == 0)
            set_truetype(fm);
        else if (strcasecmp(strend(fm_fontfile(fm)) - 4, ".otf") == 0)
            set_opentype(fm);
        else
            set_type1(fm);
    } else
        set_type1(fm);          /* assume a builtin font is Type1 */
    if (check_fm_entry(fm, true) != 0)
        goto bad_line;
    /*
       Until here the map line has been completely scanned without errors;
       fm points to a valid, freshly filled-out fm_entry structure.
       Now follows the actual work of registering/deleting.
     */
    if (handle_subfont_fm(fm, mitem->mode))     /* is this a subfont? */
        return;
    if (avl_do_entry(fm, mitem->mode) == 0)
        return;
  bad_line:
    delete_fm_entry(fm);
}

/**********************************************************************/

static void fm_read_info(void)
{
    int callback_id;
    int file_opened = 0;

    if (tfm_tree == NULL)
        create_avl_trees();
    if (mitem->line == NULL)    /* nothing to do */
        return;
    mitem->lineno = 1;
    switch (mitem->type) {
    case MAPFILE:
        xfree(fm_buffer);
        fm_curbyte = 0;
        fm_size = 0;
        cur_file_name = luatex_find_file(mitem->line, find_map_file_callback);
        if (cur_file_name) {
            callback_id = callback_defined(read_map_file_callback);
            if (callback_id > 0) {
                if (run_callback(callback_id, "S->bSd", cur_file_name,
                                 &file_opened, &fm_buffer, &fm_size)) {
                    if (file_opened) {
                        if (fm_size > 0) {
                            report_start_file(filetype_map,cur_file_name);
                            while (!fm_eof()) {
                                fm_scan_line();
                                mitem->lineno++;
                            }
                            report_stop_file(filetype_map);
                            fm_file = NULL;
                        }
                    } else {
                        formatted_warning("map file", "cannot open font map file '%s'", cur_file_name);
                    }
                } else {
                    formatted_warning("map file", "cannot open font map file '%s'", cur_file_name);
                }
            } else {
                if (!fm_open(cur_file_name)) {
                    formatted_warning("map file", "cannot open font map file '%s'", cur_file_name);
                } else {
                    fm_read_file();
                    report_start_file(filetype_map,cur_file_name);
                    while (!fm_eof()) {
                        fm_scan_line();
                        mitem->lineno++;
                    }
                    fm_close();
                    report_stop_file(filetype_map);
                    fm_file = NULL;
                }
            }
            cur_file_name = NULL;
        }
        break;
    case MAPLINE:
        cur_file_name = NULL;
        fm_scan_line();
        break;
    default:
        assert(0);
    }
    mitem->line = NULL;         /* done with this line */
    cur_file_name = NULL;
    return;
}

/**********************************************************************/

fm_entry *getfontmap(char *tfm_name)
{
    fm_entry *fm;
    fm_entry tmp;
    if (tfm_name == NULL)       /* wide, lua loaded fonts may not have a name */
        return NULL;
    if (tfm_tree == NULL)
        fm_read_info();         /* only to read default map file */
    tmp.tfm_name = tfm_name;    /* Look up for tfmname */
    fm = (fm_entry *) avl_find(tfm_tree, &tmp);
    if (fm == NULL)
        return NULL;
    set_inuse(fm);
    return fm;
}

/**********************************************************************/
/*
 * Process map file given by its name or map line contents. Items not
 * beginning with [+-=] flush default map file, if it has not yet been
 * read. Leading blanks and blanks immediately following [+-=] are
 * ignored.
 */

void process_map_item(char *s, int type)
{
    char *p;
    int mode;
    if (*s == ' ')
        s++;                    /* ignore leading blank */
    switch (*s) {
    case '+':                  /* +mapfile.map, +mapline */
        mode = FM_DUPIGNORE;    /* insert entry, if it is not duplicate */
        s++;
        break;
    case '=':                  /* =mapfile.map, =mapline */
        mode = FM_REPLACE;      /* try to replace earlier entry */
        s++;
        break;
    case '-':                  /* -mapfile.map, -mapline */
        mode = FM_DELETE;       /* try to delete entry */
        s++;
        break;
    default:
        mode = FM_DUPIGNORE;    /* like +, but also: */
        mitem->line = NULL;     /* flush default map file name */
    }
    if (*s == ' ')
        s++;                    /* ignore blank after [+-=] */
    p = s;                      /* map item starts here */
    switch (type) {
    case MAPFILE:              /* remove blank at end */
        while (*p != '\0' && *p != ' ')
            p++;
        *p = '\0';
        break;
    case MAPLINE:              /* blank at end allowed */
        break;
    default:
        assert(0);
    }
    if (mitem->line != NULL)    /* read default map file first */
        fm_read_info();
    if (*s != '\0') {           /* only if real item to process */
        mitem->mode = mode;
        mitem->type = type;
        mitem->line = s;
        fm_read_info();
    }
}

void pdfmapfile(int t)
{
    char *s = tokenlist_to_cstring(t, true, NULL);
    process_map_item(s, MAPFILE);
    free(s);
}

void pdfmapline(int t)
{
    char *s = tokenlist_to_cstring(t, true, NULL);
    process_map_item(s, MAPLINE);
    free(s);
}

void pdf_init_map_file(char *map_name)
{
    assert(mitem == NULL);
    mitem = xtalloc(1, mapitem);
    mitem->mode = FM_DUPIGNORE;
    mitem->type = MAPFILE;
    mitem->line = map_name;
}

/**********************************************************************/
/*
 * Early check whether a font file exists. Search tree ff_tree is used
 * in 1st instance, as it may be faster than the kpse_find_file(), and
 * kpse_find_file() is called only once per font file name + expansion
 * parameter. This might help keeping speed, if many PDF pages with
 * same fonts are to be embedded.
 *
 * The ff_tree contains only font files, which are actually needed,
 * so this tree typically is much smaller than the tfm_tree.
 */

ff_entry *check_ff_exist(char *ff_name, boolean is_tt)
{
    ff_entry *ff;
    ff_entry tmp;
    void **aa;
    int callback_id;
    char *filepath = NULL;

    assert(ff_name != NULL);
    tmp.ff_name = ff_name;
    ff = (ff_entry *) avl_find(ff_tree, &tmp);
    if (ff == NULL) {           /* not yet in database */
        ff = new_ff_entry();
        ff->ff_name = xstrdup(ff_name);
        if (is_tt) {
            callback_id = callback_defined(find_truetype_file_callback);
            if (callback_id > 0) {
                run_callback(callback_id, "S->S", ff_name, &filepath);
                if (filepath && strlen(filepath) == 0)
                    filepath = NULL;
                ff->ff_path = filepath;
            } else {
                ff->ff_path = kpse_find_file(ff_name, kpse_truetype_format, 0);
            }
        } else {
            callback_id = callback_defined(find_type1_file_callback);
            if (callback_id > 0) {
                run_callback(callback_id, "S->S", ff_name, &filepath);
                if (filepath && strlen(filepath) == 0)
                    filepath = NULL;
                ff->ff_path = filepath;
            } else {
                ff->ff_path = kpse_find_file(ff_name, kpse_type1_format, 0);
            }
        }
        aa = avl_probe(ff_tree, ff);
        assert(aa != NULL);
    }
    return ff;
}

/**********************************************************************/

int is_subsetable(fm_entry * fm)
{
    assert(is_included(fm));
    return is_subsetted(fm);
}

/**********************************************************************/
/* cleaning up... */

static void destroy_fm_entry_tfm(void *pa, void *pb)
{
    fm_entry *fm;
    (void) pb;
    fm = (fm_entry *) pa;
    delete_fm_entry(fm);
}

static void destroy_ff_entry(void *pa, void *pb)
{
    ff_entry *ff;
    (void) pb;
    ff = (ff_entry *) pa;
    delete_ff_entry(ff);
}

void fm_free(void)
{
    if (tfm_tree != NULL) {
        avl_destroy(tfm_tree, destroy_fm_entry_tfm);
        tfm_tree = NULL;
    }
    if (ff_tree != NULL) {
        avl_destroy(ff_tree, destroy_ff_entry);
        ff_tree = NULL;
    }
}
