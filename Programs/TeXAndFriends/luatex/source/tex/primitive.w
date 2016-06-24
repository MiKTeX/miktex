% primitive.w
%
% Copyright 2008-2010 Taco Hoekwater <taco@@luatex.org>
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

@ Control sequences are stored and retrieved by means of a fairly standard hash
table algorithm called the method of ``coalescing lists'' (cf.\ Algorithm 6.4C
in {\sl The Art of Computer Programming\/}). Once a control sequence enters the
table, it is never removed, because there are complicated situations
involving \.{\\gdef} where the removal of a control sequence at the end of
a group would be a mistake preventable only by the introduction of a
complicated reference-count mechanism.

The actual sequence of letters forming a control sequence identifier is
stored in the |str_pool| array together with all the other strings. An
auxiliary array |hash| consists of items with two halfword fields per
word. The first of these, called |next(p)|, points to the next identifier
belonging to the same coalesced list as the identifier corresponding to~|p|;
and the other, called |text(p)|, points to the |str_start| entry for
|p|'s identifier. If position~|p| of the hash table is empty, we have
|text(p)=0|; if position |p| is either empty or the end of a coalesced
hash list, we have |next(p)=0|. An auxiliary pointer variable called
|hash_used| is maintained in such a way that all locations |p>=hash_used|
are nonempty. The global variable |cs_count| tells how many multiletter
control sequences have been defined, if statistics are being kept.

A global boolean variable called |no_new_control_sequence| is set to
|true| during the time that new hash table entries are forbidden.

@c
two_halves *hash;               /* the hash table */
halfword hash_used;             /* allocation pointer for |hash| */
int hash_extra;                 /* |hash_extra=hash| above |eqtb_size| */
halfword hash_top;              /* maximum of the hash array */
halfword hash_high;             /* pointer to next high hash location */
boolean no_new_control_sequence;        /* are new identifiers legal? */
int cs_count;                   /* total number of known identifiers */

#define hash_is_full (hash_used==hash_base)     /* test if all positions are occupied */

@ \.{\\primitive} support needs a few extra variables and definitions

@c
#define prim_base 1

@ The arrays |prim| and |prim_eqtb| are used for name -> cmd,chr lookups.

 The are  modelled after |hash| and |eqtb|, except that primitives do not
  have an |eq_level|, that field is replaced by |origin|.

@c
#define prim_next(a) prim[(a)].lhfield  /* link for coalesced lists */
#define prim_text(a) prim[(a)].rh       /* string number for control sequence name */
#define prim_is_full (prim_used==prim_base)     /* test if all positions are occupied */

#define prim_origin_field(a) (a).hh.b1
#define prim_eq_type_field(a)  (a).hh.b0
#define prim_equiv_field(a) (a).hh.rh
#define prim_origin(a) prim_origin_field(prim_eqtb[(a)])        /* level of definition */
#define prim_eq_type(a) prim_eq_type_field(prim_eqtb[(a)])      /* command code for equivalent */
#define prim_equiv(a) prim_equiv_field(prim_eqtb[(a)])  /* equivalent value */

static pointer prim_used;       /* allocation pointer for |prim| */
static two_halves prim[(prim_size + 1)];        /* the primitives table */
static memory_word prim_eqtb[(prim_size + 1)];

@ The array |prim_data| works the other way around, it is used for
   cmd,chr -> name lookups.

@c
typedef struct prim_info {
    halfword subids;            /* number of name entries */
    halfword offset;            /* offset to be used for |chr_code|s */
    str_number *names;          /* array of names */
} prim_info;

static prim_info prim_data[(last_cmd + 1)];

@ initialize the memory arrays
@c
void init_primitives(void)
{
    int k;
    memset(prim_data, 0, (sizeof(prim_info) * (last_cmd + 1)));
    memset(prim, 0, (sizeof(two_halves) * (prim_size + 1)));
    memset(prim_eqtb, 0, (sizeof(memory_word) * (prim_size + 1)));
    for (k = 0; k <= prim_size; k++)
        prim_eq_type(k) = undefined_cs_cmd;
}

void ini_init_primitives(void)
{
    prim_used = prim_size;      /* nothing is used */
}


