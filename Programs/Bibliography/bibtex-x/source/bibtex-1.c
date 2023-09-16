/*-
******************************************************************************
******************************************************************************
**
**  MODULE
**
**      file: bibtex-1.c
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
**      This is the first of 4 source modules for BibTeX.  The source has
**      been split into 4 parts so that some of the more primitive editors
**      can cope.  This code mimics the BibTeX WEB source as closely as
**      possible and there should be NO system dependent code in any of the 
**      bibtex-#.c modules.
**
**      The functions defined in this module are:
**
**          aux_err_stuff_after_right_brace
**          aux_err_white_space_in_argument
**          aux_input_command
**          bad_argument_token
**          bad_cross_reference_print
**          bib_cmd_confusion
**          bib_equals_sign_print
**          bib_err_print
**          bib_field_too_long_print
**          bib_id_print
**          bib_ln_num_print
**          bib_one_of_two_print
**          bib_unbalanced_braces_print
**          bib_warn_print
**          brace_lvl_one_letters_complaint
**          braces_unbalanced_complaint
**          bst_1print_string_size_exceeded
**          bst_2print_string_size_exceeded
**          bst_cant_mess_with_entries_prin
**          bst_entry_command
**          bst_err_print_and_look_for_blan
**          bst_execute_command
**          bst_ex_warn_print
**          bst_function_command
**          bst_id_print
**          bst_integers_command
**          build_in
**          case_conversion_confusion
**          check_brace_level
**          check_cite_overflow
**          check_command_execution
**          check_field_overflow
**          cite_key_disappeared_confusion
**          compress_bib_white
**          decr_brace_level
**          eat_bib_print
**          eat_bib_white_space
**          eat_bst_print
**          eat_bst_white_space
**          enough_text_chars
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
**      $Log: bibtex-1.c,v $
**      Revision 3.71  1996/08/18  20:47:30  kempson
**      Official release 3.71 (see HISTORY file for details).
**
**      Revision 3.70  1996/04/08  10:08:40  kempson
**      Final documentation & cosmetic changes for official release 3.70.
**
**      Revision 3.5  1995/09/24  20:44:37  kempson
**      Many changes for final beta test version.
**
**      Revision 3.4  1995/04/09  22:15:38  kempson
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
 * WEB section number:	 61
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure copies the default logical area name |area| into the
 * array |name_of_file| starting at position 1, after shifting up the rest
 * of the filename.  It also sets the global variable |name_length| to the
 * appropriate value.
 *
 * REMOVED: |add_area|.
 ***************************************************************************/

/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  61 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 320
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure adds to the execution buffer the given string in
 * |str_pool| if it will fit.  It assumes the global variable
 * |ex_buf_length| gives the length of the current string in |ex_buf|, and
 * thus also gives the location of the next character.
 ***************************************************************************/
void          add_buf_pool (StrNumber_T p_str)
BEGIN
  p_ptr1 = str_start[p_str];
  p_ptr2 = str_start[p_str + 1];
  if ((ex_buf_length + (p_ptr2 - p_ptr1)) > Buf_Size)
  BEGIN
    buffer_overflow ();
  END
  ex_buf_ptr = ex_buf_length;
  while (p_ptr1 < p_ptr2)
  BEGIN
    APPEND_EX_BUF_CHAR(str_pool[p_ptr1]);
    INCR (p_ptr1);
  END
  ex_buf_length = ex_buf_ptr;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 320 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 265
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure adds (or restores) to |cite_list| a cite key; it is
 * called only when |all_entries| is |true| or when adding
 * cross references, and it assumes that |cite_loc| and |lc_cite_loc| are
 * set.  It also INCRs its argument.
 ***************************************************************************/
void          add_database_cite (CiteNumber_T *new_cite)
BEGIN
  check_cite_overflow (*new_cite);
  check_field_overflow (num_fields * (*new_cite + 1));
  cite_list[*new_cite] = hash_text[cite_loc];
  ilk_info[cite_loc] = *new_cite;
  ilk_info[lc_cite_loc] = cite_loc;
  INCR (*new_cite);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 265 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	  60
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure copies file extension |ext| into the array
 * |name_of_file| starting at position |name_length+1|.  It also sets the
 * global variable |name_length| to the appropriate value.
 *
 * NOTE: because C arrays start at index 0, not 1, the subscripts of array
 *	 |name_of_file| are generally 1 less than those in the WEB source.
 ***************************************************************************/
void          add_extension (StrNumber_T ext)
BEGIN
  PoolPointer_T       p_ptr;

  name_ptr = name_length;
  p_ptr = str_start[ext];
  while (p_ptr < str_start[ext + 1])
  BEGIN
    name_of_file[name_ptr] = CHR (str_pool[p_ptr]);
    INCR (name_ptr);
    INCR (p_ptr);
  END
  name_length = name_length + LENGTH (ext);
  name_of_file[name_length] = 0;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  60 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 322
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure adds to the output buffer the given string in
 * |str_pool|.  It assumes the global variable |out_buf_length| gives the
 * length of the current string in |out_buf|, and thus also gives the
 * location for the next character.  If there are enough characters
 * present in the output buffer, it writes one or more lines out to the
 * .bbl file.  It breaks a line only at a |white_space| character,
 * and when it does, it adds two |space|s to the next output line.
 ***************************************************************************/
void          add_out_pool (StrNumber_T p_str)
BEGIN
  BufPointer_T      break_ptr;
  BufPointer_T      end_ptr;
  Boolean_T         break_pt_found;
  Boolean_T         unbreakable_tail;

  p_ptr1 = str_start[p_str];
  p_ptr2 = str_start[p_str + 1];
  if ((out_buf_length + (p_ptr2 - p_ptr1)) > Buf_Size)
  BEGIN
    buffer_overflow ();
  END
  out_buf_ptr = out_buf_length;
  while (p_ptr1 < p_ptr2)
  BEGIN
    out_buf[out_buf_ptr] = str_pool[p_ptr1];
    INCR (p_ptr1);
    INCR (out_buf_ptr);
  END
  out_buf_length = out_buf_ptr;
  unbreakable_tail = FALSE;
  while ((out_buf_length > Max_Print_Line) && ! unbreakable_tail)

/***************************************************************************
 * WEB section number:	 323
 * ~~~~~~~~~~~~~~~~~~~
 * Here we break the line by looking for a |white_space| character,
 * backwards from |out_buf[max_print_line]| until
 * |out_buf[min_print_line]|; we break at the |white_space| and indent
 * the next line two |space|s.  The next module handles things when
 * there's no |white_space| character to break at.  (It seems that the
 * annoyances to the average user of a warning message when there's an
 * output line longer than |max_print_line| outweigh the benefits, so we
 * don't issue such warnings in the current code.)
 ***************************************************************************/
  BEGIN
    end_ptr = out_buf_length;
    out_buf_ptr = Max_Print_Line;
    break_pt_found = FALSE;
    while ((lex_class[out_buf[out_buf_ptr]] != WHITE_SPACE)
        && (out_buf_ptr >= MIN_PRINT_LINE))
    BEGIN
      DECR (out_buf_ptr);
    END
    if (out_buf_ptr == (MIN_PRINT_LINE - 1))

/***************************************************************************
 * WEB section number:	 324
 * ~~~~~~~~~~~~~~~~~~~
 * If there's no |white_space| character up through
 * |out_buf[max_print_line]|, we instead break the line at the first
 * following |white_space| character, if one exists.  And if, starting
 * with that |white_space| character, there are multiple consecutive
 * |white_space| characters, |out_buf_ptr| points to the last of them.
 * If no |white_space| character exists, we haven't found a viable break
 * point, so we don't break the line (yet).
 ***************************************************************************/
    BEGIN
      out_buf_ptr = Max_Print_Line + 1;
      while (out_buf_ptr < end_ptr)
      BEGIN
#ifdef UTF_8
        if (lex_class[out_buf[out_buf_ptr]] == WHITE_SPACE)
        BEGIN
          UChar32 ch;
          U8_GET_OR_FFFD(out_buf, out_buf_ptr-4, out_buf_ptr-1, -1, ch);
          switch ( ublock_getCode(ch) )
          BEGIN
            case UBLOCK_BASIC_LATIN:
            case UBLOCK_LATIN_1_SUPPLEMENT:
            case UBLOCK_LATIN_EXTENDED_A:
            case UBLOCK_LATIN_EXTENDED_B:
            case UBLOCK_LATIN_EXTENDED_C:
            case UBLOCK_LATIN_EXTENDED_D:
            case UBLOCK_LATIN_EXTENDED_E:
#if defined(U_ICU_VERSION_MAJOR_NUM) && (U_ICU_VERSION_MAJOR_NUM > 69)
            case UBLOCK_LATIN_EXTENDED_F:
            case UBLOCK_LATIN_EXTENDED_G:
#endif
            case UBLOCK_LATIN_EXTENDED_ADDITIONAL:
            case UBLOCK_GREEK:
            case UBLOCK_GREEK_EXTENDED:
            case UBLOCK_CYRILLIC:
            case UBLOCK_CYRILLIC_SUPPLEMENT:
            case UBLOCK_CYRILLIC_EXTENDED_A:
            case UBLOCK_CYRILLIC_EXTENDED_B:
            case UBLOCK_CYRILLIC_EXTENDED_C:
#if defined(U_ICU_VERSION_MAJOR_NUM) && (U_ICU_VERSION_MAJOR_NUM > 71)
            case UBLOCK_CYRILLIC_EXTENDED_D:
#endif
            case UBLOCK_HANGUL_SYLLABLES:
              goto Loop1_Exit; /* break line */
            break;
          END
        END
        INCR (out_buf_ptr); /* do not break line */
#else
        if (lex_class[out_buf[out_buf_ptr]] != WHITE_SPACE)
        BEGIN
          INCR (out_buf_ptr);
        END
        else
        BEGIN
          goto Loop1_Exit;
        END
#endif
      END
Loop1_Exit:
      if (out_buf_ptr == end_ptr)
      BEGIN
        unbreakable_tail = TRUE;
      END
      else
      BEGIN
        break_pt_found = TRUE;
        while (out_buf_ptr + 1 < end_ptr)
        BEGIN
          if (lex_class[out_buf[out_buf_ptr + 1]] == WHITE_SPACE)
          BEGIN
            INCR (out_buf_ptr);
          END
          else
          BEGIN
            goto Loop2_Exit;
          END
        END
Loop2_Exit: DO_NOTHING;
      END
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 324 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    else
    BEGIN
      break_pt_found = TRUE;
    END
    if (break_pt_found)
    BEGIN
      out_buf_length = out_buf_ptr;
      break_ptr = out_buf_length + 1;
      output_bbl_line ();
      out_buf[0] = SPACE;
      out_buf[1] = SPACE;
      out_buf_ptr = 2;
      tmp_ptr = break_ptr;
      while (tmp_ptr < end_ptr)
      BEGIN
        out_buf[out_buf_ptr] = out_buf[tmp_ptr];
        INCR (out_buf_ptr);
        INCR (tmp_ptr);
      END
      out_buf_length = end_ptr - break_ptr + 2;
    END
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 323 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 322 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 318
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure adds to |str_pool| the string from |ex_buf[0]| through
 * |ex_buf[ex_buf_length-1]| if it will fit.  It assumes the global
 * variable |ex_buf_length| gives the length of the current string in
 * |ex_buf|.  It then pushes this string onto the literal stack.
 ***************************************************************************/
