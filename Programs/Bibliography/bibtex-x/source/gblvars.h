/*-
******************************************************************************
******************************************************************************
**
**  MODULE
**
**      file: gblvars.h
**
**  DESCRIPTION
**
**      A 32-bit implementation of BibTeX v0.99c for MS-DOS, OS/2 2.x, 
**      Unix and VMS.  This C language implementation is based on the 
**      original WEB source but it has been enhanced to support 8-bit input
**      characters and a very large processing capacity.
**
**      For documentation describing how to use and build this program, 
**      see the 00README.TXT file that accompanies this distribution.
**
**  MODULE CONTENTS
**
**      This module contains the declarations for all of the global
**      variables used by BibTeX.  This module must be included by all of
**      the .C modules.  The variables are declared in alphabetical order.
**
**  AUTHORS
**
**      Original WEB translation to C, conversion to "big" (32-bit) capacity,
**      addition of run-time selectable capacity and 8-bit support extensions
**      by:
**
**          Niel Kempson
**          Snowy Owl Systems Limited, Cheltenham, England
**          E-mail: kempson@snowyowl.co.uk
**      
**      8-bit support extensions also by:
**
**          Alejandro Aguilar-Sierra
**          Centro de Ciencias de la Atm\'osfera, 
**          Universidad Nacional Aut\'onoma de M\'exico, M\'exico
**          E-mail: asierra@servidor.unam.mx
**
**  COPYRIGHT
**
**      This implementation copyright (c) 1991-1995 by Niel Kempson
**           and copyright (c) 1995 by Alejandro Aguilar-Sierra.
**
**      This program is free software; you can redistribute it and/or
**      modify it under the terms of the GNU General Public License as
**      published by the Free Software Foundation; either version 1, or
**      (at your option) any later version.
**
**      This program is distributed in the hope that it will be useful,
**      but WITHOUT ANY WARRANTY; without even the implied warranty of
**      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**      General Public License for more details.
**
**      You should have received a copy of the GNU General Public License
**      along with this program; if not, write to the Free Software
**      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**      In other words, you are welcome to use, share and improve this
**      program.  You are forbidden to forbid anyone else to use, share
**      and improve what you give them.  Help stamp out software-hoarding!
**
**  ACKNOWLEDGEMENT
**      
**      The original BibTeX was written by Oren Patashnik using Donald 
**      Knuth's WEB system.  This format produces a PASCAL program for 
**      execution and a TeX documented version of the source code. This 
**      program started as a (manual) translation of the WEB source into C.
**  
**  CHANGE LOG
**
**      $Log: gblvars.h,v $
**      Revision 3.71  1996/08/18  20:47:30  kempson
**      Official release 3.71 (see HISTORY file for details).
**
**      Revision 3.70  1996/04/08  10:08:40  kempson
**      Final documentation & cosmetic changes for official release 3.70.
**
**      Revision 3.6  1995/10/21  22:18:36  kempson
**      Added flag to support --wolfgang option.
**
**      Revision 3.5  1995/09/24  20:44:37  kempson
**      Many changes for final beta test version.
**
**      Revision 3.4  1995/04/09  22:15:45  kempson
**      Placed under RCS control
**
******************************************************************************
******************************************************************************
*/
#ifndef __GBLVARS_H__
# define __GBLVARS_H__              1


/*-
**----------------------------------------------------------------------------
** If this file is included by BIBTEX.C, declare all of the variables,
** otherwise just reference them as extern.
**----------------------------------------------------------------------------
*/
#ifdef  __BIBTEX_C__
# define __EXTERN__
#else                           /* NOT __BIBTEX_C__ */
# define __EXTERN__                 extern
#endif                          /* __BIBTEX_C__ */


__EXTERN__ Boolean_T                    all_entries;
__EXTERN__ CiteNumber_T                 all_marker;
__EXTERN__ Boolean_T                    alpha_found;
__EXTERN__ Boolean_T                    and_found;
__EXTERN__ Boolean_T                    at_bib_command;
__EXTERN__ LongJumpBuf_T                Aux_Done_Flag;
__EXTERN__ Integer_T                    aux_name_length;
__EXTERN__ AuxNumber_T                  aux_ptr;

