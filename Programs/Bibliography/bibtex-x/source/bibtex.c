/*-
******************************************************************************
******************************************************************************
**
**  MODULE
**
**      $RCSfile: bibtex.c,v $
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
**      This module contains the system-dependent C code for BibTeX.
**      There should NOT be any such code code in any of the bibtex-#.c
**      modules.  Functions defined in this module are:
**
**          a_close
**          a_open_in
**          a_open_out
**          eoln
**          main
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
**      $Log: bibtex.c,v $
**      Revision 3.71  1996/08/18  20:37:06  kempson
**      Official release 3.71 (see HISTORY file for details).
**
**      Revision 3.70  1996/04/08  10:08:40  kempson
**      Final documentation & cosmetic changes for official release 3.70.
**
**      Revision 3.5  1995/09/24  20:44:37  kempson
**      Many changes for final beta test version.
**
**      Revision 3.4  1995/04/09  22:15:42  kempson
**      Placed under RCS control
**
******************************************************************************
******************************************************************************
*/

#define __BIBTEX_C__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef KPATHSEA
#include <kpathsea/config.h>
#include <kpathsea/progname.h>
#endif

#include "sysdep.h"
#include "bibtex.h"
#include "datatype.h"
#include "gblprocs.h"
#include "gblvars.h"
#include "utils.h"
#include "version.h"


/***************************************************************************
 * WEB section number:	 39
 * ~~~~~~~~~~~~~~~~~~~
 * Close the text file.
 ***************************************************************************/
void          a_close (const AlphaFile_T file_pointer)
BEGIN

  close_file (file_pointer);

END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  39 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/***************************************************************************
 * WEB section number:	 38
 * ~~~~~~~~~~~~~~~~~~~
 * Open a text (ASCII) file in the current working directory for input.
 ***************************************************************************/
Boolean_T         a_open_in (AlphaFile_T *file_pointer, 
                             Integer_T search_path)
BEGIN
  *file_pointer = open_ip_file (search_path);

  if (*file_pointer != NULL)
    return TRUE;
  else
    return FALSE;
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  39 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	 38
 * ~~~~~~~~~~~~~~~~~~~
 * Open a text (ASCII) file for output in the current directory.
 ***************************************************************************/
Boolean_T         a_open_out (AlphaFile_T *file_pointer)
BEGIN

  *file_pointer = open_op_file ();

  if (*file_pointer != (AlphaFile_T) NULL)
    return TRUE;
  else
    return FALSE;

END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  39 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/***************************************************************************
 * WEB section number:	  47
 * ~~~~~~~~~~~~~~~~~~~
 * Standard \PASCAL\ says that a file should have |eoln| immediately
 * before |eof|, but \BibTeX\ needs only a weaker restriction: If |eof|
 * occurs in the middle of a line, the system function |eoln| should
 * return a |true| result (even though |f^| will be undefined).
 ***************************************************************************/
Boolean_T         eoln (const AlphaFile_T file_pointer)
BEGIN
  int             c;

  if (feof (file_pointer))
  BEGIN
    return (TRUE);
  END
  c = getc (file_pointer);
  if (c != EOF)
  BEGIN
    (void) ungetc (c, file_pointer);
  END
  if (c == '\n' || c == '\r' || c == EOF)
  BEGIN
    return (TRUE);
  END
  else
  BEGIN
    return (FALSE);
  END
END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  47 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




/***************************************************************************
 * WEB section number:	 10
 * ~~~~~~~~~~~~~~~~~~~
 * This program first reads the .aux file that \LaTeX\ produces,
 *
 *   1. determining which .bib file(s) and .bst file to read and
 *   2. constructing a list of cite keys in order of occurrence.
 *
 * The .aux file may have other .aux files nested
 * within.  Second, it reads and executes the .bst file,
 *
 *   1. determining how and in which order to process the
 * 	database entries in the .bib file(s) corresponding to those cite
 * 	keys in the list (or in some cases, to all the entries in the .bib
 * 	file(s)),
 *   2. determining what text to be output for each
 * 	entry and determining any additional text to be output, and
 *   3.	outputting this text to the .bbl file.
 *
 * In addition, the program sends error messages and other remarks to the
 * |log_file| and terminal.
 ***************************************************************************/
