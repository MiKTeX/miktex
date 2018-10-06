/* loslibext.c

   Copyright 2006-2012 Taco Hoekwater <taco@luatex.org>

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

#include "ptexlib.h"
#include "lua/luatex-api.h"

#include <kpathsea/c-stat.h>
#include <kpathsea/c-dir.h>
#include <time.h>


#if defined(_WIN32) || defined(__NT__)
#  define MKDIR(a,b) mkdir(a)
#else
#  define MKDIR(a,b) mkdir(a,b)
#endif

/* An attempt to figure out the basic platform, does not
  care about niceties like version numbers yet,
  and ignores platforms where luatex is unlikely to
  successfully compile without major prorting effort
  (amiga|mac|os2|vms) */

#if defined(_WIN32) || defined(__NT__)
#  define OS_PLATTYPE "windows"
#  define OS_PLATNAME "windows"
#elif defined(__GO32__) || defined(__DJGPP__) || defined(__DOS__)
#  define OS_PLATTYPE "msdos"
#  define OS_PLATNAME "msdos"
#else                           /* assume everything else is unix-y */
#  include <sys/utsname.h>
#  define OS_PLATTYPE "unix"
/* this is just a first guess */
#  if defined(__BSD__)
#    define OS_PLATNAME "bsd"
#  elif defined(__CYGWIN__)
#    define OS_PLATNAME "cygwin"
#  elif defined(__SYSV__)
#    define OS_PLATNAME "sysv"
#  else
#    define OS_PLATNAME "generic"
#  endif
/* attempt to be more precise */
#  if defined(__LINUX__) || defined (__linux)
#    undef OS_PLATNAME
#    define OS_PLATNAME "linux"
#  elif defined(__FREEBSD__) || defined(__FreeBSD__) || defined(__FreeBSD)
#    undef OS_PLATNAME
#    define OS_PLATNAME "freebsd"
#  elif defined(__FreeBSD_kernel__)
#    undef OS_PLATNAME
#    define OS_PLATNAME "kfreebsd"
#  elif defined(__OPENBSD__)  || defined(__OpenBSD)
#    undef OS_PLATNAME
#    define OS_PLATNAME "openbsd"
#  elif defined(__SOLARIS__)
#    undef OS_PLATNAME
#    define OS_PLATNAME "solaris"
#  elif defined(__SUNOS__) ||  defined(__SUN__) ||  defined(sun)
#    undef OS_PLATNAME
#    define OS_PLATNAME "sunos"
#  elif defined(HPUX) || defined(__hpux)
#    undef OS_PLATNAME
#    define OS_PLATNAME "hpux"
#  elif defined(__sgi)
#    undef OS_PLATNAME
#    define OS_PLATNAME "irix"
#  elif defined(__MACH__) && defined(__APPLE__)
#    undef OS_PLATNAME
#    define OS_PLATNAME "macosx"
#  elif defined(__GNU__)
#    undef OS_PLATNAME
#    define OS_PLATNAME "gnu"
#  endif
#endif





/* there could be more platforms that don't have these two,
 *  but win32 and sunos are for sure.
 * gettimeofday() for win32 is using an alternative definition
 */

#if (! defined(_WIN32)) && (! defined(__SUNOS__))
#  include <sys/time.h>         /* gettimeofday() */
#  include <sys/times.h>        /* times() */
#  include <sys/wait.h>
#endif

/* set this to one for spawn instead of exec on windows */

#define DONT_REALLY_EXIT 1

/* Note: under WIN32, |environ| is nothing but a copy of the actual
   environment as it was during program startup. That variable
   can then be changed (which is a little odd), but such changes
   do not survive in the actual environment *unless* they are
   passed on down in e.g. |_execvpe|, when they would be inherited
   by the child process.

   This gives trouble because some parts of the texk code actually
   change the contents of the |environ| variable as a side-effect
   of other processing, and that explains why the current code does
   not use |environ| at all for the moment.

   The API is kept in place for now, just in case. Thanks to Tomek
   for the patch and the persistence in tracking this down.
*/

#ifdef _WIN32
#  include <process.h>
#  define spawn_command(a,b,c) c ? \
  _spawnvpe(_P_WAIT,(const char *)a,(const char* const*)b,(const char* const*)c) : \
  _spawnvp(_P_WAIT,(const char *)a,(const char* const*)b)
