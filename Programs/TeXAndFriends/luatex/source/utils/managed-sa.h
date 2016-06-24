/* managed-sa.h

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
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>. */


#ifndef MANAGED_SA_H
#  define MANAGED_SA_H 1

/* the next two sets of three had better match up exactly, but using bare numbers
  is easier on the C compiler */

#  define HIGHPART 128
#  define MIDPART 128
#  define LOWPART 128

#  define HIGHPART_PART(a) (((a)>>14)&127)
#  define MIDPART_PART(a)  (((a)>>7)&127)
#  define LOWPART_PART(a)  ((a)&127)

#  define Mxmalloc_array(a,b)  xmalloc((unsigned)((unsigned)(b)*sizeof(a)))
#  define Mxcalloc_array(a,b)  xcalloc((b),sizeof(a))
#  define Mxrealloc_array(a,b,c)  xrealloc((a),(unsigned)((unsigned)(c)*sizeof(b)))

typedef union {
    unsigned int uint_value;
    int int_value;
    struct {
        int value_1;
        int value_2;
    } dump_int;
    struct {
        unsigned int value_1;
        unsigned int value_2;
    } dump_uint;
    struct {
        unsigned int character_value:21;
        unsigned int family_value:8;
        unsigned int class_value:3;
    } math_code_value;
    struct {
        unsigned int small_character_value:21;
        unsigned int small_family_value:8;
        unsigned int class_value:3;
        unsigned int large_character_value:21;
        unsigned int large_family_value:8;
        unsigned int dummy_value:3;
    } del_code_value;
} sa_tree_item;

typedef struct {
    int code;
    int level;
    sa_tree_item value;
} sa_stack_item;


typedef struct {
    int stack_size;             /* initial stack size   */
    int stack_step;             /* increment stack step */
    int stack_type;
    int stack_ptr;              /* current stack point  */
    sa_tree_item ***tree;       /* item tree head       */
    sa_stack_item *stack;       /* stack tree head      */
    sa_tree_item dflt;          /* default item value   */
} sa_tree_head;

typedef sa_tree_head *sa_tree;

extern sa_tree_item get_sa_item(const sa_tree head, const int n);
extern void set_sa_item(sa_tree head, int n, sa_tree_item v, int gl);
extern void rawset_sa_item(sa_tree head, int n, sa_tree_item v);

extern sa_tree new_sa_tree(int size, int type, sa_tree_item dflt);

extern sa_tree copy_sa_tree(sa_tree head);
extern void destroy_sa_tree(sa_tree head);

extern void dump_sa_tree(sa_tree a, const char * name);
extern sa_tree undump_sa_tree(const char * name);

extern void restore_sa_stack(sa_tree a, int gl);
extern void clear_sa_stack(sa_tree a);

#endif
