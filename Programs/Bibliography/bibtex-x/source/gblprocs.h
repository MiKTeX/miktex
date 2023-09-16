/*-
******************************************************************************
******************************************************************************
**
**  MODULE
**
**      file: gblprocs.h
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
**      This module declares (using ANSI function prototypes) all of the
**      global functions that will be used in the program.  The functions
**      are declared in alphabetical order.
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
**      $Log: gblprocs.h,v $
**      Revision 3.71  1996/08/18  20:47:30  kempson
**      Official release 3.71 (see HISTORY file for details).
**
**      Revision 3.70  1996/04/08  10:08:40  kempson
**      Final documentation & cosmetic changes for official release 3.70.
**
**      Revision 3.5  1995/09/24  20:44:37  kempson
**      Many changes for final beta test version.
**
**      Revision 3.4  1995/04/09  22:15:44  kempson
**      Placed under RCS control
**
******************************************************************************
******************************************************************************
*/
#ifndef __GBLPROCS_H__
# define __GBLPROCS_H__             1


#ifdef UTF_8
#if defined(WIN32) && defined(WITH_SYSTEM_ICU)
#include "icu.h"
#else
#include "unicode/uchar.h"
#include "unicode/ustdio.h"
#include "unicode/ustring.h"
#include "unicode/ucol.h"
#endif
int32_t                 icu_toUChars (BufType_T buf,
                                      BufPointer_T bf_ptr,
                                      BufPointer_T len,
                                      UChar * target,
                                      int32_t tarcap);
int32_t                 icu_strToLower (UChar * tarlow,
                                        int32_t tlcap,
                                        UChar * target,
                                        int32_t tarlen);
int32_t                 icu_fromUChars (unsigned char * dest,
                                        int32_t destcap,
                                        const UChar * src,
                                        int32_t srclen);
int32_t                 icu_strToUpper (UChar * tarup,
                                int32_t tucap,
                                UChar * target,
                                int32_t tarlen);
int32_t                 icu_toUChar32s (BufType_T buf,
                                      BufPointer_T bf_ptr,
                                      BufPointer_T len,
                                      UChar32 * target,
                                      int32_t tarcap,
                                      UChar * buf16);
int32_t                 icu_fromUChar32s (unsigned char * dest,
                                        int32_t destcap,
                                        const UChar32 * src,
                                        int32_t srclen,
                                        UChar * buf16);
#define  utf8len(a)  ((a)<0x80 ? 1 : ((a)<0xc2 ? -2 : ((a)<0xe0 ? 2 : ((a)<0xf0 ? 3 : ((a)<0xf5 ? 4 : -1)))))
#endif
void                    a_close (const AlphaFile_T file_pointer);
Boolean_T               a_open_in (AlphaFile_T *file_pointer,
                                   Integer_T search_path);
Boolean_T               a_open_out (AlphaFile_T *file_pointer);
void                    add_buf_pool (StrNumber_T pstr);
void                    add_database_cite (CiteNumber_T *newcite);
void                    add_extension (StrNumber_T ext);
void                    add_out_pool (StrNumber_T pstr);
void                    add_pool_buf_and_push (void);
void                    already_seen_function_print (HashLoc_T seenfnloc);
void                    aux_bib_data_command (void);
void                    aux_bib_style_command (void);
void                    aux_citation_command (void);
void                    aux_end1_err_print (void);
void                    aux_end2_err_print (void);
void                    aux_err_illegal_another_print (Integer_T cmdnum);
void                    aux_err_no_right_brace_print (void);
void                    aux_err_print (void);
void                    aux_err_stuff_after_right_brace (void);
void                    aux_err_white_space_in_argument (void);
void                    aux_input_command (void);

Boolean_T               bad_argument_token (void);
void                    bad_cross_reference_print (StrNumber_T s);
void                    bib_cmd_confusion (void);
void                    bib_equals_sign_print (void);
void                    bib_err_print (void);
void                    bib_field_too_long_print (void);
void                    bib_id_print (void);
void                    bib_ln_num_print (void);
void                    bib_one_of_two_print (ASCIICode_T char1,
                                ASCIICode_T char2);
void                    bib_unbalanced_braces_print (void);
void                    bib_warn_print (void);
void                    brace_lvl_one_letters_complaint (void);
void                    braces_unbalanced_complaint (StrNumber_T poplitvar);
void                    bst_1print_string_size_exceeded (void);
void                    bst_2print_string_size_exceeded (void);
void                    bst_cant_mess_with_entries_prin (void);
void                    bst_entry_command (void);
void                    bst_err_print_and_look_for_blan (void);
void                    bst_ex_warn_print (void);
void                    bst_execute_command (void);
void                    bst_function_command (void);
void                    bst_id_print (void);
void                    bst_integers_command (void);
void                    bst_iterate_command (void);
void                    bst_left_brace_print (void);
void                    bst_ln_num_print (void);
void                    bst_macro_command (void);
void                    bst_mild_ex_warn_print (void);
void                    bst_read_command (void);
void                    bst_reverse_command (void);
void                    bst_right_brace_print (void);
void                    bst_sort_command (void);
void                    bst_strings_command (void);
void                    bst_warn_print (void);
void                    buffer_overflow (void);
void                    build_in (PdsType_T pds,
                                PdsLen_T len,
                                HashLoc_T *fn_hash_loc,
                                BltInRange_T bltinnum);

