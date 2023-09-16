/*-
******************************************************************************
******************************************************************************
**
**  MODULE
**
**      file: bibtex.h
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
**      This module defines the macros used in the BibTeX WEB source.  
**      This file also contains the definitions that determine BibTeX's 
**      processing capacity.
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
**      $Log: bibtex.h,v $
**      Revision 3.71  1996/08/18  20:47:30  kempson
**      Official release 3.71 (see HISTORY file for details).
**
**      Revision 3.70  1996/04/08  10:08:40  kempson
**      Final documentation & cosmetic changes for official release 3.70.
**
**      Revision 3.5  1995/09/24  20:44:37  kempson
**      Many changes for final beta test version.
**
**      Revision 3.4  1995/04/09  22:15:43  kempson
**      Placed under RCS control
**
******************************************************************************
******************************************************************************
*/
#ifndef __BIBTEX_H__
#define __BIBTEX_H__                1


/***************************************************************************
 * WEB section number:   1
 * ~~~~~~~~~~~~~~~~~~~
 * The BANNER string is defined here and should be changed whenever BibTeX
 * is modified.
 ***************************************************************************/
#ifdef UTF_8
# define BANNER     "This is BibTeXu: a UTF-8 Big BibTeX version 0.99d"
# define PROGNAME "bibtexu"
# define SUPPORT_8BIT
#else
# ifdef SUPPORT_8BIT
#  define BANNER     "This is 8-bit Big BibTeX version 0.99d"
# else
#  define BANNER     "This is Big BibTeX version 0.99d"
# endif
# define PROGNAME "bibtex8"
#endif


/***************************************************************************
 * WEB section number:   2
 * ~~~~~~~~~~~~~~~~~~~
 * Terminal output goes to the file |TERM_OUT|, while terminal input comes
 * from |TERM_IN|.  On our system, these (system-dependent) files are
 * already opened at the beginning of the program.
 ***************************************************************************/
#define TERM_OUT                    stdout
#define TERM_IN                     stdin

/***************************************************************************
 * WEB section number:   3
 * ~~~~~~~~~~~~~~~~~~~
 * This program uses the term |print| instead of |write| when writing on
 * both the |log_file| and (system-dependent) |term_out| file, and it
 * uses |trace_pr| when in |trace| mode, for which it writes on just the
 * |log_file|.  If you want to change where either set of macros writes
 * to, you should also change the other macros in this program for that
 * set; each such macro begins with |print_| or |trace_pr_|.
 ***************************************************************************/
#define PRINT_NEWLINE               print_a_newline ()

#define PRINT(X) \
            {\
                if (log_file != NULL)\
                    FPRINTF (log_file, X);\
                FPRINTF (TERM_OUT, X);\
            }

#define PRINT2(X, Y) \
            {\
                if (log_file != NULL)\
                    FPRINTF (log_file, X, Y);\
                FPRINTF (TERM_OUT, X, Y);\
            }

#define PRINT3(X, Y, Z) \
            {\
                if (log_file != NULL)\
                    FPRINTF (log_file, X, Y, Z);\
                FPRINTF (TERM_OUT, X, Y, Z);\
            }

#define PRINT4(W, X, Y, Z) \
            {\
                if (log_file != NULL)\
                    FPRINTF (log_file, W, X, Y, Z);\
                FPRINTF (TERM_OUT, W, X, Y, Z);\
            }

#define PRINT_LN(X) \
            {\
                if (log_file != NULL) {\
                    FPRINTF (log_file, X); FPUTC ('\n', log_file);\
                }\
                FPRINTF (TERM_OUT, X); FPUTC('\n', TERM_OUT);\
            }

#define PRINT_LN2(X, Y) \
            {\
                if (log_file != NULL) {\
                    FPRINTF (log_file, X, Y); FPUTC ('\n', log_file);\
                }\
                FPRINTF (TERM_OUT, X, Y); FPUTC('\n', TERM_OUT);\
            }

#define PRINT_LN3(X, Y, Z) \
            {\
                if (log_file != NULL) {\
                    FPRINTF (log_file, X, Y, Z); FPUTC ('\n', log_file);\
                }\
                FPRINTF (TERM_OUT, X, Y, Z); FPUTC('\n', TERM_OUT);\
            }

#define PRINT_LN4(W, X, Y, Z) \
            {\
                if (log_file != NULL) {\
                    FPRINTF (log_file, W, X, Y, Z); FPUTC ('\n', log_file);\
                }\
                FPRINTF (TERM_OUT, W, X, Y, Z); FPUTC('\n', TERM_OUT);\
            }

#define TRACE_PR(X) \
            if (log_file != NULL) FPRINTF (log_file, X);

#define TRACE_PR2(X, Y) \
            if (log_file != NULL) FPRINTF (log_file, X, Y);

#define TRACE_PR3(X, Y, Z) \
            if (log_file != NULL) FPRINTF (log_file, X, Y, Z);

#define TRACE_PR_LN(X) \
            {\
                if (log_file != NULL) {\
                    FPRINTF (log_file, X);  FPUTC ('\n', log_file);\
                }\
            }

#define TRACE_PR_LN2(X, Y) \
            {\
                if (log_file != NULL) {\
                    FPRINTF (log_file, X, Y); FPUTC ('\n', log_file);\
                }\
            }

#define TRACE_PR_LN3(X, Y, Z) \
            {\
                if (log_file != NULL) {\
                    FPRINTF (log_file, X, Y, Z); FPUTC ('\n', log_file);\
                }\
            }

#define TRACE_PR_NEWLINE \
            {\
                if (log_file != NULL)\
                    FPUTC ('\n', log_file);\
            }


/***************************************************************************
 * WEB section number:   4
 * ~~~~~~~~~~~~~~~~~~~
 * Some of the code is intended to be used only when diagnosing the strange
 * behaviour that sometimes occurs when BibTeX is being installed or when
 * system wizards are fooling around with BibTeX without quite knowing what
 * they are doing. Such code will not normally be compiled; it is delimited
 * by the preprocessor conditionals `#ifdef DEBUG ... #endif'. Similarly,
 * there is some conditional code delimited by `#ifdef STAT ... #endif'
 * that is intended only for use when statistics are to be kept about
 * BibTeX's memory/CPU usage, and there is conditional code delimited by
 * `#ifdef TRACE ... #endif' that is intended to be a trace facility for
 * use mainly when debugging .BST files.
 ***************************************************************************/
#define DEBUG                       1
#define STAT                        1
#define TRACE                       1

/***************************************************************************
 * WEB section number:   9
 * ~~~~~~~~~~~~~~~~~~~
 * Here are some macros for common programming idioms.
 ***************************************************************************/
#define INCR(X)                     ++(X)
#define DECR(X)                     --(X)
#define LOOP                        while (TRUE)
#define DO_NOTHING                  ;
#define EMPTY                       0
#define ANY_VALUE                   0

/***************************************************************************
 * WEB section number:   14
 * ~~~~~~~~~~~~~~~~~~~
 * These parameters may be changed at compile time to extend or reduce
 * BiBTeX's capacity. They are set to accommodate a maximum of about 750
 * cites when used with the standard styles.
 ***************************************************************************/
#define MIN_PRINT_LINE              3
#define MAX_PRINT_LINE              79
#define AUX_STACK_SIZE              20
#define MAX_BIB_FILES               20

