/* directions.h

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


#ifndef DIRECTIONS_H
#  define DIRECTIONS_H

/*
    #  define dir_TLT  0
    #  define dir_TRT  4
    #  define dir_LTL  9
    #  define dir_RTT  24

    extern const char *dir_strings[128];
*/

extern const char *dir_strings[8];

extern int dir_swap;

/*
#  define RETURN_DIR_VALUES(a)       \
    if (s==luaS_##a##_ptr) {         \
        return (dir_##a);            \
    } else if (!absolute_only)  {    \
        if (s==luaS_p##a##_ptr)      \
            return (dir_##a);        \
        else if (s==luaS_m##a##_ptr) \
            return ((dir_##a)-4);    \
    }
*/

#  define RETURN_DIR_VALUES(a)    \
    if (s==lua_key(a)) {          \
        return (dir_##a);         \
    } else if (!absolute_only)  { \
        if (s==lua_key_plus(a))        \
            return (dir_##a);     \
        else if (s==lua_key_minus(a))   \
            return ((dir_##a)-4); \
    }

#  define is_mirrored(a) 0

#  define is_rotated(a) (a == dir_RTT)

/*

    #  define textdir_parallel(a,b) ( \
        ((a == dir_TLT || a == dir_TRT) && (b == dir_TLT || b == dir_TRT)) || \
        ((a == dir_LTL || a == dir_RTT) && (b == dir_LTL || b == dir_RTT)) \
    )

    #  define pardir_parallel(a,b) ( \
        ((a == dir_TLT || a == dir_TRT) && (b == dir_TLT || b == dir_TRT)) || \
        ((a == dir_LTL || a == dir_RTT) && (b == dir_LTL || b == dir_RTT)) \
    )

    #  define pardir_eq(a,b) ( \
        ((a == dir_TLT || a == dir_TRT) && (b == dir_TLT || b == dir_TRT)) || \
        (a == dir_LTL && b == dir_LTL) || \
        (a == dir_RTT && b == dir_RTT) \
    )

    #  define textdir_eq(a,b) ( \
        (a == dir_TLT && b == dir_TLT) || \
        (a == dir_TRT && b == dir_TRT) || \
        (a == dir_LTL && (b == dir_LTL || b == dir_RTT)) || \
        (a == dir_RTT && (b == dir_LTL || b == dir_RTT))

    #  define partextdir_eq(a,b) ( \
        ((a == dir_TLT || a == dir_TRT) && (b == dir_LTL || b == dir_RTT)) || \
        (a == dir_LTL && b == dir_TLT) || \
        (a == dir_RTT && b == dir_TRT) \
    )

*/

/* # define dir_TLT_or_TRT(a) (a == dir_TLT || a == dir_TRT) */
/* # define dir_LTL_or_RTT(a) (a == dir_LTL || a == dir_RTT) */

# define dir_TLT_or_TRT(a) (a < 2)
# define dir_LTL_or_RTT(a) (a > 1)

#  define textdir_parallel(a,b) ( \
    (dir_TLT_or_TRT(a) && dir_TLT_or_TRT(b)) || \
    (dir_LTL_or_RTT(a) && dir_LTL_or_RTT(b)) \
)

#  define pardir_parallel(a,b) ( \
    (dir_TLT_or_TRT(a) && dir_TLT_or_TRT(b)) || \
    (dir_LTL_or_RTT(a) && dir_LTL_or_RTT(b)) \
)

#  define pardir_opposite(a,b) ( \
    (a == dir_LTL && b == dir_RTT) || \
    (a == dir_RTT && b == dir_LTL) \
)

#  define textdir_opposite(a,b) ( \
    (a == dir_TLT && b == dir_TRT) || \
    (a == dir_TRT && b == dir_TLT) \
)

#  define glyphdir_opposite(a,b) 0

#  define pardir_eq(a,b) ( \
    (dir_TLT_or_TRT(a) && dir_TLT_or_TRT(b)) || \
    (a == dir_LTL && b == dir_LTL) || \
    (a == dir_RTT && b == dir_RTT) \
)

#  define textdir_eq(a,b) ( \
    (a == dir_TLT && b == dir_TLT) || \
    (a == dir_TRT && b == dir_TRT) || \
    (a == dir_LTL && dir_LTL_or_RTT(b)) || \
    (a == dir_RTT && dir_LTL_or_RTT(b)) \
)

#  define glyphdir_eq(a,b) ( \
    (a != dir_LTL && b != dir_LTL) || \
    (a == dir_LTL && b == dir_LTL) \
)

#  define partextdir_eq(a,b) ( \
    (dir_TLT_or_TRT(a) && dir_LTL_or_RTT(b)) || \
    (a == dir_LTL && b == dir_TLT) || \
    (a == dir_RTT && b == dir_TRT) \
)

#  define textglyphdir_orthogonal(a) (a != dir_RTT)

#  define textdir_is_L(a) (a == dir_TLT)

#  define push_dir(p,a) { \
    halfword dir_tmp=new_dir((a)); \
    vlink(dir_tmp)=p; \
    p=dir_tmp; \
}

#  define push_dir_node(p,a) { \
    halfword dir_tmp=copy_node((a)); \
    vlink(dir_tmp)=p; \
    p=dir_tmp; \
}

#  define pop_dir_node(p) { \
    halfword dir_tmp=p; \
    p=vlink(dir_tmp); \
    flush_node(dir_tmp); \
}

extern halfword dir_ptr;

extern halfword text_dir_ptr;

extern void initialize_directions(void);
extern halfword new_dir(int s);

extern const char *string_dir(int d);
extern void print_dir(int d);

extern void scan_direction(void);

extern halfword do_push_dir_node(halfword p, halfword a);
extern halfword do_pop_dir_node(halfword p);

scaled pack_width(int curdir, int pdir, halfword p, boolean isglyph);
scaled_whd pack_width_height_depth(int curdir, int pdir, halfword p, boolean isglyph);

void update_text_dir_ptr(int val);

#endif