void          add_pool_buf_and_push (void)
BEGIN
  STR_ROOM (ex_buf_length);
  ex_buf_ptr = 0;
  while (ex_buf_ptr < ex_buf_length)
  BEGIN
    APPEND_CHAR (ex_buf[ex_buf_ptr]);
    INCR (ex_buf_ptr);
  END
  push_lit_stk (make_string (), STK_STR);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 318 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 169
 * ~~~~~~~~~~~~~~~~~~~
 * This macro complains if we've already encountered a function to be
 * inserted into the hash table.
 ***************************************************************************/
void          already_seen_function_print (HashLoc_T seen_fn_loc)
BEGIN
  PRINT_POOL_STR (hash_text[seen_fn_loc]);
  PRINT (" is already a type \"");
  print_fn_class (seen_fn_loc);
  PRINT_LN ("\" function name");
  BST_ERR_PRINT_AND_LOOK_FOR_BLAN;
Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 169 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 120
 * ~~~~~~~~~~~~~~~~~~~
 * A \bibdata command will have its arguments between braces and
 * separated by commas.  There must be exactly one such command in the
 * .aux file(s).  All upper-case letters are converted to lower case.
 ***************************************************************************/
void          aux_bib_data_command (void)
BEGIN
  if (bib_seen)
  BEGIN
    AUX_ERR_ILLEGAL_ANOTHER (N_AUX_BIBDATA);
  END
  bib_seen = TRUE;
  while (SCAN_CHAR != RIGHT_BRACE)
  BEGIN
    INCR (buf_ptr2);
    if ( ! scan2_white (RIGHT_BRACE, COMMA))
    BEGIN
      AUX_ERR_NO_RIGHT_BRACE;
    END
    if (lex_class[SCAN_CHAR] == WHITE_SPACE)
    BEGIN
      AUX_ERR_WHITE_SPACE_IN_ARGUMENT;
    END
    if ((last > (buf_ptr2 + 1)) && (SCAN_CHAR == RIGHT_BRACE))
    BEGIN
      AUX_ERR_STUFF_AFTER_RIGHT_BRACE;
    END

/***************************************************************************
 * WEB section number:	123
 * ~~~~~~~~~~~~~~~~~~~
 * Now we add the just-found argument to |bib_list| if it hasn't already
 * been encountered as a \bibdata argument and if, after appending
 * the |s_bib_extension| string, the resulting file name can be opened.
 ***************************************************************************/
    BEGIN
      if (bib_ptr == Max_Bib_Files)
      BEGIN
        BIB_XRETALLOC_NOSET ("bib_file", bib_file, AlphaFile_T,
                             Max_Bib_Files, Max_Bib_Files + MAX_BIB_FILES);
        BIB_XRETALLOC_NOSET ("bib_list", bib_list, StrNumber_T,
                             Max_Bib_Files, Max_Bib_Files + MAX_BIB_FILES);
        BIB_XRETALLOC ("s_preamble", s_preamble, StrNumber_T,
                       Max_Bib_Files, Max_Bib_Files + MAX_BIB_FILES);
      END
      CUR_BIB_STR = hash_text[str_lookup (buffer, buf_ptr1, TOKEN_LEN,
  					  BIB_FILE_ILK, DO_INSERT)];
      if (hash_found)
      BEGIN
        OPEN_BIBDATA_AUX_ERR ("This database file appears more than once: ");
      END
      start_name (CUR_BIB_STR);
      add_extension (s_bib_extension);
      if ( ! a_open_in (&CUR_BIB_FILE, BIB_FILE_SEARCH_PATH))
      BEGIN
        BEGIN
          OPEN_BIBDATA_AUX_ERR ("I couldn't open database file ");
          perror ("\nReason");
        END
      END

#ifdef TRACE
      if (Flag_trace) {
        TRACE_PR_POOL_STR (CUR_BIB_STR);
        TRACE_PR_POOL_STR (s_bib_extension);
        TRACE_PR_LN (" is a bibdata file");
      }
#endif                      			/* TRACE */

      INCR (bib_ptr);
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 123 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  END
Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 120 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 126
 * ~~~~~~~~~~~~~~~~~~~
 * A \bibstyle command will have exactly one argument, and it will be
 * between braces.  There must be exactly one such command in the .aux
 * file(s).  All upper-case letters are converted to lower case.
 ***************************************************************************/
void          aux_bib_style_command (void)
BEGIN
  if (bst_seen)
  BEGIN
    AUX_ERR_ILLEGAL_ANOTHER (N_AUX_BIBSTYLE);
  END
  bst_seen = TRUE;
  INCR (buf_ptr2);
  if ( ! scan1_white (RIGHT_BRACE))
  BEGIN
    AUX_ERR_NO_RIGHT_BRACE;
  END
  if (lex_class[SCAN_CHAR] == WHITE_SPACE)
  BEGIN
    AUX_ERR_WHITE_SPACE_IN_ARGUMENT;
  END
  if (last > (buf_ptr2 + 1))
  BEGIN
    AUX_ERR_STUFF_AFTER_RIGHT_BRACE;
  END

/***************************************************************************
 * WEB section number:	127
 * ~~~~~~~~~~~~~~~~~~~
 * Now we open the file whose name is the just-found argument appended
 * with the |s_bst_extension| string, if possible.
 ***************************************************************************/
  BEGIN
    bst_str = hash_text[str_lookup (buffer, buf_ptr1, TOKEN_LEN,
				    BST_FILE_ILK, DO_INSERT)];
    if (hash_found)
    BEGIN

#ifdef TRACE
      if (Flag_trace)
        print_bst_name ();
#endif                      			/* TRACE */

      CONFUSION ("Already encountered style file");
    END
    start_name (bst_str);
    add_extension (s_bst_extension);
    if ( ! a_open_in (&bst_file, BST_FILE_SEARCH_PATH))
    BEGIN
      BEGIN
	PRINT ("I couldn't open style file ");
	print_bst_name ();
        perror ("\nReason");
	bst_str = 0;
	AUX_ERR_RETURN;
      END
    END
    PRINT ("The style file: ");
    print_bst_name ();
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 127 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 126 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/





/***************************************************************************
 * WEB section number:	 132
 * ~~~~~~~~~~~~~~~~~~~
 * A \citation command will have its arguments between braces and
 * separated by commas.  Upper/lower cases are considered to be different
 * for \citation arguments, which is the same as the rest of \LaTeX\
 * but different from the rest of \BibTeX.  A cite key needn't exactly
 * case-match its corresponding database key to work, although two cite
 * keys that are case-mismatched will produce an error message.  (A BEGIN\sl
 * case mismatch\/END is a mismatch, but only because of a case difference.)
 *
 * A \citation command having * as an argument indicates that the
 * entire database will be included (almost as if a \nocite command
 * that listed every cite key in the database, in order, had been given at
 * the corresponding spot in the .tex file).
 ***************************************************************************/
void          aux_citation_command (void)
BEGIN
  citation_seen = TRUE;
  while (SCAN_CHAR != RIGHT_BRACE)
  BEGIN
    INCR (buf_ptr2);
    if ( ! scan2_white (RIGHT_BRACE, COMMA))
    BEGIN
      AUX_ERR_NO_RIGHT_BRACE;
    END
    if (lex_class[SCAN_CHAR] == WHITE_SPACE)
    BEGIN
      AUX_ERR_WHITE_SPACE_IN_ARGUMENT;
    END
    if ((last > (buf_ptr2 + 1)) && (SCAN_CHAR == RIGHT_BRACE))
    BEGIN
      AUX_ERR_STUFF_AFTER_RIGHT_BRACE;
    END

/***************************************************************************
 * WEB section number:	133
 * ~~~~~~~~~~~~~~~~~~~
 * We must check if (the lower-case version of) this cite key has been
 * previously encountered, and proceed accordingly.  The alias kludge
 * helps make the stack space not overflow on some machines.
 ***************************************************************************/
    BEGIN

#ifdef TRACE
      if (Flag_trace) {
        TRACE_PR_TOKEN;
        TRACE_PR (" cite key encountered");
      }
#endif                      			/* TRACE */

/***************************************************************************
 * WEB section number:	134
 * ~~~~~~~~~~~~~~~~~~~
 * Here we check for a \citation command having * as an
 * argument, indicating that the entire database will be included.
 ***************************************************************************/
      BEGIN
	if (TOKEN_LEN == 1)
	BEGIN
	  if (buffer[buf_ptr1] == STAR)
	  BEGIN

#ifdef TRACE
            if (Flag_trace)
              TRACE_PR_LN ("---entire database to be included");
#endif                      			/* TRACE */

	    if (all_entries)
	    BEGIN
	      PRINT_LN ("Multiple inclusions of entire database");
	      AUX_ERR_RETURN;
	    END
	    else
	    BEGIN
	      all_entries = TRUE;
	      all_marker = cite_ptr;
	      goto Next_Cite_Label;
	    END
	  END
	END
     END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 134 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      tmp_ptr = buf_ptr1;
      while (tmp_ptr < buf_ptr2)
      BEGIN
	ex_buf[tmp_ptr] = buffer[tmp_ptr];
	INCR (tmp_ptr);
      END
      lower_case (ex_buf, buf_ptr1, TOKEN_LEN);
      lc_cite_loc = str_lookup (ex_buf, buf_ptr1, TOKEN_LEN, LC_CITE_ILK,
			      DO_INSERT);
      if (hash_found)

/***************************************************************************
 * WEB section number:	135
 * ~~~~~~~~~~~~~~~~~~~
 * We've previously encountered the lower-case version, so we check that
 * the actual version exactly matches the actual version of the
 * previously-encountered cite key(s).
 ***************************************************************************/
     BEGIN

#ifdef	TRACE
        if (Flag_trace)
	  TRACE_PR_LN (" previously");
#endif                      			/* TRACE */

	dummy_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN, CITE_ILK,
				DONT_INSERT);
	if ( ! hash_found)
	BEGIN
	  PRINT ("Case mismatch error between cite keys ");
	  PRINT_TOKEN;
	  PRINT (" and ");
	  PRINT_POOL_STR (cite_list[ilk_info[ilk_info[lc_cite_loc]]]);
	  PRINT_NEWLINE;
	  AUX_ERR_RETURN;
	END
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 135 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      else

/***************************************************************************
 * WEB section number:	136
 * ~~~~~~~~~~~~~~~~~~~
 * Now we add the just-found argument to |cite_list| if there isn't
 * anything funny happening.
 ***************************************************************************/
      BEGIN

#ifdef TRACE
        if (Flag_trace)
	  TRACE_PR_NEWLINE;