@ The value of |hash_prime| should be roughly 85\%! of |hash_size|, and it
   should be a prime number.  The theory of hashing tells us to expect fewer
   than two table probes, on the average, when the search is successful.
   [See J.~S. Vitter, {\sl Journal of the ACM\/ \bf30} (1983), 231--258.]
   @^Vitter, Jeffrey Scott@>

@c
static halfword compute_hash(const char *j, unsigned int l,
                             halfword prime_number)
{
    int k;
    halfword h = (unsigned char) *j;
    for (k = 1; k <= (int)(l - 1); k++) {
        h = h + h + (unsigned char) *(j + k);
        while (h >= prime_number)
            h = h - prime_number;
    }
    return h;
}


@ Here is the subroutine that searches the primitive table for an identifier
@c
pointer prim_lookup(str_number s)
{
    int h;                      /* hash code */
    pointer p;                  /* index in |hash| array */
    unsigned char *j;
    unsigned l;
    if (s < STRING_OFFSET) {
        p = s;
        if ((p < 0) || (get_prim_eq_type(p) == undefined_cs_cmd)) {
            p = undefined_primitive;
        }
    } else {
        j = str_string(s);
        l = (unsigned) str_length(s);
        h = compute_hash((char *) j, l, prim_prime);
        p = h + prim_base;      /* we start searching here; note that |0<=h<hash_prime| */
        while (1) {
            if (prim_text(p) > 0)
                if (str_length(prim_text(p)) == l)
                    if (str_eq_str(prim_text(p), s))
                        goto FOUND;
            if (prim_next(p) == 0) {
                if (no_new_control_sequence) {
                    p = undefined_primitive;
                } else {
                    /* Insert a new primitive after |p|, then make |p| point to it */
                    if (prim_text(p) > 0) {
                        do {    /* search for an empty location in |prim| */
                            if (prim_is_full)
                                overflow("primitive size", prim_size);
                            decr(prim_used);
                        } while (prim_text(prim_used) != 0);
                        prim_next(p) = prim_used;
                        p = prim_used;
                    }
                    prim_text(p) = s;
                }
                goto FOUND;
            }
            p = prim_next(p);
        }
    }
  FOUND:
    return p;
}

@ how to test a csname for primitive-ness
@c
boolean is_primitive(str_number csname)
{
    int n, m;
    char *ss;
    m = prim_lookup(csname);
    ss = makecstring(csname);
    n = string_lookup(ss, str_length(csname));
    free(ss);
    return ((n != undefined_cs_cmd) &&
            (m != undefined_primitive) &&
            (eq_type(n) == prim_eq_type(m)) && (equiv(n) == prim_equiv(m)));
}


@ a few simple accessors
@c
quarterword get_prim_eq_type(int p)
{
    return prim_eq_type(p);
}

quarterword get_prim_origin(int p)
{
    return prim_origin(p);
}

halfword get_prim_equiv(int p)
{
    return prim_equiv(p);
}

str_number get_prim_text(int p)
{
    return prim_text(p);
}


@ dumping and undumping
@c
void dump_primitives(void)
{
    int p, q;
    for (p = 0; p <= prim_size; p++)
        dump_hh(prim[p]);
    for (p = 0; p <= prim_size; p++)
        dump_wd(prim_eqtb[p]);
    for (p = 0; p <= last_cmd; p++) {
        dump_int(prim_data[p].offset);
        dump_int(prim_data[p].subids);
        for (q = 0; q < prim_data[p].subids; q++) {
            dump_int(prim_data[p].names[q]);
        }
    }
}

void undump_primitives(void)
{
    int p, q;
    for (p = 0; p <= prim_size; p++)
        undump_hh(prim[p]);
    for (p = 0; p <= prim_size; p++)
        undump_wd(prim_eqtb[p]);

    for (p = 0; p <= last_cmd; p++) {
        undump_int(prim_data[p].offset);
        undump_int(prim_data[p].subids);
        if (prim_data[p].subids > 0) {
            prim_data[p].names = (str_number *)
                xmalloc((unsigned)
                        ((unsigned) prim_data[p].subids *
                         sizeof(str_number *)));
            for (q = 0; q < prim_data[p].subids; q++)
                undump_int(prim_data[p].names[q]);
        }
    }
}

@   We need to put \TeX's ``primitive'' control sequences into the hash
   table, together with their command code (which will be the |eq_type|)
   and an operand (which will be the |equiv|). The |primitive| procedure
   does this, in a way that no \TeX\ user can. The global value |cur_val|
   contains the new |eqtb| pointer after |primitive| has acted.


