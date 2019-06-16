/* conditional.h

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

#ifndef CONDITIONAL_H
#  define CONDITIONAL_H

#  define unless_code 32        /* amount added for `\.{\\unless}' prefix */

typedef enum {
    if_char_code       =  0, /* \.{\\if}          */
    if_cat_code        =  1, /* \.{\\ifcat}       */
    if_int_code        =  2, /* \.{\\ifnum}       */
    if_dim_code        =  3, /* \.{\\ifdim}       */
    if_odd_code        =  4, /* \.{\\ifodd}       */
    if_vmode_code      =  5, /* \.{\\ifvmode}     */
    if_hmode_code      =  6, /* \.{\\ifhmode}     */
    if_mmode_code      =  7, /* \.{\\ifmmode}     */
    if_inner_code      =  8, /* \.{\\ifinner}     */
    if_void_code       =  9, /* \.{\\ifvoid}      */
    if_hbox_code       = 10, /* \.{\\ifhbox}      */
    if_vbox_code       = 11, /* \.{\\ifvbox}      */
    if_x_code          = 12, /* \.{\\ifx}         */
    if_eof_code        = 13, /* \.{\\ifeof}       */
    if_true_code       = 14, /* \.{\\iftrue}      */
    if_false_code      = 15, /* \.{\\iffalse}     */
    if_case_code       = 16, /* \.{\\ifcase}      */
    if_def_code        = 17, /* \.{\\ifdefined}   */
    if_cs_code         = 18, /* \.{\\ifcsname}    */
    if_font_char_code  = 19, /* \.{\\iffontchar}  */
    if_in_csname_code  = 20, /* \.{\\ifincsname}  */
    if_primitive_code  = 21, /* \.{\\ifprimitive} */
    if_abs_num_code    = 22, /* \.{\\ifabsnum}    */
    if_abs_dim_code    = 23, /* \.{\\ifabsdim}    */
    if_condition_code  = 24, /* \.{\\ifcondition} */
} if_type_codes;

#  define if_limit_subtype(A) subtype((A)+1)
#  define if_limit_type(A) type((A)+1)
#  define if_line_field(A) vlink((A)+1)

typedef enum {
    if_code   = 1, /* \.{\\if...} */
    fi_code   = 2, /* \.{\\fi}    */
    else_code = 3, /* \.{\\else}  */
    or_code   = 4, /* \.{\\or}    */
} else_type_codes;

extern halfword cond_ptr; /* top of the condition stack */
extern int if_limit;      /* upper bound on |fi_or_else| codes */
extern int cur_if;        /* type of conditional being worked on */
extern int if_line;       /* line where that conditional began */
extern int skip_line;     /* skipping began here */

extern void pass_text(void);
extern void push_condition_stack(void);
extern void pop_condition_stack(void);
extern void change_if_limit(int l, halfword p);

extern void conditional(void);

#endif
