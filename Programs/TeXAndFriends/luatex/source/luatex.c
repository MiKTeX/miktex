/* luatex.c: Hand-coded routines for TeX or Metafont in C.  Originally
   written by Tim Morgan, drawing from other Unix ports of TeX.  This is
   a collection of miscellany, everything that's easier (or only
   possible) to do in C.

   This file is public domain.  */

/* This file is used to create texextra.c etc., with this line
   changed to include texd.h or mfd.h.  The ?d.h file is what
   #defines TeX or MF, which avoids the need for a special
   Makefile rule.  */

/* We |#define DLLPROC| in order to build LuaTeX and LuajitTeX as DLL
   for W32TeX.  */
#if defined LuajitTeX
#define DLLPROC dllluajittexmain
#else
#define DLLPROC dllluatexmain
#endif

#include "ptexlib.h"
#include "luatex.h"
#include "lua/luatex-api.h"
/*
#include "luatex_svnversion.h"
*/


#define TeX

int luatex_version = 100;        /* \.{\\luatexversion}  */
int luatex_revision = '4';      /* \.{\\luatexrevision}  */
int luatex_date_info = 2017021514;     /* the compile date is now hardwired :YEAR MONTH DAY HOUR*/
const char *luatex_version_string = "1.0.4";
const char *engine_name = my_name;     /* the name of this engine */

#include <kpathsea/c-ctype.h>
#include <kpathsea/line.h>
#include <kpathsea/readable.h>
#include <kpathsea/variable.h>
#include <kpathsea/absolute.h>
#ifdef WIN32
#include <kpathsea/concatn.h>
#endif

#ifdef _MSC_VER
#undef timezone
#endif

#include <time.h>               /* For `struct tm'.  */
#if defined (HAVE_SYS_TIME_H)
#  include <sys/time.h>
#elif defined (HAVE_SYS_TIMEB_H)
#  include <sys/timeb.h>
#endif

#if defined(__STDC__)
#  include <locale.h>
#endif

#include <signal.h>             /* Catch interrupts.  */



/* Shell escape.

   If shellenabledp == 0, all shell escapes are forbidden.
   If (shellenabledp == 1 && restrictedshell == 0), any command
     is allowed for a shell escape.
   If (shellenabledp == 1 && restrictedshell == 1), only commands
     given in the configuration file as
   shell_escape_commands = kpsewhich,ebb,extractbb,mpost,metafun,...
     (no spaces between commands) in texmf.cnf are allowed for a shell
     escape in a restricted form: command name and arguments should be
     separated by a white space. The first word should be a command
     name. The quotation character for an argument with spaces,
     including a pathname, should be ".  ' should not be used.

     Internally, all arguments are quoted by ' (Unix) or " (Windows)
     before calling the system() function in order to forbid execution
     of any embedded command.

   If the --shell-escape option is given, we set
     shellenabledp = 1 and restrictedshell = 0, i.e., any command is allowed.
   If the --shell-restricted option is given, we set
     shellenabledp = 1 and restrictedshell = 1, i.e., only given cmds allowed.
   If the --no-shell-escape option is given, we set
     shellenabledp = -1 (and restrictedshell is irrelevant).
   If none of these option are given, there are three cases:
   (1) In the case where
       shell_escape = y or
       shell_escape = t or
       shell_escape = 1
       it becomes shellenabledp = 1 and restrictedshell = 0,
       that is, any command is allowed.
   (2) In the case where
       shell_escape = p
       it becomes shellenabledp = 1 and restrictedshell = 1,
       that is, restricted shell escape is allowed.
   (3) In all other cases, shellenabledp = 0, that is, shell
       escape is forbidden. The value of restrictedshell is
       irrelevant if shellenabledp == 0.
*/

#ifdef TeX

/* cmdlist is a list of allowed commands which are given like this:
   shell_escape_commands = kpsewhich,ebb,extractbb,mpost,metafun
   in texmf.cnf. */

static char **cmdlist = NULL;