#  if DONT_REALLY_EXIT
#    define exec_command(a,b,c) exit(spawn_command((a),(b),(c)))
#  else
#    define exec_command(a,b,c) c ? \
       _execvpe((const char *)a,(const char* const*)b,(const char* const*)c) : \
       _execvp((const char *)a,(const char* const*)b)
#  endif
#else
#  include <unistd.h>
#  define DEFAULT_PATH    "/bin:/usr/bin:."

static int exec_command(const char *file, char *const *av, char *const *envp)
{
    char *path;
    const char *searchpath, *esp;
    size_t prefixlen, filelen, totallen;

    if (strchr(file, '/'))      /* Specific path */
        return envp ? execve(file, av, envp) : execv(file, av);

    filelen = strlen(file);
    path = NULL;

    searchpath = getenv("PATH");
    if (!searchpath)
        searchpath = DEFAULT_PATH;

    errno = ENOENT;             /* Default errno, if execve() doesn't  change it */

    do {
        esp = strchr(searchpath, ':');
        if (esp)
            prefixlen = (size_t) (esp - searchpath);
        else
            prefixlen = strlen(searchpath);

        if (prefixlen == 0 || searchpath[prefixlen - 1] == '/') {
            totallen = prefixlen + filelen;
#  ifdef PATH_MAX
            if (totallen >= PATH_MAX)
                continue;
#  endif
            path = malloc(totallen + 1);
            memcpy(path, searchpath, prefixlen);
            memcpy(path + prefixlen, file, filelen);
        } else {
            totallen = prefixlen + filelen + 1;
#  ifdef PATH_MAX
            if (totallen >= PATH_MAX)
                continue;
#  endif
            path = malloc(totallen + 1);
            memcpy(path, searchpath, prefixlen);
            path[prefixlen] = '/';
            memcpy(path + prefixlen + 1, file, filelen);
        }
        path[totallen] = '\0';

        if (envp) {
          execve(path, av, envp);
        } else {
          execv(path, av);
        }
        free(path);
        path = NULL;
        if (errno == E2BIG || errno == ENOEXEC ||
            errno == ENOMEM || errno == ETXTBSY)
            break;              /* Report this as an error, no more search */

        searchpath = esp + 1;
    } while (esp);

    return -1;
}

/*
   It is not possible to mimic |spawnve()| completely. The main problem is
   that the |fork|--|waitpid| combination cannot really do identical error
   reporting to the parent process, because it has to pass all the possible
   error conditions as well as the actual process return status through a
   single 8-bit value.

   The current implementation tries to give back meaningful results for |execve()|
   errors in the child, for the cases that could also be returned by |spawnve()|,
   and for |ETXTBSY|, because that can be triggered by our path searching routine.

   This implementation does not differentiate abnormal status conditions reported
   by |waitpid()|, but will simply return a single error indication value.

   For all this, hyjacking a bunch of numbers in the range 1...255 is needed.
   The chance of collisions is hopefully diminished by using a rather random
   range in the 8-bit section.
*/

#  define INVALID_RET_E2BIG   143
#  define INVALID_RET_ENOENT  144
#  define INVALID_RET_ENOEXEC 145
#  define INVALID_RET_ENOMEM  146
#  define INVALID_RET_ETXTBSY 147
#  define INVALID_RET_UNKNOWN 148
#  define INVALID_RET_INTR    149

static int spawn_command(const char *file, char *const *av, char *const *envp)
{
    pid_t pid, wait_pid;
    int status;
    pid = fork();
    if (pid < 0) {
        return -1;              /* fork failed */
    }
    if (pid > 0) {              /* parent */
        status = 0;
        wait_pid = waitpid(pid, &status, 0);
        if (wait_pid == pid) {
            if (WIFEXITED(status))
                return WEXITSTATUS(status);
            else
                return INVALID_RET_INTR;
        } else {
            return -1;          /* some waitpid error */
        }
    } else {
        int f;
        /* somewhat random upper limit. ignore errors on purpose */
        for (f = 0; f < 256; f++)
            (void) fsync(f);

        if (exec_command(file, av, envp)) {
            /* let's hope no-one uses these values  */
            switch (errno) {
            case E2BIG:
                exit(INVALID_RET_E2BIG);
            case ETXTBSY:
                exit(INVALID_RET_ETXTBSY);
            case ENOENT:
                exit(INVALID_RET_ENOENT);
            case ENOEXEC:
                exit(INVALID_RET_ENOEXEC);
            case ENOMEM:
                exit(INVALID_RET_ENOMEM);
            default:
                exit(INVALID_RET_UNKNOWN);
            }
            return -1;
        }
    }
    return 0;
}