#endif                      			/* TRACE */

	cite_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN, CITE_ILK,
			       DO_INSERT);
	if (hash_found)
	BEGIN
	  hash_cite_confusion ();
	END
	check_cite_overflow (cite_ptr);
	CUR_CITE_STR = hash_text[cite_loc];
	ilk_info[cite_loc] = cite_ptr;
	ilk_info[lc_cite_loc] = cite_loc;
	INCR (cite_ptr);
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 136 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 133 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


Next_Cite_Label: DO_NOTHING
  END
Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 132 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 144
 * ~~~~~~~~~~~~~~~~~~~
 * We must complain if anything's amiss.
 ***************************************************************************/
void          aux_end1_err_print (void)
BEGIN
  PRINT ("I found no ");
END


void          aux_end2_err_print (void)
BEGIN
  PRINT ("---while reading file ");
  print_aux_name ();
  mark_error ();
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 144 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 112
 * ~~~~~~~~~~~~~~~~~~~
 * Here are a bunch of macros whose print statements are used at least
 * twice.  Thus we save space by making the statements procedures.  This
 * macro complains when there's a repeated command that's to be used just
 * once.
 ***************************************************************************/
void          aux_err_illegal_another_print (Integer_T cmd_num)
BEGIN
  PRINT ("Illegal, another \\bib");
  switch (cmd_num)
  BEGIN
    case N_AUX_BIBDATA:
      PRINT ("data");
      break;
    case N_AUX_BIBSTYLE:
      PRINT ("style");
      break;
    default:
      CONFUSION ("Illegal auxiliary-file command");
  END
  PRINT (" command");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 112 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 113
 * ~~~~~~~~~~~~~~~~~~~
 * This one complains when a command is missing its |right_brace|.
 ***************************************************************************/
void          aux_err_no_right_brace_print (void)
BEGIN
  PRINT2 ("No \"%c\"", xchr[RIGHT_BRACE]);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 113 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 111
 * ~~~~~~~~~~~~~~~~~~~
 * When we find a bug, we print a message and flush the rest of the line.
 * This macro must be called from within a procedure that has an |exit|
 * label.
 ***************************************************************************/
void          aux_err_print (void)
BEGIN
  PRINT2 ("---line %ld of file ", (long) CUR_AUX_LINE);
  print_aux_name ();
  print_bad_input_line ();
  print_skipping_whatever_remains ();
  PRINT_LN ("command");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 111 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 114
 * ~~~~~~~~~~~~~~~~~~~
 * This one complains when a command has stuff after its |right_brace|.
 ***************************************************************************/
void          aux_err_stuff_after_right_brace (void)
BEGIN
  PRINT2 ("Stuff after \"%c\"", xchr[RIGHT_BRACE]);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 114 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 115
 * ~~~~~~~~~~~~~~~~~~~
 * And this one complains when a command has |white_space| in its
 * argument.
 ***************************************************************************/
void          aux_err_white_space_in_argument (void)
BEGIN
  PRINT ("White space in argument");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 115 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 139
 * ~~~~~~~~~~~~~~~~~~~
 * An \@input command will have exactly one argument, it will be
 * between braces, and it must have the |s_aux_extension|.  All upper-case
 * letters are converted to lower case.
 ***************************************************************************/
void          aux_input_command (void)
BEGIN
  Boolean_T         aux_extension_ok;

  INCR (buf_ptr2);
  if ( ! scan1_white (RIGHT_BRACE))
  BEGIN
    AUX_ERR_NO_RIGHT_BRACE;
  END
  if (lex_class[SCAN_CHAR] == WHITE_SPACE)
  BEGIN
    AUX_ERR_WHITE_SPACE_IN_ARGUMENT;
  END
  if (last > (buf_ptr2 + 1))
  BEGIN
    AUX_ERR_STUFF_AFTER_RIGHT_BRACE;
  END

/***************************************************************************
 * WEB section number:	140
 * ~~~~~~~~~~~~~~~~~~~
 * We must check that this potential .aux file won't overflow the
 * stack, that it has the correct extension, that we haven't encountered
 * it before (to prevent, among other things, an infinite loop).
 ***************************************************************************/
  BEGIN
    INCR (aux_ptr);
    if (aux_ptr == AUX_STACK_SIZE)
    BEGIN
      PRINT_TOKEN;
      PRINT (": ");
      BIBTEX_OVERFLOW ("auxiliary file depth ", AUX_STACK_SIZE);
    END
    aux_extension_ok = TRUE;
    if (TOKEN_LEN < LENGTH (s_aux_extension))
    BEGIN
      aux_extension_ok = FALSE;
    END
    else if ( ! str_eq_buf (s_aux_extension, buffer,
			    (buf_ptr2 - LENGTH (s_aux_extension)),
			    LENGTH (s_aux_extension)))
    BEGIN
      aux_extension_ok = FALSE;
    END
    if ( ! aux_extension_ok)
    BEGIN
      PRINT_TOKEN;
      PRINT (" has a wrong extension");
      DECR (aux_ptr);
      AUX_ERR_RETURN;
    END
    CUR_AUX_STR = hash_text[str_lookup (buffer, buf_ptr1, TOKEN_LEN,
					AUX_FILE_ILK, DO_INSERT)];
    if (hash_found)
    BEGIN
      PRINT ("Already encountered file ");
      print_aux_name ();
      DECR (aux_ptr);
      AUX_ERR_RETURN;
    END

/***************************************************************************
 * WEB section number:	141
 * ~~~~~~~~~~~~~~~~~~~
 * We check that this .aux file can actually be opened, and then open it.
 *
 * NOTE: because C arrays start at index 0, not 1, the subscripts of array
 *	 |name_of_file| are generally 1 less than those in the WEB source.
 ***************************************************************************/
    BEGIN
      start_name (CUR_AUX_STR);
      name_of_file[name_length] = 0;
      if ( ! a_open_in (&CUR_AUX_FILE, AUX_FILE_SEARCH_PATH))
      BEGIN
	PRINT ("I couldn't open auxiliary file ");
	print_aux_name ();
        perror ("\nReason");
	DECR (aux_ptr);
	AUX_ERR_RETURN;
      END
      PRINT2 ("A level-%ld auxilliary file: ", (long) aux_ptr);
      print_aux_name ();
      CUR_AUX_LINE = 0;
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 141 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 140 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 139 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 177
 * ~~~~~~~~~~~~~~~~~~~
 * A legal argument for an execute, iterate, or reverse
 * command must exist and be |built_in| or |wiz_defined|.  Here's where we
 * check, returning |true| if the argument is illegal.
 ***************************************************************************/
Boolean_T         bad_argument_token (void)
BEGIN
  Boolean_T       bad_argument_token;

  bad_argument_token= TRUE;
  lower_case (buffer, buf_ptr1, TOKEN_LEN);
  fn_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN, BST_FN_ILK, DONT_INSERT);
  if ( ! hash_found)
  BEGIN
    PRINT_TOKEN;
    BST_ERR (" is an unknown function");
  END
  else if ((fn_type[fn_loc] != BUILT_IN) && (fn_type[fn_loc] != WIZ_DEFINED))
  BEGIN
    PRINT_TOKEN;
    PRINT (" has bad function type ");
    print_fn_class (fn_loc);
    BST_ERR_PRINT_AND_LOOK_FOR_BLAN;
  END
  bad_argument_token= FALSE;

Exit_Label:
  return (bad_argument_token);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 177 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 280
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure exists to save space, since it's used twice---once for
 * each of the two succeeding modules.
 ***************************************************************************/
void          bad_cross_reference_print (StrNumber_T s)
BEGIN
  PRINT ("--entry \"");
  PRINT_POOL_STR (CUR_CITE_STR);
  PRINT_LN ("\"");
  PRINT ("refers to entry \"");
  PRINT_POOL_STR (s);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 280 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 240
 * ~~~~~~~~~~~~~~~~~~~
 * Here's another bug.
 ***************************************************************************/
void          bib_cmd_confusion (void)
BEGIN
  CONFUSION ("Unknown database-file command");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 240 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 231
 * ~~~~~~~~~~~~~~~~~~~
 * This one's for an expected |equals_sign|.
 ***************************************************************************/
void          bib_equals_sign_print (void)
BEGIN
  BIB_ERR2 ("I was expecting an \"%c\"", xchr[EQUALS_SIGN]);

Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 231 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 221
 * ~~~~~~~~~~~~~~~~~~~
 * When there's a serious error parsing a .bib file, we flush
 * everything up to the beginning of the next entry.
 ***************************************************************************/
void          bib_err_print (void)
BEGIN
  PRINT ("-");
  bib_ln_num_print ();
  print_bad_input_line ();
  print_skipping_whatever_remains ();
  if (at_bib_command)
  BEGIN
    PRINT_LN ("command");
  END
  else
  BEGIN
    PRINT_LN ("entry");
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 221 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 233
 * ~~~~~~~~~~~~~~~~~~~
 * And this one about an overly exuberant field.
 ***************************************************************************/
void          bib_field_too_long_print (void)
BEGIN
  BIB_ERR2 ("Your field is more than %ld characters",  (long) Buf_Size);

Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 233 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 235
 * ~~~~~~~~~~~~~~~~~~~
 * This macro is used to scan all .bib identifiers.  The argument
 * tells what was happening at the time.  The associated procedure simply
 * prints an error message.
 ***************************************************************************/
void          bib_id_print (void)
BEGIN
  if (scan_result == ID_NULL)
  BEGIN
    PRINT ("You're missing ");
  END
  else if (scan_result == OTHER_CHAR_ADJACENT)
  BEGIN
    PRINT2 ("\"%c\" immediately follows ", xchr[SCAN_CHAR]);
  END
  else
  BEGIN
    id_scanning_confusion ();
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 235 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 220
 * ~~~~~~~~~~~~~~~~~~~
 * This little procedure exists because it's used by at least two other
 * procedures and thus saves some space.
 ***************************************************************************/
void          bib_ln_num_print (void)
BEGIN
  PRINT2 ("--line %ld of file ", (long) bib_line_num);
  print_bib_name ();
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 220 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 230
 * ~~~~~~~~~~~~~~~~~~~
 * And here are a bunch of error-message macros, each called more than
 * once, that thus save space as implemented.  This one is for when one of
 * two possible characters is expected while scanning.
 ***************************************************************************/
void          bib_one_of_two_print (ASCIICode_T char1, ASCIICode_T char2)
BEGIN
  BIB_ERR3 ("I was expecting a `%c' or a `%c'", xchr[char1], xchr[char2]);
Exit_Label: DO_NOTHING
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 230 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 232
 * ~~~~~~~~~~~~~~~~~~~
 * This complains about unbalanced braces.
 ***************************************************************************/
void          bib_unbalanced_braces_print (void)
BEGIN
  BIB_ERR ("Unbalanced braces");
Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 232 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 222
 * ~~~~~~~~~~~~~~~~~~~
 * When there's a harmless error parsing a .bib file, we just give a
 * warning message.  This is always called after other stuff has been
 * printed out.
 ***************************************************************************/
void          bib_warn_print (void)
BEGIN
  bib_ln_num_print ();
  mark_warning ();
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 222 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 406
 * ~~~~~~~~~~~~~~~~~~~
 * At most one of the important letters, perhaps doubled, may appear at
 * |sp_brace_level = 1|.
 ***************************************************************************/
