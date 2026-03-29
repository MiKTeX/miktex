/*-
******************************************************************************
******************************************************************************
**
**  MODULE
**
**      file: bibtex-4.c
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
**      This is the fourth of 4 source modules for BibTeX.  The source has 
**      been split into 4 parts so that some of the more primitive editors
**      can cope.  This code mimics the BibTeX WEB source as closely as
**      possible and there should be NO system dependent code in any of the 
**      bibtex-#.c modules.
**
**      The functions defined in this module are:
**
**          x_change_case
**          x_chr_to_int
**          x_cite
**          x_concatenate
**          x_duplicate
**          x_empty
**          x_equals
**          x_format_name
**          x_gets
**          x_greater_than
**          x_int_to_chr
**          x_int_to_str
**          x_less_than
**          x_minus
**          x_missing
**          x_num_names
**          x_plus
**          x_preamble
**          x_purify
**          x_quote
**          x_substring
**          x_swap
**          x_text_length
**          x_text_prefix
**          x_type
**          x_warning
**          x_width
**          x_write
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
**      $Log: bibtex-4.c,v $
**      Revision 3.71  1996/08/18  20:47:30  kempson
**      Official release 3.71 (see HISTORY file for details).
**
**      Revision 3.70  1996/04/08  10:08:40  kempson
**      Final documentation & cosmetic changes for official release 3.70.
**
**      Revision 3.5  1995/09/24  20:44:37  kempson
**      Many changes for final beta test version.
**
**      Revision 3.4  1995/04/09  22:15:41  kempson
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
 * WEB section number:	 364
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function change.case$ pops the top two (string)
 * literals; it changes the case of the second according to the
 * specifications of the first, as follows.  (Note: The word `letters' in
 * the next sentence refers only to those at brace-level~0, the top-most
 * brace level; no other characters are changed, except perhaps for
 * special characters, described shortly.)  If the first literal is the
 * string t, it converts to lower case all letters except the very
 * first character in the string, which it leaves alone, and except the
 * first character following any |colon| and then nonnull |white_space|,
 * which it also leaves alone; if it's the string l, it converts all
 * letters to lower case; if it's the string u, it converts all
 * letters to upper case; and if it's anything else, it complains and
 * does no conversion.  It then pushes this resulting string.  If either
 * type is incorrect, it complains and pushes the null string; however,
 * if both types are correct but the specification string (i.e., the
 * first string) isn't one of the legal ones, it merely pushes the second
 * back onto the stack, after complaining.  (Another note: It ignores
 * case differences in the specification string; for example, the strings
 * t and T are equivalent for the purposes of this |built_in|
 * function.)
 ***************************************************************************/
void          x_change_case (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  pop_lit_stk (&pop_lit2, &pop_typ2);
  if (pop_typ1 != STK_STR)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_STR);
    push_lit_stk (s_null, STK_STR);
  END
  else if (pop_typ2 != STK_STR)
  BEGIN
    print_wrong_stk_lit (pop_lit2, pop_typ2, STK_STR);
    push_lit_stk (s_null, STK_STR);
  END
  else
  BEGIN

/***************************************************************************
 * WEB section number:	366
 * ~~~~~~~~~~~~~~~~~~~
 * First we define a few variables for case conversion.  The constant
 * definitions, to be used in |case| statements, are in order of probable
 * frequency.
 ***************************************************************************/
    BEGIN
      switch (str_pool[str_start[pop_lit1]])
      BEGIN
	case 't':
	case 'T':
	  conversion_type = TITLE_LOWERS;
	  break;
	case 'l':
	case 'L':
	  conversion_type = ALL_LOWERS;
	  break;
	case 'u':
	case 'U':
	  conversion_type = ALL_UPPERS;
	  break;
	default:
	  conversion_type = BAD_CONVERSION;
	  break;
      END
      if ((LENGTH (pop_lit1) != 1) || (conversion_type == BAD_CONVERSION))
      BEGIN
	conversion_type = BAD_CONVERSION;
	PRINT_POOL_STR (pop_lit1);
	BST_EX_WARN (" is an illegal case-conversion string");
      END
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 366 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    ex_buf_length = 0;
    add_buf_pool (pop_lit2);

/***************************************************************************
 * WEB section number:	370
 * ~~~~~~~~~~~~~~~~~~~
 * Here's where we actually go through the string and do the case
 * conversion.
 ***************************************************************************/
    BEGIN
      brace_level = 0;
      ex_buf_ptr = 0;
      while (ex_buf_ptr < ex_buf_length)
      BEGIN
	if (ex_buf[ex_buf_ptr] == LEFT_BRACE)
	BEGIN
	  INCR (brace_level);
	  if (brace_level != 1)
	  BEGIN
	    goto OK_Pascal_I_Give_Up_Label;
	  END
	  if ((ex_buf_ptr + 4) > ex_buf_length)
	  BEGIN
	    goto OK_Pascal_I_Give_Up_Label;
	  END
	  else if (ex_buf[ex_buf_ptr + 1] != BACKSLASH)
	  BEGIN
	    goto OK_Pascal_I_Give_Up_Label;
	  END
	  if (conversion_type == TITLE_LOWERS)
	  BEGIN
	    if (ex_buf_ptr == 0)
	    BEGIN
	      goto OK_Pascal_I_Give_Up_Label;
	    END
	    else if ((prev_colon)
		      && (lex_class[ex_buf[ex_buf_ptr - 1]] == WHITE_SPACE))
	    BEGIN
	      goto OK_Pascal_I_Give_Up_Label;
	    END
	  END

/***************************************************************************
 * WEB section number:	371
 * ~~~~~~~~~~~~~~~~~~~
 * We're dealing with a special character (usually either an undotted
 * `\i' or `\j', or an accent like one in Table~3.1 of the \LaTeX\
 * manual, or a foreign character like one in Table~3.2) if the first
 * character after the |left_brace| is a |backslash|; the special
 * character ends with the matching |right_brace|.  How we handle what's
 * in between depends on the special character.  In general, this code
 * will do reasonably well if there is other stuff, too, between braces,
 * but it doesn't try to do anything special with |colon|s.
 ***************************************************************************/
	  BEGIN
	    INCR (ex_buf_ptr);
	    while ((ex_buf_ptr < ex_buf_length) && (brace_level > 0))
	    BEGIN
	      INCR (ex_buf_ptr);
	      ex_buf_xptr = ex_buf_ptr;
	      while ((ex_buf_ptr < ex_buf_length)
			  && (lex_class[ex_buf[ex_buf_ptr]] == ALPHA))
	      BEGIN
		INCR (ex_buf_ptr);
	      END
	      control_seq_loc = str_lookup (ex_buf, ex_buf_xptr,
					    ex_buf_ptr - ex_buf_xptr,
					    CONTROL_SEQ_ILK, DONT_INSERT);
	      if (hash_found)

/***************************************************************************
 * WEB section number:	372
 * ~~~~~~~~~~~~~~~~~~~
 * A control sequence, for the purposes of this program, consists just of
 * the consecutive alphabetic characters following the |backslash|; it
 * might be empty (although ones in this section aren't).
 ***************************************************************************/
	      BEGIN
		switch (conversion_type)
		BEGIN
		  case TITLE_LOWERS:
		  case ALL_LOWERS:
		    switch (ilk_info[control_seq_loc])
		    BEGIN
		      case N_L_UPPER:
		      case N_O_UPPER:
		      case N_OE_UPPER:
		      case N_AE_UPPER:
		      case N_AA_UPPER:
			Lower_case (ex_buf, ex_buf_xptr,
				    ex_buf_ptr - ex_buf_xptr);
			break;
		      default:
			DO_NOTHING;
			break;
		    END
		    break;
		  case ALL_UPPERS:
		    switch (ilk_info[control_seq_loc])
		    BEGIN
		      case N_L:
		      case N_O:
		      case N_OE:
		      case N_AE:
		      case N_AA:
			upper_case (ex_buf, ex_buf_xptr,
				    ex_buf_ptr - ex_buf_xptr);
			break;
		      case N_I:
		      case N_J:
		      case N_SS:

/***************************************************************************
 * WEB section number:	374
 * ~~~~~~~~~~~~~~~~~~~
 * After converting the control sequence, we need to remove the preceding
 * |backslash| and any following |white_space|.
 ***************************************************************************/
			BEGIN
			  upper_case (ex_buf, ex_buf_xptr,
				      ex_buf_ptr - ex_buf_xptr);
			  while (ex_buf_xptr < ex_buf_ptr)
			  BEGIN
			    ex_buf[ex_buf_xptr - 1] = ex_buf[ex_buf_xptr];
			    INCR (ex_buf_xptr);
			  END
			  DECR (ex_buf_xptr);
			  while ((ex_buf_ptr < ex_buf_length)
			    && (lex_class[ex_buf[ex_buf_ptr]] == WHITE_SPACE))
			  BEGIN
			    INCR (ex_buf_ptr);
			  END
			  tmp_ptr = ex_buf_ptr;
			  while (tmp_ptr < ex_buf_length)
			  BEGIN
			    ex_buf[tmp_ptr - (ex_buf_ptr - ex_buf_xptr)]
			      = ex_buf[tmp_ptr];
			    INCR (tmp_ptr);
			  END
			  ex_buf_length = tmp_ptr - (ex_buf_ptr - ex_buf_xptr);
			  ex_buf_ptr = ex_buf_xptr;
			END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 374 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

			break;
		      default:
			DO_NOTHING;
			break;
		    END
		    break;
		  case BAD_CONVERSION:
		    DO_NOTHING;
		    break;
		  default:
		    case_conversion_confusion ();
		    break;
		END
	      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 372 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	      ex_buf_xptr = ex_buf_ptr;
	      while ((ex_buf_ptr < ex_buf_length) && (brace_level > 0)
		      && (ex_buf[ex_buf_ptr] != BACKSLASH))
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

/***************************************************************************
 * WEB section number:	375
 * ~~~~~~~~~~~~~~~~~~~
 * There are no control sequences in what we're about to convert,
 * so a straight conversion suffices.
 ***************************************************************************/
	      switch (conversion_type)
	      BEGIN
		case TITLE_LOWERS:
		case ALL_LOWERS:
		  Lower_case (ex_buf, ex_buf_xptr, ex_buf_ptr - ex_buf_xptr);
		  break;
		case ALL_UPPERS:
		  upper_case (ex_buf, ex_buf_xptr, ex_buf_ptr - ex_buf_xptr);
		  break;
		case BAD_CONVERSION:
		  DO_NOTHING;
		  break;
		default:
		  case_conversion_confusion ();
		  break;
	      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 375 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    END
	    DECR (ex_buf_ptr);
	  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 371 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

