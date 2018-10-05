/* routines.h: Generating the finite state automaton

This file is part of the Omega project, which
is based in the web2c distribution of TeX.

Copyright (c) 1994--2000 John Plaice and Yannis Haralambous

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#if defined(MIKTEX)
#define P1H(x1) (x1)
#define P2H(p1,p2) (p1, p2)
#define P3H(p1,p2,p3) (p1, p2, p3)
#define P4H(p1,p2,p3,p4) (p1, p2, p3, p4)
#define P5H(p1,p2,p3,p4,p5) (p1, p2, p3, p4, p5)
#define P6H(p1,p2,p3,p4,p5,p6) (p1, p2, p3, p4, p5, p6)
#define P7H(p1,p2,p3,p4,p5,p6,p7) (p1, p2, p3, p4, p5, p6, p7)
#define P8H(p1,p2,p3,p4,p5,p6,p7,p8) (p1, p2, p3, p4, p5, p6, p7, p8)
#define P9H(p1,p2,p3,p4,p5,p6,p7,p8,p9) (p1, p2, p3, p4, p5, p6, p7, p8, p9)
#define P1C(t1, v1) (t1 v1)
#define P2C(t1, v1, t2, v2) (t1 v1, t2 v2)
#define P3C(t1, v1, t2, v2, t3, v3) (t1 v1, t2 v2, t3 v3)
#define P4C(t1, v1, t2, v2, t3, v3, t4, v4) (t1 v1, t2 v2, t3 v3, t4 v4)
#define P5C(t1, v1, t2, v2, t3, v3, t4, v4, t5, v5) \
  (t1 v1, t2 v2, t3 v3, t4 v4, t5 v5)
#define P6C(t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, t6, v6)	\
  (t1 v1, t2 v2, t3 v3, t4 v4, t5 v5, t6 v6)
#define P7C(t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, t6, v6, t7, v7)	\
  (t1 v1, t2 v2, t3 v3, t4 v4, t5 v5, t6 v6, t7 v7)
#define P8C(t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, t6, v6, t7, v7, t8, v8) \
  (t1 v1, t2 v2, t3 v3, t4 v4, t5 v5, t6 v6, t7 v7, t8 v8)
#define P9C(t1,v1, t2,v2, t3,v3, t4,v4, t5,v5, t6,v6, t7,v7, t8,v8, t9,v9) \
  (t1 v1, t2 v2, t3 v3, t4 v4, t5 v5, t6 v6, t7 v7, t8 v8, t9 v9)
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef KPATHSEA
#include <kpathsea/config.h>
#include <kpathsea/c-memstr.h>
#include <kpathsea/c-std.h>
#else /* !KPATHSEA */
#include <stdio.h>
#include <string.h>
#ifdef __STDC__
#include <stdlib.h>
#else
extern void exit();
#endif
#endif /* KPATHSEA */

#define nil 0

#define WILDCARD 0
#define STRINGLEFT 1
#define SINGLELEFT 2
#define DOUBLELEFT 3
#define CHOICELEFT 4
#define NOTCHOICELEFT 5
#define PLUSLEFT 6
#define COMPLETELEFT 7
#define BEGINNINGLEFT 8
#define ENDLEFT 9

extern int yyparse();
extern int yylex();

extern int line_number;
typedef struct cell_struct {struct cell_struct *ptr;
        int val; } cell;
typedef cell *list;

typedef struct left_cell {
	int kind;
	int val1, val2;
	char *valstr;
        struct lcell_struct *more_lefts;
	struct left_cell *one_left;
} lft_cell;
typedef lft_cell *left;

typedef struct lcell_struct {struct lcell_struct *ptr;
        left val; } lcell;
typedef lcell *llist;

extern list cons();
extern list list1();
extern list list2();
extern list append();
extern list append1();

extern llist lcons();
extern llist llist1();
extern llist llist2();
extern llist lappend();
extern llist lappend1();

extern left WildCard();
extern left SingleLeft();
extern left StringLeft();
extern left DoubleLeft();
extern left ChoiceLeft();
extern left NotChoiceLeft();
extern left PlusLeft();
extern left CompleteLeft();
extern left BeginningLeft();
extern left EndLeft();
extern list gen_left();
extern void fill_in_left();
extern void out_left();
extern int no_lefts;

extern void store_alias();
extern left lookup_alias();

typedef struct {
	int length;
	char * str;
	int table[10000];
} table_type;

extern int no_tables;
extern int cur_table;
extern int room_for_tables;
extern table_type tables[];
extern void add_to_table();

typedef struct {
	int length;
	char * str;
	int no_exprs;
	int instrs[10000];
} state_type;

extern int no_states;
extern int cur_state;
extern int room_for_states;
extern state_type states[];
extern void add_to_state();

extern void fill_in();
extern void right_int();
extern void out_int();
extern void out_right();

extern void store_state();
extern int lookup_state();
extern void store_table();
extern int lookup_table();

typedef struct { char * str; left left_val; } alias_pair;

extern alias_pair aliases[];

extern int right_offset;
extern int input_bytes;
extern int output_bytes;