#endif

#ifdef _WIN32
static char *get_command_name(char *maincmd)
{
    /* retrieve argv[0] part from the command string,
       it will be truncated to MAX_PATH if it's too long */
    char *cmdname = (char *) malloc(sizeof(char) * MAX_PATH);
    int i, k, quoted;
    quoted = k = 0;
    for (i = 0; (i < MAX_PATH) && maincmd[i] &&
	     ((maincmd[i] != ' ' && maincmd[i] != '\t') || quoted); i++) {
        if (maincmd[i] == '"') {
            quoted = !quoted;
        } else {
            cmdname[k] = maincmd[i];
            k++;
        }
    }
    cmdname[k] = '\0';
    return cmdname;
}
#endif

static char **do_split_command(const char *maincmd, char **runcmd)
{
    char **cmdline = NULL;
#ifdef _WIN32
    /* On WIN32 don't split anything, because
       _spawnvpe can't put it back together properly
       if there are quoted arguments with spaces.
       Instead, dump everything into one argument
       and it will be passed through as is */
    cmdline = malloc(sizeof(char *) * 2);
    cmdline[0] = xstrdup(maincmd);
    cmdline[1] = NULL;
    *runcmd = get_command_name(cmdline[0]);
#else
    char *piece, *start_piece;
    const char *cmd;
    unsigned int i, j;
    int ret = 0;
    int in_string = 0;
    int quoted = 0;
    if (strlen(maincmd) == 0)
        return NULL;
    /* allocate the array of options first. it will probably be
       be a little bit too big, but better too much than to little */
    j = 2;
    for (i = 0; i < strlen(maincmd); i++) {
        if (maincmd[i] == ' ')
            j++;
    }
    cmdline = malloc(sizeof(char *) * j);
    for (i = 0; i < j; i++) {
        cmdline[i] = NULL;
    }
    cmd = maincmd;
    i = 0;
    while (cmd[i] == ' ')
        i++;                    /* skip leading spaces */
    start_piece = malloc(strlen(cmd) + 1);      /* a buffer */
    piece = start_piece;
    for (; i <= strlen(maincmd); i++) {
        if (cmd[i] == '\\' &&
            (cmd[i + 1] == '\\' || cmd[i + 1] == '\'' || cmd[i + 1] == '"')) {
            quoted = 1;
            continue;
        }
        if (in_string && cmd[i] == in_string && !quoted) {
            in_string = 0;
            continue;
        }
        if ((cmd[i] == '"' || cmd[i] == '\'') && !quoted) {
            in_string = cmd[i];
            continue;
        }
        if ((in_string == 0 && cmd[i] == ' ') || cmd[i] == 0) {
            *piece = 0;
            cmdline[ret++] = xstrdup(start_piece);
            piece = start_piece;
            while (i < strlen(maincmd) && cmd[(i + 1)] == ' ')
                i++;
            continue;
        }
        *piece++ = cmd[i];
        quoted = 0;
    }
    xfree(start_piece);
    *runcmd = cmdline[0];
#endif
    return cmdline;
}

static char **do_flatten_command(lua_State * L, char **runcmd)
{
    unsigned int i;
    int j;
    const char *s;
    char **cmdline = NULL;
    *runcmd = NULL;

    for (j = 1;; j++) {
        lua_rawgeti(L, -1, j);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            break;
        }
        lua_pop(L, 1);
    }
    if (j == 1)
        return NULL;
    cmdline = malloc(sizeof(char *) * (unsigned) (j + 1));
    for (i = 1; i <= (unsigned) j; i++) {
        cmdline[i] = NULL;
        lua_rawgeti(L, -1, (int) i);
        if (lua_isnil(L, -1) || (s = lua_tostring(L, -1)) == NULL) {
            lua_pop(L, 1);
            if (i == 1) {
                xfree(cmdline);
                return NULL;
            } else {
                break;
            }
        } else {
            lua_pop(L, 1);
            cmdline[(i - 1)] = xstrdup(s);
        }
    }
    cmdline[i] = NULL;

    lua_rawgeti(L, -1, 0);
    if (lua_isnil(L, -1) || (s = lua_tostring(L, -1)) == NULL) {
#ifdef _WIN32
        *runcmd = get_command_name(cmdline[0]);
#else
        *runcmd = cmdline[0];
#endif
    } else {
        *runcmd = xstrdup(s);
    }
    lua_pop(L, 1);

    return cmdline;
}


