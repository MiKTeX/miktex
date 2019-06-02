/*-
******************************************************************************
******************************************************************************
**
**  MODULE
**
**      $RCSfile: bibtex-2.c,v $
**      $Revision: 3.71 $
**      $Date: 1996/08/18 20:47:30 $
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
**      This is the second of 4 source modules for BibTeX.  The source has
**      been split into 4 parts so that some of the more primitive editors
**      can cope.  This code mimics the BibTeX WEB source as closely as
**      possible and there should be NO system dependent code in any of the 
**      bibtex-#.c modules.
**
**      The functions defined in this module are:
**
**	    execute_fn
**	    figure_out_the_formatted_name
**	    find_cite_locs_for_this_cite_ke
**	    get_aux_command_and_process
**	    get_bib_command_or_entry_and_pr
**          get_bst_command_and_process
**          get_the_top_level_aux_file_name
**          hash_cite_confusion
**          id_scanning_confusion
**          illegl_literal_confusion
**          init_command_execution
**          initialize
**          input_ln
**          int_to_ASCII
**          last_check_for_aux_errors
**          less_than
**          lower_case
**          macro_warn_print
**          make_string
**          mark_error
**          mark_fatal
**          mark_warning
**          name_scan_for_and
**          non_existent_cross_reference_er
**          out_pool_str
**          output_bbl_line
**          out_token
**          pool_overflow
**          pop_lit_stk
**          pop_the_aux_stack
**          pop_top_and_print
**          pop_whole_stack
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
**      $Log: bibtex-2.c,v $
**      Revision 3.71  1996/08/18  20:47:30  kempson
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
**      Revision 3.4  1995/04/09  22:15:39  kempson
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
 * WEB section number:	 325
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure executes a single specified function; it is the single
 * execution-primitive that does everything (except windows, and it takes
 * Tuesdays off).
 ***************************************************************************/
void          execute_fn (HashLoc_T ex_fn_loc)
BEGIN

/***************************************************************************
 * WEB section number:	343
 * ~~~~~~~~~~~~~~~~~~~
 * Now it's time to declare some things for executing |built_in|
 * functions only.  These (and only these) variables are used
 * recursively, so they can't be global.
 ***************************************************************************/
  Integer_T         r_pop_lt1,
		    r_pop_lt2;
  StkType_T         r_pop_tp1,
		    r_pop_tp2;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 343 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  WizFnLoc_T        wiz_ptr;

#ifdef TRACE
  if (Flag_trace) {
    TRACE_PR ("execute_fn `");
    TRACE_PR_POOL_STR (hash_text[ex_fn_loc]);
    TRACE_PR_LN ("'");
  }
#endif                      			/* TRACE */

  switch (fn_type[ex_fn_loc])
  BEGIN
    case BUILT_IN:
/***************************************************************************
 * WEB section number:	341
 * ~~~~~~~~~~~~~~~~~~~
 * This module branches to the code for the appropriate |built_in|
 * function.  Only three---call.type$, if$, and while$---do a recursive call.
 ***************************************************************************/
      BEGIN

#ifdef STAT
        if (Flag_stats)
          INCR (execution_count[FN_INFO[ex_fn_loc]]);
#endif                      			/* STAT */

	switch (FN_INFO[ex_fn_loc])
	BEGIN
	  case N_EQUALS:
	    x_equals ();
	    break;
	  case N_GREATER_THAN:
	    x_greater_than ();
	    break;
	  case N_LESS_THAN:
	    x_less_than ();
	    break;
	  case N_PLUS:
	    x_plus ();
	    break;
	  case N_MINUS:
	    x_minus ();
	    break;
	  case N_CONCATENATE:
	    x_concatenate ();
	    break;
	  case N_GETS:
	    x_gets ();
	    break;
	  case N_ADD_PERIOD:
	    x_add_period ();
	    break;
	  case N_CALL_TYPE:

/***************************************************************************
 * WEB section number:	363
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function call.type$ executes the function
 * specified in |type_list| for this entry unless it's |undefined|, in
 * which case it executes the default function default.type defined
 * in the .bst file, or unless it's |empty|, in which case it does
 * nothing.
 ***************************************************************************/
	    BEGIN
	      if ( ! mess_with_entries)
	      BEGIN
		bst_cant_mess_with_entries_prin ();
	      END
	      else if (type_list[cite_ptr] == UNDEFINED)
	      BEGIN
		execute_fn (b_default);
	      END
	      else if (type_list[cite_ptr] == EMPTY)
	      BEGIN
		DO_NOTHING;
	      END
	      else
	      BEGIN
		execute_fn (type_list[cite_ptr]);
	      END
	    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 363 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    break;
	  case N_CHANGE_CASE:
	    x_change_case ();
	    break;
	  case N_CHR_TO_INT:
	    x_chr_to_int ();
	    break;
	  case N_CITE:
	    x_cite ();
	    break;
	  case N_DUPLICATE:
	    x_duplicate ();
	    break;
	  case N_EMPTY:
	    x_empty ();
	    break;
	  case N_FORMAT_NAME:
	    x_format_name ();
	    break;
	  case N_IF:

/***************************************************************************
 * WEB section number:	421
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function if$ pops the top three literals (they
 * are two function literals and an integer literal, in that order); if
 * the integer is greater than 0, it executes the second literal, else it
 * executes the first.  If any of the types is incorrect, it complains
 * but does nothing else.
 ***************************************************************************/
	    BEGIN
	      pop_lit_stk (&pop_lit1, &pop_typ1);
	      pop_lit_stk (&pop_lit2, &pop_typ2);
	      pop_lit_stk (&pop_lit3, &pop_typ3);
	      if (pop_typ1 != STK_FN)
	      BEGIN
		print_wrong_stk_lit (pop_lit1, pop_typ1, STK_FN);
	      END
	      else if (pop_typ2 != STK_FN)
	      BEGIN
		print_wrong_stk_lit (pop_lit2, pop_typ2, STK_FN);
	      END
	      else if (pop_typ3 != STK_INT)
	      BEGIN
		print_wrong_stk_lit (pop_lit3, pop_typ3, STK_INT);
	      END
	      else if (pop_lit3 > 0)
	      BEGIN
		execute_fn (pop_lit2);
	      END
	      else
	      BEGIN
		execute_fn (pop_lit1);
	      END
	    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 421 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    break;
	  case N_INT_TO_CHR:
	    x_int_to_chr ();
	    break;
	  case N_INT_TO_STR:
	    x_int_to_str ();
	    break;
	  case N_MISSING:
	    x_missing ();
	    break;
	  case N_NEWLINE:

/***************************************************************************
 * WEB section number:	425
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function newline$ writes whatever has
 * accumulated in the output buffer |out_buf| onto the .bbl file.
 ***************************************************************************/
	    BEGIN
	      output_bbl_line ();
	    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 425 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    break;
	  case N_NUM_NAMES:
	    x_num_names ();
	    break;
	  case N_POP:

/***************************************************************************
 * WEB section number:	428
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function pop$ pops the top of the stack but
 * doesn't print it.
 ***************************************************************************/
	    BEGIN
	      pop_lit_stk (&pop_lit1, &pop_typ1);
	    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 428 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    break;
	  case N_PREAMBLE:
	    x_preamble ();
	    break;
	  case N_PURIFY:
	    x_purify ();
	    break;
	  case N_QUOTE:
	    x_quote ();
	    break;
	  case N_SKIP:

/***************************************************************************
 * WEB section number:	435
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function skip$ is a no-op.
 ***************************************************************************/
	    BEGIN
	      DO_NOTHING;
	    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 435 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    break;
	  case N_STACK:

/***************************************************************************
 * WEB section number:	436
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function stack$ pops and prints the whole stack; it's
 * meant to be used for style designers while debugging.
 ***************************************************************************/
	    BEGIN
	      pop_whole_stack ();
	    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 436 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    break;
	  case N_SUBSTRING:
	    x_substring ();
	    break;
	  case N_SWAP:
	    x_swap ();
	    break;
	  case N_TEXT_LENGTH:
	    x_text_length ();
	    break;
	  case N_TEXT_PREFIX:
	    x_text_prefix ();
	    break;
	  case N_TOP_STACK:

/***************************************************************************
 * WEB section number:	446
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function top$ pops and prints the top of the
 * stack.
 ***************************************************************************/
	    BEGIN
	      pop_top_and_print ();
	    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 446 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    break;
	  case N_TYPE:
	    x_type ();
	    break;
	  case N_WARNING:
	    x_warning ();
	    break;
	  case N_WHILE:

/***************************************************************************
 * WEB section number:	449
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function while$ pops the top two (function)
 * literals, and keeps executing the second as long as the (integer)
 * value left on the stack by executing the first is greater than 0.  If
 * either type is incorrect, it complains but does nothing else.
 ***************************************************************************/
	    BEGIN
	      pop_lit_stk (&r_pop_lt1, &r_pop_tp1);
	      pop_lit_stk (&r_pop_lt2, &r_pop_tp2);
	      if (r_pop_tp1 != STK_FN)
	      BEGIN
		print_wrong_stk_lit (r_pop_lt1, r_pop_tp1, STK_FN);
	      END
	      else if (r_pop_tp2 != STK_FN)
	      BEGIN
		print_wrong_stk_lit (r_pop_lt2, r_pop_tp2, STK_FN);
	      END
	      else
	      BEGIN
		LOOP
		BEGIN
		  execute_fn (r_pop_lt2);
		  pop_lit_stk (&pop_lit1, &pop_typ1);
		  if (pop_typ1 != STK_INT)
		  BEGIN
		    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_INT);
		    goto End_While_Label;
		  END
		  else if (pop_lit1 > 0)
		  BEGIN
		    execute_fn (r_pop_lt1);
		  END
		  else
		  BEGIN
		    goto End_While_Label;
		  END
		END
	      END
End_While_Label: DO_NOTHING;
	    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 449 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    break;
	  case N_WIDTH:
	    x_width ();
	    break;
	  case N_WRITE:
	    x_write ();
	    break;
	  default:
	    CONFUSION ("Unknown built-in function");
	    break;
	END
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 341 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      break;
    case WIZ_DEFINED:

/***************************************************************************
 * WEB section number:	326
 * ~~~~~~~~~~~~~~~~~~~
 * To execute a |wiz_defined| function, we just execute all those
 * functions in its definition, except that the special marker
 * |quote_next_fn| means we push the next function onto the stack.
 ***************************************************************************/
      BEGIN
	wiz_ptr = FN_INFO[ex_fn_loc];
	while (wiz_functions[wiz_ptr] != END_OF_DEF)
	BEGIN
	  if (wiz_functions[wiz_ptr] != QUOTE_NEXT_FN)
	  BEGIN
	    execute_fn (wiz_functions[wiz_ptr]);
	  END
	  else
	  BEGIN
	    INCR (wiz_ptr);
	    push_lit_stk (wiz_functions[wiz_ptr], STK_FN);
	  END
	  INCR (wiz_ptr);
	END
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 326 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      break;
    case INT_LITERAL:
      push_lit_stk (FN_INFO[ex_fn_loc], STK_INT);
      break;
    case STR_LITERAL:
      push_lit_stk (hash_text[ex_fn_loc], STK_STR);
      break;
    case FIELD:

/***************************************************************************
 * WEB section number:	327
 * ~~~~~~~~~~~~~~~~~~~
 * This module pushes the string given by the field onto the literal
 * stack unless it's |missing|, in which case it pushes a special value
 * onto the stack.
 ***************************************************************************/
      BEGIN
	if ( ! mess_with_entries)
	BEGIN
	  bst_cant_mess_with_entries_prin ();
	END
	else
	BEGIN
	  field_ptr = (cite_ptr * num_fields) + FN_INFO[ex_fn_loc];
	  if (field_info[field_ptr] == MISSING)
	  BEGIN
	    push_lit_stk (hash_text[ex_fn_loc], STK_FIELD_MISSING);
	  END
	  else
	  BEGIN
	    push_lit_stk (field_info[field_ptr], STK_STR);
	  END
	END
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 327 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      break;
    case INT_ENTRY_VAR:

/***************************************************************************
 * WEB section number:	328
 * ~~~~~~~~~~~~~~~~~~~
 * This module pushes the integer given by an |int_entry_var| onto the
 * literal stack.
 ***************************************************************************/
      BEGIN
	if ( ! mess_with_entries)
	BEGIN
	  bst_cant_mess_with_entries_prin ();
	END
	else
	BEGIN
	  push_lit_stk (entry_ints[(cite_ptr * num_ent_ints)
				   + FN_INFO[ex_fn_loc]], STK_INT);
	END
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 328 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      break;
    case STR_ENTRY_VAR:

/***************************************************************************
 * WEB section number:	329
 * ~~~~~~~~~~~~~~~~~~~
 * This module adds the string given by a |str_entry_var| to |str_pool|
 * via the execution buffer and pushes it onto the literal stack.
 ***************************************************************************/
      BEGIN
	if ( ! mess_with_entries)
	BEGIN
	  bst_cant_mess_with_entries_prin  ();
	END
	else
	BEGIN
	  str_ent_ptr = (cite_ptr * num_ent_strs) + FN_INFO[ex_fn_loc];
	  ex_buf_ptr = 0;
	  while (ENTRY_STRS(str_ent_ptr, ex_buf_ptr) != END_OF_STRING)
	  BEGIN
	    APPEND_EX_BUF_CHAR (ENTRY_STRS(str_ent_ptr, ex_buf_ptr));
	  END
	  ex_buf_length = ex_buf_ptr;
	  add_pool_buf_and_push ();
	END
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 329 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      break;
    case INT_GLOBAL_VAR:
      push_lit_stk (FN_INFO[ex_fn_loc], STK_INT);
      break;
    case STR_GLOBAL_VAR:

/***************************************************************************
 * WEB section number:	330
 * ~~~~~~~~~~~~~~~~~~~
 * This module pushes the string given by a |str_global_var| onto the
 * literal stack, but it copies the string to |str_pool| (character by
 * character) only if it has to---it BEGIN\it doesn't\/END have to if the
 * string is static (that is, if the string isn't at the top, temporary
 * part of the string pool).
 ***************************************************************************/
      BEGIN
	str_glb_ptr = FN_INFO[ex_fn_loc];
	if (glb_str_ptr[str_glb_ptr] > 0)
	BEGIN
	  push_lit_stk (glb_str_ptr[str_glb_ptr], STK_STR);
	END
	else
	BEGIN
	  STR_ROOM (glb_str_end[str_glb_ptr]);
	  glob_chr_ptr = 0;
	  while (glob_chr_ptr < glb_str_end[str_glb_ptr])
	  BEGIN
	    APPEND_CHAR (GLOBAL_STRS(str_glb_ptr, glob_chr_ptr));
	    INCR (glob_chr_ptr);
	  END
	  push_lit_stk (make_string (), STK_STR);
	END
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 330 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      break;
    default:
      unknwn_function_class_confusion ();
      break;
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 325 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 420
 * ~~~~~~~~~~~~~~~~~~~
 * This is a procedure so that |x_format_name| is smaller.
 ***************************************************************************/
void          figure_out_the_formatted_name (void)
BEGIN

/***************************************************************************
 * WEB section number:	402
 * ~~~~~~~~~~~~~~~~~~~
 * This module uses the information in |pop_lit1| to format the name.
 * Everything at |sp_brace_level = 0| is copied verbatim to the formatted
 * string; the rest is described in the succeeding modules.
 ***************************************************************************/
  BEGIN
    ex_buf_ptr = 0;
    sp_brace_level = 0;
    sp_ptr = str_start[pop_lit1];
    sp_end = str_start[pop_lit1 + 1];
    while (sp_ptr < sp_end)
    BEGIN
      if (str_pool[sp_ptr] == LEFT_BRACE)
      BEGIN
	INCR (sp_brace_level);
	INCR (sp_ptr);

/***************************************************************************
 * WEB section number:	403
 * ~~~~~~~~~~~~~~~~~~~
 * When we arrive here we're at |sp_brace_level = 1|, just past the
 * |left_brace|.  Letters at this |sp_brace_level| other than those
 * denoting the parts of the name (i.e., the first letters of `first,'
 * `last,' `von,' and `jr,' ignoring case) are illegal.  We do two passes
 * over this group; the first determines whether we're to output
 * anything, and, if we are, the second actually outputs it.
 ***************************************************************************/
	BEGIN
	  sp_xptr1 = sp_ptr;
	  alpha_found = FALSE;
	  double_letter = FALSE;
	  end_of_group = FALSE;
	  to_be_written = TRUE;
	  while (( ! end_of_group) && (sp_ptr < sp_end))
	  BEGIN
	    if (lex_class[str_pool[sp_ptr]] == ALPHA)
	    BEGIN
	      INCR (sp_ptr);

/***************************************************************************
 * WEB section number:	405
 * ~~~~~~~~~~~~~~~~~~~
 * We won't output anything for this part of the name if this is a second
 * occurrence of an |sp_brace_level = 1| letter, if it's an illegal
 * letter, or if there are no tokens corresponding to this part.  We also
 * determine if we're we to output complete tokens (indicated by a double
 * letter).
 ***************************************************************************/
	      BEGIN
		if (alpha_found)
		BEGIN
		  brace_lvl_one_letters_complaint ();
		  to_be_written = FALSE;
		END
		else
		BEGIN
		  switch (str_pool[sp_ptr - 1])
		  BEGIN
		    case 'f':
		    case 'F':

/***************************************************************************
 * WEB section number:	407
 * ~~~~~~~~~~~~~~~~~~~
 * Here we set pointers into |name_tok| and note whether we'll be dealing
 * with a full first-name tokens (|double_letter = true|) or
 * abbreviations (|double_letter = false|).
 ***************************************************************************/
		      BEGIN
			cur_token = first_start;
			last_token = first_end;
			if (cur_token == last_token)
			BEGIN
			  to_be_written = FALSE;
			END
			if ((str_pool[sp_ptr] == 'f')
			      || (str_pool[sp_ptr] == 'F'))
			BEGIN
			  double_letter = TRUE;
			END
		      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 407 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

		      break;
		    case 'v':
		    case 'V':

/***************************************************************************
 * WEB section number:	408
 * ~~~~~~~~~~~~~~~~~~~
 * The same as above but for von-name tokens.
 ***************************************************************************/
		      BEGIN
			cur_token = von_start;
			last_token = von_end;
			if (cur_token == last_token)
			BEGIN
			  to_be_written = FALSE;
			END
			if ((str_pool[sp_ptr] == 'v')
			      || (str_pool[sp_ptr] == 'V'))
			BEGIN
			  double_letter = TRUE;
			END
		      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 408 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

		      break;
		    case 'l':
		    case 'L':

/***************************************************************************
 * WEB section number:	409
 * ~~~~~~~~~~~~~~~~~~~
 * The same as above but for last-name tokens.
 ***************************************************************************/
		      BEGIN
			cur_token = von_end;
			last_token = last_end;
			if (cur_token == last_token)
			BEGIN
			  to_be_written = FALSE;
			END
			if ((str_pool[sp_ptr] == 'l')
			      || (str_pool[sp_ptr] == 'L'))
			BEGIN
			  double_letter = TRUE;
			END
		      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 409 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

		      break;
		    case 'j':
		    case 'J':

/***************************************************************************
 * WEB section number:	410
 * ~~~~~~~~~~~~~~~~~~~
 * The same as above but for jr-name tokens.
 ***************************************************************************/
		      BEGIN
			cur_token = last_end;
			last_token = jr_end;
			if (cur_token == last_token)
			BEGIN
			  to_be_written = FALSE;
			END
			if ((str_pool[sp_ptr] == 'j')
			      || (str_pool[sp_ptr] == 'J'))
			BEGIN
			  double_letter = TRUE;
			END
		      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 410 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

		      break;
		    default:
		      brace_lvl_one_letters_complaint ();
		      to_be_written = FALSE;
		      break;
		  END
		  if (double_letter)
		  BEGIN
		    INCR (sp_ptr);
		  END
		END
		alpha_found = TRUE;
	      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 405 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    END
	    else if (str_pool[sp_ptr] == RIGHT_BRACE)
	    BEGIN
	      DECR (sp_brace_level);
	      INCR (sp_ptr);
	      end_of_group = TRUE;
	    END
	    else if (str_pool[sp_ptr] == LEFT_BRACE)
	    BEGIN
	      INCR (sp_brace_level);
	      INCR (sp_ptr);
	      skip_stuff_at_sp_brace_level_gr ();
	    END
	    else
	    BEGIN
	      INCR (sp_ptr);
	    END
	  END
	  if ((end_of_group) && (to_be_written))
	  BEGIN

/***************************************************************************
 * WEB section number:	411
 * ~~~~~~~~~~~~~~~~~~~
 * This is the second pass over this part of the name; here we actually
 * write stuff out to |ex_buf|.
 ***************************************************************************/
	    ex_buf_xptr = ex_buf_ptr;
	    sp_ptr = sp_xptr1;
	    sp_brace_level = 1;
	    while (sp_brace_level > 0)
	    BEGIN
	      if ((lex_class[str_pool[sp_ptr]] == ALPHA)
		    && (sp_brace_level == 1))
	      BEGIN
		INCR (sp_ptr);

/***************************************************************************
 * WEB section number:	412
 * ~~~~~~~~~~~~~~~~~~~
 * When we come here, |sp_ptr| is just past the letter indicating the
 * part of the name for which we're about to output tokens.  When we
 * leave, it's at the first character of the rest of the group.
 ***************************************************************************/
		BEGIN
		  if (double_letter)
		  BEGIN
		    INCR (sp_ptr);
		  END
		  use_default = TRUE;
		  sp_xptr2 = sp_ptr;
		  if (str_pool[sp_ptr] == LEFT_BRACE)
		  BEGIN
		    use_default = FALSE;
		    INCR (sp_brace_level);
		    INCR (sp_ptr);
		    sp_xptr1 = sp_ptr;
		    skip_stuff_at_sp_brace_level_gr ();
		    sp_xptr2 = sp_ptr - 1;
		  END

/***************************************************************************
 * WEB section number:	413
 * ~~~~~~~~~~~~~~~~~~~
 * Here, for each token in this part, we output either a full or an
 * abbreviated token and the inter-token string for all but the last
 * token of this part.
 ***************************************************************************/
		  BEGIN
		    while (cur_token < last_token)
		    BEGIN
		      if (double_letter)

/***************************************************************************
 * WEB section number:	414
 * ~~~~~~~~~~~~~~~~~~~
 * Here we output all the characters in the token, verbatim.
 ***************************************************************************/
		      BEGIN
			name_bf_ptr = name_tok[cur_token];
			name_bf_xptr = name_tok[cur_token + 1];
			if (ex_buf_length + (name_bf_xptr - name_bf_ptr)
			      > Buf_Size)
			BEGIN
			  buffer_overflow ();
			END
			while (name_bf_ptr < name_bf_xptr)
			BEGIN
			  APPEND_EX_BUF_CHAR (NAME_BUF[name_bf_ptr]);
			  INCR (name_bf_ptr);
			END
		      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 414 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

		      else

/***************************************************************************
 * WEB section number:	415
 * ~~~~~~~~~~~~~~~~~~~
 * Here we output the first alphabetic or special character of the token;
 * brace level is irrelevant for an alphabetic (but not a special)
 * character.
 ***************************************************************************/
		      BEGIN
			name_bf_ptr = name_tok[cur_token];
			name_bf_xptr = name_tok[cur_token + 1];
			while (name_bf_ptr < name_bf_xptr)
			BEGIN
#ifdef UTF_8
/*
For output the first character which is encodage UTF-8, we sould discuter different length of character. 23/sep/2009
*/
			  if((lex_class[NAME_BUF[name_bf_ptr]] != WHITE_SPACE) && (NAME_BUF[name_bf_ptr] != LEFT_BRACE))
			  BEGIN
			    DO_UTF8(NAME_BUF[name_bf_ptr],
			      APPEND_EX_BUF_CHAR_AND_CHECK (NAME_BUF[name_bf_ptr]),
			      APPEND_EX_BUF_CHAR_AND_CHECK (NAME_BUF[name_bf_ptr]);
			      APPEND_EX_BUF_CHAR_AND_CHECK (NAME_BUF[name_bf_ptr+1]),
			      APPEND_EX_BUF_CHAR_AND_CHECK (NAME_BUF[name_bf_ptr]);
			      APPEND_EX_BUF_CHAR_AND_CHECK (NAME_BUF[name_bf_ptr+1]);
			      APPEND_EX_BUF_CHAR_AND_CHECK (NAME_BUF[name_bf_ptr+2]),
			      APPEND_EX_BUF_CHAR_AND_CHECK (NAME_BUF[name_bf_ptr]);
			      APPEND_EX_BUF_CHAR_AND_CHECK (NAME_BUF[name_bf_ptr+1]);
			      APPEND_EX_BUF_CHAR_AND_CHECK (NAME_BUF[name_bf_ptr+2]);
			      APPEND_EX_BUF_CHAR_AND_CHECK (NAME_BUF[name_bf_ptr+3]));
#else
			  if (lex_class[NAME_BUF[name_bf_ptr]] == ALPHA)
			  BEGIN
			    APPEND_EX_BUF_CHAR_AND_CHECK (
						      NAME_BUF[name_bf_ptr]);
#endif
			    goto Loop_Exit_Label;
			  END
			  else if ((NAME_BUF[name_bf_ptr] == LEFT_BRACE)
				    && ((name_bf_ptr + 1) < name_bf_xptr))
			  BEGIN
			    if (NAME_BUF[name_bf_ptr + 1] == BACKSLASH)

/***************************************************************************
 * WEB section number:	416
 * ~~~~~~~~~~~~~~~~~~~
 * We output a special character here even if the user has been silly
 * enough to make it nonalphabetic (and even if the user has been sillier
 * still by not having a matching |right_brace|).
 ***************************************************************************/
			    BEGIN
			      if ((ex_buf_ptr + 2) > Buf_Size)
			      BEGIN
				buffer_overflow ();
			      END
			      APPEND_EX_BUF_CHAR (LEFT_BRACE);
			      APPEND_EX_BUF_CHAR (BACKSLASH);
			      name_bf_ptr = name_bf_ptr + 2;
			      nm_brace_level = 1;
			      while ((name_bf_ptr < name_bf_xptr)
				      && (nm_brace_level > 0))
			      BEGIN
				if (NAME_BUF[name_bf_ptr] == RIGHT_BRACE)
				BEGIN
				  DECR (nm_brace_level);
				END
				else if (NAME_BUF[name_bf_ptr] == LEFT_BRACE)
				BEGIN
				  INCR (nm_brace_level);
				END
				APPEND_EX_BUF_CHAR_AND_CHECK (
						      NAME_BUF[name_bf_ptr]);
				INCR (name_bf_ptr);
			      END
			      goto Loop_Exit_Label;
			    END
			  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 416 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

			  INCR (name_bf_ptr);
			END
Loop_Exit_Label:  DO_NOTHING;
		      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 415 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

		      INCR (cur_token);
		      if (cur_token < last_token)

