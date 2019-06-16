/* tt_glyf.h

   Copyright 2002 by Jin-Hwan Cho and Shunsaku Hirata,
   the dvipdfmx project team <dvipdfmx@project.ktug.or.kr>
   Copyright 2006-2008 Taco Hoekwater <taco@luatex.org>

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


#ifndef _TT_GLYF_H_
#  define _TT_GLYF_H_

struct tt_glyph_desc {
    USHORT gid;
    USHORT ogid;                /* GID in original font */
    USHORT advw, advh;
    SHORT lsb, tsb;
    SHORT llx, lly, urx, ury;
    ULONG length;
    BYTE *data;
};

struct tt_glyphs {
    USHORT num_glyphs;
    USHORT max_glyphs;
    USHORT last_gid;
    USHORT emsize;
    USHORT dw;                  /* optimal value for DW */
    USHORT default_advh;        /* default value */
    SHORT default_tsb;          /* default value */
    struct tt_glyph_desc *gd;
    unsigned char *used_slot;
};

extern struct tt_glyphs *tt_build_init(void);
extern void tt_build_finish(struct tt_glyphs *g);

extern USHORT tt_add_glyph(struct tt_glyphs *g, USHORT gid, USHORT new_gid);
extern USHORT tt_get_index(struct tt_glyphs *g, USHORT gid);
extern USHORT tt_find_glyph(struct tt_glyphs *g, USHORT gid);

extern int tt_build_tables(sfnt * sfont, struct tt_glyphs *g, fd_entry * fd);
extern int tt_get_metrics(sfnt * sfont, struct tt_glyphs *g);

#endif                          /* _TT_GLYF_H_ */