static int os_exec(lua_State * L)
{
    int allow = 0;
    const char *maincmd = NULL;
    char *runcmd = NULL;
    char *safecmd = NULL, *cmdname = NULL;
    char **cmdline = NULL;
    char **envblock = NULL;

    if (lua_gettop(L) != 1) {
        lua_pushnil(L);
        lua_pushliteral(L, "invalid arguments passed");
        return 2;
    }
    if (shellenabledp <= 0) {
        lua_pushnil(L);
        lua_pushliteral(L, "All command execution disabled.");
        return 2;
    }
    if (lua_type(L, 1) == LUA_TSTRING) {
        maincmd = lua_tostring(L, 1);
        cmdline = do_split_command(maincmd, &runcmd);
    } else if (lua_type(L, 1) == LUA_TTABLE) {
        cmdline = do_flatten_command(L, &runcmd);
    }
    /* If restrictedshell == 0, any command is allowed. */
    /* this is a little different from \write18/ os.execute processing
     * because it does not test for commands with fixed arguments,
     * but I am not so eager to attempt to fix that. Just document
     * that os.exec() checks only the command name.
     */
    if (restrictedshell == 0) {
        allow = 1;
    } else {
        const char *theruncmd = runcmd;
        allow = shell_cmd_is_allowed(theruncmd, &safecmd, &cmdname);
    }

    if (allow > 0 && cmdline != NULL && runcmd != NULL) {
#if defined(_WIN32) && DONT_REALLY_EXIT
        if (allow == 2)
            exec_command(safecmd, cmdline, envblock);
        else
            exec_command(runcmd, cmdline, envblock);
#else
        {
            int r;
            if (allow == 2)
                r = exec_command(safecmd, cmdline, envblock);
            else
                r = exec_command(runcmd, cmdline, envblock);
            if (r == -1) {
                lua_pushnil(L);
                lua_pushfstring(L, "%s: %s", runcmd, strerror(errno));
                lua_pushinteger(L, errno);
                return 3;
            }
        }
#endif
    }
    if (safecmd)
        free(safecmd);
    if (cmdname)
        free(cmdname);
    if (allow == 0) {
        lua_pushnil(L);
        lua_pushliteral(L, "Command execution disabled via shell_escape='p'");
        return 2;
    }
    lua_pushnil(L);
    lua_pushliteral(L, "invalid command line passed");
    return 2;
}

#define do_error_return(A,B) do {                                       \
        lua_pushnil(L);                                                 \
        lua_pushfstring(L,"%s: %s",runcmd,(A));                         \
        lua_pushinteger(L, B);                                          \
        return 3;                                                       \
    } while (0)

