/*-
******************************************************************************
******************************************************************************
**
**  MODULE
**
**      $RCSfile: sysdep.h,v $
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
**      This module defines as many system dependent things as possible.
**      System / compiler dependent code should only appear in sysdep.h,
**      bibtex.c and utils.c.  System dependent code should be delimited by
**      preprocessor conditionals.  Look for:
**
**          #ifdef MSDOS
**          #ifdef OS2
**          #ifdef UNIX
**          #ifdef VMS
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
**      $Log: sysdep.h,v $
**      Revision 3.71  1996/08/18  20:37:06  kempson
**      Official release 3.71 (see HISTORY file for details).
**
**      Revision 3.70  1996/04/08  10:08:40  kempson
**      Final documentation & cosmetic changes for official release 3.70.
**
**      Revision 3.6  1995/10/21  22:13:13  kempson
**      Added support for DJGPP under MSDOS.
**
**      Revision 3.5  1995/09/24  20:44:37  kempson
**      Many changes for final beta test version.
**
**      Revision 3.4  1995/04/09  22:15:46  kempson
**      Placed under RCS control
**
******************************************************************************
******************************************************************************
*/
#ifndef __SYSDEP_H__
# define __SYSDEP_H__               1


/*-
**============================================================================
** Include the relevant header files.
**============================================================================
*/
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>


/*-
**============================================================================
** For each environment (MSDOS, OS/2, UNIX & VMS), the following
** preprocessor macros must be defined:
**
** IMPLEMENTATION       the implementation string reported by BibTeX when
**                      the --version is specified.
**
** PATH_DELIM           the character used to separate components of a
**                      search path string.
**
**                          Note: usually ';' for DOS & OS/2, ':' for Unix
**                                and ',' for VMS.
**============================================================================
*/
#ifdef WIN32
# ifdef _MSC_VER
#  define IMPLEMENTATION            "Microsoft(R) C/C++ for Win32"
# elif defined(__MINGW32__)
#  define IMPLEMENTATION            "MinGW32 C/C++ for Win32"
# else
#  define IMPLEMENTATION            "Unknown C/C++ for Win32"
# endif				/* _MSC_VER */
#endif

#ifdef  MSDOS

# ifdef DJGPP
#  define IMPLEMENTATION            "DJGPP/GNU C 32 bit bound for DOS"
# else
#  define IMPLEMENTATION            "EMX/GNU C 32 bit bound for DOS"
# endif		                /* DJGPP */

# define PATH_DELIM                 ';'
#endif				/* MSDOS */

#ifdef  OS2
# define IMPLEMENTATION             "EMX/GNU C 32 bit for OS/2"
# define PATH_DELIM                 ';'
#endif				/* OS2 */

#if defined(MIKTEX) && !defined(MIKTEX_WINDOWS)
# ifndef IMPLEMENTATION
#  define IMPLEMENTATION            "GNU C"
# endif
# define PATH_DELIM                 ':'
#endif

#ifdef  UNIX
# ifndef IMPLEMENTATION
#  define IMPLEMENTATION            "C for Unix"
# endif
# define PATH_DELIM                 ':'
#endif				/* UNIX */

#ifdef  VMS
# define IMPLEMENTATION             "GNU C for VMS"
# define PATH_DELIM                 ','
#endif				/* VMS */


/*-
**============================================================================
** Define the exit status values that should be returned to the operating
** system.
**============================================================================
*/
#ifdef VMS
# define NORMAL_EXIT_STATUS         0x10000001L
# define WARNING_EXIT_STATUS        0x10000000L
# define ERROR_EXIT_STATUS          0x10000002L
# define FATAL_EXIT_STATUS          0x10000004L
#else                           /* NOT VMS */
# define NORMAL_EXIT_STATUS         0
# define WARNING_EXIT_STATUS        1
# define ERROR_EXIT_STATUS          2
# define FATAL_EXIT_STATUS          3
#endif                          /* VMS */