void mk_shellcmdlist(char *v)
{
    char **p;
    char *q, *r;
    size_t n;

    q = v;
    n = 1;

/* analyze the variable shell_escape_commands = foo,bar,...
   spaces before and after (,) are not allowed. */

    while ((r = strchr(q, ',')) != 0) {
        n++;
        q = r + 1;
    }
    if (*q)
        n++;
    cmdlist = (char **) xmalloc(n * sizeof (char *));
    p = cmdlist;
    q = v;
    while ((r = strchr(q, ',')) != 0) {
        *r = '\0';
        *p++ = xstrdup (q);
        q = r + 1;
    }
    if (*q)
        *p++ = xstrdup (q);
    *p = NULL;
}

/* Called from maininit.  Not static because also called from
   luatexdir/lua/luainit.c.  */

/*
    In order to avoid all kind of time code in the backend code we use a function.
    The start time can be overloaded in several ways:

    (1) By setting the environmment variable SOURCE_DATE_EPOCH. This will influence
    the tex parameters, random seed, pdf timestamp and pdf id that is derived
    from the time. This variable is consulted when the kpse library is enabled
    which is analogue to other properties.

    (2) By setting the texconfig.start_time variable (as with other variables
    we use the internal name there). This has the same effect as (1) and is
    provided for when kpse is not used to set these variables or when an overloaded
    is wanted. This is analogue to other properties.

    When an utc time is needed one can provide the flag --utc. This property is
    independent of this time hackery. This flag has a corresponding texconfig
    option use_utc_time.

    To some extend a cleaner solution would be to have a flag that disables all
    variable data in one go (like filenames and so) but we just follow the method
    implemented in pdftex where primitives are used tod disable it.

*/

static int start_time = -1;

int get_start_time(void) {
    if (start_time < 0) {
        start_time = time((time_t *) NULL);
    }
    return start_time;
}

/*
    This one is called as part of the kpse initialization which only happens
    when this library is enabled.
*/
#if defined(_MSC_VER)
#define strtoull _strtoui64
#endif

void init_start_time(void) {
    if (start_time < 0) {
        unsigned long long epoch;
        char *endptr;
        /*
            We don't really care how kpse sets up this variable but we prefer to
            just use its abstract interface.
        */
     /* char *source_date_epoch = getenv("SOURCE_DATE_EPOCH"); */
        char *source_date_epoch = kpse_var_value("SOURCE_DATE_EPOCH");
        if (source_date_epoch && source_date_epoch != '\0' ) {
            errno = 0;
            epoch = strtoull(source_date_epoch, &endptr, 10);
            if (*endptr != '\0' || errno != 0) {
                epoch = 0;
            }
#if defined(_MSC_VER)
            /* We avoid to crash if users test a large value which is not
             * supported by Visual Studio 2010:
             * a later time than 3001/01/01 20:59:59.
             */
            if (epoch > 32535291599ULL)
                epoch = 32535291599ULL;
#endif
            start_time = epoch;
        }
    }
}

/*
    This one is used to fetch a value from texconfig which can also be used to
    set properties. This might come in handy when one has other ways to get date
    info in the pdf file.
*/

void set_start_time(int s) {
    if (s >= 0) {
        start_time = s ;
    }
}

void init_shell_escape(void)
{
    if (shellenabledp < 0) {    /* --no-shell-escape on cmd line */
        shellenabledp = 0;

    } else {
        if (shellenabledp == 0) {       /* no shell options on cmd line, check cnf */
            char *v1 = kpse_var_value("shell_escape");
            if (v1) {
                if (*v1 == 't' || *v1 == 'y' || *v1 == '1') {
                    shellenabledp = 1;
                } else if (*v1 == 'p') {
                    shellenabledp = 1;
                    restrictedshell = 1;
                }
                free(v1);
            }
        }

        /* If shell escapes are restricted, get allowed cmds from cnf.  */
        if (shellenabledp && restrictedshell == 1) {
            char *v2 = kpse_var_value("shell_escape_commands");
            if (v2) {
                mk_shellcmdlist(v2);
                free(v2);
            }
        }
    }
}

#  ifdef WIN32
#    define QUOTE '"'
#  else
#    define QUOTE '\''
#  endif

#  if 0
#  ifdef WIN32
#    if !defined(MIKTEX)
static int char_needs_quote(int c)
{
/* special characters of cmd.exe */

    return (c == '&' || c == '|' || c == '%' || c == '<' ||
            c == '>' || c == ';' || c == ',' || c == '(' || c == ')');
}
#    endif
#  endif
#  endif