/***************************************************************************
 * WEB section number:	417
 * ~~~~~~~~~~~~~~~~~~~
 * Here we output either the .bst given string if it exists, or else
 * the .bib |sep_char| if it exists, or else the default string.  A
 * |tie| is the default space character between the last two tokens of
 * the name part, and between the first two tokens if the first token is
 * short enough; otherwise, a |space| is the default.
 ***************************************************************************/
		      BEGIN
			if (use_default)
			BEGIN
			  if ( ! double_letter)
			  BEGIN
			    APPEND_EX_BUF_CHAR_AND_CHECK (PERIOD);
			  END
			  if (lex_class[name_sep_char[cur_token]]
				== SEP_CHAR)
			  BEGIN
			    APPEND_EX_BUF_CHAR_AND_CHECK (
						  name_sep_char[cur_token]);
			  END
			  else if ((cur_token == (last_token - 1))
				    || ( ! enough_text_chars (LONG_TOKEN)))
			  BEGIN
			    APPEND_EX_BUF_CHAR_AND_CHECK (TIE);
			  END
			  else
			  BEGIN
			    APPEND_EX_BUF_CHAR_AND_CHECK (SPACE);
			  END
			END
			else
			BEGIN
			  if ((ex_buf_length + (sp_xptr2 - sp_xptr1))
				  > Buf_Size)
			  BEGIN
			    buffer_overflow ();
			  END
			  sp_ptr = sp_xptr1;
			  while (sp_ptr < sp_xptr2)
			  BEGIN
			    APPEND_EX_BUF_CHAR (str_pool[sp_ptr]);
			    INCR (sp_ptr);
			  END
			END
		      END
		    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 417 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

		  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 413 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

		  if ( ! use_default)
		  BEGIN
		    sp_ptr = sp_xptr2 + 1;
		  END
		END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 412 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	      END
	      else if (str_pool[sp_ptr] == RIGHT_BRACE)
	      BEGIN
		DECR (sp_brace_level);
		INCR (sp_ptr);
		if (sp_brace_level > 0)
		BEGIN
		  APPEND_EX_BUF_CHAR_AND_CHECK (RIGHT_BRACE);
		END
	      END
	      else if (str_pool[sp_ptr] == LEFT_BRACE)
	      BEGIN
		INCR (sp_brace_level);
		INCR (sp_ptr);
		APPEND_EX_BUF_CHAR_AND_CHECK (LEFT_BRACE);
	      END
	      else
	      BEGIN
		APPEND_EX_BUF_CHAR_AND_CHECK (str_pool[sp_ptr]);
		INCR (sp_ptr);
	      END
	    END
	    if (ex_buf_ptr > 0)
	    BEGIN
	      if (ex_buf[ex_buf_ptr - 1] == TIE)

/***************************************************************************
 * WEB section number:	419
 * ~~~~~~~~~~~~~~~~~~~
 * If the last character output for this name part is a |tie| but the
 * previous character it isn't, we're dealing with a discretionary |tie|;
 * thus we replace it by a |space| if there are enough characters in the
 * rest of the name part.
 ***************************************************************************/
	      BEGIN
		DECR (ex_buf_ptr);
		if (ex_buf[ex_buf_ptr - 1] == TIE)
		BEGIN
		  DO_NOTHING;
		END
		else if ( ! enough_text_chars (LONG_NAME))
		BEGIN
		  INCR (ex_buf_ptr);
		END
		else
		BEGIN
		  APPEND_EX_BUF_CHAR (SPACE);
		END
	      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 419 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 411 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	  END
	END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 403 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      END
      else if (str_pool[sp_ptr] == RIGHT_BRACE)
      BEGIN
	braces_unbalanced_complaint (pop_lit1);
	INCR (sp_ptr);
      END
      else
      BEGIN
	APPEND_EX_BUF_CHAR_AND_CHECK (str_pool[sp_ptr]);
	INCR (sp_ptr);
      END
    END
    if (sp_brace_level > 0)
    BEGIN
      braces_unbalanced_complaint (pop_lit1);
    END
    ex_buf_length = ex_buf_ptr;
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 402 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 420 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 59
 * ~~~~~~~~~~~~~~~~~~~
 * Yet another complaint-before-quiting.
 *
 * REMOVED: |file_nm_size_overflow|.
 ***************************************************************************/

/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  59 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 278
 * ~~~~~~~~~~~~~~~~~~~
 * Occasionally we need to figure out the hash-table location of a given
 * cite-key string and its lower-case equivalent.  This function does
 * that.  To perform the task it needs to borrow a buffer, a need that
 * gives rise to the alias kludge---it helps make the stack space not
 * overflow on some machines (and while it's at it, it'll borrow a
 * pointer, too).  Finally, the function returns |true| if the cite key
 * exists on |cite_list|, and its sets |cite_hash_found| according to
 * whether or not it found the actual version (before |lower_case|ing) of
 * the cite key; however, its raison d'etre
 * (literally, ``to eat a raisin'') is to compute |cite_loc| and
 * |lc_cite_loc|.
 ***************************************************************************/
Boolean_T         find_cite_locs_for_this_cite_ke (StrNumber_T cite_str)
BEGIN
  Boolean_T	  find_cite_locs_for_this_cite_ke;

  EX_BUF5_PTR = 0;
  tmp_ptr = str_start[cite_str];
  tmp_end_ptr = str_start[cite_str + 1];
  while (tmp_ptr < tmp_end_ptr)
  BEGIN
    EX_BUF5[EX_BUF5_PTR] = str_pool[tmp_ptr];
    INCR (EX_BUF5_PTR);
    INCR (tmp_ptr);
  END
  cite_loc = str_lookup (ex_buf, 0, LENGTH (cite_str), CITE_ILK, DONT_INSERT);
  cite_hash_found = hash_found;
  lower_case (EX_BUF5, 0, LENGTH (cite_str));
  lc_cite_loc = str_lookup (EX_BUF5, 0, LENGTH (cite_str), LC_CITE_ILK,
			    DONT_INSERT);
  if (hash_found)
  BEGIN
    find_cite_locs_for_this_cite_ke = TRUE;
  END
  else
  BEGIN
    find_cite_locs_for_this_cite_ke = FALSE;
  END
  return (find_cite_locs_for_this_cite_ke);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 278 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 116
 * ~~~~~~~~~~~~~~~~~~~
 * We're not at the end of an .aux file, so we see if the current line
 * might be a command of interest.  A command of interest will be a line
 * without blanks, consisting of a command name, a |left_brace|, one or
 * more arguments separated by commas, and a |right_brace|.
 ***************************************************************************/
void          get_aux_command_and_process (void)
BEGIN
  buf_ptr2 = 0;
  if ( ! scan1 (LEFT_BRACE))
  BEGIN
    goto Exit_Label;
  END
  command_num = ilk_info[str_lookup (buffer, buf_ptr1, TOKEN_LEN,
				     AUX_COMMAND_ILK, DONT_INSERT)];
  if (hash_found)
  BEGIN
    switch (command_num)
    BEGIN
      case N_AUX_BIBDATA:
        aux_bib_data_command ();
        break;
      case N_AUX_BIBSTYLE:
        aux_bib_style_command ();
        break;
      case N_AUX_CITATION:
        aux_citation_command ();
        break;
      case N_AUX_INPUT:
        aux_input_command ();
        break;
      default:
        CONFUSION ("Unknown auxiliary-file command");
        break;
    END
  END
Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 116 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 236
 * ~~~~~~~~~~~~~~~~~~~
 * This module either reads a database entry, whose three main components
 * are an entry type, a database key, and a list of fields, or it reads a
 * .bib command, whose structure is command dependent and explained
 * later.
 ***************************************************************************/
void          get_bib_command_or_entry_and_pr (void)
BEGIN
  at_bib_command = FALSE;

/***************************************************************************
 * WEB section number:	237
 * ~~~~~~~~~~~~~~~~~~~
 * This module skips over everything until hitting an |at_sign| or the
 * end of the file.  It also updates |bib_line_num|, the line counter.
 ***************************************************************************/
  while ( ! scan1 (AT_SIGN))
  BEGIN
    if ( ! input_ln (CUR_BIB_FILE))
    BEGIN
      goto Exit_Label;
    END
    INCR (bib_line_num);
    buf_ptr2 = 0;
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 237 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	238
 * ~~~~~~~~~~~~~~~~~~~
 * This module reads an |at_sign| and an entry type (like `book' or
 * `article') or a .bib command.  If it's an entry type, it must be
 * defined in the .bst file if this entry is to be included in the
 * reference list.
 ***************************************************************************/
  BEGIN
    if (SCAN_CHAR != AT_SIGN)
    BEGIN
      CONFUSION2 ("An \"%c\" disappeared", xchr[AT_SIGN]);
    END
    INCR (buf_ptr2);
    EAT_BIB_WHITE_AND_EOF_CHECK;
    scan_identifier (LEFT_BRACE, LEFT_PAREN, LEFT_PAREN);
    BIB_IDENTIFIER_SCAN_CHECK ("an entry type");

#ifdef TRACE
    if (Flag_trace) {
      TRACE_PR_TOKEN;
      TRACE_PR_LN (" is an entry type or a database-file command");
    }
#endif                      			/* TRACE */

    lower_case (buffer, buf_ptr1, TOKEN_LEN);
    command_num = ilk_info[str_lookup (buffer, buf_ptr1, TOKEN_LEN,
					BIB_COMMAND_ILK, DONT_INSERT)];
    if (hash_found)

/***************************************************************************
 * WEB section number:	239
 * ~~~~~~~~~~~~~~~~~~~
 * Here we determine which .bib command we're about to process, then
 * go to it.
 ***************************************************************************/
    BEGIN
      at_bib_command = TRUE;
      switch (command_num)
      BEGIN
	case N_BIB_COMMENT:

/***************************************************************************
 * WEB section number:	241
 * ~~~~~~~~~~~~~~~~~~~
 * The comment command is implemented for SCRIBE compatibility.  It's
 * not really needed because \BibTeX\ treats (flushes) everything not
 * within an entry as a comment anyway.
 ***************************************************************************/
	  BEGIN
	    goto Exit_Label;
	  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 239 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	case N_BIB_PREAMBLE:

/***************************************************************************
 * WEB section number:	242
 * ~~~~~~~~~~~~~~~~~~~
 * The preamble command lets a user have \TeX\ stuff inserted (by the
 * standard styles, at least) directly into the .bbl file.  It is
 * intended primarily for allowing \TeX\ macro definitions used within
 * the bibliography entries (for better sorting, for example).  One
 * preamble command per .bib file should suffice.
 *
 * A preamble command has either braces or parentheses as outer
 * delimiters.  Inside is the preamble string, which has the same syntax
 * as a field value: a nonempty list of field tokens separated by
 * |concat_char|s.  There are three types of field tokens---nonnegative
 * numbers, macro names, and delimited strings.
 *
 * This module does all the scanning (that's not subcontracted), but the
 * .bib-specific scanning function
 * |scan_and_store_the_field_value_and_eat_white| actually stores the
 * value.
 ***************************************************************************/
	  BEGIN
	    if (preamble_ptr == Max_Bib_Files)
	    BEGIN
              BIB_XRETALLOC_NOSET ("bib_file", bib_file, AlphaFile_T,
                                   Max_Bib_Files, Max_Bib_Files + MAX_BIB_FILES);
              BIB_XRETALLOC_NOSET ("bib_list", bib_list, StrNumber_T,
                                   Max_Bib_Files, Max_Bib_Files + MAX_BIB_FILES);
              BIB_XRETALLOC ("s_preamble", s_preamble, StrNumber_T,
                             Max_Bib_Files, Max_Bib_Files + MAX_BIB_FILES);
	    END
	    EAT_BIB_WHITE_AND_EOF_CHECK;
	    if (SCAN_CHAR == LEFT_BRACE)
	    BEGIN
	      right_outer_delim = RIGHT_BRACE;
	    END
	    else if (SCAN_CHAR == LEFT_PAREN)
	    BEGIN
	      right_outer_delim = RIGHT_PAREN;
	    END
	    else
	    BEGIN
	      BIB_ONE_OF_TWO_EXPECTED_ERR (LEFT_BRACE, LEFT_PAREN);
	    END
	    INCR (buf_ptr2);
	    EAT_BIB_WHITE_AND_EOF_CHECK;
	    store_field = TRUE;
	    if ( ! scan_and_store_the_field_value ())
	    BEGIN
	      goto Exit_Label;
	    END
	    if (SCAN_CHAR != right_outer_delim)
	    BEGIN
	      BIB_ERR2 ("Missing \"%c\" in preamble command",
			xchr[right_outer_delim]);
	    END
	    INCR (buf_ptr2);
	    goto Exit_Label;
	  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 242 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
	  break;
	case N_BIB_STRING:

/***************************************************************************
 * WEB section number:	243
 * ~~~~~~~~~~~~~~~~~~~
 * The string command is implemented both for SCRIBE compatibility
 * and for allowing a user: to override a .bst-file macro
 * command, to define one that the .bst file doesn't, or to engage in
 * good, wholesome, typing laziness.
 *
 * The string command does mostly the same thing as the
 * .bst-file's macro command (but the syntax is different and the
 * string command compresses |white_space|).  In fact, later in this
 * program, the term ``macro'' refers to either a .bst ``macro'' or a
 * .bib ``string'' (when it's clear from the context that it's not
 * a WEB macro).
 *
 * A string command has either braces or parentheses as outer
 * delimiters.  Inside is the string's name (it must be a legal
 * identifier, and case differences are ignored---all upper-case letters
 * are converted to lower case), then an |equals_sign|, and the string's
 * definition, which has the same syntax as a field value: a nonempty
 * list of field tokens separated by |concat_char|s.  There are three
 * types of field tokens---nonnegative numbers, macro names, and
 * delimited strings.
 ***************************************************************************/
	  BEGIN
	    EAT_BIB_WHITE_AND_EOF_CHECK;

/***************************************************************************
 * WEB section number:	244
 * ~~~~~~~~~~~~~~~~~~~
 * This module reads a left outer-delimiter and a string name.
 ***************************************************************************/
	    BEGIN
	      if (SCAN_CHAR == LEFT_BRACE)
	      BEGIN
		right_outer_delim = RIGHT_BRACE;
	      END
	      else if (SCAN_CHAR == LEFT_PAREN)
	      BEGIN
		right_outer_delim = RIGHT_PAREN;
	      END
	      else
	      BEGIN
		BIB_ONE_OF_TWO_EXPECTED_ERR (LEFT_BRACE, LEFT_PAREN);
	      END
	      INCR (buf_ptr2);
	      EAT_BIB_WHITE_AND_EOF_CHECK;
	      scan_identifier (EQUALS_SIGN, EQUALS_SIGN, EQUALS_SIGN);
	      BIB_IDENTIFIER_SCAN_CHECK ("a string name");

/***************************************************************************
 * WEB section number:	245
 * ~~~~~~~~~~~~~~~~~~~
 * This module marks this string as |macro_ilk|; the commented-out code
 * will give a warning message when overwriting a previously defined
 * macro.
 ***************************************************************************/
	      BEGIN
#ifdef TRACE
                if (Flag_trace) {
		  TRACE_PR_TOKEN;
		  TRACE_PR_LN (" is a database-defined macro");
                }
#endif                      			/* TRACE */

		lower_case (buffer, buf_ptr1, TOKEN_LEN);
		cur_macro_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN,
					    MACRO_ILK, DO_INSERT);
		ilk_info[cur_macro_loc] = hash_text[cur_macro_loc];
	/*-------------------------------------------------------------------
	 *	if (hash_found)
	 *	BEGIN
	 *	  MACRO_NAME_WARNING ("having its definition overwritten");
	 *	END
	 *------------------------------------------------------------------*/
	      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 245 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 244 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    EAT_BIB_WHITE_AND_EOF_CHECK;