__EXTERN__ HashLoc_T                    b_add_period;
__EXTERN__ HashLoc_T                    b_bit_and;
__EXTERN__ HashLoc_T                    b_bit_or;
__EXTERN__ HashLoc_T                    b_call_type;
__EXTERN__ HashLoc_T                    b_change_case;
__EXTERN__ HashLoc_T                    b_chr_to_int;
__EXTERN__ HashLoc_T                    b_cite;
__EXTERN__ HashLoc_T                    b_concatenate;
__EXTERN__ HashLoc_T                    b_default;
__EXTERN__ HashLoc_T                    b_duplicate;
__EXTERN__ HashLoc_T                    b_empty;
__EXTERN__ HashLoc_T                    b_equals;
__EXTERN__ HashLoc_T                    b_format_name;
__EXTERN__ HashLoc_T                    b_gets;
__EXTERN__ HashLoc_T                    b_greater_than;
__EXTERN__ HashLoc_T                    b_if;
__EXTERN__ HashLoc_T                    b_int_to_chr;
__EXTERN__ HashLoc_T                    b_int_to_str;
__EXTERN__ HashLoc_T                    b_less_than;
__EXTERN__ HashLoc_T                    b_minus;
__EXTERN__ HashLoc_T                    b_missing;
__EXTERN__ HashLoc_T                    b_newline;
__EXTERN__ HashLoc_T                    b_num_names;
__EXTERN__ HashLoc_T                    b_plus;
__EXTERN__ HashLoc_T                    b_pop;
__EXTERN__ HashLoc_T                    b_preamble;
__EXTERN__ HashLoc_T                    b_purify;
__EXTERN__ HashLoc_T                    b_quote;
__EXTERN__ HashLoc_T                    b_skip;
__EXTERN__ HashLoc_T                    b_stack;
__EXTERN__ HashLoc_T                    b_substring;
__EXTERN__ HashLoc_T                    b_swap;
__EXTERN__ HashLoc_T                    b_text_length;
__EXTERN__ HashLoc_T                    b_text_prefix;
__EXTERN__ HashLoc_T                    b_top_stack;
__EXTERN__ HashLoc_T                    b_type;
__EXTERN__ HashLoc_T                    b_warning;
__EXTERN__ HashLoc_T                    b_while;
__EXTERN__ HashLoc_T                    b_width;
__EXTERN__ HashLoc_T                    b_write;
#ifdef UTF_8
__EXTERN__ HashLoc_T                    b_cjk_string;
#endif
__EXTERN__ Integer_T                    bad;
__EXTERN__ AlphaFile_T                  bbl_file;
__EXTERN__ Integer_T                    bbl_line_num;
__EXTERN__ Integer_T                    bib_brace_level;
__EXTERN__ Integer_T                    bib_line_num;
__EXTERN__ BibNumber_T                  bib_ptr;
__EXTERN__ Boolean_T                    bib_seen;
__EXTERN__ Integer_T                    brace_level;
__EXTERN__ LongJumpBuf_T                Bst_Done_Flag;
__EXTERN__ AlphaFile_T                  bst_file;
__EXTERN__ Integer_T                    bst_line_num;
__EXTERN__ Boolean_T                    bst_seen;
__EXTERN__ StrNumber_T                  bst_str;
__EXTERN__ BufPointer_T                 buf_ptr1;
__EXTERN__ BufPointer_T                 buf_ptr2;

__EXTERN__ Boolean_T                    citation_seen;
__EXTERN__ Boolean_T                    cite_found;
__EXTERN__ Boolean_T                    cite_hash_found;
__EXTERN__ HashLoc_T                    cite_loc;
__EXTERN__ CiteNumber_T                 cite_parent_ptr;
__EXTERN__ CiteNumber_T                 cite_ptr;
__EXTERN__ CiteNumber_T                 cite_xptr;
__EXTERN__ LongJumpBuf_T                Close_Up_Shop_Flag;
__EXTERN__ StrNumber_T                  cmd_str_ptr;
__EXTERN__ BufPointer_T                 comma1;
__EXTERN__ BufPointer_T                 comma2;
__EXTERN__ Integer_T                    command_num;
__EXTERN__ HashLoc_T                    control_seq_loc;
__EXTERN__ Integer8_T                   conversion_type;
__EXTERN__ FieldLoc_T                   crossref_num;
__EXTERN__ HashLoc_T                    cur_macro_loc;
__EXTERN__ BufPointer_T                 cur_token;

__EXTERN__ Boolean_T                    double_letter;
__EXTERN__ HashLoc_T                    dummy_loc;

__EXTERN__ Boolean_T                    end_of_group;
__EXTERN__ Integer_T                    ent_chr_ptr;
__EXTERN__ CiteNumber_T                 entry_cite_ptr;
__EXTERN__ Boolean_T                    entry_seen;
__EXTERN__ HashLoc_T                    entry_type_loc;
__EXTERN__ Integer_T                    err_count;
__EXTERN__ BufPointer_T                 ex_buf_length;
__EXTERN__ BufPointer_T                 ex_buf_ptr;
__EXTERN__ BufPointer_T                 ex_buf_xptr;
__EXTERN__ BufPointer_T                 ex_buf_yptr;
__EXTERN__ LongJumpBuf_T                Exit_Program_Flag;