static int Isspace(char c)
{
    return (c == ' ' || c == '\t');
}

/* return values:
  -1 : invalid quotation of an argument
   0 : command is not allowed
   2 : restricted shell escape, CMD is allowed.

   We set *SAFECMD to a safely-quoted version of *CMD; this is what
   should get executed.  And we set CMDNAME to its first word; this is
   what is checked against the shell_escape_commands list.  */

int shell_cmd_is_allowed(const char *cmd, char **safecmd, char **cmdname)
{
    char **p;
    char *buf;
    char *c, *d;
    const char *s;
    int pre;
    unsigned spaces;
    int allow = 0;

    /* pre == 1 means that the previous character is a white space
       pre == 0 means that the previous character is not a white space */
    buf = xmalloc(strlen(cmd) + 1);
    strcpy(buf, cmd);
    c = buf;
    while (Isspace(*c))
        c++;
    d = c;
    while (!Isspace(*d) && *d)
        d++;
    *d = '\0';

    /* *cmdname is the first word of the command line.  For example,
     *cmdname == "kpsewhich" for
     \write18{kpsewhich --progname=dvipdfm --format="other text files" config}
     */
    *cmdname = xstrdup(c);
    free(buf);

    /* Is *cmdname listed in a texmf.cnf vriable as
       shell_escape_commands = foo,bar,... ? */
    p = cmdlist;
    if (p) {
        while (*p) {
            if (strcmp(*p, *cmdname) == 0) {
                /* *cmdname is found in the list, so restricted shell escape
                   is allowed */
                allow = 2;
                break;
            }
            p++;
        }
    }
    if (allow == 2) {
        spaces = 0;
        for (s = cmd; *s; s++) {
            if (Isspace(*s))
                spaces++;
        }

        /* allocate enough memory (too much?) */
#  ifdef WIN32
        *safecmd = xmalloc(2 * strlen(cmd) + 3 + 2 * spaces);
#  else
        *safecmd = xmalloc(strlen(cmd) + 3 + 2 * spaces);
#  endif

        /* make a safe command line *safecmd */
        s = cmd;
        while (Isspace(*s))
            s++;
        d = *safecmd;
        while (!Isspace(*s) && *s)
            *d++ = *s++;

        pre = 1;
        while (*s) {
            /* Quotation given by a user.  " should always be used; we
               transform it below.  On Unix, if ' is used, simply immediately
               return a quotation error.  */
            if (*s == '\'') {
                return -1;
            }

            if (*s == '"') {
                /* All arguments are quoted as 'foo' (Unix) or "foo" (Windows)
                   before calling system(). Therefore closing QUOTE is necessary
                   if the previous character is not a white space.
                   example:
                   --format="other text files" becomes
                   '--format=''other text files' (Unix)
                   "--format"="other text files" (Windows) */

                if (pre == 0) {
#  ifdef WIN32
                    if (*(s-1) == '=') {
                        *(d-1) = QUOTE;
                        *d++ = '=';
                    } else {
                      *d++ = QUOTE;
                    }
#  else
                    *d++ = QUOTE;
#  endif
                }

                pre = 0;
                /* output the quotation mark for the quoted argument */
                *d++ = QUOTE;
                s++;

                while (*s != '"') {
                    /* Illegal use of ', or closing quotation mark is missing */
                    if (*s == '\'' || *s == '\0')
                        return -1;
#  if 0
#  ifdef WIN32
#    if !defined(MIKTEX)
                    if (char_needs_quote(*s))
                        *d++ = '^';
#    endif
#  endif
#  endif
                    *d++ = *s++;
                }

                /* Closing quotation mark will be output afterwards, so
                   we do nothing here */
                s++;

                /* The character after the closing quotation mark
                   should be a white space or NULL */
                if (!Isspace(*s) && *s)
                    return -1;

                /* Beginning of a usual argument */
            } else if (pre == 1 && !Isspace(*s)) {
                pre = 0;
                *d++ = QUOTE;
#  if 0
#  ifdef WIN32
#    if !defined(MIKTEX)
                if (char_needs_quote(*s))
                    *d++ = '^';
#    endif
#  endif
#  endif
                *d++ = *s++;
                /* Ending of a usual argument */

            } else if (pre == 0 && Isspace(*s)) {
                pre = 1;
                /* Closing quotation mark */
                *d++ = QUOTE;
                *d++ = *s++;
            } else {
                /* Copy a character from cmd to *safecmd. */
#  if 0
#  ifdef WIN32
#    if !defined(MIKTEX)
                if (char_needs_quote(*s))
                    *d++ = '^';
#    endif
#  endif
#  endif
                *d++ = *s++;
            }
        }
        /* End of the command line */
        if (pre == 0) {
            *d++ = QUOTE;
        }
        *d = '\0';
#ifdef WIN32
#if defined(MIKTEX)
#else
        {
          char *p, *q, *r;
          p = *safecmd;
          if (strlen (p) > 2 && p[1] == ':' && !IS_DIR_SEP (p[2])) {
              q = xmalloc (strlen (p) + 2);
              q[0] = p[0];
              q[1] = p[1];
              q[2] = '/';
              q[3] = '\0';
              strcat (q, (p + 2));
              free (*safecmd);
              *safecmd = q;
          } else if (!IS_DIR_SEP (p[0]) && !(p[1] == ':' && IS_DIR_SEP (p[2]))) {
            p = kpse_var_value ("SELFAUTOLOC");
            if (p) {
              r = *safecmd;
              while (*r && !Isspace(*r))
                r++;
              if (*r == '\0')
                q = concatn ("\"", p, "/", *safecmd, "\"", NULL);
              else {
                *r = '\0';
                r++;
                while (*r && Isspace(*r))
                  r++;
                if (*r)
                  q = concatn ("\"", p, "/", *safecmd, "\" ", r, NULL);
                else
                  q = concatn ("\"", p, "/", *safecmd, "\"", NULL);
              }
              free (p);
              free (*safecmd);
              *safecmd = q;
            }
          }
        }
#endif
#endif
    }

    return allow;
}