/***************************************************************************
 * WEB section number:	246
 * ~~~~~~~~~~~~~~~~~~~
 * This module skips over the |equals_sign|, reads and stores the list of
 * field tokens that defines this macro (compressing |white_space|), and
 * reads a |right_outer_delim|.
 ***************************************************************************/
	    BEGIN
	      if (SCAN_CHAR != EQUALS_SIGN)
	      BEGIN
		BIB_EQUALS_SIGN_EXPECTED_ERR;
	      END
	      INCR (buf_ptr2);
	      EAT_BIB_WHITE_AND_EOF_CHECK;
	      store_field = TRUE;
	      if ( ! scan_and_store_the_field_value ())
	      BEGIN
		goto Exit_Label;
	      END
	      if (SCAN_CHAR != right_outer_delim)
	      BEGIN
		BIB_ERR2 ("Missing \"%c\" in string command",
			   xchr[right_outer_delim]);
	      END
	      INCR (buf_ptr2);
	    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 246 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    goto Exit_Label;
	  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 243 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	  break;
	default:
	  bib_cmd_confusion ();
	  break;
      END
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 239 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    else
    BEGIN
      entry_type_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN, BST_FN_ILK,
				   DONT_INSERT);
      if (( ! hash_found) || (fn_type[entry_type_loc] != WIZ_DEFINED))
      BEGIN
	type_exists = FALSE;
      END
      else
      BEGIN
	type_exists = TRUE;
      END
    END
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 238 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  EAT_BIB_WHITE_AND_EOF_CHECK;

/***************************************************************************
 * WEB section number:	266
 * ~~~~~~~~~~~~~~~~~~~
 * And now, back to processing an entry (rather than a command).  This
 * module reads a left outer-delimiter and a database key.
 ***************************************************************************/
  BEGIN
    if (SCAN_CHAR == LEFT_BRACE)
    BEGIN
      right_outer_delim = RIGHT_BRACE;
    END
    else if (SCAN_CHAR == LEFT_PAREN)
    BEGIN
      right_outer_delim = RIGHT_PAREN;
    END
    else
    BEGIN
      BIB_ONE_OF_TWO_EXPECTED_ERR (LEFT_BRACE, LEFT_PAREN);
    END
    INCR (buf_ptr2);
    EAT_BIB_WHITE_AND_EOF_CHECK;
    if (right_outer_delim == RIGHT_PAREN)
    BEGIN
      if (scan1_white (COMMA))
      BEGIN
	DO_NOTHING;
      END
    END
    else if (scan2_white (COMMA, RIGHT_BRACE))
    BEGIN
      DO_NOTHING;
    END

/***************************************************************************
 * WEB section number:	267
 * ~~~~~~~~~~~~~~~~~~~
 * The lower-case version of this database key must correspond to one in
 * |cite_list|, or else |all_entries| must be |true|, if this entry is to
 * be included in the reference list.  Accordingly, this module sets
 * |store_entry|, which determines whether the relevant information for
 * this entry is stored.  The alias kludge helps make the stack space not
 * overflow on some machines.
 ***************************************************************************/
      BEGIN
#ifdef TRACE
        if (Flag_trace) {
          TRACE_PR_TOKEN;
          TRACE_PR_LN (" is a database key");
        }
#endif                     		 	/* TRACE */

	tmp_ptr = buf_ptr1;
	while (tmp_ptr < buf_ptr2)
	BEGIN
	  EX_BUF3[tmp_ptr] = buffer[tmp_ptr];
	  INCR (tmp_ptr);
	END
	lower_case (EX_BUF3, buf_ptr1, TOKEN_LEN);
	if (all_entries)
	BEGIN
	  lc_cite_loc = str_lookup (EX_BUF3, buf_ptr1, TOKEN_LEN, LC_CITE_ILK,
				    DO_INSERT);
	END
	else
	BEGIN
	  lc_cite_loc = str_lookup (EX_BUF3, buf_ptr1, TOKEN_LEN, LC_CITE_ILK,
				    DONT_INSERT);
	END
	if (hash_found)
	BEGIN
	  entry_cite_ptr = ilk_info[ilk_info[lc_cite_loc]];

/***************************************************************************
 * WEB section number:	268
 * ~~~~~~~~~~~~~~~~~~~
 * It's illegal to have two (or more) entries with the same database key
 * (even if there are case differrences), and we skip the rest of the
 * entry for such a repeat occurrence.  Also, we make this entry's
 * database key the official |cite_list| key if it's on |cite_list| only
 * because of cross references.
 ***************************************************************************/
	  BEGIN
	    if (( ! all_entries) || (entry_cite_ptr < all_marker)
		    || (entry_cite_ptr >= old_num_cites))
	    BEGIN
	      if (type_list[entry_cite_ptr] == EMPTY)
	      BEGIN

/***************************************************************************
 * WEB section number:	269
 * ~~~~~~~~~~~~~~~~~~~
 * An entry that's on |cite_list| only because of cross referencing must
 * have its database key (rather than one of the crossref keys) as
 * the official |cite_list| string.  Here's where we assure that.  The
 * variable |hash_found| is |true| upon entrance to and exit from this
 * module.
 ***************************************************************************/
		BEGIN
		  if (( ! all_entries) && (entry_cite_ptr >= old_num_cites))
		  BEGIN
		    cite_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN,
					   CITE_ILK, DO_INSERT);
		    if ( ! hash_found)
		    BEGIN
		      ilk_info[lc_cite_loc] = cite_loc;
		      ilk_info[cite_loc] = entry_cite_ptr;
		      cite_list[entry_cite_ptr] = hash_text[cite_loc];
		      hash_found = TRUE;
		    END
		  END
		END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 269 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

		goto First_Time_Entry_Label;
	      END
	    END
	    else if ( ! entry_exists[entry_cite_ptr])
	    BEGIN

/***************************************************************************
 * WEB section number:	270
 * ~~~~~~~~~~~~~~~~~~~
 * This module, a simpler version of the
 * |find_cite_locs_for_this_cite_key| function, exists primarily to
 * compute |lc_xcite_loc|.  When this code is executed we have
 * |(all_entries) and (entry_cite_ptr >= all_marker) and (not
 * entry_exists[entry_cite_ptr])|.  The alias kludge helps make the stack
 * space not overflow on some machines.
 ***************************************************************************/
	      BEGIN
		EX_BUF4_PTR = 0;
		tmp_ptr = str_start[cite_info[entry_cite_ptr]];
		tmp_end_ptr = str_start[cite_info[entry_cite_ptr] + 1];
		while (tmp_ptr < tmp_end_ptr)
		BEGIN
		  EX_BUF4[EX_BUF4_PTR] = str_pool[tmp_ptr];
		  INCR (EX_BUF4_PTR);
		  INCR (tmp_ptr);
		END
		lower_case (EX_BUF4, 0, LENGTH (cite_info[entry_cite_ptr]));
		lc_xcite_loc = str_lookup (EX_BUF4, 0,
					   LENGTH (cite_info[entry_cite_ptr]),
					   LC_CITE_ILK, DONT_INSERT);
		if ( ! hash_found)
		BEGIN
		  cite_key_disappeared_confusion ();
		END
	      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 270 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	      if (lc_xcite_loc == lc_cite_loc)
	      BEGIN
		goto First_Time_Entry_Label;
	      END
	    END
	    if (type_list[entry_cite_ptr] == EMPTY)
	    BEGIN
	      CONFUSION ("The cite list is messed up");
	    END
	    BIB_ERR ("Repeated entry");
First_Time_Entry_Label:  DO_NOTHING;
	  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 268 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	END
	store_entry = TRUE;
	if (all_entries)

/***************************************************************************
 * WEB section number:	272
 * ~~~~~~~~~~~~~~~~~~~
 * This module, which gets executed only when |all_entries| is |true|,
 * does one of three things, depending on whether or not, and where, the
 * cite key appears on |cite_list|: If it's on |cite_list| before
 * |all_marker|, there's nothing to be done; if it's after |all_marker|,
 * it must be reinserted (at the current place) and we must note that its
 * corresponding entry exists; and if it's not on |cite_list| at all, it
 * must be inserted for the first time.  The |goto| construct must stay
 * as is, partly because some \PASCAL\ compilers might complain if
 * ``|and|'' were to connect the two boolean expressions (since
 * |entry_cite_ptr| could be uninitialized when |hash_found| is |false|).
 ***************************************************************************/
	BEGIN
	  if (hash_found)
	  BEGIN
	    if (entry_cite_ptr < all_marker)
	    BEGIN
	      goto Cite_Already_Set_Label;
	    END
	    else
	    BEGIN
	      entry_exists[entry_cite_ptr] = TRUE;
	      cite_loc = ilk_info[lc_cite_loc];
	    END
	  END
	  else
	  BEGIN
	    cite_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN, CITE_ILK,
				   DO_INSERT);
	    if (hash_found)
	    BEGIN
	      hash_cite_confusion ();
	    END
	  END
	  entry_cite_ptr = cite_ptr;
	  add_database_cite (&cite_ptr);
Cite_Already_Set_Label: DO_NOTHING;
	END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 272 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	else if ( ! hash_found)
	BEGIN
	  store_entry = FALSE;
	END
	if (store_entry)

/***************************************************************************
 * WEB section number:	273
 * ~~~~~~~~~~~~~~~~~~~
 * We must give a warning if this entry type doesn't exist.  Also, we
 * point the appropriate entry of |type_list| to the entry type just read
 * above.
 *
 * For SCRIBE compatibility, the code to give a warning for a case
 * mismatch between a cite key and a database key has been commented out.
 * In fact, SCRIBE is the reason that it doesn't produce an error message
 * outright.  (Note: Case mismatches between two cite keys produce
 * full-blown errors.)
 ***************************************************************************/
	BEGIN
    /*----------------------------------------------------------------------
     *    dummy_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN, CITE_ILK,
     * 			          DONT_INSERT);
     *    if ( ! hash_found)
     *    BEGIN
     * 	    PRINT ("Warning--case mismatch, database key \"");
     * 	    PRINT_TOKEN;
     * 	    PRINT ("\", cite key \"");
     * 	    PRINT_POOL_STR (cite_list[entry_cite_ptr]);
     * 	    BIB_WARN_NEWLINE ("\"");
     *    END
     *---------------------------------------------------------------------*/
	  if (type_exists)
	  BEGIN
	    type_list[entry_cite_ptr] = entry_type_loc;
	  END
	  else
	  BEGIN
	    type_list[entry_cite_ptr] = UNDEFINED;
	    PRINT ("Warning--entry type for \"");
	    PRINT_TOKEN;
	    BIB_WARN_NEWLINE ("\" isn't style-file defined");
	  END
	END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 273 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 267 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 266 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  EAT_BIB_WHITE_AND_EOF_CHECK;

/***************************************************************************
 * WEB section number:	274
 * ~~~~~~~~~~~~~~~~~~~
 * This module reads a |comma| and a field as many times as it can, and
 * then reads a |right_outer_delim|, ending the current entry.
 ***************************************************************************/
  BEGIN
    while (SCAN_CHAR != right_outer_delim)
    BEGIN
      if (SCAN_CHAR != COMMA)
      BEGIN
	BIB_ONE_OF_TWO_EXPECTED_ERR (COMMA, right_outer_delim);
      END
      INCR (buf_ptr2);
      EAT_BIB_WHITE_AND_EOF_CHECK;
      if (SCAN_CHAR == right_outer_delim)
      BEGIN
	goto Loop_Exit_Label;
      END

