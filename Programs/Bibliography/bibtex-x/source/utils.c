/*-
******************************************************************************
******************************************************************************
**
**  MODULE
**
**      $RCSfile: utils.c,v $
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
**      This module contains the new utility functions that will be used in
**      the program.  The functions are declared in alphabetical order.
**      Functions defined in this module are:
**
**          close_file
**          debug_msg
**          find_file
**          open_ip_file
**          open_op_file
**          mymalloc
**          myrealloc
**          parse_cmd_line
**          report_bibtex_capacity
**          report_search_paths
**          set_array_sizes
**          usage
**
**  8-bit support functions:
**
**          c8read_csf
**          c8initialise
**          c8read_lowercase
**          c8read_lowupcase
**          c8read_uppercase
**          c8read_order
**          c8report_8bit_handling
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
**      $Log: utils.c,v $
**      Revision 3.71  1996/08/18  20:37:06  kempson
**      Official release 3.71 (see HISTORY file for details).
**
**      Revision 3.70  1996/04/08  10:08:40  kempson
**      Final documentation & cosmetic changes for official release 3.70.
**
**      Revision 3.6  1995/10/21  22:20:42  kempson
**      Fixed numerous bugs, improved error reporting and added the --wolfgang
**      option.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif

#include <stdarg.h>
#ifdef WIN32
#include <getopt.h>
#else
#include <unistd.h>
#endif

#ifdef KPATHSEA
#include <kpathsea/config.h>
#include <kpathsea/c-fopen.h>
#include <kpathsea/tex-file.h>
#include <kpathsea/paths.h>
#include <kpathsea/variable.h>
#include <kpathsea/lib.h>
#ifndef WIN32
#include <kpathsea/getopt.h>
#endif
#endif

#include "sysdep.h"
#include "bibtex.h"
#include "datatype.h"
#include "gblprocs.h"
#include "gblvars.h"
#include "utils.h"
#include "version.h"


/*
** Useful macros to keep the code nicely formatted.
*/
#define FSE(_a)         fprintf (stderr, _a);
#define FSO(_a)         fprintf (TERM_OUT, _a);
#define LOG_CAPACITY(_a) \
        FPRINTF (log_file, "    %-15s = %7ld\n", #_a, (long) _a)
#define ISEMPTYSTR(_a)  ((_a == NULL) || (*_a == '\0'))
#define NULLCHECK(_a)   (ISEMPTYSTR(_a) ? "<undefined>" : _a)


/*-
** Options supported by the program and parsed by getopt().  Each option
** can be specified by a "traditional" single character argument
** preceeded by a dash (e.g. "-7") or a more descriptive parameter
** preceeded by two dashes (e.g. "--ascii").  Each option is also defined
** as either not taking a value, optinally taking a value or requiring a
** value.
*/
#define VALUE_NONE      0
#define VALUE_REQD      1
#define VALUE_OPT       2

static struct option long_options[] = {
#ifndef UTF_8
    {"8bit",            VALUE_NONE, 0, '8'},
    {"csfile",          VALUE_REQD, 0, 'c'},
#endif
    {"debug",           VALUE_REQD, 0, 'd'},
    {"help",            VALUE_NONE, 0, '?'},
    {"statistics",      VALUE_NONE, 0, 's'},
    {"trace",           VALUE_NONE, 0, 't'},
#ifndef UTF_8
    {"traditional",     VALUE_NONE, 0, '7'},
#endif
    {"version",         VALUE_NONE, 0, 'v'},

#ifdef UTF_8
    {"language",        VALUE_REQD, 0, 'l'},
    {"location",        VALUE_REQD, 0, 'o'},
#endif

    {"big",             VALUE_NONE, 0, 'B'},
    {"huge",            VALUE_NONE, 0, 'H'},
    {"wolfgang",        VALUE_NONE, 0, 'W'},
    {"min_crossrefs",   VALUE_REQD, 0, 'M'},

    {"mcites",          VALUE_REQD, 0, '\x0A'}, /* obsolete */
    {"mentints",        VALUE_REQD, 0, '\x0B'}, /* obsolete */
    {"mentstrs",        VALUE_REQD, 0, '\x0C'}, /* obsolete */
    {"mfields",         VALUE_REQD, 0, '\x0D'}, /* obsolete */
    {"mpool",           VALUE_REQD, 0, '\x0E'}, /* obsolete */
    {"mstrings",        VALUE_REQD, 0, '\x0F'},
    {"mwizfuns",        VALUE_REQD, 0, '\x10'}, /* obsolete */
    {0, 0, 0, 0}
};

static const char *getopt_str =
#ifndef UTF_8
  "78c:"
#endif
  "d:?stv"
#ifdef UTF_8
  "l:o:"
#endif
  "BHM:W";



/*
** Forward declaration of private functions and variables used in this
** module for 8-bit support.
*/
#ifdef SUPPORT_8BIT
static int              c8read_csf (void);
static void             c8read_lowupcase (void);
static void             c8read_lowercase (void);
static void             c8read_order (void);
static void             c8read_uppercase (void);
static void             c8report_8bit_handling (void);

static FILE            *c8_cs_file;
static int              c8_line;
static int              sort_weight;
#endif                          /* SUPPORT_8BIT */



/*-
**============================================================================
** allocate_arrays()
**
**  Allocate memory dynamically for the large arrays whose size is fixed,
**  regardless of the amount memory of memory available. If these arrays are
**  declared statically, the linker will complain about an overflowing text
**  segment.
**
**  Allocate memory dynamically for the large arrays whose size is set
**  dynamically, depending on the amount memory of memory available.
**
**	AlphaFile_T     bib_file[Max_Bib_Files + 1];
**	StrNumber_T     bib_list[Max_Bib_Files + 1];
**	ASCIICode_T     buffer[Buf_Size + 1];
**	StrNumber_T     cite_info[Max_Cites + 1];
**	StrNumber_T     cite_list[Max_Cites + 1];
**	Boolean_T       entry_exists[Max_Cites + 1];
**	Integer_T       entry_ints[Max_Ent_Ints + 1];
**	ASCIICode_T	entry_strs[Max_Ent_Strs + 1][Ent_Str_Size + 1];
**	ASCIICode_T     ex_buf[Buf_Size + 1];
**	StrNumber_T     field_info[Max_Fields + 1];
**	FnClass_T       fn_type[Hash_Size + 1];
**      Integer_T       glb_str_end[Max_Glob_Strs];
**      StrNumber_T     glb_str_ptr[Max_Glob_Strs];
**	ASCIICode_T     global_strs[Max_Glob_Strs][Glob_Str_Size + 1];;
**	StrIlk_T        hash_ilk[Hash_Size + 1];
**	HashPointer_T   hash_next[Hash_Size + 1];
**	StrNumber_T     hash_text[Hash_Size + 1];
**	Integer_T       ilk_info[Hash_Size + 1];
**      Integer_T       lit_stack[Lit_Stk_Size + 1];
**      StkType_T       lit_stk_type[Lit_Stk_Size + 1];
**	ASCIICode_T     name_sep_char[Buf_Size + 1];
**	BufPointer_T    name_tok[Buf_Size + 1];
**	ASCIICode_T     out_buf[Buf_Size + 1];
**	StrNumber_T     s_preamble[Max_Bib_Files + 1];
**	ASCIICode_T     str_pool[Pool_Size + 1];
**	PoolPointer_T   str_start[Max_Strings + 1];
**	ASCIICode_T     sv_buffer[Buf_Size + 1];
**	HashPtr2_T      type_list[Max_Cites + 1];
**	HashPtr2_T      wiz_functions[Wiz_Fn_Space + 1];
**============================================================================
*/
static void allocate_arrays (void)
{
    unsigned long           bytes_required;

    debug_msg (DBG_MEM, "Starting to allocate memory for arrays ... ");

    /*
    ** AlphaFile_T bib_file[Max_Bib_Files + 1];
    */
    bytes_required = (Max_Bib_Files + 1) * (unsigned long) sizeof (AlphaFile_T);
    bib_file = (AlphaFile_T *) mymalloc (bytes_required, "bib_file");

    /*
    ** StrNumber_T bib_list[Max_Bib_Files + 1];
    */
    bytes_required = (Max_Bib_Files + 1) * (unsigned long) sizeof (StrNumber_T);
    bib_list = (StrNumber_T *) mymalloc (bytes_required, "bib_list");

    /*
    ** ASCIICode_T buffer[Buf_Size + 1];
    */
    bytes_required = (Buf_Size + 1) * (unsigned long) sizeof (ASCIICode_T);
    buffer = (ASCIICode_T *) mymalloc (bytes_required, "buffer");

    /*
    ** StrNumber_T cite_info[Max_Cites + 1];
    */
    bytes_required = (Max_Cites + 1) * (unsigned long) sizeof (StrNumber_T);
    cite_info = (StrNumber_T *) mymalloc (bytes_required, "cite_info");

    /*
    ** StrNumber_T cite_list[Max_Cites + 1];
    */
    bytes_required = (Max_Cites + 1) * (unsigned long) sizeof (StrNumber_T);
    cite_list = (StrNumber_T *) mymalloc (bytes_required, "cite_list");

    /*
    ** Boolean_T entry_exists[Max_Cites + 1];
    */
    bytes_required = (Max_Cites + 1) * (unsigned long) sizeof (Boolean_T);
    entry_exists = (Boolean_T *) mymalloc (bytes_required, "entry_exists");

    /*
    ** Boolean_T entry_ints[Max_Ent_Ints + 1];
    ** allocated when num_ent_ints and num_cites are known
    */
    entry_ints = NULL;

    /*
    ** ASCIICode_T entry_strs[Max_Ent_Strs + 1][Ent_Str_Size + 1];
    ** allocated when num_ent_strs and num_cites are known
    */
    entry_strs = NULL;

    /*
    ** ASCIICode_T ex_buf[Buf_Size + 1];
    */
    bytes_required = (Buf_Size + 1) * (unsigned long) sizeof (ASCIICode_T);
    ex_buf = (ASCIICode_T *) mymalloc (bytes_required, "ex_buf");

    /*
    ** StrNumber_T field_info[Max_Fields + 1];
    */
    bytes_required = (Max_Fields + 1) * (unsigned long) sizeof (StrNumber_T);
    field_info = (StrNumber_T *) mymalloc (bytes_required, "field_info");

    /*
    ** FnClass_T fn_type[Hash_Size + 1];
    */
    bytes_required = (Hash_Size + 1) * (unsigned long) sizeof (FnClass_T);
    fn_type = (FnClass_T *) mymalloc (bytes_required, "fn_type");

    /*
    ** Integer_T glb_str_end[Max_Glob_Strs];
    */
    bytes_required = Max_Glob_Strs * (unsigned long) sizeof (Integer_T);
    glb_str_end = (Integer_T *) mymalloc (bytes_required, "glb_str_end");

    /*
    ** StrNumber_T glb_str_ptr[Max_Glob_Strs];
    */
    bytes_required = Max_Glob_Strs * (unsigned long) sizeof (StrNumber_T);
    glb_str_ptr = (StrNumber_T *) mymalloc (bytes_required, "glb_str_ptr");

    /*
    ** ASCIICode_T global_strs[Max_Glob_Strs][Glob_Str_Size + 1];
    */
    bytes_required = (unsigned long) (Max_Glob_Strs)
        * (Glob_Str_Size + 1)
        * (unsigned long) sizeof (ASCIICode_T);
    global_strs = (ASCIICode_T *) mymalloc (bytes_required, "global_strs");

    /*
    ** StrIlk_T hash_ilk[Hash_Size + 1];
    */
    bytes_required = (Hash_Size + 1) * (unsigned long) sizeof (StrIlk_T);
    hash_ilk = (StrIlk_T *) mymalloc (bytes_required, "hash_ilk");

    /*
    ** HashPointer_T hash_next[Hash_Size + 1];
    */
    bytes_required = (Hash_Size + 1) * (unsigned long) sizeof (HashPointer_T);
    hash_next = (HashPointer_T *) mymalloc (bytes_required, "hash_next");

    /*
    ** StrNumber_T hash_text[Hash_Size + 1];
    */
    bytes_required = (Hash_Size + 1) * (unsigned long) sizeof (StrNumber_T);
    hash_text = (StrNumber_T *) mymalloc (bytes_required, "hash_text");

    /*
    ** Integer_T ilk_info[Hash_Size + 1];
    */
    bytes_required = (Hash_Size + 1) * (unsigned long) sizeof (Integer_T);
    ilk_info = (Integer_T *) mymalloc (bytes_required, "ilk_info");

    /*
    ** Integer_T lit_stack[Lit_Stk_Size + 1];
    */
    bytes_required = (Lit_Stk_Size + 1) * (unsigned long) sizeof (Integer_T);
    lit_stack = (Integer_T *) mymalloc (bytes_required, "lit_stack");

    /*
    ** StkType_T lit_stk_type[Lit_Stk_Size + 1];
    */
    bytes_required = (Lit_Stk_Size + 1) * (unsigned long) sizeof (StkType_T);
    lit_stk_type = (StkType_T *) mymalloc (bytes_required, "lit_stk_type");

    /*
    ** ASCIICode_T name_sep_char[Buf_Size + 1];
    */
    bytes_required = (Buf_Size + 1) * (unsigned long) sizeof (ASCIICode_T);
    name_sep_char = (ASCIICode_T *) mymalloc (bytes_required, "name_sep_char");

    /*
    ** BufPointer_T name_tok[Buf_Size + 1];
    */
    bytes_required = (Buf_Size + 1) * (unsigned long) sizeof (BufPointer_T);
    name_tok = (BufPointer_T *) mymalloc (bytes_required, "name_tok");

    /*
    ** ASCIICode_T out_buf[Buf_Size + 1];
    */
    bytes_required = (Buf_Size + 1) * (unsigned long) sizeof (ASCIICode_T);
    out_buf = (ASCIICode_T *) mymalloc (bytes_required, "out_buf");

    /*
    ** StrNumber_T s_preamble[Max_Bib_Files + 1];
    */
    bytes_required = (Max_Bib_Files + 1) * (unsigned long) sizeof (StrNumber_T);
    s_preamble = (StrNumber_T *) mymalloc (bytes_required, "s_preamble");

    /*
    ** ASCIICode_T str_pool[Pool_Size + 1];
    */
    bytes_required = (Pool_Size + 1) * (unsigned long) sizeof (ASCIICode_T);
    str_pool = (ASCIICode_T *) mymalloc (bytes_required, "str_pool");

    /*
    ** PoolPointer_T str_start[Max_Strings + 1];
    */
    bytes_required = (Max_Strings + 1) * (unsigned long) sizeof (PoolPointer_T);
    str_start = (PoolPointer_T *) mymalloc (bytes_required, "str_start");

    /*
    ** ASCIICode_T sv_buffer[Buf_Size + 1];
    */
    bytes_required = (Buf_Size + 1) * (unsigned long) sizeof (ASCIICode_T);
    sv_buffer = (ASCIICode_T *) mymalloc (bytes_required, "sv_buffer");

    /*
    ** HashPtr2_T type_list[Max_Cites + 1];
    */
    bytes_required = (Max_Cites + 1) * (unsigned long) sizeof (HashPtr2_T);
    type_list = (HashPtr2_T *) mymalloc (bytes_required, "type_list");

    /*
    ** HashPtr2_T wiz_functions[Wiz_Fn_Space + 1];
    */
    bytes_required = (Wiz_Fn_Space + 1) * (unsigned long) sizeof (HashPtr2_T);
    wiz_functions = (HashPtr2_T *) mymalloc (bytes_required, "wiz_functions");

}                               /* allocate_arrays () */



/*-
**============================================================================
** checkdbg()
**
**  Examine a string representing different debug options, usually provided 
**  as a value to a command line option, and return the value as a long 
**  integer.  Return 0 (no debugging) if a valid option was not parsed.
**============================================================================
*/
static int checkdbg (char *str)
{
    int                 dbgval = 0;
    
    if ((str == NULL) || (*str == '\0'))
        return (0);

    if (strstr (str, "all") != NULL)
        dbgval |= DBG_ALL;
    if (strstr (str, "csf") != NULL)
        dbgval |= DBG_CSF;
    if (strstr (str, "io") != NULL)
        dbgval |= DBG_IO;
    if (strstr (str, "mem") != NULL)
        dbgval |= DBG_MEM;
    if (strstr (str, "misc") != NULL)
        dbgval |= DBG_MISC;
    if (strstr (str, "search") != NULL)
        dbgval |= DBG_SRCH;

    return (dbgval);
}                               /* checkdbg() */



/*-
**============================================================================
** checklong()
**
**  Examine a string representing an integer, usually provided as a value to
**  a command line option, and return the value as a long integer.
**  Return -1 if a valid integer was not parsed.
**============================================================================
*/
static long checklong (char *str)
{
    long                value;
    char               *endptr;
    int                 len;
    
    len = strlen (str);
    value = strtol (str, &endptr, 10);

    if (str == endptr)
        return (-1);
    if ((endptr - str) < len)
        return (-1);

    return (value);
}                               /* checklong() */



/*-
**============================================================================
** close_file()
**
**  Close a file - as long as it's open.
**============================================================================
*/
void close_file (const AlphaFile_T file_pointer)
{
    if (file_pointer != NULL)
        fclose (file_pointer);
}                               /* close_file() */



/*-
**============================================================================
** debug_msg()
**
**  Write a debugging message to the stderr stream, but only if the
**  appropriate debugging option has been selected.
**============================================================================
*/
void debug_msg (const int status, const char *printf_fmt, ...)
{
    va_list             printf_args;
    const char         *prefix;    

    switch (status) {
        case DBG_CSF:
            if ((Flag_debug & DBG_CSF) == 0) return;
            prefix = "D-CSF";
            break;
        case DBG_IO:
            if ((Flag_debug & DBG_IO) == 0) return;
            prefix = "D-I/O";
            break;
        case DBG_MEM:
            if ((Flag_debug & DBG_MEM) == 0) return;
            prefix = "D-MEM";
            break;
        case DBG_MISC:
            if ((Flag_debug & DBG_MISC) == 0) return;
            prefix = "D-MSC";
            break;
        case DBG_SRCH:
            if ((Flag_debug & DBG_SRCH) == 0) return;
            prefix = "D-SCH";
            break;
        default:
            return;
    }

    if (printf_fmt != NULL) {
        fprintf (stderr, "%s: ", prefix);
        va_start (printf_args, printf_fmt);
        vfprintf (stderr, printf_fmt, printf_args);
        va_end (printf_args);
        fprintf (stderr, "\n");
        fflush (stderr);
    }
}                               /* debug_msg () */


#ifndef KPATHSEA
/*-
**============================================================================
** find_file()
**
**  Search along a path for a file.  The search path is specified by the
**  value of an environment variable which can contain multiple directories,
**  separated by an appropriate delimiter.  The best known example is the
**  PATH environment variable found on DOS, OS/2 and Unix.
**
**  If the environment variable is not specified, or it has not been set,
**  a 'fallback' path will be used.  The current working directory will be
**  searched first, then directories along the search path define dby the
**  environment variable or fallback path.
**
**  If the file is found along the search path, a value of 0 is returned and 
**  the full file specification is stored in parameter full_file_spec.  If
**  the file was not found, -1 is returned.
**============================================================================
*/
int find_file (const char *envvar_name, const char *fallback_path,
               const char *filename, char *full_file_spec)
{
    char                c;
    int                 len = 0;
    char               *search_path;

    debug_msg (DBG_SRCH, "find_file: searching for file name `%s'",
               filename);
    debug_msg (DBG_SRCH,
               "find_file: environment variable for search path = `%s'", 
               (envvar_name == NULL) ? "NULL" : envvar_name);
    debug_msg (DBG_SRCH, "find_file: fallback path = `%s'",
               (fallback_path == NULL) ? "NULL" : fallback_path);

    if ((envvar_name == NULL) || (*envvar_name == '\0')) {
        search_path = NULL;
        debug_msg (DBG_SRCH, 
                   "find_file: no environment variable specified");    
    } else {
        search_path = getenv (envvar_name);
        debug_msg (DBG_SRCH, "find_file: %s = `%s'", envvar_name, 
                   (search_path == NULL) ? "NULL" : search_path);
    }

    /*
    ** If we have a NULL search path because an environment variable name was 
    ** not specified or specified but not defined, we use the fallback
    ** search path.
    */
    if (search_path == NULL) {
        debug_msg (DBG_SRCH, "find_file: using fallback search path `%s'", 
                   (fallback_path == NULL) ? "NULL" : fallback_path);
        search_path = (char *) fallback_path;
    }


    /*
    ** Work along the search path looking for the wanted file.  Stop when
    ** the first match is found.
    */
    while (1) {
        full_file_spec[len] = '\0';
        
        /*
        ** If the path is not empty, and it doesn't end in '/' or '\',
        ** append a trailing '/'.
        */
        if (len != 0) {
            c = full_file_spec[len - 1];
            if ((c != '/') && (c != '\\'))
                strcat (full_file_spec, "/");
        }

        /*
        ** Append the filename to the path and then test whether the
        ** file exists.  If not, return -1 and an empty string.
        */
        strcat (full_file_spec, filename);
        debug_msg (DBG_SRCH, "find_file: trying `%s'", full_file_spec);

        if (access (full_file_spec, 0) == 0)
            break;

        if ((search_path == NULL) || (*search_path == '\0')) {
            full_file_spec[0] = '\0';
            return (-1);
        }

        len = 0;

        for (;;) {
            if ((*search_path != PATH_DELIM) && (*search_path != '\0')) {
                if (*search_path == '\\')
                    full_file_spec[len] = '/';
                else
                    full_file_spec[len] = *search_path;

                ++len;
                ++search_path;    
            } else
                break;
        }                       /* end for (;;) */

        if (*search_path != '\0')
            ++search_path;
    }                           /* end while (1) */    

    debug_msg (DBG_SRCH, "find_file: returning `%s'", full_file_spec);
    return (0);
}                               /* find_file() */
#endif /* ! KPATHSEA */


/*-
**============================================================================
** mymalloc()
**
**  Allocate memory for a specified array and check whether the allocation 
**  was successful.  If not, issue an error message and cause the program
**  to stop with a fatal exit status.
**  Allocate at least 1 byte, otherwise malloc may return NULL.
**============================================================================
*/
void *mymalloc (const unsigned long bytes_required, const char *array_name)
{
    void               *ptr;

    ptr = malloc (bytes_required ? bytes_required : 1);

    if (ptr == NULL) {
        printf ("\nFatal error: couldn't allocate %lu bytes for array `%s'",
                bytes_required, array_name);
        mark_fatal ();
        debug_msg (DBG_MISC, "calling longjmp (Exit_Program_Flag) ... ");
        longjmp (Exit_Program_Flag, 1);
    } else {
        debug_msg (DBG_MEM, "allocated %7lu bytes for array `%s'",
	           bytes_required, array_name);
    }

    return (ptr);
}                               /* mymalloc () */



/*-
**============================================================================
** myrealloc()
**
**  Reallocate memory for a specified array and check whether the allocation 
**  was successful.  If not, issue an error message and cause the program
**  to stop with a fatal exit status.
**  Allocate at least 1 byte, otherwise realloc may return NULL.
**============================================================================
*/
void *myrealloc (void *old_ptr, const unsigned long bytes_required, const char *array_name)
{
    void               *ptr;

    if (old_ptr == NULL)
        return mymalloc (bytes_required, array_name);

    ptr = realloc (old_ptr, bytes_required ? bytes_required : 1);

    if (ptr == NULL) {
        printf ("\nFatal error: couldn't reallocate %lu bytes for array `%s'",
                bytes_required, array_name);
        mark_fatal ();
        debug_msg (DBG_MISC, "calling longjmp (Exit_Program_Flag) ... ");
        longjmp (Exit_Program_Flag, 1);
    } else {
        debug_msg (DBG_MEM, "reallocated %7lu bytes for array `%s'",
	           bytes_required, array_name);
    }

    return (ptr);
}                               /* myrealloc () */



/*-
**============================================================================
** open_ip_file()
**
**  Open a file for input after locating it using one of the five predefined
**  search strategies defined by the parameter "search_path".  This parameter
**  can take 5 values:
**
**  AUX_FILE  - used for locating .aux files.  Search for the file along the
**              path defined by the environment variable AUX_INPUT_ENVVAR or
**              if it is not defined, use the fallback path defined by 
**              AUX_INPUT_PATH.
**
**  BIB_FILE  - used for locating .bib files.  Search for the file along the
**              path defined by the environment variable BIB_INPUT_ENVVAR or
**              if it is not defined, use the fallback path defined by 
**              BIB_INPUT_PATH.
**
**  BST_FILE  - used for locating .bst files.  Search for the file along the
**              path defined by the environment variable BST_INPUT_ENVVAR or
**              if it is not defined, use the fallback path defined by 
**              BST_INPUT_PATH.
**
**  CSF_FILE  - used for locating .csf files.  Search for the file along the
**              path defined by the environment variable CSF_INPUT_ENVVAR or
**              if it is not defined, use the fallback path defined by 
**              CSF_INPUT_PATH.
**
**  NO_SEARCH_PATH  - only look for the file in the current directory.
**
**  Searching for the file is delegated to the function find_file(), and
**  only if it was successful, does this function attempt to open the file.
**============================================================================
*/
FILE *open_ip_file (Integer_T search_path)
{
    const char          *filename = (const char *) name_of_file;
#ifdef KPATHSEA
    string              full_filespec;
#else
    char                full_filespec[FILE_NAME_SIZE + 1];
#endif
    FILE               *fptr;
    int                 status;
      
    switch (search_path) {
        case AUX_FILE_SEARCH_PATH:
#ifdef KPATHSEA
	  status = ((full_filespec = kpse_find_file(filename, kpse_tex_format, false)) == NULL);
#else
            status = find_file (AUX_INPUT_ENVVAR, AUX_INPUT_PATH,
                                filename, full_filespec);
#endif
            break;
        case BIB_FILE_SEARCH_PATH:
#ifdef KPATHSEA
	  status = ((full_filespec = kpse_find_file(filename, kpse_bib_format, false)) == NULL);
#else
            status = find_file (BIB_INPUT_ENVVAR, BIB_INPUT_PATH,
                                filename, full_filespec);
#endif
            break;
        case BST_FILE_SEARCH_PATH:
#ifdef KPATHSEA
	  status = ((full_filespec = kpse_find_file(filename, kpse_bst_format, false)) == NULL);
#else
            status = find_file (BST_INPUT_ENVVAR, BST_INPUT_PATH,
                                filename, full_filespec);
#endif
            break;
        case CSF_FILE_SEARCH_PATH:
#ifdef KPATHSEA
	  status = ((full_filespec = kpse_find_file(filename, kpse_bst_format, false)) == NULL);
#else
            status = find_file (CSF_INPUT_ENVVAR, CSF_INPUT_PATH,
                                filename, full_filespec);
#endif
            break;
        case NO_SEARCH_PATH:
        default:
#ifdef KPATHSEA
	  status = ((full_filespec = kpse_find_file(filename, kpse_program_text_format, false)) == NULL);
#else
            status = find_file (NULL, NULL, filename, full_filespec);
#endif
            break;
    }                           /* end switch (search_path) */

    /*
    ** find_file() will return zero if the file was found somewhere
    ** along the search path.  If it does, try to open the file.
    */
    if (status == 0) {
        debug_msg (DBG_IO, "open_ip_file: trying to open `%s' ... ", 
                   full_filespec);
#ifdef KPATHSEA
	if (!kpse_in_name_ok(full_filespec))
	    goto not_ok;
	fptr = fopen (full_filespec, FOPEN_R_MODE);
	free (full_filespec);
#else
# if defined(MSDOS) || defined(OS2)
        fptr = fopen (full_filespec, "rt");
# endif

# if defined(UNIX) || defined(VMS)
        fptr = fopen (full_filespec, "r");
#endif
#endif /* ! KPATHSEA */
    }

    /*
    ** Otherwise, return a NULL pointer.
    */
    else {
not_ok:
        debug_msg (DBG_IO, "open_ip_file: unable to open `%s' ... ", 
                   full_filespec);
        fptr = NULL;
    }
    return (fptr);
}                               /* open_ip_file() */



/*-
**============================================================================
** open_op_file()
**
**  Open a file for output in the current working directory.  If the operating 
**  system makes a distinction between text and non-text files, make sure
**  that the file is opened in "text" mode.
**============================================================================
*/
FILE *open_op_file (void)
{
    const char         *tmp_file_name = (const char *) name_of_file;
    FILE               *fptr;
      
    debug_msg (DBG_IO, "open_op_file: trying to open `%s' ... ",
               tmp_file_name);

    /*
    ** Make sure that we open the output file as a "text" file.  This
    ** varies according to the operating system.
    */
#if defined(KPATHSEA)
    if (kpse_out_name_ok(tmp_file_name))
	fptr = fopen(tmp_file_name, FOPEN_W_MODE);
    else
	fptr = NULL;
#else
# if defined(MSDOS) || defined(OS2)
    fptr = fopen (tmp_file_name, "wt");
# endif

# ifdef UNIX
    fptr = fopen (tmp_file_name, "w");
# endif

# ifdef VMS
    fptr = fopen (tmp_file_name, "w", "rfm=var", "rat=cr");
# endif
#endif /* ! KPATHSEA */

    if (fptr == NULL) {
        printf ("open_op_file: error opening `%s'\n", tmp_file_name);
    }
    return (fptr);
}                               /* open_op_file() */



/*-
**============================================================================
** parse_cmd_line()
**
**  Parse the command line used to launch BibTeX and set options accordingly.
**  Most of the work is done by the GNU getopt() function to support these
**  long and short options:
**
**      -?  --help              display this help text
**      -7  --traditional       operate in the traditional 7-bit mode
**      -8  --8bit              force 8-bit mode, even if no CS file
**      -c  --csfile FILE       read FILE as the BibTeX character set
**                              and sort definition file
**      -d  --debug TYPE        report debugging information.  TYPE is one or
**                              more of all, csf, io, mem, misc, search.
**      -s  --statistics        report internal statistics
**      -t  --trace             report execution tracing
**      -v  --version           report BibTeX version\n
**      -B  --big               set large BibTeX capacity
**      -H  --huge              set huge BibTeX capacity
**      -W  --wolfgang          set really huge BibTeX capacity for Wolfgang
**      -M  --min_crossrefs ##  set ## as the minimum number of cross refs
**                              required for automatic inclusion of the cross
**                              referenced entry in the citation list
**                              (default = 2)
**          --mcites ##         ignored
**          --mentints ##       ignored
**          --mentstrs ##       ignored
**          --mfields ##        ignored
**          --mpool ##          ignored
**          --mstrings ##       allow ## unique strings
**          --mwizfuns ##       ignored
**============================================================================
*/
void parse_cmd_line (int argc, char **argv)
{
    int                 c;
    int                 no_files;

    Flag_7bit = FALSE;
    Flag_8bit = FALSE;
    Flag_big = FALSE;
    Flag_debug = FALSE;
    Flag_huge = FALSE;
    Flag_wolfgang = FALSE;
    Flag_stats = FALSE;
    Flag_trace = FALSE;
    Str_auxfile = NULL;
    Str_csfile = NULL;
#ifdef UTF_8
    Flag_language = FALSE;
    Str_language = NULL;
    Flag_location = FALSE;
    Str_location = NULL;
#endif

    while (1) {
        int             option_index = 0;

        c = getopt_long (argc, argv, getopt_str, long_options, &option_index);

        if (c == EOF)
            break;

        switch (c) {
            case '?':       /**************** -?, --help ***************/
                usage (NULL);
                break;

#ifndef UTF_8
            case '7':       /**************** -7, --traditional ********/
                Flag_7bit = TRUE;
                break;

            case '8':       /**************** -8, --8bit ***************/
                Flag_8bit = TRUE;
                break;
#endif

            case 'B':       /**************** -B, --big ****************/
                Flag_big = TRUE;
                break;

#ifndef UTF_8
            case 'c':       /**************** -c, --csfile *************/
                Str_csfile = strdup (optarg);
                break;
#endif

            case 'd':       /**************** -d, --debug **************/
                if ((optarg == NULL) || (*optarg == '\0'))
                    Flag_debug = DBG_ALL;
                else
                    Flag_debug = checkdbg (optarg);
                break;

            case 'H':       /**************** -H, --huge ***************/
                Flag_huge = TRUE;
                break;

#ifdef UTF_8
            case 'l':       /**************** -l, --language ***********/
                Flag_language =TRUE;
                Str_language = optarg;
                break;
#endif

            case 'M':       /**************** -M, --min_crossrefs ******/
                M_min_crossrefs = checklong (optarg);
                if (M_min_crossrefs < 0) {
                    mark_fatal ();
                    usage ("invalid minimum cross references `%s'\n", optarg);
                }
                break;

#ifdef UTF_8
            case 'o':       /**************** -o, --location ***********/
                Flag_location =TRUE;
                Str_location = optarg;
                break;
#endif

            case 's':       /**************** -s, --statistics *********/
                Flag_stats = TRUE;
                break;

            case 't':       /**************** -t, --trace **************/
                Flag_trace = TRUE;
                break;

            case 'v':       /**************** -v, --version ************/
                FPRINTF (TERM_OUT, "%s\n", BANNER);
                FPRINTF (TERM_OUT, "Implementation:  %s\n", IMPLEMENTATION);
                FPRINTF (TERM_OUT, "Release version: %s\n", VERSION);
                debug_msg (DBG_MISC, 
                           "calling longjmp (Exit_Program_Flag) ... ");
                longjmp (Exit_Program_Flag, 1);
                break;

            case 'W':       /**************** -W, --wolfgang ***********/
                Flag_wolfgang = TRUE;
                break;

            case '\x0A':    /**************** --mcites *****************/
            case '\x0B':    /**************** --mentints ***************/
            case '\x0C':    /**************** --mentstrs ***************/
            case '\x0D':    /**************** --mfields ****************/
            case '\x0E':    /**************** --mpool *****************/
            case '\x10':    /**************** --mwizfuns ***************/
                (void) checklong (optarg);   /******** ignored ********/
                break;

            case '\x0F':    /**************** --mstrings ***************/
                M_strings = checklong (optarg);
                if (M_strings < 0) {
                    mark_fatal ();
                    usage ("invalid max number of strings `%s'\n", optarg);
                }
                break;

	    default:        /**************** Unknown argument ********/
                mark_fatal ();
                usage ("unknown option");
        }                       /* end switch (c) */
    }                           /* end while (1) */


    /*
    ** Check that a single .aux file was specified.
    */
    no_files = argc - optind;

    if (no_files == 1)
        Str_auxfile = argv[optind];
    else if (no_files < 1) {
        mark_fatal ();
        usage ("no aux file specified");
    } else {
        mark_fatal ();
        usage ("only one aux file may be specified");
    }

    /*
    ** Check for contradictory options
    */
    if (Flag_7bit && Flag_8bit) {
        mark_fatal ();
        usage ("can't specify --traditional and --8bit");
    }

    if ((Flag_big && Flag_huge) || (Flag_big && Flag_wolfgang)
                                || (Flag_huge && Flag_wolfgang)) {
        mark_fatal ();
        usage ("can only specify one of --big, --huge and --wolfgang");
    }

    if ((Str_csfile != NULL) && (Flag_7bit)) {
        mark_fatal ();
        usage ("can't specify --csfile and --traditional");
    }

    if ((Str_csfile != NULL) && (Flag_8bit)) {
        mark_fatal ();
        usage ("can't specify --csfile and --8bit");
    }
    
    /*
    ** Check for options which have not been #defined into the code
    */
#ifndef DEBUG
    if (Flag_debug)
        FSO ("BibTeX: debugging support was not compiled into this version\n");
#endif                          /* DEBUG */

#ifndef STAT
    if (Flag_stats)
        FSO ("BibTeX: statistics support was not compiled into this version\n");
#endif                          /* STAT */

#ifndef TRACE 
    if (Flag_trace)
        FSO ("BibTeX: tracing support was not compiled into this version\n");
#endif                          /* TRACE */


}                               /* parse_cmd_line() */



/*-
**============================================================================
** report_bibtex_capacity()
**
**  If the log file has been opened, indicate BibTeX's capacity by reporting
**  the size/values of some key values and arrays.  Anything in mixed case
**  has been set at run-time.
**============================================================================
*/
void report_bibtex_capacity (void)
{
    if (log_file != NULL) {
        FPRINTF (log_file, "BibTeX's capacity set as follows:\n\n");
        LOG_CAPACITY (AUX_STACK_SIZE);
        LOG_CAPACITY (Buf_Size);
        LOG_CAPACITY (Ent_Str_Size);
        LOG_CAPACITY (Glob_Str_Size);
        LOG_CAPACITY (Hash_Prime);
        LOG_CAPACITY (Hash_Size);
        LOG_CAPACITY (Lit_Stk_Size);
        LOG_CAPACITY (Max_Bib_Files);
        LOG_CAPACITY (Max_Cites);
        LOG_CAPACITY (Max_Fields);
        LOG_CAPACITY (Max_Glob_Strs);
        LOG_CAPACITY (MAX_PRINT_LINE);
        LOG_CAPACITY (Max_Strings);
        LOG_CAPACITY (Min_Crossrefs);
        LOG_CAPACITY (MIN_PRINT_LINE);
        LOG_CAPACITY (Pool_Size);
        LOG_CAPACITY (SINGLE_FN_SPACE);
        LOG_CAPACITY (Wiz_Fn_Space);
        FPRINTF (log_file, "\n");
    }
}                               /* report_bibtex_capacity() */



/*-
**============================================================================
** report_search_paths()
**
**  Report the environment variables and fallback search paths used by
**  BibTeX for locating input files.
**============================================================================
*/
void report_search_paths (void)
{
#ifndef KPATHSEA
    /*
    ** Define a couple of useful macros to simplify reporting of the desired
    ** information.
    */
#define REPORT_SEARCH_PATH(_a,_b,_c) \
    debug_msg (DBG_SRCH, "Search strategy for %s files:", _a); \
    debug_msg (DBG_SRCH, "  search path environment variable: %s", NULLCHECK (_b)); \
    if (! ISEMPTYSTR (_b)) { \
        char *ptr; \
        ptr = getenv (_b); \
        debug_msg (DBG_SRCH, "  %s value: %s", _b, NULLCHECK (ptr)); \
    } \
    debug_msg (DBG_SRCH, "  fallback search path: %s", NULLCHECK (_c));


#define REPORT_SEARCH_FILE(_a,_b,_c) \
    debug_msg (DBG_SRCH, "Default %s file:", _a); \
    debug_msg (DBG_SRCH, "  file name environment variable: %s", NULLCHECK (_b)); \
    if (! ISEMPTYSTR (_b)) { \
        char *ptr; \
        ptr = getenv (_b); \
        debug_msg (DBG_SRCH, "  %s value: %s", _b, NULLCHECK (ptr)); \
    } \
    debug_msg (DBG_SRCH, "  fallback file name: %s", NULLCHECK (_c));

    /*
    ** Now report the information.
    */
    REPORT_SEARCH_PATH (".aux", AUX_INPUT_ENVVAR, AUX_INPUT_PATH);
    REPORT_SEARCH_PATH (".bib", BIB_INPUT_ENVVAR, BIB_INPUT_PATH);
    REPORT_SEARCH_PATH (".bst", BST_INPUT_ENVVAR, BST_INPUT_PATH);
    REPORT_SEARCH_PATH (".csf", CSF_INPUT_ENVVAR, CSF_INPUT_PATH);

    REPORT_SEARCH_FILE (".csf", CSF_FILE_ENVVAR, CSF_FILE_NAME);
#endif /* ! KPATHSEA */
}                               /* report_search_paths() */


/*-
**============================================================================
** setup_bound_variable()
**
**  Obtain parameters from environment or configuration file,
**  or use the default value.
**============================================================================
*/
static void setup_bound_variable (Integer_T *var, const char *name,
                                  unsigned long def_value)
{
#ifdef KPATHSEA
    char *expansion = kpse_var_value (name);
    const char *me = kpse_invocation_name;
#if defined(MIKTEX)
    const char *src = " or bibtex-x.ini";
#else
    const char *src = " or texmf.cnf";
#endif
#else
    char *expansion = getenv (name);
    const char *me = PROGNAME;
    const char *src = "";
#endif

    *var = def_value;
    if (expansion) {
        int conf_val = atoi (expansion);
        if (conf_val < def_value)
            fprintf (stderr,
            "%s: Bad value (%ld) in environment%s for %s, keeping %ld.\n",
            me, (long) conf_val, src, name, def_value);
        else
            *var = conf_val;
#ifdef KPATHSEA
        free (expansion);
#endif
    }
}                               /* setup_bound_variable() */

/*-
**============================================================================
** setup_params()
**
**  Determine |ent_str_size|, |glob_str_size|, and |max_strings| from the
**  environment, configuration file, or default value.  Set
**  |hash_size:=max_strings|, but not less than |HASH_SIZE|.
**============================================================================
*/
static void setup_params (void)
{
    setup_bound_variable (&Ent_Str_Size, "ent_str_size", ENT_STR_SIZE);
    setup_bound_variable (&Glob_Str_Size, "glob_str_size", GLOB_STR_SIZE);
    setup_bound_variable (&Max_Strings, "max_strings", MAX_STRINGS);

    /* Obsolete: Max_Strings specified via command line.  */
    if (Flag_big)
        Max_Strings = 10000;
    if (Flag_huge)
        Max_Strings = 19000;
    if (Flag_wolfgang)
        Max_Strings = 30000;
    if (M_strings > 0)
        Max_Strings = M_strings;

    if (Max_Strings < MAX_STRINGS)
        Max_Strings = MAX_STRINGS;
    Hash_Size = Max_Strings;
    if (Hash_Size < HASH_SIZE)
        Hash_Size = HASH_SIZE;
}                               /* setup_params() */


/*-
**============================================================================
** compute_hash_prime()
**
**  We use the algorithm from Knuth's \.{primes.web} to compute
**  |hash_prime| as the smallest prime number not less than 85\%
**  of |hash_size| (and |>=128|).
**
**  This algorithm uses two arrays |primes| and |mult|.  We use the
**  already allocated |hash_next| and |hash_text| for that purpose.
**============================================================================
*/
#define primes hash_next
#define mult hash_text
static void compute_hash_prime (void)
{
    Integer32_T Hash_Want = (Hash_Size / 20) * 17; /* 85\% of |hash_size| */
    Integer32_T k = 1; /*number of prime numbers $p_i$ in |primes| */
    Integer32_T j = 1; /* a prime number candidate */
    Integer32_T o = 2; /* number of odd multiples of primes in |mult| */
    Integer32_T square = 9; /* $p_o^2$ */
    Integer32_T n; /* loop index */
    Boolean_T j_prime; /* is |j| a prime? */

    debug_msg (DBG_MEM, "Computing Hash_Prime ... ");

    primes[k] = Hash_Prime = 2;
    while (Hash_Prime < Hash_Want) {
        do {
            j += 2;
            if (j == square) {
                mult[o++] = j;
                j += 2;
                square = primes[o] * primes[o];
            }
            n = 2;
            j_prime = TRUE;
            while ((n < o) && j_prime) {
                while (mult[n] < j)
                    mult[n] += 2 * primes[n];
                if (mult[n++] == j)
                    j_prime = FALSE;
            }
        } while (!j_prime);
        primes[++k] = Hash_Prime = j;
    }
}                               /* compute_hash_prime() */


/*-
**============================================================================
** set_array_sizes()
**
**  Allocate memory dynamically for the large arrays whose size is set
**  dynamically, depending on the amount memory of memory available.
**
**  On virtual memory operating systems, there is not normally a problem
**  allocating memory; the array sizes can be predetermined and memory
**  allocated at run-time.
**
**    Parameter       Cmd   Standard       --big      --huge  --wolfgang
**    ------------------------------------------------------------------
**    Buf_Size        *** initialy 20000, increased as required ***
**    Hash_Prime      *** computed from Hash_Size ***
**    Hash_Size       *** determined from Max_Strings ***
**    Max_Bib_Files   *** initialy 20, increased as required ***
**    Max_Cites       *** initialy 750, increased as required ***
**    Max_Ent_Ints    *** as required ***
**    Max_Ent_Strs    *** as required ***
**    Max_Fields      *** initialy 5000, increased as required ***
**    Max_Strings     Y        4,000      10,000      19,000      30,000
**    Pool_Size       *** initialy 65,000, increased as required ***
**    Wiz_Fn_Space    *** initialy 3000, increased as required ***
**    ------------------------------------------------------------------
**
**============================================================================
*/
void set_array_sizes (void)
{
    debug_msg (DBG_MEM, "Setting BibTeX's capacity ... ");

    Max_Strings = 4000;
    Min_Crossrefs = 2;

    setup_params ();
    Buf_Size = BUF_SIZE;

    Lit_Stk_Size = LIT_STK_SIZE;

    Max_Bib_Files = MAX_BIB_FILES;

    Max_Cites = MAX_CITES;

    Max_Fields = MAX_FIELDS;

    if (M_min_crossrefs > 0)
        Min_Crossrefs = M_min_crossrefs;

    Pool_Size = POOL_SIZE;

    Wiz_Fn_Space = WIZ_FN_SPACE;


    allocate_arrays ();
    compute_hash_prime ();


    debug_msg (DBG_MEM, "Hash_Prime = %d, Hash_Size = %d", 
               Hash_Prime, Hash_Size);
    debug_msg (DBG_MEM, "Buf_Size = %d, Max_Bib_Files = %d", 
               Buf_Size, Max_Bib_Files);
    debug_msg (DBG_MEM, "Max_Cites = %d, Max_Fields = %d", 
               Max_Cites, Max_Fields);
    debug_msg (DBG_MEM, "Max_Strings = %d, Pool_Size = %d",
               Max_Strings, Pool_Size);
    debug_msg (DBG_MEM, "Min_Crossrefs = %d, Wiz_Fn_Space = %d", 
               Min_Crossrefs, Wiz_Fn_Space);


    /*-
    ** Check that the key values are sensible.  These checks are stolen
    ** from initialize(), but the error messages are more informative.
    */
    debug_msg(DBG_MISC, "Sanity checking capacity values ... ");
    
    if (Hash_Prime < 128)
        usage ("Hash_Prime (%d) must be >= 128", Hash_Prime);
        
    if (Hash_Prime > Hash_Size)
        usage ("Hash_Prime (%d) must be <= Hash_Size (%d)",
               Hash_Prime, Hash_Size);

    /*
    ** The original WEB version of BibTeX imposed a maximum value of
    ** 16321 on Hash_Prime.  A WEB constant was defined:
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
    **     usage ("Hash_Prime (%d) must be < %d", Hash_Prime, (16384-64));
    */

    if (Max_Strings > Hash_Size)
        usage ("Max_Strings (%d) must be <= Hash_Size (%d)",
               Max_Strings, Hash_Size);

    if (Max_Cites > Max_Strings)
        usage ("Max_Cites (%d) must be <= Max_Strings (%d)",
               Max_Cites, Max_Strings);

}                               /* set_array_sizes() */



/*-
**============================================================================
** usage()
**
**  Display a helpful message reporting the command line options
**  supported by this version of BibTeX.  If a message string is
**  provided, this is displayed first.
**============================================================================
*/
void usage (const char *printf_fmt, ...)
{
    va_list             printf_args;
    
    if (printf_fmt != NULL) {
#ifdef UTF_8
        fprintf (stderr, "BibTeXu: ");
#else
        fprintf (stderr, "BibTeX8: ");
#endif
        va_start (printf_args, printf_fmt);
        vfprintf (stderr, printf_fmt, printf_args);
        va_end (printf_args);
        fprintf (stderr, "\n");
    }

    FSO ("\nUsage: " PROGNAME " [options] aux-file\n\n");
    FSO ("  Valid options are:\n\n");
    FSO ("  -?  --help              display this help text\n");

#ifndef UTF_8
    FSO ("  -7  --traditional       operate in the original 7-bit mode\n");
    FSO ("  -8  --8bit              force 8-bit mode, no CS file used\n");
    FSO ("  -c  --csfile FILE       read FILE as the BibTeX character set\n");
    FSO ("                          and sort definition file\n");
#endif

#ifdef DEBUG
    FSO ("  -d  --debug TYPE        report debugging information.  TYPE is one\n");
    FSO ("                          or more of all, csf, io, mem, misc, search.\n");
#endif                          /* DEBUG */

#ifdef STAT
    FSO ("  -s  --statistics        report internal statistics\n");
#endif                          /* STAT */

#ifdef TRACE
    FSO ("  -t  --trace             report execution tracing\n");
#endif                          /* TRACE */

    FSO ("  -v  --version           report BibTeX version\n\n");

#ifdef UTF_8
    FSO ("  -l  --language LANG     use language LANG, ex: -l fr\n");
    FSO ("  -o  --location LOC      use location LOC, ex: -o fr\n\n");
#endif

    FSO ("  -B  --big               same as --mstrings 10000\n");
    FSO ("  -H  --huge              same as --mstrings 19000\n");
    FSO ("  -W  --wolfgang          same as --mstrings 30000\n");
    FSO ("  -M  --min_crossrefs ##  set min_crossrefs to ##\n");
    FSO ("      --mstrings ##       allow ## unique strings\n");

    debug_msg (DBG_MISC, "calling longjmp (Exit_Program_Flag) ... ");
    longjmp (Exit_Program_Flag, 1);
}                               /* usage() */



/*-
******************************************************************************
******************************************************************************
**
**  Functions specifically for 8-bit support.
**
**      c8read_csf
**      c8initialise
**      c8read_lowercase
**      c8read_lowupcase
**      c8read_uppercase
**      c8read_order
**      c8report_8bit_handling
**
******************************************************************************
******************************************************************************
*/
#ifdef SUPPORT_8BIT

/*
** Useful definitions.
*/
#define c8is_alpha(c)	        (isalpha(c) || c > 127)
#define c8is_alphanumeric(c)	(isalnum(c) || c > 127)
#define SORT_LAST               4095



/*-
**============================================================================
** c8initialise ()
**
**  Initialise BibTeX's handling of 8-bit character sets.  The steps are:
**
**      o   initialise default behaviour for sorting and upper/lowercase
**      o   if appropriate, read and parse the CS file
**      o   use CS data to set behaviour for sorting and upper/lowercase
**      o   update xchr/xord arrays for 8-bit characters.  These arrays
**          are initialised for ASCII character sets in the original
**          initialize() function.  Further initialisation for 8-bit
**          characters is performed here to minimise "contamination" of
**          the original BibTeX code.
**
**  We have to deal with three modes of operation:
**
**      --traditional - the original BibTeX behaviour
**      --8bit        - every character with a code > 127 is treated as 
**                      a letter
**      default       - a CS file is read and parsed to define sorting order
**                      and how 8-bit characters should be handled
**============================================================================
*/
void c8initialise (void)
{
    int                 ascii_case_diff = 'a' - 'A';
    int                 c;
    int                 status;

    /*
    ** Traditional mode (--traditional, -7) results in identical behaviour 
    ** to the original BibTeX.
    **
    **  o   sort order by character code value ('A'=65 ... 'z'=122)
    **  o   upper/lowercase relationships for A..Za..z established
    **  o   no CS file read
    **  o   only characters < 128 allowed
    **  o   no need to update xchr/xord arrays
    */
    if (Flag_7bit) {

        for (c = 0; c <= 255; c++) {
            c8order[c] = c;
            c8lowcase[c] = (UChar_T) c;
            c8upcase[c] = (UChar_T) c;
        }

        for (c = 'A'; c <= 'Z'; c++) {
            c8lowcase[c] = (UChar_T) (c + ascii_case_diff);
            c8upcase[c + ascii_case_diff] = (UChar_T) c;
        }

        for (c = 128; c <= 255; c++)
            lex_class[c] = ILLEGAL;
    } 

    /*
    ** Raw 8-bit (--8bit, -8).
    ** 
    **  o   sort order by character code value ('A'=65 ... 'z'=122)
    **  o   upper/lowercase relationships for A..Za..z established
    **  o   no CS file read
    **  o   need to update xchr/xord arrays
    **  o   characters > 127 allowed and all treated as type ALPHA
    */
    else if (Flag_8bit) {

        for (c = 0; c <= 255; c++) {
            c8order[c] = c;
            c8lowcase[c] = (UChar_T) c;
            c8upcase[c] = (UChar_T) c;
        }

        for (c = 'A'; c <= 'Z'; c++) {
            c8lowcase[c] = (UChar_T) (c + ascii_case_diff);
            c8upcase[c + ascii_case_diff] = (UChar_T) c;
        }

        for (c = 128; c <= 255; c++)
            xchr[c] = (char) c;

        for (c = 128; c <= 255; c++)
            xord[xchr[c]] = (ASCIICode_T) c;

        for (c = 128; c <= 255; c++)
            lex_class[c] = ALPHA;
    }

    /*
    ** Default initialisation using CS file.
    ** 
    **  o   no sort order defined - all set to weight SORT_LAST
    **  o   upper/lowercase relationships for A..Za..z established
    **  o   only characters < 128 allowed initially
    **  o   a CS file is read and parsed by BibTeX to modify these defaults
    **  o   need to update xchr/xord/lex_class arrays after reading CS file
    */
    else {

        for (c = 0; c <= 255; c++) {
            c8order[c] = SORT_LAST;
            c8lowcase[c] = (UChar_T) c;
            c8upcase[c] = (UChar_T) c;
        }

        c8order[32] = 0;

        for (c = 'A'; c <= 'Z'; c++) {
            c8lowcase[c] = (UChar_T) (c + ascii_case_diff);
            c8upcase[c + ascii_case_diff] = (UChar_T) c;
        }

        for (c = 128; c <= 255; c++)
            xchr[c] = (char) c;

        for (c = 128; c <= 255; c++)
            xord[xchr[c]] = (ASCIICode_T) c;

        for (c = 128; c <= 255; c++)
            lex_class[c] = ILLEGAL;

        /*
        ** Read the CS file.  If it was read and parsed OK, update the
        ** definition of 8-bit characters.  If an 8-bit character is defined 
        ** in any section of the CS file, it is treated as type ALPHA.
        */
        if ((status = c8read_csf ()) == TRUE) {

            for (c = 128; c <= 255; c++)
                if ((c8order[c] != SORT_LAST)
                        || (c8lowcase[c] != c) 
                        || (c8upcase[c] != c))
                    lex_class[c] = ALPHA;                    
        }
    }                           /* end if (Flag_7bit) */

    /*
    ** If the CSF debugging option has been selected, report results.
    */
    if ((Flag_debug & DBG_CSF))
        c8report_8bit_handling ();

}                               /* c8initialise () */



/*-
**============================================================================
** c8read_csf()
**
**  Read the Case and Sorting definitions (CS) File.  Steps taken are:
**
**      o   determine which CS file should be opened
**      o   open the CS file
**      o   look for valid CS file sections (\lowupcase, \lowercase, 
**          \uppercase, \order) and delegate parsing of each section to the
**          appropriate function
**      o   close the CS file
**
**  Returns TRUE if a CS file was successfully read and parsed, FALSE
**  otherwise.
**============================================================================
*/
int c8read_csf (void)
{
    int                 c, i;
    int                 in_comment;
#define SX_BUF_SIZE 15
    /* a section name: "order", "uppercase", "lowercase", or "lowupcase".  */
    char                sx[SX_BUF_SIZE + 1];
#ifndef KPATHSEA
    char               *value;
#endif
    
    /*
    ** Construct the name of the CS file to be used:
    **
    **  1.  If the --csfile option was specified, use its value.
    **  or
    **  2.  If CSF_FILE_ENVVAR is defined to a non-empty value, it
    **      contains the name of an environment variable.  The value of 
    **      this environment variable will be used as the default CS file.
    */
    if (Str_csfile == NULL) {
#ifdef KPATHSEA
#ifndef CSF_FILE_ENVVAR
#define CSF_FILE_ENVVAR "BIBTEX_CSFILE"
#endif
      /* Default value from environment or texmf.cnf */
#if defined(MIKTEX)
      Str_csfile = kpse_var_value ("BIBTEX_CSFILE");
      if (Str_csfile == NULL)  /* Use fallback value */
          Str_csfile = xstrdup (DEFAULT_BIBTEX_CSFILE);
#else
      Str_csfile = kpse_var_value (CSF_FILE_ENVVAR);
      if (Str_csfile == NULL)  /* Use fallback value */
          Str_csfile = xstrdup ("88591lat.csf");
#endif
#else
        if (strlen (CSF_FILE_ENVVAR) != 0) {
            debug_msg (DBG_CSF, "c8read_csf: --csfile not set, checking '%s'", 
                       CSF_FILE_ENVVAR);
            value = getenv (CSF_FILE_ENVVAR);            

            if (value == NULL)
                debug_msg (DBG_CSF, "c8read_csf: %s = `NULL'", CSF_FILE_ENVVAR);
            else
                debug_msg (DBG_CSF, "c8read_csf: %s = `%s'",
                           CSF_FILE_ENVVAR, value);
            /*
            ** If the environment variable has a non-empty value, use it
            ** as the file specifaction for the default CS file.
            */
            if ((value != NULL) && (*value != '\0'))
                Str_csfile = strdup (value);
            else {
                PRINT_LN ("Warning: cannot open CS file: --csfile not specfied");
                PRINT_LN2 ("and environment variable %s has no value.", 
                           CSF_FILE_ENVVAR);
                return FALSE;
            }
        } else {
            PRINT_LN ("Warning: cannot open CS file: --csfile not specified");
            PRINT_LN ("and default environment variable not specified.");
            return FALSE;
        }                       /* if (strlen (CSF_FILE_ENVVAR) != 0) */
#endif /* KPATHSEA */
    }                           /* if (Str_csfile == NULL) */
    /*
    ** Make sure that we have ended up with a CS file name.  If not,
    ** return FALSE;
    */
    if ((Str_csfile == NULL) || (*Str_csfile == '\0')) {
        PRINT_LN ("Warning: cannot open CS file: empty --csfile value specified.");
        return FALSE;
    }

    /*
    ** If the CS file name doesn't contain a '.', append ".csf".
    */
    free (name_of_file);
    name_of_file = (unsigned char *) mymalloc (strlen (Str_csfile) + 5, "name_of_file");
    strcpy ((char *)name_of_file, Str_csfile);

    if (strchr ((char *)name_of_file, '.') == NULL)
        strcat ((char *)name_of_file, ".csf");
    name_length = strlen ((char *)name_of_file);

    debug_msg (DBG_CSF, "c8read_csf: trying to open CS file `%s' ...",
               name_of_file);

    /*
    ** Attempt to open the CS file, using the CS file search path if
    ** necessary.  If the open fails, return FALSE.
    */
    if (! a_open_in (&c8_cs_file, CSF_FILE_SEARCH_PATH)) {
        PRINT_LN2 ("Error: cannot open CS file: %s", name_of_file);
        return FALSE;
    }
    PRINT_LN2 ("The 8-bit codepage and sorting file: %s", name_of_file);

    /*
    ** The main loop for reading the CS file.
    */
    c8_line = 1;
    while (! feof (c8_cs_file)) {

        /*
        ** Read the CS file until a '\\' character is found, denoting the
        ** start of a major section.  Comments begin with a '%' and
        ** anything else on the same line is ignored.
        */
        in_comment = FALSE;
        do {
            c = getc (c8_cs_file);

            if (c == '\n') {
                ++c8_line;
                in_comment = FALSE;
            }
            if (c == '%')
                in_comment = TRUE;
            if ((c == '\\') && (in_comment == FALSE))
                break;
        } while (!feof (c8_cs_file));

        /*
        ** If we break out of the do loop on EOF, there were no more 
        ** sections in the CS file.  Exit the main reading loop.
        */
        if (feof (c8_cs_file))
            break;

        /*
        ** After '\', a section name must be found.  This name is a word
        ** composed only of lower case ASCII letters.
        */
        i = 0;

        do {
            c = getc (c8_cs_file);
            if ((c >= 'a') && (c <= 'z'))
                sx[i++] = c;
            else
                break;
        } while (!feof (c8_cs_file) && (i < SX_BUF_SIZE));
        sx[i] = '\0';

        /*
        ** A '{' should follow the section name, but it could be preceeded 
        ** by whitespace, comments or newlines.
        */
	if (c != '{')
	    do {
		if (c == '\n') {
		    ++c8_line;
		    in_comment = FALSE;
		}
		if (c == '%')
		    in_comment = TRUE;
		if ((c == '{') && (in_comment == FALSE))
		    break;
		c = getc (c8_cs_file);
	    } while (!feof (c8_cs_file));


        /*
        ** The major sections supported in the CS file are: \order{},
        ** \uppercase{}, \lowercase{} and \lowupcase{}.  Anything else will
        ** be ignored.  Known sections are processed until they terminate
        ** and then the loop starts again.
        */
        if (strcmp (sx, "order") == 0)
            c8read_order ();
        else if (strcmp (sx, "uppercase") == 0)
            c8read_uppercase ();
        else if (strcmp (sx, "lowercase") == 0)
            c8read_lowercase ();
        else if (strcmp (sx, "lowupcase") == 0)
            c8read_lowupcase ();
        else {
            PRINT_LN2 ("Unknown section on line in CS file %s", Str_csfile);
            PRINT_LN3 ("%04d: \\%s", c8_line, buffer);
        }
    }				/* while (! feof (c8_cs_file)) */

    /*
    ** Finished reading, close the CS file then return TRUE.
    */
    close_file (c8_cs_file);
    free (Str_csfile);
    return TRUE;
}				/* c8read_csf () */



/*-
**============================================================================
** c8read_lowercase()
**
**  Read and parse the \lowercase section of the CS file.  This section is
**  used to define the upper -> lower case relationship of pairs of specified 
**  characters.  It should normally only be used if the relationship
**  isn't symmetrical - use \lowupcase if it is.
**
**  The syntax of the \lowercase section is:
**
**      \lowercase{
**          <UC-val-1> <LC-val-1>   % Comment begins with a percent sign
**          <UC-val-2> <LC-val-2>  <UC-val-3> <LC-val-3>
**          ...
**          <UC-val-N> <LC-val-N>
**      }
**
**  Notes:
**
**  o   characters can be entered normally, or in TeX notation for
**      character codes, i.e. ^^XX, where XX is the hexadecimal value.
**
**  o   reading of the \lowercase section ends when the first '}' character 
**      is reached, so '}' can't be included in a case relationship pair.
**      You can't use ^^7d either.
**
**  o   '%' is used to introduce a trailing comment, so '%' can't be included 
**      in the \lowercase section.  You can't use ^^25 either.
**
**  o   you cannot redefine the lower case equivalent of an ASCII character 
**      (code < 128).  In other words, all "left hand" characters in the
**      relationship pairs must have codes > 127.
**============================================================================
*/
static void c8read_lowercase (void)
{
    int                 c;
    int                 uppercase_val;

    debug_msg (DBG_CSF, "reading the \\lowercase section ... ");
    uppercase_val = -1;

    /*
    ** Keep reading characters until the end of the section is encountered 
    ** at a '}' or EOF is reached.
    */
    c = getc (c8_cs_file);

    while ((c != '}') && !feof (c8_cs_file)) {

        /*
        ** Characters may be specified using the TeX syntax for the
        ** 8-bit code, i.e. ^^XX, where XX is the hexadecimal value.
        ** We should really check the result of this fscanf().  Read the
        ** character code and then start the while loop again.
        */
        if (c == '^')
            fscanf (c8_cs_file, "^%x", &c);

        /*
        ** If we have reached the end of a line, simply increment the
        ** line count.
        */
        if (c == '\n') {
            c8_line++;
        }

        /*
        ** If we encounter a '%', it signifies the start of a comment.
        ** Ignore characters until the end of line is reached.
        */
        else if (c == '%') {
            do {
                c = getc(c8_cs_file);
            } while ((c != '\n') && (! feof( c8_cs_file)));
            continue;
        }

        /*
        ** The normal case is for alpha characters to be included in the
        ** case relationship.  Alpha characters are assumed to be ASCII
        ** alpha (A..Za..z) plus any character with a code > 127.
        */
        else if (c8is_alpha (c)) {

            /*
            ** The case relationship consists of a pair of alpha characters 
            ** separated by whitespace.  In the \lowercase section, the pair
            ** is in the order <uppercase-char> <lowercase-char>.  We
            ** must remember which character we are looking for - this is
            ** achieved by setting uppercase_val to -1 when we are
            ** looking for the first character of a pair.
            **
            ** If uppercase_val is < 0, we treat the current character as
            ** the upper case member of the pair and set uppercase_val
            ** accordingly.
            */
            if (uppercase_val < 0)
                uppercase_val = c;
            else {

                /*
                ** We are not allowed to redefine the lower case
                ** equivalent of an ASCII character. 
                */
                if (uppercase_val < 127) {
                    PRINT_LN2 ("Error: attempt to redefine lower case of "
                               "an ASCII character [0x%02x]", uppercase_val);
                    PRINT_LN3 ("Context: line %04d: of CS file %s", 
                               c8_line, Str_csfile);
                }

                /*
                ** All OK, so set the appropriate element of the c8lowcase 
                ** array and reset the value of uppercase_val to signify
                ** that we are looking for the next pair.
                */
                else {
                    c8lowcase[uppercase_val] = c;
                    uppercase_val = -1;
                }
            }
        }                       /* end if */

        /*
        ** Get the next character from the file and start the loop again.
        */
        c = getc (c8_cs_file);
    }				/* end while */

    /*
    ** The only normal reason for breaking out of the while loop is that
    ** we encountered '}' at the end of the \lowercase section.  Two
    ** conditions should be true:
    **
    **   (uppercase_val = -1)   - there is an unmatched character
    **   (c == '}')             - the terminating '}' has been read
    */
    if (uppercase_val != -1){
	PRINT_LN ("Error: unmatched character pair in \\lowercase section");
	PRINT_LN3 ("Context: line %04d: of CS file %s", c8_line, Str_csfile);
    }

    if (c != '}') {
        PRINT_LN ("Error: unexpected end of file in \\lowercase section");
        PRINT_LN3 ("Context: line %04d: of CS file %s", c8_line, Str_csfile);
    }

    debug_msg (DBG_CSF, "finished reading the \\lowercase section.");
}				/* c8read_lowercase () */



/*-
**============================================================================
** c8read_lowupcase()
**
**  Read and parse the \lowupcase section of the CS file.  This section is
**  used to define the lower -> upper and upper -> lower case relationship 
**  of pairs of specified characters.  It is only used if the relationship
**  is symmetrical - use \lowercase or \uppercase if it isn't.
**
**  The syntax of the \lowupcase section is:
**
**      \uppercase{
**          <LC-val-1> <UC-val-1>   % Comment begins with a percent sign
**          <LC-val-2> <UC-val-2>  <LC-val-3> <UC-val-3>
**          ...
**          <LC-val-N> <UC-val-N>
**      }
**
**  Notes:
**
**  o   characters can be entered normally, or in TeX notation for
**      character codes, i.e. ^^XX, where XX is the hexadecimal value.
**
**  o   reading of the \lowupcase section ends when the first '}' character 
**      is reached, so '}' can't be included in a case relationship pair.
**      You can't use ^^7d either.
**
**  o   '%' is used to introduce a trailing comment, so '%' can't be included 
**      in the \lowupcase section.  You can't use ^^25 either.
**
**  o   you cannot redefine the lower or upper case equivalent of an ASCII 
**      character (code < 128).  In other words, all characters in the
**      relationship pairs must have codes > 127.
**============================================================================
*/
static void c8read_lowupcase (void)
{
    int                 c;
    int                 lowercase_val;

    debug_msg (DBG_CSF, "reading the \\lowupcase section ... ");
    lowercase_val = -1;

    /*
    ** Keep reading characters until the end of the section is encountered 
    ** at a '}' or EOF is reached.
    */
    c = getc (c8_cs_file);

    while ((c != '}') && !feof (c8_cs_file)) {

        /*
        ** Characters may be specified using the TeX syntax for the
        ** 8-bit code, i.e. ^^XX, where XX is the hexadecimal value.
        ** We should really check the result of this fscanf().
        */
        if (c == '^')
            fscanf (c8_cs_file, "^%x", &c);

        /*
        ** If we have reached the end of a line, simply increment the
        ** line count.
        */
        if (c == '\n') {
            c8_line++;
        }

        /*
        ** If we encounter a '%', it signifies the start of a comment,
        ** unless it was translated from a ^^xx sequence.
        ** Ignore characters until the end of line is reached.
        */
        else if (c == '%') {
            do {
                c = getc(c8_cs_file);
            } while ((c != '\n') && (! feof( c8_cs_file)));
            continue;
        }

        /*
        ** The normal case is for alpha characters to be included in the
        ** case relationship.  Alpha characters are assumed to be ASCII
        ** alpha (A..Za..z) plus any character with a code > 127.
        */
        else if (c8is_alpha (c)) {

            /*
            ** The case relationship consists of a pair of alpha characters 
            ** separated by whitespace.  In the \lowupcase section, the pair
            ** is in the order <lowercase-char> <uppercase-char>.  We
            ** must remember which character we are looking for - this is
            ** achieved by setting lowercase_val to -1 when we are
            ** looking for the first character of a pair.
            **
            ** If lowercase_val is < 0, we treat the current character as
            ** the lower case member of the pair and set lowercase_val
            ** accordingly.
            */
            if (lowercase_val < 0)
                lowercase_val = c;
            else {

                /*
                ** We are not allowed to redefine the lower or upper case
                ** equivalent of an ASCII character. 
                */
                if (lowercase_val < 127) {
                    PRINT_LN2 ("Error: attempt to redefine upper case of "
                               "an ASCII character [0x%02x]", lowercase_val);
                    PRINT_LN3 ("Context: line %04d: of CS file %s", 
                               c8_line, Str_csfile);
                }
                else if (c < 127) {
                    PRINT_LN2 ("Error: attempt to redefine lower case of "
                               "an ASCII character [0x%02x]", c);
                    PRINT_LN3 ("Context: line %04d: of CS file %s", 
                               c8_line, Str_csfile);
                }

                /*
                ** All OK, so set the appropriate element of the c8lowcase 
                ** and c8upcase arrays, then reset the value of lowercase_val
                ** to signify that we are looking for the next pair.
                */
                else {
                    c8lowcase[c] = lowercase_val;
                    c8upcase[lowercase_val] = c;
                    lowercase_val = -1;
                }
            }
        }                       /* end if */

        /*
        ** Get the next character from the file and start the loop again.
        */
        c = getc (c8_cs_file);
    }				/* end while */

    /*
    ** The only normal reason for breaking out of the while loop is that
    ** we encountered '}' at the end of the \lowupcase section.  Two
    ** conditions should be true:
    **
    **   (lowercase_val = -1)   - there is an unmatched character
    **   (c == '}')             - the terminating '}' has been read
    */
    if (lowercase_val != -1){
	PRINT_LN ("Error: unmatched character pair in \\lowupcase section");
	PRINT_LN3 ("Context: line %04d: of CS file %s", c8_line, Str_csfile);
    }

    if (c != '}') {
        PRINT_LN ("Error: unexpected end of file in \\lowupcase section");
        PRINT_LN3 ("Context: line %04d: of CS file %s", c8_line, Str_csfile);
    }

    debug_msg (DBG_CSF, "finished reading the \\lowupcase section.");
}				/* c8read_lowupcase () */



/*-
**============================================================================
** c8read_order()
**
**  Read and parse the \order section of the CS file.  This section is
**  used to define the sorting order of characters in the 8-bit character
**  set.
**
**  The syntax of the \order section is:
**
**      \order{
**          <char-1>                % Comment begins with a percent sign
**          <char-2> <char-3> 
**          <char-4> - <char-5> 
**          <char-4> _ <char-5> 
**          ...
**          <char-n>
**      }
**
**  Notes:
**
**  o   characters can be entered normally, or in TeX notation for
**      character codes, i.e. ^^XX, where XX is the hexadecimal value.
**
**  o   reading of the \order section ends when the first '}' character 
**      is reached, so '}' can't be included in the \order section.  You
**      can't use ^^7d either.
**
**  o   '%' is used to introduce a trailing comment, so '%' can't be included
**      in the \order section.  You can't use ^^25 either.
**
**  o   All characters on the same line are given the same sorting weight.
**
**  o   The construct <char-1> <underscore> <char-2> is used to denote that 
**      all characters in the range <char-1> to <char-2> should be given the
**      same sorting weight.  For example, "A _ Z a _ z" would cause all
**      ASCII alphabetical characters to have the same sorting weight and
**      would be equivalent to placing all 26 characters on the same line.
**
**  o   The construct <char-1> <hyphen> <char-2> is used to denote that all
**      characters in the range <char-1> to <char-2> should be given an
**      ascending set of sorting weights, starting with <char-1> and
**      ending with <char-2>.  For example, "A - Z" would cause all
**      upper case ASCII alphabetical characters to be sorted in
**      ascending order and would be equivalent to placing 'A' on the
**      first line, 'B' on the second, through to 'Z' on the 26th line.
**
**  o   the characters at the beginning of the order section are given a
**      lower sorting weight than characters occuring later.  When
**      sorting alphabetically, characters with the lowest weight come
**      first.
**
**  o   all characters not in the \order section (including ASCII characters)
**      are given the same very high sorting weight to ensure that they
**      come last when sorting alphabetically.
**============================================================================
*/
static void c8read_order (void)
{
    int                 c, i;
    int                 c_previous = 0;
    int                 c_previous_previous = 0;
    int                 seen_chars;

    debug_msg (DBG_CSF, "reading the \\order section ... ");
    sort_weight = 1;
    seen_chars = FALSE;

    /*
    ** Keep reading characters until the end of the section is
    ** encountered at a '}' or EOF is reached.
    */
    c = getc (c8_cs_file);

    while ((c != '}') && (! feof (c8_cs_file))) {

        /*
        ** If we have reached the end of a line, the next line denotes
        ** characters with an increased sorting weight.  Any other
        ** characters with a value less than ' ' are ignored.
        */
        if (c == '\n') {
            if (seen_chars)
                sort_weight++;
            c8_line++;
            seen_chars = FALSE;
        }
        else if (c < ' ') {
            debug_msg (DBG_CSF, "ignoring char 0x%02X on line %d of CS file", 
                       c, c8_line);
        }        
        else if (c > ' ') {

            /*
            ** Characters may be specified using the TeX syntax for the
            ** 8-bit code, i.e. ^^XX, where XX is the hexadecimal value.
            ** We should really check the result of this fscanf().  Read the
            ** character code and then start the while loop again.
            */
            if (c == '^')
                fscanf (c8_cs_file, "^%x", &c);

            /*
            ** If we encounter a '%', it signifies the start of a comment.
            ** Ignore characters until the end of line is reached.
            */
            if (c == '%') {
                do {
                    c = getc(c8_cs_file);
                } while ((c != '\n') && (! feof( c8_cs_file)));
	        continue;
            }            

            /*
            ** The normal case is for alpha characters to be included
            ** in the sorting sequence.  Alpha characters are ASCII alpha
            ** plus any character with a code > 127.
            */
            else if (c8is_alphanumeric (c) || c == ' ') {

                /*
                ** If the previous char was '-', we expect this char to
                ** terminate a "X-Y" sequence.  The character before
                ** the '-' must also be an alpha type.  The characters in
                ** the sequence are assigned an increasing sort weight.
                */
	        if (c_previous == '-') {
	            if (c8is_alphanumeric (c_previous_previous)) {
	                for (i = c_previous_previous; i < c; i++)
	                    c8order[i] = sort_weight++;
                        seen_chars = TRUE;
	            } else {
                        PRINT_LN ("Error: non-alphanumeric character "
                                   "starts X-Y sequence");
                        PRINT_LN3 ("Context: line %04d: of CS file %s", 
                                   c8_line, Str_csfile);
                    }

                /*
                ** If the previous char was '_', we expect this char to
                ** terminate a "X_Y" sequence.  The character before
                ** the '_' must also be an alpha type.  The characters in
                ** the sequence are assigned the same sort weight.
                */
                } else if (c_previous == '_') {
	            if (c8is_alphanumeric (c_previous_previous)) {
	                for (i = c_previous_previous; i < c; i++)
	                    c8order[i] = sort_weight;
                        seen_chars = TRUE;
	            } else {
                        PRINT_LN ("Error: non-alphanumeric character "
                                   "starts X_Y sequence");
                        PRINT_LN3 ("Context: line %04d: of CS file %s", 
                                   c8_line, Str_csfile);
                    }
	        }
	        c8order[c] = sort_weight;
                seen_chars = TRUE;
            }
            
            /*
            ** The only other non-alpha characters allowed are '-' & '_'.
            */
            else if ((c != '-') && (c != '_')) {
                PRINT_LN ("Error: only '-' or '_' may follow an "
                          "alpha character");
                PRINT_LN3 ("Context: line %04d: of CS file %s", 
                           c8_line, Str_csfile);
            }

            /*
            ** Keep an up-to-date note of the two previous values of c.
            */
            c_previous_previous = c_previous;
            c_previous = c;
        }                       /* end if */

        /*
        ** Get the next character and start the loop again.
        */
        c = getc (c8_cs_file);
    }				/* end while */

    /*
    ** If c isn't '}', we ran into EOF.
    */
    if (c != '}') {
        PRINT_LN ("Error: unexpected end of file in \\order section");
        PRINT_LN3 ("Context: line %04d: of CS file %s", c8_line, Str_csfile);
    }
    debug_msg (DBG_CSF, "finished reading the \\order section.");
}				/* c8read_order () */



/*-
**============================================================================
** c8read_uppercase()
**
**  Read and parse the \uppercase section of the CS file.  This section is
**  used to define the lower -> upper case relationship of pairs of specified 
**  characters.  It should normally only be used if the relationship
**  isn't symmetrical - use \lowupcase if it is.
**
**  The syntax of the \uppercase section is:
**
**      \uppercase{
**          <LC-val-1> <UC-val-1>   % Comment begins with a percent sign
**          <LC-val-2> <UC-val-2>  <LC-val-3> <UC-val-3>
**          ...
**          <LC-val-N> <UC-val-N>
**      }
**
**  Notes:
**
**  o   characters can be entered normally, or in TeX notation for
**      character codes, i.e. ^^XX, where XX is the hexadecimal value.
**
**  o   reading of the \uppercase section ends when the first '}' character 
**      is reached, so '}' can't be included in a case relationship pair.
**      You can't use ^^7d either.
**
**  o   '%' is used to introduce a trailing comment, so '%' can't be included 
**      in the \uppercase section.  You can't use ^^25 either.
**
**  o   you cannot redefine the upper case equivalent of an ASCII character 
**      (code < 128).  In other words, all "left hand" characters in the
**      relationship pairs must have codes > 127.
**============================================================================
*/
static void c8read_uppercase (void)
{
    int                 c;
    int                 lowercase_val;

    debug_msg (DBG_CSF, "reading the \\uppercase section ... ");
    lowercase_val = -1;

    /*
    ** Keep reading characters until the end of the section is encountered 
    ** at a '}' or EOF is reached.
    */
    c = getc (c8_cs_file);

    while ((c != '}') && (! feof (c8_cs_file))) {

        /*
        ** Characters may be specified using the TeX syntax for the
        ** 8-bit code, i.e. ^^XX, where XX is the hexadecimal value.
        ** We should really check the result of this fscanf().
        */
        if (c == '^')
            fscanf (c8_cs_file, "^%x", &c);

        /*
        ** If we have reached the end of a line, simply increment the
        ** line count.
        */
        if (c == '\n') {
            c8_line++;
        }

        /*
        ** If we encounter a '%', it signifies the start of a comment.
        ** Ignore characters until the end of line is reached.
        */
        else if (c == '%') {
            do {
                c = getc(c8_cs_file);
            } while ((c != '\n') && (! feof( c8_cs_file)));
            continue;
        }

        /*
        ** The normal case is for alpha characters to be included in the
        ** case relationship.  Alpha characters are assumed to be ASCII
        ** alpha (A..Za..z) plus any character with a code > 127.
        */
        else if (c8is_alpha (c)) {

            /*
            ** The case relationship consists of a pair of alpha characters 
            ** separated by whitespace.  In the \uppercase section, the pair
            ** is in the order <lowercase-char> <uppercase-char>.  We
            ** must remember which character we are looking for - this is
            ** achieved by setting lowercase_val to -1 when we are
            ** looking for the first character of a pair.
            **
            ** If lowercase_val is < 0, we treat the current character as
            ** the lower case member of the pair and set lowercase_val
            ** accordingly.
            */
            if (lowercase_val < 0)
                lowercase_val = c;
            else {

                /*
                ** We are not allowed to redefine the upper case
                ** equivalent of an ASCII character. 
                */
                if (lowercase_val < 127) {
                    PRINT_LN2 ("Error: attempt to redefine upper case of "
                               "an ASCII character [0x%02x]", lowercase_val);
                    PRINT_LN3 ("Context: line %04d: of CS file %s", 
                               c8_line, Str_csfile);
                }

                /*
                ** All OK, so set the appropriate element of the c8lowcase 
                ** array and reset the value of lowercase_val to signify
                ** that we are looking for the next pair.
                */
                else {
                    c8upcase[lowercase_val] = c;
                    lowercase_val = -1;
                }
            }
        }                       /* end if */

        /*
        ** Get the next character from the file and start the loop again.
        */
        c = getc (c8_cs_file);
    }				/* end while */

    /*
    ** The only normal reason for breaking out of the while loop is that
    ** we encountered '}' at the end of the \uppercase section.  Two
    ** conditions should be true:
    **
    **   (lowercase_val = -1)   - there is an unmatched character
    **   (c == '}')             - the terminating '}' has been read
    */
    if (lowercase_val != -1){
	PRINT_LN ("Error: unmatched character pair in \\uppercase section");
	PRINT_LN3 ("Context: line %04d: of CS file %s", c8_line, Str_csfile);
    }

    if (c != '}') {
        PRINT_LN ("Error: unexpected end of file in \\uppercase section");
        PRINT_LN3 ("Context: line %04d: of CS file %s", c8_line, Str_csfile);
    }

    debug_msg (DBG_CSF, "finished reading the \\uppercase section.");
}				/* c8read_uppercase () */



/*
**============================================================================
** c8report_8bit_handling()
**
** Report the internal status of BibTeX's 8-bit character set handling.  The
** information reported is:
**
**  o   8-bit characters defined as type ALPHA
**  o   character codes with upper case equivalents
**  o   character codes with lower case equivalents
**  o   the character codes in sorting weight order
**============================================================================
*/
void c8report_8bit_handling (void)
{
    int                 c;
    int                 column;
    int                 weight;

    /*
    ** Exit immediately if the CSF debugging option hasn't been selected.
    */
    if ((Flag_debug & DBG_CSF) == 0)
        return;
    
    /*
    ** Report the 8-bit characters defined as type ALPHA, 16 to a row.
    */
    fprintf (stderr, "\nD-CSF: 8-bit ALPHA characters\n");
    fprintf (stderr, "D-CSF: ----------------------\n");

    for (c = 128; c <= 255; c += 16) {
        fprintf (stderr, "D-CSF: %02x:  ", c);
        
        for (column = 0; column < 16; column++) {
            if (lex_class[c + column] == ALPHA)
                fprintf (stderr, "%c  ", c + column);
            else
                fputs (".  ", stderr);
        }
        fputc ('\n', stderr);
    }

    /*
    ** Report the lower / upper case relationships, three pairs to a row.
    */
    fprintf (stderr, "\nD-CSF: Characters with upper case equivalents\n");
    fprintf (stderr, "D-CSF: --------------------------------------");
    column = 0;

    for (c = 1; c <= 255; c++)
        if (c8upcase[c] != c) {
            if ((column % 3) == 0)
                fprintf (stderr, "\nD-CSF: %c [%02x] <<< %c [%02x]",
                         c, (int) c, c8upcase[c], (int) c8upcase[c]);
            else
                fprintf (stderr, "      %c [%02x] <<< %c [%02x]",
                         c, (int) c, c8upcase[c], (int) c8upcase[c]);
            ++column;
        }

    /*
    ** Report the upper / lower case relationships, three pairs to a row.
    */
    fprintf (stderr, "\n\nD-CSF: Characters with lower case equivalents\n");
    fprintf (stderr, "D-CSF: --------------------------------------");
    column = 0;

    for (c = 1; c <= 255; c++)
        if (c8lowcase[c] != c) {
            if ((column % 3) == 0)
                fprintf (stderr, "\nD-CSF: %c [%02x] >>> %c [%02x]",
                         c, (int) c, c8lowcase[c], (int) c8lowcase[c]);
            else
                fprintf (stderr, "      %c [%02x] >>> %c [%02x]",
                         c, (int) c, c8lowcase[c], (int) c8lowcase[c]);
            ++column;
        }

    /*
    ** Report character codes in sorting weight order.  If BibTeX is
    ** operating in --traditional or --8bit mode, sorting order is simply
    ** character code order.  In that case, just give an abbreviated
    ** summary.
    */
    fprintf (stderr, "\n\nD-CSF: Characters in sorting order\n");
    fprintf (stderr, "D-CSF: ---------------------------\n");

    if (Flag_7bit || Flag_8bit) {
        fprintf (stderr, "D-CSF: 00:    [00]\n");
        fprintf (stderr, "D-CSF: 01:    [01]\n");
        fprintf (stderr, "D-CSF: ..     ....\n");
        fprintf (stderr, "D-CSF: 41:  A [41]\n");
        fprintf (stderr, "D-CSF: ..     ....\n");
        fprintf (stderr, "D-CSF: 5a:  Z [5a]\n");
        fprintf (stderr, "D-CSF: ..     ....\n");
        fprintf (stderr, "D-CSF: 61:  a [61]\n");
        fprintf (stderr, "D-CSF: ..     ....\n");
        fprintf (stderr, "D-CSF: 7a:  z [7a]\n");
        fprintf (stderr, "D-CSF: ..     ....\n");
        fprintf (stderr, "D-CSF: fe:    [fe]\n");
        fprintf (stderr, "D-CSF: ff:    [ff]\n\n");
    }

    /*
    ** When a CS file has been used, sort_weight will be set to the value of
    ** the highest sorting weight.  Report every character with a given
    ** sorting weight, for 0 <= weight < sort_weight.
    */
    else {
        for (weight = 0; weight < sort_weight; weight++) {
            fprintf (stderr, "D-CSF: %02x:  ", weight);

            for (c = 0; c <= 255; c++) {
                if (c8order[c] == weight)
                    fprintf (stderr, "%c [%02x] ", c, c);
            }
            fputc ('\n', stderr);
        }    
        fprintf (stderr, "D-CSF: (All other characters are sorted "
                         "equally after any of the above.)\n\n");
    }
}                       /* c8report_8bit_handling() */    


#endif                          /* SUPPORT_8BIT */
