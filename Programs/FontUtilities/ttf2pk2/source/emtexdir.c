/* emtexdir.c -- written by Eberhard Mattes, donated to the public domain */

#if defined (__EMX__)
#include <emx/syscalls.h>
#else
#include "emdir.h"
#endif
#if defined(DJGPP) || defined(GO32)
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include "emtexdir.h"

#define FALSE 0
#define TRUE  1

void (*emtex_dir_find_callback)(const char *name, int ok) = NULL;

static int setup_add (struct emtex_dir *dst, const char *path)
{
  char *p;

  if (dst->used >= dst->alloc)
    {
      dst->alloc += 8;
      dst->list = realloc (dst->list, dst->alloc * sizeof (*dst->list));
      if (dst->list == NULL)
	return (FALSE);
    }
  p = strdup (path);
  if (p == NULL)
    return (FALSE);
  dst->list[dst->used++] = p;
  return (TRUE);
}


static int setup_subdir (struct emtex_dir *dst, char *path, size_t add,
			 unsigned flags, int recurse)
{
  int ok, i, end, len;
#if defined (__EMX__)
  struct _find find;
#else
  struct ll_findbuffer find;
#endif

  i = dst->used;
  strcpy (path + add, "*.*");
#if defined (__EMX__)
  ok = __findfirst (path, 0x10, &find) == 0;
#else
  ok = ll_findfirst (path, 0x10, &find);
#endif
  while (ok)
    {
      if ((find.attr & 0x10)
	  && !(strcmp (find.name, ".") == 0 || strcmp (find.name, "..") == 0))
	{
	  len = strlen (find.name);
	  memcpy (path + add, find.name, len);
	  path[add+len] = '\\';
	  path[add+len+1] = 0;
	  if (!setup_add (dst, path))
	    return (FALSE);
	}
#if defined (__EMX__)
      ok = __findnext (&find) == 0;
#else
      ok = ll_findnext (&find);
#endif
    }
  if (recurse)
    {
      end = dst->used;
      while (i < end)
	{
	  strcpy (path, dst->list[i]);
	  if (!setup_subdir (dst, path, strlen (path), flags, TRUE))
	    return (FALSE);
	  ++i;
	}
    }
  return (TRUE);
}


static int setup_dir (struct emtex_dir *dst, char *path,
		      const char *base_dir, size_t base_dir_len,
		      const char *sub_dir,
		      unsigned flags)
{
  size_t i, len;

  memcpy (path, base_dir, base_dir_len);
  i = base_dir_len;
  if ((flags & EDS_BANG) && sub_dir == NULL)
    {
      flags &= ~(EDS_ONESUBDIR|EDS_ALLSUBDIR);
      if (i >= 2 && path[i-1] == '!' && path[i-2] == '!')
	{
	  flags |= EDS_ALLSUBDIR;
	  i -= 2;
	}
      else if (i >= 1 && path[i-1] == '!')
	{
	  flags |= EDS_ONESUBDIR;
	  --i;
	}
    }
  if (sub_dir != NULL && *sub_dir != 0)
    {
      if (i != 0 && path[i-1] != ':' && path[i-1] != '/' && path[i-1] != '\\')
	path[i++] = '\\';
      len = strlen (sub_dir);
      memcpy (path+i, sub_dir, len);
      i += len;
    }
  if (path[i-1] != ':' && path[i-1] != '/' && path[i-1] != '\\')
    path[i++] = '\\';
  path[i] = 0;
  if (!setup_add (dst, path))
    return (FALSE);
  if (flags & EDS_ALLSUBDIR)
    return (setup_subdir (dst, path, i, flags, TRUE));
  else if (flags & EDS_ONESUBDIR)
    return (setup_subdir (dst, path, i, flags, FALSE));
  else
    return (TRUE);
}


int setup_list (struct emtex_dir *dst, char *path,
		       const char *list, unsigned flags)
{
  const char *end;
  size_t i;

  for (;;)
    {
      while (*list == ' ' || *list == '\t')
	++list;
      if (*list == 0)
	return (TRUE);
      end = list;
      while (*end != 0 && *end != ';')
	++end;
      i = end - list;
      while (i > 0 && (list[i-1] == ' ' || list[i-1] == '\t'))
	--i;
      if (i != 0 && !setup_dir (dst, path, list, i, NULL, flags))
	return (FALSE);
      if (*end == 0)
	return (TRUE);
      list = end + 1;
    }
}


int emtex_dir_setup (struct emtex_dir *ed, const char *env, const char *dir,
		     unsigned flags)
{
  const char *val;
  char path[260];

  ed->alloc = 0;
  ed->used = 0;
  ed->list = NULL;
  if (env != NULL && (val = getenv (env)) != NULL)
    return (setup_list (ed, path, val, flags));
  else if ((val = getenv ("EMTEXDIR")) != NULL)
    return (setup_dir (ed, path, val, strlen (val), dir, flags));
  else
    return (setup_dir (ed, path, "\\emtex", 6, dir, flags));
}


static void pretty (char *path, unsigned flags)
{
  char *p;

  if (flags & EDF_FSLASH)
    for (p = path; *p != 0; ++p)
      if (*p == '\\')
	*p = '/';
}


#define ADDCHAR(C) \
    if (dst_size < 1) return (EDT_TOOLONG); \
    *dst++ = (C); --dst_size