/*-
**============================================================================
** Some macros to satisfy ANSI C's strict (for C anyway) type checking.
**============================================================================
*/
#define FPRINTF                     (void) fprintf
#define FPUTC                       (void) fputc
#define PRINTF                      (void) printf
#define PUTC                        (void) putc
#define SPRINTF                     (void) sprintf
#define CHR(x)                      (x)


/*-
**============================================================================
** Some macros to make the C code look more like the WEB from which it was
** translated.
**============================================================================
*/
#define BEGIN                       {
#define END                         }
#ifndef TRUE
# define TRUE                       1
#endif
#ifndef FALSE
# define FALSE                      0
#endif
#define REPEAT                      do
#define UNTIL(X)                    while ( ! (X))
#define DIV                         /
#define MOD                         %


/*-
**============================================================================
** WEB section number:   39
** ~~~~~~~~~~~~~~~~~~~~
** We implement path searching for the function |a_open_in|. If
** |name_of_file| doesn't start with `\' then path names from the
** appropriate path list are prepended until success or the end of the path
** list is reached.
**============================================================================
*/
#define NO_SEARCH_PATH              0
#define AUX_FILE_SEARCH_PATH        1
#define BIB_FILE_SEARCH_PATH        2
#define BST_FILE_SEARCH_PATH        3
#define CSF_FILE_SEARCH_PATH        4


/*-
**============================================================================
** The Makefile specifies a number of preprocessor macros used to control
** how BibTeX searches for input files.  If any of them are undefined, an
** error message will be issued by the C preprocessor.  The macros
** required are:
**
**  AUX_INPUT_ENVVAR  - environment variable specifying the search path for
**                      .aux files 
**  AUX_INPUT_PATH    - fallback search path for .aux files
**  BIB_INPUT_ENVVAR  - environment variable specifying the search path for
**                      .bib files 
**  BIB_INPUT_PATH    - fallback search path for .bib files
**  BST_INPUT_ENVVAR  - environment variable specifying the search path for
**                      .bst files 
**  BST_INPUT_PATH    - fallback search path for .bst files
**  CSF_INPUT_ENVVAR  - environment variable specifying the search path for
**                      .csf files 
**  CSF_INPUT_PATH    - fallback search path for .csf files
**
**  CSF_FILE_ENVVAR   - environment variable specifying the default .csf file
**  CSF_FILE_NAME     - fallback name of the default .csf file
**============================================================================
*/
#ifndef KPATHSEA
#ifndef AUX_INPUT_ENVVAR
# error Macro 'AUX_INPUT_ENVVAR' has not been defined.  Please check Makefile.
#endif

#ifndef AUX_INPUT_PATH
# error Macro 'AUX_INPUT_PATH' has not been defined.  Please check Makefile.
#endif

#ifndef BIB_INPUT_ENVVAR
# error Macro 'BIB_INPUT_ENVVAR' has not been defined.  Please check Makefile.
#endif

#ifndef BIB_INPUT_PATH
# error Macro 'BIB_INPUT_PATH' has not been defined.  Please check Makefile.
#endif

#ifndef BST_INPUT_ENVVAR
# error Macro 'BST_INPUT_ENVVAR' has not been defined.  Please check Makefile.
#endif

#ifndef BST_INPUT_PATH
# error Macro 'BST_INPUT_PATH' has not been defined.  Please check Makefile.
#endif

#ifndef CSF_INPUT_ENVVAR
# error Macro 'CSF_INPUT_ENVVAR' has not been defined.  Please check Makefile.
#endif

#ifndef CSF_INPUT_PATH
# error Macro 'CSF_INPUT_PATH' has not been defined.  Please check Makefile.
#endif

#ifndef CSF_FILE_ENVVAR
# error Macro 'CSF_FILE_ENVVAR' has not been defined.  Please check Makefile.
#endif

#ifndef CSF_FILE_NAME
# error Macro 'CSF_FILE_NAME' has not been defined.  Please check Makefile.
#endif
#endif /* ! KPATHSEA */


#endif                          /* __SYSDEP_H__ */
