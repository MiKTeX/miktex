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


/*
 * Some functions which have to be made different from OS to OS,
 * unfortunately...:\
 *
 */

#include "ChkTeX.h"
#include "OpSys.h"
#include "Utility.h"

#ifdef KPATHSEA
#include <kpathsea/variable.h>
#endif

#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif
#ifdef HAVE_STAT_H
#  include <stat.h>
#endif

#if HAVE_DIRENT_H
#  include <dirent.h>
#  define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#  define dirent direct
#  define NAMLEN(dirent) (dirent)->d_namlen
#  if HAVE_SYS_NDIR_H
#    include <sys/ndir.h>
#  endif
#  if HAVE_SYS_DIR_H
#    include <sys/dir.h>
#  endif
#  if HAVE_NDIR_H
#    include <ndir.h>
#  endif
#endif

#if defined(HAVE_OPENDIR) && defined(HAVE_CLOSEDIR) && \
    defined(HAVE_READDIR) && defined(HAVE_STAT) && \
    defined(S_IFDIR) && defined(SLASH)
#  define USE_RECURSE 1
#else
#  define USE_RECURSE 0
#endif

#if defined(HAVE_LIBTERMCAP) || defined(HAVE_LIBTERMLIB)
#  define USE_TERMCAP 1
#endif


#ifdef USE_TERMCAP
#  ifdef HAVE_TERMCAP_H
#    include <termcap.h>
#  elif HAVE_TERMLIB_H
#    include <termlib.h>
#  else
int tgetent(char *BUFFER, char *TERMTYPE);
char *tgetstr(char *NAME, char **AREA);
#  endif
static char term_buffer[2048];
#endif

/*  -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=-  */

/*
 * This is the name of the global resource file.
 */

#ifndef SYSCONFDIR
#  if defined(__unix__)
#    define SYSCONFDIR "/usr/local/lib/"
#  elif defined(__MSDOS__)
#    define SYSCONFDIR "\\emtex\\data\\"
#  else
#    define SYSCONFDIR
#  endif
#endif
#define RCBASENAME              "chktexrc"

#ifdef __MSDOS__
#  define LOCALRCFILE             RCBASENAME
#elif defined(WIN32)
#  define LOCALRCFILE             RCBASENAME
#else
#  define LOCALRCFILE             "." RCBASENAME
#endif

char ConfigFile[BUFSIZ] = LOCALRCFILE;
const char *ReverseOn;
const char *ReverseOff;


static int HasFile(char *Dir, const char *Filename, const char *App);

#if USE_RECURSE
static int SearchFile(char *Dir, const char *Filename, const char *App);
#endif /* USE_RECURSE */

/*  -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=-  */


/*
 * Modify this one to suit your needs. In any case, it should fill
 * the ConfigFile (sized BUFLEN) buffer above with full name & path
 * for the configuration file. The macro RCFILE will give you the
 * filename part of the file, if you need that.
 *
 * Note: This routine will be called several times. Your mission will
 * be to look in each location, and see whether a .chktexrc file exist
 * there.
 *
 * If you choose to do nothing, only the current directory will be
 * searched.
 *
 */


enum LookIn
{
    liMin,
    liSysDir,
    liUsrDir,
    liEnvir,
    liCurDir,
    liNFound,
    liMax
};