/***************************************************************************
 * WEB section number:	275
 * ~~~~~~~~~~~~~~~~~~~
 * This module reads a field name; its contents won't be stored unless it
 * was declared in the .bst file and |store_entry = true|.
 ***************************************************************************/
      BEGIN
	scan_identifier (EQUALS_SIGN, EQUALS_SIGN, EQUALS_SIGN);
	BIB_IDENTIFIER_SCAN_CHECK ("a field name");

#ifdef TRACE
        if (Flag_trace) {
	  TRACE_PR_TOKEN;
	  TRACE_PR_LN (" is a field name");
        }
#endif                      			/* TRACE */

	store_field = FALSE;
	if (store_entry)
	BEGIN
	  lower_case (buffer, buf_ptr1, TOKEN_LEN);
	  field_name_loc = str_lookup (buffer, buf_ptr1, TOKEN_LEN,
				       BST_FN_ILK, DONT_INSERT);
	  if (hash_found)
	  BEGIN
	    if (fn_type[field_name_loc] == FIELD)
	    BEGIN
	      store_field = TRUE;
	    END
	  END
	END
	EAT_BIB_WHITE_AND_EOF_CHECK;
	if (SCAN_CHAR != EQUALS_SIGN)
	BEGIN
	  BIB_EQUALS_SIGN_EXPECTED_ERR;
	END
	INCR (buf_ptr2);
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 275 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      EAT_BIB_WHITE_AND_EOF_CHECK;
      if ( ! scan_and_store_the_field_value ())
      BEGIN
	goto Exit_Label;
      END
    END
Loop_Exit_Label: DO_NOTHING;
    INCR (buf_ptr2);
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 274 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 236 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/***************************************************************************
 * WEB section number:	 154
 * ~~~~~~~~~~~~~~~~~~~
 * We must attend to a few details before getting to work on this .bst
 * command.
 ***************************************************************************/
void          get_bst_command_and_process (void)
BEGIN
  if ( ! scan_alpha ())
  BEGIN
    BST_ERR2 ("\"%c\"can't start a style-file command", xchr[SCAN_CHAR]);
  END
  lower_case (buffer, buf_ptr1, TOKEN_LEN);
  command_num = ilk_info[str_lookup (buffer, buf_ptr1, TOKEN_LEN,
				     BST_COMMAND_ILK, DONT_INSERT)];
  if ( ! hash_found)
  BEGIN
    PRINT_TOKEN;
    BST_ERR (" is an illegal style-file command");
  END

/***************************************************************************
 * WEB section number:	155
 * ~~~~~~~~~~~~~~~~~~~
 * Here we determine which .bst command we're about to process, and
 * then go to it.
 ***************************************************************************/
  switch (command_num)
  BEGIN
    case N_BST_ENTRY:
      bst_entry_command ();
      break;
    case N_BST_EXECUTE:
      bst_execute_command ();
      break;
    case N_BST_FUNCTION:
      bst_function_command ();
      break;
    case N_BST_INTEGERS:
      bst_integers_command ();
      break;
    case N_BST_ITERATE:
      bst_iterate_command ();
      break;
    case N_BST_MACRO:
      bst_macro_command ();
      break;
    case N_BST_READ:
      bst_read_command ();
      break;
    case N_BST_REVERSE:
      bst_reverse_command ();
      break;
    case N_BST_SORT:
      bst_sort_command ();
      break;
    case N_BST_STRINGS:
      bst_strings_command ();
      break;
    default:
      CONFUSION ("Unknown style-file command");
      break;
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 155 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 154 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 100
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure consists of a loop that reads and processes a (nonnull)
 * .aux file name.  It's this module and the next two that must be
 * changed on those systems using command-line arguments.  Note: The
 * |TERM_OUT| and |term_in| files are system dependent.
 *
 * This procedure now does almost nothing because the command line has 
 * already been parsed and checked for us.
 ***************************************************************************/
void          get_the_top_level_aux_file_name (void)
BEGIN

/***************************************************************************
 * WEB section number:	101
 * ~~~~~~~~~~~~~~~~~~~
 * The switch |check_cmnd_line| tells us whether we're to check for a
 * possible command-line argument.
 *
 * This procedure now does nothing because the command line has already
 * been parsed and checked for us.
 ***************************************************************************/
  Boolean_T	check_cmnd_line;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 101 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  check_cmnd_line = TRUE;
  LOOP
  BEGIN
    if (check_cmnd_line)

/***************************************************************************
 * WEB section number:	102
 * ~~~~~~~~~~~~~~~~~~~
 * Here's where we do the real command line work.  Actually, the command
 * line has already been parsed for us and the .aux file name stored in
 * Str_Auxfile.  If an .aux file was not specified, BibTeX will have
 * already stopped after issuing a "Usage" error.
 ***************************************************************************/
    BEGIN
      /*
      ** Leave room for the extension and the null byte at the end. 
      */
      aux_name_length = strlen (Str_auxfile);
      name_of_file = (unsigned char *) mymalloc (aux_name_length + 5, "name_of_file");
      strncpy ((char *) name_of_file, Str_auxfile, aux_name_length);

      /*
      ** If the auxilliary file was specified with the ".aux" part already
      ** appended, we strip it off here.
      */
      if (aux_name_length > 4) {
          if ((Str_auxfile[aux_name_length-4] == '.')
                  && ((Str_auxfile[aux_name_length-3] == 'a') || (Str_auxfile[aux_name_length-3] == 'A'))
                  && ((Str_auxfile[aux_name_length-2] == 'u') || (Str_auxfile[aux_name_length-2] == 'U'))
                  && ((Str_auxfile[aux_name_length-1] == 'x') || (Str_auxfile[aux_name_length-1] == 'X')))
              aux_name_length -= 4;
      }

      name_of_file[aux_name_length] = 0;
    END        

/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 102 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/***************************************************************************
 * WEB section number:	103
 * ~~~~~~~~~~~~~~~~~~~
 * Here we orchestrate this .aux name's handling: we add the various
 * extensions, try to open the files with the resulting name, and
 * store the name strings we'll need later.
 ***************************************************************************/
    BEGIN

/***************************************************************************
 * WEB section number:	106
 * ~~~~~~~~~~~~~~~~~~~
 * We must make sure the (top-level) .aux, .blg, and .bbl
 * files can be opened.
 ***************************************************************************/
      BEGIN
	name_length = aux_name_length;
	add_extension (s_aux_extension);
	aux_ptr = 0;
	if ( ! a_open_in (&CUR_AUX_FILE, AUX_FILE_SEARCH_PATH))
	BEGIN
	  SAM_YOU_MADE_THE_FILE_NAME_WRON;
	END

	name_length = aux_name_length;
	add_extension (s_log_extension);
	if ( ! a_open_out (&log_file))
	BEGIN
	  SAM_YOU_MADE_THE_FILE_NAME_WRON;
	END

	name_length = aux_name_length;
	add_extension (s_bbl_extension);
	if ( ! a_open_out (&bbl_file))
	BEGIN
	  SAM_YOU_MADE_THE_FILE_NAME_WRON;
	END
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 106 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	107
 * ~~~~~~~~~~~~~~~~~~~
 * This code puts the .aux file name, both with and without the
 * extension, into the hash table, and it initializes |aux_list|.  Note
 * that all previous top-level .aux-file stuff must have been
 * successful.
 ***************************************************************************/
      BEGIN
	name_length = aux_name_length;
	add_extension (s_aux_extension);
	name_ptr = 1;
	while (name_ptr <= name_length)
	BEGIN
	  buffer[name_ptr] = xord[name_of_file[name_ptr - 1]];
	  INCR (name_ptr);
	END
	top_lev_str = hash_text[str_lookup (buffer, 1, aux_name_length,
					    TEXT_ILK, DO_INSERT)];
	CUR_AUX_STR = hash_text[str_lookup (buffer, 1, name_length,
					    AUX_FILE_ILK, DO_INSERT)];
	if (hash_found)
	BEGIN

#ifdef TRACE
          if (Flag_trace)
	    print_aux_name ();
#endif                      	/* TRACE */

	  CONFUSION ("Already encountered auxiliary file");
	END
	CUR_AUX_LINE = 0;
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 107 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      goto Aux_Found_Label;
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 103 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

Aux_Not_Found_Label: check_cmnd_line = FALSE;
                     mark_fatal ();
                     debug_msg (DBG_MISC, 
                                "calling longjmp (Close_Up_Shop_Flag) ... ");
                     longjmp (Close_Up_Shop_Flag, 1);

  END
Aux_Found_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 100 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 137
 * ~~~~~~~~~~~~~~~~~~~
 * Here's a serious complaint (that is, a bug) concerning hash problems.
 * This is the first of several similar bug-procedures that exist only
 * because they save space.
 ***************************************************************************/
void          hash_cite_confusion (void)
BEGIN
  CONFUSION ("Cite hash error");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 137 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/***************************************************************************
 * WEB section number:	 165
 * ~~~~~~~~~~~~~~~~~~~
 * Here's another bug.
 ***************************************************************************/
void          id_scanning_confusion (void)
BEGIN
  CONFUSION ("Identifier scanning error");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 165 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 310
 * ~~~~~~~~~~~~~~~~~~~
 * More bug complaints, this time about bad literals.
 ***************************************************************************/
void          illegl_literal_confusion (void)
BEGIN
  CONFUSION ("Illegal literal type");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 310 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 316
 * ~~~~~~~~~~~~~~~~~~~
 * This module executes a single specified function once.  It can't do
 * anything with the entries.
 ***************************************************************************/
void          init_command_execution (void)
BEGIN
  lit_stk_ptr = 0;
  cmd_str_ptr = str_ptr;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 316 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/***************************************************************************
 * WEB section number:	 13
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure gets things started properly.
 ***************************************************************************/
void             initialize (void)
BEGIN
/***************************************************************************
 * WEB section number:	23
 * ~~~~~~~~~~~~~~~~~~~
 ***************************************************************************/
    short                   i;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  23 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	66
 * ~~~~~~~~~~~~~~~~~~~
 ***************************************************************************/
    HashLoc_T               k;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  66 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/***************************************************************************
 * WEB section number:	17
 * ~~~~~~~~~~~~~~~~~~~
 * Each digit-value of |bad| has a specific meaning.
 ***************************************************************************/
    bad = 0;

    if (MIN_PRINT_LINE < 3)
        bad = 1;

    if (MAX_PRINT_LINE <= MIN_PRINT_LINE)
        bad = 10 * bad + 2;

    if (MAX_PRINT_LINE >= Buf_Size)
        bad = 10 * bad + 3;

    if (Hash_Prime < 128)
        bad = 10 * bad + 4;

    if (Hash_Prime > Hash_Size)
        bad = 10 * bad + 5;

    /*
    ** The original WEB version of BibTeX imposed a maximum value of
    ** 16320 on Hash_Prime.  A WEB constant was defined:
    **
    **   MAX_HASH_VALUE = (Hash_Prime + Hash_Prime - 2 + 127)
    **
    ** but, because WEB constants are limited to 32767, Hash_Prime was
    ** consequently limited to a maximum of (32767 + 2 - 127) / 2 = 16321.
    **
    ** We're using C #defines, so that limit doesn't apply.  The
    ** following original sanity check is therefore commented out.
    **
    **   if (Hash_Prime >= (16384 - 64))
    **     bad = 10 * bad + 6;
    */

    if (Max_Strings > Hash_Size)
        bad = 10 * bad + 7;

    if (Max_Cites > Max_Strings)
        bad = 10 * bad + 8;

    /*
    ** The following checks have been removed because
    ** Buf_Size can be dynamically changed, whereas
    ** Ent_Str_Size and Glob_Str_Size are maximum values
    ** which are constants.
    **
    **    if (Ent_Str_Size > Buf_Size)
    **        bad = 10 * bad + 9;
    **
    **    if (Glob_Str_Size > Buf_Size)
    **        bad = 100 * bad + 11;
    */

/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 17 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	302
 * ~~~~~~~~~~~~~~~~~~~
 ***************************************************************************/
#if (SHORT_LIST < ((2 * END_OFFSET) + 2))
    bad = 100 * bad + 22;
#endif                          /* (SHORT_LIST < ((2 * END_OFFSET) + 2)) */
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 302 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


    if (bad > 0)
    BEGIN
        FPRINTF (TERM_OUT, "%ld is a bad bad\n", (long) bad);
        debug_msg (DBG_MISC, "calling longjmp (Exit_Program_Flag) ... ");
	longjmp (Exit_Program_Flag, 1);
    END

/***************************************************************************
 * WEB section number:	20
 * ~~~~~~~~~~~~~~~~~~~
 * The |err_count| gets set or reset when |history| first changes to
 * |warning_message| or |error_message|, so we don't need to initialize
 * it.
 ***************************************************************************/
    history = SPOTLESS;