#define BUF_SIZE                   20000
#define MAX_CITES                  750
#define MAX_FIELDS                 5000
#define MAX_STRINGS                4000
#define POOL_SIZE                  65000L

#define MIN_CROSSREFS               2
#define WIZ_FN_SPACE                3000
#define SINGLE_FN_SPACE             50
#define ENT_STR_SIZE                100
#define GLOB_STR_SIZE               1000
#define LIT_STK_SIZE                50


/***************************************************************************
 * WEB section number:   15
 * ~~~~~~~~~~~~~~~~~~~
 * These parameters can also be changed at compile time, but they're
 * needed to define some WEB numeric macros so they must be defined
 * themselves.
 ***************************************************************************/
#define HASH_SIZE                  5000

#ifdef MSDOS
# define FILE_NAME_SIZE             64
#else                           /* NOT MSDOS */
# define FILE_NAME_SIZE             255
#endif                          /* MSDOS */

#define MAX_GLOB_STRS               10

/***************************************************************************
 * WEB section number:   18
 * ~~~~~~~~~~~~~~~~~~~
 * A global variable called |history| will contain one of four values at
 * the end of every run: |spotless| means that no unusual messages were
 * printed; |warning_message| means that a message of possible interest
 * was printed but no serious errors were detected; |error_message| means
 * that at least one error was found; |fatal_message| means that the
 * program terminated abnormally. The value of |history| does not
 * influence the behavior of the program; it is simply computed for the
 * convenience of systems that might want to use such information.
 ***************************************************************************/
#define SPOTLESS                    0
#define WARNING_MESSAGE             1
#define ERROR_MESSAGE               2
#define FATAL_MESSAGE               3

/***************************************************************************
 * WEB section number:   23
 * ~~~~~~~~~~~~~~~~~~~
 * We shall use the |text_char| to stand for the data type of the
 * characters that are converted to and from |ASCIICode_T| when they are
 * input and output. We shall also assume that |text_char| consists of the
 * elements |chr[FIRST_TEXT_CHAR]| through |chr[LAST_TEXT_CHAR]| inclusive.
 * The following definitions should be adjusted if necessary.
 ***************************************************************************/
#define FIRST_TEXT_CHAR             0
#define LAST_TEXT_CHAR              255

#ifdef SUPPORT_8BIT
#define LAST_ASCII_CHAR             255
#else
#define LAST_ASCII_CHAR             127
#endif                          /* SUPPORT_8BIT */
#ifdef UTF_8
#define LAST_LATIN_CHAR             0x17F
#define LAST_UCS_CHAR               0x10FFFF
#else
#define LAST_LATIN_CHAR             LAST_ASCII_CHAR
#endif


/***************************************************************************
 * WEB section number:   26
 * ~~~~~~~~~~~~~~~~~~~
 * Some of the ASCII codes without visible characters have been given
 * symbolic names in this program because they are used with a special
 * meaning.  The |tab| character may be system dependent.
 ***************************************************************************/
#define NULL_CODE                   0
#define TAB                         9
#define SPACE                       32
#define INVALID_CODE                127

/***************************************************************************
 * WEB section number:   29
 * ~~~~~~~~~~~~~~~~~~~
 * Also, various characters are given symbolic names; all the ones this
 * program uses are collected here.  We use the sharp sign as the
 * |concat_char|, rather than something more natural (like an ampersand),
 * for uniformity of database syntax (ampersand is a valid character in
 * identifiers).
 ***************************************************************************/
#define DOUBLE_QUOTE                '"'
#define NUMBER_SIGN                 '#'
#define COMMENT                     '%'
#define SINGLE_QUOTE                '\''
#define LEFT_PAREN                  '('
#define RIGHT_PAREN                 ')'
#define COMMA                       ','
#define MINUS_SIGN                  '-'
#define EQUALS_SIGN                 '='
#define AT_SIGN                     '@'
#define LEFT_BRACE                  '{'
#define RIGHT_BRACE                 '}'
#define PERIOD                      '.'
#define QUESTION_MARK               '?'
#define EXCLAMATION_MARK            '!'
#define TIE                         '~'
#define HYPHEN                      '-'
#define STAR                        '*'
#define CONCAT_CHAR                 '#'
#define COLON                       ':'
#define BACKSLASH                   '\\'

/***************************************************************************
 * WEB section number:   31
 * ~~~~~~~~~~~~~~~~~~~~
 * Every character has two types of the lexical classifications.  The
 * first type is general, and the second type tells whether the character
 * is legal in identifiers.
 ***************************************************************************/
#define ILLEGAL                     0
#define WHITE_SPACE                 1
#define ALPHA                       2
#define NUMERIC                     3
#define SEP_CHAR                    4
#define OTHER_LEX                   5
#define LAST_LEX                    5
#define ILLEGAL_ID_CHAR             0
#define LEGAL_ID_CHAR               1

/***************************************************************************
 * WEB section number:   35
 * ~~~~~~~~~~~~~~~~~~~~
 * Now we initialize the system-dependent |char_width| array, for which
 * |space| is the only |white_space| character given a nonzero printing
 * width.  The widths here are taken from Stanford's June '87 cmr10 font
 * and represent hundredths of a point (rounded), but since they're used
 * only for relative comparisons, the units have no meaning.
 ***************************************************************************/
#define SS_WIDTH                    500
#define AE_WIDTH                    722
#define OE_WIDTH                    778
#define UPPER_AE_WIDTH              903
#define UPPER_OE_WIDTH              1014

/***************************************************************************
 * WEB section number:   44
 * ~~~~~~~~~~~~~~~~~~~~
 * When something in the program wants to be bigger or something out
 * there wants to be smaller, it's time to call it a run.  Here's the
 * first of several macros that have associated procedures so that they
 * produce less inline code.
 ***************************************************************************/
#define BIBTEX_OVERFLOW(X, Y)              {\
    print_overflow ();\
    PRINT_LN3 ("%s%ld\n", (X), (long) (Y));\
    longjmp (Close_Up_Shop_Flag, 1);}

/***************************************************************************
 * WEB section number:   45
 * ~~~~~~~~~~~~~~~~~~~~
 * When something happens that the program thinks is impossible, call the
 * maintainer.
 ***************************************************************************/
#define CONFUSION(X)                {\
            PRINT (X); print_confusion();\
            longjmp (Close_Up_Shop_Flag, 1);}
#define CONFUSION2(X, Y)            {\
            PRINT2 (X, Y); print_confusion();\
            longjmp (Close_Up_Shop_Flag, 1);}

/***************************************************************************
 * WEB section number:   50
 * ~~~~~~~~~~~~~~~~~~~~
 * These macros send a string in |str_pool| to an output file.
 ***************************************************************************/
#define MAX_POP                     3
#define PRINT_POOL_STR(X)           print_a_pool_str(X)
#define TRACE_PR_POOL_STR(X)        { out_pool_str(log_file, X);}

/***************************************************************************
 * WEB section number:  52
 * ~~~~~~~~~~~~~~~~~~~
 * Several of the elementary string operations are performed using WEB
 * macros instead of using PASCAL procedures, because many of the operations
 * are done quite frequently and we want to avoid the overhead of procedure
 * calls. For example, here's a simple macro that computes the length of a
 * string.
 ***************************************************************************/