int SetupVars(void)
{
    char *Env;
#ifdef __MSDOS__

    char *Ptr;
#endif

    static enum LookIn i = liMin;
    static int FoundFile;

    while (++i < liMax)
    {
        switch (i)
        {
        case liCurDir:         /* Current directory */
            strcpy(ConfigFile, LOCALRCFILE);
            break;
        case liEnvir:          /* Environment defined */
#ifdef __MSDOS__

            if ((Env = getenv("CHKTEXRC")) || (Env = getenv("CHKTEX_HOME")))
#elif defined(TEX_LIVE)
            if ((Env = kpse_var_value("CHKTEXRC")))
#else

            if ((Env = getenv("CHKTEXRC")))
#endif

            {
                strcpy(ConfigFile, Env);
                tackon(ConfigFile, LOCALRCFILE);
#ifdef TEX_LIVE
                free(Env);
#endif
            }
            else
#ifdef __MSDOS__
            if ((Env = getenv("EMTEXDIR")))
            {
                strcpy(ConfigFile, Env);
                tackon(ConfigFile, "data");
                tackon(ConfigFile, LOCALRCFILE);
            }
            else
#endif

                *ConfigFile = 0;
            break;
        case liUsrDir:         /* User dir for resource files */
#if defined(__unix__)

            if ((Env = getenv("HOME")) || (Env = getenv("LOGDIR")))
            {
                strcpy(ConfigFile, Env);
                tackon(ConfigFile, LOCALRCFILE);
            }
            else
#elif defined(__MSDOS__)

            strcpy(ConfigFile, PrgName);
            if ((Ptr = strrchr(ConfigFile, '\\')) ||
                (Ptr = strchr(ConfigFile, ':')))
                strcpy(++Ptr, RCBASENAME);
            else
#endif
                *ConfigFile = 0;

            break;
        case liSysDir:         /* System dir for resource files */
#ifdef TEX_LIVE
            if ((Env = kpse_var_value("CHKTEX_CONFIG")))
            {
                strcpy(ConfigFile, Env);
                free(Env);
            }
#if defined(MIKTEX)
            if (!miktex_find_file(RCBASENAME, "%R/chktex", ConfigFile))
            {
              *ConfigFile = 0;
            }
#else
            else if ((Env = kpse_var_value("TEXMFMAIN")))
            {
                strcpy(ConfigFile, Env);
                tackon(ConfigFile, "chktex");
                tackon(ConfigFile, RCBASENAME);
                free(Env);
            }
            else
                *ConfigFile = 0;
#endif
#else /* TEX_LIVE */
#if defined(__unix__) || defined(__MSDOS__)
            strcpy(ConfigFile, SYSCONFDIR);
            tackon(ConfigFile, RCBASENAME);
#else
            *ConfigFile = 0;
#endif
#endif /* TEX_LIVE */

            break;
        case liNFound:
        case liMin:
        case liMax:
            *ConfigFile = 0;
            if (!FoundFile)
                PrintPrgErr(pmNoRsrc);
        }

        if (*ConfigFile && fexists(ConfigFile))
            break;
    }
    FoundFile |= *ConfigFile;

    return (*ConfigFile);
}


/*  -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=-  */

/*
 * This function should initialize the global variables ReverseOn
 * and ReverseOff to magic cookies, which when printed, makes the
 * text in between stand out.
 */

void SetupTerm(void)
{
#ifdef USE_TERMCAP
    char *termtype = getenv("TERM");
    int success;
    char *buffer;
    static char str_so[3] = "so", str_se[3] = "se";

    if (termtype)
    {

        success = tgetent(term_buffer, termtype);
        if (success < 0)
            PrintPrgErr(pmNoTermData);
        if (success == 0)
            PrintPrgErr(pmNoTermDefd);

        buffer = (char *) malloc(sizeof(term_buffer));
        ReverseOn = tgetstr(str_so, &buffer);
        ReverseOff = tgetstr(str_se, &buffer);

        if (ReverseOn && ReverseOff)
            return;
    }
#endif

    ReverseOn = PRE_ERROR_STR;
    ReverseOff = POST_ERROR_STR;
}

/*  -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=-  */

/*
 * Concatenates the `File' string to the `Dir' string, leaving the result
 * in the `Dir' buffer. Takes care of inserting `directory' characters;
 * if we've got the strings "/usr/foo" and "bar", we'll get
 * "/usr/foo/bar".
 *
 * Behaviour somewhat controlled by the macros SLASH and DIRCHARS in the
 * OpSys.h file.
 *
 */