static int os_spawn(lua_State * L)
{
    int allow = 0;
    const char *maincmd = NULL;
    char *runcmd = NULL;
    char *safecmd = NULL, *cmdname = NULL;
    char **cmdline = NULL;
    char **envblock = NULL;
    int i;

    if (lua_gettop(L) != 1) {
        lua_pushnil(L);
        lua_pushliteral(L, "invalid arguments passed");
        return 2;
    }
    if (shellenabledp <= 0) {
        lua_pushnil(L);
        lua_pushliteral(L, "All command execution disabled.");
        return 2;
    }
    if (lua_type(L, 1) == LUA_TSTRING) {
        maincmd = lua_tostring(L, 1);
        cmdline = do_split_command(maincmd, &runcmd);
    } else if (lua_type(L, 1) == LUA_TTABLE) {
        cmdline = do_flatten_command(L, &runcmd);
    }
    /* If restrictedshell == 0, any command is allowed. */
    /* this is a little different from \write18/ os.execute processing
     * because it does not test for commands with fixed arguments,
     * but I am not so eager to attempt to fix that. Just document
     * that os.exec() checks only the command name.
     */
    if (restrictedshell == 0) {
        allow = 1;
    } else {
        const char *theruncmd = runcmd;
        allow = shell_cmd_is_allowed(theruncmd, &safecmd, &cmdname);
    }
    if (allow > 0 && cmdline != NULL && runcmd != NULL) {
        if (allow == 2)
            i = spawn_command(safecmd, cmdline, envblock);
        else
            i = spawn_command(runcmd, cmdline, envblock);
        if (safecmd)
            free(safecmd);
        if (cmdname)
            free(cmdname);
        if (i == 0) {
            lua_pushinteger(L, i);
            return 1;
        } else if (i == -1) {
            /* this branch covers WIN32 as well as fork() and waitpid() errors */
            do_error_return(strerror(errno), errno);
#ifndef _WIN32
        } else if (i == INVALID_RET_E2BIG) {
            do_error_return(strerror(E2BIG), i);
        } else if (i == INVALID_RET_ENOENT) {
            do_error_return(strerror(ENOENT), i);
        } else if (i == INVALID_RET_ENOEXEC) {
            do_error_return(strerror(ENOEXEC), i);
        } else if (i == INVALID_RET_ENOMEM) {
            do_error_return(strerror(ENOMEM), i);
        } else if (i == INVALID_RET_ETXTBSY) {
            do_error_return(strerror(ETXTBSY), i);
        } else if (i == INVALID_RET_UNKNOWN) {
            do_error_return("execution failed", i);
        } else if (i == INVALID_RET_INTR) {
            do_error_return("execution interrupted", i);
#endif
        } else {
            lua_pushinteger(L, i);
            return 1;
        }
    }
    if (safecmd)
        free(safecmd);
    if (cmdname)
        free(cmdname);
    if (allow == 0) {
        lua_pushnil(L);
        lua_pushliteral(L, "Command execution disabled via shell_escape='p'");
        return 2;
    }
    lua_pushnil(L);
    lua_pushliteral(L, "invalid command line passed");
    return 2;
}

/*  Hans wants to set env values */

static int os_setenv(lua_State * L)
{
    const char *key, *val;
    char *value;
    key = luaL_optstring(L, 1, NULL);
    val = luaL_optstring(L, 2, NULL);
    if (key) {
        if (val) {
            value = xmalloc((unsigned) (strlen(key) + strlen(val) + 2));
            sprintf(value, "%s=%s", key, val);
            if (putenv(value)) {
                return luaL_error(L, "unable to change environment");
            }
        } else {
#if defined(_WIN32) || defined(__sun__) || defined(__sun) || defined(_AIX)
            value = xmalloc(strlen(key) + 2);
            sprintf(value, "%s=", key);
            if (putenv(value)) {
                return luaL_error(L, "unable to change environment");
            }
#else
            (void) unsetenv(key);
#endif
        }
    }
    lua_pushboolean(L, 1);
    return 1;
}


static void find_env(lua_State * L)
{
    char *envitem, *envitem_orig;
    char *envkey;
    char **envpointer;
    envpointer = environ;
    lua_getglobal(L, "os");
    if (envpointer != NULL && lua_istable(L, -1)) {
        luaL_checkstack(L, 2, "out of stack space");
        lua_pushstring(L, "env");
        lua_newtable(L);
        while (*envpointer) {
            /* TODO: perhaps a memory leak here  */
            luaL_checkstack(L, 2, "out of stack space");
            envitem = xstrdup(*envpointer);
            envitem_orig = envitem;
            envkey = envitem;
            while (*envitem != '=') {
                envitem++;
            }
            *envitem = 0;
            envitem++;
            lua_pushstring(L, envkey);
            lua_pushstring(L, envitem);
            lua_rawset(L, -3);
            envpointer++;
            free(envitem_orig);
        }
        lua_rawset(L, -3);
    }
    lua_pop(L, 1);
}

static int ex_sleep(lua_State * L)
{
    lua_Number interval = luaL_checknumber(L, 1);
    lua_Number units = luaL_optnumber(L, 2, 1);
#ifdef _WIN32
    Sleep((DWORD) (1e3 * interval / units));
#else                           /* assumes posix or bsd */
    usleep((unsigned) (1e6 * interval / units));
#endif
    return 0;
}