/***************************************************************************
 * WEB section number:	25
 * ~~~~~~~~~~~~~~~~~~~
 * Since we are assuming that our \PASCAL\ system is able to read and write the
 * visible characters of standard ASCII (although not necessarily using the
 * ASCII codes to represent them), the following assignment statements
 * initialize
 * most of the |xchr| array properly, without needing any system-dependent
 * changes. On the other hand, it is possible to implement \TeX\ with
 * less complete character sets, and in such cases it will be necessary to
 * change something here.
 ***************************************************************************/
    xchr[0040] = ' ';
    xchr[0041] = '!';
    xchr[0042] = '"';
    xchr[0043] = '#';
    xchr[0044] = '$';
    xchr[0045] = '%';
    xchr[0046] = '&';
    xchr[0047] = '\'';
    xchr[0050] = '(';
    xchr[0051] = ')';
    xchr[0052] = '*';
    xchr[0053] = '+';
    xchr[0054] = ',';
    xchr[0055] = '-';
    xchr[0056] = '.';
    xchr[0057] = '/';
    xchr[0060] = '0';
    xchr[0061] = '1';
    xchr[0062] = '2';
    xchr[0063] = '3';
    xchr[0064] = '4';
    xchr[0065] = '5';
    xchr[0066] = '6';
    xchr[0067] = '7';
    xchr[0070] = '8';
    xchr[0071] = '9';
    xchr[0072] = ':';
    xchr[0073] = ';';
    xchr[0074] = '<';
    xchr[0075] = '=';
    xchr[0076] = '>';
    xchr[0077] = '?';
    xchr[0100] = '@';
    xchr[0101] = 'A';
    xchr[0102] = 'B';
    xchr[0103] = 'C';
    xchr[0104] = 'D';
    xchr[0105] = 'E';
    xchr[0106] = 'F';
    xchr[0107] = 'G';
    xchr[0110] = 'H';
    xchr[0111] = 'I';
    xchr[0112] = 'J';
    xchr[0113] = 'K';
    xchr[0114] = 'L';
    xchr[0115] = 'M';
    xchr[0116] = 'N';
    xchr[0117] = 'O';
    xchr[0120] = 'P';
    xchr[0121] = 'Q';
    xchr[0122] = 'R';
    xchr[0123] = 'S';
    xchr[0124] = 'T';
    xchr[0125] = 'U';
    xchr[0126] = 'V';
    xchr[0127] = 'W';
    xchr[0130] = 'X';
    xchr[0131] = 'Y';
    xchr[0132] = 'Z';
    xchr[0133] = '[';
    xchr[0134] = '\\';
    xchr[0135] = ']';
    xchr[0136] = '^';
    xchr[0137] = '_';
    xchr[0140] = '`';
    xchr[0141] = 'a';
    xchr[0142] = 'b';
    xchr[0143] = 'c';
    xchr[0144] = 'd';
    xchr[0145] = 'e';
    xchr[0146] = 'f';
    xchr[0147] = 'g';
    xchr[0150] = 'h';
    xchr[0151] = 'i';
    xchr[0152] = 'j';
    xchr[0153] = 'k';
    xchr[0154] = 'l';
    xchr[0155] = 'm';
    xchr[0156] = 'n';
    xchr[0157] = 'o';
    xchr[0160] = 'p';
    xchr[0161] = 'q';
    xchr[0162] = 'r';
    xchr[0163] = 's';
    xchr[0164] = 't';
    xchr[0165] = 'u';
    xchr[0166] = 'v';
    xchr[0167] = 'w';
    xchr[0170] = 'x';
    xchr[0171] = 'y';
    xchr[0172] = 'z';
    xchr[0173] = '{';
    xchr[0174] = '|';
    xchr[0175] = '}';
    xchr[0176] = '~';
    xchr[0000] = ' ';
    xchr[0177] = ' ';


#ifdef SUPPORT_8BIT
/*
**  Full 8Bit Support Note [ASIERRA95]:
**  BibTeX just must recognize characters greater than 127.
**    for (i=128; i<=255; i++)
**      xchr [i] = (unsigned char) i;
*/
#endif                          /* SUPPORT_8BIT */

/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 25 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	27
 * ~~~~~~~~~~~~~~~~~~~
 * The ASCII code is ``standard'' only to a certain extent, since many
 * computer installations have found it advantageous to have ready access
 * to more than 94 printing characters. Appendix C of The \TeX book
 * gives a complete specification of the intended correspondence between
 * characters and \TeX's internal representation.
 *
 * If \TeX\ is being used
 * on a garden-variety \PASCAL\ for which only standard ASCII
 * codes will appear in the input and output files, it doesn't really matter
 * what codes are specified in |xchr[1..@'37]|, but the safest policy is to
 * blank everything out by using the code shown below.
 *
 * However, other settings of |xchr| will make \TeX\ more friendly on
 * computers that have an extended character set, so that users can type things
 * like `\.^^Z' instead of `\ne'. At MIT, for example, it would be more
 * appropriate to substitute the code
 *
 *	  for i:=1 to '37 do xchr[i]:=chr(i);
 *
 * \TeX's character set is essentially the same as MIT's, even with respect to
 * characters less than~@'40. People with extended character sets can
 * assign codes arbitrarily, giving an |xchr| equivalent to whatever
 * characters the users of \TeX\ are allowed to have in their input files.
 * It is best to make the codes correspond to the intended interpretations as
 * shown in Appendix~C whenever possible; but this is not necessary. For
 * example, in countries with an alphabet of more than 26 letters, it is
 * usually best to map the additional letters into codes less than~@'40.
 ***************************************************************************/
    for (i=1; i<=0037; i++)
    BEGIN
        xchr[i] = ' ';
    END
    xchr[TAB] = CHR (TAB);
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 27 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	28
 * ~~~~~~~~~~~~~~~~~~~
 * This system-independent code makes the |xord| array contain a suitable
 * inverse to the information in |xchr|. Note that if |xchr[i]=xchr[j]|
 * where |i<j<@'177|, the value of |xord[xchr[i]]| will turn out to be
 * |j| or more; hence, standard ASCII code numbers will be used instead
 * of codes below @'40 in case there is a coincidence.
 ***************************************************************************/
    for (i=FIRST_TEXT_CHAR; i<=LAST_TEXT_CHAR; i++)
    BEGIN
	xord[CHR (i)] = INVALID_CODE;
    END

    for (i=1; i<=0176; i++)
    BEGIN
        xord[xchr[i]] = i;
    END

#ifdef SUPPORT_8BIT
/*
**    if (!Flag_7bit)
**      for (i=128; i<=LAST_ASCII_CHAR; i++)
**      BEGIN
**	  xord[xchr[i]] = i;
**      END
*/
#endif                          /* SUPPORT_8BIT */

/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 28 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	32
 * ~~~~~~~~~~~~~~~~~~~
 * Now we initialize the system-dependent |lex_class| array.  The |tab|
 * character may be system dependent.  Note that the order of these
 * assignments is important here.
 ***************************************************************************/
    for (i=0; i<=0177; i++)
    BEGIN
        lex_class[i] = OTHER_LEX;
    END

    for (i=0; i<=0037; i++)
    BEGIN
        lex_class[i] = ILLEGAL;
    END

    lex_class[INVALID_CODE] = ILLEGAL;
    lex_class[TAB] = WHITE_SPACE;
    lex_class[SPACE] = WHITE_SPACE;
    lex_class[TIE] = SEP_CHAR;
    lex_class[HYPHEN] = SEP_CHAR;

    for (i=0060; i<=0071; i++)
    BEGIN
        lex_class[i] = NUMERIC;
    END

    for (i=0101; i<=0132; i++)
    BEGIN
        lex_class[i] = ALPHA;
    END

    for (i=0141; i<=0172; i++)
    BEGIN
        lex_class[i] = ALPHA;
    END

#ifdef SUPPORT_8BIT
/*
**  Full 8Bit Support Note [ASIERRA95]:
**
**  All character that can be case-changed are supposed to be ALPHA.
**    for (i=128; i<=255; i++)
**        if ((c8lowcase[i] != c8upcase[i]) || (Flag_8bit))
**	     lex_class[i] = ALPHA;
*/
#endif                          /* SUPPORT_8BIT */


/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 32 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	33
 * ~~~~~~~~~~~~~~~~~~~
 * And now the |id_class| array.
 ***************************************************************************/
    for (i=0; i<=0177; i++)
    BEGIN
        id_class[i] = LEGAL_ID_CHAR;
    END

    for (i=0; i<=0037; i++)
    BEGIN
        id_class[i] = ILLEGAL_ID_CHAR;
    END

    id_class[SPACE] = ILLEGAL_ID_CHAR;
    id_class[TAB] = ILLEGAL_ID_CHAR;
    id_class[DOUBLE_QUOTE] = ILLEGAL_ID_CHAR;
    id_class[NUMBER_SIGN] = ILLEGAL_ID_CHAR;
    id_class[COMMENT] = ILLEGAL_ID_CHAR;
    id_class[SINGLE_QUOTE] = ILLEGAL_ID_CHAR;
    id_class[LEFT_PAREN] = ILLEGAL_ID_CHAR;
    id_class[RIGHT_PAREN] = ILLEGAL_ID_CHAR;
    id_class[COMMA] = ILLEGAL_ID_CHAR;
    id_class[EQUALS_SIGN] = ILLEGAL_ID_CHAR;
    id_class[LEFT_BRACE] = ILLEGAL_ID_CHAR;
    id_class[RIGHT_BRACE] = ILLEGAL_ID_CHAR;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 33 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	35
 * ~~~~~~~~~~~~~~~~~~~
 * Now we initialize the system-dependent |char_width| array, for which
 * |space| is the only |white_space| character given a nonzero printing
 * width.  The widths here are taken from Stanford's June~'87
 * $cmr10$~font and represent hundredths of a point (rounded), but since
 * they're used only for relative comparisons, the units have no meaning.
 ***************************************************************************/
    for (i=0; i<=0177; i++)
    BEGIN
        char_width[i] = 0;
    END

    char_width[0040] = 278;
    char_width[0041] = 278;
    char_width[0042] = 500;
    char_width[0043] = 833;
    char_width[0044] = 500;
    char_width[0045] = 833;
    char_width[0046] = 778;
    char_width[0047] = 278;
    char_width[0050] = 389;
    char_width[0051] = 389;
    char_width[0052] = 500;
    char_width[0053] = 778;
    char_width[0054] = 278;
    char_width[0055] = 333;
    char_width[0056] = 278;
    char_width[0057] = 500;
    char_width[0060] = 500;
    char_width[0061] = 500;
    char_width[0062] = 500;
    char_width[0063] = 500;
    char_width[0064] = 500;
    char_width[0065] = 500;
    char_width[0066] = 500;
    char_width[0067] = 500;
    char_width[0070] = 500;
    char_width[0071] = 500;
    char_width[0072] = 278;
    char_width[0073] = 278;
    char_width[0074] = 278;
    char_width[0075] = 778;
    char_width[0076] = 472;
    char_width[0077] = 472;
    char_width[0100] = 778;
    char_width[0101] = 750;
    char_width[0102] = 708;
    char_width[0103] = 722;
    char_width[0104] = 764;
    char_width[0105] = 681;
    char_width[0106] = 653;
    char_width[0107] = 785;
    char_width[0110] = 750;
    char_width[0111] = 361;
    char_width[0112] = 514;
    char_width[0113] = 778;
    char_width[0114] = 625;
    char_width[0115] = 917;
    char_width[0116] = 750;
    char_width[0117] = 778;
    char_width[0120] = 681;
    char_width[0121] = 778;
    char_width[0122] = 736;
    char_width[0123] = 556;
    char_width[0124] = 722;
    char_width[0125] = 750;
    char_width[0126] = 750;
    char_width[0127] = 1028;
    char_width[0130] = 750;
    char_width[0131] = 750;
    char_width[0132] = 611;
    char_width[0133] = 278;
    char_width[0134] = 500;
    char_width[0135] = 278;
    char_width[0136] = 500;
    char_width[0137] = 278;
    char_width[0140] = 278;
    char_width[0141] = 500;
    char_width[0142] = 556;
    char_width[0143] = 444;
    char_width[0144] = 556;
    char_width[0145] = 444;
    char_width[0146] = 306;
    char_width[0147] = 500;
    char_width[0150] = 556;
    char_width[0151] = 278;
    char_width[0152] = 306;
    char_width[0153] = 528;
    char_width[0154] = 278;
    char_width[0155] = 833;
    char_width[0156] = 556;
    char_width[0157] = 500;
    char_width[0160] = 556;
    char_width[0161] = 528;
    char_width[0162] = 392;
    char_width[0163] = 394;
    char_width[0164] = 389;
    char_width[0165] = 556;
    char_width[0166] = 528;
    char_width[0167] = 722;
    char_width[0170] = 528;
    char_width[0171] = 528;
    char_width[0172] = 444;
    char_width[0173] = 500;
    char_width[0174] = 1000;
    char_width[0175] = 500;
    char_width[0176] = 500;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 35 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	67
 * ~~~~~~~~~~~~~~~~~~~
 * Now it's time to initialize the hash table; note that |str_start[0]|
 * must be unused if |hash_text[k] := 0| is to have the desired effect.
 ***************************************************************************/
    for (k=HASH_BASE; k<=HASH_MAX; k++)
    BEGIN
        hash_next[k] = EMPTY;
        hash_text[k] = 0;
    END
    hash_used = HASH_MAX + 1;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 67 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	72
 * ~~~~~~~~~~~~~~~~~~~
 * Now that we've defined the hash-table workings we can initialize the
 * string pool.  Unlike \TeX, \BibTeX\ does not use a |pool_file| for
 * string storage; instead it inserts its pre-defined strings into
 * |str_pool|---this makes one file fewer for the \BibTeX\ implementor
 * to deal with.  This section initializes |str_pool|; the pre-defined
 * strings will be inserted into it shortly; and other strings are
 * inserted while processing the input files.
 ***************************************************************************/
    pool_ptr = 0;
    str_ptr = 1;
    str_start[str_ptr] = pool_ptr;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 72 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	119
 * ~~~~~~~~~~~~~~~~~~~
 *
 ***************************************************************************/
    bib_ptr = 0;
    bib_seen = FALSE;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 119 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	125
 * ~~~~~~~~~~~~~~~~~~~
 * And we initialize.
 ***************************************************************************/
    bst_str = 0;
    bst_seen = FALSE;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 125 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	131
 * ~~~~~~~~~~~~~~~~~~~
 *
 ***************************************************************************/
    cite_ptr = 0;
    citation_seen = FALSE;
    all_entries = FALSE;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 131 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	162
 * ~~~~~~~~~~~~~~~~~~~
 * Now we initialize storage for the |wiz_defined| functions and we
 * initialize variables so that the first |str_entry_var|,
 * |int_entry_var|, |str_global_var|, and |field| name will be assigned
 * the number~0.  Note: The variables |num_ent_strs| and |num_fields|
 * will also be set when pre-defining strings.
 ***************************************************************************/
    wiz_def_ptr = 0;
    num_ent_ints = 0;
    num_ent_strs = 0;            
    num_fields = 0;
    str_glb_ptr = 0;
    while (str_glb_ptr < Max_Glob_Strs)
    BEGIN
        glb_str_ptr[str_glb_ptr] = 0;
        glb_str_end[str_glb_ptr] = 0;
        INCR (str_glb_ptr);
    END
    num_glb_strs = 0;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 162 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	164
 * ~~~~~~~~~~~~~~~~~~~
 * And we initialize them.
 ***************************************************************************/
    entry_seen = FALSE;
    read_seen = FALSE;
    read_performed = FALSE;
    reading_completed = FALSE;
    read_completed = FALSE;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 164 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	196
 * ~~~~~~~~~~~~~~~~~~~
 * Now we initialize it.
 ***************************************************************************/
    impl_fn_num = 0;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 196 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	292
 * ~~~~~~~~~~~~~~~~~~~
 * And the first output line requires this initialization.
 ***************************************************************************/
    out_buf_length = 0;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 292 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 20 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    pre_def_certain_strings ();
    get_the_top_level_aux_file_name ();
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  13 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 47
 * ~~~~~~~~~~~~~~~~~~~
 * The |input_ln| function brings the next line of input from the
 * specified file into available positions of the buffer array and
 * returns the value |true|, unless the file has already been entirely
 * read, in which case it returns |false| and sets |last:=0|.  In
 * general, the |ASCII_code| numbers that represent the next line of the
 * file are input into |buffer[0]|, |buffer[1]|, \dots, |buffer[last-1]|;
 * and the global variable |last| is set equal to the length of the line.
 * Trailing |white_space| characters are removed from the line
 * (|white_space| characters are explained in the character-set section%
 * ---most likely they're blanks); thus, either |last=0| (in which case
 * the line was entirely blank) or |lex_class[buffer[last-1]]<>white_space|.
 * An overflow error is given if the normal actions of |input_ln| would
 * make |last>buf_size|.
 ***************************************************************************/
Boolean_T         input_ln (AlphaFile_T f)
BEGIN
  Boolean_T    	  input_ln;

  last = 0;

  if (feof (f))
  BEGIN
    input_ln = FALSE;
  END
  else
  BEGIN
    while ( ! eoln (f))
    BEGIN
      if (last >= Buf_Size)
      BEGIN
        buffer_overflow ();
      END

      buffer[last] = xord[getc(f)];
      INCR (last);
    END
    (void) getc (f);

    while (last > 0)
    BEGIN
      if (lex_class[buffer[last - 1]] == WHITE_SPACE)
      BEGIN
	DECR (last);
      END
      else
      BEGIN
	goto Loop_Exit_Label;
      END
    END
Loop_Exit_Label:  input_ln = TRUE;
  END
  return (input_ln);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 47 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 198
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure takes the integer |int|, copies the appropriate
 * |ASCII_code| string into |int_buf| starting at |int_begin|, and sets
 * the |var| parameter |int_end| to the first unused |int_buf| location.
 * The ASCII string will consist of decimal digits, the first of which
 * will be not be a~0 if the integer is nonzero, with a prepended minus
 * sign if the integer is negative.
 ***************************************************************************/
void          int_to_ASCII (Integer_T integer, BufType_T int_buf,
			    BufPointer_T int_begin, BufPointer_T *int_end)
BEGIN
  BufPointer_T      int_ptr,
		    int_xptr;
  ASCIICode_T       int_tmp_val;

  int_ptr = int_begin;
  if (integer < 0)
  BEGIN
    APPEND_INT_CHAR (MINUS_SIGN);
    integer = -integer;
  END
  int_xptr = int_ptr;
  REPEAT
  BEGIN
    APPEND_INT_CHAR ('0' + (integer MOD 10));
    integer = integer DIV 10;
  END UNTIL (integer == 0);
  *int_end = int_ptr;
  DECR (int_ptr);
  while (int_xptr < int_ptr)
  BEGIN
    int_tmp_val = int_buf[int_xptr];
    int_buf[int_xptr] = int_buf[int_ptr];
    int_buf[int_ptr] = int_tmp_val;
    DECR (int_ptr);
    INCR (int_xptr);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 198 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 145
 * ~~~~~~~~~~~~~~~~~~~
 * Before proceeding, we see if we have any complaints.
 ***************************************************************************/
void          last_check_for_aux_errors (void)
BEGIN
  num_cites = cite_ptr;
  num_bib_files = bib_ptr;
  if ( ! citation_seen)
  BEGIN
    AUX_END_ERR ("\\citation commands");
  END
  else if ((num_cites == 0) && ( ! all_entries))
  BEGIN
    AUX_END_ERR ("cite keys");
  END
  if ( ! bib_seen)
  BEGIN
    AUX_END_ERR ("\\bibdata command");
  END
  else if (num_bib_files == 0)
  BEGIN
    AUX_END_ERR ("database files");
  END
  if ( ! bst_seen)
  BEGIN
    AUX_END_ERR ("\\bibstyle command");
  END
  else if (bst_str == 0)
  BEGIN
    AUX_END_ERR ("style file");
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 145 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 301
 * ~~~~~~~~~~~~~~~~~~~
 * The function |less_than| compares the two sort.key$s indirectly
 * pointed to by its arguments and returns |true| if the first argument's
 * sort.key$ is lexicographically less than the second's (that is,
 * alphabetically earlier).  In case of ties the function compares the
 * indices |arg1| and |arg2|, which are assumed to be different, and
 * returns |true| if the first is smaller.  This function uses
 * |ASCII_code|s to compare, so it might give ``interesting'' results
 * when handling nonletters.
 ***************************************************************************/
