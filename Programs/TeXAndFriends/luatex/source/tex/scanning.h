/* scanning.h

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
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

#ifndef SCANNING_H
#  define SCANNING_H

typedef enum {
    int_val_level = 0,          /* integer values */
    attr_val_level,             /* integer values */
    dimen_val_level,            /* dimension values */
    glue_val_level,             /* glue specifications */
    mu_val_level,               /* math glue specifications */
    dir_val_level,              /* directions */
    ident_val_level,            /* font identifier */
    tok_val_level,              /* token lists */
} value_level_code;

extern void scan_left_brace(void);
extern void scan_optional_equals(void);

extern int cur_val;             /* value returned by numeric scanners */
extern int cur_val1;            /* delcodes are sometimes 51 digits */
extern int cur_val_level;       /* the ``level'' of this value */

extern void scan_something_simple(halfword cmd, halfword subitem);
extern void scan_something_internal(int level, boolean negative);

extern void scan_limited_int(int max, const char *name);

extern void negate_cur_val(boolean delete_glue);

#  define scan_register_num() scan_limited_int(65535,"register code")
#  define scan_mark_num() scan_limited_int(65535,"marks code")
#  define scan_char_num() scan_limited_int(biggest_char,"character code")
#  define scan_four_bit_int() scan_limited_int(15,NULL)
#  define scan_math_family_int() scan_limited_int(255,"math family")
#  define scan_real_fifteen_bit_int() scan_limited_int(32767,"mathchar")
#  define scan_big_fifteen_bit_int() scan_limited_int(0x7FFFFFF,"extended mathchar")
#  define scan_twenty_seven_bit_int() scan_limited_int(0777777777,"delimiter code")

extern void scan_fifteen_bit_int(void);
extern void scan_fifty_one_bit_int(void);

#  define octal_token (other_token+'\'')            /* apostrophe, indicates an octal constant */
#  define hex_token (other_token+'"')               /* double quote, indicates a hex constant */
#  define alpha_token (other_token+'`')             /* reverse apostrophe, precedes alpha constants */
#  define point_token (other_token+'.')             /* decimal point */
#  define continental_point_token (other_token+',') /* decimal point, Eurostyle */
#  define infinity 017777777777                     /* the largest positive value that \TeX\ knows */
#  define zero_token (other_token+'0')              /* zero, the smallest digit */
#  define A_token (letter_token+'A')                /* the smallest special hex digit */
#  define other_A_token (other_token+'A')           /* special hex digit of type |other_char| */
extern int radix;

extern void scan_int(void);

extern int cur_order;

#  define scan_normal_dimen() scan_dimen(false,false,false)

extern void scan_dimen(boolean mu, boolean inf, boolean shortcut);
extern void scan_glue(int level);
extern void scan_scaled(void);

extern halfword the_toks(void);
extern str_number the_scanned_result(void);
extern void set_font_dimen(void);
extern void get_font_dimen(void);

#  define default_rule 26214    /* 0.4\thinspace pt */

extern halfword scan_rule_spec(void);

extern void scan_font_ident(void);
extern void scan_general_text(void);
extern void get_x_or_protected(void);
extern halfword scan_toks(boolean macrodef, boolean xpand);

extern void scan_normal_glue(void);
extern void scan_mu_glue(void);

/* extern int add_or_sub(int x, int y, int max_answer, boolean negative); */
/* extern int quotient(int n, int d); */
extern int fract(int x, int n, int d, int max_answer);

/* extern void scan_expr(void); */

#endif