#endif


/* What we were invoked as and with.  */
char **argv;
int argc;

/* The C version of what might wind up in |TEX_format_default|.  */
string dump_name;

/* The C version of the jobname, if given. */
const_string c_job_name;

const char *luatex_banner;

#ifdef _MSC_VER
/* Invalid parameter handler */
static void myInvalidParameterHandler(const wchar_t * expression,
   const wchar_t * function,
   const wchar_t * file,
   unsigned int line,
   uintptr_t pReserved)
{
/*
   printf(L"Invalid parameter detected in function %s."
            L" File: %s Line: %d\n", function, file, line);
   printf(L"Expression: %s\n", expression);
*/
/*
   I return silently to avoid an exit with the error 0xc0000417
   (invalid parameter) when we use non-embedded fonts in luatex-ja,
   which works without any problem on Unix systems.
   I hope it is not dangerous.
*/
   return;
}
#endif

/* The entry point: set up for reading the command line, which will
   happen in `topenin', then call the main body.  */

#if defined(MIKTEX)
int MIKTEXCEECALL Main (int ac, string * av)
#else
int
#if defined(DLLPROC)
DLLPROC (int ac, string *av)
#else
main (int ac, string *av)
#endif
#endif
{
#if defined(MIKTEX) && WITH_CONTEXT_SUPPORT
  // create texmf.cnf for ConTeXt
  char * lpszPath = miktex_kpsemu_create_texmf_cnf();
  xputenv ("TEXMFCNF", lpszPath);
  free (lpszPath);
  // this is also for ConTeXt
  lpszPath = kpse_var_value("TEXMFVAR");
  xputenv ("TEXMFCACHE", lpszPath);
  free (lpszPath);
  // this is also for ConTeXt
  lpszPath = kpse_var_value("TEXMF");
  xputenv ("TEXMF", lpszPath);
  free (lpszPath);
#endif

#  ifdef __EMX__
    _wildcard(&ac, &av);
    _response(&ac, &av);
#  endif

#  ifdef WIN32
#    ifdef _MSC_VER
    _set_invalid_parameter_handler(myInvalidParameterHandler);
#    endif
    av[0] = kpse_program_basename (av[0]);
    _setmaxstdio(2048);
/*
 *  We choose to crash for fatal errors
    SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
 */
    setmode(fileno(stdin), _O_BINARY);
#  endif

    lua_initialize(ac, av);

#  ifdef WIN32
    if (ac > 1) {
      char *pp;
      if ((strlen(av[ac-1]) > 2) &&
          isalpha(av[ac-1][0]) &&
          (av[ac-1][1] == ':') &&
          (av[ac-1][2] == '\\')) {
        for (pp=av[ac-1]+2; *pp; pp++) {
#if !defined(MIKTEX)
          if (IS_KANJI(pp)) {
            pp++;
            continue;
          }
#endif
          if (*pp == '\\')
            *pp = '/';
        }
      }
    }
#  endif

    /* Call the real main program.  */
    main_body();

    return EXIT_SUCCESS;
}