__EXTERN__ FieldLoc_T                   field_end_ptr;
__EXTERN__ HashLoc_T                    field_name_loc;
__EXTERN__ FieldLoc_T                   field_parent_ptr;
__EXTERN__ FieldLoc_T                   field_ptr;
__EXTERN__ HashLoc_T                    field_val_loc;
__EXTERN__ BufPointer_T                 first_end;
__EXTERN__ BufPointer_T                 first_start;
__EXTERN__ HashLoc_T                    fn_loc;

__EXTERN__ Integer_T                    glob_chr_ptr;

__EXTERN__ Boolean_T                    hash_found;
__EXTERN__ Integer16_T                  hash_used;
__EXTERN__ Integer8_T                   history;

__EXTERN__ Integer_T                    impl_fn_num;
__EXTERN__ IntEntLoc_T                  int_ent_ptr;

__EXTERN__ BufPointer_T                 jr_end;

__EXTERN__ BufPointer_T                 last;
__EXTERN__ BufPointer_T                 last_end;
__EXTERN__ BufPointer_T                 last_token;
__EXTERN__ HashLoc_T                    lc_cite_loc;
__EXTERN__ HashLoc_T                    lc_xcite_loc;
__EXTERN__ HashLoc_T                    literal_loc;
__EXTERN__ LitStkLoc_T                  lit_stk_ptr;
__EXTERN__ AlphaFile_T                  log_file;

__EXTERN__ HashLoc_T                    macro_def_loc;
__EXTERN__ HashLoc_T                    macro_name_loc;
__EXTERN__ Boolean_T                    mess_with_entries;

__EXTERN__ BufPointer_T                 name_bf_ptr;
__EXTERN__ BufPointer_T                 name_bf_xptr;
__EXTERN__ BufPointer_T                 name_bf_yptr;
__EXTERN__ Integer_T                    name_length;
__EXTERN__ Integer_T                    name_ptr;
__EXTERN__ Integer_T                    nm_brace_level;
__EXTERN__ Boolean_T                    no_fields;
__EXTERN__ BibNumber_T                  num_bib_files;
__EXTERN__ CiteNumber_T                 num_cites;
__EXTERN__ BufPointer_T                 num_commas;
__EXTERN__ IntEntLoc_T                  num_ent_ints;
__EXTERN__ StrEntLoc_T                  num_ent_strs;
__EXTERN__ FieldLoc_T                   num_fields;
__EXTERN__ Integer8_T                   num_glb_strs;
__EXTERN__ Integer_T                    num_names;
__EXTERN__ BibNumber_T                  num_preamble_strings;
__EXTERN__ FieldLoc_T                   num_pre_defined_fields;
__EXTERN__ BufPointer_T                 num_text_chars;
__EXTERN__ BufPointer_T                 num_tokens;

__EXTERN__ CiteNumber_T                 old_num_cites;
__EXTERN__ BufPointer_T                 out_buf_length;
__EXTERN__ BufPointer_T                 out_buf_ptr;

__EXTERN__ PoolPointer_T                pool_ptr;
__EXTERN__ Integer_T                    pop_lit1;
__EXTERN__ Integer_T                    pop_lit2;
__EXTERN__ Integer_T                    pop_lit3;
__EXTERN__ StkType_T                    pop_typ1;
__EXTERN__ StkType_T                    pop_typ2;
__EXTERN__ StkType_T                    pop_typ3;
__EXTERN__ PoolPointer_T                p_ptr1;
__EXTERN__ PoolPointer_T                p_ptr2;
__EXTERN__ BibNumber_T                  preamble_ptr;
__EXTERN__ Boolean_T                    preceding_white;
__EXTERN__ HashLoc_T                    pre_def_loc;
__EXTERN__ Boolean_T                    prev_colon;

__EXTERN__ Boolean_T                    read_completed;
__EXTERN__ Boolean_T                    reading_completed;
__EXTERN__ Boolean_T                    read_performed;
__EXTERN__ Boolean_T                    read_seen;
__EXTERN__ ASCIICode_T                  right_outer_delim;
__EXTERN__ ASCIICode_T                  right_str_delim;