#define LENGTH(s)                   (str_start[(s)+1] - str_start[s])

/***************************************************************************
 * WEB section number:   53
 * ~~~~~~~~~~~~~~~~~~~
 * Strings are created by appending character codes to |str_pool|. The macro
 * called |append_char|, defined here, does not check to see if the value
 * of |pool_ptr| is too high; this test is supposed to be made before
 * |append_char| is used.
 ***************************************************************************/
#define APPEND_CHAR(X)              {str_pool[pool_ptr] = (X);\
                                     INCR(pool_ptr);}
#define STR_ROOM(X)                 {while((pool_ptr+(X))>Pool_Size)\
                                        pool_overflow();}

/***************************************************************************
 * WEB section number:   55
 * ~~~~~~~~~~~~~~~~~~~
 * These macros destroy an recreate the string at the end of the pool.
 ***************************************************************************/
#define FLUSH_STRING                {DECR(str_ptr);\
                                     pool_ptr=str_start[str_ptr];}
#define UNFLUSH_STRING              {INCR(str_ptr);\
                                     pool_ptr=str_start[str_ptr];}

/***************************************************************************
 * WEB section number:  62
 * ~~~~~~~~~~~~~~~~~~~
 * This system-independent procedure converts upper-case characters to
 * lower case for the specified part of |buf|.  It is system independent
 * because it uses only the internal representation for characters.
 ***************************************************************************/
#define CASE_DIFFERENCE             ('a' - 'A')

/***************************************************************************
 * WEB section number:   64
 * ~~~~~~~~~~~~~~~~~~~
 * All static strings that BibTeX might have to search for, generally
 * identifiers, are stored and retrieved by means of a fairly standard
 * hash-table algorithm (but slightly altered here) called the method of
 * ``coalescing lists'' (cf.\ Algorithm 6.4C in The Art of Computer
 * Programming).  Once a string enters the table, it is never removed.
 * The actual sequence of characters forming a string is stored in the
 * |str_pool| array.
 ***************************************************************************/
#define HASH_BASE                   (EMPTY + 1)
#define HASH_MAX                    (HASH_BASE + Hash_Size - 1)
#define HASH_IS_FULL                ((hash_used) == (HASH_BASE))
#define TEXT_ILK                    0
#define INTEGER_ILK                 1
#define AUX_COMMAND_ILK             2
#define AUX_FILE_ILK                3
#define BST_COMMAND_ILK             4
#define BST_FILE_ILK                5
#define BIB_FILE_ILK                6
#define FILE_EXT_ILK                7
#define FILE_AREA_ILK               8
#define CITE_ILK                    9
#define LC_CITE_ILK                 10
#define BST_FN_ILK                  11
#define BIB_COMMAND_ILK             12
#define MACRO_ILK                   13
#define CONTROL_SEQ_ILK             14
#define LAST_ILK                    14

/***************************************************************************
 * WEB section number:   68
 * ~~~~~~~~~~~~~~~~~~~
 * Here is the subroutine that searches the the hash table for a (string,
 * str_ilk) pair, where the string is of length l >= 0 and appears in
 * |buffer[j..(j+l-1)]|. If it finds the pair, it returns the corresponding
 * hash table location and sets the global variable |hash_found| to |TRUE|.
 * Otherwise is sets |hash_found| to |FALSE|, and if the parameter
 * |insert_it| is |TRUE|, it inserts the pair into the hash table, inserts
 * the string into |str_pool| if not previously encountered, and returns its
 * location. Note that two different pairs can have the same string but
 * different |str_ilks|, in which case the second pair encountered, if
 * |insert_it| were |TRUE|, would be inserted into the hash table though its
 * string wouldn't be inserted into |str_pool| because it would already be
 * there.
 ***************************************************************************/
#define MAX_HASH_VALUE              (Hash_Prime + Hash_Prime - 2 + 127)
#define DO_INSERT                   TRUE
#define DONT_INSERT                 FALSE

/***************************************************************************
 * WEB section number:   73
 * ~~~~~~~~~~~~~~~~~~~
 * The longest pre-defined string determines type definitions used to
 * insert the pre-defined strings into |str_pool|.
 * Set to accommodate the longest translations of the environment variables.
 ***************************************************************************/
#define LONGEST_PDS                 MAX_FILE_NAME

/***************************************************************************
 * WEB section number:   78
 * ~~~~~~~~~~~~~~~~~~~
 * These constants all begin with |n_| and are used for the |case|
 * statement that determines which command to execute.  The variable
 * |command_num| is set to one of these and is used to do the branching,
 * but it must have the full |integer| range because at times it can
 * assume an arbitrary |ilk_info| value (though it will be one of the
 * values here when we actually use it).
 ***************************************************************************/
#define N_AUX_BIBDATA               0
#define N_AUX_BIBSTYLE              1
#define N_AUX_CITATION              2
#define N_AUX_INPUT                 3

#define N_BST_ENTRY                 0
#define N_BST_EXECUTE               1
#define N_BST_FUNCTION              2
#define N_BST_INTEGERS              3
#define N_BST_ITERATE               4
#define N_BST_MACRO                 5
#define N_BST_READ                  6
#define N_BST_REVERSE               7
#define N_BST_SORT                  8
#define N_BST_STRINGS               9

#define N_BIB_COMMENT               0
#define N_BIB_PREAMBLE              1
#define N_BIB_STRING                2

/***************************************************************************
 * WEB section number:   80
 * ~~~~~~~~~~~~~~~~~~~
 * This section describes the various |buffer| scanning routines.  The
 * two global variables |buf_ptr1| and |buf_ptr2| are used in scanning an
 * input line.  Between scans, |buf_ptr1| points to the first character
 * of the current token and |buf_ptr2| points to that of the next.  The
 * global variable |last|, set by the function |input_ln|, marks the end
 * of the current line; it equals 0 at the end of the current file.  All
 * the procedures and functions in this section will indicate an
 * end-of-line when it's the end of the file.
 ***************************************************************************/
#define TOKEN_LEN                   (buf_ptr2 - buf_ptr1)
#define SCAN_CHAR                   buffer[buf_ptr2]

/***************************************************************************
 * WEB section number:  81
 * ~~~~~~~~~~~~~~~~~~~
 * These macros send the current token, in |buffer[buf_ptr1]| to
 * |buffer[buf_ptr2-1]|, to an output file.
 ***************************************************************************/
#define PRINT_TOKEN                 print_a_token ();
#define TRACE_PR_TOKEN              out_token (log_file);

/***************************************************************************
 * WEB section number:   89
 * ~~~~~~~~~~~~~~~~~~~
 * These are the possible values for |scan_result|; they're set by the
 * |scan_identifier| procedure and are described in the next section.
 ***************************************************************************/
#define ID_NULL                     0
#define SPECIFIED_CHAR_ADJACENT     1
#define OTHER_CHAR_ADJACENT         2
#define WHITE_ADJACENT              3

/***************************************************************************
 * WEB section number:  91
 * ~~~~~~~~~~~~~~~~~~~
 * The next two procedures scan for an integer, setting the global
 * variable |token_value| to the corresponding integer.
 ***************************************************************************/
#define CHAR_VALUE                  (SCAN_CHAR - '0')

