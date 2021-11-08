/*-
******************************************************************************
******************************************************************************
**
**  MODULE
**
**      $RCSfile: bibtex-3.c,v $
**      $Revision: 3.71 $
**      $Date: 1996/08/18 20:37:06 $
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
**      This is the third of 4 source modules for BibTeX.  The source has 
**      been split into 4 parts so that some of the more primitive editors
**      can cope.  This code mimics the BibTeX WEB source as closely as
**      possible and there should be NO system dependent code in any of the 
**      bibtex-#.c modules.
**
**      The functions defined in this module are:
**
**          pre_def_certain_strings
**          pre_define
**          print_lit
**          print_missing_entry
**          print_overflow
**          print_recursion_illegal
**          print_skipping_whatever_remains
**          print_stk_lit
**          print_wrong_stk_lit
**          push_lit_stk
**          quick_sort
**          sam_too_long_file_name_print
**          sam_wrong_file_name_print
**          scan1
**          scan1_white
**          scan2
**          scan2_white
**          scan3
**          scan_a_field_token_and_eat_whit
**          scan_alpha
**          scan_and_store_the_field_value
**          scan_balanced_braces
**          str_eq_buf
**          str_eq_str
**          str_lookup
**          swap
**          trace_and_stat_printing
**          trace_pr_fn_class
**          unknwn_function_class_confusion
**          unknwn_literal_confusion
**          upper_case
**          von_name_ends_and_last_name_sta
**          von_token_found
**          x_add_period
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
**      $Log: bibtex-3.c,v $
**      Revision 3.71  1996/08/18  20:37:06  kempson
**      Official release 3.71 (see HISTORY file for details).
**
**      Revision 3.70  1996/04/08  10:08:40  kempson
**      Final documentation & cosmetic changes for official release 3.70.
**
**      Revision 3.6  1995/10/21  22:19:33  kempson
**      Removed previous WEB-imposed limit on the maximum value of Hash_Prime.
**
**      Revision 3.5  1995/09/24  20:44:37  kempson
**      Many changes for final beta test version.
**
**      Revision 3.4  1995/04/09  22:15:40  kempson
**      Placed under RCS control
**
******************************************************************************
******************************************************************************
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "sysdep.h"
#include "bibtex.h"
#include "datatype.h"
#include "gblprocs.h"
#include "gblvars.h"
#include "utils.h"
#include "version.h"


/***************************************************************************
 * WEB section number:	 336
 * ~~~~~~~~~~~~~~~~~~~
 * This is a procedure so that |initialize| is smaller.
 ***************************************************************************/
void          pre_def_certain_strings (void)
BEGIN

/***************************************************************************
 * WEB section number:	75
 * ~~~~~~~~~~~~~~~~~~~
 * It's time to insert some of the pre-defined strings into |str_pool|
 * (and thus the hash table).  These system-dependent strings should
 * contain no upper-case letters, and they must all be exactly
 * |longest_pds| characters long (even if fewer characters are actually
 * stored).  The |pre_define| routine appears shortly.
 *
 * Important notes: These pre-definitions must not have any glitches or
 * the program may bomb because the |log_file| hasn't been opened yet,
 * and |text_ilk|s should be pre-defined later, for
 * .bst-function-execution purposes.
 ***************************************************************************/
  pre_define (".aux        ", 4, FILE_EXT_ILK);
  s_aux_extension = hash_text[pre_def_loc];
  pre_define (".bbl        ", 4, FILE_EXT_ILK);
  s_bbl_extension = hash_text[pre_def_loc];
  pre_define (".blg        ", 4, FILE_EXT_ILK);
  s_log_extension = hash_text[pre_def_loc];
  pre_define (".bst        ", 4, FILE_EXT_ILK);
  s_bst_extension = hash_text[pre_def_loc];
  pre_define (".bib        ", 4, FILE_EXT_ILK);
  s_bib_extension = hash_text[pre_def_loc];

/***************************************************************************
 * WEB section number:	79
 * ~~~~~~~~~~~~~~~~~~~
 * Now we pre-define the command strings; they must all be exactly
 * |longest_pds| characters long.
 *
 * Important note: These pre-definitions must not have any glitches or
 * the program may bomb because the |log_file| hasn't been opened yet.
 ***************************************************************************/
  pre_define ("\\citation  ", 9, AUX_COMMAND_ILK);
  ilk_info[pre_def_loc] = N_AUX_CITATION;
  pre_define ("\\bibdata   ", 8, AUX_COMMAND_ILK);
  ilk_info[pre_def_loc] = N_AUX_BIBDATA;
  pre_define ("\\bibstyle  ", 9, AUX_COMMAND_ILK);
  ilk_info[pre_def_loc] = N_AUX_BIBSTYLE;
  pre_define ("\\@input    ", 7, AUX_COMMAND_ILK);
  ilk_info[pre_def_loc] = N_AUX_INPUT;
  pre_define ("entry       ", 5, BST_COMMAND_ILK);
  ilk_info[pre_def_loc] = N_BST_ENTRY;
  pre_define ("execute     ", 7, BST_COMMAND_ILK);
  ilk_info[pre_def_loc] = N_BST_EXECUTE;
  pre_define ("function    ", 8, BST_COMMAND_ILK);
  ilk_info[pre_def_loc] = N_BST_FUNCTION;
  pre_define ("integers    ", 8, BST_COMMAND_ILK);
  ilk_info[pre_def_loc] = N_BST_INTEGERS;
  pre_define ("iterate     ", 7, BST_COMMAND_ILK);
  ilk_info[pre_def_loc] = N_BST_ITERATE;
  pre_define ("macro       ", 5, BST_COMMAND_ILK);
  ilk_info[pre_def_loc] = N_BST_MACRO;
  pre_define ("read        ", 4, BST_COMMAND_ILK);
  ilk_info[pre_def_loc] = N_BST_READ;
  pre_define ("reverse     ", 7, BST_COMMAND_ILK);
  ilk_info[pre_def_loc] = N_BST_REVERSE;
  pre_define ("sort        ", 4, BST_COMMAND_ILK);
  ilk_info[pre_def_loc] = N_BST_SORT;
  pre_define ("strings     ", 7, BST_COMMAND_ILK);
  ilk_info[pre_def_loc] = N_BST_STRINGS;
  pre_define ("comment     ", 7, BIB_COMMAND_ILK);
  ilk_info[pre_def_loc] = N_BIB_COMMENT;
  pre_define ("preamble    ", 8, BIB_COMMAND_ILK);
  ilk_info[pre_def_loc] = N_BIB_PREAMBLE;
  pre_define ("string      ", 6, BIB_COMMAND_ILK);
  ilk_info[pre_def_loc] = N_BIB_STRING;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 79 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	334
 * ~~~~~~~~~~~~~~~~~~~
 * It's time for us to insert more pre-defined strings into |str_pool|
 * (and thus the hash table) and to insert the |built_in| functions into
 * the hash table.  The strings corresponding to these functions should
 * contain no upper-case letters, and they must all be exactly
 * |longest_pds| characters long.  The |build_in| routine (to appear
 * shortly) does the work.
 *
 * Important note: These pre-definitions must not have any glitches or the
 * program may bomb because the |log_file| hasn't been opened yet.
 ***************************************************************************/
  build_in ("=           ", 1, &b_equals, N_EQUALS);
  build_in (">           ", 1, &b_greater_than, N_GREATER_THAN);
  build_in ("<           ", 1, &b_less_than, N_LESS_THAN);
  build_in ("+           ", 1, &b_plus, N_PLUS);
  build_in ("-           ", 1, &b_minus, N_MINUS);
  build_in ("*           ", 1, &b_concatenate, N_CONCATENATE);
  build_in (":=          ", 2, &b_gets, N_GETS);
  build_in ("add.period$ ", 11, &b_add_period, N_ADD_PERIOD);
  build_in ("call.type$  ", 10, &b_call_type, N_CALL_TYPE);
  build_in ("change.case$", 12, &b_change_case, N_CHANGE_CASE);
  build_in ("chr.to.int$ ", 11, &b_chr_to_int, N_CHR_TO_INT);
  build_in ("cite$       ", 5, &b_cite, N_CITE);
  build_in ("duplicate$  ", 10, &b_duplicate, N_DUPLICATE);
  build_in ("empty$      ", 6, &b_empty, N_EMPTY);
  build_in ("format.name$", 12, &b_format_name, N_FORMAT_NAME);
  build_in ("if$         ", 3, &b_if, N_IF);
  build_in ("int.to.chr$ ", 11, &b_int_to_chr, N_INT_TO_CHR);
  build_in ("int.to.str$ ", 11, &b_int_to_str, N_INT_TO_STR);
  build_in ("missing$    ", 8, &b_missing, N_MISSING);
  build_in ("newline$    ", 8, &b_newline, N_NEWLINE);
  build_in ("num.names$  ", 10, &b_num_names, N_NUM_NAMES);
  build_in ("pop$        ", 4, &b_pop, N_POP);
  build_in ("preamble$   ", 9, &b_preamble, N_PREAMBLE);
  build_in ("purify$     ", 7, &b_purify, N_PURIFY);
  build_in ("quote$      ", 6, &b_quote, N_QUOTE);
  build_in ("skip$       ", 5, &b_skip, N_SKIP);
  build_in ("stack$      ", 6, &b_stack, N_STACK);
  build_in ("substring$  ", 10, &b_substring, N_SUBSTRING);
  build_in ("swap$       ", 5, &b_swap, N_SWAP);
  build_in ("text.length$", 12, &b_text_length, N_TEXT_LENGTH);
  build_in ("text.prefix$", 12, &b_text_prefix, N_TEXT_PREFIX);
  build_in ("top$        ", 4, &b_top_stack, N_TOP_STACK);
  build_in ("type$       ", 5, &b_type, N_TYPE);
  build_in ("warning$    ", 8, &b_warning, N_WARNING);
  build_in ("width$      ", 6, &b_width, N_WIDTH);
  build_in ("while$      ", 6, &b_while, N_WHILE);
  build_in ("width$      ", 6, &b_width, N_WIDTH);
  build_in ("write$      ", 6, &b_write, N_WRITE);
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 334 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	339
 * ~~~~~~~~~~~~~~~~~~~
 * Here we pre-define a few strings used in executing the .bst file:
 * the null string, which is sometimes pushed onto the stack; a string
 * used for default entry types; and some control sequences used to spot
 * foreign characters.  We also initialize the |s_preamble| array to
 * empty.  These pre-defined strings must all be exactly |longest_pds|
 * characters long.
 *
 * Important note: These pre-definitions must not have any glitches or
 * the program may bomb because the |log_file| hasn't been opened yet,
 * and |text_ilk|s should be pre-defined here, not earlier, for
 * .bst-function-execution purposes.
 ***************************************************************************/
  pre_define ("            ", 0, TEXT_ILK);
  s_null = hash_text[pre_def_loc];
  fn_type[pre_def_loc] = STR_LITERAL;
  pre_define ("default.type", 12, TEXT_ILK);
  s_default = hash_text[pre_def_loc];
  fn_type[pre_def_loc] = STR_LITERAL;
  b_default = b_skip;
  preamble_ptr = 0;
  pre_define ("i           ", 1, CONTROL_SEQ_ILK);
  ilk_info[pre_def_loc] = N_I;
  pre_define ("j           ", 1, CONTROL_SEQ_ILK);
  ilk_info[pre_def_loc] = N_J;
  pre_define ("oe          ", 2, CONTROL_SEQ_ILK);
  ilk_info[pre_def_loc] = N_OE;
  pre_define ("OE          ", 2, CONTROL_SEQ_ILK);
  ilk_info[pre_def_loc] = N_OE_UPPER;
  pre_define ("ae          ", 2, CONTROL_SEQ_ILK);
  ilk_info[pre_def_loc] = N_AE;
  pre_define ("AE          ", 2, CONTROL_SEQ_ILK);
  ilk_info[pre_def_loc] = N_AE_UPPER;
  pre_define ("aa          ", 2, CONTROL_SEQ_ILK);
  ilk_info[pre_def_loc] = N_AA;
  pre_define ("AA          ", 2, CONTROL_SEQ_ILK);
  ilk_info[pre_def_loc] = N_AA_UPPER;
  pre_define ("o           ", 1, CONTROL_SEQ_ILK);
  ilk_info[pre_def_loc] = N_O;
  pre_define ("O           ", 1, CONTROL_SEQ_ILK);
  ilk_info[pre_def_loc] = N_O_UPPER;
  pre_define ("l           ", 1, CONTROL_SEQ_ILK);
  ilk_info[pre_def_loc] = N_L;
  pre_define ("L           ", 1, CONTROL_SEQ_ILK);
  ilk_info[pre_def_loc] = N_L_UPPER;
  pre_define ("ss          ", 2, CONTROL_SEQ_ILK);
  ilk_info[pre_def_loc] = N_SS;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 339 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	340
 * ~~~~~~~~~~~~~~~~~~~
 * Now we pre-define any built-in |field|s, |str_entry_var|s, and
 * |int_global_var|s; these strings must all be exactly |longest_pds|
 * characters long.  Note that although these are built-in functions, we
 * classify them (in the |fn_type| array) otherwise.
 *
 * Important note: These pre-definitions must not have any glitches or
 * the program may bomb because the |log_file| hasn't been opened yet.
 ***************************************************************************/
  pre_define ("crossref    ", 8, BST_FN_ILK);
  fn_type[pre_def_loc] = FIELD;
  FN_INFO[pre_def_loc] = num_fields;
  crossref_num = num_fields;
  INCR (num_fields);
  num_pre_defined_fields = num_fields;
  pre_define ("sort.key$   ", 9, BST_FN_ILK);
  fn_type[pre_def_loc] = STR_ENTRY_VAR;
  FN_INFO[pre_def_loc] = num_ent_strs;
  sort_key_num = num_ent_strs;
  INCR (num_ent_strs);
  pre_define ("entry.max$  ", 10, BST_FN_ILK);
  fn_type[pre_def_loc] = INT_GLOBAL_VAR;
  FN_INFO[pre_def_loc] = Ent_Str_Size;
  pre_define ("global.max$ ", 11, BST_FN_ILK);
  fn_type[pre_def_loc] = INT_GLOBAL_VAR;
  FN_INFO[pre_def_loc] = Glob_Str_Size;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 340 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 336 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 77
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure initializes a pre-defined string of length at most
 * |longest_pds|.
 ***************************************************************************/