#ifdef _WIN32
#  define _UTSNAME_LENGTH 65

/* Structure describing the system and machine.  */
struct utsname {
    char sysname[_UTSNAME_LENGTH];
    char nodename[_UTSNAME_LENGTH];
    char release[_UTSNAME_LENGTH];
    char version[_UTSNAME_LENGTH];
    char machine[_UTSNAME_LENGTH];
};

/*
 * Get name and information about current kernel.
 */
static int uname(struct utsname *uts)
{
    enum { WinNT, Win95, Win98, WinUnknown };
    OSVERSIONINFO osver;
    SYSTEM_INFO sysinfo;
    DWORD sLength;
    DWORD os = WinUnknown;

    memset(uts, 0, sizeof(*uts));

    osver.dwOSVersionInfoSize = sizeof(osver);
    GetVersionEx(&osver);
    GetSystemInfo(&sysinfo);


    /*
        Windows 10               10.0*
        Windows Server 2016      10.0*
        Windows 8.1               6.3*
        Windows Server 2012 R2    6.3*
        Windows 8                 6.2
        Windows Server 2012       6.2
        Windows 7                 6.1
        Windows Server 2008 R2	  6.1
        Windows Server 2008       6.0
        Windows Vista             6.0
        Windows Server 2003 R2	  5.2
        Windows Server 2003       5.2
        Windows XP 64-Bit Edition 5.2
        Windows XP                5.1
        Windows 2000              5.0
    */

    switch (osver.dwPlatformId) {
    case VER_PLATFORM_WIN32_NT:
        if (osver.dwMajorVersion == 4)
            strcpy(uts->sysname, "Windows NT 4");
        else if (osver.dwMajorVersion <= 3)
            strcpy(uts->sysname, "Windows NT 3");
        else if (osver.dwMajorVersion == 5) {
            if (osver.dwMinorVersion == 0)
                strcpy(uts->sysname, "Windows 2000");
            else if (osver.dwMinorVersion == 1)
                strcpy(uts->sysname, "Windows XP");
            else if (osver.dwMinorVersion == 2)
                strcpy(uts->sysname, "Windows XP 64-Bit");
        } else if (osver.dwMajorVersion == 6) {
            if (osver.dwMinorVersion == 0)
                strcpy(uts->sysname, "Windows Vista");
            else if (osver.dwMinorVersion == 1)
                strcpy(uts->sysname, "Windows 7");
            else if (osver.dwMinorVersion == 2)
                strcpy(uts->sysname, "Windows 8");
            else if (osver.dwMinorVersion == 3)
                strcpy(uts->sysname, "Windows 8.1");
        } else if (osver.dwMajorVersion == 10) {
                strcpy(uts->sysname, "Windows 10");
        }
        os = WinNT;
        break;

    case VER_PLATFORM_WIN32_WINDOWS:   /* Win95, Win98 or WinME */
        if ((osver.dwMajorVersion > 4) ||
            ((osver.dwMajorVersion == 4) && (osver.dwMinorVersion > 0))) {
            if (osver.dwMinorVersion >= 90)
                strcpy(uts->sysname, "Windows ME");     /* ME */
            else
                strcpy(uts->sysname, "Windows 98");     /* 98 */
            os = Win98;
        } else {
            strcpy(uts->sysname, "Windows 95"); /* 95 */
            os = Win95;
        }
        break;

    case VER_PLATFORM_WIN32s:  /* Windows 3.x */
        strcpy(uts->sysname, "Windows");
        break;

    default:                   /* anything else */
        strcpy(uts->sysname, "Windows");
        break;
    }

    sprintf(uts->version, "%ld.%02ld",
            osver.dwMajorVersion, osver.dwMinorVersion);

    if (osver.szCSDVersion[0] != '\0' &&
        (strlen(osver.szCSDVersion) + strlen(uts->version) + 1) <
        sizeof(uts->version)) {
        strcat(uts->version, " ");
        strcat(uts->version, osver.szCSDVersion);
    }

    sprintf(uts->release, "build %ld", osver.dwBuildNumber & 0xFFFF);

    switch (sysinfo.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_PPC:
        strcpy(uts->machine, "ppc");
        break;
    case PROCESSOR_ARCHITECTURE_ALPHA:
        strcpy(uts->machine, "alpha");
        break;
    case PROCESSOR_ARCHITECTURE_MIPS:
        strcpy(uts->machine, "mips");
        break;
    case PROCESSOR_ARCHITECTURE_INTEL:
        /*
         * dwProcessorType is only valid in Win95 and Win98 and WinME
         * wProcessorLevel is only valid in WinNT
         */
        switch (os) {
        case Win95:
        case Win98:
            switch (sysinfo.dwProcessorType) {
            case PROCESSOR_INTEL_386:
            case PROCESSOR_INTEL_486:
            case PROCESSOR_INTEL_PENTIUM:
                sprintf(uts->machine, "i%ld", sysinfo.dwProcessorType);
                break;
            default:
                strcpy(uts->machine, "i386");
                break;
            }
            break;
        case WinNT:
            sprintf(uts->machine, "i%d86", sysinfo.wProcessorLevel);
            break;
        default:
            strcpy(uts->machine, "unknown");
            break;
        }
        break;
    default:
        strcpy(uts->machine, "unknown");
        break;
    }

    sLength = sizeof(uts->nodename) - 1;
    GetComputerName(uts->nodename, &sLength);
    return 0;
}
#endif