/***************************************************************************
 * WEB section number:  93
 * ~~~~~~~~~~~~~~~~~~~
 * This procedure scans for an integer, stopping at the first nondigit;
 * it sets the value of |token_value| accordingly.  It returns |true| if
 * the token was a legal integer (i.e., consisted of an optional
 * |minus_sign| followed by one or more digits).
 ***************************************************************************/
#define NEGATIVE                    (sign_length == 1)

/***************************************************************************
 * WEB section number:  98
 * ~~~~~~~~~~~~~~~~~~~
 * I mean, this is truly disgraceful ...
 *
 * Note: The |term_out| file is system dependent.
 ***************************************************************************/
#define SAM_YOU_MADE_THE_FILE_NAME_TOO {\
            sam_too_long_file_name_print ();\
            goto Aux_Not_Found_Label;}

/***************************************************************************
 * WEB section number:  99
 * ~~~~~~~~~~~~~~~~~~~
 * We've abused the user enough for one section; suffice it to
 * say here that most of what we said last module still applies.
 * Note: The |term_out| file is system dependent.
 ***************************************************************************/
#define SAM_YOU_MADE_THE_FILE_NAME_WRON {\
            sam_wrong_file_name_print ();\
            goto Aux_Not_Found_Label;}

/***************************************************************************
 * WEB section number:  104
 * ~~~~~~~~~~~~~~~~~~~
 * Here we set up definitions and declarations for files opened in this
 * section.  Each element in |aux_list| (except for
 * |aux_list[aux_stack_size]|, which is always unused) is a pointer to
 * the appropriate |str_pool| string representing the \.{.aux} file name.
 * The array |aux_file| contains the corresponding \PASCAL\ |file|
 * variables.
 ***************************************************************************/
#define CUR_AUX_STR                 aux_list[aux_ptr]
#define CUR_AUX_FILE                aux_file[aux_ptr]
#define CUR_AUX_LINE                aux_ln_stack[aux_ptr]

/***************************************************************************
 * WEB section number:  111
 * ~~~~~~~~~~~~~~~~~~~
 * When we find a bug, we print a message and flush the rest of the line.
 * This macro must be called from within a procedure that has an |exit|
 * label.
 ***************************************************************************/
#define AUX_ERR_RETURN              {aux_err_print(); goto Exit_Label;}

/***************************************************************************
 * WEB section number:  112
 * ~~~~~~~~~~~~~~~~~~~
 * Here are a bunch of macros whose print statements are used at least
 * twice.  Thus we save space by making the statements procedures.  This
 * macro complains when there's a repeated command that's to be used just
 * once.
 ***************************************************************************/
#define AUX_ERR_ILLEGAL_ANOTHER(X)  {aux_err_illegal_another_print (X);\
                                     AUX_ERR_RETURN;}

/***************************************************************************
 * WEB section number:  113
 * ~~~~~~~~~~~~~~~~~~~
 * This one complains when a command is missing its |right_brace|.
 ***************************************************************************/
#define AUX_ERR_NO_RIGHT_BRACE      {\
            PRINT2 ("No \"%c\"", xchr[RIGHT_BRACE]);\
            AUX_ERR_RETURN;}

/***************************************************************************
 * WEB section number:  114
 * ~~~~~~~~~~~~~~~~~~~
 * This one complains when a command has stuff after its |right_brace|.
 ***************************************************************************/
#define AUX_ERR_STUFF_AFTER_RIGHT_BRACE {\
            PRINT2 ("Stuff after \"%c\"", xchr[RIGHT_BRACE]);\
            AUX_ERR_RETURN;}

/***************************************************************************
 * WEB section number:  115
 * ~~~~~~~~~~~~~~~~~~~
 * And this one complains when a command has white space in its argument.
 ***************************************************************************/
#define AUX_ERR_WHITE_SPACE_IN_ARGUMENT {\
            PRINT ("White space in argument");\
            AUX_ERR_RETURN;}

/***************************************************************************
 * WEB section number:  117
 * ~~~~~~~~~~~~~~~~~~~
 * Here we introduce some variables for processing a \.{\\bibdata}
 * command.  Each element in |bib_list| (except for
 * |bib_list[max_bib_files]|, which is always unused) is a pointer to the
 * appropriate |str_pool| string representing the \.{.bib} file name.
 * The array |bib_file| contains the corresponding \PASCAL\ |file|
 * variables.
 ***************************************************************************/
#define CUR_BIB_STR                 bib_list[bib_ptr]
#define CUR_BIB_FILE                bib_file[bib_ptr]

/***************************************************************************
 * WEB section number:  122
 * ~~~~~~~~~~~~~~~~~~~
 * This macro is very similar to aux_err but it complains specifically about
 * opening a file for a \bibdata command.
 ***************************************************************************/
#define OPEN_BIBDATA_AUX_ERR(X)     {\
            PRINT (X);\
            print_bib_name();\
            AUX_ERR_RETURN;}

/***************************************************************************
 * WEB section number:  129
 * ~~~~~~~~~~~~~~~~~~~
 * Here we introduce some variables for processing a \.{\\citation}
 * command.  Each element in |cite_list| (except for
 * |cite_list[Max_Cites]|, which is always unused) is a pointer to the
 * appropriate |str_pool| string.  The cite-key list is kept in order of
 * occurrence with duplicates removed.
 ***************************************************************************/
#define CUR_CITE_STR                cite_list[cite_ptr]

/***************************************************************************
 * WEB section number:  133
 * ~~~~~~~~~~~~~~~~~~~
 * We must check if (the lower-case version of) this cite key has been
 * previously encountered, and proceed accordingly.  The alias kludge
 * helps make the stack space not overflow on some machines.
 ***************************************************************************/
#define EX_BUF1                     ex_buf

/***************************************************************************
 * WEB section number:  144
 * ~~~~~~~~~~~~~~~~~~~
 * We must complain if anything's amiss.
 ***************************************************************************/
#define AUX_END_ERR(X)              {\
            aux_end1_err_print();\
            PRINT (X);\
            aux_end2_err_print();}

/***************************************************************************
 * WEB section number:   149
 * ~~~~~~~~~~~~~~~~~~~
 * When there's a serious error parsing the .bst file, we flush the rest of
 * the current command; a blank line is assumed to mark the end of a command
 * (but for the purposes of error recovery only). Thus, error recovery will
 * be better if style designers leave blank lines between .bst commands.
 * This macro must be called from within a procedure that has an 'exit'
 * label.
 ***************************************************************************/
#define BST_ERR_PRINT_AND_LOOK_FOR_BLAN {\
            bst_err_print_and_look_for_blan();\
            goto Exit_Label;}

#define BST_ERR(X)                  {\
            PRINT (X);\
            bst_err_print_and_look_for_blan ();\
            goto Exit_Label;}

#define BST_ERR2(X, Y)              {\
            PRINT2 (X, Y);\
            bst_err_print_and_look_for_blan ();\
            goto Exit_Label;}

/***************************************************************************
 * WEB section number:   150
 * ~~~~~~~~~~~~~~~~~~~
 * When there's a harmless error parsing the .bst file (harmless
 * syntactically, at least) we give just a warning message.
 ***************************************************************************/
#define BST_WARN(X)                 {\
            PRINT (X); bst_warn_print();}