@  Because the definitions of the actual user-accessible name of a
   primitive can be postponed until runtime, the function |primitive_def|
   is needed that does nothing except creating the control sequence name.

@c
void primitive_def(const char *s, size_t l, quarterword c, halfword o)
{
    int nncs = no_new_control_sequence;
    no_new_control_sequence = false;
    cur_val = string_lookup(s, l);      /* this creates the |text()| string */
    no_new_control_sequence = nncs;
    eq_level(cur_val) = level_one;
    eq_type(cur_val) = c;
    equiv(cur_val) = o;
}

@ The function |store_primitive_name| sets up the bookkeeping for the
   reverse lookup. It is quite paranoid, because it is easy to mess this up
   accidentally.

   The |offset| is needed because sometimes character codes (in |o|)
   are indices into |eqtb| or are offset by a magical value to make
   sure they do not conflict with something else. We don't want the
   |prim_data[c].names| to have too many entries as it will just be
   wasted room, so |offset| is substracted from |o| because creating
   or accessing the array. The |assert(idx<=0xFFFF)| is not strictly
   needed, but it helps catch errors of this kind.

@c
static void
store_primitive_name(str_number s, quarterword c, halfword o, halfword offset)
{
    int idx;
    if (prim_data[c].offset != 0 && prim_data[c].offset != offset) {
        assert(false);
    }
    prim_data[c].offset = offset;
    idx = ((int) o - offset);
    assert(idx >= 0);
    assert(idx <= 0xFFFF);
    if (prim_data[c].subids < (idx + 1)) {
        str_number *new =
            (str_number *) xcalloc((unsigned) (idx + 1), sizeof(str_number *));
        if (prim_data[c].names != NULL) {
            assert(prim_data[c].subids);
            memcpy(new, (prim_data[c].names),
                   (unsigned) (prim_data[c].subids) * sizeof(str_number));
            free(prim_data[c].names);
        }
        prim_data[c].names = new;
        prim_data[c].subids = idx + 1;
    }
    prim_data[c].names[idx] = s;
}

@ Compared to tex82, |primitive| has two extra parameters. The |off| is an offset
   that will be passed on to |store_primitive_name|, the |cmd_origin| is the bit
   that is used to group primitives by originator.

@c
void
primitive(const char *thes, quarterword c, halfword o, halfword off,
          int cmd_origin)
{
    int prim_val;               /* needed to fill |prim_eqtb| */
    str_number ss;
    assert(o >= off);
    ss = maketexstring(thes);
    if (cmd_origin == tex_command || cmd_origin == core_command) {
        primitive_def(thes, strlen(thes), c, o);
    }
    prim_val = prim_lookup(ss);
    prim_origin(prim_val) = (quarterword) cmd_origin;
    prim_eq_type(prim_val) = c;
    prim_equiv(prim_val) = o;
    store_primitive_name(ss, c, o, off);
}



@ Here is a helper that does the actual hash insertion.

@c
static halfword insert_id(halfword p, const unsigned char *j, unsigned int l)
{
    unsigned saved_cur_length;
    unsigned saved_cur_string_size;
    unsigned char *saved_cur_string;
    const unsigned char *k;
    /* This code far from ideal: the existance of |hash_extra| changes
       all the potential (short) coalesced lists into a single (long)
       one. This will create a slowdown. */
    if (cs_text(p) > 0) {
        if (hash_high < hash_extra) {
            incr(hash_high);
            /* can't use |eqtb_top| here (perhaps because that is not finalized
               yet when called from |primitive|?) */
            cs_next(p) = hash_high + eqtb_size;
            p = cs_next(p);
        } else {
            do {
                if (hash_is_full)
                    overflow("hash size", (unsigned) (hash_size + hash_extra));
                decr(hash_used);
            } while (cs_text(hash_used) != 0);  /* search for an empty location in |hash| */
            cs_next(p) = hash_used;
            p = hash_used;
        }
    }
    saved_cur_length = cur_length;
    saved_cur_string = cur_string;
    saved_cur_string_size = cur_string_size;
    reset_cur_string();
    for (k = j; k <= j + l - 1; k++)
        append_char(*k);
    cs_text(p) = make_string();
    cur_length = saved_cur_length;
    xfree(cur_string);
    cur_string = saved_cur_string;
    cur_string_size = saved_cur_string_size;
    incr(cs_count);
    return p;
}