Boolean_T         less_than (CiteNumber_T arg1, CiteNumber_T arg2)
BEGIN
  StrEntLoc_T		ptr1,
			ptr2;
#ifdef UTF_8
/*
We use ICU libs to processing UTF-8. First, we have to transform UTF-8 to 
Unicode/UChar with the fonction icu_UCHars. Then we use the UCollator 
in the ICU libs to conparer the Unicode. There is an option "location", 
we use "-o" to indicate the rule of conpare.             23/sep/2009
*/
  Integer_T lenk1, lenk2;
  UChar uch1[BUF_SIZE+1], uch2[BUF_SIZE+1];
  UBool u_less;
  UCollator * ucol1;
  int32_t ucap = BUF_SIZE+1;
  int32_t uchlen1, uchlen2;
  UErrorCode err1 = U_ZERO_ERROR;
#else
  Boolean_T		less_than;
  Integer_T		char_ptr;
  ASCIICode_T		char1,
			char2;
#endif

#ifdef TRACE
  if (Flag_trace)
    TRACE_PR_LN3 ("Comparing entry %ld and %ld ...", arg1, arg2);
#endif                      			/* TRACE */
  ptr1 = (arg1 * num_ent_strs) + sort_key_num;
  ptr2 = (arg2 * num_ent_strs) + sort_key_num;
#ifdef UTF_8
  lenk1 = strlen((char *)&ENTRY_STRS(ptr1, 0));
  lenk2 = strlen((char *)&ENTRY_STRS(ptr2, 0));

/*
icu_toUChars() seems not working here, using u_strFromUTF8 instead. (04/mar/2019)
*/

/*
Use u_strFromUTF8WithSub() with a substitution character 0xfffd,
instead of u_strFromUTF8(). (05/mar/2019)
If err1 != U_ZERO_ERROR, the original functions are used. (06/mar/2019)
*/

/*
  uchlen1 = icu_toUChars(entry_strs, (ptr1 * (ENT_STR_SIZE+1)), lenk1, uch1, ucap);
  uchlen2 = icu_toUChars(entry_strs, (ptr2 * (ENT_STR_SIZE+1)), lenk2, uch2, ucap);
*/

  u_strFromUTF8WithSub(uch1, ucap, &uchlen1, (char *)&ENTRY_STRS(ptr1, 0), lenk1, 0xfffd, NULL, &err1);
  if (!U_SUCCESS(err1)) {
    printf("Error in u_strFromUTF8WithSub 1.\n");
#ifdef TRACE
    if (Flag_trace)
      TRACE_PR_LN ("Error in u_strFromUTF8WithSub 1");
#endif                      			/* TRACE */
    uchlen1 = icu_toUChars(entry_strs, (ptr1 * (ENT_STR_SIZE+1)), lenk1, uch1, ucap);
    err1 = U_ZERO_ERROR;
  }

  u_strFromUTF8WithSub(uch2, ucap, &uchlen2, (char *)&ENTRY_STRS(ptr2, 0), lenk2, 0xfffd, NULL, &err1);
  if (!U_SUCCESS(err1)) {
    printf("Error in u_strFromUTF8WithSub 2.\n");
#ifdef TRACE
    if (Flag_trace)
      TRACE_PR_LN ("Error in u_strFromUTF8WithSub 2");
#endif                      			/* TRACE */
    uchlen2 = icu_toUChars(entry_strs, (ptr2 * (ENT_STR_SIZE+1)), lenk2, uch2, ucap);
    err1 = U_ZERO_ERROR;
  }

  if(Flag_location)
    ucol1 = ucol_open(Str_location, &err1);
  else
    ucol1 = ucol_open(NULL, &err1);
  if (!U_SUCCESS(err1))
    printf("Error in opening a ucol in less_than.\n");
  u_less = !ucol_greaterOrEqual(ucol1, uch1, uchlen1, uch2, uchlen2);
#ifdef TRACE
  if (Flag_trace)
    TRACE_PR_LN2 ("... first is smaller than second? -- %s (ICU)", (u_less?"T":"F"));
#endif                      			/* TRACE */

  ucol_close(ucol1);
  return u_less;
#else
  char_ptr = 0;
  LOOP
  BEGIN
    char1 = ENTRY_STRS(ptr1, char_ptr);
    char2 = ENTRY_STRS(ptr2, char_ptr);
    if (char1 == END_OF_STRING)
    BEGIN
      if (char2 == END_OF_STRING)
      BEGIN
        if (arg1 < arg2)
        BEGIN
	  COMPARE_RETURN (TRUE);
	END
        else if (arg1 > arg2)
        BEGIN
	  COMPARE_RETURN (FALSE);
        END
        else
        BEGIN
          CONFUSION ("Duplicate sort key");
        END
      END
      else
      BEGIN
	COMPARE_RETURN (TRUE);
      END
    END
    else if (char2 == END_OF_STRING)
    BEGIN
      COMPARE_RETURN (FALSE);
    END
    else if char_less_than(char1, char2)
    BEGIN
      COMPARE_RETURN (TRUE);
    END
    else if char_greater_than(char1, char2)
    BEGIN
      COMPARE_RETURN (FALSE);
    END
    INCR (char_ptr);
  END
Exit_Label:
#ifdef TRACE
  if (Flag_trace)
    TRACE_PR_LN2 ("... first is smaller than second? -- %s", (less_than?"T":"F"));
#endif                      			/* TRACE */
  return (less_than);
#endif
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 301 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 62
 * ~~~~~~~~~~~~~~~~~~~
 * This system-independent procedure converts upper-case characters to
 * lower case for the specified part of |buf|.  It is system independent
 * because it uses only the internal representation for characters.
 ***************************************************************************/
void          lower_case (BufType_T buf, BufPointer_T bf_ptr,
			  BufPointer_T len)
BEGIN
  BufPointer_T      i;

  if (len > 0)
  BEGIN
    for (i = bf_ptr; i <= (bf_ptr + len - 1); i++)
    BEGIN

#ifdef SUPPORT_8BIT
      if (IsUpper (buf[i]))
      BEGIN
	buf[i] = ToLower (buf[i]);
      END
#else                           /* NOT SUPPORT_8BIT */
      if ((buf[i] >= 'A') && (buf[i] <= 'Z'))
      BEGIN
        buf[i] = buf[i] + CASE_DIFFERENCE;
      END
#endif                          /* SUPPORT_8BIT */

    END
  END
END

#ifdef UTF_8
/*
"lower_case_uni" is the fonction for processing the characters, actually the UTF-8.
We transform UTF-8 to Unicode, then to low case, then back to UTF-8 for output.
When we transform the character, the length have been changed. So we have do 
some job for the length. And the output of this fonction we should be careful 
to the length.                                                   23/sep/2009
*/
BufPointer_T       lower_case_uni (BufType_T buf, BufPointer_T bf_ptr,
			  BufPointer_T len)
BEGIN

	UChar target[BUF_SIZE+1];
	int32_t tarcap=BUF_SIZE+1;
	int32_t tarlen = icu_toUChars(buf, bf_ptr, len, target, tarcap);

	UChar tarlow[BUF_SIZE+1];
	int32_t tlcap=BUF_SIZE+1;
	int32_t tllen=icu_strToLower(tarlow, tlcap,target, tarlen);
	
	unsigned char dest[BUF_SIZE+1];
	int32_t destcap= BUF_SIZE-bf_ptr;
		
	int32_t tblen=icu_fromUChars(dest, destcap, (const UChar *) tarlow, tllen);

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
This fonction is for transform UTF-8 to Unicode with ICU libs.		 23/sep/2009
*/
int32_t icu_toUChars(BufType_T buf, BufPointer_T bf_ptr,BufPointer_T len,UChar * target, int32_t tarcap)
BEGIN
	UConverter * ucon1;
	UErrorCode err1 = U_ZERO_ERROR;
	ucon1 = ucnv_open(NULL, &err1);
	if (!U_SUCCESS(err1))
	BEGIN
		printf("Error in opening a ucnv in icu_toUChars.\n");
	END
	ucnv_toUChars(ucon1, target, tarcap, (char *)&buf[bf_ptr], len, &err1);
	if (!U_SUCCESS(err1))
	BEGIN
		printf("Error in icu_toUChars.\n");