void          pre_define (PdsType_T pds, PdsLen_T len, StrIlk_T ilk)
BEGIN
  PdsLen_T        i;

  for (i=1; i<=len; i++)
  BEGIN

    /*----------------------------------------------------------------------
     * Unlike the WEB, the string in `pds' will occupy the positions
     * pds[0] ... pds[len-1], rather than pds[1] ... pds[len].
     *----------------------------------------------------------------------*/
    buffer[i] = xord[(UChar_T)pds[i-1]];
  END
  pre_def_loc = str_lookup (buffer, 1, len, ilk, DO_INSERT);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  77 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 3
 * ~~~~~~~~~~~~~~~~~~~
 ***************************************************************************/
void          print_a_newline (void)
BEGIN
  if (log_file != NULL)
    FPUTC ('\n', log_file);
  FPUTC ('\n', TERM_OUT);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION   3 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 51
 * ~~~~~~~~~~~~~~~~~~~
 * And here are the associated procedures. Note the |term_out| file is
 * system dependent.
 ***************************************************************************/
void          print_a_pool_str (StrNumber_T s)
BEGIN
  out_pool_str (TERM_OUT, s);
  out_pool_str (log_file, s);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  51 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 82
 * ~~~~~~~~~~~~~~~~~~~
 * And here are the associated procedures. Note the |term_out| file is
 * system dependent.
 ***************************************************************************/
void          print_a_token (void)
BEGIN
  out_token (TERM_OUT);
  out_token (log_file);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  82 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 108
 * ~~~~~~~~~~~~~~~~~~~
 * Print the name of the current .aux file, followed by a |newline|.
 ***************************************************************************/
void          print_aux_name (void)
BEGIN
  PRINT_POOL_STR (CUR_AUX_STR);
  PRINT_NEWLINE;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 108 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 95
 * ~~~~~~~~~~~~~~~~~~~
 * The |print_bad_input_line| procedure prints the current input line,
 * splitting it at the character being scanned: It prints |buffer[0]|,
 * |buffer[1]|, \dots, |buffer[buf_ptr2-1]| on one line and
 * |SCAN_CHAR|, \dots, |buffer[last-1]| on the next (and both
 * lines start with a colon between two |space|s).  Each |white_space|
 * character is printed as a |space|.
 ***************************************************************************/
void          print_bad_input_line (void)
BEGIN
  BufPointer_T      bf_ptr;

  PRINT (" : ");
  bf_ptr = 0;
  while (bf_ptr < buf_ptr2)
  BEGIN
    if (lex_class[buffer[bf_ptr]] == WHITE_SPACE)
    BEGIN
      PRINT2 ("%c", xchr[SPACE]);
    END
    else
    BEGIN
      PRINT2 ("%c", xchr[buffer[bf_ptr]]);
    END
    INCR (bf_ptr);
  END
  PRINT_NEWLINE;
  PRINT (" : ");
  bf_ptr = 0;
  while (bf_ptr < buf_ptr2)
  BEGIN
    PRINT2 ("%c", xchr[SPACE]);
    INCR (bf_ptr);
  END
  bf_ptr = buf_ptr2;
  while (bf_ptr < last)
  BEGIN
    if (lex_class[buffer[bf_ptr]] == WHITE_SPACE)
    BEGIN
      PRINT2 ("%c", xchr[SPACE]);
    END
    else
    BEGIN
      PRINT2 ("%c", xchr[buffer[bf_ptr]]);
    END
    INCR (bf_ptr);
  END
  PRINT_NEWLINE;
  bf_ptr = 0;
  while ((bf_ptr < buf_ptr2) && (lex_class[buffer[bf_ptr]] == WHITE_SPACE))
  BEGIN
    INCR (bf_ptr);
  END
  if (bf_ptr == buf_ptr2)
  BEGIN
    PRINT_LN ("(Error may have been on previous line)");
  END
  mark_error ();
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  95 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 121
 * ~~~~~~~~~~~~~~~~~~~
 * Here's a procedure we'll need shortly.  It prints the name of the
 * current .bib file, followed by a |newline|.
 ***************************************************************************/
void          print_bib_name (void)
BEGIN
  PRINT_POOL_STR (CUR_BIB_STR);
  PRINT_POOL_STR (s_bib_extension);
  PRINT_NEWLINE;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 121 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 128
 * ~~~~~~~~~~~~~~~~~~~
 * Print the name of the .bst file, followed by a |newline|.
 ***************************************************************************/
void          print_bst_name (void)
BEGIN
  PRINT_POOL_STR (bst_str);
  PRINT_POOL_STR (s_bst_extension);
  PRINT_NEWLINE;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 128 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 45
 * ~~~~~~~~~~~~~~~~~~~
 * When something happens that the program thinks is impossible,
 * call the maintainer.
 ***************************************************************************/
void          print_confusion (void)
BEGIN
  PRINT_LN ("---this can't happen");
  PRINT_LN ("*Please notify the BibTeX maintainer*");
  mark_fatal ();
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 45 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 158
 * ~~~~~~~~~~~~~~~~~~~
 * Occasionally we'll want to |print| the name of one of these function
 * classes.
 ***************************************************************************/
void          print_fn_class (HashLoc_T fn_loc)
BEGIN
  switch (fn_type[fn_loc])
  BEGIN
    case BUILT_IN:
      PRINT ("built-in");
      break;
    case WIZ_DEFINED:
      PRINT ("wizard-defined");
      break;
    case INT_LITERAL:
      PRINT ("integer-literal");
      break;
    case STR_LITERAL:
      PRINT ("string-literal");
      break;
    case FIELD:
      PRINT ("field");
      break;
    case INT_ENTRY_VAR:
      PRINT ("integer-entry-variable");
      break;
    case STR_ENTRY_VAR:
      PRINT ("string-entry-variable");
      break;
    case INT_GLOBAL_VAR:
      PRINT ("integer-global-variable");
      break;
    case STR_GLOBAL_VAR:
      PRINT ("string-global-variable");
      break;
    default:
      unknwn_function_class_confusion ();
      break;
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 158 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 313
 * ~~~~~~~~~~~~~~~~~~~
 * This is similar to |print_stk_lit|, but here we don't give the
 * literal's type, and here we end with a new line.  This procedure
 * should never be called after popping an empty stack.
 ***************************************************************************/
void          print_lit (Integer_T stk_lt, StkType_T stk_tp)
BEGIN
  switch (stk_tp)
  BEGIN
    case STK_INT:
      PRINT_LN2 ("%ld", (long) stk_lt);
      break;
    case STK_STR:
      PRINT_POOL_STR (stk_lt);
      PRINT_NEWLINE;
      break;
    case STK_FN:
      PRINT_POOL_STR (hash_text[stk_lt]);
      PRINT_NEWLINE;
      break;
    case STK_FIELD_MISSING:
      PRINT_POOL_STR (stk_lt);
      PRINT_NEWLINE;
      break;
    case STK_EMPTY:
      illegl_literal_confusion ();
      break;
    default:
      unknwn_literal_confusion ();
      break;
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 313 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/**************************************************************************
 * WEB section number:	 284
 * ~~~~~~~~~~~~~~~~~~~
 * When a cite key on the original |cite_list| (or added to |cite_list|
 * because of cross~referencing) didn't appear in the database, complain.
 ***************************************************************************/
void          print_missing_entry (StrNumber_T s)
BEGIN
  PRINT ("Warning--I didn't find a database entry for \"");
  PRINT_POOL_STR (s);
  PRINT_LN ("\"");
  mark_warning ();
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 284 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 44
 * ~~~~~~~~~~~~~~~~~~~
 * When something in the program wants to be bigger or something out
 * there wants to be smaller, it's time to call it a run.  Here's the
 * first of several macros that have associated procedures so that they
 * produce less inline code.
 ***************************************************************************/
void          print_overflow (void)
BEGIN
  PRINT ("Sorry---you've exceeded BibTeX's ");
  mark_fatal ();
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  44 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 184
 * ~~~~~~~~~~~~~~~~~~~
 * This macro is similar to the last one but is specifically for
 * recursion in a |wiz_defined| function, which is illegal; it helps save
 * space.
 ***************************************************************************/
void          print_recursion_illegal (void)
BEGIN

#ifdef TRACE
  if (Flag_trace)
    TRACE_PR_NEWLINE;
#endif                      			/* TRACE */

  PRINT_LN ("Curse you, wizard, before you recurse me:");
  PRINT ("function ");
  PRINT_TOKEN;
  PRINT_LN (" is illegal in its own definition");

  /*------------------------------------------------------------------------
   * print_recursion_illegal();
   *----------------------------------------------------------------------*/
  skip_token_print ();
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 184 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 96
 * ~~~~~~~~~~~~~~~~~~~
 * This little procedure exists because it's used by at least two other
 * procedures and thus saves some space.
 ***************************************************************************/
void          print_skipping_whatever_remains (void)
BEGIN
  PRINT ("I'm skipping whatever remains of this ");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  96 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 311
 * ~~~~~~~~~~~~~~~~~~~
 * Occasionally we'll want to know what's on the literal stack.  Here we
 * print out a stack literal, giving its type.  This procedure should
 * never be called after popping an empty stack.
 ***************************************************************************/
void          print_stk_lit (Integer_T stk_lt, StkType_T stk_tp)
BEGIN
  switch (stk_tp)
  BEGIN
    case STK_INT:
      PRINT2 ("%ld is an integer literal", (long) stk_lt);
      break;
    case STK_STR:
      PRINT ("\"");
      PRINT_POOL_STR (stk_lt);
      PRINT ("\" is a string literal");
      break;
    case STK_FN:
      PRINT ("`");
      PRINT_POOL_STR (hash_text[stk_lt]);
      PRINT ("' is a function literal");
      break;
    case STK_FIELD_MISSING:
      PRINT ("`");
      PRINT_POOL_STR (stk_lt);
      PRINT ("' is a missing field");
      break;
    case STK_EMPTY:
      illegl_literal_confusion ();
      break;
    default:
      unknwn_literal_confusion ();
      break;
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 311 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 312
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure appropriately chastises the style designer; however, if
 * the wrong literal came from popping an empty stack, the procedure
 * |pop_lit_stack| will have already done the chastising (because this
 * procedure is called only after popping the stack) so there's no need
 * for more.
 ***************************************************************************/
void          print_wrong_stk_lit (Integer_T stk_lt, StkType_T stk_tp1,
				   StkType_T stk_tp2)
BEGIN
  if (stk_tp1 != STK_EMPTY)
  BEGIN
    print_stk_lit (stk_lt, stk_tp1);
    switch (stk_tp2)
    BEGIN
      case STK_INT:
        PRINT (", not an integer,");
        break;
      case STK_STR:
        PRINT (", not a string,");
        break;
      case STK_FN:
        PRINT (", not a function,");
        break;
      case STK_FIELD_MISSING:
      case STK_EMPTY:
        illegl_literal_confusion ();
        break;
      default:
        unknwn_literal_confusion ();
        break;
    END
    bst_ex_warn_print ();
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 312 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 307
 * ~~~~~~~~~~~~~~~~~~~
 * Ok, that's it for sorting; now we'll play with the literal stack.
 * This procedure pushes a literal onto the stack, checking for stack
 * overflow.
 ***************************************************************************/
void          push_lit_stk (Integer_T push_lt, StkType_T push_type)
BEGIN

#ifdef TRACE
  LitStkLoc_T       dum_ptr;
#endif                      			/* TRACE */

  lit_stack[lit_stk_ptr] = push_lt;
  lit_stk_type[lit_stk_ptr] = push_type;