@ Here is the subroutine that searches the hash table for an identifier
 that matches a given string of length |l>1| appearing in |buffer[j..
 (j+l-1)]|. If the identifier is found, the corresponding hash table address
 is returned. Otherwise, if the global variable |no_new_control_sequence|
 is |true|, the dummy address |undefined_control_sequence| is returned.
 Otherwise the identifier is inserted into the hash table and its location
 is returned.

@c
pointer id_lookup(int j, int l)
{                               /* search the hash table */
    int h;                      /* hash code */
    pointer p;                  /* index in |hash| array */

    h = compute_hash((char *) (buffer + j), (unsigned) l, hash_prime);
#ifdef VERBOSE
    {
        unsigned char *todo = xmalloc(l + 2);
        strncpy(todo, (buffer + j), l);
        todo[l] = '\0';
        todo[l + 1] = '\0';
        fprintf(stdout, "id_lookup(%s)\n", todo);
        free(todo);
    }
#endif
    p = h + hash_base;          /* we start searching here; note that |0<=h<hash_prime| */
    while (1) {
        if (cs_text(p) > 0)
            if (str_length(cs_text(p)) == (unsigned) l)
                if (str_eq_buf(cs_text(p), j))
                    goto FOUND;
        if (cs_next(p) == 0) {
            if (no_new_control_sequence) {
                p = undefined_control_sequence;
            } else {
                p = insert_id(p, (buffer + j), (unsigned) l);
            }
            goto FOUND;
        }
        p = cs_next(p);
    }
  FOUND:
    return p;
}

@ Here is a similar subroutine for finding a primitive in the hash.
This one is based on a C string.

@c
pointer string_lookup(const char *s, size_t l)
{                               /* search the hash table */
    int h;                      /* hash code */
    pointer p;                  /* index in |hash| array */
    h = compute_hash(s, (unsigned) l, hash_prime);
    p = h + hash_base;          /* we start searching here; note that |0<=h<hash_prime| */
    while (1) {
        if (cs_text(p) > 0)
            if (str_eq_cstr(cs_text(p), s, l))
                goto FOUND;
        if (cs_next(p) == 0) {
            if (no_new_control_sequence) {
                p = undefined_control_sequence;
            } else {
                p = insert_id(p, (const unsigned char *) s, (unsigned) l);
            }
            goto FOUND;
        }
        p = cs_next(p);
    }
  FOUND:
    return p;
}