void          brace_lvl_one_letters_complaint (void)
BEGIN
  PRINT ("The format string \"");
  PRINT_POOL_STR (pop_lit1);
  BST_EX_WARN ("\" has an illegal brace-level-1 letter");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 406 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 368
 * ~~~~~~~~~~~~~~~~~~~
 * This complaint often arises because the style designer has to type lots
 * of braces.
 ***************************************************************************/
void          braces_unbalanced_complaint (StrNumber_T pop_lit_var)
BEGIN
  PRINT ("Warning--\"");
  PRINT_POOL_STR (pop_lit_var);
  BST_MILD_EX_WARN ("\" isn't a brace-balanced string");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 368 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 356
 * ~~~~~~~~~~~~~~~~~~~
 * It's time for a complaint if either of the two (entry or global) string
 * lengths is exceeded.
 ***************************************************************************/
void          bst_1print_string_size_exceeded (void)
BEGIN
  PRINT ("Warning--you've exceeded ");
END

void          bst_2print_string_size_exceeded (void)
BEGIN
  PRINT ("-string-size,");
  bst_mild_ex_warn_print ();
  PRINT_LN ("*Please notify the bibstyle designer*");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 356 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 295
 * ~~~~~~~~~~~~~~~~~~~
 * It's illegal to mess with the entry information at certain times;
 * here's a complaint for these times.
 ***************************************************************************/
void          bst_cant_mess_with_entries_prin (void)
BEGIN
  BST_EX_WARN ("You can't mess with entries here");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 295 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 170
 * ~~~~~~~~~~~~~~~~~~~
 * An entry command has three arguments, each a (possibly empty) list
 * of function names between braces (the names are separated by one or
 * more |white_space| characters).  All function names in this and other
 * commands must be legal .bst identifiers.  Upper/lower cases are
 * considered to be the same for function names in these lists---all
 * upper-case letters are converted to lower case.  These arguments give
 * lists of |field|s, |int_entry_var|s, and |str_entry_var|s.
 ***************************************************************************/
void          bst_entry_command (void)
BEGIN
  if (entry_seen)
  BEGIN
    BST_ERR ("Illegal, another entry command");
  END
  entry_seen = TRUE;
  EAT_BST_WHITE_AND_EOF_CHECK ("entry");

/***************************************************************************
 * WEB section number:	171
 * ~~~~~~~~~~~~~~~~~~~
 * This module reads a |left_brace|, the list of |field|s, and a
 * |right_brace|.  The |field|s are those like `author' and `title.'
 ***************************************************************************/
  BEGIN
    BST_GET_AND_CHECK_LEFT_BRACE ("entry");
    EAT_BST_WHITE_AND_EOF_CHECK ("entry");
    while (SCAN_CHAR != RIGHT_BRACE)
    BEGIN
      BST_IDENTIFIER_SCAN ("entry");

/***************************************************************************
 * WEB section number:	172
 * ~~~~~~~~~~~~~~~~~~~
 * Here we insert the just found field name into the hash table, record
 * it as a |field|, and assign it a number to be used in indexing into
 * the |field_info| array.
 ***************************************************************************/
      BEGIN

#ifdef TRACE
        if (Flag_trace) {
	  TRACE_PR_TOKEN;
	  TRACE_PR_LN (" is a field");
        }
#endif                      			/* TRACE */

	lower_case (buffer, buf_ptr1, TOKEN_LEN);
	fn_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN,
			     BST_FN_ILK, DO_INSERT);
	CHECK_FOR_ALREADY_SEEN_FUNCTION (fn_loc);
	fn_type[fn_loc] = FIELD;
	ilk_info[fn_loc] = num_fields;
	INCR (num_fields);
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 172 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


      EAT_BST_WHITE_AND_EOF_CHECK ("entry");
    END
    INCR (buf_ptr2);
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 171 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  EAT_BST_WHITE_AND_EOF_CHECK ("entry");
  if (num_fields == num_pre_defined_fields)
  BEGIN
    BST_WARN ("Warning--I didn't find any fields");
  END

/***************************************************************************
 * WEB section number:	173
 * ~~~~~~~~~~~~~~~~~~~
 * This module reads a |left_brace|, the list of |int_entry_var|s,
 * and a |right_brace|.
 ***************************************************************************/
  BEGIN
    BST_GET_AND_CHECK_LEFT_BRACE ("entry");
    EAT_BST_WHITE_AND_EOF_CHECK ("entry");
    while (SCAN_CHAR != RIGHT_BRACE)
    BEGIN
      BST_IDENTIFIER_SCAN ("entry");

/***************************************************************************
 * WEB section number:	174
 * ~~~~~~~~~~~~~~~~~~~
 * Here we insert the just found |int_entry_var| name into the hash table
 * and record it as an |int_entry_var|.  An |int_entry_var| is one that
 * the style designer wants a separate copy of for each entry.
 ***************************************************************************/
      BEGIN

#ifdef TRACE
        if (Flag_trace) {
	  TRACE_PR_TOKEN;
	  TRACE_PR_LN (" is an integer entry-variable");
        }
#endif                      			/* TRACE */

	lower_case (buffer, buf_ptr1, TOKEN_LEN);
	fn_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN,
			     BST_FN_ILK, DO_INSERT);
	CHECK_FOR_ALREADY_SEEN_FUNCTION (fn_loc);
	fn_type[fn_loc] = INT_ENTRY_VAR;
	FN_INFO[fn_loc] = num_ent_ints;
	INCR (num_ent_ints);
      END

/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 174 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      EAT_BST_WHITE_AND_EOF_CHECK ("entry");
    END
    INCR (buf_ptr2);
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 173 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  EAT_BST_WHITE_AND_EOF_CHECK ("entry");

/***************************************************************************
 * WEB section number:	175
 * ~~~~~~~~~~~~~~~~~~~
 * This module reads a |left_brace|, the list of |str_entry_var|s, and a
 * |right_brace|.  A |str_entry_var| is one that the style designer wants
 * a separate copy of for each entry.
 ***************************************************************************/
  BEGIN
    BST_GET_AND_CHECK_LEFT_BRACE ("entry");
    EAT_BST_WHITE_AND_EOF_CHECK ("entry");
    while (SCAN_CHAR != RIGHT_BRACE)
    BEGIN
      BST_IDENTIFIER_SCAN ("entry");

/***************************************************************************
 * WEB section number:	176
 * ~~~~~~~~~~~~~~~~~~~
 * Here we insert the just found |str_entry_var| name into the hash
 * table, record it as a |str_entry_var|, and set its pointer into
 * |entry_strs|.
 ***************************************************************************/
      BEGIN
#ifdef TRACE
        if (Flag_trace) {
	  TRACE_PR_TOKEN;
	  TRACE_PR_LN (" is a string entry-variable");
        }
#endif                      			/* TRACE */

	lower_case (buffer, buf_ptr1, TOKEN_LEN);
	fn_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN,
			     BST_FN_ILK, DO_INSERT);
	CHECK_FOR_ALREADY_SEEN_FUNCTION (fn_loc);
	fn_type[fn_loc] = STR_ENTRY_VAR;
	FN_INFO[fn_loc] = num_ent_strs;
	INCR (num_ent_strs);
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 176 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      EAT_BST_WHITE_AND_EOF_CHECK ("entry");
    END
    INCR (buf_ptr2);
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 175 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 170 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 149
 * ~~~~~~~~~~~~~~~~~~~
 * When there's a serious error parsing the .bst file, we flush the
 * rest of the current command; a blank line is assumed to mark the end of
 * a command (but for the purposes of error recovery only).  Thus, error
 * recovery will be better if style designers leave blank lines between
 * .bst commands.  This macro must be called from within a procedure
 * that has an |exit| label.
 ***************************************************************************/
void          bst_err_print_and_look_for_blan (void)
BEGIN
  PRINT ("-");
  bst_ln_num_print ();
  print_bad_input_line ();
  while (last != 0)
    if ( ! input_ln (bst_file))
    BEGIN
      longjmp (Bst_Done_Flag, 1);
    END
    else
    BEGIN
      INCR (bst_line_num);
    END
  buf_ptr2 = last;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 149 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 178
 * ~~~~~~~~~~~~~~~~~~~
 * An execute command has one argument, a single |built_in| or
 * |wiz_defined| function name between braces.  Upper/lower cases are
 * considered to be the same---all upper-case letters are converted to
 * lower case.  Also, we must make sure we've already seen a read
 * command.
 *
 * This module reads a |left_brace|, a single function to be executed, and
 * a |right_brace|.
 ***************************************************************************/
void          bst_execute_command (void)
BEGIN
  if ( ! read_seen)
  BEGIN
    BST_ERR ("Illegal, execute command before read command");
  END
  EAT_BST_WHITE_AND_EOF_CHECK ("execute");
  BST_GET_AND_CHECK_LEFT_BRACE ("execute");
  EAT_BST_WHITE_AND_EOF_CHECK ("execute");
  BST_IDENTIFIER_SCAN ("execute");


/***************************************************************************
 * WEB section number:	179
 * ~~~~~~~~~~~~~~~~~~~
 * Before executing the function, we must make sure it's a legal one.  It
 * must exist and be |built_in| or |wiz_defined|.
 ***************************************************************************/
  BEGIN

#ifdef TRACE
    if (Flag_trace) {
      TRACE_PR_TOKEN;
      TRACE_PR_LN (" is a to be executed function");
    }
#endif	                     			/* TRACE */

    if (bad_argument_token ())
    BEGIN
      goto Exit_Label;
    END
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 179 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  EAT_BST_WHITE_AND_EOF_CHECK ("execute");
  BST_GET_AND_CHECK_RIGHT_BRACE ("execute");

/***************************************************************************
 * WEB section number:	296
 * ~~~~~~~~~~~~~~~~~~~
 * This module executes a single specified function once.  It can't do
 * anything with the entries.
 ***************************************************************************/
  BEGIN
    init_command_execution ();
    mess_with_entries = FALSE;
    execute_fn (fn_loc);
    check_command_execution ();
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 296 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 178 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 293
 * ~~~~~~~~~~~~~~~~~~~
 * When there's an error while executing .bst functions, what we do
 * depends on whether the function is messing with the entries.
 * Furthermore this error is serious enough to classify as an
 * |error_message| instead of a |warning_message|.  These messages (that
 * is, from |bst_ex_warn|) are meant both for the user and for the style
 * designer while debugging.
 ***************************************************************************/
void          bst_ex_warn_print (void)
BEGIN
  if (mess_with_entries)
  BEGIN
    PRINT (" for entry ");
    PRINT_POOL_STR (CUR_CITE_STR);
  END
  PRINT_NEWLINE;
  PRINT ("while executing-");
  bst_ln_num_print ();
  mark_error ();
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 293 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 180
 * ~~~~~~~~~~~~~~~~~~~
 * A function command has two arguments; the first is a |wiz_defined|
 * function name between braces.  Upper/lower cases are considered to be
 * the same---all upper-case letters are converted to lower case.  The
 * second argument defines this function.  It consists of a sequence of
 * functions, between braces, separated by |white_space| characters.
 * Upper/lower cases are considered to be the same for function names but
 * not for |str_literal|s.
 ***************************************************************************/