__EXTERN__ StrNumber_T                  s_aux_extension;
__EXTERN__ StrNumber_T                  s_bbl_extension;
__EXTERN__ StrNumber_T                  s_bib_extension;
__EXTERN__ StrNumber_T                  s_bst_extension;
__EXTERN__ StrNumber_T                  s_default;
__EXTERN__ StrNumber_T                  s_l;
__EXTERN__ StrNumber_T                  s_log_extension;
__EXTERN__ StrNumber_T                  s_null;
__EXTERN__ StrNumber_T                  s_t;
__EXTERN__ StrNumber_T                  s_u;
__EXTERN__ Integer8_T                   scan_result;
__EXTERN__ CiteNumber_T                 sort_cite_ptr;
__EXTERN__ StrEntLoc_T                  sort_key_num;
__EXTERN__ Integer_T                    sp_brace_level;
__EXTERN__ PoolPointer_T                sp_end;
__EXTERN__ PoolPointer_T                sp_length;
__EXTERN__ PoolPointer_T                sp_ptr;
__EXTERN__ PoolPointer_T                sp_xptr1;
__EXTERN__ PoolPointer_T                sp_xptr2;
__EXTERN__ PoolPointer_T                sp2_length;
__EXTERN__ Boolean_T                    store_entry;
__EXTERN__ Boolean_T                    store_field;
__EXTERN__ Boolean_T                    store_token;
__EXTERN__ StrEntLoc_T                  str_ent_ptr;
__EXTERN__ UChar_T                      str_glb_ptr;
__EXTERN__ Integer_T                    string_width;
__EXTERN__ StrNumber_T                  str_num;
__EXTERN__ StrNumber_T                  str_ptr;
__EXTERN__ BufPointer_T                 sv_ptr1;
__EXTERN__ BufPointer_T                 sv_ptr2;

__EXTERN__ Integer_T                    tmp_end_ptr;
__EXTERN__ Integer_T                    tmp_ptr;
__EXTERN__ Boolean_T                    to_be_written;
__EXTERN__ Boolean_T                    token_starting;
__EXTERN__ Integer_T                    token_value;
__EXTERN__ StrNumber_T                  top_lev_str;
__EXTERN__ Boolean_T                    type_exists;

__EXTERN__ Boolean_T                    use_default;

__EXTERN__ BufPointer_T                 von_end;
__EXTERN__ BufPointer_T                 von_start;

__EXTERN__ WizFnLoc_T                   wiz_def_ptr;
__EXTERN__ WizFnLoc_T                   wiz_fn_ptr;
__EXTERN__ HashLoc_T                    wiz_loc;

#ifdef STAT
__EXTERN__ HashLoc_T                    blt_in_loc[NUM_BLT_IN_FNS + 1];
__EXTERN__ BltInRange_T                 blt_in_ptr;
__EXTERN__ Integer_T                    execution_count[NUM_BLT_IN_FNS + 1];
__EXTERN__ Integer_T                    total_ex_count;
#endif                          /* STAT */


/*-
**----------------------------------------------------------------------------
** STATIC GLOBAL ARRAYS DECLARED HERE
**----------------------------------------------------------------------------
*/
__EXTERN__ AlphaFile_T                  aux_file[AUX_STACK_SIZE + 1];
__EXTERN__ StrNumber_T                  aux_list[AUX_STACK_SIZE + 1];
__EXTERN__ Integer_T                    aux_ln_stack[AUX_STACK_SIZE + 1];
__EXTERN__ Integer_T                    char_width[LAST_LATIN_CHAR + 1];
__EXTERN__ IDType_T                     id_class[LAST_ASCII_CHAR + 1];
__EXTERN__ LexType_T                    lex_class[LAST_ASCII_CHAR + 1];
__EXTERN__ unsigned char                xchr[LAST_ASCII_CHAR + 1];
__EXTERN__ ASCIICode_T                  xord[LAST_TEXT_CHAR + 1];

#ifdef SUPPORT_8BIT
/*
** Special arrays to manage character set report.
**
**  c8upcase    - contains the lowercase equivalent for each character.
**                If a character has no lowercase equivalent (e.g.
**                because it's not a letter), the value is set to that of
**                the character.
**
**  c8lowcase   - contains the uppercase equivalent for each character
**                If a character has no uppercase equivalent (e.g.
**                because it's not a letter), the value is set to that of
**                the character.
**
**  c8order     - contains the sorting weight for each character.  If a
**                character doesn't have a specified sorting weight, it
**                will be set to the default SORT_LAST to ensure that it
**                is sorted after any defined character.
*/
__EXTERN__ unsigned char                c8upcase[LAST_TEXT_CHAR + 1];
__EXTERN__ unsigned char                c8lowcase[LAST_TEXT_CHAR + 1];
__EXTERN__ int                          c8order[LAST_TEXT_CHAR + 1];
#endif                          /* SUPPORT_8BIT */