#ifdef TRACE
		if (Flag_trace)
			TRACE_PR_LN ("Error in icu_toUChars");
#endif                      			/* TRACE */
	END
	ucnv_close(ucon1);
	
	return len;
END

/*
This fonction is for transform Unicode string to low case. 23/sep/2009
*/
int32_t icu_strToLower(UChar * tarlow, int32_t tlcap, UChar * target, int32_t tarlen)
BEGIN
	int32_t tllen;
	UErrorCode err1 = U_ZERO_ERROR;
	if (!U_SUCCESS(err1))
	BEGIN
		printf("Error in icu_strToLower?\n");
	END
	if (Flag_language)
	{
		tllen=u_strToLower(tarlow,tlcap, target,tarlen,Str_language,&err1);
	}
	else
	{
		tllen=u_strToLower(tarlow,tlcap, target,tarlen,NULL,&err1);
	}
	if (!U_SUCCESS(err1))
	BEGIN
		int16_t i=0;
		printf("Error in icu_strToLower.\n");
#ifdef TRACE
		if (Flag_trace)
			TRACE_PR_LN ("Error in icu_strToLower");
#endif                      			/* TRACE */
		for (i=0;i<tarlen;i++)
		BEGIN
			printf("%X", target[i]);
		END
		printf("\n");
		for (i=0;i<tllen;i++)
		BEGIN
			printf("%X", tarlow[i]);
		END
		printf("\n");
	END
	
	return tllen;
END


/*
This fonction is for transform Unicode to UTF-8. 23/sep/2009
*/
int32_t icu_fromUChars(unsigned char * dest, int32_t destcap, const UChar * src, int32_t srclen)
BEGIN
	UConverter * ucon2;
	UErrorCode err2 = U_ZERO_ERROR;
	int32_t tblen;
	ucon2 = ucnv_open(NULL, &err2);
	if (!U_SUCCESS(err2))
	BEGIN
		printf("Error in opening a ucnv in icu_fromUChars.\n");
	END
	tblen=ucnv_fromUChars(ucon2, (char *)dest, destcap, src, srclen, &err2);
	if (!U_SUCCESS(err2))
	BEGIN
		printf("Error in icu_fromUChars.\n");
#ifdef TRACE
		if (Flag_trace)
			TRACE_PR_LN ("Error in icu_fromUChars");
#endif                      			/* TRACE */
	END
	
	return tblen;
END
#endif
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 62 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 234
 * ~~~~~~~~~~~~~~~~~~~
 * This one is just a warning, not an error.  It's for when something
 * isn't (or might not be) quite right with a macro name.
 ***************************************************************************/
void          macro_warn_print (void)
BEGIN
  PRINT ("Warning--string name \"");
  PRINT_TOKEN;
  PRINT ("\" is ");
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 234 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 54
 * ~~~~~~~~~~~~~~~~~~~
 * Once a sequence of characters has been appended to |str_pool|, it
 * officially becomes a string when the function |make_string| is called.
 * It returns the string number of the string it just made.
 ***************************************************************************/
StrNumber_T       make_string (void)
BEGIN
  if (str_ptr == Max_Strings)
  BEGIN
    BIBTEX_OVERFLOW ("number of strings ", Max_Strings);
  END
  INCR (str_ptr);
  str_start[str_ptr] = pool_ptr;
  return (str_ptr - 1);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 54 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 18
 * ~~~~~~~~~~~~~~~~~~~
 *
 ***************************************************************************/
void          mark_error (void)
BEGIN
  if (history < ERROR_MESSAGE)
  BEGIN
    history = ERROR_MESSAGE;
    err_count = 1;
  END
  else
  BEGIN
    INCR (err_count);
  END
END


void          mark_fatal (void)
BEGIN
  history = FATAL_MESSAGE;
END


void          mark_warning (void)
BEGIN
  if (history == WARNING_MESSAGE)
  BEGIN
    INCR (err_count);
  END
  else if (history == SPOTLESS)
  BEGIN
    history = WARNING_MESSAGE;
    err_count = 1;
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  18 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 384
 * ~~~~~~~~~~~~~~~~~~~
 * This module, starting at |ex_buf_ptr|, looks in |ex_buf| for an
 * ``and'' surrounded by nonnull |white_space|.  It stops either at
 * |ex_buf_length| or just past the ``and'', whichever comes first,
 * setting |ex_buf_ptr| accordingly.  Its parameter |pop_lit_var| is
 * either |pop_lit3| or |pop_lit1|, depending on whether
 * format.name$ or num.names$ calls it.
 ***************************************************************************/
void          name_scan_for_and (StrNumber_T pop_lit_var)
BEGIN
  brace_level = 0;
  preceding_white = FALSE;
  and_found = FALSE;
  while (( ! and_found) && (ex_buf_ptr < ex_buf_length))
  BEGIN
    switch (ex_buf[ex_buf_ptr])
    BEGIN
      case 'a':
      case 'A':
        INCR (ex_buf_ptr);
        if (preceding_white)

/***************************************************************************
 * WEB section number:	386
 * ~~~~~~~~~~~~~~~~~~~
 * When we come here |ex_buf_ptr| is just past the ``a'' or ``A'', and when
 * we leave it's either at the same place or, if we found an ``and'', at
 * the following |white_space| character.
 ***************************************************************************/
        BEGIN
          if (ex_buf_ptr <= (ex_buf_length - 3))
          BEGIN
            if ((ex_buf[ex_buf_ptr] == 'n') || (ex_buf[ex_buf_ptr] == 'N'))
            BEGIN
	      if ((ex_buf[ex_buf_ptr + 1] == 'd')
		    || (ex_buf[ex_buf_ptr + 1] == 'D'))
              BEGIN
	        if (lex_class[ex_buf[ex_buf_ptr + 2]] == WHITE_SPACE)
                BEGIN
                  ex_buf_ptr = ex_buf_ptr + 2;
                  and_found = TRUE;
                END
	      END
	    END
          END
        END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 386 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

        preceding_white = FALSE;
        break;
      case LEFT_BRACE:
        INCR (brace_level);
        INCR (ex_buf_ptr);

/***************************************************************************
 * WEB section number:	385
 * ~~~~~~~~~~~~~~~~~~~
 * When we come here |ex_buf_ptr| is just past the |left_brace|, and when
 * we leave it's either at |ex_buf_length| or just past the matching
 * |right_brace|.
 ***************************************************************************/
        BEGIN
	  while ((brace_level > 0) && (ex_buf_ptr < ex_buf_length))
	  BEGIN
	    if (ex_buf[ex_buf_ptr] == RIGHT_BRACE)
	    BEGIN
	      DECR (brace_level);
	    END
	    else if (ex_buf[ex_buf_ptr] == LEFT_BRACE)
	    BEGIN
	      INCR (brace_level);
	    END
	    INCR (ex_buf_ptr);
	  END
	END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 385 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

        preceding_white = FALSE;
        break;
      case RIGHT_BRACE:
        decr_brace_level (pop_lit_var);
        INCR (ex_buf_ptr);
        preceding_white = FALSE;
        break;
      default:
        if (lex_class[ex_buf[ex_buf_ptr]] == WHITE_SPACE)
        BEGIN
          INCR (ex_buf_ptr);
          preceding_white = TRUE;
        END
        else
        BEGIN
          INCR (ex_buf_ptr);
          preceding_white = FALSE;
        END
        break;
    END
  END
  check_brace_level (pop_lit_var);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 384 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 281
 * ~~~~~~~~~~~~~~~~~~~
 * When an entry being cross referenced doesn't exist on |cite_list|, we
 * complain.
 ***************************************************************************/
void          non_existent_cross_reference_er (void)
BEGIN
  PRINT ("A bad cross reference-");
  bad_cross_reference_print (field_info[field_ptr]);
  PRINT_LN ("\", which doesn't exist");
  mark_error ();
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 281 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/***************************************************************************
 * WEB section number:	 51
 * ~~~~~~~~~~~~~~~~~~~
 * And here are the associated procedures.  Note: The |TERM_OUT| file is
 * system dependent.
 ***************************************************************************/
void          out_pool_str (AlphaFile_T f, StrNumber_T s)
BEGIN
  PoolPointer_T       i;

  if ((s < 0) || (s >= (str_ptr + MAX_POP)) || (s >= Max_Strings))
  BEGIN
    CONFUSION2 ("Illegal string number:%ld", (long) s);
  END

  if (f != NULL)
  BEGIN
    for (i=str_start[s]; i<=(str_start[s+1] - 1); i++)
    BEGIN
      FPUTC (xchr[str_pool[i]], f);
    END
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 51 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 321
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure actually writes onto the .bbl file a line of output
 * (the characters from |out_buf[0]| to |out_buf[out_buf_length-1]|,
 * after removing trailing |white_space| characters).  It also updates
 * |bbl_line_num|, the line counter.  It writes a blank line if and only
 * if |out_buf| is empty.  The program uses this procedure in such a way
 * that |out_buf| will be nonempty if there have been characters put in
 * it since the most recent newline$.
 ***************************************************************************/
void          output_bbl_line (void)
BEGIN
  if (out_buf_length != 0)
  BEGIN
    while (out_buf_length > 0)
    BEGIN
      if (lex_class[out_buf[out_buf_length - 1]] == WHITE_SPACE)
      BEGIN
        DECR (out_buf_length);
      END
      else
      BEGIN
        goto Loop_Exit_Label;
      END
    END
Loop_Exit_Label:  if (out_buf_length == 0)
    BEGIN
      goto Exit_Label;
    END
    out_buf_ptr = 0;
    while (out_buf_ptr < out_buf_length)
    BEGIN
      FPUTC (xchr[out_buf[out_buf_ptr]], bbl_file);
      INCR (out_buf_ptr);
    END
  END
  FPUTC ('\n', bbl_file);
  INCR (bbl_line_num);
  out_buf_length = 0;
Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 321 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 82
 * ~~~~~~~~~~~~~~~~~~~
 * And here are the associated procedures.  Note: The |term_out| file is
 * system dependent.
 ***************************************************************************/
void          out_token (AlphaFile_T f)
BEGIN
  BufPointer_T      i;

  if (f != NULL)
  BEGIN
    i = buf_ptr1;
    while (i < buf_ptr2)
    BEGIN
      FPUTC (xchr[buffer[i]], f);
      INCR (i);
    END
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 82 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 53
 * ~~~~~~~~~~~~~~~~~~~
 * To test if there is room to append |l| more characters to |str_pool|,
 * we shall write |str_room(l)|, which aborts \BibTeX\ and gives an
 * error message if there isn't enough room.
 ***************************************************************************/
void          pool_overflow (void)
BEGIN
  BIB_XRETALLOC ("str_pool", str_pool, ASCIICode_T,
                 Pool_Size, Pool_Size + POOL_SIZE);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  53 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 309
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure pops the stack, checking for, and trying to recover
 * from, stack underflow.  (Actually, this procedure is really a
 * function, since it returns the two values through its |var|
 * parameters.)  Also, if the literal being popped is a |stk_str| that's
 * been created during the execution of the current .bst command, pop
 * it from |str_pool| as well (it will be the string corresponding to
 * |str_ptr-1|).  Note that when this happens, the string is no longer
 * `officially' available so that it must be used before anything else is
 * added to |str_pool|.
 ***************************************************************************/
void          pop_lit_stk (Integer_T *poplit, StkType_T *poptype)
BEGIN
  if (lit_stk_ptr == 0)
  BEGIN
    BST_EX_WARN ("You can't pop an empty literal stack");
    *poptype = STK_EMPTY;
  END
  else
  BEGIN
    DECR (lit_stk_ptr);
    *poplit = lit_stack[lit_stk_ptr];
    *poptype = lit_stk_type[lit_stk_ptr];
    if (*poptype == STK_STR)
    BEGIN
      if (*poplit >= cmd_str_ptr)
      BEGIN
        if (*poplit != (str_ptr - 1))
        BEGIN
          CONFUSION ("Nontop top of string stack");
        END
	FLUSH_STRING;
      END
    END
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 309 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 142
 * ~~~~~~~~~~~~~~~~~~~
 * Here we close the current-level .aux file and go back up a level,
 * if possible, by DECRing |aux_ptr|.
 ***************************************************************************/
void          pop_the_aux_stack (void)
BEGIN
  a_close (CUR_AUX_FILE);
  if (aux_ptr == 0)
  BEGIN
    debug_msg (DBG_MISC, "calling longjmp (Aux_Done_Flag) ... ");
    longjmp (Aux_Done_Flag, 1);
    return;
  END
  else
  BEGIN
    DECR (aux_ptr);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 142 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 314
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure pops and prints the top of the stack; when the stack is
 * empty the procedure |pop_lit_stk| complains.
 ***************************************************************************/
void          pop_top_and_print (void)
BEGIN
  Integer_T         stk_lt;
  StkType_T         stk_tp;

  pop_lit_stk (&stk_lt, &stk_tp);
  if (stk_tp == STK_EMPTY)
  BEGIN
    PRINT_LN ("Empty literal");
  END
  else
  BEGIN
    print_lit (stk_lt, stk_tp);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 314 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 315
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure pops and prints the whole stack.
 ***************************************************************************/
void          pop_whole_stack (void)
BEGIN
  while (lit_stk_ptr > 0)
  BEGIN
    pop_top_and_print ();
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 315 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