OK_Pascal_I_Give_Up_Label:  prev_colon = FALSE;
	END
	else if (ex_buf[ex_buf_ptr] == RIGHT_BRACE)
	BEGIN
	  decr_brace_level (pop_lit2);
	  prev_colon = FALSE;
	END
	else if (brace_level == 0)

/***************************************************************************
 * WEB section number:	376
 * ~~~~~~~~~~~~~~~~~~~
 * This code does any needed conversion for an ordinary character; it
 * won't touch nonletters.
 ***************************************************************************/
	BEGIN
	  switch (conversion_type)
	  BEGIN
	    case TITLE_LOWERS:
	      if (ex_buf_ptr == 0)
	      BEGIN
#ifdef UTF_8
/*
For the case of TITLE_LOWERS, we transform the characters to low case except the first 
character. When it's UTF-8, we should care about the length of charater.   23/sep/2009
*/
		DO_UTF8(ex_buf[ex_buf_ptr], , ex_buf_ptr = 1, ex_buf_ptr = 2, ex_buf_ptr = 3);
#else
		DO_NOTHING;
#endif
	      END
	      else if ((prev_colon)
			&& (lex_class[ex_buf[ex_buf_ptr - 1]] == WHITE_SPACE))
	      BEGIN
		DO_NOTHING;
	      END
	      else
	      BEGIN
#ifdef UTF_8
/*
When we do lower_case_uni, the length of string have been changed. So we should do some job 
for the precessing after lower case. Here there may be some potential bug.      23/sep/2009
*/
		int16_t llen;

		llen=utf8len(ex_buf[ex_buf_ptr]);
		ex_buf_ptr=ex_buf_ptr+lower_case_uni (ex_buf, ex_buf_ptr, llen)-1;
#else
		lower_case (ex_buf, ex_buf_ptr, 1);
#endif
	      END
	      if (ex_buf[ex_buf_ptr] == COLON)
	      BEGIN
		prev_colon = TRUE;
	      END
	      else if (lex_class[ex_buf[ex_buf_ptr]] != WHITE_SPACE)
	      BEGIN
		prev_colon = FALSE;
	      END
	      break;
	    case ALL_LOWERS:
#ifdef UTF_8
/*
Here the same for processing the length of string after change case. 23/sep/2009
*/
	      BEGIN
		int16_t llen;

		llen=utf8len(ex_buf[ex_buf_ptr]);
		ex_buf_ptr=ex_buf_ptr+lower_case_uni (ex_buf, ex_buf_ptr, llen)-1;
	      END
#else
	      lower_case (ex_buf, ex_buf_ptr, 1);
#endif
	      break;
	    case ALL_UPPERS:
#ifdef UTF_8
/*
Here the same for processing the length of string after change case. 23/sep/2009
*/
              BEGIN
		int16_t ulen;

		ulen=utf8len(ex_buf[ex_buf_ptr]);
		ex_buf_ptr=ex_buf_ptr+upper_case_uni (ex_buf, ex_buf_ptr, ulen)-1;
              END
#else
	      upper_case (ex_buf, ex_buf_ptr, 1);
#endif
	      break;
	    case BAD_CONVERSION:
	      DO_NOTHING;
	      break;
	    default:
	      case_conversion_confusion ();
	      break;
	  END
	END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 376 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	INCR (ex_buf_ptr);
      END
      check_brace_level (pop_lit2);
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 370 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    add_pool_buf_and_push ();
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 364 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 377
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function chr.to.int$ pops the top (string)
 * literal, makes sure it's a single character, converts it to the
 * corresponding |ASCII_code| integer, and pushes this integer.  If the
 * literal isn't an appropriate string, it complains and pushes the
 * integer~0.
 ***************************************************************************/
void          x_chr_to_int (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  if (pop_typ1 != STK_STR)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_STR);
    push_lit_stk (0, STK_INT);
  END
#ifdef UTF_8
  else if (LENGTH (pop_lit1) != utf8len(str_pool[str_start[pop_lit1]]))
#else
  else if (LENGTH (pop_lit1) != 1)
#endif
  BEGIN
    PRINT ("\"");
    PRINT_POOL_STR (pop_lit1);
    BST_EX_WARN ("\" isn't a single character");
    push_lit_stk (0, STK_INT);
  END
  else
  BEGIN
#ifdef UTF_8
    UChar32 ch;
    U8_GET_OR_FFFD(&str_pool[str_start[pop_lit1]], 0, 0, -1, ch);
    push_lit_stk (ch, STK_INT);
#else
    push_lit_stk (str_pool[str_start[pop_lit1]], STK_INT);
#endif
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 377 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 378
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function cite pushes the appropriate string
 * from |cite_list| onto the stack.
 ***************************************************************************/
void          x_cite (void)
BEGIN
  if ( ! mess_with_entries)
  BEGIN
    bst_cant_mess_with_entries_prin ();
  END
  else
  BEGIN
    push_lit_stk (CUR_CITE_STR, STK_STR);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 378 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 350
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function * pops the top two (string) literals,
 * concatenates them (in reverse order, that is, the order in which
 * pushed), and pushes the resulting string back onto the stack.  If
 * either isn't a string literal, it complains and pushes the null
 * string.
 ***************************************************************************/
void          x_concatenate (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  pop_lit_stk (&pop_lit2, &pop_typ2);
  if (pop_typ1 != STK_STR)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_STR);
    push_lit_stk (s_null, STK_STR);
  END
  else if (pop_typ2 != STK_STR)
  BEGIN
    print_wrong_stk_lit (pop_lit2, pop_typ2, STK_STR);
    push_lit_stk (s_null, STK_STR);
  END
  else

/***************************************************************************
 * WEB section number:	351
 * ~~~~~~~~~~~~~~~~~~~
 * Often both strings will be at the top of the string pool, in which
 * case we just move some pointers.  Furthermore, it's worth doing some
 * special stuff in case either string is null, since empirically this
 * seems to happen about $20\%$ of the time.  In any case, we don't need
 * the execution buffer---we simple move the strings around in the string
 * pool when necessary.
 ***************************************************************************/
  BEGIN
    if (pop_lit2 >= cmd_str_ptr)
    BEGIN
      if (pop_lit1 >= cmd_str_ptr)
      BEGIN
        str_start[pop_lit1] = str_start[pop_lit1 + 1];
	UNFLUSH_STRING;
        INCR (lit_stk_ptr);
      END
      else if (LENGTH (pop_lit2) == 0)
      BEGIN
        push_lit_stk (pop_lit1, STK_STR);
      END
      else
      BEGIN
        pool_ptr = str_start[pop_lit2 + 1];
	STR_ROOM (LENGTH (pop_lit1));
        sp_ptr = str_start[pop_lit1];
        sp_end = str_start[pop_lit1 + 1];
        while (sp_ptr < sp_end)
        BEGIN
	  APPEND_CHAR (str_pool[sp_ptr]);
          INCR (sp_ptr);
        END
        push_lit_stk (make_string (), STK_STR);
      END
    END
    else

/***************************************************************************
 * WEB section number:	352
 * ~~~~~~~~~~~~~~~~~~~
 * We simply continue the previous module.
 ***************************************************************************/
    BEGIN
      if (pop_lit1 >= cmd_str_ptr)
      BEGIN
        if (LENGTH (pop_lit2) == 0)
        BEGIN
	  UNFLUSH_STRING;
          lit_stack[lit_stk_ptr] = pop_lit1;
          INCR (lit_stk_ptr);
        END
        else if (LENGTH (pop_lit1) == 0)
        BEGIN
          INCR (lit_stk_ptr);
        END
        else
        BEGIN
          sp_length = LENGTH (pop_lit1);
          sp2_length = LENGTH (pop_lit2);
	  STR_ROOM (sp_length + sp2_length);
          sp_ptr = str_start[pop_lit1 + 1];
          sp_end = str_start[pop_lit1];
          sp_xptr1 = sp_ptr + sp2_length;
          while (sp_ptr > sp_end)
          BEGIN
            DECR (sp_ptr);
            DECR (sp_xptr1);
            str_pool[sp_xptr1] = str_pool[sp_ptr];
          END
          sp_ptr = str_start[pop_lit2];
          sp_end = str_start[pop_lit2 + 1];
          while (sp_ptr < sp_end)
          BEGIN
            APPEND_CHAR (str_pool[sp_ptr]);
            INCR (sp_ptr);
          END
          pool_ptr = pool_ptr + sp_length;
          push_lit_stk (make_string (), STK_STR);
        END
      END
      else

/***************************************************************************
 * WEB section number:	353
 * ~~~~~~~~~~~~~~~~~~~
 * Again, we simply continue the previous module.
 ***************************************************************************/
      BEGIN
        if (LENGTH (pop_lit1) == 0)
        BEGIN
          INCR (lit_stk_ptr);
        END
        else if (LENGTH (pop_lit2) == 0)
        BEGIN
          push_lit_stk (pop_lit1, STK_STR);
        END
        else
        BEGIN
	  STR_ROOM (LENGTH (pop_lit1) + LENGTH (pop_lit2));
          sp_ptr = str_start[pop_lit2];
          sp_end = str_start[pop_lit2 + 1];
          while (sp_ptr < sp_end)
          BEGIN
	    APPEND_CHAR (str_pool[sp_ptr]);
            INCR (sp_ptr);
          END
          sp_ptr = str_start[pop_lit1];
          sp_end = str_start[pop_lit1 + 1];
          while (sp_ptr < sp_end)
          BEGIN
	    APPEND_CHAR (str_pool[sp_ptr]);
            INCR (sp_ptr);
          END
          push_lit_stk (make_string (), STK_STR);
        END
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 353 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 352 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 351 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 350 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 379
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function duplicate$ pops the top literal from
 * the stack and pushes two copies of it.
 ***************************************************************************/