#ifdef TRACE
  if (Flag_trace) {
    for (dum_ptr=0; dum_ptr <= lit_stk_ptr; dum_ptr++)
    BEGIN
      TRACE_PR ("  ");
    END
    TRACE_PR ("Pushing ");

    switch (lit_stk_type[lit_stk_ptr])
    BEGIN
      case STK_INT:
        TRACE_PR_LN2 ("%ld", (long) lit_stack[lit_stk_ptr]);
        break;
      case STK_STR:
        TRACE_PR ("\"");
        TRACE_PR_POOL_STR (lit_stack[lit_stk_ptr]);
        TRACE_PR_LN ("\"");
        break;
      case STK_FN:
        TRACE_PR ("`");
        TRACE_PR_POOL_STR (hash_text[lit_stack[lit_stk_ptr]]);
        TRACE_PR_LN ("'");
        break;
      case STK_FIELD_MISSING:
        TRACE_PR ("missing field `");
        out_pool_str (log_file, lit_stack[lit_stk_ptr]);
        TRACE_PR_LN ("'");
        break;
      case STK_EMPTY:
        TRACE_PR_LN ("a bad literal--popped from an empty stack");
        break;
      default:
        unknwn_literal_confusion ();
        break;
    END
  }                             /* end if (Flag_trace) */
#endif                      			/* TRACE */

  if (lit_stk_ptr == Lit_Stk_Size)
  BEGIN
    BIB_XRETALLOC_NOSET ("lit_stack", lit_stack, Integer_T,
                         Lit_Stk_Size, Lit_Stk_Size + LIT_STK_SIZE);
    BIB_XRETALLOC ("lit_stk_type", lit_stk_type, StkType_T,
                   Lit_Stk_Size, Lit_Stk_Size + LIT_STK_SIZE);
  END
  INCR (lit_stk_ptr);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 307 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/***************************************************************************
 * WEB section number:	 303
 * ~~~~~~~~~~~~~~~~~~~
 * These next two procedures (actually, one procedures and one function,
 * but who's counting) are subroutines for |quick_sort|, which follows.
 * The |swap| procedure exchanges the two elements its arguments point
 * to.
 ***************************************************************************/
void          quick_sort (CiteNumber_T left_end, CiteNumber_T right_end)
BEGIN
  CiteNumber_T      left,
		    right;
  CiteNumber_T      insert_ptr;
  CiteNumber_T      middle;
  CiteNumber_T      partition;

#ifdef TRACE
  if (Flag_trace)
    TRACE_PR_LN3 ("Sorting %ld through %ld", (long) left_end,
		  (long) right_end);
#endif                      			/* TRACE */

  if ((right_end - left_end) < SHORT_LIST)
  BEGIN

/***************************************************************************
 * WEB section number:	304
 * ~~~~~~~~~~~~~~~~~~~
 * This code sorts the entries between |left_end| and |right_end| when
 * the difference is less than |short_list|.  Each iteration of the outer
 * loop inserts the element indicated by |insert_ptr| into its proper
 * place among the (sorted) elements from |left_end| through
 * |insert_ptr-1|.
 ***************************************************************************/
    for (insert_ptr=(left_end+1); insert_ptr<=right_end; insert_ptr++)
    BEGIN
      for (right=insert_ptr; right >= (left_end+1); right--)
      BEGIN
        if (less_than (SORTED_CITES[right-1], SORTED_CITES[right]))
        BEGIN
          goto Next_Insert_Label;
        END
        swap (right - 1, right);
      END
Next_Insert_Label: DO_NOTHING;
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 304 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  END
  else
  BEGIN

/***************************************************************************
 * WEB section number:	305
 * ~~~~~~~~~~~~~~~~~~~
 * Now we find the median of the three sort.key$s to which the three
 * elements |sorted_cites[left_end+end_offset]|,
 * |sorted_cites[right_end]-end_offset|, and
 * |sorted_cites[(left_end+right_end) div 2]| point (a nonzero
 * |end_offset| avoids using as the leftmost of the three elements the
 * one that was swapped there when the old partition element was swapped
 * into its final spot; this turns out to avoid $n^2$ behavior when the
 * list is nearly sorted to start with).  This code determines which of
 * the six possible permutations we're dealing with and moves the median
 * element to |left_end|.  The comments next to the |swap| actions give
 * the known orderings of the corresponding elements of |sorted_cites|
 * before the action.
 ***************************************************************************/
    left = left_end + END_OFFSET;
    middle = (left_end + right_end) / 2;
    right = right_end - END_OFFSET;
    if (less_than (SORTED_CITES[left], SORTED_CITES[middle]))
    BEGIN
      if (less_than (SORTED_CITES[middle], SORTED_CITES[right]))
      BEGIN
        swap (left_end, middle);
      END
      else if (less_than (SORTED_CITES[left], SORTED_CITES[right]))
      BEGIN
        swap (left_end, right);
      END
      else
      BEGIN
        swap (left_end, left);
      END
    END
    else if (less_than (SORTED_CITES[right], SORTED_CITES[middle]))
    BEGIN
      swap (left_end, middle);
    END
    else if (less_than (SORTED_CITES[right], SORTED_CITES[left]))
    BEGIN
      swap (left_end, right);
    END
    else
    BEGIN
      swap (left_end, left);
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 305 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	306
 * ~~~~~~~~~~~~~~~~~~~
 * This module uses the median-of-three computed above to partition the
 * elements into those less than and those greater than the median.
 * Equal sort.key$s are sorted by order of occurrence (in
 * |cite_list|).
 ***************************************************************************/
    partition = SORTED_CITES[left_end];
    left = left_end + 1;
    right = right_end;
    do
    BEGIN
      while (less_than (SORTED_CITES[left], partition))
      BEGIN
        INCR (left);
      END
      while (less_than (partition, SORTED_CITES[right]))
      BEGIN
        DECR (right);
      END
      if (left < right)
      BEGIN
        swap (left, right);
        INCR (left);
        DECR (right);
      END
    END while ( ! (left == (right + 1)));
    swap (left_end, right);
    quick_sort (left_end, right - 1);
    quick_sort (left, right_end);
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 306 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 303 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 98
 * ~~~~~~~~~~~~~~~~~~~
 * I mean, this is truly disgraceful.  A user has to type something in to
 * the terminal just once during the entire run.  And it's not some
 * complicated string where you have to get every last punctuation mark
 * just right, and it's not some fancy list where you get nervous because
 * if you forget one item you have to type the whole thing again; it's
 * just a simple, ordinary, file name.  Now you'd think a five-year-old
 * could do it; you'd think it's so simple a user should be able to do it
 * in his sleep.  But noooooooooo.  He had to sit there droning on and on
 * about who knows what until he exceeded the bounds of common sense, and
 * he probably didn't even realize it.  Just pitiful.  What's this world
 * coming to?  We should probably just delete all his files and be done
 * with him.  Note: The |TERM_OUT| file is system dependent.
 *
 * NOTE: because C arrays start at index 0, not 1, the subscripts of array
 *	 |name_of_file| are generally 1 less than those in the WEB source.
 ***************************************************************************/
void          sam_too_long_file_name_print (void)
BEGIN
  FPRINTF (TERM_OUT, "File name `");
  name_ptr = 0;
  while (name_ptr < aux_name_length)
  BEGIN
    FPUTC (name_of_file[name_ptr], TERM_OUT);
    INCR (name_ptr);
  END
  FPRINTF (TERM_OUT, "' is too long\n");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  98 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 99
 * ~~~~~~~~~~~~~~~~~~~
 * We've abused the user enough for one section; suffice it to
 * say here that most of what we said last module still applies.
 * Note: The |TERM_OUT| file is system dependent.
 *
 * NOTE: because C arrays start at index 0, not 1, the subscripts of array
 *	 |name_of_file| are generally 1 less than those in the WEB source.
 ***************************************************************************/
void          sam_wrong_file_name_print (void)
BEGIN
  FPRINTF (TERM_OUT, "I couldn't open file name `");
  name_ptr = 0;
  while (name_ptr < name_length)
  BEGIN
    FPUTC (name_of_file[name_ptr], TERM_OUT);
    INCR (name_ptr);
  END
  FPRINTF (TERM_OUT, "'\n");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  99 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 83
 * ~~~~~~~~~~~~~~~~~~~
 * This function scans the |buffer| for the next token, starting at the
 * global variable |buf_ptr2| and ending just before either the single
 * specified stop-character or the end of the current line, whichever
 * comes first, respectively returning |true| or |false|; afterward,
 * |scan_char| is the first character following this token.
 ***************************************************************************/
Boolean_T         scan1 (ASCIICode_T char1)
BEGIN
  Boolean_T	  scan1;

  buf_ptr1 = buf_ptr2;
  while ((SCAN_CHAR != char1) && (buf_ptr2 < last))
  BEGIN
    INCR (buf_ptr2);
  END
  if (buf_ptr2 < last)
  BEGIN
    scan1 = TRUE;
  END
  else
  BEGIN
    scan1 = FALSE;
  END
  return (scan1);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  83 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 84
 * ~~~~~~~~~~~~~~~~~~~
 * This function is the same but stops at |white_space| characters as well.
 ***************************************************************************/
Boolean_T         scan1_white (ASCIICode_T char1)
BEGIN
  Boolean_T       scan1_white;

  buf_ptr1 = buf_ptr2;
  while ((lex_class[SCAN_CHAR] != WHITE_SPACE)
	    && (SCAN_CHAR != char1) && (buf_ptr2 < last))
  BEGIN
    INCR (buf_ptr2);
  END
  if (buf_ptr2 < last)
  BEGIN
    scan1_white = TRUE;
  END
  else
  BEGIN
    scan1_white = FALSE;
  END
  return (scan1_white);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  84 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 85
 * ~~~~~~~~~~~~~~~~~~~
 * This function is similar to |scan1|, but stops at either of two
 * stop-characters as well as the end of the current line.
 ***************************************************************************/
Boolean_T         scan2 (ASCIICode_T char1, ASCIICode_T char2)
BEGIN
  Boolean_T	  scan2;

  buf_ptr1 = buf_ptr2;
  while ((SCAN_CHAR != char1) && (SCAN_CHAR != char2)
	    && (buf_ptr2 < last))
  BEGIN
    INCR (buf_ptr2);
  END
  if (buf_ptr2 < last)
  BEGIN
    scan2 = TRUE;
  END
  else
  BEGIN
    scan2 = FALSE;
  END
  return (scan2);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  85 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 86
 * ~~~~~~~~~~~~~~~~~~~
 * This function is the same but stops at |white_space| characters as well.
 ***************************************************************************/
Boolean_T         scan2_white (ASCIICode_T char1, ASCIICode_T char2)
BEGIN
  Boolean_T	  scan2_white;

  buf_ptr1 = buf_ptr2;
  while ((SCAN_CHAR != char1) && (SCAN_CHAR != char2)
	    && (lex_class[SCAN_CHAR] != WHITE_SPACE) && (buf_ptr2 < last))
  BEGIN
    INCR (buf_ptr2);
  END
  if (buf_ptr2 < last)
  BEGIN
    scan2_white = TRUE;
  END
  else
  BEGIN
    scan2_white = FALSE;
  END
  return (scan2_white);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  86 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 87
 * ~~~~~~~~~~~~~~~~~~~
 * This function is similar to |scan2|, but stops at either of three
 * stop-characters as well as the end of the current line.
 ***************************************************************************/
Boolean_T         scan3 (ASCIICode_T char1, ASCIICode_T char2,
			 ASCIICode_T char3)