/* This is supposed to ``open the terminal for input'', but what we
   really do is copy command line arguments into TeX's or Metafont's
   buffer, so they can handle them.  If nothing is available, or we've
   been called already (and hence, argc==0), we return with
   `last=first'.  */

void topenin(void)
{
    int i;


    buffer[first] = 0;          /* In case there are no arguments.  */

    if (optind < argc) {        /* We have command line arguments.  */
        int k = first;
        for (i = optind; i < argc; i++) {
            char *ptr = &(argv[i][0]);
            /* Don't use strcat, since in Aleph the buffer elements aren't
               single bytes.  */
            while (*ptr) {
                buffer[k++] = (packed_ASCII_code) * (ptr++);
            }
            buffer[k++] = ' ';
        }
        argc = 0;               /* Don't do this again.  */
        buffer[k] = 0;
    }

    /* Find the end of the buffer.  */
    for (last = first; buffer[last]; ++last);

    /* Make `last' be one past the last non-blank character in `buffer'.  */
    /* ??? The test for '\r' should not be necessary.  */
    for (--last; last >= first
         && ISBLANK(buffer[last]) && buffer[last] != '\r'; --last);
    last++;

    /* One more time, this time converting to TeX's internal character
       representation.  */
}

/* IPC for TeX.  By Tom Rokicki for the NeXT; it makes TeX ship out the
   DVI file in a pipe to TeXView so that the output can be displayed
   incrementally.  Shamim Mohamed adapted it for Web2c.  */
#if defined (TeX) && defined (IPC)

#ifdef WIN32
#undef _WINSOCKAPI_
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <fcntl.h>
#ifndef O_NONBLOCK /* POSIX */
#ifdef O_NDELAY    /* BSD */
#define O_NONBLOCK O_NDELAY
#elif defined(O_FNDELAY)     /* NeXT */
#define O_NONBLOCK O_FNDELAY
#else
what the fcntl? cannot implement IPC without equivalent for O_NONBLOCK.
#endif
#endif /* no O_NONBLOCK */
#endif /* !WIN32 */

#ifdef WIN32
# define IPC_AF AF_INET
# ifndef IPC_LOCAL_HOST
#  define IPC_LOCAL_HOST "127.0.0.1"
#  define FIXED_PORT     (unsigned short)4242
# endif
#else
# define IPC_AF AF_UNIX
# ifndef IPC_PIPE_NAME /* $HOME is prepended to this.  */
#  define IPC_PIPE_NAME "/.TeXview_Pipe"
# endif
#endif
#ifndef IPC_SERVER_CMD /* Command to run to start the server.  */
# ifdef WIN32
#  define IPC_SERVER_CMD "texview.exe"
# else
#  define IPC_SERVER_CMD "open `which TeXview`"
# endif
#endif

struct msg
{
  int   namelength; /* length of auxiliary data */
  int   eof;        /* new eof for dvi file */
#if 0  /* see usage of struct msg below */
  char more_data[0]; /* where the rest of the stuff goes */
#endif
};

static struct sockaddr *ipc_addr;
static int ipc_addr_len;