void          bst_function_command (void)
BEGIN
  EAT_BST_WHITE_AND_EOF_CHECK ("function");

/***************************************************************************
 * WEB section number:	181
 * ~~~~~~~~~~~~~~~~~~~
 * This module reads a |left_brace|, a |wiz_defined| function name, and
 * a |right_brace|.
 ***************************************************************************/
  BEGIN
    BST_GET_AND_CHECK_LEFT_BRACE ("function");
    EAT_BST_WHITE_AND_EOF_CHECK ("function");
    BST_IDENTIFIER_SCAN ("function");

/***************************************************************************
 * WEB section number:	182
 * ~~~~~~~~~~~~~~~~~~~
 * The function name must exist and be a new one; we mark it as
 * |wiz_defined|.  Also, see if it's the default entry-type function.
 ***************************************************************************/
    BEGIN
#ifdef TRACE
      if (Flag_trace) {
        TRACE_PR_TOKEN;
        TRACE_PR_LN (" is a wizard-defined function");
      }
#endif                      			/* TRACE */

      lower_case (buffer, buf_ptr1, TOKEN_LEN);
      wiz_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN, BST_FN_ILK,
			    DO_INSERT);
      CHECK_FOR_ALREADY_SEEN_FUNCTION (wiz_loc);
      fn_type[wiz_loc] = WIZ_DEFINED;
      if (hash_text[wiz_loc] == s_default)
      BEGIN
	b_default = wiz_loc;
      END
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 182 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    EAT_BST_WHITE_AND_EOF_CHECK ("function");
    BST_GET_AND_CHECK_RIGHT_BRACE ("function");
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 181 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  EAT_BST_WHITE_AND_EOF_CHECK ("function");
  BST_GET_AND_CHECK_LEFT_BRACE ("function");
  scan_fn_def (wiz_loc);
Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 180 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 166
 * ~~~~~~~~~~~~~~~~~~~
 * This macro is used to scan all .bst identifiers.  The argument
 * supplies the .bst command name.  The associated procedure simply
 * prints an error message.
 ***************************************************************************/
void          bst_id_print (void)
BEGIN
  if (scan_result == ID_NULL)
  BEGIN
    PRINT2 ("\"%c\" begins identifier, command: ", xchr[SCAN_CHAR]);
  END
  else if (scan_result == OTHER_CHAR_ADJACENT)
  BEGIN
    PRINT2 ("\"%c\" immediately follows identifier, command: ",
	    xchr[SCAN_CHAR]);
  END
  else
  BEGIN
    id_scanning_confusion ();
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 166 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 201
 * ~~~~~~~~~~~~~~~~~~~
 * An integers command has one argument, a list of function names
 * between braces (the names are separated by one or more |white_space|
 * characters).  Upper/lower cases are considered to be the same for
 * function names in these lists---all upper-case letters are converted to
 * lower case.  Each name in this list specifies an |int_global_var|.
 * There may be several integers commands in the .bst file.
 *
 * This module reads a |left_brace|, a list of |int_global_var|s, and a
 * |right_brace|.
 ***************************************************************************/
void          bst_integers_command (void)
BEGIN
  EAT_BST_WHITE_AND_EOF_CHECK ("integers");
  BST_GET_AND_CHECK_LEFT_BRACE ("integers");
  EAT_BST_WHITE_AND_EOF_CHECK ("integers");
  while (SCAN_CHAR != RIGHT_BRACE)
  BEGIN
    BST_IDENTIFIER_SCAN ("integers");

/***************************************************************************
 * WEB section number:	202
 * ~~~~~~~~~~~~~~~~~~~
 * Here we insert the just found |int_global_var| name into the hash
 * table and record it as an |int_global_var|.  Also, we initialize it by
 * setting |FN_INFO[fn_loc]| to 0.
 ***************************************************************************/
    BEGIN

#ifdef TRACE
      if (Flag_trace) {
        TRACE_PR_TOKEN;
        TRACE_PR_LN (" is an integer global-variable");
      }
#endif                      			/* TRACE */

      lower_case (buffer, buf_ptr1, TOKEN_LEN);
      fn_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN,
			   BST_FN_ILK, DO_INSERT);
      CHECK_FOR_ALREADY_SEEN_FUNCTION (fn_loc);
      fn_type[fn_loc] = INT_GLOBAL_VAR;
      FN_INFO[fn_loc] = 0;
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 202 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    EAT_BST_WHITE_AND_EOF_CHECK ("integers");
  END
  INCR (buf_ptr2);
Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 201 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/***************************************************************************
 * WEB section number:	 203
 * ~~~~~~~~~~~~~~~~~~~
 * An iterate command has one argument, a single |built_in| or
 * |wiz_defined| function name between braces.  Upper/lower cases are
 * considered to be the same---all upper-case letters are converted to
 * lower case.  Also, we must make sure we've already seen a read
 * command.
 *
 * This module reads a |left_brace|, a single function to be iterated, and
 * a |right_brace|.
 ***************************************************************************/
void          bst_iterate_command (void)
BEGIN
  if ( ! read_seen)
  BEGIN
    BST_ERR ("Illegal, iterate command before read command");
  END
  EAT_BST_WHITE_AND_EOF_CHECK ("iterate");
  BST_GET_AND_CHECK_LEFT_BRACE ("iterate");
  EAT_BST_WHITE_AND_EOF_CHECK ("iterate");
  BST_IDENTIFIER_SCAN ("iterate");

/***************************************************************************
 * WEB section number:	204
 * ~~~~~~~~~~~~~~~~~~~
 * Before iterating the function, we must make sure it's a legal one.  It
 * must exist and be |built_in| or |wiz_defined|.
 ***************************************************************************/
  BEGIN

#ifdef TRACE
    if (Flag_trace) {
      TRACE_PR_TOKEN;
      TRACE_PR_LN (" is a to be iterated function");
    }
#endif                      			/* TRACE */

    if (bad_argument_token ())
    BEGIN
      goto Exit_Label;
    END
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 204 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  EAT_BST_WHITE_AND_EOF_CHECK ("iterate");
  BST_GET_AND_CHECK_RIGHT_BRACE ("iterate");

/***************************************************************************
 * WEB section number:	297
 * ~~~~~~~~~~~~~~~~~~~
 * This module iterates a single specified function for all entries
 * specified by |cite_list|.
 ***************************************************************************/
  BEGIN
    init_command_execution ();
    mess_with_entries = TRUE;
    sort_cite_ptr = 0;
    while (sort_cite_ptr < num_cites)
    BEGIN
      cite_ptr = SORTED_CITES[sort_cite_ptr];

#ifdef TRACE
      if (Flag_trace) {
        TRACE_PR_POOL_STR (hash_text[fn_loc]);
        TRACE_PR (" to be iterated on ");
        TRACE_PR_POOL_STR (CUR_CITE_STR);
        TRACE_PR_NEWLINE;
      }
#endif                      			/* TRACE */

      execute_fn (fn_loc);
      check_command_execution ();
      INCR (sort_cite_ptr);
    END
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 297 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 203 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 167
 * ~~~~~~~~~~~~~~~~~~~
 * This macro just makes sure we're at a |left_brace|.
 ***************************************************************************/
void          bst_left_brace_print (void)
BEGIN
  PRINT2 ("\"%c\" is missing in command: ", xchr[LEFT_BRACE]);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 167 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 148
 * ~~~~~~~~~~~~~~~~~~~
 * This little procedure exists because it's used by at least two other
 * procedures and thus saves some space.
 ***************************************************************************/
void          bst_ln_num_print (void)
BEGIN
  PRINT2 ("--line %ld of file ", (long) bst_line_num);
  print_bst_name ();
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 148 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 205
 * ~~~~~~~~~~~~~~~~~~~
 * A macro command, like a function command, has two arguments;
 * the first is a macro name between braces.  The name must be a legal
 * .bst identifier.  Upper/lower cases are considered to be the
 * same---all upper-case letters are converted to lower case.  The second
 * argument defines this macro.  It consists of a |double_quote|-delimited
 * string (which must be on a single line) between braces, with optional
 * |white_space| characters between the braces and the |double_quote|s.
 * This |double_quote|-delimited string is parsed exactly as a
 * |str_literal| is for the function command.
 ***************************************************************************/
void          bst_macro_command (void)
BEGIN
  if (read_seen)
  BEGIN
    BST_ERR ("Illegal, macro command after read command");
  END
  EAT_BST_WHITE_AND_EOF_CHECK ("macro");

/***************************************************************************
 * WEB section number:	206
 * ~~~~~~~~~~~~~~~~~~~
 * This module reads a |left_brace|, a macro name, and a |right_brace|.
 ***************************************************************************/
  BEGIN
    BST_GET_AND_CHECK_LEFT_BRACE ("macro");
    EAT_BST_WHITE_AND_EOF_CHECK ("macro");
    BST_IDENTIFIER_SCAN ("macro");

/***************************************************************************
 * WEB section number:	207
 * ~~~~~~~~~~~~~~~~~~~
 * The macro name must be a new one; we mark it as |macro_ilk|.
 ***************************************************************************/
    BEGIN

#ifdef TRACE
      if (Flag_trace) {
        TRACE_PR_TOKEN;
        TRACE_PR_LN (" is a macro");
      }
#endif                      			/* TRACE */

      lower_case (buffer, buf_ptr1, TOKEN_LEN);
      macro_name_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN, MACRO_ILK,
				   DO_INSERT);
      if (hash_found)
      BEGIN
	PRINT_TOKEN;
	BST_ERR (" is already defined as a macro");
      END
      ilk_info[macro_name_loc] = hash_text[macro_name_loc];
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 207 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    EAT_BST_WHITE_AND_EOF_CHECK ("macro");
    BST_GET_AND_CHECK_RIGHT_BRACE ("macro");
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 206 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  EAT_BST_WHITE_AND_EOF_CHECK ("macro");

/***************************************************************************
 * WEB section number:	208
 * ~~~~~~~~~~~~~~~~~~~
 * This module reads a |left_brace|, the |double_quote|-delimited string
 * that defines this macro, and a |right_brace|.
 ***************************************************************************/
  BEGIN
    BST_GET_AND_CHECK_LEFT_BRACE ("macro");
    EAT_BST_WHITE_AND_EOF_CHECK ("macro");
    if (SCAN_CHAR != DOUBLE_QUOTE)
    BEGIN
       BST_ERR2 ("A macro definition must be %c-delimited",
		 xchr[DOUBLE_QUOTE]);
    END

/***************************************************************************
 * WEB section number:	209
 * ~~~~~~~~~~~~~~~~~~~
 * A macro definition-string is preceded by a |double_quote| and consists
 * of all characters on this line up to the next |double_quote|.  The
 * array |ilk_info| contains a pointer to this string for the macro name.
 ***************************************************************************/
    BEGIN
      INCR (buf_ptr2);
      if ( ! scan1 (DOUBLE_QUOTE))
      BEGIN
	BST_ERR2 ("There's no `%c' to end macro definition",
		   xchr[DOUBLE_QUOTE]);
      END