void                    case_conversion_confusion (void);
void                    check_brace_level (StrNumber_T poplitvar);
void                    check_cite_overflow (CiteNumber_T lastcite);
void                    check_command_execution (void);
void                    check_field_overflow (Integer_T totalfields);
void                    cite_key_disappeared_confusion (void);
Boolean_T               compress_bib_white (void);

void                    decr_brace_level (StrNumber_T poplitvar);

void                    eat_bib_print (void);
Boolean_T               eat_bib_white_space (void);
void                    eat_bst_print (void);
Boolean_T               eat_bst_white_space (void);
Boolean_T               enough_text_chars (BufPointer_T enoughchars);
Boolean_T               eoln (const AlphaFile_T file_pointer);
void                    execute_fn (HashLoc_T exfnloc);

void                    figure_out_the_formatted_name (void);
Boolean_T               find_cite_locs_for_this_cite_ke (StrNumber_T citestr);

void                    get_aux_command_and_process (void);
void                    get_bib_command_or_entry_and_pr (void);
void                    get_bst_command_and_process (void);
void                    get_the_top_level_aux_file_name (void);

void                    hash_cite_confusion (void);

void                    id_scanning_confusion (void);
void                    illegl_literal_confusion (void);
void                    init_command_execution (void);
void                    initialize (void);
Boolean_T               input_ln (AlphaFile_T f);
void                    int_to_ASCII (Integer_T inte,
                                BufType_T int_buf,
                                BufPointer_T int_begin,
                                BufPointer_T *int_end);

void                    last_check_for_aux_errors (void);
Boolean_T               less_than (CiteNumber_T arg1,
                                CiteNumber_T arg2);
void                    lower_case (BufType_T buf,
                                BufPointer_T bf_ptr,
                                BufPointer_T len);
#ifdef UTF_8
BufPointer_T            lower_case_uni (BufType_T buf,
                                BufPointer_T bf_ptr,
                                BufPointer_T len);
static inline
void Lower_case (BufType_T buf, BufPointer_T bf_ptr, BufPointer_T len)
{
  bf_ptr += lower_case_uni (buf, bf_ptr, len);
}
#else
# define Lower_case lower_case
#endif

void                    macro_warn_print (void);
StrNumber_T             make_string (void);
void                    mark_error (void);
void                    mark_fatal (void);
void                    mark_warning (void);

void                    name_scan_for_and (StrNumber_T poplitvar);
void                    non_existent_cross_reference_er (void);

void                    out_pool_str (AlphaFile_T f,
                                StrNumber_T s);

void                    output_bbl_line (void);
void                    out_token (AlphaFile_T f);

void                    pool_overflow (void);
void                    pop_lit_stk (Integer_T *pop_lit,
                                StkType_T *pop_type);
void                    pop_the_aux_stack (void);
void                    pop_top_and_print (void);
void                    pop_whole_stack (void);
void                    pre_def_certain_strings (void);
void                    pre_define (PdsType_T pds,
                                PdsLen_T len,
                                StrIlk_T ilk);
void                    print_a_newline (void);
void                    print_a_pool_str (StrNumber_T s);
void                    print_a_token (void);
void                    print_aux_name (void);
void                    print_bad_input_line (void);
void                    print_bib_name (void);
void                    print_bst_name (void);
void                    print_confusion (void);
void                    print_fn_class (HashLoc_T fnloc);
void                    print_lit (Integer_T stk_lt,
                                StkType_T stk_tp);
void                    print_missing_entry (StrNumber_T s);
void                    print_overflow (void);
void                    print_recursion_illegal (void);
void                    print_skipping_whatever_remains (void);
void                    print_stk_lit (Integer_T stk_lt,
                                StkType_T stk_tp);
void                    print_wrong_stk_lit (Integer_T stk_lt,
                                StkType_T stk_tp1,
                                StkType_T stk_tp2);
void                    push_lit_stk (Integer_T push_lt,
                                StkType_T push_type);

void                    quick_sort (CiteNumber_T left_end,
                                CiteNumber_T right_end);

