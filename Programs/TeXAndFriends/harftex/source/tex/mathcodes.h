/* mathcodes.h

   Copyright 2009-2012 Taco Hoekwater <taco@luatex.org>

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

#ifndef MATHCODES_H
#  define MATHCODES_H

/* this is a flag for |scan_delimiter| */

#  define no_mathcode        0
#  define tex_mathcode       8
#  define umath_mathcode    21
#  define umathnum_mathcode 22

typedef struct mathcodeval {
    int class_value;
    int family_value;
    int character_value;
} mathcodeval;

void set_math_code(int n, int mathclass, int mathfamily, int mathcharacter, quarterword gl);

mathcodeval get_math_code(int n);
int get_math_code_num(int n);
int get_del_code_num(int n);
mathcodeval scan_mathchar(int extcode);
mathcodeval scan_delimiter_as_mathchar(int extcode);

mathcodeval mathchar_from_integer(int value, int extcode);
void show_mathcode_value(mathcodeval d);
void show_mathcode_value_old(int value);

typedef struct delcodeval {
    int class_value;
    int small_family_value;
    int small_character_value;
    int large_family_value;
    int large_character_value;
} delcodeval;

void set_del_code(int n, int smathfamily, int smathcharacter, int lmathfamily, int lmathcharacter, quarterword gl);

delcodeval get_del_code(int n);

void unsave_math_codes(quarterword grouplevel);
void initialize_math_codes(void);
void dump_math_codes(void);
void undump_math_codes(void);

#endif