@ The |print_cmd_chr| routine prints a symbolic interpretation of a
   command code and its modifier. This is used in certain `\.{You can\'t}'
   error messages, and in the implementation of diagnostic routines like
   \.{\\show}.

   The body of |print_cmd_chr| use to be  a rather tedious listing of print
   commands, and most of it was essentially an inverse to the |primitive|
   routine that enters a \TeX\ primitive into |eqtb|.

   Thanks to |prim_data|, there is no need for all that tediousness. What
   is left of |primt_cnd_chr| are just the exceptions to the general rule
   that the  |cmd,chr_code| pair represents in a single primitive command.

@c
#define chr_cmd(A) do { tprint(A); print(chr_code); } while (0)

static void prim_cmd_chr(quarterword cmd, halfword chr_code)
{
    int idx = chr_code - prim_data[cmd].offset;
    if (cmd <= last_cmd &&
        idx >= 0 && idx < prim_data[cmd].subids &&
        prim_data[cmd].names != NULL && prim_data[cmd].names[idx] != 0) {
        tprint_esc("");
        print(prim_data[cmd].names[idx]);
    } else {
        /* TEX82 didn't print the |cmd,idx| information, but it may be useful */
        tprint("[unknown command code! (");
        print_int(cmd);
        tprint(", ");
        print_int(idx);
        tprint(")]");
    }
}

void print_cmd_chr(quarterword cmd, halfword chr_code)
{
    int n;                      /* temp variable */
    switch (cmd) {
    case left_brace_cmd:
        chr_cmd("begin-group character ");
        break;
    case right_brace_cmd:
        chr_cmd("end-group character ");
        break;
    case math_shift_cmd:
        chr_cmd("math shift character ");
        break;
    case mac_param_cmd:
        if (chr_code == tab_mark_cmd_code)
            tprint_esc("alignmark");
        else
            chr_cmd("macro parameter character ");
        break;
    case sup_mark_cmd:
        chr_cmd("superscript character ");
        break;
    case sub_mark_cmd:
        chr_cmd("subscript character ");
        break;
    case endv_cmd:
        tprint("end of alignment template");
        break;
    case spacer_cmd:
        chr_cmd("blank space ");
        break;
    case letter_cmd:
        chr_cmd("the letter ");
        break;
    case other_char_cmd:
        chr_cmd("the character ");
        break;
    case tab_mark_cmd:
        if (chr_code == span_code)
            tprint_esc("span");
        else if (chr_code == tab_mark_cmd_code)
            tprint_esc("aligntab");
        else
            chr_cmd("alignment tab character ");
        break;
    case if_test_cmd:
        if (chr_code >= unless_code)
            tprint_esc("unless");
        prim_cmd_chr(cmd, (chr_code % unless_code));
        break;
    case char_given_cmd:
        tprint_esc("char");
        print_hex(chr_code);
        break;
    case math_given_cmd:
        if (mathoption_int_par(c_mathoption_umathcode_meaning_code) == 1) {
            tprint_esc("Umathchar");
            show_mathcode_value(mathchar_from_integer(chr_code, tex_mathcode));
        } else {
            /* better for old macro packages that mess with meaning */
            tprint_esc("mathchar");
            show_mathcode_value_old(chr_code);
        }
        break;
    case xmath_given_cmd:
        tprint_esc("Umathchar");
        show_mathcode_value(mathchar_from_integer(chr_code, umath_mathcode));
        break;
    case set_font_cmd:
        tprint("select font ");
        tprint(font_name(chr_code));
        if (font_size(chr_code) != font_dsize(chr_code)) {
            tprint(" at ");
            print_scaled(font_size(chr_code));
            tprint("pt");
        }
        break;
    case undefined_cs_cmd:
        tprint("undefined");
        break;
    case call_cmd:
    case long_call_cmd:
    case outer_call_cmd:
    case long_outer_call_cmd:
        n = cmd - call_cmd;
        if (token_info(token_link(chr_code)) == protected_token)
            n = n + 4;
        if (odd(n / 4))
            tprint_esc("protected");
        if (odd(n))
            tprint_esc("long");
        if (odd(n / 2))
            tprint_esc("outer");
        if (n > 0)
            tprint(" ");
        tprint("macro");
        break;
    case assign_glue_cmd:
    case assign_mu_glue_cmd:
        if (chr_code < skip_base) {
            prim_cmd_chr(cmd, chr_code);
        } else if (chr_code < mu_skip_base) {
            tprint_esc("skip");
            print_int(chr_code - skip_base);
        } else {
            tprint_esc("muskip");
            print_int(chr_code - mu_skip_base);
        }
        break;
    case assign_toks_cmd:
        if (chr_code >= toks_base) {
            tprint_esc("toks");
            print_int(chr_code - toks_base);
        } else {
            prim_cmd_chr(cmd, chr_code);
        }
        break;
    case assign_int_cmd:
        if (chr_code < count_base) {
            prim_cmd_chr(cmd, chr_code);
        } else {
            tprint_esc("count");
            print_int(chr_code - count_base);
        }
        break;
    case assign_attr_cmd:
        tprint_esc("attribute");
        print_int(chr_code - attribute_base);
        break;
    case assign_dimen_cmd:
        if (chr_code < scaled_base) {
            prim_cmd_chr(cmd, chr_code);
        } else {
            tprint_esc("dimen");
            print_int(chr_code - scaled_base);
        }
        break;
    case normal_cmd:
        if (chr_code < prim_data[cmd].subids && prim_data[cmd].names[chr_code] != 0) {
            prim_cmd_chr(cmd, chr_code);
        } else {
            tprint("[unknown command! (");
            print_int(chr_code);
            tprint(")]");
        }
        break;
    case extension_cmd:
        if (chr_code < prim_data[cmd].subids && prim_data[cmd].names[chr_code] != 0) {
            prim_cmd_chr(cmd, chr_code);
        } else {
            tprint("[unknown extension! (");
            print_int(chr_code);
            tprint(")]");

        }
        break;
    default:
        /* these are most commands, actually */
        prim_cmd_chr(cmd, chr_code);
        break;
    }
}
