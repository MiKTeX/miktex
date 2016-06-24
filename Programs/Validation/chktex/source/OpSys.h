/*
 *  ChkTeX, operating system specific code for ChkTeX.
 *  Copyright (C) 1995-96 Jens T. Berger Thielemann
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  Contact the author at:
 *		Jens Berger
 *		Spektrumvn. 4
 *		N-0666 Oslo
 *		Norway
 *		E-mail: <jensthi@ifi.uio.no>
 *
 *
 */

#ifndef OPSYS_H
#define OPSYS_H

#include "ChkTeX.h"
#include "Utility.h"

/********************************************************************/
/**************** START OF USER SETTABLE PREFERENCES ****************/

/*
 * Note: This file contains most defines you'll wish to change if you
 * wish to adopt ChkTeX to a new system. It is, as you might notice,
 * heavily documented. If you wish to get into the internals of ChkTeX,
 * the interesting stuff is at the bottom of this file, and in the .c
 * files. However, you should also take a look at the "config.h.in" file
 * in this directory if you haven't got a shell able to run the "configure"
 * script.
 *
 * This program relies heavily on that the system which
 * automagically free()'s all malloc()'ed memory, works. The program
 * itself does not call free() very much. This is because we're doing
 * lots of tiny allocations, and a properly designed pooling system will
 * hopefully do a quicker job than we'll be able to do. So there.
 *
 * To keep things simple, we trust that the fclose()'ing of fopen()'ed
 * also happens automagically.
 *
 * Please use the getopt included, as we will modify optarg during
 * command processing.
 *
 * You may wish to modify the SetupVars() (OpSys.c) to better suit your
 * preferences. In any case, it should put the filename (and full path)
 * of the `.chktexrc' file into the ConfigFile array. The array is sized
 * BUFSIZ bytes.
 *
 * The program does also assume that __unix__ is defined if the source is
 * compiled on a UNIX machine.
 *
 */


/*  -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=-  */

/*
 * Here you should define what codes which should be returned to the
 * shell upon success/failure.
 *
 */

#ifndef EXIT_FAILURE
#    define  EXIT_FAILURE    1
#endif

#ifndef EXIT_SUCCESS
#  define  EXIT_SUCCESS    0
#endif

/*  -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=-  */

/*
 * SLASH should be defined to the character your computer uses to
 * separate files/directories. Most systems use '/', messydos uses
 * '\'.
 *
 * DIRCHARS should be defined to the characters a directory entry
 * may end on. On Amigas, this is ":/" (either "FOO:BAR/" or "FOO:"),
 * Unix uses only "/", while messydos uses ":\\".
 *
 * This data will be used to automatically concatenate a directory
 * path and a filename.
 *
 * Adjust both to suit your needs.
 */


#ifdef TEX_LIVE
#if defined(__MSDOS__) || defined(WIN32)
#  define SLASH   '\\'
#  define DIRCHARS ":\\"
#else
#  define  SLASH  '/'
#  define DIRCHARS "/"
#endif
#else /* TEX_LIVE */
#if defined(__unix__)
#  define  SLASH  '/'
#elif defined(__MSDOS__)
#  define SLASH   '\\'
#endif

#if defined(__unix__)
#  define DIRCHARS "/"
#elif defined(__MSDOS__)
#  define DIRCHARS ":\\"
#endif
#endif /* TEX_LIVE */

/*  -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=-  */

/*
 * Here, define what key-combination which is used to abort stdin
 * keyboard input. It should be a string, as we we'll type it out as
 * information to the user.
 */

#if defined(__unix__)
#  define STDIN_BREAK "Ctrl-D"
#elif defined(__MSDOS__)
#  define STDIN_BREAK "Ctrl-Z + Enter"
#else
#  define STDIN_BREAK "stdin break combination"
#endif

/*  -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=-  */

/*
 * For fancy printing of commands, we'll use these strings to turn
 * on/off the error indication. The codes listed here are ANSI
 * compatible; if you don't have that type of terminal, you may wish
 * to adjust this. Use "chktex -v2 Test.tex" to check the effects of
 * these macros. Note: These strings will be printf()'ed, so watch your
 * %'s.
 *
 * Under UNIX, we'll ignore these values and use termcap instead, where
 * that is installed.
 *
 * If these strings can't be specified statically, you'll have to add
 * code in the SetupTerm() function.
 *
 * PRE_ERROR_STR is of course printed in front of each location we
 * wish to show as an error, and POST_ERROR_STR after each location.
 *
 * The codes #defined here, will switch back- and foreground colours.
 * We're using '\033[' as escape character, some terminals may like
 * '\233' better.
 *
 */

#  define PRE_ERROR_STR   "\033[7m"
#  define POST_ERROR_STR  "\033[0m"


/*  -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=-  */

/*
 * This macro should contain the appendix for backup files, which
 * will be appended onto the original filename. It should contain
 * a leading dot.
 */

#ifdef __MSDOS__
#  define BAKAPPENDIX ".$cl"
#else
#  define BAKAPPENDIX ".bak"
#endif

/***************** END OF USER SETTABLE PREFERENCES *****************/
/********************************************************************/

#ifndef WORDLIST_DEFINED
struct WordList;
#endif
/* Sorry; there are now cyclic dependencies in the
* source tree. :-/ 
*/

extern const char *ReverseOn;
extern const char *ReverseOff;
extern char ConfigFile[BUFSIZ];

char *MatchFileName(char *String);
int SetupVars(void);
void SetupTerm(void);
void AddAppendix(char *Name, const char *App);
void tackon(char *, const char *);
int LocateFile(const char *Filename, char *Dest, const char *App,
               struct WordList *wl);

#endif /* OPSYS_H */