static int
ipc_make_name (void)
{
  if (ipc_addr_len == 0) {
#ifdef WIN32
    unsigned long remote_addr = inet_addr(IPC_LOCAL_HOST);
    if (remote_addr != INADDR_NONE) {
      struct sockaddr_in *ipc_sin_addr = xmalloc (sizeof (struct sockaddr_in));
      ipc_sin_addr->sin_family = AF_INET;
      ipc_sin_addr->sin_addr.s_addr = remote_addr;
      ipc_sin_addr->sin_port = htons (FIXED_PORT);
      ipc_addr = ((struct sockaddr *) ipc_sin_addr);
      ipc_addr_len = sizeof(struct sockaddr_in);
    }
#else
    string s = getenv ("HOME");
    if (s) {
      char *ipc_name;
      ipc_addr = xmalloc (strlen (s) + 40);
      ipc_addr->sa_family = 0;
      ipc_name = ipc_addr->sa_data;
      strcpy (ipc_name, s);
      strcat (ipc_name, IPC_PIPE_NAME);
      ipc_addr_len = strlen (ipc_name) + 3;
    }
#endif
  }
  return ipc_addr_len;
}

static int sock = -1;

#ifdef WIN32
# define CLOSE_SOCKET(s) closesocket (s); WSACleanup ()
#else
# define CLOSE_SOCKET(s) close (s)
#endif

static int
ipc_is_open (void)
{
   return sock != -1;
}

static void
ipc_open_out (void) {
#ifdef WIN32
  struct WSAData wsaData;
  int nCode;
  unsigned long mode = 1;
#endif
#ifdef IPC_DEBUG
  fputs ("tex: Opening socket for IPC output ...\n", stderr);
#endif
  if (sock != -1) {
    return;
  }

#ifdef WIN32
  if ((nCode = WSAStartup(MAKEWORD(1, 1), &wsaData)) != 0) {
    fprintf(stderr,"WSAStartup() returned error code %d.\n", nCode);
    return;
  }
#endif

  if (ipc_make_name () <= 0)
    return;

  sock = socket (IPC_AF, SOCK_STREAM, 0);
#ifdef IPC_DEBUG
  if(sock != -1)
    fprintf(stderr, "tex: Socket handle is %d\n", sock);
  else
    fprintf(stderr, "tex: Socket is invalid.\n");
#endif

  if (sock != -1) {
    if (connect (sock, ipc_addr, ipc_addr_len) != 0 ||
#ifdef WIN32
        ioctlsocket (sock, FIONBIO, &mode) < 0
#else
        fcntl (sock, F_SETFL, O_NONBLOCK) < 0
#endif
        ) {
      CLOSE_SOCKET (sock);
      sock = -1;
#ifdef IPC_DEBUG
      fputs ("tex: IPC socket cannot be connected.\n", stderr);
      fputs ("tex: Socket is closed.\n", stderr);
#endif
      return;
    }
#ifdef IPC_DEBUG
    fputs ("tex: Successfully opened IPC socket.\n", stderr);
#endif
  }
}

static void
ipc_close_out (void)
{
#ifdef IPC_DEBUG
  fputs ("tex: Closing output socket ...\n", stderr);
#endif
  if (ipc_is_open ()) {
    CLOSE_SOCKET (sock);
    sock = -1;
  }
}