BEGIN
  Boolean_T	  scan3;

  buf_ptr1 = buf_ptr2;
  while ((SCAN_CHAR != char1) && (SCAN_CHAR != char2)
	    && (SCAN_CHAR != char3) && (buf_ptr2 < last))
  BEGIN
    INCR (buf_ptr2);
  END
  if (buf_ptr2 < last)
  BEGIN
    scan3 = TRUE;
  END
  else
  BEGIN
    scan3 = FALSE;
  END
  return (scan3);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  87 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 250
 * ~~~~~~~~~~~~~~~~~~~
 * Each field token is either a nonnegative number, a macro name (like
 * `jan'), or a brace-balanced string delimited by either |double_quote|s
 * or braces.  Thus there are four possibilities for the first character
 * of the field token: If it's a |left_brace| or a |double_quote|, the
 * token (with balanced braces, up to the matching |right_str_delim|) is
 * a string; if it's |numeric|, the token is a number; if it's anything
 * else, the token is a macro name (and should thus have been defined by
 * either the .bst-file's macro command or the .bib-file's
 * string command).  This function returns |false| if there was a
 * serious syntax error.
 ***************************************************************************/
Boolean_T         scan_a_field_token_and_eat_whit (void)
BEGIN
  Boolean_T scan_a_field_token_And_eat_whit;

  scan_a_field_token_And_eat_whit = FALSE;
  switch (SCAN_CHAR)
  BEGIN
    case LEFT_BRACE:
      right_str_delim = RIGHT_BRACE;
      if ( ! scan_balanced_braces ())
        goto Exit_Label;
      break;
    case DOUBLE_QUOTE:
      right_str_delim = DOUBLE_QUOTE;
      if ( ! scan_balanced_braces ())
        goto Exit_Label;
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':

/***************************************************************************
 * WEB section number:	258
 * ~~~~~~~~~~~~~~~~~~~
 * This module scans a nonnegative number and copies it to |field_vl_str|
 * if it's to store the field.
 ***************************************************************************/
      if ( ! scan_nonneg_integer ())
      BEGIN
        CONFUSION ("A digit disappeared");
      END
      if (store_field)
      BEGIN
        tmp_ptr = buf_ptr1;
        while (tmp_ptr < buf_ptr2)
        BEGIN
	  COPY_CHAR (buffer[tmp_ptr]);
          INCR (tmp_ptr);
        END
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 258 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      break;
    default:

/***************************************************************************
 * WEB section number:	259
 * ~~~~~~~~~~~~~~~~~~~
 * This module scans a macro name and copies its string to |field_vl_str|
 * if it's to store the field, complaining if the macro is recursive or
 * undefined.
 ***************************************************************************/
      scan_identifier (COMMA, right_outer_delim, CONCAT_CHAR);
      BIB_IDENTIFIER_SCAN_CHECK ("a field part");
      if (store_field)
      BEGIN
        lower_case (buffer, buf_ptr1, TOKEN_LEN);
        macro_name_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN, MACRO_ILK,
					DONT_INSERT);
        store_token = TRUE;
        if (at_bib_command)
        BEGIN
          if (command_num == N_BIB_STRING)
          BEGIN
            if (macro_name_loc == cur_macro_loc)
            BEGIN
              store_token = FALSE;
              MACRO_NAME_WARNING ("used in its own definition");
            END
          END
        END
        if ( ! hash_found)
        BEGIN
          store_token = FALSE;
	  MACRO_NAME_WARNING ("undefined");
        END
        if (store_token)
        BEGIN

/***************************************************************************
 * WEB section number:	260
 * ~~~~~~~~~~~~~~~~~~~
 * The macro definition may have |white_space| that needs compressing,
 * because it may have been defined in the .bst file.
 ***************************************************************************/
          tmp_ptr = str_start[ilk_info[macro_name_loc]];
          tmp_end_ptr = str_start[ilk_info[macro_name_loc] + 1];
          if (FIELD_END == 0)
          BEGIN
            if ((lex_class[str_pool[tmp_ptr]] == WHITE_SPACE)
		    && (tmp_ptr < tmp_end_ptr))
            BEGIN
	      COPY_CHAR (SPACE);
              INCR (tmp_ptr);
              while ((lex_class[str_pool[tmp_ptr]] == WHITE_SPACE)
			&& (tmp_ptr < tmp_end_ptr))
              BEGIN
                INCR (tmp_ptr);
              END
            END
          END
          while (tmp_ptr < tmp_end_ptr)
          BEGIN
            if (lex_class[str_pool[tmp_ptr]] != WHITE_SPACE)
            BEGIN
              COPY_CHAR (str_pool[tmp_ptr]);
            END
            else if (FIELD_VL_STR[FIELD_END - 1] != SPACE)
            BEGIN
	      COPY_CHAR (SPACE);
            END
            INCR (tmp_ptr);
          END
        END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 260 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 259 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      break;
  END
  EAT_BIB_WHITE_AND_EOF_CHECK;
  scan_a_field_token_And_eat_whit = TRUE;
Exit_Label: DO_NOTHING;
  return (scan_a_field_token_And_eat_whit);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 250 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 88
 * ~~~~~~~~~~~~~~~~~~~
 * This function scans for letters, stopping at the first nonletter; it
 * returns |true| if there is at least one letter.
 ***************************************************************************/
Boolean_T         scan_alpha (void)
BEGIN
  Boolean_T	  scan_alpha;

  buf_ptr1 = buf_ptr2;
  while ((lex_class[SCAN_CHAR] == ALPHA) && (buf_ptr2 < last))
  BEGIN
    INCR (buf_ptr2);
  END
  if (TOKEN_LEN == 0)
  BEGIN
    scan_alpha = FALSE;
  END
  else
  BEGIN
    scan_alpha = TRUE;
  END
  return (scan_alpha);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 88 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 249
 * ~~~~~~~~~~~~~~~~~~~
 * This function scans the list of field tokens that define the field
 * value string.  If |store_field| is |true| it accumulates (indirectly)
 * in |field_vl_str| the concatenation of all the field tokens,
 * compressing nonnull |white_space| to a single |space| and, if the
 * field value is for a field (rather than a string definition), removing
 * any leading or trailing |white_space|; when it's finished it puts the
 * string into the hash table.  It returns |false| if there was a serious
 * syntax error.
 ***************************************************************************/
Boolean_T         scan_and_store_the_field_value (void)
BEGIN
  Boolean_T    scan_and_store_the_field_value;

  scan_and_store_the_field_value = FALSE;
  FIELD_END = 0;
  if ( ! scan_a_field_token_and_eat_whit ())
  BEGIN
    goto Exit_Label;
  END
  while (SCAN_CHAR == CONCAT_CHAR)
  BEGIN
    INCR (buf_ptr2);
    EAT_BIB_WHITE_AND_EOF_CHECK;
    if ( ! scan_a_field_token_and_eat_whit ())
    BEGIN
      goto Exit_Label;
    END
  END
  if (store_field)
  BEGIN

/***************************************************************************
 * WEB section number:	261
 * ~~~~~~~~~~~~~~~~~~~
 * Now it's time to store the field value in the hash table, and store an
 * appropriate pointer to it (depending on whether it's for a database
 * entry or command).  But first, if necessary, we remove a trailing
 * |space| and a leading |space| if these exist.  (Hey, if we had some
 * ham we could make ham-and-eggs if we had some eggs.)
 ***************************************************************************/
    if ( ! at_bib_command)
    BEGIN
      if (FIELD_END > 0)
      BEGIN
        if (FIELD_VL_STR[FIELD_END - 1] == SPACE)
        BEGIN
          DECR (FIELD_END);
        END
      END
    END
    if (( ! at_bib_command) && (FIELD_VL_STR[0] == SPACE) && (FIELD_END > 0))
    BEGIN
      FIELD_START = 1;
    END
    else
    BEGIN
      FIELD_START = 0;
    END
    field_val_loc = str_lookup (FIELD_VL_STR, FIELD_START,
				FIELD_END - FIELD_START, TEXT_ILK,
				DO_INSERT);
    fn_type[field_val_loc] = STR_LITERAL;

#ifdef TRACE
    if (Flag_trace) {
      TRACE_PR ("\"");
      TRACE_PR_POOL_STR (hash_text[field_val_loc]);
      TRACE_PR_LN ("\" is a field value");
    }
#endif                      			/* TRACE */

    if (at_bib_command)
    BEGIN

/***************************************************************************
 * WEB section number:	262
 * ~~~~~~~~~~~~~~~~~~~
 * Here's where we store the goods when we're dealing with a command
 * rather than an entry.
 ***************************************************************************/
      switch (command_num)
      BEGIN
        case N_BIB_PREAMBLE:
          s_preamble[preamble_ptr] = hash_text[field_val_loc];
          INCR (preamble_ptr);
          break;
        case N_BIB_STRING:
          ilk_info[cur_macro_loc] = hash_text[field_val_loc];
          break;
        default:
          bib_cmd_confusion ();
          break;
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 262 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    END
    else
    BEGIN

/***************************************************************************
 * WEB section number:	263
 * ~~~~~~~~~~~~~~~~~~~
 * And here, an entry.
 ***************************************************************************/
      field_ptr = (entry_cite_ptr * num_fields) + FN_INFO[field_name_loc];
      if (field_info[field_ptr] != MISSING)
      BEGIN
        PRINT ("Warning--I'm ignoring ");
        PRINT_POOL_STR (cite_list[entry_cite_ptr]);
        PRINT ("'s extra \"");
        PRINT_POOL_STR (hash_text[field_name_loc]);
        BIB_WARN_NEWLINE ("\" field");
      END
      else
      BEGIN
        field_info[field_ptr] = hash_text[field_val_loc];
        if ((ilk_info[field_name_loc] == crossref_num)
		&& ( ! all_entries))
        BEGIN

/***************************************************************************
 * WEB section number:	264
 * ~~~~~~~~~~~~~~~~~~~
 * If the cross-referenced entry isn't already on |cite_list| we add it
 * (at least temporarily); if it is already on |cite_list| we update the
 * cross-reference count, if necessary.  Note that |all_entries| is
 * |false| here.  The alias kludge helps make the stack space not
 * overflow on some machines.
 ***************************************************************************/
          tmp_ptr = FIELD_START;
          while (tmp_ptr < FIELD_END)
          BEGIN
            EXTRA_BUF[tmp_ptr] = FIELD_VL_STR[tmp_ptr];
            INCR (tmp_ptr);
          END
          lower_case (EXTRA_BUF, FIELD_START, FIELD_END - FIELD_START);
          lc_cite_loc = str_lookup (EXTRA_BUF, FIELD_START,
				    FIELD_END - FIELD_START, LC_CITE_ILK,
				    DO_INSERT);
          if (hash_found)
          BEGIN
            cite_loc = ilk_info[lc_cite_loc];
            if (ilk_info[cite_loc] >= old_num_cites)
            BEGIN
              INCR (cite_info[ilk_info[cite_loc]]);
            END
          END
          else
          BEGIN
            cite_loc = str_lookup (FIELD_VL_STR, FIELD_START,
				   FIELD_END - FIELD_START, CITE_ILK,
				   DO_INSERT);
            if (hash_found)
            BEGIN
              hash_cite_confusion ();
            END
            add_database_cite (&cite_ptr);
            cite_info[ilk_info[cite_loc]] = 1;
          END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 264 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

        END
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 263 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 261 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  END
  scan_and_store_the_field_value = TRUE;
Exit_Label: DO_NOTHING;
  return (scan_and_store_the_field_value);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 249 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 253
 * ~~~~~~~~~~~~~~~~~~~
 * This .bib-specific function scans a string with balanced braces,
 * stopping just past the matching |right_str_delim|.  How much work it
 * does depends on whether |store_field = true|.  It returns |false| if
 * there was a serious syntax error.
 ***************************************************************************/
Boolean_T         scan_balanced_braces (void)
BEGIN
  Boolean_T    scan_balanced_braces;

  scan_balanced_braces = FALSE;
  INCR (buf_ptr2);
  CHECK_FOR_AND_COMPRESS_BIB_WHIT;
  if (FIELD_END > 1)
  BEGIN
    if (FIELD_VL_STR[FIELD_END- 1] == SPACE)
    BEGIN
      if (ex_buf[FIELD_END- 2] == SPACE)
      BEGIN
        DECR (FIELD_END);
      END
    END
  END
  bib_brace_level = 0;
  if (store_field)
  BEGIN

/***************************************************************************
 * WEB section number:	256
 * ~~~~~~~~~~~~~~~~~~~
 * This module scans over a brace-balanced string, compressing multiple
 * |white_space| characters into a single |space|.  It starts with
 * |bib_brace_level = 0| and starts at a non|white_space| character.
 ***************************************************************************/
    while (SCAN_CHAR != right_str_delim)
    BEGIN
      switch (SCAN_CHAR)
      BEGIN
        case LEFT_BRACE:
          INCR (bib_brace_level);
	  COPY_CHAR (LEFT_BRACE);
          INCR (buf_ptr2);
	  CHECK_FOR_AND_COMPRESS_BIB_WHIT;

/***************************************************************************
 * WEB section number:	257
 * ~~~~~~~~~~~~~~~~~~~
 * This module is similar to the last but starts with |bib_brace_level > 0|
 * (and, like the last, it starts at a non|white_space| character).
 ***************************************************************************/
          LOOP
          BEGIN
            switch (SCAN_CHAR)
            BEGIN
              case RIGHT_BRACE:
                DECR (bib_brace_level);
                COPY_CHAR (RIGHT_BRACE);
                INCR (buf_ptr2);
		CHECK_FOR_AND_COMPRESS_BIB_WHIT;
                if (bib_brace_level == 0)
                BEGIN
                  goto Loop_Exit_Label;
                END
                break;
              case LEFT_BRACE:
                INCR (bib_brace_level);
		COPY_CHAR (LEFT_BRACE);
                INCR (buf_ptr2);
		CHECK_FOR_AND_COMPRESS_BIB_WHIT;
                break;
              default:
		COPY_CHAR (SCAN_CHAR);
                INCR (buf_ptr2);
		CHECK_FOR_AND_COMPRESS_BIB_WHIT;
                break;
            END
          END
      Loop_Exit_Label: DO_NOTHING
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 257 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

          break;
        case RIGHT_BRACE:
          BIB_UNBALANCED_BRACES_ERR;
        default:
	  COPY_CHAR (SCAN_CHAR);
          INCR (buf_ptr2);
	  CHECK_FOR_AND_COMPRESS_BIB_WHIT;
          break;
      END
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 256 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  END
  else
  BEGIN

/***************************************************************************
 * WEB section number:	254
 * ~~~~~~~~~~~~~~~~~~~
 * This module scans over a brace-balanced string without keeping track
 * of anything but the brace level.  It starts with |bib_brace_level = 0|
 * and at a non|white_space| character.
 ***************************************************************************/
    while (SCAN_CHAR != right_str_delim)
    BEGIN
      if (SCAN_CHAR == LEFT_BRACE)
      BEGIN
        INCR (bib_brace_level);
        INCR (buf_ptr2);
	EAT_BIB_WHITE_AND_EOF_CHECK;
        while (bib_brace_level > 0)
        BEGIN

/***************************************************************************
 * WEB section number:	255
 * ~~~~~~~~~~~~~~~~~~~
 * This module does the same as above but, because |bib_brace_level > 0|, it
 * doesn't have to look for a |right_str_delim|.
 ***************************************************************************/
          if (SCAN_CHAR == RIGHT_BRACE)
          BEGIN
            DECR (bib_brace_level);
            INCR (buf_ptr2);
	    EAT_BIB_WHITE_AND_EOF_CHECK;
	  END
          else if (SCAN_CHAR == LEFT_BRACE)
          BEGIN
            INCR (bib_brace_level);
            INCR (buf_ptr2);
	    EAT_BIB_WHITE_AND_EOF_CHECK;
          END
          else
          BEGIN
            INCR (buf_ptr2);
            if ( ! scan2 (RIGHT_BRACE, LEFT_BRACE))
            BEGIN
	      EAT_BIB_WHITE_AND_EOF_CHECK;
            END
          END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 255 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

        END
      END
      else if (SCAN_CHAR == RIGHT_BRACE)
      BEGIN
        BIB_UNBALANCED_BRACES_ERR;
      END
      else
      BEGIN
        INCR (buf_ptr2);
        if ( ! scan3 (right_str_delim, LEFT_BRACE, RIGHT_BRACE))
        BEGIN
	  EAT_BIB_WHITE_AND_EOF_CHECK;
        END
      END
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 254 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  END
  INCR (buf_ptr2);
  scan_balanced_braces = TRUE;
Exit_Label: DO_NOTHING;
  return (scan_balanced_braces);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 253 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/***************************************************************************
 * WEB section number:  188
 * ~~~~~~~~~~~~~~~~~~~
 * This macro inserts a hash-table location (or one of the two
 * special markers |quote_next_fn| and |end_of_def|) into the
 * |singl_function| array, which will later be copied into the
 * |wiz_functions| array.
 ***************************************************************************/
#define INSERT_FN_LOC(X)            {\
    singl_function[single_ptr] = (X);\
    if (single_ptr == Single_Fn_Space)\
        {BIB_XRETALLOC ("singl_function", singl_function, HashPtr2_T,\
                        Single_Fn_Space, Single_Fn_Space + Single_Fn_Space);}\
    INCR (single_ptr);}


/***************************************************************************
 * WEB section number:	 187
 * ~~~~~~~~~~~~~~~~~~~
 * This recursive function reads and stores the list of functions
 * (separated by |white_space| characters or ends-of-line) that define
 * this new function, and reads a |right_brace|.
 ***************************************************************************/
void          scan_fn_def (HashLoc_T fn_hash_loc)
BEGIN
  typedef Integer_T     FnDefLoc_T;
  HashPtr2_T	*singl_function;
  FnDefLoc_T		single_ptr;
  FnDefLoc_T		copy_ptr;
  BufPointer_T		end_of_num;
  HashLoc_T		impl_fn_loc;
  Integer_T             Single_Fn_Space;

  Single_Fn_Space = SINGLE_FN_SPACE;
  singl_function = (HashPtr2_T *) mymalloc ((Single_Fn_Space + 1)
      * (unsigned long) sizeof (HashPtr2_T), "singl_function");
  EAT_BST_WHITE_AND_EOF_CHECK ("function");
  single_ptr = 0;
  while (SCAN_CHAR != RIGHT_BRACE)
  BEGIN

/***************************************************************************
 * WEB section number:	189
 * ~~~~~~~~~~~~~~~~~~~
 * There are five possibilities for the first character of the token
 * representing the next function of the definition: If it's a
 * |number_sign|, the token is an |int_literal|; if it's a
 * |double_quote|, the token is a |str_literal|; if it's a
 * |single_quote|, the token is a quoted function; if it's a
 * |left_brace|, the token isn't really a token, but rather the start of
 * another function definition (which will result in a recursive call to
 * |scan_fn_def|); if it's anything else, the token is the name of an
 * already-defined function.  Note: To prevent the wizard from using
 * recursion, we have to check that neither a quoted function nor an
 * already-defined-function is actually the currently-being-defined
 * function (which is stored at |wiz_loc|).
 ***************************************************************************/
    switch (SCAN_CHAR)
    BEGIN
      case NUMBER_SIGN:

/***************************************************************************
 * WEB section number:	190
 * ~~~~~~~~~~~~~~~~~~~
 * An |int_literal| is preceded by a |number_sign|, consists of an
 * integer (i.e., an optional |minus_sign| followed by one or more
 * |numeric| characters), and is followed either by a |white_space|
 * character, an end-of-line, or a |right_brace|.  The array |fn_info|
 * contains the value of the integer for |int_literal|s.
 ***************************************************************************/
	BEGIN
	  INCR (buf_ptr2);
	  if ( ! scan_integer ())
	  BEGIN
	    SKIP_TOKEN ("Illegal integer in integer literal");
	  END

#ifdef TRACE
          if (Flag_trace) {
	    TRACE_PR ("#");
	    TRACE_PR_TOKEN;
	    TRACE_PR_LN2 (" is an integer literal with value %ld",
			  (long) token_value);
          }
#endif                      			/* TRACE */

	  literal_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN, INTEGER_ILK,
				    DO_INSERT);
	  if ( ! hash_found)
	  BEGIN
	    fn_type[literal_loc] = INT_LITERAL;
	    FN_INFO[literal_loc] = token_value;
	  END
	  if ((lex_class[SCAN_CHAR] != WHITE_SPACE) && (buf_ptr2 < last)
		  && (SCAN_CHAR != RIGHT_BRACE) && (SCAN_CHAR != COMMENT))
	  BEGIN
	    SKIP_TOKEN_ILLEGAL_STUFF_AFTER;
	  END
	  INSERT_FN_LOC (literal_loc);
	END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 190 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

        break;
      case DOUBLE_QUOTE:

/***************************************************************************
 * WEB section number:	191
 * ~~~~~~~~~~~~~~~~~~~
 * A |str_literal| is preceded by a |double_quote| and consists of all
 * characters on this line up to the next |double_quote|.  Also, there
 * must be either a |white_space| character, an end-of-line, a
 * |right_brace|, or a |comment| following (since functions in the
 * definition must be separated by |white_space|).  The array |fn_info|
 * contains nothing for |str_literal|s.
 ***************************************************************************/
	BEGIN
	  INCR (buf_ptr2);
	  if ( ! scan1 (DOUBLE_QUOTE))
	  BEGIN
	    SKIP_TOKEN2 ("No `%c' to end string literal", xchr[DOUBLE_QUOTE]);
	  END

#ifdef TRACE
          if (Flag_trace) {
  	    TRACE_PR ("\"");
	    TRACE_PR_TOKEN;
	    TRACE_PR ("\"");
	    TRACE_PR_LN (" is a string literal");
          }
#endif                      			/* TRACE */

	  literal_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN, TEXT_ILK,
				   DO_INSERT);
	  fn_type[literal_loc] = STR_LITERAL;
	  INCR (buf_ptr2);
	  if ((lex_class[SCAN_CHAR] != WHITE_SPACE) && (buf_ptr2 < last)
	      && (SCAN_CHAR != RIGHT_BRACE)  && (SCAN_CHAR != COMMENT))
	  BEGIN
	    SKIP_TOKEN_ILLEGAL_STUFF_AFTER;
	  END
	  INSERT_FN_LOC (literal_loc);
	END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 191 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

        break;
      case SINGLE_QUOTE:

/***************************************************************************
 * WEB section number:	192
 * ~~~~~~~~~~~~~~~~~~~
 * A quoted function is preceded by a |single_quote| and consists of all
 * characters up to the next |white_space| character, end-of-line,
 * |right_brace|, or |comment|.
 ***************************************************************************/
	BEGIN
	  INCR (buf_ptr2);
	  if (scan2_white (RIGHT_BRACE, COMMENT))
	  BEGIN
	    DO_NOTHING;
	  END

#ifdef TRACE
          if (Flag_trace) {
	    TRACE_PR ("'");
	    TRACE_PR_TOKEN;
	    TRACE_PR (" is a quoted function ");
          }
#endif                      			/* TRACE */

	  lower_case (buffer, buf_ptr1, TOKEN_LEN);
	  fn_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN, BST_FN_ILK,
			       DONT_INSERT);
	  if ( ! hash_found)
	  BEGIN
	    SKIP_TOKEN_UNKNOWN_FUNCTION;
	  END
	  else

/***************************************************************************
 * WEB section number:	193
 * ~~~~~~~~~~~~~~~~~~~
 * Here we check that this quoted function is a legal one---the function
 * name must already exist, but it mustn't be the currently-being-defined
 * function (which is stored at |wiz_loc|).
 ***************************************************************************/
	  BEGIN
	    if (fn_loc == wiz_loc)
	    BEGIN
	      SKIP_RECURSIVE_TOKEN;
	    END
	    else
	    BEGIN

#ifdef TRACE
              if (Flag_trace) {
	        TRACE_PR ("of type ");
	        trace_pr_fn_class (fn_loc);
	        TRACE_PR_NEWLINE;
              }
#endif                      			/* TRACE */

	      INSERT_FN_LOC (QUOTE_NEXT_FN);
	      INSERT_FN_LOC (fn_loc);
	    END
	  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 193 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 192 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

        break;
      case LEFT_BRACE:

/***************************************************************************
 * WEB section number:	194
 * ~~~~~~~~~~~~~~~~~~~
 * This module marks the implicit function as being quoted, generates a
 * name, and stores it in the hash table.  This name is strictly internal
 * to this program, starts with a |single_quote| (since that will make
 * this function name unique), and ends with the variable |impl_fn_num|
 * converted to ASCII.  The alias kludge helps make the stack space not
 * overflow on some machines.
 ***************************************************************************/
        BEGIN
	  EX_BUF2[0] = SINGLE_QUOTE;
	  int_to_ASCII (impl_fn_num, EX_BUF2, 1, &end_of_num);
	  impl_fn_loc = str_lookup (EX_BUF2, 0, end_of_num, BST_FN_ILK,
				    DO_INSERT);
	  if (hash_found)
	  BEGIN
	    CONFUSION ("Already encountered implicit function");
	  END

#ifdef TRACE
          if (Flag_trace) {
	    TRACE_PR_POOL_STR (hash_text[impl_fn_loc]);
	    TRACE_PR_LN (" is an implicit function");
          }
#endif                      			/* TRACE */

	  INCR (impl_fn_num);
	  fn_type[impl_fn_loc] = WIZ_DEFINED;
	  INSERT_FN_LOC (QUOTE_NEXT_FN);
	  INSERT_FN_LOC (impl_fn_loc);
	  INCR (buf_ptr2);
	  scan_fn_def (impl_fn_loc);
	END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 194 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

        break;
      default:

/***************************************************************************
 * WEB section number:	199
 * ~~~~~~~~~~~~~~~~~~~
 * An already-defined function consists of all characters up to the next
 * |white_space| character, end-of-line, |right_brace|, or |comment|.
 * This function name must already exist, but it mustn't be the
 * currently-being-defined function (which is stored at |wiz_loc|).
 ***************************************************************************/
        BEGIN
	    if (scan2_white (RIGHT_BRACE, COMMENT))
	    BEGIN
	      DO_NOTHING;
	    END

#ifdef TRACE
            if (Flag_trace) {
	      TRACE_PR_TOKEN;
	      TRACE_PR (" is a function ");
            }
#endif                      			/* TRACE */

	    lower_case (buffer, buf_ptr1, TOKEN_LEN);
	    fn_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN, BST_FN_ILK,
				    DONT_INSERT);
	    if ( ! hash_found)
	    BEGIN
	      SKIP_TOKEN_UNKNOWN_FUNCTION;
	    END
	    else if (fn_loc == wiz_loc)
	    BEGIN
	      SKIP_RECURSIVE_TOKEN;
	    END
	    else
	    BEGIN

#ifdef TRACE
              if (Flag_trace) {
	        TRACE_PR ("of type ");
	        trace_pr_fn_class (fn_loc);
	        TRACE_PR_NEWLINE;
              }
#endif                      			/* TRACE */

	      INSERT_FN_LOC (fn_loc);
	    END
	  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 199 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

        break;
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 189 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

Next_Token_Label:
    EAT_BST_WHITE_AND_EOF_CHECK ("function");
  END