static int ex_uname(lua_State * L)
{
    struct utsname uts;
    if (uname(&uts) >= 0) {
        lua_newtable(L);
        lua_pushstring(L, uts.sysname);
        lua_setfield(L, -2, "sysname");
        lua_pushstring(L, uts.machine);
        lua_setfield(L, -2, "machine");
        lua_pushstring(L, uts.release);
        lua_setfield(L, -2, "release");
        lua_pushstring(L, uts.version);
        lua_setfield(L, -2, "version");
        lua_pushstring(L, uts.nodename);
        lua_setfield(L, -2, "nodename");
    } else {
        lua_pushnil(L);
    }
    return 1;
}


#if (! defined (_WIN32))  && (! defined (__SUNOS__))
static int os_times(lua_State * L)
{
    struct tms r;
    (void) times(&r);
    lua_newtable(L);
    lua_pushnumber(L, /* float */
                   ((lua_Number) (r.tms_utime)) /
                   (lua_Number) sysconf(_SC_CLK_TCK));
    lua_setfield(L, -2, "utime");
    lua_pushnumber(L, /* float */
                   ((lua_Number) (r.tms_stime)) /
                   (lua_Number) sysconf(_SC_CLK_TCK));
    lua_setfield(L, -2, "stime");
    lua_pushnumber(L, /* float */
                   ((lua_Number) (r.tms_cutime)) /
                   (lua_Number) sysconf(_SC_CLK_TCK));
    lua_setfield(L, -2, "cutime");
    lua_pushnumber(L, /* float */
                   ((lua_Number) (r.tms_cstime)) /
                   (lua_Number) sysconf(_SC_CLK_TCK));
    lua_setfield(L, -2, "cstime");
    return 1;
}
#endif

#if ! defined (__SUNOS__)

#  if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#    define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#  else
#    define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#  endif

static int os_gettimeofday(lua_State * L)
{
    double v;
#  ifndef _WIN32
    struct timeval tv;
    gettimeofday(&tv, NULL);
    v = (double) tv.tv_sec + (double) tv.tv_usec / 1000000.0;
#  else
    FILETIME ft;
    int64_t tmpres = 0;

    GetSystemTimeAsFileTime(&ft);

    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;
    tmpres /= 10;
    tmpres -= DELTA_EPOCH_IN_MICROSECS; /*converting file time to unix epoch */
    v = (double) tmpres / 1000000.0;
#  endif
    lua_pushnumber(L, v); /* float */
    return 1;
}
#endif


#define MAXTRIES 36*36*36

#ifndef HAVE_MKDTEMP
static const char repl[] = "0123456789abcdefghijklmnopqrstuvwxyz";
static int dirs_made = 0;

