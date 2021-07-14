/*
 *   The search routine takes a directory list, separated by PATHSEP, and
 *   tries to open a file.  Null directory components indicate current
 *   directory.  In an environment variable, null directory components
 *   indicate substitution of the default path list at that point.
 *   This source file assumes KPATHSEA is defined.
 */
#include "dvips.h" /* The copyright notice in that file is included too! */
#if defined(MIKTEX)
#include <miktex/Core/c/api.h>
#endif

#ifndef KPATHSEA
"goodbye, search.c can only be compiled with -DKPATHSEA"
#endif

#include "protos_add.h" /* external declarations */

#include <kpathsea/c-ctype.h>
#include <kpathsea/tex-file.h>
#include <kpathsea/tex-glyph.h>
#include <kpathsea/absolute.h>
#include <kpathsea/c-pathch.h>
#include <kpathsea/variable.h>

#if !defined(MIKTEX)
#ifdef WIN32
#undef fopen
#undef popen
#undef pclose
extern FILE *generic_fsyscp_fopen(const char *name, const char *mode);
#define fopen(file, fmode)  generic_fsyscp_fopen(file, fmode)
#define popen(pcmd, pmode)  fsyscp_popen(pcmd, pmode)
#define pclose(pstream) _pclose(pstream)

#undef FATAL_PERROR
#if defined (KPSE_COMPAT_API)
#define FATAL_PERROR(str) do { \
  win32_fprintf (stderr, "%s: ", kpse_def->invocation_name); \
  win32_perror (str); exit (EXIT_FAILURE); } while (0)
#else
/* If there is no global variable available, just output the error */
#define FATAL_PERROR(str) do { \
  win32_perror (str); exit (EXIT_FAILURE); } while (0)
#endif

#endif
#endif

#ifndef GUNZIP
#define GUNZIP          "gzip -d"
#endif

int to_close;  /* use fclose or pclose? */
char *realnameoffile;

/* Return safely quoted version of NAME.  That means using "..." on
   Windows and '...' on Unix, with any occurrences of the quote char
   quoted with a \.  Result is always in new memory; NAME is unchanged.
   This is a generic function; if we end up needing it in other
   programs, we should move it to kpathsea.  */

#ifdef WIN32
#define QUOTE '"'
#else
#define QUOTE '\''
#endif

static char *
quote_name (const char *name)
{
#if defined(MIKTEX)
  char* quoted = (char*)xmalloc(strlen(name) * 2 + 3);
#else
  char *quoted = xmalloc (strlen (name) * 2 + 3);
#endif
  char *q = quoted;
  const char *p = name;  
  
  *q++ = QUOTE;
  while (*p) {
    if (*p == QUOTE)
      *q++ = QUOTE;
    *q++ = *p++;
  }
  *q++ = QUOTE;
  *q = 0;
  
  return quoted;
}


/* On Windows, return PROG with SELFAUTOLOC prepended, and / separators
   changed to \.  On Unix, return a copy of PROG.  Result is always in
   new memory.  No safeness or other quoting is done on PROG.
   
   This is necessary because Windows likes to run programs out of the
   current directory.  So for security, we want to ensure that we are
   invoking programs from our own binary directory, not via PATH.

   This is a generic function; if we end up needing it in other
   programs, we should move it to kpathsea.  */

#ifdef WIN32
#include <kpathsea/concatn.h>
#endif

static char *
selfautoloc_prog (const char *prog)
{
  char *ret;
#ifdef WIN32
  char *p, *q, *r;
  /* Get value of SELFAUTOLOC, but \ works better for path separator.  */
  char *selfautoloc = kpse_var_value ("SELFAUTOLOC");
  for (p = selfautoloc; *p; p++) {
    if (*p == '/')
      *p = '\\';
  }
  q = xstrdup (prog);
  r = q;
  while (*r && *r != ' ' && *r != '\t')
    r++;
  if (*r == '\0')
    ret = concatn ("\"", selfautoloc, "\\", q, "\"", NULL);
  else {
    *r = '\0';
    r++;
    if (*r)
      ret = concatn ("\"", selfautoloc, "\\", q, "\" ", r, NULL);
    else
      ret = concatn ("\"", selfautoloc, "\\", q, "\"", NULL);
  }
  free (q);
  free (selfautoloc);
  
#else /* not WIN32 */
  ret = xstrdup (prog);
#endif

  return ret;
}



