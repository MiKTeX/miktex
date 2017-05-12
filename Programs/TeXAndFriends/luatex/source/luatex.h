/* luatex.h

   Copyright 1996-2006 Han The Thanh <thanh@pdftex.org>
   Copyright 2006-2013 Taco Hoekwater <taco@luatex.org>

   This file is part of LuaTeX.

   LuaTeX is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   LuaTeX is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
   License for more details.

   You should have received a copy of the GNU General Public License along
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>. */


#ifndef LUATEX_H
#  define LUATEX_H

/* texmf.h: Main include file for TeX and Metafont in C. This file is
   included by {tex,mf}d.h, which is the first include in the C files
   output by web2c.  */

#  include "cpascal.h"
#  if defined(MIKTEX) && defined(round)
#    undef round
#  endif

#  include <kpathsea/c-pathch.h>        /* for IS_DIR_SEP, used in the change files */
#  include <kpathsea/tex-make.h>        /* for kpse_make_tex_discard_errors */

/* If we have these macros, use them, as they provide a better guide to
   the endianess when cross-compiling. */
#  if defined (BYTE_ORDER) && defined (BIG_ENDIAN) && defined (LITTLE_ENDIAN)
#    ifdef WORDS_BIGENDIAN
#      undef WORDS_BIGENDIAN
#    endif
#    if BYTE_ORDER == BIG_ENDIAN
#      define WORDS_BIGENDIAN
#    endif
#  endif
/* More of the same, but now NeXT-specific. */
#  ifdef NeXT
#    ifdef WORDS_BIGENDIAN
#      undef WORDS_BIGENDIAN
#    endif
#    ifdef __BIG_ENDIAN__
#      define WORDS_BIGENDIAN
#    endif
#  endif


/* Some things are the same except for the name.  */

#  define TEXMFPOOLNAME "luatex.pool"
#  define TEXMFENGINENAME "luatex"

#  define DUMP_FILE fmt_file
#  define DUMP_FORMAT kpse_fmt_format
#  define write_dvi WRITE_OUT
#  define flush_dvi flush_out
#  define OUT_FILE dvi_file
#  define OUT_BUF dvi_buf

/* Restore underscores.  */
#  define kpsetexformat kpse_tex_format
#  define t_open_in topenin

/* Executing shell commands.  */
extern void mk_shellcmdlist(char *);
extern void init_shell_escape(void);
extern void init_start_time(void);
extern int get_start_time(void);
extern void set_start_time(int);
extern boolean get_start_time(void);

extern int shell_cmd_is_allowed(const char *cmd, char **safecmd,
                                char **cmdname);


#if defined(WIN32) && !defined(__MINGW32__) && defined(DLLPROC)
extern __declspec(dllexport) int DLLPROC (int ac, string *av);
#else
#undef DLLPROC
#endif

#  ifndef GLUERATIO_TYPE
#    define GLUERATIO_TYPE double
#  endif
typedef GLUERATIO_TYPE glueratio;

#  if defined(__DJGPP__) && defined (IPC)
#    undef IPC
#  endif

#  ifdef IPC
extern void ipcpage(int);
#  endif                        /* IPC */


#  define flush_out() fflush (OUT_FILE)

/* Read a line of input as quickly as possible.  */
#  define	input_ln(stream, flag) input_line (stream)

extern boolean input_line(FILE *);

#  define COPYRIGHT_HOLDER "Taco Hoekwater"
#  define AUTHOR NULL
#  define PROGRAM_HELP LUATEXHELP
#  define BUG_ADDRESS "dev-luatex@ntg.nl"
#  define DUMP_OPTION "fmt"
#  define DUMP_EXT ".fmt"
#  define INPUT_FORMAT kpse_tex_format
#  define INI_PROGRAM "luainitex"
#  define VIR_PROGRAM "luavirtex"
#  define TEXMFENGINENAME "luatex"

/* this counteracts the macro definition in cpascal.h */
#  undef Xchr
#  define Xchr(a) a

#endif