static void
ipc_snd (int n, int is_eof, char *data)
{
  struct
  {
    struct msg msg;
    char more_data[1024];
  } ourmsg;

  if (!ipc_is_open ()) {
    return;
  }

#ifdef IPC_DEBUG
  fprintf(stderr, "%d\t%d\n", ourmsg.msg.namelength, ourmsg.msg.eof);
  fputs ("tex: Sending message to socket ...\n", stderr);
#endif
  ourmsg.msg.namelength = n;
  ourmsg.msg.eof = is_eof;
  if (n) {
    strcpy (ourmsg.more_data, data);
  }
  n += sizeof (struct msg);
#ifdef IPC_DEBUG
  fprintf(stderr, "%d\t%d\n", ourmsg.msg.namelength, ourmsg.msg.eof);
  fputs ("tex: Writing to socket...\n", stderr);
#endif
#if defined(WIN32)
  if (send (sock, (char *)&ourmsg, n, 0) != n) {
#else
  if (write (sock, &ourmsg, n) != n) {
#endif
    ipc_close_out ();
  }
#ifdef IPC_DEBUG
  fputs ("tex: IPC message sent.\n", stderr);
#endif
}

/* This routine notifies the server if there is an eof, or the filename
   if a new DVI file is starting.  This is the routine called by TeX.
   Aleph defines str_start(#) as str_start_ar[# - too_big_char], with
   too_big_char = biggest_char + 1 = 65536 (omstr.ch).  */

void
ipcpage (int is_eof)
{
  static boolean begun = false;
  unsigned len = 0;
  string p = NULL;

  if (!begun) {
    string name; /* Just the filename.  */
    string cwd = xgetcwd ();

    ipc_open_out ();

    /* Have to pass whole filename to the other end, since it may have
       been started up and running as a daemon, e.g., as with the NeXT
       preview program.  */
    name = static_pdf->file_name;
    p = concat3 (cwd, DIR_SEP_STRING, name);
    free (cwd);
    free (name);

#if defined (WIN32)
    { char *q;
      for (q = p; *q; q++) {
        if (*q == '\\')
          *q = '/';
        else if (IS_KANJI(q))
          q++;
      }
    }
#endif
    len = strlen(p);
    begun = true;
  }
  ipc_snd (len, is_eof, p);

  if (p)
    free (p);
}
#endif /* TeX && IPC */

/* Normalize quoting of filename -- that is, only quote if there is a space,
   and always use the quote-name-quote style. */
string normalize_quotes(const_string name, const_string mesg)
{
    boolean quoted = false;
    boolean must_quote = (strchr(name, ' ') != NULL);
    /* Leave room for quotes and NUL. */
    string ret = (string) xmalloc((unsigned) strlen(name) + 3);
    string p;
    const_string q;
    p = ret;
    if (must_quote)
        *p++ = '"';
    for (q = name; *q; q++) {
        if (*q == '"')
            quoted = !quoted;
        else
            *p++ = *q;
    }
    if (must_quote)
        *p++ = '"';
    *p = '\0';
    if (quoted) {
        fprintf(stderr, "! Unbalanced quotes in %s %s\n", mesg, name);
        uexit(1);
    }
    return ret;
}


/* All our interrupt handler has to do is set TeX's or Metafont's global
   variable `interrupt'; then they will do everything needed.  */
#ifdef WIN32
/* Win32 doesn't set SIGINT ... */
static BOOL WINAPI catch_interrupt(DWORD arg)
{
    switch (arg) {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
        interrupt = 1;
        return TRUE;
    default:
        /* No need to set interrupt as we are exiting anyway */
        return FALSE;
    }
}
#else                           /* not WIN32 */
static RETSIGTYPE catch_interrupt(int arg)
{
    (void) arg;
    interrupt = 1;
#  ifdef OS2
    (void) signal(SIGINT, SIG_ACK);
#  else
    (void) signal(SIGINT, catch_interrupt);
#  endif                        /* not OS2 */
}
#endif                          /* not WIN32 */

/* Besides getting the date and time here, we also set up the interrupt
   handler, for no particularly good reason.  It's just that since the
   `fix_date_and_time' routine is called early on (section 1337 in TeX,
   ``Get the first line of input and prepare to start''), this is as
   good a place as any.  */

void get_date_and_time(int *minutes, int *day, int *month, int *year)
{
    time_t myclock = get_start_time();
    struct tm *tmptr ;
    if (utc_option) {
        tmptr = gmtime(&myclock);
    } else {
        tmptr = localtime(&myclock);
    }

    *minutes = tmptr->tm_hour * 60 + tmptr->tm_min;
    *day = tmptr->tm_mday;
    *month = tmptr->tm_mon + 1;
    *year = tmptr->tm_year + 1900;

    {
#ifdef SA_INTERRUPT
        /* Under SunOS 4.1.x, the default action after return from the
           signal handler is to restart the I/O if nothing has been
           transferred.  The effect on TeX is that interrupts are ignored if
           we are waiting for input.  The following tells the system to
           return EINTR from read() in this case.  From ken@cs.toronto.edu.  */

        struct sigaction a, oa;

        a.sa_handler = catch_interrupt;
        sigemptyset(&a.sa_mask);
        sigaddset(&a.sa_mask, SIGINT);
        a.sa_flags = SA_INTERRUPT;
        sigaction(SIGINT, &a, &oa);
        if (oa.sa_handler != SIG_DFL)
            sigaction(SIGINT, &oa, (struct sigaction *) 0);
#else                           /* no SA_INTERRUPT */
#  ifdef WIN32
        SetConsoleCtrlHandler(catch_interrupt, TRUE);
#  else                         /* not WIN32 */
        RETSIGTYPE(*old_handler) (int);

        old_handler = signal(SIGINT, catch_interrupt);
        if (old_handler != SIG_DFL)
            signal(SIGINT, old_handler);
#  endif                        /* not WIN32 */
#endif                          /* no SA_INTERRUPT */
    }
}

/*
 Getting a high resolution time.
 */
void get_seconds_and_micros(int *seconds, int *micros)
{
#if defined (HAVE_GETTIMEOFDAY)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *seconds = (int)tv.tv_sec;
    *micros = (int)tv.tv_usec;
#elif defined (HAVE_FTIME)
    struct timeb tb;
    ftime(&tb);
    *seconds = tb.time;
    *micros = tb.millitm * 1000;
#else
    time_t myclock = time((time_t *) NULL);
    *seconds = (int) myclock;
    *micros = 0;
#endif
}

/*
  Generating a better seed numbers
  */
int getrandomseed(void)
{
#if defined (HAVE_GETTIMEOFDAY)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int)(tv.tv_usec + 1000000 * tv.tv_usec);
#elif defined (HAVE_FTIME)
    struct timeb tb;
    ftime(&tb);
    return (tb.millitm + 1000 * tb.time);
#else
#if defined(MIKTEX)
    time_t myclock = get_start_time();
#else
    time_t myclock = get_start_time((time_t *) NULL);
#endif
    struct tm *tmptr ;
    if (utc_option) {
        tmptr = gmtime(&myclock);
    } else {
        tmptr = localtime(&myclock);
    }
    return (tmptr->tm_sec + 60 * (tmptr->tm_min + 60 * tmptr->tm_hour));
#endif
}

/* Read a line of input as efficiently as possible while still looking
   like Pascal.  We set `last' to `first' and return `false' if we get
   to eof.  Otherwise, we return `true' and set last = first +
   length(line except trailing whitespace).  */

boolean input_line(FILE * f)
{
    int i = EOF;

#ifdef WIN32
    if (f != Poptr && fileno (f) != fileno (stdin)) {
        long position = ftell (f);

        if (position == 0L) {  /* Detect and skip Byte order marks.  */
            int k1 = getc (f);

            if (k1 != 0xff && k1 != 0xfe && k1 != 0xef)
                rewind (f);
            else {
                int k2 = getc (f);

                if (k2 != 0xff && k2 != 0xfe && k2 != 0xbb)
                    rewind (f);
                else if ((k1 == 0xff && k2 == 0xfe) || /* UTF-16(LE) */
                         (k1 == 0xfe && k2 == 0xff))   /* UTF-16(BE) */
                    ;
                else {
                    int k3 = getc (f);

                    if (k1 == 0xef && k2 == 0xbb && k3 == 0xbf) /* UTF-8 */
                        ;
                    else
                        rewind (f);
                }
            }
        }
    }
#endif
    /* Recognize either LF or CR as a line terminator.  */
    last = first;
    while (last < buf_size && (i = getc(f)) != EOF && i != '\n' && i != '\r')
        buffer[last++] = (packed_ASCII_code) i;

    if (i == EOF && errno != EINTR && last == first)
        return false;

    /* We didn't get the whole line because our buffer was too small.  */
    if (i != EOF && i != '\n' && i != '\r') {
        fprintf(stderr, "! Unable to read an entire line---bufsize=%u.\n",
                (unsigned) buf_size);
        fputs("Please increase buf_size in texmf.cnf.\n", stderr);
        uexit(1);
    }

    buffer[last] = ' ';
    if (last >= max_buf_stack)
        max_buf_stack = last;

    /* If next char is LF of a CRLF, read it.  */
    if (i == '\r') {
        while ((i = getc(f)) == EOF && errno == EINTR);
        if (i != '\n')
            ungetc(i, f);
    }

    /* Trim trailing whitespace.  */
    while (last > first && ISBLANK(buffer[last - 1]))
        --last;

    /* Don't bother using xord if we don't need to.  */

    return true;
}




/* Get the job name to be used, which may have been set from the
   command line. */
str_number getjobname(str_number name)
{
    str_number ret = name;
    if (c_job_name != NULL)
        ret = maketexstring(c_job_name);
    return ret;
}
