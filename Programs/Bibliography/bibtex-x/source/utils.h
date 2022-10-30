/*-
******************************************************************************
******************************************************************************
**
**  MODULE
**
**      file: utils.h
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
**      new utility functions that will be used in the program.  The 
**      functions are declared in alphabetical order.
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
**      $Log: utils.h,v $
**      Revision 3.71  1996/08/18  20:37:06  kempson
**      Official release 3.71 (see HISTORY file for details).
**
**      Revision 3.70  1996/04/08  10:08:40  kempson
**      Final documentation & cosmetic changes for official release 3.70.
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
#ifndef __UTILS_H__
#define __UTILS_H__                 1


#ifdef WIN32
#  undef CDECL
#  define CDECL __cdecl
#else
#  define CDECL
#endif /* WIN32 */


/*
** Declaration of public functions defined in utils.c
*/
void                    close_file (const AlphaFile_T file_pointer);
void CDECL            debug_msg (const int status, const char *printf_fmt, ...);
#ifndef KPATHSEA
int                     find_file (const char *envvar,
                                const char *fallback_path,
                                const char *filename, char *full_file_spec);
#endif
FILE                   *open_ip_file (Integer_T search_path);
FILE                   *open_op_file (void);
void                   *mymalloc (const unsigned long bytes_required,
				const char *var_name);
void                   *myrealloc (void *old_ptr, const unsigned long bytes_required,
				const char *var_name);
void                    parse_cmd_line (int argc, char **argv);
void                    report_bibtex_capacity (void);
void                    report_search_paths (void);
void		        set_array_sizes (void);
void CDECL            usage (const char *printf_fmt, ...);
                                                              

#ifdef SUPPORT_8BIT
void                    c8initialise (void);
#endif                          /* SUPPORT_8BIT */


/*
** Debugging categories.
*/
#define DBG_ALL                 255
#define DBG_CSF                 1
#define DBG_IO                  2
#define DBG_MEM                 4
#define DBG_MISC                8
#define DBG_SRCH                16

#endif                          /* __UTILS_H__ */