/***************************************************************************
 * WEB section number:   153
 * ~~~~~~~~~~~~~~~~~~~
 * It's often illegal to end a .bst command in certain places, and this is
 * where we come to check.
 ***************************************************************************/
#define EAT_BST_WHITE_AND_EOF_CHECK(X)  {\
            if (! eat_bst_white_space()) {\
                PRINT (X); BST_ERR(X);}}

/***************************************************************************
 * WEB section number:   156
 * ~~~~~~~~~~~~~~~~~~~
 * We need data structures for the function definitions, the entry
 * variables, the global variables, and the actual entries corresponding
 * to the cite-key list.  First we define the classes of `function's used.
 * Functions in all classes are of |bst_fn_ilk| except for |int_literal|s,
 * which are of |integer_ilk|; and |str_literal|s, which are of
 * |text_ilk|.
 ***************************************************************************/
#define BUILT_IN                    0
#define WIZ_DEFINED                 1
#define INT_LITERAL                 2
#define STR_LITERAL                 3
#define FIELD                       4
#define INT_ENTRY_VAR               5
#define STR_ENTRY_VAR               6
#define INT_GLOBAL_VAR              7
#define STR_GLOBAL_VAR              8
#define LAST_FN_CLASS               8

/***************************************************************************
 * WEB section number:  160
 * ~~~~~~~~~~~~~~~~~~~
 * Besides the function classes, we have types based on \BibTeX's
 * capacity limitations and one based on what can go into the array
 * |wiz_functions| explained below.
 ***************************************************************************/
#define QUOTE_NEXT_FN               (HASH_BASE - 1)
#define END_OF_DEF                  (HASH_MAX + 1)

/***************************************************************************
 * WEB section number:  161
 * ~~~~~~~~~~~~~~~~~~~
 * We store information about the \.{.bst} functions in arrays the same
 * size as the hash-table arrays and in locations corresponding to their
 * hash-table locations.  The two arrays |fn_info| (an alias of
 * |ilk_info| described earlier) and |fn_type| accomplish this: |fn_type|
 * specifies one of the above classes, and |fn_info| gives information
 * dependent on the class.
 ***************************************************************************/
#define FN_INFO                     ilk_info
#define MISSING                     EMPTY

/***************************************************************************
 * WEB section number:  166
 * ~~~~~~~~~~~~~~~~~~~
 * This macro is used to scan all .bst identifiers. The argument supplies
 * .bst command name. The associated procedure simply prints an error
 * message.
 ***************************************************************************/
#define BST_IDENTIFIER_SCAN(X)      {\
        scan_identifier (RIGHT_BRACE, COMMENT, COMMENT);\
        if ((scan_result == WHITE_ADJACENT) \
            || (scan_result == SPECIFIED_CHAR_ADJACENT)) {DO_NOTHING;}\
        else {bst_id_print(); BST_ERR(X)}}

/***************************************************************************
 * WEB section number:  167
 * ~~~~~~~~~~~~~~~~~~~
 * This macro just makes sure we're at a |left_brace|.
 ***************************************************************************/
#define BST_GET_AND_CHECK_LEFT_BRACE(X) {\
        if (SCAN_CHAR != LEFT_BRACE) \
                {bst_left_brace_print(); BST_ERR(X);}\
        INCR (buf_ptr2);}

/***************************************************************************
 * WEB section number:  168
 * ~~~~~~~~~~~~~~~~~~~
 * This macro just makes sure we're at a |right_brace|.
 ***************************************************************************/
#define BST_GET_AND_CHECK_RIGHT_BRACE(X)        {\
        if (SCAN_CHAR != RIGHT_BRACE) \
                {bst_right_brace_print(); BST_ERR(X);}\
        INCR (buf_ptr2);}

/***************************************************************************
 * WEB section number:  169
 * ~~~~~~~~~~~~~~~~~~~
 * This macro complains if we've already encountered a function to be
 * inserted into the hash table.
 ***************************************************************************/
#define CHECK_FOR_ALREADY_SEEN_FUNCTION(X)      {\
        if (hash_found) {\
            already_seen_function_print(X);\
            goto Exit_Label;}}

/***************************************************************************
 * WEB section number:  183
 * ~~~~~~~~~~~~~~~~~~~
 * We're about to start scanning tokens in a function definition.  When a
 * function token is illegal, we skip until it ends; a |white_space|
 * character, an end-of-line, a |right_brace|, or a |comment| marks the
 * end of the current token.
 ***************************************************************************/
#define SKIP_TOKEN(X)               {\
            PRINT (X);\
            skip_token_print ();\
            goto Next_Token_Label;}
#define SKIP_TOKEN2(X, Y)           {\
            PRINT2 (X, Y);\
            skip_token_print ();\
            goto Next_Token_Label;}

/***************************************************************************
 * WEB section number:  184
 * ~~~~~~~~~~~~~~~~~~~
 * This macro is similar to the last one but is specifically for
 * recursion in a |wiz_defined| function, which is illegal; it helps save
 * space.
 ***************************************************************************/
#define SKIP_RECURSIVE_TOKEN        { print_recursion_illegal (); \
                                      goto Next_Token_Label;}

/***************************************************************************
 * WEB section number:  185
 * ~~~~~~~~~~~~~~~~~~~
 * Here's another macro for saving some space when there's a problem with
 * a token.
 ***************************************************************************/
#define SKIP_TOKEN_UNKNOWN_FUNCTION {skp_token_unknown_function_prin ();\
                                     goto Next_Token_Label;}

/***************************************************************************
 * WEB section number:  186
 * ~~~~~~~~~~~~~~~~~~~
 * And another.
 ***************************************************************************/
#define SKIP_TOKEN_ILLEGAL_STUFF_AFTER {\
            skip_illegal_stuff_after_token ();\
            goto Next_Token_Label;}

/***************************************************************************
 * WEB section number:  194
 * ~~~~~~~~~~~~~~~~~~~
 * This module marks the implicit function as being quoted, generates a
 * name, and stores it in the hash table.  This name is strictly internal
 * to this program, starts with a |single_quote| (since that will make
 * this function name unique), and ends with the variable |impl_fn_num|
 * converted to ASCII.  The alias kludge helps make the stack space not
 * overflow on some machines.
 ***************************************************************************/
#define EX_BUF2                     ex_buf

/***************************************************************************
 * WEB section number:  197
 * ~~~~~~~~~~~~~~~~~~~
 * This module appends a character to |int_buf| after checking to make
 * sure it will fit; for use in |int_to_ASCII|.
 ***************************************************************************/
#define APPEND_INT_CHAR(X)          {\
            if (int_ptr == Buf_Size)\
                { buffer_overflow ();}\
            int_buf[int_ptr] = (X);\
            INCR (int_ptr); }

/***************************************************************************
 * WEB section number:  216
 * ~~~~~~~~~~~~~~~~~~~
 * Here we insert the just found |int_global_var| name into the hash
 * table and record it as an |int_global_var|.  Also, we initialize it by
 * setting |fn_info[fn_loc]| to 0.
 ***************************************************************************/
#define END_OF_STRING               INVALID_CODE

/***************************************************************************
 * WEB section number:  219
 * ~~~~~~~~~~~~~~~~~~~
 * These global variables are used ...
 ***************************************************************************/
#define UNDEFINED                   (HASH_MAX + 1)