int emtex_dir_trunc (char *dst, size_t dst_size, const char *src,
		     unsigned flags, int method)
{
  int len, truncated, dot;

  if (src[0] != 0 && src[1] == ':')
    {
      ADDCHAR (src[0]);
      ADDCHAR (src[1]);
      src += 2;
    }

  truncated = FALSE; dot = FALSE; len = 0;
  for (;;)
    {
      switch (*src)
	{
	case 0:
	  ADDCHAR (0);
	  return (truncated ? EDT_CHANGED : EDT_UNCHANGED);

	case ':':
	  return (EDT_INVALID);

	case '/':
	case '\\':
	  ADDCHAR (*src);
	  len = 0; dot = FALSE;
	  break;

	case '.':
	  if (dot)
	    return (EDT_INVALID);
	  ADDCHAR (*src);

	  /* ".." is allowed -- don't return EDT_INVALID for the next
	     dot. */

	  if (!(len == 0 && src[1] == '.'
		&& (src[2] == 0 || src[2] == '/' || src[2] == '\\')))
	    {
	      len = 0; dot = TRUE;
	    }
	  break;

	default:
	  if (dot && len == 3)
	    truncated = TRUE;
	  else if (!dot && len == 8)
	    {
	      truncated = TRUE;
	      if (method == 0)
		{
		  dst[-3] = dst[-2];
		  dst[-2] = dst[-1];
		  dst[-1] = *src;
		}
	    }
	  else
	    {
	      ADDCHAR (*src);
	      ++len;
	    }
	  break;
	}
      ++src;
    }
}


static int find2 (const char *name, unsigned flags)
{
  int ok;

  ok = (access (name, 4) == 0);
  if (flags & EDF_TRACE)
    emtex_dir_find_callback (name, ok);
  return (ok);
}


static int find1 (char *path, size_t path_size, const char *dir,
		  const char *fname, unsigned flags)
{
  char buf[260];
  int method, rc;
  size_t len, tmp;

  len = 0;
  if (dir != NULL)
    {
      tmp = strlen (dir);
      if (tmp >= sizeof (buf))
	return (FALSE);
      memcpy (buf, dir, tmp);
      len = tmp;
    }
  tmp = strlen (fname);
  if (len + tmp >= sizeof (buf))
    return (FALSE);
  memcpy (buf + len, fname, tmp + 1);
  len += tmp;
#if 0	/* wkim */
/* disabled for Win95's long file name support  */
/* -- Wonkoo Kim (wkim+@pitt.edu), May 18, 1997 */
  if (_osmode == DOS_MODE)
    {
      rc = emtex_dir_trunc (path, path_size, buf, flags, EDT_5_PLUS_3);
      if ((rc == EDT_UNCHANGED || rc == EDT_CHANGED) && find2 (path, flags))
	{
	  pretty (path, flags);
	  return (TRUE);
	}
      rc = emtex_dir_trunc (path, path_size, buf, flags, EDT_8);
      if (rc == EDT_CHANGED && find2 (path, flags))
	{
	  pretty (path, flags);
	  return (TRUE);
	}
      return (FALSE);
    }
  else
#endif	/* wkim */
    {
      if (len < path_size && find2 (buf, flags))
	{
	  memcpy (path, buf, len + 1);
	  pretty (path, flags);
	  return (TRUE);
	}
      for (method = 0; method < 2; ++method)
	{
	  rc = emtex_dir_trunc (path, path_size, buf, flags, method);
	  if (rc == EDT_CHANGED && find2 (path, flags))
	    {
	      pretty (path, flags);
	      return (TRUE);
	    }
	}
      return (FALSE);
    }
}


int emtex_dir_find (char *path, size_t path_size,
		    const struct emtex_dir *ed,
		    const char *fname, unsigned flags)
{
  int i, absp;
  const char *p;

  absp = FALSE;
  for (p = fname; *p != 0; ++p)
    if (*p == ':' || *p == '/' || *p == '\\')
      {
	absp = TRUE;
	break;
      }

  if (absp)
    return (find1 (path, path_size, NULL, fname, flags));

  if ((flags & EDF_CWD) && find1 (path, path_size, NULL, fname, flags))
    return (TRUE);

  for (i = 0; i < ed->used; ++i)
    if (find1 (path, path_size, ed->list[i], fname, flags))
      return (TRUE);
  return (FALSE);
}


#if defined (TEST)

#include <stdio.h>

int main (int argc, char *argv[])
{
  struct emtex_dir ed;
  int i;
  unsigned flags1, flags2;
  char path[260];

  if (argc != 6)
    {
      puts ("Usage: emtexdir <flags> <flags> <env> <dir> <fname>");
      return (1);
    }

  flags1 = (unsigned)strtol (argv[1], NULL, 0);
  flags2 = (unsigned)strtol (argv[2], NULL, 0);

  if (!emtex_dir_setup (&ed, argv[3], argv[4], flags1))
    {
      fputs ("emtex_dir_setup failed\n", stderr);
      return (2);
    }

  printf ("Directories:\n");
  for (i = 0; i < ed.used; ++i)
    printf ("  %s\n", ed.list[i]);

  if (!emtex_dir_find (path, sizeof (path), &ed, argv[5], flags2))
    puts ("File not found");
  else
    printf ("Path: %s\n", path);
  return (0);
}

#endif
