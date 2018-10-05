/* routines.c: Generating the finite state automaton.

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
#include <miktex/KPSE/Emulation>
#else
#include <kpathsea/config.h>
#include <kpathsea/types.h>
#include <kpathsea/c-proto.h>
#include <kpathsea/c-std.h>
#endif
#include "routines.h"
#include "otp.h"

#define out_ptr (states[cur_state].length)
#define out_array (states[cur_state].instrs)

list left_false_holes;

int left_state;
int right_offset;
int input_bytes;
int output_bytes;

int no_states = 0;
int cur_state = 0;
int room_for_states = 0;
state_type states[100];

int no_tables = 0;
int cur_table = 0;
int room_for_tables = 0;
table_type tables[100];

int no_aliases = 0;
alias_pair aliases[1000];

list
cons P2C(int, x, list, L)
{
list temp;
temp = (list) malloc(sizeof(cell));
temp->val = x;
temp->ptr = L;
return temp;
}

list
list1 P1C(int, x)
{
list temp;
temp = (list) malloc(sizeof(cell));
temp->val = x;
temp->ptr = nil;
return temp;
}

list
list2 P2C(int, x, int, y)
{
list temp, temp1;
temp = (list) malloc(sizeof(cell));
temp1 = (list) malloc(sizeof(cell));
temp->val = x;
temp->ptr = temp1;
temp1->val = y;
temp1->ptr = nil;
return temp;
}

list
append P2C(list, K, list, L)
{
if (K==nil) return L;
return cons(K->val, append(K->ptr, L));
}

list
append1 P2C(list, L, int, x)
{
return (append(L,list1(x)));
}

llist
lcons P2C(left, x, llist, L)
{
llist temp;
temp = (llist) malloc(sizeof(lcell));
temp->val = x;
temp->ptr = L;
return temp;
}

llist
llist1 P1C(left, x)
{
llist temp;
temp = (llist) malloc(sizeof(lcell));
temp->val = x;
temp->ptr = nil;
return temp;
}

llist
llist2 P2C(left, x, left, y)
{
llist temp, temp1;
temp = (llist) malloc(sizeof(lcell));
temp1 = (llist) malloc(sizeof(lcell));
temp->val = x;
temp->ptr = temp1;
temp1->val = y;
temp1->ptr = nil;
return temp;
}

llist
lappend P2C(llist, K, llist, L)
{
if (K==nil) return L;
return lcons(K->val, lappend(K->ptr, L));
}

llist
lappend1 P2C(llist, L, left, x)
{
return (lappend(L,llist1(x)));
}

left
WildCard P1H(void)
{
left temp;
temp = (left) malloc(sizeof(lft_cell));
temp->kind=WILDCARD;
return temp;
}

left
StringLeft P1C(char *, x)
{
left temp;
temp = (left) malloc(sizeof(lft_cell));
temp->kind=STRINGLEFT;
temp->valstr=x;
return temp;
}

left
SingleLeft P1C(int, x)
{
left temp;
temp = (left) malloc(sizeof(lft_cell));
temp->kind=SINGLELEFT;
temp->val1=x;
return temp;
}

left
DoubleLeft P2C(int, x, int, y)
{
left temp;
temp = (left) malloc(sizeof(lft_cell));
temp->kind=DOUBLELEFT;
temp->val1=x;
temp->val2=y;
return temp;
}

left
ChoiceLeft P1C(llist, L)
{
left temp;
temp = (left) malloc(sizeof(lft_cell));
temp->kind=CHOICELEFT;
temp->more_lefts=L;
return temp;
}

left
NotChoiceLeft P1C(llist, L)
{
left temp;
temp = (left) malloc(sizeof(lft_cell));
temp->kind=NOTCHOICELEFT;
temp->more_lefts=L;
return temp;
}

left
PlusLeft P2C(left, l, int, n)
{
left temp;
temp = (left) malloc(sizeof(lft_cell));
temp->kind=PLUSLEFT;
temp->one_left=l;
temp->val1=n;
return temp;
}

left
CompleteLeft P3C(left, l, int, n, int, m)
{
left temp;
temp = (left) malloc(sizeof(lft_cell));
temp->kind=COMPLETELEFT;
temp->one_left=l;
temp->val1=n;
temp->val2=m;
return temp;
}

left
BeginningLeft P1H(void)
{
left temp;
temp = (left) malloc(sizeof(lft_cell));
temp->kind=BEGINNINGLEFT;
return temp;
}

left
EndLeft P1H(void)
{
left temp;
temp = (left) malloc(sizeof(lft_cell));
temp->kind=ENDLEFT;
return temp;
}

list
gen_left P1C(left, arg)
{
int save_ptr, k;
list holes, false_holes, true_holes, backup_holes;
char *runner;
llist p;

switch(arg->kind) {
case WILDCARD:
	return nil;
case STRINGLEFT:
        runner=arg->valstr;
        holes=nil;
	while (*runner) {
		out_int(OTP_GOTO_NE, *runner);
		out_int(0, 0);
		holes=cons(out_ptr-1,holes);
		runner++;
		if (*runner) {
			out_int(OTP_GOTO_NO_ADVANCE, 0);
			holes = cons(out_ptr-1, holes);
		}
	}
	return holes;
case SINGLELEFT:
	out_int(OTP_GOTO_NE, arg->val1);
	out_int(0, 0);
	return list1(out_ptr-1);
case DOUBLELEFT:
	out_int(OTP_GOTO_LT, arg->val1);
	out_int(0, 0);
	save_ptr = out_ptr;
	out_int(OTP_GOTO_GT, arg->val2);
	out_int(0, 0);
	return list2(save_ptr-1, out_ptr-1);
case CHOICELEFT:
	true_holes=nil;
	false_holes=nil;
	p=arg->more_lefts;
	while (p!=nil) {
		false_holes = gen_left(p->val);
		if (p->ptr) {
			out_int(OTP_GOTO, 0);
			true_holes=cons(out_ptr-1, true_holes);
			fill_in(false_holes);
		}
		p=p->ptr;
	}
	fill_in(true_holes);
	return false_holes;
case NOTCHOICELEFT:
	true_holes=nil;
	p=arg->more_lefts;
	while (p!=nil) {
		false_holes = gen_left(p->val);
		out_int(OTP_GOTO, 0);
		true_holes=cons(out_ptr-1, true_holes);
		fill_in(false_holes);
		p=p->ptr;
	}
	return true_holes;
case PLUSLEFT:
	false_holes=nil;
	true_holes=nil;
	backup_holes=nil;
	k=1;
	while (k<arg->val1) {
		holes = gen_left(arg->one_left);
		false_holes = append(false_holes, holes);
		out_int(OTP_GOTO_NO_ADVANCE, 0);
		false_holes = cons(out_ptr-1, false_holes);
		k++;
	}
	holes=gen_left(arg->one_left);
	false_holes = append(false_holes, holes);
	save_ptr = out_ptr;
	out_int(OTP_GOTO_NO_ADVANCE, 0);
	true_holes=cons(out_ptr-1, true_holes);
	backup_holes=gen_left(arg->one_left);
	out_int(OTP_GOTO, save_ptr);
	fill_in(backup_holes);
	out_int(OTP_LEFT_BACKUP, 0);
	fill_in(true_holes);
	return false_holes;
case COMPLETELEFT:
	false_holes=nil;
	true_holes=nil;
	backup_holes=nil;
	k=1;
	if (arg->val1 > arg->val2) {
		return nil;
	}
	while (k<=arg->val1) {
		holes = gen_left(arg->one_left);
		false_holes = append(false_holes, holes);
		out_int(OTP_GOTO_NO_ADVANCE, 0);
		false_holes = cons(out_ptr-1, false_holes);
		k++;
	}
	while (k<arg->val2) {
		holes = gen_left(arg->one_left);
		true_holes = append(true_holes, holes);
		out_int(OTP_GOTO_NO_ADVANCE, 0);
		backup_holes = cons(out_ptr-1, backup_holes);
		k++;
	}
	holes = gen_left(arg->one_left);
	true_holes = append(true_holes, holes);
	out_int(OTP_GOTO, out_ptr+2);
	fill_in(true_holes);
	out_int(OTP_LEFT_BACKUP, 0);
	fill_in(backup_holes);
	return false_holes;
case BEGINNINGLEFT:
	out_int(OTP_GOTO_BEG, 0);
	true_holes=list1(out_ptr-1);
	out_int(OTP_GOTO, 0);
	false_holes=list1(out_ptr-1);
	fill_in(true_holes);
	return false_holes;
case ENDLEFT:
	out_int(OTP_GOTO_END, 0);
	true_holes=list1(out_ptr-1);
	out_int(OTP_GOTO, 0);
	false_holes=list1(out_ptr-1);
	fill_in(true_holes);
	return false_holes;
default:
	fprintf(stderr, "Unrecognized left: %d\n", arg->kind);
	exit(EXIT_FAILURE);
}
}

void
store_alias P2C(string, str, left, l)
{
int i;
for (i=0; i<no_aliases; i++) {
    if (!strcmp(str,aliases[i].str)) {
        fprintf(stderr, "alias %s already defined\n", str);
        exit(1);
    }
}
aliases[i].str=xstrdup(str);
aliases[i].left_val=l;
no_aliases++;
}

left
lookup_alias P1C(string, str)
{
int i;
for (i=0; i<no_aliases; i++) {
    if (!strcmp(str,aliases[i].str)) {
        return aliases[i].left_val;
    }
}
fprintf(stderr, "state %s not defined\n", str);
exit(EXIT_FAILURE);
}

void
out_left P1C(llist, L)
{
llist p;
list holes;
if ((states[cur_state].no_exprs)==1) {
	out_int(OTP_LEFT_START, 0);
} else {
	out_int(OTP_LEFT_RETURN, 0);
}
p=L;
left_false_holes=nil;
while (p!=nil) {
	holes = gen_left(p->val);
	if ((p->ptr != nil) &&
            ((p->val)->kind !=BEGINNINGLEFT) &&
            (((p->ptr)->val)->kind !=ENDLEFT)) {
		out_int(OTP_GOTO_NO_ADVANCE, 0);
		left_false_holes = cons(out_ptr-1,left_false_holes);
	}
	left_false_holes = append(left_false_holes, holes);
	p=p->ptr;
}
}

void
fill_in_left P1H(void)
{
	out_int(OTP_STOP, 0);
	fill_in(left_false_holes);
}

void
fill_in P1C(list, L) 
{
list p;
p=L;
while (p!=0) {
    out_array[p->val] = out_array[p->val] + out_ptr;
    p=p->ptr;
}
}

void
out_right P2C(int, instr, int, val)
{
out_int(instr+right_offset, val);
}

void
out_int P2C(int, instr, int, val)
{
if (val>=(1<<24)) {
    fprintf(stderr, "Argument (%d) of instruction too big\n", val);
    exit(EXIT_FAILURE);
}
add_to_state((instr<<24)+val);
}

void
store_state P1C(string, str)
{
int i;
for (i=0; i<no_states; i++) {
   if (!strcmp(str,states[i].str)) {
      fprintf(stderr, "state %s already defined\n", str);
       exit(EXIT_FAILURE);
   }
}
states[i].str=xstrdup(str);
states[i].length=0;
states[i].no_exprs=0;
cur_state=i;
no_states++;
}

int
lookup_state P1C(string, str)
{
int i;
for (i=0; i<no_states; i++) {
    if (!strcmp(str,states[i].str)) {
        return i;
    }
}
fprintf(stderr, "state %s not defined\n", str);
exit(EXIT_FAILURE);
}

void
add_to_state P1C(int, x)
{
int len;
len = states[cur_state].length;
(states[cur_state].instrs)[len] = x;
states[cur_state].length = len+1;
}

void
store_table P2C(string, str, int, len)
{
int i;
for (i=0; i<no_tables; i++) {
    if (!strcmp(str,tables[i].str)) {
        fprintf(stderr, "table %s already defined\n", str);
        exit(EXIT_FAILURE);
    }
}
tables[i].str=xstrdup(str);
tables[i].length=0;
cur_table=i;
no_tables++;
}

void
add_to_table P1C(int, x)
{
int len;
len = tables[cur_table].length;
(tables[cur_table].table)[len] = x;
tables[cur_table].length = len+1;
}

int
lookup_table P1C(string, str)
{
int i;
for (i=0; i<no_tables; i++) {
    if (!strcmp(str,tables[i].str)) {
        return i;
    }
}
fprintf(stderr, "table %s not defined\n", str);
exit(EXIT_FAILURE);
}