#ifdef TRACE
      if (Flag_trace) {
        TRACE_PR ("\"");
        TRACE_PR_TOKEN;
        TRACE_PR ("\"");
        TRACE_PR_LN (" is a macro string");
      }
#endif                      			/* TRACE */

      macro_def_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN, TEXT_ILK,
				  DO_INSERT);
      fn_type[macro_def_loc] = STR_LITERAL;
      ilk_info[macro_name_loc] = hash_text[macro_def_loc];
      INCR (buf_ptr2);
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 209 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    EAT_BST_WHITE_AND_EOF_CHECK ("macro");
    BST_GET_AND_CHECK_RIGHT_BRACE ("macro");
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 208 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 205 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 294
 * ~~~~~~~~~~~~~~~~~~~
 * When an error is so harmless, we print a |warning_message| instead of
 * an |error_message|.
 ***************************************************************************/
void          bst_mild_ex_warn_print (void)
BEGIN
  if (mess_with_entries)
  BEGIN
    PRINT (" for entry ");
    PRINT_POOL_STR (CUR_CITE_STR);
  END
  PRINT_NEWLINE;
  BST_WARN ("while executing");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 294 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 211
 * ~~~~~~~~~~~~~~~~~~~
 * The read command has no arguments so there's no more parsing to do.
 * We must make sure we haven't seen a read command before and we've
 * already seen an entry command.
 ***************************************************************************/
void          bst_read_command (void)
BEGIN
  if (read_seen)
  BEGIN
    BST_ERR ("Illegal, another read command");
  END
  read_seen = TRUE;
  if ( ! entry_seen)
  BEGIN
    BST_ERR ("Illegal, read command before entry command");
  END
  sv_ptr1 = buf_ptr2;
  sv_ptr2 = last;
  tmp_ptr = sv_ptr1;
  while (tmp_ptr < sv_ptr2)
  BEGIN
    sv_buffer[tmp_ptr] = buffer[tmp_ptr];
    INCR (tmp_ptr);
  END

/***************************************************************************
 * WEB section number:	223
 * ~~~~~~~~~~~~~~~~~~~
 * For all |num_bib_files| database files, we keep reading and processing
 * .bib entries until none left.
 ***************************************************************************/
  BEGIN

/***************************************************************************
 * WEB section number:	224
 * ~~~~~~~~~~~~~~~~~~~
 * We need to initialize the |field_info| array, and also various things
 * associated with the |cite_list| array (but not |cite_list| itself).
 ***************************************************************************/
    BEGIN

/***************************************************************************
 * WEB section number:	225
 * ~~~~~~~~~~~~~~~~~~~
 * This module initializes all fields of all entries to |missing|, the
 * value to which all fields are initialized.
 ***************************************************************************/
      BEGIN
	check_field_overflow (num_fields * num_cites);
	field_ptr = 0;
	while (field_ptr < Max_Fields)
	BEGIN
	  field_info[field_ptr] = MISSING;
	  INCR (field_ptr);
	END
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 225 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	227
 * ~~~~~~~~~~~~~~~~~~~
 * We must initialize the |type_list| array so that we can detect
 * duplicate (or missing) entries for cite keys on |cite_list|.  Also,
 * when we're to include the entire database, we use the array
 * |entry_exists| to detect those missing entries whose |cite_list| info
 * will (or to be more precise, might) be overwritten; and we use the
 * array |cite_info| to save the part of |cite_list| that will (might) be
 * overwritten.  We also use |cite_info| for counting cross~references
 * when it's appropriate---when an entry isn't otherwise to be included
 * on |cite_list| (that is, the entry isn't \cited or
 * \nocited).  Such an entry is included on the final |cite_list| if
 * it's cross~referenced at least |min_crossrefs| times.
 ***************************************************************************/
      BEGIN
	cite_ptr = 0;
	while (cite_ptr < Max_Cites)
	BEGIN
	  type_list[cite_ptr] = EMPTY;
	  cite_info[cite_ptr] = ANY_VALUE;
	  INCR (cite_ptr);
	END
	old_num_cites = num_cites;
	if (all_entries)
	BEGIN
	  cite_ptr = all_marker;
	  while (cite_ptr < old_num_cites)
	  BEGIN
	    cite_info[cite_ptr] = cite_list[cite_ptr];
	    entry_exists[cite_ptr] = FALSE;
	    INCR (cite_ptr);
	  END
	  cite_ptr = all_marker;
	END
	else
	BEGIN
	  cite_ptr = num_cites;
	  all_marker = ANY_VALUE;
	END
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 227 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 224 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    read_performed = TRUE;
    bib_ptr = 0;
    while (bib_ptr < num_bib_files)
    BEGIN
      PRINT2 ("Database file #%ld: ", (long) bib_ptr + 1);
      print_bib_name ();
      bib_line_num = 0;
      buf_ptr2 = last;
      while ( ! feof (CUR_BIB_FILE))
      BEGIN
	get_bib_command_or_entry_and_pr ();
      END
      a_close (CUR_BIB_FILE);
      INCR (bib_ptr);
    END
    reading_completed = TRUE;

#ifdef TRACE
    if (Flag_trace)
      TRACE_PR_LN ("Finished reading the database file(s)");
#endif                      			/* TRACE */

/***************************************************************************
 * WEB section number:	276
 * ~~~~~~~~~~~~~~~~~~~
 * This gets things ready for further .bst processing.
 ***************************************************************************/
    BEGIN
      num_cites = cite_ptr;
      num_preamble_strings = preamble_ptr;

/***************************************************************************
 * WEB section number:	277
 * ~~~~~~~~~~~~~~~~~~~
 * Now we update any entry (here called a child entry) that
 * cross referenced another (here called a parent entry); this
 * cross referencing occurs when the child's crossref field (value)
 * consists of the parent's database key.  To do the update, we replace
 * the child's |missing| fields by the corresponding fields of the
 * parent.  Also, we make sure the crossref field contains the
 * case-correct version.  Finally, although it is technically illegal to
 * nest cross references, and although we give a warning (a few modules
 * hence) when someone tries, we do what we can to accommodate the
 * attempt.
 ***************************************************************************/
      BEGIN
	cite_ptr = 0;
	while (cite_ptr < num_cites)
	BEGIN
	  field_ptr = (cite_ptr * num_fields) + crossref_num;
	  if (field_info[field_ptr] != MISSING)
	  BEGIN
	    if (find_cite_locs_for_this_cite_ke (field_info[field_ptr]))
	    BEGIN
	      cite_loc = ilk_info[lc_cite_loc];
	      field_info[field_ptr] = hash_text[cite_loc];
	      cite_parent_ptr = ilk_info[cite_loc];
	      field_ptr = (cite_ptr * num_fields) + num_pre_defined_fields;
	      field_end_ptr = field_ptr - num_pre_defined_fields + num_fields;
	      field_parent_ptr = (cite_parent_ptr * num_fields)
				   + num_pre_defined_fields;
	      while (field_ptr < field_end_ptr)
	      BEGIN
		if (field_info[field_ptr] == MISSING)
		BEGIN
		  field_info[field_ptr] = field_info[field_parent_ptr];
		END
		INCR (field_ptr);
		INCR (field_parent_ptr);
	      END
	    END
	  END
	  INCR (cite_ptr);
	END
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 277 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	279
 * ~~~~~~~~~~~~~~~~~~~
 * Here we remove the crossref field value for each child whose
 * parent was cross referenced too few times.  We also issue any
 * necessary warnings arising from a bad cross reference.
 ***************************************************************************/
      BEGIN
	cite_ptr = 0;
	while (cite_ptr < num_cites)
	BEGIN
	  field_ptr = (cite_ptr * num_fields) + crossref_num;
	  if (field_info[field_ptr] != MISSING)
	  BEGIN
	    if ( ! find_cite_locs_for_this_cite_ke (field_info[field_ptr]))
	    BEGIN
	      if (cite_hash_found)
	      BEGIN
		hash_cite_confusion ();
	      END
	      non_existent_cross_reference_er ();
	      field_info[field_ptr] = MISSING;
	    END
	    else
	    BEGIN
	      if (cite_loc != ilk_info[lc_cite_loc])
	      BEGIN
		hash_cite_confusion ();
	      END
	      cite_parent_ptr = ilk_info[cite_loc];
	      if (type_list[cite_parent_ptr] == EMPTY)
	      BEGIN
		non_existent_cross_reference_er ();
		field_info[field_ptr] = MISSING;
	      END
	      else
	      BEGIN
		field_parent_ptr = (cite_parent_ptr * num_fields)
				    + crossref_num;
		if (field_info[field_parent_ptr] != MISSING)

/***************************************************************************
 * WEB section number:	282
 * ~~~~~~~~~~~~~~~~~~~
 * We also complain when an entry being cross referenced has a
 * non|missing| crossref field itself, but this one is just a
 * warning, not a full-blown error.
 ***************************************************************************/
		BEGIN
		  PRINT ("Warning--you've nested cross references");
		  bad_cross_reference_print (cite_list[cite_parent_ptr]);
		  PRINT_LN ("\", which also refers to something");
		  mark_warning ();
		END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 282 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

		if (( ! all_entries) && (cite_parent_ptr >= old_num_cites)
		      && (cite_info[cite_parent_ptr] < Min_Crossrefs))
		BEGIN
		  field_info[field_ptr] = MISSING;
		END
	      END
	    END
	  END
	  INCR (cite_ptr);
	END
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 279 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	283
 * ~~~~~~~~~~~~~~~~~~~
 * We remove (and give a warning for) each cite key on the original
 * |cite_list| without a corresponding database entry.  And we remove any
 * entry that was included on |cite_list| only because it was
 * cross~referenced, yet was cross~referenced fewer than |min_crossrefs|
 * times.  Throughout this module, |cite_ptr| points to the next cite key
 * to be checked and |cite_xptr| points to the next permanent spot on
 * |cite_list|.
 ***************************************************************************/
      BEGIN
	cite_ptr = 0;
	while (cite_ptr < num_cites)
	BEGIN
	  if (type_list[cite_ptr] == EMPTY)
	  BEGIN
	    print_missing_entry (CUR_CITE_STR);
	  END
	  else if ((all_entries) || (cite_ptr < old_num_cites)
		      || (cite_info[cite_ptr] >= Min_Crossrefs))
	  BEGIN
	    if (cite_ptr > cite_xptr)

/***************************************************************************
 * WEB section number:	285
 * ~~~~~~~~~~~~~~~~~~~
 * We have to move to its final resting place all the entry information
 * associated with the exact location in |cite_list| of this cite key.
 ***************************************************************************/
	    BEGIN
	      cite_list[cite_xptr] = cite_list[cite_ptr];
	      type_list[cite_xptr] = type_list[cite_ptr];
	      if ( ! find_cite_locs_for_this_cite_ke (cite_list[cite_ptr]))
	      BEGIN
		cite_key_disappeared_confusion ();
	      END
	      if (( ! cite_hash_found) || (cite_loc != ilk_info[lc_cite_loc]))
	      BEGIN
		hash_cite_confusion ();
	      END
	      ilk_info[cite_loc] = cite_xptr;
	      field_ptr = cite_xptr * num_fields;
	      field_end_ptr = field_ptr + num_fields;
	      tmp_ptr = cite_ptr * num_fields;
	      while (field_ptr < field_end_ptr)
	      BEGIN
		field_info[field_ptr] = field_info[tmp_ptr];
		INCR (field_ptr);
		INCR (tmp_ptr);
	      END
	    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 285 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    INCR (cite_xptr);
	  END
	  INCR (cite_ptr);
	END
	num_cites = cite_xptr;
	if (all_entries)