/***************************************************************************
 * WEB section number:	200
 * ~~~~~~~~~~~~~~~~~~~
 * Now we add the |end_of_def| special marker, make sure this function will
 * fit into |wiz_functions|, and put it there.
 ***************************************************************************/
  BEGIN
      INSERT_FN_LOC (END_OF_DEF);
      while ((single_ptr + wiz_def_ptr) > Wiz_Fn_Space)
      BEGIN
	BIB_XRETALLOC ("wiz_functions", wiz_functions, HashPtr2_T,
                       Wiz_Fn_Space, Wiz_Fn_Space + WIZ_FN_SPACE);
      END
      FN_INFO[fn_hash_loc] = wiz_def_ptr;
      copy_ptr = 0;
      while (copy_ptr < single_ptr)
      BEGIN
	wiz_functions[wiz_def_ptr] = singl_function[copy_ptr];
	INCR (copy_ptr);
	INCR (wiz_def_ptr);
      END
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 200 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  INCR (buf_ptr2);
Exit_Label: free (singl_function);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 187 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 90
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure scans for an identifier, stopping at the first
 * |illegal_id_char|, or stopping at the first character if it's
 * |numeric|.  It sets the global variable |scan_result| to |id_null| if
 * the identifier is null, else to |white_adjacent| if it ended at a
 * |white_space| character or an end-of-line, else to
 * |specified_char_adjacent| if it ended at one of |char1| or |char2| or
 * |char3|, else to |other_char_adjacent| if it ended at a nonspecified,
 * non|white_space| |illegal_id_char|.  By convention, when some calling
 * code really wants just one or two ``specified'' characters, it merely
 * repeats one of the characters.
 ***************************************************************************/
void          scan_identifier (ASCIICode_T char1, ASCIICode_T char2,
			       ASCIICode_T char3)
BEGIN
  buf_ptr1 = buf_ptr2;
  if (lex_class[SCAN_CHAR] != NUMERIC)
  BEGIN
    while ((id_class[SCAN_CHAR] == LEGAL_ID_CHAR) && (buf_ptr2 < last))
    BEGIN
      INCR (buf_ptr2);
    END
  END
  if (TOKEN_LEN == 0)
  BEGIN
    scan_result = ID_NULL;
  END
  else if ((lex_class[SCAN_CHAR] == WHITE_SPACE) || (buf_ptr2 == last))
  BEGIN
    scan_result = SPECIFIED_CHAR_ADJACENT;
  END
  else if ((SCAN_CHAR == char1) || (SCAN_CHAR == char2)
	    || (SCAN_CHAR == char3))
  BEGIN
    scan_result = SPECIFIED_CHAR_ADJACENT;
  END
  else
  BEGIN
    scan_result = OTHER_CHAR_ADJACENT;
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 90 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 93
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure scans for an integer, stopping at the first nondigit;
 * it sets the value of |token_value| accordingly.  It returns |true| if
 * the token was a legal integer (i.e., consisted of an optional
 * |minus_sign| followed by one or more digits).
 ***************************************************************************/
Boolean_T         scan_integer (void)
BEGIN
  Integer8_T		sign_length;
  Boolean_T		scan_integer;

  buf_ptr1 = buf_ptr2;
  if (SCAN_CHAR == MINUS_SIGN)
  BEGIN
    sign_length = 1;
    INCR (buf_ptr2);
  END
  else
  BEGIN
    sign_length = 0;
  END
  token_value = 0;
  while ((lex_class[SCAN_CHAR] == NUMERIC) && (buf_ptr2 < last))
  BEGIN
    token_value = (token_value * 10) + CHAR_VALUE;
    INCR (buf_ptr2);
  END
  if (NEGATIVE)
  BEGIN
    token_value = -token_value;
  END
  if (TOKEN_LEN == sign_length)
  BEGIN
    scan_integer = FALSE;
  END
  else
  BEGIN
    scan_integer = TRUE;
  END
  return(scan_integer);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  93 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 92
 * ~~~~~~~~~~~~~~~~~~~
 * This function scans for a nonnegative integer, stopping at the first
 * nondigit; it sets the value of |token_value| accordingly.  It returns
 * |true| if the token was a legal nonnegative integer (i.e., consisted
 * of one or more digits).
 ***************************************************************************/
Boolean_T         scan_nonneg_integer (void)
BEGIN
  Boolean_T	  scan_nonneg_integer;

  buf_ptr1 = buf_ptr2;
  token_value = 0;
  while ((lex_class[SCAN_CHAR] == NUMERIC) && (buf_ptr2 < last))
  BEGIN
    token_value = (token_value * 10) + CHAR_VALUE;
    INCR (buf_ptr2);
  END
  if (TOKEN_LEN == 0)
  BEGIN
    scan_nonneg_integer = FALSE;
  END
  else
  BEGIN
    scan_nonneg_integer = TRUE;
  END
  return (scan_nonneg_integer);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  92 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 94
 * ~~~~~~~~~~~~~~~~~~~
 * This function scans over |white_space| characters, stopping either at
 * the first nonwhite character or the end of the line, respectively
 * returning |true| or |false|.
 ***************************************************************************/
Boolean_T         scan_white_space (void)
BEGIN
  Boolean_T	  scan_white_space;

  while ((lex_class[SCAN_CHAR] == WHITE_SPACE) && (buf_ptr2 < last))
  BEGIN
    INCR (buf_ptr2);
  END
  if (buf_ptr2 < last)
  BEGIN
    scan_white_space = TRUE;
  END
  else
  BEGIN
    scan_white_space = FALSE;
  END
  return (scan_white_space);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  94 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 186
 * ~~~~~~~~~~~~~~~~~~~
 * And another.
 ***************************************************************************/
void          skip_illegal_stuff_after_token (void)
BEGIN
  PRINT2 ("\"%c\" can't follow a literal", xchr[SCAN_CHAR]);
  skip_token_print ();
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 186 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 404
 * ~~~~~~~~~~~~~~~~~~~
 * When we come here |sp_ptr| is just past the |left_brace|, and when we
 * leave it's either at |sp_end| or just past the matching |right_brace|.
 ***************************************************************************/
void          skip_stuff_at_sp_brace_level_gr (void)
BEGIN
  while ((sp_brace_level > 1) && (sp_ptr < sp_end))
  BEGIN
    if (str_pool[sp_ptr] == RIGHT_BRACE)
    BEGIN
      DECR (sp_brace_level);
    END
    else if (str_pool[sp_ptr] == LEFT_BRACE)
    BEGIN
      INCR (sp_brace_level);
    END
    INCR (sp_ptr);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 404 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 183
 * ~~~~~~~~~~~~~~~~~~~
 * We're about to start scanning tokens in a function definition.  When a
 * function token is illegal, we skip until it ends; a |white_space|
 * character, an end-of-line, a |right_brace|, or a |comment| marks the
 * end of the current token.
 ***************************************************************************/
void          skip_token_print (void)
BEGIN
  PRINT ("-");
  bst_ln_num_print ();
  mark_error ();
  if (scan2_white (RIGHT_BRACE, COMMENT))
  BEGIN
    DO_NOTHING;
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 183 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 185
 * ~~~~~~~~~~~~~~~~~~~
 * Here's another macro for saving some space when there's a problem with
 * a token.
 ***************************************************************************/
void          skp_token_unknown_function_prin (void)
BEGIN
  PRINT_TOKEN;
  PRINT (" is an unknown function");
  skip_token_print ();
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 185 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 58
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure copies file name |file_name| into the beginning of
 * |name_of_file|, if it will fit.  It also sets the global variable
 * |name_length| to the appropriate value.
 *
 * NOTE: because C arrays start at index 0, not 1, the subscripts of array
 *	 |name_of_file| are generally 1 less than those in the WEB source.
 ***************************************************************************/
void          start_name (StrNumber_T file_name)
BEGIN
  PoolPointer_T       p_ptr;

  free (name_of_file);
  name_of_file = (unsigned char *) mymalloc (LENGTH (file_name) + 5, "name_of_file");
  name_ptr = 0;
  p_ptr = str_start[file_name];
  while (p_ptr < str_start[file_name + 1])
  BEGIN
    name_of_file[name_ptr] = CHR (str_pool[p_ptr]);
    INCR (name_ptr);
    INCR (p_ptr);
  END
  name_length = LENGTH (file_name);
  name_of_file[name_length] = 0;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  58 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 56
 * ~~~~~~~~~~~~~~~~~~~
 * This subroutine compares string |s| with another string that appears
 * in the buffer |buf| between positions |bf_ptr| and |bf_ptr+len-1|; the
 * result is |true| if and only if the strings are equal.
 ***************************************************************************/
Boolean_T         str_eq_buf (StrNumber_T s, BufType_T buf,
			      BufPointer_T bf_ptr, BufPointer_T len)
BEGIN
  BufPointer_T		i;
  PoolPointer_T		j;
  Boolean_T		str_eq_buf;

  if (LENGTH (s) != len)
  BEGIN
    str_eq_buf = FALSE;
    goto Exit_Label;
  END
  i = bf_ptr;
  j = str_start[s];
  while (j < str_start[s + 1])
  BEGIN
    if (str_pool[j] != buf[i])
    BEGIN
      str_eq_buf = FALSE;
      goto Exit_Label;
    END
    INCR (i);
    INCR (j);
  END
  str_eq_buf = TRUE;
Exit_Label:
  return (str_eq_buf);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  56 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 57
 * ~~~~~~~~~~~~~~~~~~~
 * This subroutine compares two |str_pool| strings and returns true
 * |true| if and only if the strings are equal.
 ***************************************************************************/
Boolean_T         str_eq_str (StrNumber_T s1, StrNumber_T s2)
BEGIN
  Boolean_T	  str_eq_str;

  if (LENGTH (s1) != LENGTH (s2))
  BEGIN
    str_eq_str = FALSE;
    goto Exit_Label;
  END
  p_ptr1 = str_start[s1];
  p_ptr2 = str_start[s2];
  while (p_ptr1 < str_start[s1 + 1])
  BEGIN
    if (str_pool[p_ptr1] != str_pool[p_ptr2])
    BEGIN
      str_eq_str = FALSE;
      goto Exit_Label;
    END
    INCR (p_ptr1);
    INCR (p_ptr2);
  END
  str_eq_str = TRUE;
Exit_Label:
  return (str_eq_str);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  57 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 68
 * ~~~~~~~~~~~~~~~~~~~
 * Here is the subroutine that searches the hash table for a
 * (string,~|str_ilk|) pair, where the string is of length |l>=0| and
 * appears in |buffer[j..(j+l-1)]|.  If it finds the pair, it returns the
 * corresponding hash-table location and sets the global variable
 * |hash_found| to |true|.  Otherwise it sets |hash_found| to |false|,
 * and if the parameter |insert_it| is |true|, it inserts the pair into
 * the hash table, inserts the string into |str_pool| if not previously
 * encountered, and returns its location.  Note that two different pairs
 * can have the same string but different |str_ilk|s, in which case the
 * second pair encountered, if |insert_it| were |true|, would be inserted
 * into the hash table though its string wouldn't be inserted into
 * |str_pool| because it would already be there.
 ***************************************************************************/
HashLoc_T         str_lookup (BufType_T buf, BufPointer_T j, BufPointer_T l,
			      StrIlk_T ilk, Boolean_T insert_it)
BEGIN
  HashLoc_T		str_lookup;
  long                  h;
  HashLoc_T		p;
  BufPointer_T		k;
  Boolean_T		old_string;
  StrNumber_T		str_num;

/***************************************************************************
 * WEB section number:	69
 * ~~~~~~~~~~~~~~~~~~~
 * The value of |Hash_Prime| should be roughly 85\% of |Hash_Size|, and
 * it should be a prime number
 * (it should also be less than $2^14 + 2^6 = 16320$ because of
 * WEB's simple-macro bound).  The theory of hashing tells us to expect
 * fewer than two table probes, on the average, when the search is
 * successful.
 *
 * NOTE: the limit imposed on the value of Hash_Prime no longer applies
 *       because we are using C #defines.
 ***************************************************************************/
  BEGIN
    h = 0;
    k = j;
    while (k < (j + l))
    BEGIN
      h = h + h + buf[k];
      while (h >= Hash_Prime)
	h = h - Hash_Prime;
      INCR (k);
    END
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  69 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  p = h + HASH_BASE;
  hash_found = FALSE;
  old_string = FALSE;
  str_num = 0;	/* avoid uninitialized warning */
  LOOP
  BEGIN

/***************************************************************************
 * WEB section number:	70
 * ~~~~~~~~~~~~~~~~~~~
 * Here we handle the case in which we've already encountered this
 * string; note that even if we have, we'll still have to insert the pair
 * into the hash table if |str_ilk| doesn't match.
 ***************************************************************************/
    BEGIN
      if (hash_text[p] > 0)
      BEGIN
	if (str_eq_buf (hash_text[p], buf, j, l))
	BEGIN
	  if (hash_ilk[p] == ilk)
	  BEGIN
	    hash_found = TRUE;
	    goto Str_Found_Label;
	  END
	  else
	  BEGIN
	    old_string = TRUE;
	    str_num = hash_text[p];
	  END
	END
      END
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  70 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    if (hash_next[p] == EMPTY)
    BEGIN
      if ( ! insert_it)
      BEGIN
        goto Str_Not_Found_Label;
      END