/*-
**----------------------------------------------------------------------------
** DYNAMICALLY ALLOCATED GLOBAL ARRAYS DECLARED HERE
**----------------------------------------------------------------------------
*/
__EXTERN__ AlphaFile_T                 *bib_file;
__EXTERN__ StrNumber_T                 *bib_list;
__EXTERN__ ASCIICode_T                 *buffer;
__EXTERN__ StrNumber_T                 *cite_info;
__EXTERN__ StrNumber_T                 *cite_list;
__EXTERN__ Boolean_T                   *entry_exists;
__EXTERN__ Integer_T                   *entry_ints;
__EXTERN__ ASCIICode_T                 *entry_strs;
__EXTERN__ ASCIICode_T                 *ex_buf;
__EXTERN__ StrNumber_T                 *field_info;
__EXTERN__ FnClass_T                   *fn_type;
__EXTERN__ Integer_T                   *glb_str_end;
__EXTERN__ StrNumber_T                 *glb_str_ptr;
__EXTERN__ ASCIICode_T                 *global_strs;
__EXTERN__ StrIlk_T                    *hash_ilk;
__EXTERN__ HashPointer_T               *hash_next;
__EXTERN__ StrNumber_T                 *hash_text;
__EXTERN__ Integer_T                   *ilk_info;
__EXTERN__ Integer_T                   *lit_stack;
__EXTERN__ StkType_T                   *lit_stk_type;
__EXTERN__ unsigned char               *name_of_file;
__EXTERN__ ASCIICode_T                 *name_sep_char;
__EXTERN__ BufPointer_T                *name_tok;
__EXTERN__ ASCIICode_T                 *out_buf;
__EXTERN__ StrNumber_T                 *s_preamble;
__EXTERN__ ASCIICode_T                 *str_pool;
__EXTERN__ PoolPointer_T               *str_start;
__EXTERN__ ASCIICode_T                 *sv_buffer;
__EXTERN__ HashPtr2_T                  *type_list;
__EXTERN__ HashPtr2_T                  *wiz_functions;


/*-
**----------------------------------------------------------------------------
** Variables used to record the results of the command line parsing.
**----------------------------------------------------------------------------
*/
#ifdef UTF_8
__EXTERN__ Boolean_T                    Flag_language;
__EXTERN__ char                        *Str_language;
__EXTERN__ Boolean_T                    Flag_location;
__EXTERN__ char                        *Str_location;
__EXTERN__ UCollator                   *u_coll;
#endif
__EXTERN__ Boolean_T                    Flag_7bit;
__EXTERN__ Boolean_T                    Flag_8bit;
__EXTERN__ Boolean_T                    Flag_8bit_alpha;
__EXTERN__ Boolean_T                    Flag_big;
__EXTERN__ Boolean_T                    Flag_debug;
__EXTERN__ Boolean_T                    Flag_huge;
__EXTERN__ Boolean_T                    Flag_stats;
__EXTERN__ Boolean_T                    Flag_trace;
__EXTERN__ Boolean_T                    Flag_wolfgang;
__EXTERN__ Integer_T                    M_min_crossrefs;
__EXTERN__ Integer_T                    M_strings;
__EXTERN__ char                        *Str_auxfile;
__EXTERN__ char                        *Str_csfile;


/*-
**----------------------------------------------------------------------------
** Variables used to hold the value of certain key capacity limits.  The
** values used to be #defined as the uppercase equivalent name, but can 
** now be set dynamically from the command line.
**----------------------------------------------------------------------------
*/
__EXTERN__ Integer_T                    Buf_Size;
__EXTERN__ Integer_T                    Ent_Str_Size;
__EXTERN__ Integer_T                    Glob_Str_Size;
__EXTERN__ Integer_T                    Hash_Prime;
__EXTERN__ Integer_T                    Hash_Size;
__EXTERN__ Integer_T                    Lit_Stk_Size;
__EXTERN__ Integer_T                    Max_Bib_Files;
__EXTERN__ Integer_T                    Max_Cites;
__EXTERN__ Integer_T                    Max_Fields;
__EXTERN__ Integer_T                    Max_Glob_Strs;
__EXTERN__ Integer_T                    Max_Print_Line;
__EXTERN__ Integer_T                    Max_Strings;
__EXTERN__ Integer_T                    Min_Crossrefs;
__EXTERN__ Integer_T                    Pool_Size;
__EXTERN__ Integer_T                    Wiz_Fn_Space;
#endif                          /* __GBLVARS.H__ */