/***************************************************************************
 * WEB section number:	286
 * ~~~~~~~~~~~~~~~~~~~
 * We need this module only when we're including the whole database.
 * It's for missing entries whose cite key originally resided in
 * |cite_list| at a spot that another cite key (might have) claimed.
 ***************************************************************************/
	BEGIN
	  cite_ptr = all_marker;
	  while (cite_ptr < old_num_cites)
	  BEGIN
	    if ( ! entry_exists[cite_ptr])
	    BEGIN
	      print_missing_entry (cite_info[cite_ptr]);
	    END
	    INCR (cite_ptr);
	  END
	END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 286 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 283 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	287
 * ~~~~~~~~~~~~~~~~~~~
 * This module initializes all |int_entry_var|s of all entries to 0, the
 * value to which all integers are initialized.
 ***************************************************************************/
      BEGIN
	int_ent_ptr = (num_ent_ints + 1) * (num_cites + 1);
	entry_ints = (Integer_T *) mymalloc ((unsigned long) sizeof (Integer_T)
	    * (unsigned long) int_ent_ptr, "entry_ints");
	while (int_ent_ptr > 0)
	BEGIN
	  DECR (int_ent_ptr);
	  entry_ints[int_ent_ptr] = 0;
	END
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 287 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	288
 * ~~~~~~~~~~~~~~~~~~~
 * This module initializes all |str_entry_var|s of all entries to the
 * null string, the value to which all strings are initialized.
 ***************************************************************************/
      BEGIN
	str_ent_ptr = (num_ent_strs + 1) * (num_cites + 1);
	entry_strs = (ASCIICode_T *) mymalloc ((unsigned long) sizeof (ASCIICode_T)
	    * (unsigned long) (Ent_Str_Size + 1)
	    * (unsigned long) str_ent_ptr, "entry_strs");
	while (str_ent_ptr > 0)
	BEGIN
	  DECR (str_ent_ptr);
	  ENTRY_STRS(str_ent_ptr,0) = END_OF_STRING;
	END
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 288 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	289
 * ~~~~~~~~~~~~~~~~~~~
 * The array |sorted_cites| initially specifies that the entries are to
 * be processed in order of cite-key occurrence.  The sort command
 * may change this to whatever it likes (which, we hope, is whatever the
 * style-designer instructs it to like).  We make |sorted_cites| an alias
 * to save space; this works fine because we're done with |cite_info|.
 ***************************************************************************/
      BEGIN
	cite_ptr = 0;
	while (cite_ptr < num_cites)
	BEGIN
	  SORTED_CITES[cite_ptr] = cite_ptr;
	  INCR (cite_ptr);
	END
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 289 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 276 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    read_completed = TRUE;
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 223 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  buf_ptr2 = sv_ptr1;
  last = sv_ptr2;
  tmp_ptr = buf_ptr2;
  while (tmp_ptr < last)
  BEGIN
    buffer[tmp_ptr] = sv_buffer[tmp_ptr];
    INCR (tmp_ptr);
  END
Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 211 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 212
 * ~~~~~~~~~~~~~~~~~~~
 * A reverse command has one argument, a single |built_in| or
 * |wiz_defined| function name between braces.  Upper/lower cases are
 * considered to be the same---all upper-case letters are converted to
 * lower case.  Also, we must make sure we've already seen a read
 * command.
 *
 * This module reads a |left_brace|, a single function to be iterated in
 * reverse, and a |right_brace|.
 ***************************************************************************/
void          bst_reverse_command (void)
BEGIN
  if ( ! read_seen)
  BEGIN
    BST_ERR ("Illegal, reverse command before read command");
  END
  EAT_BST_WHITE_AND_EOF_CHECK ("reverse");
  BST_GET_AND_CHECK_LEFT_BRACE ("reverse");
  EAT_BST_WHITE_AND_EOF_CHECK ("reverse");
  BST_IDENTIFIER_SCAN ("reverse");

/***************************************************************************
 * WEB section number:	213
 * ~~~~~~~~~~~~~~~~~~~
 * Before iterating the function, we must make sure it's a legal one.  It
 * must exist and be |built_in| or |wiz_defined|.
 ***************************************************************************/
  BEGIN

#ifdef TRACE
    if (Flag_trace) {
      TRACE_PR_TOKEN;
      TRACE_PR_LN (" is a to be iterated in reverse function");
    }
#endif                      			/* TRACE */

    if (bad_argument_token ())
    BEGIN
      goto Exit_Label;
    END
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 213 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  EAT_BST_WHITE_AND_EOF_CHECK ("reverse");
  BST_GET_AND_CHECK_RIGHT_BRACE ("reverse");

/***************************************************************************
 * WEB section number:	298
 * ~~~~~~~~~~~~~~~~~~~
 * This module iterates a single specified function for all entries
 * specified by |cite_list|, but does it in reverse order.
 ***************************************************************************/
  BEGIN
    init_command_execution ();
    mess_with_entries = TRUE;
    if (num_cites > 0)
    BEGIN
      sort_cite_ptr = num_cites;
      REPEAT
      BEGIN
	DECR (sort_cite_ptr);
	cite_ptr = SORTED_CITES[sort_cite_ptr];

#ifdef TRACE
        if (Flag_trace) {
  	  TRACE_PR_POOL_STR (hash_text[fn_loc]);
	  TRACE_PR (" to be iterated in reverse on ");
	  TRACE_PR_POOL_STR (CUR_CITE_STR);
	  TRACE_PR_NEWLINE;
        }
#endif                      			/* TRACE */

	execute_fn (fn_loc);
	check_command_execution ();
      END UNTIL (sort_cite_ptr == 0);
    END
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 298 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 212 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 168
 * ~~~~~~~~~~~~~~~~~~~
 * And this one, a |right_brace|.
 ***************************************************************************/
void          bst_right_brace_print (void)
BEGIN
  PRINT2 ("\"%c\" is missing in command: ", xchr[RIGHT_BRACE]);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 168 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 214
 * ~~~~~~~~~~~~~~~~~~~
 * The sort command has no arguments so there's no more parsing to do,
 * but we must make sure we've already seen a read command.
 ***************************************************************************/
void          bst_sort_command (void)
BEGIN
  if ( ! read_seen)
  BEGIN
    BST_ERR ("Illegal, sort command before read command");
  END

/***************************************************************************
 * WEB section number:	299
 * ~~~~~~~~~~~~~~~~~~~
 * This module sorts the entries based on sort.key$; it is a stable
 * sort.
 ***************************************************************************/
  BEGIN

#ifdef TRACE
    if (Flag_trace)
      TRACE_PR_LN ("Sorting the entries");
#endif                      			/* TRACE */

    if (num_cites > 1)
    BEGIN
      quick_sort (0, num_cites - 1);
    END

#ifdef TRACE
    if (Flag_trace)
      TRACE_PR_LN ("Done sorting");
#endif                      			/* TRACE */

  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 299 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 214 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 215
 * ~~~~~~~~~~~~~~~~~~~
 * A strings command has one argument, a list of function names
 * between braces (the names are separated by one or more |white_space|
 * characters).  Upper/lower cases are considered to be the same for
 * function names in these lists---all upper-case letters are converted to
 * lower case.  Each name in this list specifies a |str_global_var|.
 * There may be several strings commands in the .bst file.
 *
 * This module reads a |left_brace|, a list of |str_global_var|s, and a
 * |right_brace|.
 ***************************************************************************/
void          bst_strings_command (void)
BEGIN
  EAT_BST_WHITE_AND_EOF_CHECK ("strings");
  BST_GET_AND_CHECK_LEFT_BRACE ("strings");
  EAT_BST_WHITE_AND_EOF_CHECK ("strings");
  while (SCAN_CHAR != RIGHT_BRACE)
  BEGIN
    BST_IDENTIFIER_SCAN ("strings");

/***************************************************************************
 * WEB section number:	216
 * ~~~~~~~~~~~~~~~~~~~
 * Here we insert the just found |str_global_var| name into the hash
 * table, record it as a |str_global_var|, set its pointer into
 * |global_strs|, and initialize its value there to the null string.
 ***************************************************************************/
    BEGIN

#ifdef TRACE
      if (Flag_trace) {
        TRACE_PR_TOKEN;
        TRACE_PR_LN (" is a string global-variable");
      }
#endif                      			/* TRACE */

      lower_case (buffer, buf_ptr1, TOKEN_LEN);
      fn_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN, BST_FN_ILK,
			   DO_INSERT);
      CHECK_FOR_ALREADY_SEEN_FUNCTION (fn_loc);
      fn_type[fn_loc] = STR_GLOBAL_VAR;
      FN_INFO[fn_loc] = num_glb_strs;
      if (num_glb_strs == Max_Glob_Strs)
      BEGIN
        BIB_XRETALLOC_NOSET ("glb_str_ptr", glb_str_ptr, StrNumber_T,
                             Max_Glob_Strs, Max_Glob_Strs + MAX_GLOB_STRS);
        BIB_XRETALLOC_STRING ("global_strs", global_strs, Glob_Str_Size,
                              Max_Glob_Strs, Max_Glob_Strs + MAX_GLOB_STRS);
        BIB_XRETALLOC ("glb_str_end", glb_str_end, Integer_T,
                       Max_Glob_Strs, Max_Glob_Strs + MAX_GLOB_STRS);
        str_glb_ptr = num_glb_strs;
        while (str_glb_ptr < Max_Glob_Strs)
        BEGIN
          glb_str_ptr[str_glb_ptr] = 0;
          glb_str_end[str_glb_ptr] = 0;
          INCR (str_glb_ptr);
        END
      END
      INCR (num_glb_strs);
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 216 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      EAT_BST_WHITE_AND_EOF_CHECK ("strings");

  END
  INCR (buf_ptr2);
Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 215 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 150
 * ~~~~~~~~~~~~~~~~~~~
 * When there's a harmless error parsing the .bst file (harmless
 * syntactically, at least) we give just a |warning_message|.
 ***************************************************************************/
void          bst_warn_print (void)
BEGIN
  bst_ln_num_print ();
  mark_warning ();
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 150 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 46
 * ~~~~~~~~~~~~~~~~~~~
 * When a buffer overflows, it's time to complain (and then quit).
 ***************************************************************************/