void tackon(char *Dir, const char *File)
{
    int EndC;
    unsigned long SLen;

    if (Dir && (SLen = strlen(Dir)))
    {
        EndC = Dir[SLen - 1];
        if (!(strchr(DIRCHARS, EndC)))
        {
            Dir[SLen++] = SLASH;
            Dir[SLen] = 0L;
        }
    }

    strcat(Dir, File);
}

/*
 * This function should add the appendix App to the filename Name.
 * If the resulting filename gets too long due to this, it may
 * overwrite the old appendix.
 *
 * Name may be assumed to be a legal filename under your OS.
 *
 * The appendix should contain a leading dot.
 */

void AddAppendix(char *Name, const char *App)
{
#ifdef __MSDOS__
    char *p;

    if ((p = strrchr(Name, '.')))
        strcpy(p, App);
    else
        strcat(Name, App);
#else
    /*
     * NOTE! This may fail if your system has a claustrophobic file
     * name length limit.
     */
    strcat(Name, App);
#endif

}

/*  -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=- -=><=-  */


/*
 * Locates a file, given a wordlist containing paths. If a
 * dir ends in a double SLASH, we'll search it recursively.
 *
 * We assume that
 *   a) a deeper level in the dir. tree. has a longer path than
 *      one above.
 *   b) adding a level doesn't change any of the previous levels.
 *
 * If this function returns TRUE, Dest is guaranteed to contain
 * path & name of the found file.
 *
 * FALSE indicates that the file was not found; Dest is then
 * unspecified.
 */


int LocateFile(const char *Filename,    /* File to search for */
               char *Dest,      /* Where to put final file */
               const char *App, /* Extra optional appendix */
               struct WordList *wl)     /* List of paths, entries
                                         * ending in // will be recursed
                                         */
{
    unsigned long i;
#if USE_RECURSE

    unsigned long Len;
#endif

    FORWL(i, *wl)
    {
        strcpy(Dest, wl->Stack.Data[i]);

#if USE_RECURSE

        Len = strlen(Dest);

        if (Len && (Dest[Len - 1] == SLASH) && (Dest[Len - 2] == SLASH))
        {
            Dest[Len - 1] = Dest[Len - 2] = 0;
            if (SearchFile(Dest, Filename, App))
                return (TRUE);
        }
        else
#endif /* USE_RECURSE */

        {
            if (HasFile(Dest, Filename, App))
                return (TRUE);
        }
    }
    return (FALSE);
}

static int HasFile(char *Dir, const char *Filename, const char *App)
{
    int DirLen = strlen(Dir);

    tackon(Dir, Filename);
    if (fexists(Dir))
        return (TRUE);

    if (App)
    {
        AddAppendix(Dir, App);
        if (fexists(Dir))
            return (TRUE);
    }

    Dir[DirLen] = 0;
    return (FALSE);

}


#if USE_RECURSE
static int SearchFile(char *Dir, const char *Filename, const char *App)
{
    struct stat *statbuf;
    struct dirent *de;
    DIR *dh;

    int DirLen = strlen(Dir);
    int Found = FALSE;

    DEBUG(("Searching %s for %s\n", Dir, Filename));

    if (HasFile(Dir, Filename, App))
        return (TRUE);
    else
    {
        if ((statbuf = malloc(sizeof(struct stat))))
        {
            if ((dh = opendir(Dir)))
            {
                while (!Found && (de = readdir(dh)))
                {
                    Dir[DirLen] = 0;
                    if (strcmp(de->d_name, ".") && strcmp(de->d_name, ".."))
                    {
                        tackon(Dir, de->d_name);

                        if (!stat(Dir, statbuf))
                        {
                            if ((statbuf->st_mode & S_IFMT) == S_IFDIR)
                                Found = SearchFile(Dir, Filename, App);
                        }
                    }
                }
                closedir(dh);
            }
            else
                PrintPrgErr(pmNoOpenDir, Dir);
            free(statbuf);
        }
    }
    return (Found);
}
#endif /* USE_RECURSE */