#if defined(MIKTEX)
#  define main MIKTEXCEECALL Main
#endif
int                     main (int argc, char **argv)
BEGIN
    extern Integer8_T       history;
    int			    exit_status;

    /*-
    **------------------------------------------------------------------------
    ** Set |log_file| initially to point to NULL.  All writes to the log
    ** check to see that |log_file| isn't NULL before writing.
    **-----------------------------------------------------------------
    */
    log_file = NULL;

    /*-----------------------------------------------------------------
     * Set up a global label PROGRAM_EXIT which can be used if an
     * error requiring an emergency stop is encountered.
     *-----------------------------------------------------------------*/
    if (setjmp (Exit_Program_Flag) == 1)
    BEGIN
      debug_msg (DBG_MISC, "arrived at setjmp (Exit_Program_Flag)");
      goto Exit_Program_Label;
    END

    /*-----------------------------------------------------------------
     * Set up a global label CLOSE_UP_SHOP which may be used when a
     * fatal error is detected. Used by BIBTEX_OVERFLOW & CONFUSION.
     *-----------------------------------------------------------------*/
    if (setjmp (Close_Up_Shop_Flag) == 1)
    BEGIN
      debug_msg (DBG_MISC, "arrived at setjmp (Close_Up_Shop_Flag)");
      goto Close_Up_Shop_Label;
    END

    number_of_command_line_args = argc;
    command_line_arg_strings = (char **) argv;

#ifdef KPATHSEA
    kpse_set_program_name(argv[0], PROGNAME);
#endif

    history = SPOTLESS;
    parse_cmd_line (argc, argv);

    set_array_sizes ();
    report_search_paths ();

    initialize ();

    if (log_file != NULL) {
        FPRINTF (log_file, "%s\n", BANNER);
        FPRINTF (log_file, "Implementation:  %s\n", IMPLEMENTATION);
        FPRINTF (log_file, "Release version: %s\n\n", VERSION);
    }

#ifdef SUPPORT_8BIT
    /*
    ** Read the CS file and setup the 8-bit character set handling.
    ** Similar initialisation is done in the original initialise(), but
    ** we do it in a separate function to simplify the code.
    */
    c8initialise ();
#endif                          /* SUPPORT_8BIT */

    if (Flag_stats)
        report_bibtex_capacity ();



/***************************************************************************
 * WEB section number:	 110
 * ~~~~~~~~~~~~~~~~~~~
 * We keep reading and processing input lines until none left.  This is part
 * of the main program; hence, because of the |aux_done| label, there's no
 * BEGIN - END pair surrounding the entire module.
 ***************************************************************************/
    PRINT ("The top-level auxiliary file: ");
    print_aux_name ();

    /*-----------------------------------------------------------------
     * Set up a global label AUX_DONE which can be used if an error is
     * encountered while the .AUX file is being processed.
     *-----------------------------------------------------------------*/
    if (setjmp (Aux_Done_Flag) == 1)
    BEGIN
      debug_msg (DBG_MISC, "arrived at setjmp (Aux_Done_Flag)");
      goto Aux_Done_Label;
    END
    LOOP
    BEGIN
        INCR (CUR_AUX_LINE);
        if ( ! input_ln (CUR_AUX_FILE))
        BEGIN
            pop_the_aux_stack ();
        END
        else
        BEGIN
            get_aux_command_and_process ();
        END
    END

Aux_Done_Label:

#ifdef TRACE
    if (Flag_trace)
        TRACE_PR_LN ("Finished reading the auxiliary file(s)");