/***************************************************************************
 * WEB section number:	71
 * ~~~~~~~~~~~~~~~~~~~
 * This code inserts the pair in the appropriate unused location.
 ***************************************************************************/
      BEGIN
	if (hash_text[p] > 0)
	BEGIN
	  REPEAT
	  BEGIN
	    if (HASH_IS_FULL)
	    BEGIN
	      BIBTEX_OVERFLOW ("hash size ", Hash_Size);
	    END
	    DECR (hash_used);
	  END UNTIL (hash_text[hash_used] == 0);
	  hash_next[p] = hash_used;
	  p = hash_used;
	END
	if (old_string)
	BEGIN
	  hash_text[p] = str_num;
	END
	else
	BEGIN
	  STR_ROOM (l);
	  k = j;
	  while (k < (j + l))
	  BEGIN
	    APPEND_CHAR (buf[k]);
	    INCR (k);
	  END
	  hash_text[p] = make_string ();
	END
	hash_ilk[p] = ilk;
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  71 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      goto Str_Found_Label;
    END
    p = hash_next[p];
  END
Str_Not_Found_Label: DO_NOTHING;
Str_Found_Label: str_lookup = p;
  return (str_lookup);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  68 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 300
 * ~~~~~~~~~~~~~~~~~~~
 * The |swap| procedure exchanges the two elements its arguments point
 * to.
 ***************************************************************************/
void          swap (CiteNumber_T swap1, CiteNumber_T swap2)
BEGIN
  CiteNumber_T      innocent_bystander;

  innocent_bystander = SORTED_CITES[swap2];
  SORTED_CITES[swap2] = SORTED_CITES[swap1];
  SORTED_CITES[swap1] = innocent_bystander;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 300 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 456
 * ~~~~~~~~~~~~~~~~~~~
 * Here we print |trace| and/or |stat| information, if desired.
 ***************************************************************************/
void          trace_and_stat_printing (void)
BEGIN