/***************************************************************************
 * WEB section number:  221
 * ~~~~~~~~~~~~~~~~~~~
 * When there's a serious error parsing a \.{.bib} file, we flush
 * everything up to the beginning of the next entry.
 ***************************************************************************/
#define BIB_ERR(X)                  {\
            PRINT (X);\
            bib_err_print ();\
            goto Exit_Label;}
#define BIB_ERR2(X, Y)              {\
            PRINT2 (X, Y);\
            bib_err_print ();\
            goto Exit_Label;}
#define BIB_ERR3(X, Y, Z)                   {\
            PRINT3 (X, Y, Z);\
            bib_err_print ();\
            goto Exit_Label;}

/***************************************************************************
 * WEB section number:  222
 * ~~~~~~~~~~~~~~~~~~~
 * When there's a harmless error parsing a \.{.bib} file, we just give a
 * warning message.  This is always called after other stuff has been
 * printed out.
 ***************************************************************************/
#define BIB_WARN(X)                 {\
            PRINT (X);\
            bib_warn_print ();}
#define BIB_WARN_NEWLINE(X)         {\
            PRINT_LN (X);\
            bib_warn_print ();}

/***************************************************************************
 * WEB section number:  229
 * ~~~~~~~~~~~~~~~~~~~
 * It's often illegal to end a \.{.bib} command in certain places, and
 * this is where we come to check.
 ***************************************************************************/
#define EAT_BIB_WHITE_AND_EOF_CHECK {\
            if (! eat_bib_white_space())\
                {eat_bib_print();\
                goto Exit_Label;}}

/***************************************************************************
 * WEB section number:  230
 * ~~~~~~~~~~~~~~~~~~~
 * And here are a bunch of error-message macros, each called more than
 * once, that thus save space as implemented.  This one is for when one
 * of two possible characters is expected while scanning.
 ***************************************************************************/
#define BIB_ONE_OF_TWO_EXPECTED_ERR(X, Y)       {\
            bib_one_of_two_print (X, Y);\
            goto Exit_Label;}

/***************************************************************************
 * WEB section number:  231
 * ~~~~~~~~~~~~~~~~~~~
 * This one's for an unexpected equals sign.
 ***************************************************************************/
#define BIB_EQUALS_SIGN_EXPECTED_ERR    {\
            bib_equals_sign_print ();\
            goto Exit_Label;}

/***************************************************************************
 * WEB section number:  232
 * ~~~~~~~~~~~~~~~~~~~
 * This complains about unbalanced braces.
 ***************************************************************************/
#define BIB_UNBALANCED_BRACES_ERR       {\
            bib_unbalanced_braces_print ();\
            goto Exit_Label;}

/***************************************************************************
 * WEB section number:  233
 * ~~~~~~~~~~~~~~~~~~~
 * And this one about an overly exuberant field.
 ***************************************************************************/
#define BIB_FIELD_TOO_LONG_ERR      {\
            bib_field_too_long_print ();\
            goto Exit_Label;}

/***************************************************************************
 * WEB section number:  234
 * ~~~~~~~~~~~~~~~~~~~
 * This one is just a warning, not an error.  It's for when something
 * isn't (or might not be) quite right with a macro name.
 ***************************************************************************/
#define MACRO_NAME_WARNING(X)       {\
            macro_warn_print ();\
            BIB_WARN_NEWLINE (X);}

/***************************************************************************
 * WEB section number:  235
 * ~~~~~~~~~~~~~~~~~~~
 * This macro is used to scan all \.{.bib} identifiers.  The argument
 * tells what was happening at the time.  The associated procedure simply
 * prints an error message.
 ***************************************************************************/
#define BIB_IDENTIFIER_SCAN_CHECK(X)    {\
            if ((scan_result == WHITE_ADJACENT) \
                || (scan_result == SPECIFIED_CHAR_ADJACENT)) {DO_NOTHING;}\
            else {bib_id_print(); BIB_ERR (X);}}

/***************************************************************************
 * WEB section number:  247
 * ~~~~~~~~~~~~~~~~~~~
 * The variables for the function
 * |scan_and_store_the_field_value_and_eat_white| must be global since
 * the functions it calls use them too.  The alias kludge helps make the
 * stack space not overflow on some machines.
 ***************************************************************************/
#define FIELD_VL_STR                ex_buf
#define FIELD_END                   ex_buf_ptr
#define FIELD_START                 ex_buf_xptr

/***************************************************************************
 * WEB section number:  251
 * ~~~~~~~~~~~~~~~~~~~
 * Now we come to the stuff that actually accumulates the field value to
 * be stored.  This module copies a character into |field_vl_str| if it
 * will fit; since it's so low level, it's implemented as a macro.
 ***************************************************************************/
#define COPY_CHAR(X)                {\
            if (FIELD_END == Buf_Size)\
                { BIB_FIELD_TOO_LONG_ERR; }\
            else\
                { FIELD_VL_STR[FIELD_END] = (X);\
                INCR (FIELD_END);}}

/***************************************************************************
 * WEB section number:  252
 * ~~~~~~~~~~~~~~~~~~~
 * The \.{.bib}-specific scanning function |compress_bib_white| skips
 * over |white_space| characters within a string until hitting a nonwhite
 * character; in fact, it does everything |eat_bib_white_space| does, but
 * it also adds a |space| to |field_vl_str|.  This function is never
 * called if there are no |white_space| characters (or ends-of-line) to
 * be scanned (though the associated macro might be).  The function
 * returns |false| if there is a serious syntax error.
 ***************************************************************************/
#define CHECK_FOR_AND_COMPRESS_BIB_WHIT {\
            if ((lex_class[SCAN_CHAR] == WHITE_SPACE) || (buf_ptr2 == last)){\
                if (! compress_bib_white ()) { goto Exit_Label;}}}

/***************************************************************************
 * WEB section number:  264
 * ~~~~~~~~~~~~~~~~~~~
 * If the cross-referenced entry isn't already on |cite_list| we add it
 * (at least temporarily); if it is already on |cite_list| we update the
 * cross-reference count, if necessary.  Note that |all_entries| is
 * |false| here.  The alias kludge helps make the stack space not
 * overflow on some machines.
 ***************************************************************************/
#define EXTRA_BUF                   out_buf

/***************************************************************************
 * WEB section number:  267
 * ~~~~~~~~~~~~~~~~~~~
 * The lower-case version of this database key must correspond to one in
 * |cite_list|, or else |all_entries| must be |true|, if this entry is to
 * be included in the reference list.  Accordingly, this module sets
 * |store_entry|, which determines whether the relevant information for
 * this entry is stored.  The alias kludge helps make the stack space not
 * overflow on some machines.
 ***************************************************************************/
#define EX_BUF3                     ex_buf

/***************************************************************************
 * WEB section number:  270
 * ~~~~~~~~~~~~~~~~~~~
 * This module, a simpler version of the
 * |find_cite_locs_for_this_cite_key| function, exists primarily to
 * compute |lc_xcite_loc|.  When this code is executed we have
 * |(all_entries) and (entry_cite_ptr >= all_marker) and (not
 * entry_exists[entry_cite_ptr])|.  The alias kludge helps make the stack
 * space not overflow on some machines.
 ***************************************************************************/
#define EX_BUF4                     ex_buf
#define EX_BUF4_PTR                 ex_buf_ptr