#endif						/* TRACE */

    last_check_for_aux_errors ();
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 110 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/***************************************************************************
 * WEB section number:	 151
 * ~~~~~~~~~~~~~~~~~~~
 * Here's the outer loop for reading the .bst file - it keeps reading and
 * processing .bst commands until none left.  This is part of the main
 * program; hence, because of the |bst_done| label, there's no conventional
 * BEGIN - END pair surrounding the entire module.
 ***************************************************************************/
    if (bst_str == 0)
    BEGIN
        goto No_Bst_File_Label;
    END
    bst_line_num = 0;
    bbl_line_num = 1;
    buf_ptr2 = last;

    /*-----------------------------------------------------------------
     * Set up a global label BST_DONE which can be used if an error is
     * encountered while the .BST file is being processed.
     *-----------------------------------------------------------------*/
    if (setjmp (Bst_Done_Flag) == 1)
    BEGIN
      debug_msg (DBG_MISC, "arrived at setjmp (Bst_Done_Flag)");
      goto Bst_Done_Label;
    END
    LOOP
    BEGIN
      if (!eat_bst_white_space ())
      BEGIN
	goto Bst_Done_Label;
      END
      get_bst_command_and_process ();
    END
Bst_Done_Label:
    a_close (bst_file);
No_Bst_File_Label:
    a_close (bbl_file);
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 151 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

Close_Up_Shop_Label:

/***************************************************************************
 * WEB section number:	 455
 * ~~~~~~~~~~~~~~~~~~~
 * This section does any last-minute printing and ends the program.
 ***************************************************************************/
    BEGIN
      if ((read_performed) && ( ! reading_completed))
      BEGIN
	PRINT2 ("Aborted at line %ld of file ", (long) bib_line_num);
        print_bib_name ();
      END

      trace_and_stat_printing ();

      /*-
      ** Report on BibTeX memory usage to log file.
      */
      TRACE_PR_NEWLINE;
      TRACE_PR_LN  ("Here's how much of BibTeX's memory you used:");
      TRACE_PR_LN3 (" Cites:            %6ld out of %ld",
                    (long) num_cites, (long) Max_Cites);
      TRACE_PR_LN3 (" Fields:           %6ld out of %ld",
                    (long) field_ptr, (long) Max_Fields);
      TRACE_PR_LN3 (" Hash table:       %6ld out of %ld",
                    (long) hash_used, (long) Hash_Size);
      TRACE_PR_LN3 (" Strings:          %6ld out of %ld",
                    (long) str_ptr, (long) Max_Strings);
      TRACE_PR_LN3 (" Free string pool: %6ld out of %ld",
                    (long) pool_ptr, (long) Pool_Size);
      TRACE_PR_LN3 (" Wizard functions: %6ld out of %ld",
                    (long) wiz_def_ptr, (long) Wiz_Fn_Space);


/***************************************************************************
 * WEB section number:	 466
 * ~~~~~~~~~~~~~~~~~~~
 * Some implementations may wish to pass the |history| value to the
 * operating system so that it can be used to govern whether or not other
 * programs are started.  Here we simply report the history to the user.
 * The appropriate exit status will be passed to the operating system just
 * before exiting.
 ***************************************************************************/
      switch (history)
      BEGIN
        case SPOTLESS:
	  DO_NOTHING;
          break;
        case WARNING_MESSAGE:
          if (err_count == 1)
          BEGIN
	    PRINT_LN ("(There was 1 warning)");
          END
          else
          BEGIN
            PRINT_LN2 ("(There were %ld warnings)", (long) err_count);
          END
          break;
        case ERROR_MESSAGE:
          if (err_count == 1)
          BEGIN
            PRINT_LN ("(There was 1 error message)");
          END
          else
          BEGIN
            PRINT_LN2 ("(There were %ld error messages)", (long) err_count);
          END
          break;
        case FATAL_MESSAGE:
          PRINT_LN ("(That was a fatal error)");
          break;
        default:
          PRINT ("History is bunk");
          print_confusion ();
          break;
        END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 466 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

        a_close (log_file);
    END
/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION 455 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

Exit_Program_Label:

    switch (history)
    BEGIN
        case SPOTLESS:
	    exit_status = NORMAL_EXIT_STATUS;
	    break;
        case WARNING_MESSAGE:
	    exit_status = WARNING_EXIT_STATUS;
            break;
        case ERROR_MESSAGE:
	    exit_status = ERROR_EXIT_STATUS;
            break;
        case FATAL_MESSAGE:
        default:
	    exit_status = FATAL_EXIT_STATUS;
            break;
    END

    exit (exit_status);
    return (exit_status);
END

/*^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF SECTION  10 ^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
