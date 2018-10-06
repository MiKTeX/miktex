/* mapfile.h

   Copyright 2009 Taco Hoekwater <taco@luatex.org>

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
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>. */


#ifndef MAPFILE_H
#  define MAPFILE_H

#  define F_INCLUDED          (1 << 0)
#  define F_SUBSETTED         (1 << 1)
#  define F_STDT1FONT         (1 << 2)
/* we don't support sub fonts */
#  define F_TYPE1             (1 << 4)
#  define F_TRUETYPE          (1 << 5)
#  define F_OTF               (1 << 6)
#  define F_CIDKEYED          (1 << 7)
#  define F_SLANTSET          (1 << 8)
#  define F_EXTENDSET         (1 << 9)
#  define F_INUSE             (1 << 10)

typedef enum { MAPFILE, MAPLINE } maptype;

#  define set_included(fm)    ((fm)->type =  (unsigned short)((fm)->type | F_INCLUDED))
#  define set_subsetted(fm)   ((fm)->type =  (unsigned short)((fm)->type | F_SUBSETTED))
#  define set_std_t1font(fm)  ((fm)->type =  (unsigned short)((fm)->type | F_STDT1FONT))
#  define set_type1(fm)       ((fm)->type =  (unsigned short)((fm)->type | F_TYPE1))
#  define set_truetype(fm)    ((fm)->type =  (unsigned short)((fm)->type | F_TRUETYPE))
#  define set_opentype(fm)    ((fm)->type =  (unsigned short)((fm)->type | F_OTF))
#  define set_cidkeyed(fm)    ((fm)->type =  (unsigned short)((fm)->type | F_CIDKEYED))
#  define set_slantset(fm)    ((fm)->type =  (unsigned short)((fm)->type | F_SLANTSET))
#  define set_extendset(fm)   ((fm)->type =  (unsigned short)((fm)->type | F_EXTENDSET))
#  define set_inuse(fm)       ((fm)->type =  (unsigned short)((fm)->type | F_INUSE))

#  define unset_included(fm)  ((fm)->type = (unsigned short)((fm)->type & ~F_INCLUDED))
#  define unset_subsetted(fm) ((fm)->type = (unsigned short)((fm)->type & ~F_SUBSETTED))
#  define unset_std_t1font(fm)((fm)->type = (unsigned short)((fm)->type & ~F_STDT1FONT))
#  define unset_type1(fm)     ((fm)->type = (unsigned short)((fm)->type & ~F_TYPE1))
#  define unset_truetype(fm)  ((fm)->type = (unsigned short)((fm)->type & ~F_TRUETYPE))
#  define unset_opentype(fm)  ((fm)->type = (unsigned short)((fm)->type & ~F_OTF))
#  define unset_cidkeyed(fm)  ((fm)->type = (unsigned short)((fm)->type & ~F_CIDKEYED))
#  define unset_slantset(fm)  ((fm)->type = (unsigned short)((fm)->type & ~F_SLANTSET))
#  define unset_extendset(fm) ((fm)->type = (unsigned short)((fm)->type & ~F_EXTENDSET))
#  define unset_inuse(fm)     ((fm)->type = (unsigned short)((fm)->type & ~F_INUSE))

#  define is_included(fm)     (((fm)->type & F_INCLUDED) != 0)
#  define is_subsetted(fm)    (((fm)->type & F_SUBSETTED) != 0)
#  define is_std_t1font(fm)   (((fm)->type & F_STDT1FONT) != 0)
#  define is_type1(fm)        (((fm)->type & F_TYPE1) != 0)
#  define is_truetype(fm)     (((fm)->type & F_TRUETYPE) != 0)
#  define is_opentype(fm)     (((fm)->type & F_OTF) != 0)
#  define is_cidkeyed(fm)     (((fm)->type & F_CIDKEYED) != 0)
#  define is_slantset(fm)     (((fm)->type & F_SLANTSET) != 0)
#  define is_extendset(fm)    (((fm)->type & F_EXTENDSET) != 0)
#  define is_inuse(fm)        (((fm)->type & F_INUSE) != 0)

#  define fm_slant(fm)        (fm)->slant
#  define fm_extend(fm)       (fm)->extend
#  define fm_fontfile(fm)     (fm)->ff_name

#  define is_reencoded(fm)    ((fm)->encname != NULL)
#  define is_fontfile(fm)     (fm_fontfile(fm) != NULL)
#  define is_t1fontfile(fm)   (is_fontfile(fm) && is_type1(fm))
#  define is_builtin(fm)      (!is_fontfile(fm))

/**********************************************************************/

typedef struct {
    /* parameters scanned from the map file: */
    char *tfm_name;             /* TFM file name (1st field in map line) */
    char *ps_name;              /* PostScript name (optional 2nd field in map line) */
    int fd_flags;               /* font descriptor /Flags (PDF Ref. section 5.7.1) */
    int slant;                  /* SlantFont */
    int extend;                 /* ExtendFont */
    char *encname;              /* encoding file name */
    char *ff_name;              /* font file name */
    unsigned short type;        /* various flags */
} fm_entry;

typedef struct {
    char *ff_name;              /* base name of font file */
    char *ff_path;              /* full path to font file */
} ff_entry;

/**********************************************************************/

fm_entry *getfontmap(char *tfm_name);
void fm_free(void);
ff_entry *check_ff_exist(char *, boolean);
void pdfmapfile(int);
void pdfmapline(int);
void pdf_init_map_file(const char *map_name);
fm_entry *new_fm_entry(void);
void delete_fm_entry(fm_entry *);
int avl_do_entry(fm_entry *, int);
int check_std_t1font(char *s);
int is_subsetable(fm_entry * fm);
void process_map_item(char *s, int type);

#endif                          /* MAPFILE_H */