static char *do_mkdtemp(char *tmpl)
{
    int count;
    int value;
    char *xes = &tmpl[strlen(tmpl) - 6];
    /* this is not really all that random, but it will do */
    if (dirs_made == 0) {
        srand((unsigned) time(NULL));
    }
    value = rand();
    for (count = 0; count < MAXTRIES; value += 8413, ++count) {
        int v = value;
        xes[0] = repl[v % 36];
        v /= 36;
        xes[1] = repl[v % 36];
        v /= 36;
        xes[2] = repl[v % 36];
        v /= 36;
        xes[3] = repl[v % 36];
        v /= 36;
        xes[4] = repl[v % 36];
        v /= 36;
        xes[5] = repl[v % 36];
        if (MKDIR(tmpl, S_IRUSR | S_IWUSR | S_IXUSR) >= 0) {
            dirs_made++;
            return tmpl;
        } else if (errno != EEXIST) {
            return NULL;
        }
    }
    return NULL;
}
#endif

static int os_tmpdir(lua_State * L)
{
    char *s, *tempdir;
    const char *tmp = luaL_optstring(L, 1, "luatex.XXXXXX");
    if (tmp == NULL ||
        strlen(tmp) < 6 || (strcmp(tmp + strlen(tmp) - 6, "XXXXXX") != 0)) {
        lua_pushnil(L);
        lua_pushstring(L, "Invalid argument to os.tmpdir()");
        return 2;
    } else {
        tempdir = xstrdup(tmp);
    }
#ifdef HAVE_MKDTEMP
    s = mkdtemp(tempdir);
#else
    s = do_mkdtemp(tempdir);
#endif
    if (s == NULL) {
        int en = errno;         /* calls to Lua API may change this value */
        lua_pushnil(L);
        lua_pushfstring(L, "%s", strerror(en));
        return 2;
    } else {
        lua_pushstring(L, s);
        return 1;
    }
}


static int os_execute(lua_State * L)
{
    int allow = 0;
    int ret = 1;
    char *safecmd = NULL;
    char *cmdname = NULL;
    const char *cmd = luaL_optstring(L, 1, NULL);

    if (cmd == NULL) { /* pretend we are \.{\\pdfshellescape} */
	if (shellenabledp <= 0) {
	    lua_pushinteger(L, 0);
        } else if (restrictedshell == 0) {
	    lua_pushinteger(L, 1);
	} else {
	    lua_pushinteger(L, 2);
	}
	return 1;
    }
    if (shellenabledp <= 0) {
        lua_pushnil(L);
        lua_pushstring(L, "All command execution disabled.");
        return 2;
    }
    /* If restrictedshell == 0, any command is allowed. */
    if (restrictedshell == 0)
        allow = 1;
    else
        allow = shell_cmd_is_allowed(cmd, &safecmd, &cmdname);

    if (allow == 1) {
        lua_pushinteger(L, system(cmd));
    } else if (allow == 2) {
        lua_pushinteger(L, system(safecmd));
    } else {
        lua_pushnil(L);
        ret = 2;
        if (allow == 0)
            lua_pushstring(L,
                           "Command execution disabled via shell_escape='p'");
        else                    /* allow == -1 */
            lua_pushstring(L, "Quoting error in system command line.");
    }
    if (safecmd)
        free(safecmd);
    if (cmdname)
        free(cmdname);
    return ret;
}


void open_oslibext(lua_State * L)
{

    find_env(L);

    lua_getglobal(L, "os");
    lua_pushcfunction(L, ex_sleep);
    lua_setfield(L, -2, "sleep");
    lua_pushliteral(L, OS_PLATTYPE);
    lua_setfield(L, -2, "type");
    lua_pushliteral(L, OS_PLATNAME);
    lua_setfield(L, -2, "name");
    lua_pushcfunction(L, ex_uname);
    lua_setfield(L, -2, "uname");
#if (! defined (_WIN32))  && (! defined (__SUNOS__))
    lua_pushcfunction(L, os_times);
    lua_setfield(L, -2, "times");
#endif
#if ! defined (__SUNOS__)
    lua_pushcfunction(L, os_gettimeofday);
    lua_setfield(L, -2, "gettimeofday");
#endif

    lua_pushcfunction(L, os_setenv);
    lua_setfield(L, -2, "setenv");
    lua_pushcfunction(L, os_exec);
    lua_setfield(L, -2, "exec");
    lua_pushcfunction(L, os_spawn);
    lua_setfield(L, -2, "spawn");
    lua_pushcfunction(L, os_execute);
    lua_setfield(L, -2, "execute");
    lua_pushcfunction(L, os_tmpdir);
    lua_setfield(L, -2, "tmpdir");

    lua_pop(L, 1);              /* pop the table */
}