FILE *
search(kpse_file_format_type format, const char *file, const char *mode)
{
#if defined(MIKTEX_WINDOWS)
  // fixes #2336 dvips chokes on graphic files with accents
  // https://sourceforge.net/p/miktex/bugs/2336/
  std::string utf8FileName;
  if (!Utils::IsUTF8(file))
  {
    utf8FileName = MiKTeX::Util::StringUtil::AnsiToUTF8(file);
    file = utf8FileName.c_str();
  }
#endif
  FILE *ret;
  string found_name;

  /* This change suggested by maj@cl.cam.ac.uk to disallow reading of
     arbitrary files.  Further modified by Y. Oiwa. */
#ifndef SECURE
#if defined(MIKTEX)
  if (secure == 2 && !miktex_allow_all_paths) {
#else
  if (secure == 2) {
#endif
#endif
    /* an absolute path is denied */
    if (kpse_absolute_p (file, false))
      return NULL;

    if (file[0] == '.' && file[1] == '.' && IS_DIR_SEP(file[2])) {
      /* a relative path starting with ../ is denied */
      return NULL;
    } else {
      /* a relative path containing /../ is denied */
      const char *p;

      if (file[0] != '\0') {
       for(p = file + 1; *p && ((p = strstr(p, "..")) != NULL); p += 2) {
         /* p[2] is valid, because p points to string starting with ".." */
         /* p[-1] is valid, because p does not point the top of file */
         if ((p[2] == '\0' || IS_DIR_SEP(p[2])) && IS_DIR_SEP(p[-1])) {
           return NULL;
         }
       }
      }
    }
#ifndef SECURE
  }
#endif
  /* Most file looked for through here must exist -- the exception is
     VF's. Bitmap fonts go through pksearch. */
  found_name = kpse_find_file (file, format,
                               format != vfpath && format != ofmpath);
  if (found_name) {
    unsigned len = strlen (found_name);
#if defined(MIKTEX)
    if ((format == figpath || format == headerpath || format == pictpath)
#else
    if ((format == figpath || format == headerpath)
#endif
        && ((len > 2 && FILESTRCASEEQ (found_name + len - 2, ".Z"))
            || (len > 3 && FILESTRCASEEQ (found_name + len - 3, ".gz")))) {
#if defined(MIKTEX)
      std::shared_ptr<MiKTeX::Core::Session> session = MIKTEX_SESSION();
      char* command = concat3("zcat \"", found_name, "\"");
      ret = session->TryOpenFile(MiKTeX::Util::PathName(command), MiKTeX::Core::FileMode::Command, MiKTeX::Core::FileAccess::Read, false);
      free(command);
      to_close = USE_MIKTEX_CLOSE_FILE;
#else
      /* automatically but safely decompress.  */
      char *quoted_name = quote_name (found_name);
      char *cmd;
      char *prog = selfautoloc_prog (GUNZIP);
      cmd = concat3 (prog, " -c ", quoted_name);
      ret = popen (cmd, "r");
      if (dd(D_FILES)) {
        fprintf_str(stderr, "popen(%s) %s\n", cmd,
                         ret == NULL ? "failed" : "succeeded");
      }
      SET_BINARY(fileno(ret));
      to_close = USE_PCLOSE;
      free (cmd);
      free (quoted_name);
      free (prog);
#endif
    } else {
      ret = fopen (found_name, mode);
      to_close = USE_FCLOSE;
    }
    if (!ret)
      FATAL_PERROR (found_name);

    /* Free result of previous search.  */
    if (realnameoffile)
      free (realnameoffile);

    /* Save in `name' and `realnameoffile' because other routines
       access those globals.  */
    realnameoffile = found_name;
    strcpy(name, realnameoffile);
  } else
    ret = NULL;

  return ret;
}               /* end search */


FILE *
pksearch(const char *file, const char *mode, halfword dpi,
         const char **name_ret, int *dpi_ret)
{
  FILE *ret;
  kpse_glyph_file_type font_file;
  string found_name = kpse_find_pk (file, dpi, &font_file);
  
  if (found_name) {
    ret = fopen (found_name, mode);
    if (!ret)
      FATAL_PERROR (name);

    /* Free result of previous search.  */
    if (realnameoffile)
      free (realnameoffile);
    /* Save in `name' and `realnameoffile' because other routines
       access those globals.  Sigh.  */
    realnameoffile = found_name;
    strcpy(name, realnameoffile);
    *name_ret = font_file.name;
    *dpi_ret = font_file.dpi;
  } else
    ret = NULL;

  return ret;
}               /* end search */


/* do we report file openings? */

#ifdef DEBUG
#  ifdef fopen
#    undef fopen
#  endif
#  ifdef VMCMS  /* IBM: VM/CMS */
#    define fopen cmsfopen
#  endif /* IBM: VM/CMS */
FILE *
my_real_fopen(register const char *n, register const char *t)
{
   FILE *tf;
   if (dd(D_FILES)) {
      fprintf_str(stderr, "<%s(%s)> ", n, t);
      tf = fopen(n, t);
      if (tf == 0)
         fprintf(stderr, "failed\n");
      else
         fprintf(stderr, "succeeded\n");
   } else
      tf = fopen(n, t);
#ifdef OS2
   if (tf == (FILE *)NULL)
     tf = fat_fopen(n, t); /* try again with filename truncated to 8.3 */
#endif
   return tf;
}
#endif

#ifdef OS2
/* truncate filename at end of fname to FAT filesystem 8.3 limit */
/* if truncated, return fopen() with new name */
FILE *
fat_fopen(char *fname, char *t)
{
   char *np;	/* pointer to name within path */
   char nbuf[13], *ns, *nd;
   char n[MAXPATHLEN];
   int ni, ne;
   FILE *tf;
   strcpy(n, fname);
   for (ns=n; *ns; ns++) {
      if (*ns=='/')
         *ns=DIRSEP;
   }
   np = strrchr(n,DIRSEP);
   if (np==(char *)NULL)
      np = n;
   else
      np++;
   /* fail if it contains more than one '.' */
   ni = 0;
   for (ns=np; *ns; ns++) {
      if (*ns=='.')
         ni++;
   }
   if (ni>1)
      return (FILE *)NULL;
   /* copy it to nbuf, truncating to 8.3 */
   ns = np;
   nd = nbuf;
   ni = 0;
   while ((*ns!='.') && (*ns) && (ni<8)) {
      *nd++ = *ns++;
      ni++;
   }
   while ((*ns!='.') && (*ns)) {
      ns++;
      ni++;
   }
   ne = 0;
   if (*ns=='.') {
      *nd++ = *ns++;
      while ((*ns!='.') && (*ns) && (ne<3)) {
         *nd++ = *ns++;
         ne++;
      }
      while (*ns) {
         ns++;
         ne++;
      }
   }
   *nd++='\0';
   if ((ni>8) || (ne>3)) {
      strcpy(np,nbuf);
      /* now code copied from my_real_fopen() */
      if (dd(D_FILES)) {
         fprintf_str(stderr, "<%s(%s)> ", n, t);
         tf = fopen(n, t);
         if (tf == 0)
            fprintf(stderr, "failed\n");
         else
            fprintf(stderr, "succeeded\n");
      }
      else
         tf = fopen(n, t);
      return tf;
   }
   return (FILE *)NULL;
}
#endif

int
close_file(FILE *f)
{
   switch(to_close) {
#if defined(MIKTEX)
   case USE_PCLOSE:  return miktex_pclose(f);
#else
case USE_PCLOSE:  return pclose(f);
#endif
case USE_FCLOSE:  return fclose(f);
#if defined(MIKTEX)
   case USE_MIKTEX_CLOSE_FILE:
   {
     std::shared_ptr<MiKTeX::Core::Session> session = MIKTEX_SESSION();
     session->CloseFile(f);
     return 0;
   }
#endif
default:          return -1;
   }
}