void          x_duplicate (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  if (pop_typ1 != STK_STR)
  BEGIN
    push_lit_stk (pop_lit1, pop_typ1);
    push_lit_stk (pop_lit1, pop_typ1);
  END
  else
  BEGIN
    REPUSH_STRING;
    if (pop_lit1 < cmd_str_ptr)
    BEGIN
      push_lit_stk (pop_lit1, pop_typ1);
    END
    else
    BEGIN
      STR_ROOM (LENGTH (pop_lit1));
      sp_ptr = str_start[pop_lit1];
      sp_end = str_start[pop_lit1 + 1];
      while (sp_ptr < sp_end)
      BEGIN
	APPEND_CHAR (str_pool[sp_ptr]);
        INCR (sp_ptr);
      END
      push_lit_stk (make_string (), STK_STR);
    END
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 379 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 380
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function empty$ pops the top literal and pushes
 * the integer 1 if it's a missing field or a string having no
 * non|white_space| characters, 0 otherwise.  If the literal isn't a
 * missing field or a string, it complains and pushes 0.
 ***************************************************************************/
void          x_empty (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  switch (pop_typ1)
  BEGIN
    case STK_STR:

/***************************************************************************
 * WEB section number:	381
 * ~~~~~~~~~~~~~~~~~~~
 * When we arrive here we're dealing with a legitimate string.  If it has
 * no characters, or has nothing but |white_space| characters, we push~1,
 * otherwise we push~0.
 ***************************************************************************/
      BEGIN
	sp_ptr = str_start[pop_lit1];
	sp_end = str_start[pop_lit1 + 1];
	while (sp_ptr < sp_end)
	BEGIN
	  if (lex_class[str_pool[sp_ptr]] != WHITE_SPACE)
	  BEGIN
	    push_lit_stk (0, STK_INT);
	    goto Exit_Label;
	  END
	  INCR (sp_ptr);
	END
	push_lit_stk (1, STK_INT);
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 381 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      break;
    case STK_FIELD_MISSING:
      push_lit_stk (1, STK_INT);
      break;
    case STK_EMPTY:
      push_lit_stk (0, STK_INT);
      break;
    default:
      print_stk_lit (pop_lit1, pop_typ1);
      BST_EX_WARN (", not a string or missing field,");
      push_lit_stk (0, STK_INT);
      break;
  END
Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 380 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 345
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function = pops the top two (integer or string)
 * literals, compares them, and pushes the integer 1 if they're equal, 0
 * otherwise.  If they're not either both string or both integer, it
 * complains and pushes the integer 0.
 ***************************************************************************/
void          x_equals (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  pop_lit_stk (&pop_lit2, &pop_typ2);
  if (pop_typ1 != pop_typ2)
  BEGIN
    if ((pop_typ1 != STK_EMPTY) && (pop_typ2 != STK_EMPTY))
    BEGIN
      print_stk_lit (pop_lit1, pop_typ1);
      PRINT (", ");
      print_stk_lit (pop_lit2, pop_typ2);
      PRINT_NEWLINE;
      BST_EX_WARN ("---they aren't the same literal types");
    END
    push_lit_stk (0, STK_INT);
  END
  else if ((pop_typ1 != STK_INT) && (pop_typ1 != STK_STR))
  BEGIN
    if (pop_typ1 != STK_EMPTY)
    BEGIN
      print_stk_lit (pop_lit1, pop_typ1);
      BST_EX_WARN (", not an integer or a string,");
    END
    push_lit_stk (0, STK_INT);
  END
  else if (pop_typ1 == STK_INT)
  BEGIN
    if (pop_lit2 == pop_lit1)
    BEGIN
      push_lit_stk (1, STK_INT);
    END
    else
    BEGIN
      push_lit_stk (0, STK_INT);
    END
  END
  else if (str_eq_str (pop_lit2, pop_lit1))
  BEGIN
    push_lit_stk (1, STK_INT);
  END
  else
  BEGIN
    push_lit_stk (0, STK_INT);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 345 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 382
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function format.name$ pops the top three
 * literals (they are a string, an integer, and a string literal, in that
 * order).  The last string literal represents a name list (each name
 * corresponding to a person), the integer literal specifies which name
 * to pick from this list, and the first string literal specifies how to
 * format this name, as described in the \BibTeX\ documentation.
 * Finally, this function pushes the formatted name.  If any of the types
 * is incorrect, it complains and pushes the null string.
 ***************************************************************************/
void          x_format_name (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  pop_lit_stk (&pop_lit2, &pop_typ2);
  pop_lit_stk (&pop_lit3, &pop_typ3);
  if (pop_typ1 != STK_STR)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_STR);
    push_lit_stk (s_null, STK_STR);
  END
  else if (pop_typ2 != STK_INT)
  BEGIN
    print_wrong_stk_lit (pop_lit2, pop_typ2, STK_INT);
    push_lit_stk (s_null, STK_STR);
  END
  else if (pop_typ3 != STK_STR)
  BEGIN
    print_wrong_stk_lit (pop_lit3, pop_typ3, STK_STR);
    push_lit_stk (s_null, STK_STR);
  END
  else
  BEGIN
    ex_buf_length = 0;
    add_buf_pool (pop_lit3);

/***************************************************************************
 * WEB section number:	383
 * ~~~~~~~~~~~~~~~~~~~
 * This module skips over undesired names in |pop_lit3| and it throws
 * away the ``and'' from the end of the name if it exists.  When it's
 * done, |ex_buf_xptr| points to its first character and |ex_buf_ptr|
 * points just past its last.
 ***************************************************************************/
    BEGIN
      ex_buf_ptr = 0;
      num_names = 0;
      while ((num_names < pop_lit2) && (ex_buf_ptr < ex_buf_length))
      BEGIN
	INCR (num_names);
	ex_buf_xptr = ex_buf_ptr;
	name_scan_for_and (pop_lit3);
      END
      if (ex_buf_ptr < ex_buf_length)
      BEGIN
#ifdef UTF_8
        if (ex_buf[ex_buf_ptr-3]==0xE3 || ex_buf[ex_buf_ptr-3]==0xEF)
          /* expect U+3001 "、" or U+FF0C "，" :: Ideographic/Fulwidth Comma */
          ex_buf_ptr = ex_buf_ptr - 3;
        else
          /* expect "and " or "AND " */
#endif
        ex_buf_ptr = ex_buf_ptr - 4;
      END
      if (num_names < pop_lit2)
      BEGIN
	if (pop_lit2 == 1)
	BEGIN
	  PRINT ("There is no name in \"");
	END
	else
	BEGIN
	  PRINT2 ("There aren't %ld names in \"", (long) pop_lit2);
	END
	PRINT_POOL_STR (pop_lit3);
	BST_EX_WARN ("\"");
      END
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 383 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/***************************************************************************
 * WEB section number:	387
 * ~~~~~~~~~~~~~~~~~~~
 * When we arrive here, the desired name is in |ex_buf[ex_buf_xptr]|
 * through |ex_buf[ex_buf_ptr-1]|.  This module does its thing for
 * characters only at |brace_level = 0|; the rest get processed verbatim.
 * It removes leading |white_space| (and |sep_char|s), and trailing
 * |white_space| (and |sep_char|s) and |comma|s, complaining for each
 * trailing |comma|.  It then copies the name into |name_buf|, removing
 * all |white_space|, |sep_char|s and |comma|s, counting |comma|s, and
 * constructing a list of name tokens, which are sequences of characters
 * separated (at |brace_level=0|) by |white_space|, |sep_char|s or
 * |comma|s.  Each name token but the first has an associated
 * |name_sep_char|, the character that separates it from the preceding
 * token.  If there are too many (more than two) |comma|s, a complaint is
 * in order.
 ***************************************************************************/
    BEGIN
/***************************************************************************
 * WEB section number:	388
 * ~~~~~~~~~~~~~~~~~~~
 * This module removes all leading |white_space| (and |sep_char|s), and
 * trailing |white_space| (and |sep_char|s) and |comma|s.  It complains
 * for each trailing |comma|.
 ***************************************************************************/
      BEGIN
	while ((ex_buf_xptr < ex_buf_ptr)
		&& (lex_class[ex_buf[ex_buf_ptr]] == WHITE_SPACE)
		&& (lex_class[ex_buf[ex_buf_ptr]] == SEP_CHAR))
	BEGIN
	  INCR (ex_buf_xptr);
	END
	while (ex_buf_ptr > ex_buf_xptr)
	BEGIN
	  switch (lex_class[ex_buf[ex_buf_ptr - 1]])
	  BEGIN
	    case WHITE_SPACE:
	    case SEP_CHAR:
	      DECR (ex_buf_ptr);
	      break;
	    default:
	      if (ex_buf[ex_buf_ptr - 1] == COMMA)
	      BEGIN
		PRINT2 ("Name %ld in \"", (long) pop_lit2);
		PRINT_POOL_STR (pop_lit3);
		PRINT ("\" has a comma at the end");
		bst_ex_warn_print ();
		DECR (ex_buf_ptr);
	      END
	      else
	      BEGIN
		goto Loop1_Exit_Label;
	      END
	      break;
	  END
	END
Loop1_Exit_Label:  DO_NOTHING;
      END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 388 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      name_bf_ptr = 0;
      num_commas = 0;
      num_tokens = 0;
      token_starting = TRUE;
      while (ex_buf_xptr < ex_buf_ptr)
      BEGIN
	switch (ex_buf[ex_buf_xptr])
	BEGIN
	  case COMMA:

/***************************************************************************
 * WEB section number:	389
 * ~~~~~~~~~~~~~~~~~~~
 * Here we mark the token number at which this comma has occurred.
 ***************************************************************************/
	    BEGIN
	      if (num_commas == 2)
	      BEGIN
		PRINT2 ("Too many commas in name %ld of \"", (long) pop_lit2);
		PRINT_POOL_STR (pop_lit3);
		PRINT ("\"");
		bst_ex_warn_print ();
	      END
	      else
	      BEGIN
		INCR (num_commas);
		if (num_commas == 1)
		BEGIN
		  comma1 = num_tokens;
		END
		else
		BEGIN
		  comma2 = num_tokens;
		END
		name_sep_char[num_tokens] = COMMA;
	      END
	      INCR (ex_buf_xptr);
	      token_starting = TRUE;
	    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 389 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    break;
	  case LEFT_BRACE:

/***************************************************************************
 * WEB section number:	390
 * ~~~~~~~~~~~~~~~~~~~
 * We copy the stuff up through the matching |right_brace| verbatim.
 ***************************************************************************/
	    BEGIN
	      INCR (brace_level);
	      if (token_starting)
	      BEGIN
		name_tok[num_tokens] = name_bf_ptr;
		INCR (num_tokens);
	      END
	      NAME_BUF[name_bf_ptr] = ex_buf[ex_buf_xptr];
	      INCR (name_bf_ptr);
	      INCR (ex_buf_xptr);
	      while ((brace_level > 0) && (ex_buf_xptr < ex_buf_ptr))
	      BEGIN
		if (ex_buf[ex_buf_xptr] == RIGHT_BRACE)
		BEGIN
		  DECR (brace_level);
		END
		else if (ex_buf[ex_buf_xptr] == LEFT_BRACE)
		BEGIN
		  INCR (brace_level);
		END
		NAME_BUF[name_bf_ptr] = ex_buf[ex_buf_xptr];
		INCR (name_bf_ptr);
		INCR (ex_buf_xptr);
	      END
	      token_starting = FALSE;
	    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 390 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    break;
	  case RIGHT_BRACE:

/***************************************************************************
 * WEB section number:	391
 * ~~~~~~~~~~~~~~~~~~~
 * We don't copy an extra |right_brace|; this code will almost never be
 * executed.
 ***************************************************************************/
	    BEGIN
	      if (token_starting)
	      BEGIN
		name_tok[num_tokens] = name_bf_ptr;
		INCR (num_tokens);
	      END
	      PRINT2 ("Name %ld of \"", (long) pop_lit2);
	      PRINT_POOL_STR (pop_lit3);
	      BST_EX_WARN ("\" isn't brace balanced");
	      INCR (ex_buf_xptr);
	      token_starting = FALSE;
	    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 391 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

	    break;
	  default:
	    switch (lex_class[ex_buf[ex_buf_xptr]])
	    BEGIN
	      case WHITE_SPACE:

/***************************************************************************
 * WEB section number:	392
 * ~~~~~~~~~~~~~~~~~~~
 * A token will be starting soon in a buffer near you, one way$\ldots$
 ***************************************************************************/
		BEGIN
		  if ( ! token_starting)
		  BEGIN
		    name_sep_char[num_tokens] = SPACE;
		  END
		  INCR (ex_buf_xptr);
		  token_starting = TRUE;
		END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 392 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

		break;
	      case SEP_CHAR:

/***************************************************************************
 * WEB section number:	393
 * ~~~~~~~~~~~~~~~~~~~
 * or another.  If one of the valid |sep_char|s appears between tokens,
 * we usually use it instead of a |space|.  If the user has been silly
 * enough to have multiple |sep_char|s, or to have both |white_space| and
 * a |sep_char|, we use the first such character.
 ***************************************************************************/
		BEGIN
		  if ( ! token_starting)
		  BEGIN
		    name_sep_char[num_tokens] = ex_buf[ex_buf_xptr];
		  END
		  INCR (ex_buf_xptr);
		  token_starting = TRUE;
		END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 393 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

		break;
	      default:

/***************************************************************************
 * WEB section number:	394
 * ~~~~~~~~~~~~~~~~~~~
 * For ordinary characters, we just copy the character.
 ***************************************************************************/
		BEGIN
		  if (token_starting)
		  BEGIN
		    name_tok[num_tokens] = name_bf_ptr;
		    INCR (num_tokens);
		  END
		  NAME_BUF[name_bf_ptr] = ex_buf[ex_buf_xptr];
		  INCR (name_bf_ptr);
		  INCR (ex_buf_xptr);
		  token_starting = FALSE;
		END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 394 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

		break;
	    END
	    break;
	END
      END
      name_tok[num_tokens] = name_bf_ptr;
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 387 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	395
 * ~~~~~~~~~~~~~~~~~~~
 * Here we set all the pointers for the various parts of the name,
 * depending on which of the three possible syntaxes this name uses.
 ***************************************************************************/
    BEGIN
      if (num_commas == 0)
      BEGIN
	first_start = 0;
	last_end = num_tokens;
	jr_end = last_end;

  /***************************************************************************
   * WEB section number:	396
   * ~~~~~~~~~~~~~~~~~~~
   * When there are no brace-level-0 |comma|s in the name, the von name
   * starts with the first nonlast token whose first brace-level-0 letter
   * is in lower case (for the purposes of this determination, an accented
   * or foreign character at brace-level-1 that's in lower case will do, as
   * well).  A module following this one determines where the von name ends
   * and the last starts.
   ***************************************************************************/
	BEGIN
	  von_start = 0;
	  while (von_start < (last_end - 1))
	  BEGIN
	    name_bf_ptr = name_tok[von_start];
	    name_bf_xptr = name_tok[von_start + 1];
	    if (von_token_found ())
	    BEGIN
	      von_name_ends_and_last_name_sta ();
	      goto Von_Found_Label;
	    END
	    INCR (von_start);
	  END
	  while (von_start > 0)
	  BEGIN
	    if ((lex_class[name_sep_char[von_start]] != SEP_CHAR)
		  || (name_sep_char[von_start] == TIE))
	    BEGIN
	      goto Loop2_Exit_Label;
	    END
	    DECR (von_start);
	  END
  Loop2_Exit_Label:  von_end = von_start;
  Von_Found_Label:  first_end = von_start;
	END
  /*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 396 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

      END
      else if (num_commas == 1)
      BEGIN
	von_start = 0;
	last_end = comma1;
	jr_end = last_end;
	first_start = jr_end;
	first_end = num_tokens;
	von_name_ends_and_last_name_sta ();
      END
      else if (num_commas == 2)
      BEGIN
	von_start = 0;
	last_end = comma1;
	jr_end = comma2;
	first_start = jr_end;
	first_end = num_tokens;
	von_name_ends_and_last_name_sta ();
      END
      else
      BEGIN
	CONFUSION ("Illegal number of comma,s");
      END
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 395 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    ex_buf_length = 0;
    add_buf_pool (pop_lit1);
    figure_out_the_formatted_name ();
    add_pool_buf_and_push ();
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 382 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 354
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function := pops the top two literals and assigns
 * to the first (which must be an |int_entry_var|, a |str_entry_var|, an
 * |int_global_var|, or a |str_global_var|) the value of the second;
 * it complains if the value isn't of the appropriate type.
 ***************************************************************************/
void          x_gets (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  pop_lit_stk (&pop_lit2, &pop_typ2);
  if (pop_typ1 != STK_FN)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_FN);
  END
  else if (( ! mess_with_entries)
	    && ((fn_type[pop_lit1] == STR_ENTRY_VAR)
		    || (fn_type[pop_lit1] == INT_ENTRY_VAR)))
  BEGIN
    bst_cant_mess_with_entries_prin ();
  END
  else
  BEGIN
    switch (fn_type[pop_lit1])
    BEGIN
      case INT_ENTRY_VAR:

/***************************************************************************
 * WEB section number:	355
 * ~~~~~~~~~~~~~~~~~~~
 * This module checks that what we're about to assign is really an
 * integer, and then assigns.
 ***************************************************************************/
        if (pop_typ2 != STK_INT)
        BEGIN
          print_wrong_stk_lit (pop_lit2, pop_typ2, STK_INT);
        END
        else
        BEGIN
          entry_ints[(cite_ptr * num_ent_ints) + FN_INFO[pop_lit1]] = pop_lit2;
        END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 355 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

        break;
      case STR_ENTRY_VAR:

/***************************************************************************
 * WEB section number:	357
 * ~~~~~~~~~~~~~~~~~~~
 * This module checks that what we're about to assign is really a
 * string, and then assigns.
 ***************************************************************************/
	BEGIN
	  if (pop_typ2 != STK_STR)
	  BEGIN
	    print_wrong_stk_lit (pop_lit2, pop_typ2, STK_STR);
	  END
	  else
	  BEGIN
	    str_ent_ptr = (cite_ptr * num_ent_strs) + FN_INFO[pop_lit1];
	    ent_chr_ptr = 0;
	    sp_ptr = str_start[pop_lit2];
	    sp_xptr1 = str_start[pop_lit2 + 1];
	    if ((sp_xptr1 - sp_ptr) > Ent_Str_Size)
	    BEGIN
	      BST_STRING_SIZE_EXCEEDED (Ent_Str_Size, ", the entry");
	      sp_xptr1 = sp_ptr + Ent_Str_Size;
	    END
	    while (sp_ptr < sp_xptr1)
	    BEGIN
	      ENTRY_STRS(str_ent_ptr, ent_chr_ptr) = str_pool[sp_ptr];
	      INCR (ent_chr_ptr);
	      INCR (sp_ptr);
	    END
	    ENTRY_STRS(str_ent_ptr, ent_chr_ptr) = END_OF_STRING;
	  END
	END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 357 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

        break;
      case INT_GLOBAL_VAR:

/***************************************************************************
 * WEB section number:	358
 * ~~~~~~~~~~~~~~~~~~~
 * This module checks that what we're about to assign is really an
 * integer, and then assigns.
 ***************************************************************************/
        if (pop_typ2 != STK_INT)
        BEGIN
          print_wrong_stk_lit (pop_lit2, pop_typ2, STK_INT);
        END
        else
        BEGIN
          FN_INFO[pop_lit1] = pop_lit2;
        END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 358 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

        break;
      case STR_GLOBAL_VAR:

/***************************************************************************
 * WEB section number:	359
 * ~~~~~~~~~~~~~~~~~~~
 * This module checks that what we're about to assign is really a
 * string, and then assigns.
 ***************************************************************************/
	BEGIN
	  if (pop_typ2 != STK_STR)
	  BEGIN
	    print_wrong_stk_lit (pop_lit2, pop_typ2, STK_STR);
	  END
	  else
	  BEGIN
	    str_glb_ptr = FN_INFO[pop_lit1];
	    if (pop_lit2 < cmd_str_ptr)
	    BEGIN
	      glb_str_ptr[str_glb_ptr] = pop_lit2;
	    END
	    else
	    BEGIN
	      glb_str_ptr[str_glb_ptr] = 0;
	      glob_chr_ptr = 0;
	      sp_ptr = str_start[pop_lit2];
	      sp_end = str_start[pop_lit2 + 1];
	      if ((sp_end - sp_ptr) > Glob_Str_Size)
	      BEGIN
		BST_STRING_SIZE_EXCEEDED (Glob_Str_Size, ", the global");
		sp_end = sp_ptr + Glob_Str_Size;
	      END
	      while (sp_ptr < sp_end)
	      BEGIN
		GLOBAL_STRS(str_glb_ptr, glob_chr_ptr) = str_pool[sp_ptr];
		INCR (glob_chr_ptr);
		INCR (sp_ptr);
	      END
	      glb_str_end[str_glb_ptr] = glob_chr_ptr;
	    END
	  END
	END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 359 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

        break;
      default:
        PRINT ("You can't assign to type ");
        print_fn_class (pop_lit1);
        BST_EX_WARN (", a nonvariable function class");
        break;
    END
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 354 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 346
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function > pops the top two (integer) literals,
 * compares them, and pushes the integer 1 if the second is greater than
 * the first, 0 otherwise.  If either isn't an integer literal, it
 * complains and pushes the integer 0.
 ***************************************************************************/
void          x_greater_than (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  pop_lit_stk (&pop_lit2, &pop_typ2);
  if (pop_typ1 != STK_INT)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_INT);
    push_lit_stk (0, STK_INT);
  END
  else if (pop_typ2 != STK_INT)
  BEGIN
    print_wrong_stk_lit (pop_lit2, pop_typ2, STK_INT);
    push_lit_stk (0, STK_INT);
  END
  else if (pop_lit2 > pop_lit1)
  BEGIN
    push_lit_stk (1, STK_INT);
  END
  else
  BEGIN
    push_lit_stk (0, STK_INT);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 346 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 422
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function int.to.chr$ pops the top (integer)
 * literal, interpreted as the |ASCII_code| of a single character,
 * converts it to the corresponding single-character string, and pushes
 * this string.  If the literal isn't an appropriate integer, it
 * complains and pushes the null string.
 ***************************************************************************/
void          x_int_to_chr (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  if (pop_typ1 != STK_INT)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_INT);
    push_lit_stk (s_null, STK_STR);
  END
#if UTF_8
  else if ((pop_lit1 < 0) || (pop_lit1 > LAST_UCS_CHAR))
#else
  else if ((pop_lit1 < 0) || (pop_lit1 > LAST_ASCII_CHAR))
#endif
  BEGIN
    BST_EX_WARN2 ("%ld isn't valid character code", (long) pop_lit1);
    push_lit_stk (s_null, STK_STR);
  END
  else
  BEGIN
    STR_ROOM (1);
#if UTF_8
    BEGIN
      UChar ch0[3] = {0};
      unsigned char ch1[5] = {0}, *ch;
      if (pop_lit1> 0xFFFF)
      BEGIN
        ch0[0] = U16_LEAD(pop_lit1);
        ch0[1] = U16_TRAIL(pop_lit1);
      END
      else
        ch0[0] = pop_lit1;
      icu_fromUChars(ch1, 5, ch0, 3);
      ch=ch1;
      while(*ch)
      BEGIN
        APPEND_CHAR (*ch);
        INCR (ch);
      END
    END
#else
    APPEND_CHAR (pop_lit1);
#endif
    push_lit_stk (make_string (), STK_STR);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 422 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 423
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function int.to.str$ pops the top (integer)
 * literal, converts it to its (unique) string equivalent, and pushes
 * this string.  If the literal isn't an integer, it complains and pushes
 * the null string.
 ***************************************************************************/
void          x_int_to_str (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  if (pop_typ1 != STK_INT)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_INT);
    push_lit_stk (s_null, STK_STR);
  END
  else
  BEGIN
    int_to_ASCII (pop_lit1, ex_buf, 0, &ex_buf_length);
    add_pool_buf_and_push ();
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 423 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 347
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function < pops the top two (integer) literals,
 * compares them, and pushes the integer 1 if the second is less than the
 * first, 0 otherwise.  If either isn't an integer literal, it complains
 * and pushes the integer 0.
 ***************************************************************************/
void          x_less_than (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  pop_lit_stk (&pop_lit2, &pop_typ2);
  if (pop_typ1 != STK_INT)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_INT);
    push_lit_stk (0, STK_INT);
  END
  else if (pop_typ2 != STK_INT)
  BEGIN
    print_wrong_stk_lit (pop_lit2, pop_typ2, STK_INT);
    push_lit_stk (0, STK_INT);
  END
  else if (pop_lit2 < pop_lit1)
  BEGIN
    push_lit_stk (1, STK_INT);
  END
  else
  BEGIN
    push_lit_stk (0, STK_INT);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 347 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 349
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function - pops the top two (integer) literals
 * and pushes their difference (the first subtracted from the second).
 * If either isn't an integer literal, it complains and pushes the
 * integer 0.
 ***************************************************************************/
void          x_minus (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  pop_lit_stk (&pop_lit2, &pop_typ2);
  if (pop_typ1 != STK_INT)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_INT);
    push_lit_stk (0, STK_INT);
  END
  else if (pop_typ2 != STK_INT)
  BEGIN
    print_wrong_stk_lit (pop_lit2, pop_typ2, STK_INT);
    push_lit_stk (0, STK_INT);
  END
  else
  BEGIN
    push_lit_stk (pop_lit2 - pop_lit1, STK_INT);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 349 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 424
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function missing$ pops the top literal and
 * pushes the integer 1 if it's a missing field, 0 otherwise.  If the
 * literal isn't a missing field or a string, it complains and pushes 0.
 * Unlike empty$ , this function should be called only when
 * |mess_with_entries| is true.
 ***************************************************************************/
void          x_missing (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  if ( ! mess_with_entries)
  BEGIN
    bst_cant_mess_with_entries_prin ();
  END
  else if ((pop_typ1 != STK_STR) && (pop_typ1 != STK_FIELD_MISSING))
  BEGIN
    if (pop_typ1 != STK_EMPTY)
    BEGIN
      print_stk_lit (pop_lit1, pop_typ1);
      BST_EX_WARN (", not a string or missing field,");
    END
    push_lit_stk (0, STK_INT);
  END
  else if (pop_typ1 == STK_FIELD_MISSING)
  BEGIN
    push_lit_stk (1, STK_INT);
  END
  else
  BEGIN
    push_lit_stk (0, STK_INT);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 424 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 426
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function num.names$  pops the top (string)
 * literal; it pushes the number of names the string represents---one
 * plus the number of occurrences of the substring ``and'' (ignoring case
 * differences) surrounded by nonnull |white_space| at the top brace
 * level.  If the literal isn't a string, it complains and pushes the
 * value 0.
 ***************************************************************************/
void          x_num_names (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  if (pop_typ1 != STK_STR)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_STR);
    push_lit_stk (0, STK_INT);
  END
  else
  BEGIN
    ex_buf_length = 0;
    add_buf_pool (pop_lit1);

/***************************************************************************
 * WEB section number:	427
 * ~~~~~~~~~~~~~~~~~~~
 * This module, while scanning the list of names, counts the occurrences
 * of ``and'' (ignoring case differences) surrounded by nonnull
 * |white_space|, and adds 1.
 ***************************************************************************/
    BEGIN
      ex_buf_ptr = 0;
      num_names = 0;
      while (ex_buf_ptr < ex_buf_length)
      BEGIN
        name_scan_for_and (pop_lit1);
        INCR (num_names);
      END
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 427 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    push_lit_stk (num_names, STK_INT);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 426 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 348
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function + pops the top two (integer) literals
 * and pushes their sum.  If either isn't an integer literal, it
 * complains and pushes the integer 0.
 ***************************************************************************/
void          x_plus (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  pop_lit_stk (&pop_lit2, &pop_typ2);
  if (pop_typ1 != STK_INT)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_INT);
    push_lit_stk (0, STK_INT);
  END
  else if (pop_typ2 != STK_INT)
  BEGIN
    print_wrong_stk_lit (pop_lit2, pop_typ2, STK_INT);
    push_lit_stk (0, STK_INT);
  END
  else
  BEGIN
    push_lit_stk (pop_lit2 + pop_lit1, STK_INT);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 348 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 429
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function preamble$ pushes onto the stack the
 * concatenation of all the preamble strings read from the database
 * files.
 ***************************************************************************/
void          x_preamble (void)
BEGIN
  ex_buf_length = 0;
  preamble_ptr = 0;
  while (preamble_ptr < num_preamble_strings)
  BEGIN
    add_buf_pool (s_preamble[preamble_ptr]);
    INCR (preamble_ptr);
  END
  add_pool_buf_and_push ();
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 429 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 430
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function purify$ pops the top (string) literal,
 * removes nonalphanumeric characters except for |white_space| and
 * |sep_char| characters (these get converted to a |space|) and removes
 * certain alphabetic characters contained in the control sequences
 * associated with a special character, and pushes the resulting string.
 * If the literal isn't a string, it complains and pushes the null
 * string.
 ***************************************************************************/
void          x_purify (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  if (pop_typ1 != STK_STR)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_STR);
    push_lit_stk (s_null, STK_STR);
  END
  else
  BEGIN
    ex_buf_length = 0;
    add_buf_pool (pop_lit1);

/***************************************************************************
 * WEB section number:	431
 * ~~~~~~~~~~~~~~~~~~~
 * The resulting string has nonalphanumeric characters removed, and each
 * |white_space| or |sep_char| character converted to a |space|.  The next
 * module handles special characters.  This code doesn't complain if the
 * string isn't brace balanced.
 ***************************************************************************/
    BEGIN
      brace_level = 0;
      ex_buf_xptr = 0;
      ex_buf_ptr = 0;
      while (ex_buf_ptr < ex_buf_length)
      BEGIN
        switch (lex_class[ex_buf[ex_buf_ptr]])
        BEGIN
          case WHITE_SPACE:
          case SEP_CHAR:
	    BEGIN
              ex_buf[ex_buf_xptr] = SPACE;
              INCR (ex_buf_xptr);
            END
	    break;
          case ALPHA:
          case NUMERIC:
            BEGIN
#ifdef UTF_8
/*
When we processe the character UTF-8, the length has been changed. This focntion is used in
quick_sort.                                                                   23/sep/2009
*/
              DO_UTF8(ex_buf[ex_buf_ptr],
                ex_buf[ex_buf_xptr] = ex_buf[ex_buf_ptr];
                ex_buf_xptr += 1,
                ex_buf[ex_buf_xptr] = ex_buf[ex_buf_ptr];
                ex_buf[ex_buf_xptr+1] = ex_buf[ex_buf_ptr+1];
                ex_buf_xptr += 2; ex_buf_ptr += 1,
                ex_buf[ex_buf_xptr] = ex_buf[ex_buf_ptr];
                ex_buf[ex_buf_xptr+1] = ex_buf[ex_buf_ptr+1];
                ex_buf[ex_buf_xptr+2] = ex_buf[ex_buf_ptr+2];
                ex_buf_xptr += 3; ex_buf_ptr += 2,
                ex_buf[ex_buf_xptr] = ex_buf[ex_buf_ptr];
                ex_buf[ex_buf_xptr+1] = ex_buf[ex_buf_ptr+1];
                ex_buf[ex_buf_xptr+2] = ex_buf[ex_buf_ptr+2];
                ex_buf[ex_buf_xptr+3] = ex_buf[ex_buf_ptr+3];
                ex_buf_xptr += 4; ex_buf_ptr += 3);
#else
              ex_buf[ex_buf_xptr] = ex_buf[ex_buf_ptr];
              INCR (ex_buf_xptr);
#endif
            END
            break;
          default:
            if (ex_buf[ex_buf_ptr] == LEFT_BRACE)
            BEGIN
              INCR (brace_level);
              if ((brace_level == 1) && ((ex_buf_ptr + 1) < ex_buf_length))
              BEGIN
	        if (ex_buf[ex_buf_ptr + 1] == BACKSLASH)

/***************************************************************************
 * WEB section number:	432
 * ~~~~~~~~~~~~~~~~~~~
 * Special characters (even without a matching |right_brace|) are
 * purified by removing the control sequences (but restoring the correct
 * thing for `\i' and `\j' as well as the eleven alphabetic
 * foreign characters in Table~3.2 of the \LaTeX\ manual) and removing
 * all nonalphanumeric characters (including |white_space| and
 * |sep_char|s).
 ***************************************************************************/
                BEGIN
                  INCR (ex_buf_ptr);
                  while ((ex_buf_ptr < ex_buf_length) && (brace_level > 0))
                  BEGIN
                    INCR (ex_buf_ptr);
                    ex_buf_yptr = ex_buf_ptr;
                    while ((ex_buf_ptr < ex_buf_length)
			    && (lex_class[ex_buf[ex_buf_ptr]] == ALPHA))
                    BEGIN
                      INCR (ex_buf_ptr);
                    END
                    control_seq_loc = str_lookup (ex_buf, ex_buf_yptr,
						  ex_buf_ptr - ex_buf_yptr,
						  CONTROL_SEQ_ILK,
						  DONT_INSERT);
                    if (hash_found)

/***************************************************************************
 * WEB section number:	433
 * ~~~~~~~~~~~~~~~~~~~
 * We consider the purified character to be either the first alphabetic
 * character of its control sequence, or perhaps both alphabetic
 * characters.
 ***************************************************************************/
                    BEGIN
                      ex_buf[ex_buf_xptr] = ex_buf[ex_buf_yptr];
                      INCR (ex_buf_xptr);
                      switch (ilk_info[control_seq_loc])
                      BEGIN
                        case N_OE:
                        case N_OE_UPPER:
                        case N_AE:
                        case N_AE_UPPER:
                        case N_SS:
                          BEGIN
			    ex_buf[ex_buf_xptr] = ex_buf[ex_buf_yptr + 1];
                            INCR (ex_buf_xptr);
			  END
                          break;
                        default:
                          DO_NOTHING;
                          break;
                      END
                    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 433 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

                    while ((ex_buf_ptr < ex_buf_length) && (brace_level > 0)
			    && (ex_buf[ex_buf_ptr] != BACKSLASH))
                    BEGIN
                      switch (lex_class[ex_buf[ex_buf_ptr]])
                      BEGIN
                        case ALPHA:
                        case NUMERIC:
                          BEGIN
			    ex_buf[ex_buf_xptr] = ex_buf[ex_buf_ptr];
                            INCR (ex_buf_xptr);
                          END
			  break;
                        default:
                          if (ex_buf[ex_buf_ptr] == RIGHT_BRACE)
                          BEGIN
                            DECR (brace_level);
                          END
                          else if (ex_buf[ex_buf_ptr] == LEFT_BRACE)
                          BEGIN
                            INCR (brace_level);
                          END
                          break;
                      END
                      INCR (ex_buf_ptr);
                    END
                  END
                  DECR (ex_buf_ptr);
                END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 432 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

              END
            END
            else if (ex_buf[ex_buf_ptr] == RIGHT_BRACE)
            BEGIN
              if (brace_level > 0)
              BEGIN
                DECR (brace_level);
              END
            END
            break;
        END
        INCR (ex_buf_ptr);
      END
      ex_buf_length = ex_buf_xptr;
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 431 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    add_pool_buf_and_push ();
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 430 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 434
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function quote$ pushes the string consisting of
 * the |double_quote| character.
 ***************************************************************************/
void          x_quote (void)
BEGIN
  STR_ROOM (1);
  APPEND_CHAR (DOUBLE_QUOTE);
  push_lit_stk (make_string (), STK_STR);
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 434 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/***************************************************************************
 * WEB section number:	 437
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function substring$ pops the top three literals
 * (they are the two integers literals |pop_lit1| and |pop_lit2| and a
 * string literal, in that order).  It pushes the substring of the (at
 * most) |pop_lit1| consecutive characters starting at the |pop_lit2|th
 * character (assuming 1-based indexing) if |pop_lit2| is positive, and
 * ending at the |-pop_lit2|th character from the end if |pop_lit2| is
 * negative (where the first character from the end is the last
 * character).  If any of the types is incorrect, it complain and pushes
 * the null string.
 *
 * Note: If |sp_length| is compared with a signed quantity (e.g. pop_lit2),
 * must be first cast to |Integer_T| because it is an UNSIGNED variable.
 ***************************************************************************/
void          x_substring (void)
BEGIN

  pop_lit_stk (&pop_lit1, &pop_typ1);
  pop_lit_stk (&pop_lit2, &pop_typ2);
  pop_lit_stk (&pop_lit3, &pop_typ3);
  if (pop_typ1 != STK_INT)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_INT);
    push_lit_stk (s_null, STK_STR);
  END
  else if (pop_typ2 != STK_INT)
  BEGIN
    print_wrong_stk_lit (pop_lit2, pop_typ2, STK_INT);
    push_lit_stk (s_null, STK_STR);
  END
  else if (pop_typ3 != STK_STR)
  BEGIN
    print_wrong_stk_lit (pop_lit3, pop_typ3, STK_STR);
    push_lit_stk (s_null, STK_STR);
  END
  else
  BEGIN
#ifdef UTF_8
/*
This is a new code of x_substring for processing the character UTF-8.
We transform the character to Unicode and then get the substring, then 
back to UTF-8. 23/sep/2009
*/
    Integer_T str_length = LENGTH (pop_lit3);
    UChar32 uchs[BUF_SIZE+1];
    UChar uch16[BUF_SIZE+1];
    int32_t utcap = BUF_SIZE+1;
    int32_t ulen;
    unsigned char frUch1[BUF_SIZE+1];
    unsigned char frUch2[BUF_SIZE+1];
    int32_t frUchCap = BUF_SIZE + 1;
    int32_t lenfrUch;
    int32_t ptrfrUch;

    ulen = icu_toUChar32s(str_pool,str_start[pop_lit3],str_length,uchs,utcap,uch16);
    sp_length = ulen;
#else
    sp_length = LENGTH (pop_lit3);
#endif
    if (pop_lit1 >= (Integer_T) sp_length)
    BEGIN
      if ((pop_lit2 == 1) || (pop_lit2 == -1))
      BEGIN
        REPUSH_STRING;
        goto Exit_Label;
      END
    END

    if ((pop_lit1 <= 0) || (pop_lit2 == 0)
            || (pop_lit2 > (Integer_T) sp_length)
            || (pop_lit2 < -(Integer_T) sp_length))
    BEGIN
      push_lit_stk (s_null, STK_STR);
      goto Exit_Label;
    END
    else

/***************************************************************************
 * WEB section number:	438
 * ~~~~~~~~~~~~~~~~~~~
 * This module finds the substring as described in the last section,
 * and slides it into place in the string pool, if necessary.
 ***************************************************************************/
    BEGIN
      if (pop_lit2 > 0)
      BEGIN
        if (pop_lit1 > (sp_length - (pop_lit2 - 1)))
        BEGIN
          pop_lit1 = sp_length - (pop_lit2 - 1);
        END
#ifdef UTF_8
        lenfrUch = icu_fromUChar32s(frUch1, frUchCap, &uchs[pop_lit2-1], pop_lit1, uch16);
        ptrfrUch = icu_fromUChar32s(frUch2, frUchCap, uchs, pop_lit2-1, uch16);
        sp_ptr = str_start[pop_lit3] + ptrfrUch;
        sp_end = sp_ptr + lenfrUch;
#else
        sp_ptr = str_start[pop_lit3] + (pop_lit2 - 1);
        sp_end = sp_ptr + pop_lit1;
#endif
        if (pop_lit2 == 1)
        BEGIN
          if (pop_lit3 >= cmd_str_ptr)
          BEGIN
            str_start[pop_lit3 + 1] = sp_end;
            UNFLUSH_STRING;
            INCR (lit_stk_ptr);
            goto Exit_Label;
          END
        END
      END
      else
      BEGIN
        pop_lit2 = -pop_lit2;
        if (pop_lit1 > (Integer_T) (sp_length - (pop_lit2 - 1)))
        BEGIN
          pop_lit1 = sp_length - (pop_lit2 - 1);
        END
#ifdef UTF_8
        lenfrUch = icu_fromUChar32s(frUch1, frUchCap, &uchs[ulen - (pop_lit2-1) - pop_lit1], pop_lit1, uch16);
        ptrfrUch = icu_fromUChar32s(frUch2, frUchCap, uchs, ulen - (pop_lit2-1) - pop_lit1, uch16);
        sp_ptr = str_start[pop_lit3] + ptrfrUch;
        sp_end = sp_ptr + lenfrUch;
#else
        sp_end = str_start[pop_lit3 + 1] - (pop_lit2 - 1);
        sp_ptr = sp_end - pop_lit1;
#endif
      END
      STR_ROOM (sp_end - sp_ptr);
      while (sp_ptr < sp_end)
      BEGIN
        APPEND_CHAR (str_pool[sp_ptr]);
        INCR (sp_ptr);
      END
      push_lit_stk (make_string (), STK_STR);
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 438 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  END
Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 437 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 439
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function swap$ pops the top two literals from
 * the stack and pushes them back swapped.
 ***************************************************************************/
void          x_swap (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  pop_lit_stk (&pop_lit2, &pop_typ2);
  if ((pop_typ1 != STK_STR) || (pop_lit1 < cmd_str_ptr))
  BEGIN
    push_lit_stk (pop_lit1, pop_typ1);
    if ((pop_typ2 == STK_STR) && (pop_lit2 >= cmd_str_ptr))
    BEGIN
      UNFLUSH_STRING;
    END
    push_lit_stk (pop_lit2, pop_typ2);
  END
  else if ((pop_typ2 != STK_STR) || (pop_lit2 < cmd_str_ptr))
  BEGIN
    UNFLUSH_STRING;
    push_lit_stk (pop_lit1, STK_STR);
    push_lit_stk (pop_lit2, pop_typ2);
  END
  else

/***************************************************************************
 * WEB section number:	440
 * ~~~~~~~~~~~~~~~~~~~
 * We have to swap both (a)~the strings at the end of the string pool,
 * and (b)~their pointers on the literal stack.
 ***************************************************************************/
  BEGIN
    ex_buf_length = 0;
    add_buf_pool (pop_lit2);
    sp_ptr = str_start[pop_lit1];
    sp_end = str_start[pop_lit1 + 1];
    while (sp_ptr < sp_end)
    BEGIN
      APPEND_CHAR (str_pool[sp_ptr]);
      INCR (sp_ptr);
    END
    push_lit_stk (make_string (), STK_STR);
    add_pool_buf_and_push ();
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 440 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 439 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 441
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function text.length$ pops the top (string)
 * literal, and pushes the number of text characters it contains, where
 * an accented character (more precisely, a ``special character''$\!$,
 * defined earlier) counts as a single text character, even if it's
 * missing its matching |right_brace|, and where braces don't count as
 * text characters.  If the literal isn't a string, it complains and
 * pushes the null string.
 ***************************************************************************/
void          x_text_length (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  if (pop_typ1 != STK_STR)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_STR);
    push_lit_stk (s_null, STK_STR);
  END
  else
  BEGIN
    num_text_chars = 0;

/***************************************************************************
 * WEB section number:	442
 * ~~~~~~~~~~~~~~~~~~~
 * Here we determine the number of text characters in the string, where
 * an entire special character counts as a single text character (even if
 * it's missing its matching |right_brace|), and where braces don't count
 * as text characters.
 ***************************************************************************/
    BEGIN
      sp_ptr = str_start[pop_lit1];
      sp_end = str_start[pop_lit1 + 1];
      sp_brace_level = 0;
      while (sp_ptr < sp_end)
      BEGIN
        INCR (sp_ptr);
        if (str_pool[sp_ptr - 1] == LEFT_BRACE)
        BEGIN
          INCR (sp_brace_level);
          if ((sp_brace_level == 1) && (sp_ptr < sp_end))
          BEGIN
            if (str_pool[sp_ptr] == BACKSLASH)
            BEGIN
              INCR (sp_ptr);
              while ((sp_ptr < sp_end) && (sp_brace_level > 0))
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
#ifdef UTF_8
/*
The length of character of UTF-8 is different. 23/sep/2009
*/
              DO_UTF8(str_pool[sp_ptr-1], , sp_ptr++, sp_ptr += 2, sp_ptr += 3);
#endif
              INCR (num_text_chars);
            END
          END
        END
        else if (str_pool[sp_ptr - 1] == RIGHT_BRACE)
        BEGIN
          if (sp_brace_level > 0)
          BEGIN
            DECR (sp_brace_level);
          END
        END
        else
        BEGIN
#ifdef UTF_8
/*
The same for the length of character. 23/sep/2009
*/
          DO_UTF8(str_pool[sp_ptr-1], , sp_ptr++, sp_ptr += 2, sp_ptr += 3);
#endif
          INCR (num_text_chars);
        END
      END
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 442 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    push_lit_stk (num_text_chars, STK_INT);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 441 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 443
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function text.prefix$ pops the top two literals
 * (the integer literal |pop_lit1| and a string literal, in that order).
 * It pushes the substring of the (at most) |pop_lit1| consecutive text
 * characters starting from the beginning of the string.  This function
 * is similar to substring$ , but this one considers an accented
 * character (or more precisely, a ``special character''$\!$, even if
 * it's missing its matching |right_brace|) to be a single text character
 * (rather than however many |ASCII_code| characters it actually
 * comprises), and this function doesn't consider braces to be text
 * characters; furthermore, this function appends any needed matching
 * |right_brace|s.  If any of the types is incorrect, it complains and
 * pushes the null string.
 ***************************************************************************/
void          x_text_prefix (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  pop_lit_stk (&pop_lit2, &pop_typ2);
  if (pop_typ1 != STK_INT)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_INT);
    push_lit_stk (s_null, STK_STR);
  END
  else if (pop_typ2 != STK_STR)
  BEGIN
    print_wrong_stk_lit (pop_lit2, pop_typ2, STK_STR);
    push_lit_stk (s_null, STK_STR);
  END
  else if (pop_lit1 <= 0)
  BEGIN
    push_lit_stk (s_null, STK_STR);
    goto Exit_Label;
  END
  else

/***************************************************************************
 * WEB section number:	444
 * ~~~~~~~~~~~~~~~~~~~
 * This module finds the prefix as described in the last section, and
 * appends any needed matching |right_brace|s.
 ***************************************************************************/
  BEGIN
    sp_ptr = str_start[pop_lit2];
    sp_end = str_start[pop_lit2 + 1];

/***************************************************************************
 * WEB section number:	445
 * ~~~~~~~~~~~~~~~~~~~
 * This section scans |pop_lit1| text characters, where an entire special
 * character counts as a single text character (even if it's missing its
 * matching |right_brace|), and where braces don't count as text
 * characters.
 ***************************************************************************/
    BEGIN
      num_text_chars = 0;
      sp_brace_level = 0;
      sp_xptr1 = sp_ptr;
      while ((sp_xptr1 < sp_end) && (num_text_chars < pop_lit1))
      BEGIN
        INCR (sp_xptr1);
        if (str_pool[sp_xptr1 - 1] == LEFT_BRACE)
        BEGIN
          INCR (sp_brace_level);
          if ((sp_brace_level == 1) && (sp_xptr1 < sp_end))
          BEGIN
            if (str_pool[sp_xptr1] == BACKSLASH)
            BEGIN
              INCR (sp_xptr1);
              while ((sp_xptr1 < sp_end) && (sp_brace_level > 0))
              BEGIN
                if (str_pool[sp_xptr1] == RIGHT_BRACE)
                BEGIN
                  DECR (sp_brace_level);
                END
                else if (str_pool[sp_xptr1] == LEFT_BRACE)
                BEGIN
                  INCR (sp_brace_level);
                END
                INCR (sp_xptr1);
              END
#ifdef UTF_8
/*
The same for the length of character UTF-8. 23/sep/2009
*/
              DO_UTF8(str_pool[sp_xptr1-1], , sp_xptr1++, sp_xptr1 += 2, sp_xptr1 += 3);
#endif
              INCR (num_text_chars);
            END
          END
        END
        else if (str_pool[sp_xptr1 - 1] == RIGHT_BRACE)
        BEGIN
          if (sp_brace_level > 0)
          BEGIN
            DECR (sp_brace_level);
          END
        END
        else
        BEGIN
#ifdef UTF_8
/*
The same for the length of character UTF-8. 23/sep/2009
*/
          DO_UTF8(str_pool[sp_xptr1-1], , sp_xptr1++, sp_xptr1 += 2, sp_xptr1 += 3);
#endif
          INCR (num_text_chars);
        END
      END
      sp_end = sp_xptr1;
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 445 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    STR_ROOM (sp_brace_level + sp_end - sp_ptr);
    if (pop_lit2 >= cmd_str_ptr)
    BEGIN
      pool_ptr = sp_end;
    END
    else
    BEGIN
      while (sp_ptr < sp_end)
      BEGIN
	APPEND_CHAR (str_pool[sp_ptr]);
        INCR (sp_ptr);
      END
    END
    while (sp_brace_level > 0)
    BEGIN
      APPEND_CHAR (RIGHT_BRACE);
      DECR (sp_brace_level);
    END
    push_lit_stk (make_string (), STK_STR);
  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 444 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

Exit_Label: DO_NOTHING;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 443 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 447
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function type$ pushes the appropriate string
 * from |type_list| onto the stack (unless either it's |undefined| or
 * |empty|, in which case it pushes the null string).
 ***************************************************************************/
void          x_type (void)
BEGIN
  if ( ! mess_with_entries)
  BEGIN
    bst_cant_mess_with_entries_prin ();
  END
  else if ((type_list[cite_ptr] == UNDEFINED)
	    || (type_list[cite_ptr] == EMPTY))
  BEGIN
    push_lit_stk (s_null, STK_STR);
  END
  else
  BEGIN
    push_lit_stk (hash_text[type_list[cite_ptr]], STK_STR);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 447 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 448
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function warning$ pops the top (string) literal
 * and prints it following a warning message.  This is implemented as a
 * special |built_in| function rather than using the top$ function
 * so that it can |mark_warning|.
 ***************************************************************************/
void          x_warning (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  if (pop_typ1 != STK_STR)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_STR);
  END
  else
  BEGIN
    PRINT ("Warning--");
    print_lit (pop_lit1, pop_typ1);
    mark_warning ();
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 448 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

#ifdef UTF_8
Integer_T    char_width_uni (ASCIICode_T * str)
BEGIN
  UChar32 ch;
  U8_GET_OR_FFFD(str, 0, 0, -1, ch);
  if (ch<=LAST_LATIN_CHAR)
    return ( char_width[ch] );
  else
  BEGIN
    switch ( u_getIntPropertyValue(ch, UCHAR_EAST_ASIAN_WIDTH) )
    BEGIN
    case U_EA_WIDE:
    case U_EA_FULLWIDTH:
        return ( 1000 );
    case U_EA_HALFWIDTH:
        return ( 500 );
    case U_EA_NARROW:
    case U_EA_NEUTRAL:
    case U_EA_AMBIGUOUS:
    default:
        return ( 700 );
    END
  END
END
#endif

/***************************************************************************
 * WEB section number:	 450
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function width$ pops the top (string) literal
 * and pushes the integer that represents its width in units specified by
 * the |char_width| array.  This function takes the literal literally;
 * that is, it assumes each character in the string is to be printed as
 * is, regardless of whether the character has a special meaning to \TeX,
 * except that special characters (even without their |right_brace|s) are
 * handled specially.  If the literal isn't a string, it complains and
 * pushes~0.
 ***************************************************************************/
void          x_width (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  if (pop_typ1 != STK_STR)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_STR);
    push_lit_stk (0, STK_INT);
  END
  else
  BEGIN
    ex_buf_length = 0;
    add_buf_pool (pop_lit1);
    string_width = 0;

/***************************************************************************
 * WEB section number:	451
 * ~~~~~~~~~~~~~~~~~~~
 * We use the natural width for all but special characters, and we
 * complain if the string isn't brace-balanced.
 ***************************************************************************/
    BEGIN
      brace_level = 0;
      ex_buf_ptr = 0;
      while (ex_buf_ptr < ex_buf_length)
      BEGIN
        if (ex_buf[ex_buf_ptr] == LEFT_BRACE)
        BEGIN
          INCR (brace_level);
          if ((brace_level == 1) && ((ex_buf_ptr + 1) < ex_buf_length))
          BEGIN
  	  if (ex_buf[ex_buf_ptr + 1] == BACKSLASH)

/***************************************************************************
 * WEB section number:	452
 * ~~~~~~~~~~~~~~~~~~~
 * We use the natural widths of all characters except that some
 * characters have no width: braces, control sequences (except for the
 * usual 13 accented and foreign characters, whose widths are given in
 * the next module), and |white_space| following control sequences (even
 * a null control sequence).
 ***************************************************************************/
            BEGIN
              INCR (ex_buf_ptr);
              while ((ex_buf_ptr < ex_buf_length) && (brace_level > 0))
              BEGIN
                INCR (ex_buf_ptr);
                ex_buf_xptr = ex_buf_ptr;
                while ((ex_buf_ptr < ex_buf_length)
  			&& (lex_class[ex_buf[ex_buf_ptr]] == ALPHA))
                BEGIN
                  INCR (ex_buf_ptr);
                END
                if ((ex_buf_ptr < ex_buf_length)
		     && (ex_buf_ptr == ex_buf_xptr))
                BEGIN
                  INCR (ex_buf_ptr);
                END
                else
                BEGIN
                  control_seq_loc = str_lookup (ex_buf, ex_buf_xptr,
  					        ex_buf_ptr - ex_buf_xptr,
  					        CONTROL_SEQ_ILK, DONT_INSERT);
                  if (hash_found)

/***************************************************************************
 * WEB section number:	453
 * ~~~~~~~~~~~~~~~~~~~
 * Five of the 13 possibilities resort to special information not present
 * in the |char_width| array; the other eight simply use |char_width|'s
 * information for the first letter of the control sequence.
 ***************************************************************************/
                BEGIN
                    switch (ilk_info[control_seq_loc])
                    BEGIN
                      case N_SS:
                        string_width = string_width + SS_WIDTH;
                        break;
                      case N_AE:
                        string_width = string_width + AE_WIDTH;
                        break;
                      case N_OE:
                        string_width = string_width + OE_WIDTH;
                        break;
                      case N_AE_UPPER:
                        string_width = string_width + UPPER_AE_WIDTH;
                        break;
                      case N_OE_UPPER:
                        string_width = string_width + UPPER_OE_WIDTH;
                        break;
                      default:
                        string_width = string_width
  					+ char_width[ex_buf[ex_buf_xptr]];
                        break;
                    END
                  END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 453 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

                END
                while ((ex_buf_ptr < ex_buf_length)
  			&& (lex_class[ex_buf[ex_buf_ptr]] == WHITE_SPACE))
                BEGIN
                  INCR (ex_buf_ptr);
                END
                while ((ex_buf_ptr < ex_buf_length) && (brace_level > 0)
  			&& (ex_buf[ex_buf_ptr] != BACKSLASH))
                BEGIN
                  if (ex_buf[ex_buf_ptr] == RIGHT_BRACE)
                  BEGIN
                    DECR (brace_level);
                  END
                  else if (ex_buf[ex_buf_ptr] == LEFT_BRACE)
                  BEGIN
                    INCR (brace_level);
                  END
                  else
                  BEGIN
#if UTF_8
                    string_width = string_width
                                     + char_width_uni(&ex_buf[ex_buf_ptr]);
#else
                    string_width = string_width
                                     + char_width[ex_buf[ex_buf_ptr]];
#endif
                  END
#if UTF_8
                  if (utf8len(ex_buf[ex_buf_ptr])>0)
                    ex_buf_ptr = ex_buf_ptr + utf8len(ex_buf[ex_buf_ptr]);
                  else
                    INCR (ex_buf_ptr);
#else
                  INCR (ex_buf_ptr);
#endif
                END
              END
              DECR (ex_buf_ptr);
            END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 452 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

            else
            BEGIN
              string_width = string_width + char_width[LEFT_BRACE];
            END
          END
          else
          BEGIN
            string_width = string_width + char_width[LEFT_BRACE];
          END
        END
        else if (ex_buf[ex_buf_ptr] == RIGHT_BRACE)
        BEGIN
          decr_brace_level (pop_lit1);
          string_width = string_width + char_width[RIGHT_BRACE];
        END
        else
        BEGIN
#if UTF_8
          string_width = string_width + char_width_uni(&ex_buf[ex_buf_ptr]);
#else
          string_width = string_width + char_width[ex_buf[ex_buf_ptr]];
#endif
        END
#if UTF_8
        if (utf8len(ex_buf[ex_buf_ptr])>0)
          ex_buf_ptr = ex_buf_ptr + utf8len(ex_buf[ex_buf_ptr]);
        else
          INCR (ex_buf_ptr);
#else
        INCR (ex_buf_ptr);
#endif
      END
      check_brace_level (pop_lit1);
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 451 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    push_lit_stk (string_width, STK_INT);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 450 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/





/***************************************************************************
 * WEB section number:	 454
 * ~~~~~~~~~~~~~~~~~~~
 * The |built_in| function write$ pops the top (string) literal
 * and writes it onto the output buffer |out_buf| (which will result in
 * stuff being written onto the .bbl file if the buffer fills up).  If
 * the literal isn't a string, it complains but does nothing else.
 ***************************************************************************/
void          x_write (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  if (pop_typ1 != STK_STR)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_STR);
  END
  else
  BEGIN
    add_out_pool (pop_lit1);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 454 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
void          x_bit_and (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  pop_lit_stk (&pop_lit2, &pop_typ2);
  if (pop_typ1 != STK_INT)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_INT);
    push_lit_stk (0, STK_INT);
  END
  else if (pop_typ2 != STK_INT)
  BEGIN
    print_wrong_stk_lit (pop_lit2, pop_typ2, STK_INT);
    push_lit_stk (0, STK_INT);
  END
  else
  BEGIN
    push_lit_stk (pop_lit1 & pop_lit2, STK_INT);
  END
END
void          x_bit_or (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  pop_lit_stk (&pop_lit2, &pop_typ2);
  if (pop_typ1 != STK_INT)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_INT);
    push_lit_stk (0, STK_INT);
  END
  else if (pop_typ2 != STK_INT)
  BEGIN
    print_wrong_stk_lit (pop_lit2, pop_typ2, STK_INT);
    push_lit_stk (0, STK_INT);
  END
  else
  BEGIN
    push_lit_stk (pop_lit1 | pop_lit2, STK_INT);
  END
END
#ifdef UTF_8

#define FULLWIDTH_DIGIT_0    0xFF10
#define FULLWIDTH_DIGIT_9    0xFF19
#define FULLWIDTH_CAPITAL_A  0xFF21
#define FULLWIDTH_CAPITAL_Z  0xFF3A
#define FULLWIDTH_SMALL_A    0xFF41
#define FULLWIDTH_SMALL_Z    0xFF5A
#define HALFWIDTH_KATAKANA_WO         0xFF66
#define HALFWIDTH_KATAKANA_SMALL_TSU  0xFF6F
#define HALFWIDTH_KATAKANA_A          0xFF71
#define HALFWIDTH_KATAKANA_N          0xFF9D

void          x_is_cjk_string (void)
BEGIN
  pop_lit_stk (&pop_lit1, &pop_typ1);
  if (pop_typ1 != STK_STR)
  BEGIN
    print_wrong_stk_lit (pop_lit1, pop_typ1, STK_STR);
    push_lit_stk (-1, STK_INT);
  END
  else
  BEGIN
    ex_buf_length = 0;
    add_buf_pool (pop_lit1);
    string_width = 0;
    BEGIN
      ex_buf_ptr = 0;
      while (ex_buf_ptr < ex_buf_length)
      BEGIN
        UChar32 ch;
        U8_NEXT_OR_FFFD(ex_buf, ex_buf_ptr, -1, ch);
        switch ( ublock_getCode(ch) )
        BEGIN
      /* hanzi */
          case UBLOCK_CJK_UNIFIED_IDEOGRAPHS:
          case UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS:
          case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A:
          case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B:
          case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C:
          case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D:
          case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_E:
          case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_F:
#if defined(U_ICU_VERSION_MAJOR_NUM)
#if U_ICU_VERSION_MAJOR_NUM > 65
          case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_G:
#endif
#if U_ICU_VERSION_MAJOR_NUM > 71
          case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_H:
#endif
#if U_ICU_VERSION_MAJOR_NUM > 73
          case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_I:
#endif
#if U_ICU_VERSION_MAJOR_NUM > 77
          case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_J:
#endif
#endif
            string_width |= 0x001;
            break;
      /* kana */
          case UBLOCK_HIRAGANA:
          case UBLOCK_KATAKANA:
          case UBLOCK_KATAKANA_PHONETIC_EXTENSIONS:
          case UBLOCK_KANA_EXTENDED_A:
#if defined(U_ICU_VERSION_MAJOR_NUM)
#if U_ICU_VERSION_MAJOR_NUM > 69
          case UBLOCK_KANA_EXTENDED_B:
#endif
#if U_ICU_VERSION_MAJOR_NUM > 63
          case UBLOCK_SMALL_KANA_EXTENSION:
#endif
#endif
            string_width |= 0x002;
            break;
      /* hangul */
          case UBLOCK_HANGUL_SYLLABLES:
          case UBLOCK_HANGUL_JAMO:
          case UBLOCK_HANGUL_JAMO_EXTENDED_A:
          case UBLOCK_HANGUL_JAMO_EXTENDED_B:
          case UBLOCK_HANGUL_COMPATIBILITY_JAMO:
            string_width |= 0x004;
            break;
      /* bopomofo */
          case UBLOCK_BOPOMOFO:
          case UBLOCK_BOPOMOFO_EXTENDED:
            string_width |= 0x008;
            break;
          case UBLOCK_HALFWIDTH_AND_FULLWIDTH_FORMS:
      /* Fullwidth ASCII variants  except for U+FF01..FF0F, U+FF1A..FF20, U+FF3B..FF40, U+FF5B..FF5E */
            if (  (FULLWIDTH_DIGIT_0  <=ch && ch<=FULLWIDTH_DIGIT_9  )
               || (FULLWIDTH_CAPITAL_A<=ch && ch<=FULLWIDTH_CAPITAL_Z)
               || (FULLWIDTH_SMALL_A  <=ch && ch<=FULLWIDTH_SMALL_Z  ) )
              string_width |= 0x800;
      /* Halfwidth Katakana variants  except for U+FF65, U+FF70, U+FF9E..FF9F */
            if (  (HALFWIDTH_KATAKANA_WO <=ch && ch<=HALFWIDTH_KATAKANA_SMALL_TSU )
               || (HALFWIDTH_KATAKANA_A  <=ch && ch<=HALFWIDTH_KATAKANA_N  ) )
              string_width |= 0x002;
            break;
      /* miscellaneous */
          case UBLOCK_KANBUN:
          case UBLOCK_KANGXI_RADICALS:
          case UBLOCK_CJK_RADICALS_SUPPLEMENT:
            string_width |= 0x800;
            break;
        END
      END
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 451 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    push_lit_stk (string_width, STK_INT);
  END
END
#endif