/***************************************************************************
 * WEB section number:  277
 * ~~~~~~~~~~~~~~~~~~~
 * Occasionally we need to figure out the hash-table location of a given
 * cite-key string and its lower-case equivalent.  This function does
 * that.  To perform the task it needs to borrow a buffer, a need that
 * gives rise to the alias kludge---it helps make the stack space not
 * overflow on some machines (and while it's at it, it'll borrow a
 * pointer, too).  Finally, the function returns |true| if the cite key
 * exists on |cite_list|, and its sets |cite_hash_found| according to
 * whether or not it found the actual version (before |lower_case|ing) of
 * the cite key; however, its {\sl raison d'\^$\mkern-8mu$etre\/}
 * (literally, ``to eat a raisin'') is to compute |cite_loc| and
 * |lc_cite_loc|.
 ***************************************************************************/
#define EX_BUF5                     ex_buf
#define EX_BUF5_PTR                 ex_buf_ptr

/***************************************************************************
 * WEB section number:  289
 * ~~~~~~~~~~~~~~~~~~~
 * The array |sorted_cites| initially specifies that the entries are to
 * be processed in order of cite-key occurrence.  The \.{sort} command
 * may change this to whatever it likes (which, we hope, is whatever the
 * style-designer instructs it to like).  We make |sorted_cites| an alias
 * to save space; this works fine because we're done with |cite_info|.
 ***************************************************************************/
#define SORTED_CITES                cite_info

/***************************************************************************
 * WEB section number:  291
 * ~~~~~~~~~~~~~~~~~~~
 * Where |lit_stk_loc| is a stack location, and where |stk_type| gives
 * one of the three types of literals (an integer, a string, or a
 * function) or a special marker.  If a |lit_stk_type| element is a
 * |stk_int| then the corresponding |lit_stack| element is an integer; if
 * a |stk_str|, then a pointer to a |str_pool| string; and if a |stk_fn|,
 * then a pointer to the function's hash-table location.  However, if the
 * literal should have been a |stk_str| that was the value of a field
 * that happened to be |missing|, then the special value
 * |stk_field_missing| goes on the stack instead; its corresponding
 * |lit_stack| element is a pointer to the field-name's string.  Finally,
 * |stk_empty| is the type of a literal popped from an empty stack.
 ***************************************************************************/
#define STK_INT                     0
#define STK_STR                     1
#define STK_FN                      2
#define STK_FIELD_MISSING           3
#define STK_EMPTY                   4
#define LAST_LIT_TYPE               4

/***************************************************************************
 * WEB section number:  293
 * ~~~~~~~~~~~~~~~~~~~
 * When there's an error while executing \.{.bst} functions, what we do
 * depends on whether the function is messing with the entries.
 * Furthermore this error is serious enough to classify as an
 * |error_message| instead of a |warning_message|.  These messages (that
 * is, from |bst_ex_warn|) are meant both for the user and for the style
 * designer while debugging.
 ***************************************************************************/
#define BST_EX_WARN(X)              {\
            PRINT (X); bst_ex_warn_print ();}
#define BST_EX_WARN2(X, Y)          {\
            PRINT2 (X, Y); bst_ex_warn_print ();}

/***************************************************************************
 * WEB section number:  294
 * ~~~~~~~~~~~~~~~~~~~
 * When an error is so harmless, we print a warning message instead of an
 * error message.
 ***************************************************************************/
#define BST_MILD_EX_WARN(X)                 {\
            PRINT (X); bst_mild_ex_warn_print ();}

/***************************************************************************
 * WEB section number:  301
 * ~~~~~~~~~~~~~~~~~~~
 * The function |less_than| compares the two \.{sort.key\$}s indirectly
 * pointed to by its arguments and returns |true| if the first argument's
 * \.{sort.key\$} is lexicographically less than the second's (that is,
 * alphabetically earlier).  In case of ties the function compares the
 * indices |arg1| and |arg2|, which are assumed to be different, and
 * returns |true| if the first is smaller.  This function uses
 * |ASCII_code|s to compare, so it might give ``interesting'' results
 * when handling nonletters.
 ***************************************************************************/
#define COMPARE_RETURN(X)           {\
            less_than = (X);\
            goto Exit_Label;}

/***************************************************************************
 * WEB section number:  302
 * ~~~~~~~~~~~~~~~~~~~
 * The recursive procedure |quick_sort| sorts ...
 ***************************************************************************/
#define SHORT_LIST                  10
#define END_OFFSET                  4

/***************************************************************************
 * WEB section number:  308
 * ~~~~~~~~~~~~~~~~~~~
 * This macro pushes the last thing, necessarily a string, that was
 * popped.  And this module, along with others that push the literal
 * stack without explicitly calling |push_lit_stack|, have an index entry
 * under ``push the literal stack''; these implicit pushes collectively
 * speed up the program by about ten percent.
 ***************************************************************************/
#define REPUSH_STRING               {\
            if (lit_stack[lit_stk_ptr] >= cmd_str_ptr)\
                { UNFLUSH_STRING; }\
            INCR (lit_stk_ptr);}

/***************************************************************************
 * WEB section number:  319
 * ~~~~~~~~~~~~~~~~~~~
 * These macros append a character to |ex_buf|.  Which is called depends
 * on whether the character is known to fit.
 ***************************************************************************/
#define APPEND_EX_BUF_CHAR(X)       {\
            ex_buf[ex_buf_ptr] = (X);\
            INCR(ex_buf_ptr);}
#define APPEND_EX_BUF_CHAR_AND_CHECK(X)    {\
            if (ex_buf_ptr == Buf_Size) {buffer_overflow ();}; \
            APPEND_EX_BUF_CHAR(X)}

/***************************************************************************
 * WEB section number:   333
 * ~~~~~~~~~~~~~~~~~~~
 * These constants all begin with |n_| and are used for the |case|
 * statement that determines which |built_in| function to execute.
 ***************************************************************************/
#define N_EQUALS                    0
#define N_GREATER_THAN              1
#define N_LESS_THAN                 2
#define N_PLUS                      3
#define N_MINUS                     4
#define N_CONCATENATE               5
#define N_GETS                      6
#define N_ADD_PERIOD                7
#define N_CALL_TYPE                 8
#define N_CHANGE_CASE               9
#define N_CHR_TO_INT                10
#define N_CITE                      11
#define N_DUPLICATE                 12
#define N_EMPTY                     13
#define N_FORMAT_NAME               14
#define N_IF                        15
#define N_INT_TO_CHR                16
#define N_INT_TO_STR                17
#define N_MISSING                   18
#define N_NEWLINE                   19
#define N_NUM_NAMES                 20
#define N_POP                       21
#define N_PREAMBLE                  22
#define N_PURIFY                    23
#define N_QUOTE                     24
#define N_SKIP                      25
#define N_STACK                     26
#define N_SUBSTRING                 27
#define N_SWAP                      28
#define N_TEXT_LENGTH               29
#define N_TEXT_PREFIX               30
#define N_TOP_STACK                 31
#define N_TYPE                      32
#define N_WARNING                   33
#define N_WHILE                     34
#define N_WIDTH                     35
#define N_WRITE                     36
#define N_BIT_AND                   37
#define N_BIT_OR                    38
#ifdef UTF_8
#define N_IS_CJK_STRING             39
#define NUM_BLT_IN_FNS              40
#else
#define NUM_BLT_IN_FNS              39
#endif