void                    sam_too_long_file_name_print (void);
void                    sam_wrong_file_name_print (void);
Boolean_T               scan1 (ASCIICode_T char1);
Boolean_T               scan1_white (ASCIICode_T char1);
Boolean_T               scan2 (ASCIICode_T char1,
                                ASCIICode_T char2);
Boolean_T               scan2_white (ASCIICode_T char1,
                                ASCIICode_T char2);
Boolean_T               scan3 (ASCIICode_T char1,
                                ASCIICode_T char2,
                                ASCIICode_T char3);
Boolean_T               scan_a_field_token_and_eat_whit (void);
Boolean_T               scan_alpha (void);
Boolean_T               scan_and_store_the_field_value (void);
Boolean_T               scan_balanced_braces (void);
void                    scan_fn_def (HashLoc_T fn_hash_loc);
void                    scan_identifier (ASCIICode_T char1,
                                ASCIICode_T char2,
                                ASCIICode_T char3);
Boolean_T               scan_integer (void);
Boolean_T               scan_nonneg_integer (void);
Boolean_T               scan_white_space (void);
void                    skip_illegal_stuff_after_token (void);
void                    skip_stuff_at_sp_brace_level_gr (void);
void                    skip_token_print (void);
void                    skp_token_unknown_function_prin (void);
void                    start_name (StrNumber_T filename);
Boolean_T               str_eq_buf (StrNumber_T s,
                                BufType_T buf,
                                BufPointer_T bf_ptr,
                                BufPointer_T len);
Boolean_T               str_eq_str (StrNumber_T s1,
                                StrNumber_T s2);
HashLoc_T               str_lookup (BufType_T buf,
                                BufPointer_T j,
                                BufPointer_T l,
                                StrIlk_T ilk,
                                Boolean_T insert_it);
void                    swap (CiteNumber_T swap1,
                                CiteNumber_T swap2);

void                    trace_and_stat_printing (void);
void                    trace_pr_fn_class (HashLoc_T fnloc);

void                    unknwn_function_class_confusion (void);
void                    unknwn_literal_confusion (void);
#ifdef UTF_8
BufPointer_T            upper_case_uni (BufType_T buf,
                                BufPointer_T bf_ptr,
                                BufPointer_T len);
static inline
void upper_case (BufType_T buf, BufPointer_T bf_ptr, BufPointer_T len)
{
  bf_ptr += upper_case_uni (buf, bf_ptr, len);
}
#else
void                    upper_case (BufType_T buf,
                                BufPointer_T bf_ptr,
                                BufPointer_T len);
#endif

void                    von_name_ends_and_last_name_sta (void);
Boolean_T               von_token_found (void);

void                    x_add_period (void);
void                    x_change_case (void);
void                    x_chr_to_int (void);
void                    x_cite (void);
void                    x_concatenate (void);
void                    x_duplicate (void);
void                    x_empty (void);
void                    x_equals (void);
void                    x_format_name (void);
void                    x_gets (void);
void                    x_greater_than (void);
void                    x_int_to_chr (void);
void                    x_int_to_str (void);
void                    x_less_than (void);
void                    x_minus (void);
void                    x_missing (void);
void                    x_num_names (void);
void                    x_plus (void);
void                    x_preamble (void);
void                    x_purify (void);
void                    x_quote (void);
void                    x_substring (void);
void                    x_swap (void);
void                    x_text_length (void);
void                    x_text_prefix (void);
void                    x_type (void);
void                    x_warning (void);
void                    x_width (void);
void                    x_write (void);
void                    x_bit_and (void);
void                    x_bit_or (void);
#ifdef UTF_8
void                    x_is_cjk_string (void);
#endif


/*
** 8-bit support functions.  To minimise the changes required to the
** original BibTeX code in the bibtex-?.c modules, we redefine some key
** functions here:
**
**  IsUpper
**  IsLower
**  ToUpper
**  ToLower
**  char_less_than
**  char_greater_than
*/
#ifdef SUPPORT_8BIT

#define IsUpper(c) (c8lowcase[c] != (c))
#define IsLower(c) (c8upcase[c] != (c))
#define ToUpper(c) (c8upcase[c])
#define ToLower(c) (c8lowcase[c])

#define  char_less_than(char1, char2)   (c8order[char1] < c8order[char2])
#define  char_greater_than(char1, char2)   (c8order[char1] > c8order[char2])

#else                           /* NOT SUPPORT_8BIT */

#define IsUpper(c) ('A'<=(c) && (c) <='Z')
#define IsLower(c) ('a'<=(c) && (c) <='z')
#define ToUpper(c) (c - CASE_DIFFERENCE)
#define ToLower(c) (c + CASE_DIFFERENCE)

#define  char_less_than(char1, char2)		(char1 < char2)
#define  char_greater_than(char1, char2)         (char1 > char2)

#endif                          /* SUPPORT_8BIT */



#endif                          /* __GBLPROCS_H__ */
