/* emdir.c  -- Written by Eberhard Mattes, donated to the public domain */

#include "emdir.h"

#ifdef OS2

#undef HPS
#define INCL_DOSFILEMGR
#include <os2.h>
#include <string.h>

#define FHDIR(b) (*(HDIR *)(b)->reserved)

static void fconv (struct ll_findbuffer *dst, const FILEFINDBUF *src)
{
  dst->attr = src->attrFile;
  dst->time = *(unsigned *)&src->ftimeLastWrite;
  dst->date = *(unsigned *)&src->fdateLastWrite;
  dst->size = src->cbFile;
  strcpy (dst->name, src->achName);
}


int ll_findfirst (const char *path, int attr, struct ll_findbuffer *buffer)
{
  USHORT rc;
  ULONG count;
  HDIR hdir;
  FILEFINDBUF ffbuf;

  hdir = HDIR_CREATE;
  count = 1;
  rc = DosFindFirst ((PSZ)path, &hdir, attr, &ffbuf, sizeof (ffbuf),
		     &count, 0L);
  if (rc != 0 || count != 1)
    return 0;
  FHDIR (buffer) = hdir;
  fconv (buffer, &ffbuf);
  return 1;
}


int ll_findnext (struct ll_findbuffer *buffer)
{
  USHORT rc;
  ULONG count;
  HDIR hdir;
  FILEFINDBUF ffbuf;

  hdir = FHDIR (buffer);
  count = 1;
  rc = DosFindNext (hdir, &ffbuf, sizeof (ffbuf), &count);
  if (rc != 0 || count != 1)
    {
      DosFindClose (hdir);
      return 0;
    }
  fconv (buffer, &ffbuf);
  return 1;
}


#elif defined(DJGPP)

/* djgpp support by Hartmut Schirmer (hsc@techfak.uni-kiel.de), May 30, 1997 */

#include <dos.h>
#include <dir.h>

static int ll_attr = 0;
int ll_findnext (struct ll_findbuffer *buffer)
{
  int res;
  do {
    res = _dos_findnext ((struct find_t *)buffer);
    if (res != 0) return 0;
  } while ( (buffer->attrib&ll_attr) == 0);
  return 1;
}

int ll_findfirst (const char *path, int attr, struct ll_findbuffer *buffer)
{
  int res;
  ll_attr = attr;
  res = _dos_findfirst((char *)path, attr, (struct find_t *)buffer);
  if (res != 0) return 0;
  if ( (buffer->attrib&ll_attr) == 0)
    return ll_findnext(buffer);
  return 1;
}

#elif !defined(__EMX__)   /* if not OS2 nor DJGPP nor __EMX__ defined */

/* Not tested */

#include <dos.h>

int ll_findfirst (const char *path, int attr, struct ll_findbuffer *buffer)
{
  return _dos_findfirst (path, attr, (struct find_t *)buffer) == 0;
}


int ll_findnext (struct ll_findbuffer *buffer)
{
  return _dos_findnext ((struct find_t *)buffer) == 0;
}

#endif