void          buffer_overflow (void)
BEGIN
  BIB_XRETALLOC_NOSET ("buffer", buffer, ASCIICode_T,
                       Buf_Size, Buf_Size + BUF_SIZE);
  BIB_XRETALLOC_NOSET ("ex_buf", ex_buf, ASCIICode_T,
                       Buf_Size, Buf_Size + BUF_SIZE);
  BIB_XRETALLOC_NOSET ("name_sep_char", name_sep_char, ASCIICode_T,
                       Buf_Size, Buf_Size + BUF_SIZE);
  BIB_XRETALLOC_NOSET ("name_tok", name_tok, BufPointer_T,
                       Buf_Size, Buf_Size + BUF_SIZE);
  BIB_XRETALLOC_NOSET ("out_buf", out_buf, ASCIICode_T,
                       Buf_Size, Buf_Size + BUF_SIZE);
  BIB_XRETALLOC ("sv_buffer", sv_buffer, ASCIICode_T,
                 Buf_Size, Buf_Size + BUF_SIZE);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  46 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 335
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure inserts a |built_in| function into the hash table and
 * initializes the corresponding pre-defined string (of length at most
 * |longest_pds|).  The array |FN_INFO| contains a number from 0 through
 * the number of |built_in| functions minus 1 (i.e., |num_blt_in_fns - 1|
 * if we're keeping statistics); this number is used by a |case| statement
 * to execute this function and is used for keeping execution counts when
 * keeping statistics.
 ***************************************************************************/
void          build_in (PdsType_T pds, PdsLen_T len,
			HashLoc_T *fn_hash_loc, BltInRange_T blt_in_num)
BEGIN
  pre_define (pds, len, BST_FN_ILK);
  *fn_hash_loc = pre_def_loc;
  fn_type[*fn_hash_loc] = BUILT_IN;
  FN_INFO[*fn_hash_loc] = blt_in_num;

#ifdef STAT
  if (Flag_stats) {
    blt_in_loc[blt_in_num] = *fn_hash_loc;
    execution_count[blt_in_num] = 0;
  }
#endif                      			/* STAT */

END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 335 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 373
 * ~~~~~~~~~~~~~~~~~~~
 * Another bug complaint.
 ***************************************************************************/
void          case_conversion_confusion (void)
BEGIN
  CONFUSION ("Unknown type of case conversion");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 373 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 369
 * ~~~~~~~~~~~~~~~~~~~
 * This one makes sure that |brace_level=0| (it's called at a point in a
 * string where braces must be balanced).
 ***************************************************************************/
void          check_brace_level (StrNumber_T pop_lit_var)
BEGIN
  if (brace_level > 0)
  BEGIN
    braces_unbalanced_complaint (pop_lit_var);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 369 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 138
 * ~~~~~~~~~~~~~~~~~~~
 * Complain if somebody's got a cite fetish.  This procedure is called
 * when were about to add another cite key to |cite_list|.  It assumes
 * that |cite_loc| gives the potential cite key's hash table location.
 ***************************************************************************/
void          check_cite_overflow (CiteNumber_T last_cite)
BEGIN
  if (last_cite == Max_Cites)
  BEGIN
    BIB_XRETALLOC_NOSET ("cite_info", cite_info, StrNumber_T,
                         Max_Cites, Max_Cites + MAX_CITES);
    BIB_XRETALLOC_NOSET ("cite_list", cite_list, StrNumber_T,
                         Max_Cites, Max_Cites + MAX_CITES);
    BIB_XRETALLOC_NOSET ("entry_exists", entry_exists, Boolean_T,
                         Max_Cites, Max_Cites + MAX_CITES);
    BIB_XRETALLOC ("type_list", type_list, HashPtr2_T,
                   Max_Cites, Max_Cites + MAX_CITES);
    while (last_cite < Max_Cites)
    BEGIN
      type_list[last_cite] = EMPTY;
      cite_info[last_cite] = ANY_VALUE;
      INCR (last_cite);
    END
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 138 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 317
 * ~~~~~~~~~~~~~~~~~~~
 * At the end of a .bst command-execution we check that the stack and
 * |str_pool| are still in good shape.
 ***************************************************************************/
void          check_command_execution (void)
BEGIN
  if (lit_stk_ptr != 0)
  BEGIN
    PRINT_LN2 ("ptr=%ld, stack=", (long) lit_stk_ptr);
    pop_whole_stack ();
    PRINT ("---the literal stack isn't empty");
    bst_ex_warn_print ();
  END
  if (cmd_str_ptr != str_ptr)
  BEGIN

#ifdef TRACE
    if (Flag_trace)
      TRACE_PR_LN3 ("Pointer is %ld but should be %ld", (long) str_ptr,
	  	    (long) cmd_str_ptr);
#endif                      			/* TRACE */

    CONFUSION ("Nonempty empty string stack");
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 317 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 226
 * ~~~~~~~~~~~~~~~~~~~
 * Complain if somebody's got a field fetish.
 ***************************************************************************/
void          check_field_overflow (Integer_T total_fields)
BEGIN
  if (total_fields > Max_Fields)
  BEGIN
    field_ptr = Max_Fields;
    BIB_XRETALLOC ("field_info", field_info, StrNumber_T,
                   Max_Fields, total_fields + MAX_FIELDS);
    /* Initialize to |missing|.  */
    while (field_ptr < Max_Fields)
    BEGIN
      field_info[field_ptr] = MISSING;
      INCR (field_ptr);
    END
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 226 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 271
 * ~~~~~~~~~~~~~~~~~~~
 * Here's another bug complaint.
 ***************************************************************************/
void          cite_key_disappeared_confusion (void)
BEGIN
  CONFUSION ("A cite key disappeared");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 271 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 252
 * ~~~~~~~~~~~~~~~~~~~
 * The .bib-specific scanning function |compress_bib_white| skips over
 * |white_space| characters within a string until hitting a nonwhite
 * character; in fact, it does everything |eat_bib_white_space| does, but
 * it also adds a |space| to |field_vl_str|.  This function is never
 * called if there are no |white_space| characters (or ends-of-line) to be
 * scanned (though the associated macro might be).  The function returns
 * |false| if there is a serious syntax error.
 ***************************************************************************/
Boolean_T         compress_bib_white (void)
BEGIN
  Boolean_T       compress_bib_white;

  compress_bib_white = FALSE;
  COPY_CHAR (SPACE);
  while ( ! scan_white_space ())
  BEGIN
    if ( ! input_ln (CUR_BIB_FILE))
    BEGIN
      eat_bib_print ();
      goto Exit_Label;
    END
    INCR (bib_line_num);
    buf_ptr2 = 0;
  END
  compress_bib_white = TRUE;
Exit_Label: DO_NOTHING;
  return (compress_bib_white);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 252 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 367
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure complains if the just-encountered |right_brace| would
 * make |brace_level| negative.
 ***************************************************************************/
void          decr_brace_level (StrNumber_T pop_lit_var)
BEGIN
  if (brace_level == 0)
  BEGIN
    braces_unbalanced_complaint (pop_lit_var);
  END
  else
  BEGIN
    DECR (brace_level);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 367 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 229
 * ~~~~~~~~~~~~~~~~~~~
 * It's often illegal to end a .bib command in certain places, and
 * this is where we come to check.
 ***************************************************************************/
void          eat_bib_print (void)
BEGIN
  BIB_ERR (" Illegal end of database file");
Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 229 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 228
 * ~~~~~~~~~~~~~~~~~~~
 * Before we actually start the code for reading a database file, we must
 * define this .bib-specific scanning function.  It skips over
 * |white_space| characters until hitting a nonwhite character or the end
 * of the file, respectively returning |true| or |false|.  It also updates
 * |bib_line_num|, the line counter.
 ***************************************************************************/
Boolean_T         eat_bib_white_space (void)
BEGIN
  Boolean_T       eat_bib_white_space;

  while ( ! scan_white_space ())
  BEGIN
    if ( ! input_ln (CUR_BIB_FILE))
    BEGIN
      eat_bib_white_space = FALSE;
      goto Exit_Label;
    END
    INCR (bib_line_num);
    buf_ptr2 = 0;
  END
  eat_bib_white_space = TRUE;
Exit_Label:
  return (eat_bib_white_space);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 228 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 153
 * ~~~~~~~~~~~~~~~~~~~
 * It's often illegal to end a .bst command in certain places, and
 * this is where we come to check.
 ***************************************************************************/
void          eat_bst_print (void)
BEGIN
  PRINT ("Illegal end of style file in command: ");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 153 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 152
 * ~~~~~~~~~~~~~~~~~~~
 * This .bst-specific scanning function skips over |white_space|
 * characters (and comments) until hitting a nonwhite character or the end
 * of the file, respectively returning |true| or |false|.  It also updates
 * |bst_line_num|, the line counter.
 ***************************************************************************/
Boolean_T         eat_bst_white_space (void)
BEGIN
  Boolean_T	  eat_bst_white_space;

  LOOP
  BEGIN
    if (scan_white_space ())
    BEGIN
      if (SCAN_CHAR != COMMENT)
      BEGIN
	eat_bst_white_space = TRUE;
	goto Exit_Label;
      END
    END
    if ( ! input_ln (bst_file))
    BEGIN
      eat_bst_white_space = FALSE;
      goto Exit_Label;
    END
    INCR (bst_line_num);
    buf_ptr2 = 0;
  END
Exit_Label:
  return (eat_bst_white_space);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 152 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 418
 * ~~~~~~~~~~~~~~~~~~~
 * This function looks at the string in |ex_buf|, starting at
 * |ex_buf_xptr| and ending just before |ex_buf_ptr|, and it returns
 * |true| if there are |enough_chars|, where a special character (even if
 * it's missing its matching |right_brace|) counts as a single charcter.
 * This procedure is called only for strings that don't have too many
 * |right_brace|s.
 ***************************************************************************/
Boolean_T         enough_text_chars (BufPointer_T enough_chars)
BEGIN
  Boolean_T	  enough_text_chars;
  num_text_chars = 0;
  ex_buf_yptr = ex_buf_xptr;
  while ((ex_buf_yptr < ex_buf_ptr) && (num_text_chars < enough_chars))
  BEGIN
    INCR (ex_buf_yptr);
    if (ex_buf[ex_buf_yptr - 1] == LEFT_BRACE)
    BEGIN
      INCR (brace_level);
      if ((brace_level == 1) && (ex_buf_yptr < ex_buf_ptr))
      BEGIN
        if (ex_buf[ex_buf_yptr] == BACKSLASH)
        BEGIN
          INCR (ex_buf_yptr);
          while ((ex_buf_yptr < ex_buf_ptr) && (brace_level > 0))
          BEGIN
            if (ex_buf[ex_buf_yptr] == RIGHT_BRACE)
            BEGIN
              DECR (brace_level);
            END
            else if (ex_buf[ex_buf_yptr] == LEFT_BRACE)
            BEGIN
              INCR (brace_level);
            END
            INCR (ex_buf_yptr);
          END
        END
      END
    END
    else if (ex_buf[ex_buf_yptr - 1] == RIGHT_BRACE)
    BEGIN
      DECR (brace_level);
    END
    INCR (num_text_chars);
  END
  if (num_text_chars < enough_chars)
  BEGIN
    enough_text_chars = FALSE;
  END
  else
  BEGIN
    enough_text_chars = TRUE;
  END
  return (enough_text_chars);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 418 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