#ifdef TRACE

  if (Flag_trace) {
  
/***************************************************************************
 * WEB section number:	457
 * ~~~~~~~~~~~~~~~~~~~
 * This prints information obtained from the .aux file about the
 * other files.
 ***************************************************************************/
    BEGIN
      if (num_bib_files == 1)
      BEGIN
        TRACE_PR_LN ("The 1 database file is");
      END
      else
      BEGIN
        TRACE_PR_LN2 ("The %ld database files are", (long) num_bib_files);
      END
      if (num_bib_files == 0)
      BEGIN
        TRACE_PR_LN ("   undefined");
      END
      else
      BEGIN
        bib_ptr = 0;
        while (bib_ptr < num_bib_files)
        BEGIN
    	  TRACE_PR ("   ");
  	  TRACE_PR_POOL_STR (CUR_BIB_STR);
  	  TRACE_PR_POOL_STR (s_bib_extension);
  	  TRACE_PR_NEWLINE;
	  INCR (bib_ptr);
        END
      END
      TRACE_PR ("The style file is ");
      if (bst_str == 0)
      BEGIN
        TRACE_PR_LN ("undefined");
      END
      else
      BEGIN
        TRACE_PR_POOL_STR (bst_str);
        TRACE_PR_POOL_STR (s_bst_extension);
        TRACE_PR_NEWLINE;
      END
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 457 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	458
 * ~~~~~~~~~~~~~~~~~~~
 * In entry-sorted order, this prints an entry's |cite_list| string and,
 * indirectly, its entry type and entry variables.
 ***************************************************************************/
    BEGIN
      if (all_entries)
      BEGIN
        TRACE_PR2 ("all_marker=%ld, ", (long) all_marker);
      END
      if (read_performed)
      BEGIN
        TRACE_PR_LN2 ("old_num_cites=%ld", (long) old_num_cites);
      END
      else
      BEGIN
        TRACE_PR_NEWLINE;
      END
      TRACE_PR2 ("The %ld", (long) num_cites);
      if (num_cites == 1)
      BEGIN
        TRACE_PR_LN (" entry:");
      END
      else
      BEGIN
        TRACE_PR_LN (" entries:");
      END
      if (num_cites == 0)
      BEGIN
        TRACE_PR_LN ("   undefined");
      END
      else
      BEGIN
        sort_cite_ptr = 0;
        while (sort_cite_ptr < num_cites)
        BEGIN
  	  if ( ! read_completed)
	  BEGIN
	    cite_ptr = sort_cite_ptr;
	  END
  	  else
  	  BEGIN
	    cite_ptr = SORTED_CITES[sort_cite_ptr];
	  END
	  TRACE_PR_POOL_STR (CUR_CITE_STR);
	  if (read_performed)

/***************************************************************************
 * WEB section number:	458
 * ~~~~~~~~~~~~~~~~~~~
 * This prints information gathered while reading the .bst and
 * .bib files.
 ***************************************************************************/
  	  BEGIN
	    TRACE_PR (", entry-type ");
	    if (type_list[cite_ptr] == UNDEFINED)
	    BEGIN
	      TRACE_PR ("unknown");
	    END
	    else if (type_list[cite_ptr] == EMPTY)
	    BEGIN
	      TRACE_PR ("--- no type found");
	    END
	    else
	    BEGIN
	      TRACE_PR_POOL_STR (hash_text[type_list[cite_ptr]]);
	    END
	    TRACE_PR_LN (", has entry strings");

/***************************************************************************
 * WEB section number:	460
 * ~~~~~~~~~~~~~~~~~~~
 * This prints, for the current entry, the strings declared by the
 * entry command.
 ***************************************************************************/
	    BEGIN
	      if (num_ent_strs == 0)
	      BEGIN
	        TRACE_PR_LN ("    undefined");
	      END
	      else if ( ! read_completed)
	      BEGIN
	        TRACE_PR_LN ("    uninitialized");
	      END
	      else
	      BEGIN
	        str_ent_ptr = cite_ptr * num_ent_strs;
	        while (str_ent_ptr < ((cite_ptr + 1) * num_ent_strs))
	        BEGIN
	  	  ent_chr_ptr = 0;
		  TRACE_PR ("    \"");

		  while (ENTRY_STRS(str_ent_ptr, ent_chr_ptr) != END_OF_STRING)
		  BEGIN
		    TRACE_PR2 ("%c",
                               xchr[ENTRY_STRS(str_ent_ptr, ent_chr_ptr)]);
		    INCR (ent_chr_ptr);
		  END
		  TRACE_PR_LN ("\"");
		  INCR (str_ent_ptr);
	        END
	      END
	    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 460 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    TRACE_PR ("  has entry integers");

/***************************************************************************
 * WEB section number:	461
 * ~~~~~~~~~~~~~~~~~~~
 * This prints, for the current entry, the integers declared by the
 * entry command.
 ***************************************************************************/
  	    BEGIN
	      if (num_ent_ints == 0)
	      BEGIN
	        TRACE_PR (" undefined");
	      END
	      else if ( ! read_completed)
	      BEGIN
	        TRACE_PR (" uninitialized");
	      END
	      else
	      BEGIN
	        int_ent_ptr = cite_ptr * num_ent_ints;
	        while (int_ent_ptr < ((cite_ptr + 1) * num_ent_ints))
	        BEGIN
		  TRACE_PR2 (" %ld", (long) entry_ints[int_ent_ptr]);
		  INCR (int_ent_ptr);
	        END
	      END
	      TRACE_PR_NEWLINE;
	    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 461 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    TRACE_PR_LN ("  and has fields");

/***************************************************************************
 * WEB section number:	462
 * ~~~~~~~~~~~~~~~~~~~
 * This prints the fields stored for the current entry.
 ***************************************************************************/
	    BEGIN
	      if ( ! read_performed)
	      BEGIN
	        TRACE_PR_LN ("    uninitialized");
	      END
	      else
	      BEGIN
	        field_ptr = cite_ptr * num_fields;
	        field_end_ptr = field_ptr + num_fields;
	        no_fields = TRUE;
	        while (field_ptr < field_end_ptr)
	        BEGIN
		  if (field_info[field_ptr] != MISSING)
		  BEGIN
		    TRACE_PR ("    \"");
		    TRACE_PR_POOL_STR (field_info[field_ptr]);
		    TRACE_PR_LN ("\"");
		    no_fields = FALSE;
		  END
		  INCR (field_ptr);
	        END
	        if (no_fields)
	        BEGIN
		  TRACE_PR_LN ("    missing");
	        END
	      END
	    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 462 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  	  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 459 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	  else
	  BEGIN
	    TRACE_PR_NEWLINE;
	  END
	  INCR (sort_cite_ptr);
        END
      END
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 458 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	463
 * ~~~~~~~~~~~~~~~~~~~
 * This gives all the |wiz_defined| functions that appeared in the
 * .bst file.
 ***************************************************************************/
    BEGIN
      TRACE_PR_LN ("The wiz-defined functions are");
      if (wiz_def_ptr == 0)
      BEGIN
        TRACE_PR_LN ("   nonexistent");
      END
      else
      BEGIN
        wiz_fn_ptr = 0;

        while (wiz_fn_ptr < wiz_def_ptr)
        BEGIN
	  if (wiz_functions[wiz_fn_ptr] == END_OF_DEF)
	  BEGIN
	    TRACE_PR_LN2 ("%ld--end-of-def--", (long) wiz_fn_ptr);
	  END
	  else if (wiz_functions[wiz_fn_ptr] == QUOTE_NEXT_FN)
	  BEGIN
	    TRACE_PR2 ("%ld  quote_next_function    ", (long) wiz_fn_ptr);
	  END
	  else
	  BEGIN
	    TRACE_PR2 ("%ld  `", (long) wiz_fn_ptr);
	    TRACE_PR_POOL_STR (hash_text[wiz_functions[wiz_fn_ptr]]);
	    TRACE_PR_LN ("'");
	  END
	  INCR (wiz_fn_ptr);
        END
      END
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 463 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	464
 * ~~~~~~~~~~~~~~~~~~~
 * This includes all the `static' strings (that is, those that are also
 * in the hash table), but none of the dynamic strings (that is, those
 * put on the stack while executing .bst commands).
 ***************************************************************************/
    BEGIN
      TRACE_PR_LN ("The string pool is");
      str_num = 1;

      while (str_num < str_ptr)
      BEGIN
        TRACE_PR3 ("%4ld%6ld \"", (long) str_num, (long) str_start[str_num]);
        TRACE_PR_POOL_STR (str_num);
        TRACE_PR_LN ("\"");
        INCR (str_num);
      END
    END
  }                             /* end if (Flag_trace) */

/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 464 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

#endif                      			/* TRACE */


#ifdef STAT

/***************************************************************************
 * WEB section number:	465
 * ~~~~~~~~~~~~~~~~~~~
 * These statistics can help determine how large some of the constants
 * should be and can tell how useful certain |built_in| functions are.
 * They are written to the same files as tracing information.
 ***************************************************************************/
  if (Flag_stats) {
    BEGIN
      STAT_PR2 ("You've used %ld", (long) num_cites);
      if (num_cites == 1)
      BEGIN
        STAT_PR_LN (" entry,");
      END
      else
      BEGIN
        STAT_PR_LN (" entries,");
      END
      STAT_PR_LN2 ("            %ld wiz_defined-function locations,",
  		  (long) wiz_def_ptr);
      STAT_PR_LN3 ("            %ld strings with %ld characters,",
  		  (long) str_ptr, (long) str_start[str_ptr]);
      blt_in_ptr = 0;
      total_ex_count = 0;
      while (blt_in_ptr < NUM_BLT_IN_FNS)
      BEGIN
        total_ex_count = total_ex_count + execution_count[blt_in_ptr];
        INCR (blt_in_ptr);
      END
      STAT_PR_LN2 ("and the built_in function-call counts, %ld in all, are:",
  		  (long) total_ex_count);
      blt_in_ptr = 0;
      while (blt_in_ptr < NUM_BLT_IN_FNS)
      BEGIN
        STAT_PR_POOL_STR (hash_text[blt_in_loc[blt_in_ptr]]);
        STAT_PR_LN2 (" -- %ld", (long) execution_count[blt_in_ptr]);
        INCR (blt_in_ptr);
      END
    END
  }                             /* end if (Flag_stats) */

/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 465 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

#endif                      			/* STAT */
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 456 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 159
 * ~~~~~~~~~~~~~~~~~~~
 * This version is for printing when in |trace| mode.
 ***************************************************************************/
#ifdef TRACE
void          trace_pr_fn_class (HashLoc_T fn_loc)
BEGIN

  if (!Flag_trace)
    return;

  switch (fn_type[fn_loc])
  BEGIN
    case BUILT_IN:
      TRACE_PR ("built-in");
      break;
    case WIZ_DEFINED:
      TRACE_PR ("wizard-defined");
      break;
    case INT_LITERAL:
      TRACE_PR ("integer-literal");
      break;
    case STR_LITERAL:
      TRACE_PR ("string-literal");
      break;
    case FIELD:
      TRACE_PR ("field");
      break;
    case INT_ENTRY_VAR:
      TRACE_PR ("integer-entry-variable");
      break;
    case STR_ENTRY_VAR:
      TRACE_PR ("string-entry-variable");
      break;
    case INT_GLOBAL_VAR:
      TRACE_PR ("integer-global-variable");
      break;
    case STR_GLOBAL_VAR:
      TRACE_PR ("string-global-variable");
      break;
    default:
      unknwn_function_class_confusion ();
      break;
  END
END
#endif                      			/* TRACE */
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 159 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/





/***************************************************************************
 * WEB section number:	 157
 * ~~~~~~~~~~~~~~~~~~~
 * Here's another bug report.
 ***************************************************************************/
void          unknwn_function_class_confusion (void)
BEGIN
  CONFUSION ("Unknown function class");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 157 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 310
 * ~~~~~~~~~~~~~~~~~~~
 * More bug complaints, this time about bad literals.
 ***************************************************************************/
void          unknwn_literal_confusion (void)
BEGIN
  CONFUSION ("Unknown literal type");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 310 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 63
 * ~~~~~~~~~~~~~~~~~~~
 * This system-independent procedure is the same as the previous except
 * that it converts lower- to upper-case letters.
 ***************************************************************************/
#ifndef UTF_8
void          upper_case (BufType_T buf, BufPointer_T bf_ptr,
			  BufPointer_T len)
BEGIN
  BufPointer_T      i;

  if (len > 0)
  BEGIN
    for (i = bf_ptr; i <= (bf_ptr+len-1); i++)
    BEGIN

#ifdef SUPPORT_8BIT
      if (IsLower (buf[i]))
      BEGIN
	buf[i] = ToUpper(buf[i]);
      END
#else                           /* NOT SUPPORT_8BIT */
      if ((buf[i] >= 'a') && (buf[i] <= 'z'))
      BEGIN
        buf[i] = buf[i] - CASE_DIFFERENCE;
      END
#endif                          /* SUPPORT_8BIT */

    END
  END
END
#else
/*
This foction "upper_case_uni" is for prcessing the character UTF-8. 
It's like lower_case_uni.                               23/sep/2009
*/

BufPointer_T          upper_case_uni (BufType_T buf, BufPointer_T bf_ptr,
			  BufPointer_T len)
BEGIN
//	printf("O~ lala~~");
	UChar target[BUF_SIZE+1];
	int32_t tarcap=BUF_SIZE+1;
	int32_t tarlen = icu_toUChars(buf, bf_ptr, len, target, tarcap);

	UChar tarup[BUF_SIZE+1];
	int32_t tucap=BUF_SIZE+1;
	int32_t tulen=icu_strToUpper(tarup, tucap,target, tarlen);
	
	unsigned char dest[BUF_SIZE+1];
	int32_t destcap= BUF_SIZE-bf_ptr;
	
	int32_t tblen=icu_fromUChars(dest, destcap, (const UChar *) tarup, tulen);

  BufPointer_T      i;
  if (tblen > 0)
  BEGIN
	if (len!=tblen)
	BEGIN
		unsigned char tmp[BUF_SIZE+1];
		BufPointer_T      tmppr=0;
    		for (i=bf_ptr+len;i<=(BUF_SIZE-tblen+len);i++)
		BEGIN
			tmp[tmppr]=buf[i];
			tmppr++;
		END	
		i=bf_ptr+tblen;
		tmppr=0;
		for (tmppr=0;tmppr<=(BUF_SIZE-bf_ptr-tblen);tmppr++)
		BEGIN
			buf[i]=tmp[tmppr];
			i++;
		END
	END
    for (i = 0; i <= (tblen - 1); i++)
    BEGIN
	
	buf[i+bf_ptr]=dest[i];
    END
  END
	return tblen;
END

/*
This function is for transform Unicode string to up case. 23/sep/2009
*/

int32_t icu_strToUpper(UChar * tarup, int32_t tucap, UChar * target, int32_t tarlen)
BEGIN
	int32_t tulen;
	UErrorCode err1 = U_ZERO_ERROR;
	if (!U_SUCCESS(err1))
	BEGIN
		printf("Error in icu_strToUpper?\n");
	END
	if (Flag_language)
	{
		tulen=u_strToUpper(tarup,tucap, target,tarlen,Str_language,&err1);
	}
	else
	{
		tulen=u_strToUpper(tarup,tucap, target,tarlen,NULL,&err1);
	}
	if (!U_SUCCESS(err1))
	BEGIN
		printf("Error in icu_strToUpper.\n");
#ifdef TRACE
		if (Flag_trace)
			TRACE_PR_LN ("Error in icu_strToUpper");
#endif                      			/* TRACE */
	END

	return tulen;
END
#endif
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  63 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 401
 * ~~~~~~~~~~~~~~~~~~~
 * The last name starts just past the last token, before the first
 * |comma| (if there is no |comma|, there is deemed to be one at the end
 * of the string), for which there exists a first brace-level-0 letter
 * (or brace-level-1 special character), and it's in lower case, unless
 * this last token is also the last token before the |comma|, in which
 * case the last name starts with this token (unless this last token is
 * connected by a |sep_char| other than a |tie| to the previous token, in
 * which case the last name starts with as many tokens earlier as are
 * connected by non|tie|s to this last one (except on Tuesdays
 * $\ldots\,$), although this module never sees such a case).  Note that
 * if there are any tokens in either the von or last names, then the last
 * name has at least one, even if it starts with a lower-case letter.
 ***************************************************************************/
void          von_name_ends_and_last_name_sta (void)
BEGIN
  von_end = last_end - 1;
  while (von_end > von_start)
  BEGIN
    name_bf_ptr = name_tok[von_end - 1];
    name_bf_xptr = name_tok[von_end];
    if (von_token_found ())
    BEGIN
      goto Exit_Label;
    END
    DECR (von_end);
  END
Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 401 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 397
 * ~~~~~~~~~~~~~~~~~~~
 * It's a von token if there exists a first brace-level-0 letter (or
 * brace-level-1 special character), and it's in lower case; in this case
 * we return |true|.  The token is in |name_buf|, starting at
 * |name_bf_ptr| and ending just before |name_bf_xptr|.
 ***************************************************************************/
Boolean_T         von_token_found (void)
BEGIN
  Boolean_T    von_token_found;

  nm_brace_level = 0;
  von_token_found = FALSE;
  while (name_bf_ptr < name_bf_xptr)
  BEGIN

#ifdef SUPPORT_8BIT
    if IsUpper(NAME_BUF[name_bf_ptr])
    BEGIN
      goto Exit_Label;
    END
    else if IsLower(NAME_BUF[name_bf_ptr])
    BEGIN
      RETURN_VON_FOUND;
    END
#else                           /* NOT SUPPORT_8BIT */
    if ((NAME_BUF[name_bf_ptr] >= 'A') && (NAME_BUF[name_bf_ptr] <= 'Z'))
    BEGIN
      goto Exit_Label;
    END
    else if ((NAME_BUF[name_bf_ptr] >= 'a') && (NAME_BUF[name_bf_ptr] <= 'z'))
    BEGIN
      RETURN_VON_FOUND;
    END
#endif                          /* SUPPORT_8BIT */

    else if (NAME_BUF[name_bf_ptr] == LEFT_BRACE)
    BEGIN
      INCR (nm_brace_level);
      INCR (name_bf_ptr);
      if (((name_bf_ptr + 2) < name_bf_xptr)
	    && (NAME_BUF[name_bf_ptr] == BACKSLASH))

/***************************************************************************
 * WEB section number:	398
 * ~~~~~~~~~~~~~~~~~~~
 * When we come here |name_bf_ptr| is just past the |left_brace|,
 * but we always leave by |return|ing.
 ***************************************************************************/
      BEGIN
        INCR (name_bf_ptr);
        name_bf_yptr = name_bf_ptr;
        while ((name_bf_ptr < name_bf_xptr)
		&& (lex_class[NAME_BUF[name_bf_ptr]] == ALPHA))
        BEGIN
          INCR (name_bf_ptr);
        END
        control_seq_loc = str_lookup (NAME_BUF, name_bf_yptr,
					name_bf_ptr - name_bf_yptr,
					CONTROL_SEQ_ILK, DONT_INSERT);
        if (hash_found)

/***************************************************************************
 * WEB section number:	399
 * ~~~~~~~~~~~~~~~~~~~
 * The accented or foreign character is either `\i' or `\j' or
 * one of the eleven alphabetic foreign characters in Table~3.2 of the
 * \LaTeX\ manual.
 ***************************************************************************/
        BEGIN
          switch (ilk_info[control_seq_loc])
          BEGIN
            case N_OE_UPPER:
            case N_AE_UPPER:
            case N_AA_UPPER:
            case N_O_UPPER:
            case N_L_UPPER:
              goto Exit_Label;
            case N_I:
            case N_J:
            case N_OE:
            case N_AE:
            case N_AA:
            case N_O:
            case N_L:
            case N_SS:
	      RETURN_VON_FOUND;
            default:
              CONFUSION ("Control-sequence hash error");
          END
        END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 399 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

        while ((name_bf_ptr < name_bf_xptr) && (nm_brace_level > 0))
        BEGIN

#ifdef SUPPORT_8BIT
	  if IsUpper(NAME_BUF[name_bf_ptr])
	  BEGIN
	    goto Exit_Label;
	  END
	  else if IsLower(NAME_BUF[name_bf_ptr])
	  BEGIN
	    RETURN_VON_FOUND;
	  END
#else                           /* NOT SUPPORT_8BIT */
	  if ((NAME_BUF[name_bf_ptr] >= 'A') && (NAME_BUF[name_bf_ptr] <= 'Z'))
	  BEGIN
	    goto Exit_Label;
	  END
	  else if ((NAME_BUF[name_bf_ptr] >= 'a') 
                    && (NAME_BUF[name_bf_ptr] <= 'z'))
	  BEGIN
	    RETURN_VON_FOUND;
	  END
#endif                          /* SUPPORT_8BIT */

	  else if (NAME_BUF[name_bf_ptr] == RIGHT_BRACE)
	  BEGIN
	    DECR (nm_brace_level);
	  END
	  else if (NAME_BUF[name_bf_ptr] == LEFT_BRACE)
	  BEGIN
	    INCR (nm_brace_level);
	  END
	  INCR (name_bf_ptr);
	END
	goto Exit_Label;
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 398 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      else

/***************************************************************************
 * WEB section number:	400
 * ~~~~~~~~~~~~~~~~~~~
 * When we come here |name_bf_ptr| is just past the |left_brace|; when we
 * leave it's either at |name_bf_xptr| or just past the matching
 * |right_brace|.
 ***************************************************************************/
      BEGIN
        while ((nm_brace_level > 0) && (name_bf_ptr < name_bf_xptr))
        BEGIN
          if (NAME_BUF[name_bf_ptr] == RIGHT_BRACE)
          BEGIN
            DECR (nm_brace_level);
          END
          else if (NAME_BUF[name_bf_ptr] == LEFT_BRACE)
          BEGIN
            INCR (nm_brace_level);
          END
          INCR (name_bf_ptr);
        END
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 400 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    END
    else
    BEGIN
      INCR (name_bf_ptr);
    END
  END
Exit_Label: DO_NOTHING;
  return (von_token_found);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 397 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 360
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function add.period$ pops the top (string)
 * literal, adds a |period| to a nonnull string if its last
 * non|right_brace| character isn't a |period|, |question_mark|, or
 * |exclamation_mark|, and pushes this resulting string back onto the
 * stack.  If the literal isn't a string, it complains and pushes the
 * null string.
 ***************************************************************************/
void          x_add_period (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  if (pop_typ1 != STK_STR)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_STR);
    push_lit_stk (s_null, STK_STR);
  END
  else if (LENGTH (pop_lit1) == 0)
  BEGIN
    push_lit_stk (s_null, STK_STR);
  END
  else

/***************************************************************************
 * WEB section number:	361
 * ~~~~~~~~~~~~~~~~~~~
 * Here we scan backwards from the end of the string, skipping
 * non|right_brace| characters, to see if we have to add the |period|.
 ***************************************************************************/
  BEGIN
    sp_ptr = str_start[pop_lit1 + 1];
    sp_end = str_start[pop_lit1];
    while (sp_ptr > sp_end)
    BEGIN
      DECR (sp_ptr);
      if (str_pool[sp_ptr] != RIGHT_BRACE)
      BEGIN
        goto Loop_Exit_Label;
      END
    END
Loop_Exit_Label:
    switch (str_pool[sp_ptr])
    BEGIN
      case PERIOD:
      case QUESTION_MARK:
      case EXCLAMATION_MARK:
	REPUSH_STRING;
        break;
      default:

/***************************************************************************
 * WEB section number:	362
 * ~~~~~~~~~~~~~~~~~~~
 * Ok guys, we really have to do it.
 ***************************************************************************/
	BEGIN
	  if (pop_lit1 < cmd_str_ptr)
	  BEGIN
	    STR_ROOM (LENGTH (pop_lit1) + 1);
	    sp_ptr = str_start[pop_lit1];
	    sp_end = str_start[pop_lit1 + 1];
	    while (sp_ptr < sp_end)
	    BEGIN
	      APPEND_CHAR (str_pool[sp_ptr]);
	      INCR (sp_ptr);
	    END
	  END
	  else
	  BEGIN
	    pool_ptr = str_start[pop_lit1 + 1];
	    STR_ROOM (1);
	  END
	  APPEND_CHAR (PERIOD);
	  push_lit_stk (make_string (), STK_STR);
	  break;
      END
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 362 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 361 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 360 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