/***************************************************************************
 * WEB section number:  338
 * ~~~~~~~~~~~~~~~~~~~
 * These constants all begin with |n_| and are used for the |case|
 * statement that determines which, if any, control sequence we're
 * dealing with; a control sequence of interest will be either one of the
 * undotted characters `\.{\\i}' or `\.{\\j}' or one of the foreign
 * characters in Table~3.2 of the \LaTeX\ manual.
 ***************************************************************************/
#define N_I                         0
#define N_J                         1
#define N_OE                        2
#define N_OE_UPPER                  3
#define N_AE                        4
#define N_AE_UPPER                  5
#define N_AA                        6
#define N_AA_UPPER                  7
#define N_O                         8
#define N_O_UPPER                   9
#define N_L                         10
#define N_L_UPPER                   11
#define N_SS                        12

/***************************************************************************
 * WEB section number:  344
 * ~~~~~~~~~~~~~~~~~~~
 * These are nonrecursive variables that |execute_fn| uses.  Declaring
 * them here (instead of in the previous module) saves execution time and
 * stack space on most machines.
 ***************************************************************************/
#define NAME_BUF                    sv_buffer

/***************************************************************************
 * WEB section number:  356
 * ~~~~~~~~~~~~~~~~~~~
 * It's time for a complaint if either of the two (entry or global) string
 * lengths is exceeded.
 ***************************************************************************/
#define BST_STRING_SIZE_EXCEEDED(X, Y)  {\
            bst_1print_string_size_exceeded ();\
            PRINT3 ("%ld%s", (long) (X), Y);\
            bst_2print_string_size_exceeded ();}

/***************************************************************************
 * WEB section number:  365
 * ~~~~~~~~~~~~~~~~~~~
 * First we define a few variables for case conversion.  The constant
 * definitions, to be used in |case| statements, are in order of probable
 * frequency.
 ***************************************************************************/
#define TITLE_LOWERS                0
#define ALL_LOWERS                  1
#define ALL_UPPERS                  2
#define BAD_CONVERSION              3

/***************************************************************************
 * WEB section number:  397
 * ~~~~~~~~~~~~~~~~~~~
 * It's a von token if there exists a first brace-level-0 letter (or
 * brace-level-1 special character), and it's in lower case; in this case
 * we return |true|.  The token is in |name_buf|, starting at
 * |name_bf_ptr| and ending just before |name_bf_xptr|.
 ***************************************************************************/
#define RETURN_VON_FOUND            {\
            von_token_found = TRUE;\
            goto Exit_Label;}

/***************************************************************************
 * WEB section number:  417
 * ~~~~~~~~~~~~~~~~~~~
 * Here we output either the \.{.bst} given string if it exists, or else
 * the \.{.bib} |sep_char| if it exists, or else the default string.  A
 * |tie| is the default space character between the last two tokens of
 * the name part, and between the first two tokens if the first token is
 * short enough; otherwise, a |space| is the default.
 ***************************************************************************/
#define LONG_TOKEN                  3

/***************************************************************************
 * WEB section number:  419
 * ~~~~~~~~~~~~~~~~~~~
 * If the last character output for this name part is a |tie| but the
 * previous character it isn't, we're dealing with a discretionary |tie|;
 * thus we replace it by a |space| if there are enough characters in the
 * rest of the name part.
 ***************************************************************************/
#define LONG_NAME                   3

/***************************************************************************
 * WEB section number:  465
 * ~~~~~~~~~~~~~~~~~~~
 * These statistics can help determine how large some of the constants
 * should be and can tell how useful certain |built_in| functions are.
 * They are written to the same files as tracing information.
 ***************************************************************************/
#define STAT_PR                     TRACE_PR
#define STAT_PR2                    TRACE_PR2
#define STAT_PR3                    TRACE_PR3

#define STAT_PR_LN                  TRACE_PR_LN
#define STAT_PR_LN2                 TRACE_PR_LN2
#define STAT_PR_LN3                 TRACE_PR_LN3

#define STAT_PR_POOL_STR            TRACE_PR_POOL_STR


/***************************************************************************
 * WEB section number:  N/A
 * ~~~~~~~~~~~~~~~~~~~
 * C isn't very good at declaring two dimensional arrays whose sizes are
 * determined at run time.  Here we create a useful macro to simulate
 * accessing a 2D array by converting the row/col into an offset from the
 * beginning of a 1D array.
 ***************************************************************************/
#define ENTRY_STRS(_r,_c)       entry_strs[(_r * (Ent_Str_Size+1)) + _c]
#define GLOBAL_STRS(_r,_c)      global_strs[(_r * (Glob_Str_Size+1)) + _c]


/***************************************************************************
 * WEB section number:  N/A
 * ~~~~~~~~~~~~~~~~~~~
 * Define a macro to handle 1-, 2-, 3-, and 4-byte UTF-8 codes.
 ***************************************************************************/
#define DO_UTF8(ch, do_1, do_2, do_3, do_4) \
  if (ch <= 0x7F) { do_1; } \
  else if ((ch >= 0xC2) && (ch <= 0xDF)) { do_2; } \
  else if ((ch >= 0xE0) && (ch <= 0xEF)) { do_3; } \
  else if ((ch >= 0xF0) && (ch <= 0xF4)) { do_4; } \
  else printf("this (%x) isn't a right UTF-8 char!\n", ch)


/***************************************************************************
 * WEB section number:  N/A
 * ~~~~~~~~~~~~~~~~~~~
 * Macros adapted from Kpathsea (lib.h) and Web2C (cpascal.h) to dynamically
 * resize arrays.
 ***************************************************************************/
/* Reallocate N items of type T for ARRAY using myrealloc.  */
#define MYRETALLOC(array, addr, n, t) ((addr) = (t *) myrealloc(addr, (n) * sizeof(t), array))
/* BibTeX needs this to dynamically reallocate arrays.  Too bad we can't
   rely on stringification, or we could avoid the ARRAY_NAME arg.
   Actually allocate one more than requests, so we can index the last
   entry, as Pascal wants to do.  */
#define BIB_XRETALLOC_NOSET(array_name, array_var, type, size_var, new_size) \
  if (log_file != NULL)\
    fprintf (log_file, "Reallocated %s (elt_size=%d) to %ld items from %ld.\n", \
             array_name, (int) sizeof (type), new_size, size_var); \
  MYRETALLOC (array_name, array_var, new_size + 1, type)
/* Same as above, but also increase SIZE_VAR for the last (or only) array.  */
#define BIB_XRETALLOC(array_name, array_var, type, size_var, new_size) do { \
  BIB_XRETALLOC_NOSET(array_name, array_var, type, size_var, new_size); \
  size_var = new_size; \
} while (0)
/* Same as above, but for the pseudo-TYPE ASCIICode_T[LENGTH+1].  */
#define BIB_XRETALLOC_STRING(array_name, array_var, length, size_var, new_size) \
  if (log_file != NULL)\
    fprintf (log_file, "Reallocated %s (elt_size=%d) to %ld items from %ld.\n", \
             array_name, (int) (length + 1), new_size, size_var); \
  MYRETALLOC (array_name, array_var, (new_size) * (length + 1), ASCIICode_T)

#endif                          /* __BIBTEX_H__ */

