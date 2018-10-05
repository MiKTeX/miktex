/*
 *   subfont.c
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000, 2002 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>         /* for size_t */
#include <ctype.h>
#include <string.h>

#include "filesrch.h"
#include "subfont.h"
#include "newobj.h"
#include "errormsg.h"


struct _realsubfont;
typedef struct _realsubfont realsubfont;

struct _realsubfont
{
  realsubfont *next;
  char *name;
  FILE *file;
};

static realsubfont *real_sfd_name = NULL, *real_lig_name = NULL;

static realsubfont *
really_init_sfd(char *name, Boolean fatal);
static Boolean
really_get_sfd(Font *fnt, Boolean next, realsubfont *rsf, Boolean new_sfd);
static void
really_close_sfd(realsubfont *real_name);


/*
 *   Initialize subfont functionality.  The argument is the subfont
 *   definition file name.  If `fatal' is `True', the routine exits
 *   with an error.  If `fatal' is `False', a warning message is emitted
 *   and `False' returned if an error occurs; in case of success `True'
 *   will be returned.
 */

Boolean
init_sfd(Font *fnt, Boolean fatal)
{
  real_sfd_name = really_init_sfd(fnt->sfdname, fatal);
  if (fnt->ligname)
    real_lig_name = really_init_sfd(fnt->ligname, fatal);

  return (real_sfd_name ? True : False);
}


static realsubfont *
really_init_sfd(char *name, Boolean fatal)
{
  char *sf;
  char *buf, *p, *q;
  realsubfont *rsf, *temp, *real_name = NULL;


  buf = newstring(name);
  p = buf;

  while (1)
  {
    if (*p == '\0')
    {
      if (real_name)
        break;
      else
      {
        if (fatal)
          oops("No subfont definition file.");
        else
        {
          warning("No subfont definition file.");
          return NULL;
        }
      }
    }

    q = p;
    while (*q && *q != ',')          /* search end of subfont name */
      q++;
    if (*q)
      *(q++) = '\0';

    sf = newstring(p);

    p = q;

    rsf = (realsubfont *)mymalloc(sizeof (realsubfont));
    rsf->name = TeX_search_sfd_file(&sf);
    if (!rsf->name)
    {
      if (fatal)
        oops("Cannot find subfont definition file `%s'.", sf);
      else
      {
        warning("Cannot find subfont definition file `%s'.", sf);
        return NULL;
      }
    }

    rsf->file = fopen(rsf->name, "rt");
    if (rsf->file == NULL)
    {
      if (fatal)
        oops("Cannot open subfont definition file `%s'.", rsf->name);
      else
      {
        warning("Cannot open subfont definition file `%s'.", rsf->name);
        return NULL;
      }
    }

    rsf->next = real_name;
    real_name = rsf;

    free(sf);
  }

  free(buf);

  /* reverse list */

  rsf = NULL;
  while (real_name)
  {
    temp = real_name;
    real_name = real_name->next;
    temp->next = rsf;
    rsf = temp;
  }
  real_name = rsf;

  return real_name;
}


/*
 *   This function fills the font structure sequentially with subfont
 *   entries; it returns `False' if no more subfont entries are available,
 *   `True' otherwise.
 *
 *   If `next' is `True', check `rsf' whether it contains entries for the
 *   current subfont name, overwriting subfont code points.
 *
 *   fnt->subfont_name must be set to NULL before the first call.
 *
 *   The subset parser was inspired by ttf2bdf.c .
 */

Boolean get_sfd(Font *fnt, Boolean is_sfd)
{
  if (is_sfd)
    return really_get_sfd(fnt, False, real_sfd_name, True);
  else
    return really_get_sfd(fnt, False, real_lig_name, False);
}


static Boolean
really_get_sfd(Font *fnt, Boolean next, realsubfont *rsf, Boolean new_sfd)
{
  long i, offset;
  long begin, end = -1;
  char *buffer, *oldbuffer, *bufp, *bufp2, *bufp3;


  if (next)
    rewind(rsf->file);
  else
    for (i = 0; i < 256; i++)
      fnt->sf_code[i] = -1;

again:

  buffer = get_line(rsf->file);
  if (!buffer)
    oops("Error reading subfont definition file `%s'.", rsf->name);
  if (!*buffer)
    return False;

  oldbuffer = newstring(buffer);
  bufp = buffer;
  offset = 0;

  while (*bufp)             /* remove comment */
  {
    if (*bufp == '#')
    {
      bufp++;
      break;
    }
    bufp++;
  }
  *(--bufp) = '\0';         /* remove final newline character */

  bufp = buffer;

  while (isspace((unsigned char)*bufp))
    bufp++;

  if (*bufp == '\0')                    /* empty line? */
  {
    free(buffer);
    free(oldbuffer);
    goto again;
  }

  while (*bufp && !isspace((unsigned char)*bufp))      /* subfont name */
    bufp++;
  if (*bufp)
    *(bufp++) = '\0';

  while (isspace((unsigned char)*bufp))
    bufp++;

  if (*bufp == '\0')
    oops("Invalid subfont entry in `%s'.", rsf->name);

  if (next || !new_sfd)
  {
    if (strcmp(fnt->subfont_name, buffer))
      goto again;
  }
  else
  {
    if (fnt->subfont_name)
      free(fnt->subfont_name);
    fnt->subfont_name = newstring(buffer);
  }

  while (1)
  {
    bufp3 = bufp;

    begin = strtol(bufp, &bufp2, 0);

    if (bufp == bufp2 || begin < 0 || begin > 0x16FFFF)
      boops(oldbuffer, bufp - buffer,
            "Invalid subfont range or offset entry.");

    if (*bufp2 == ':')                  /* offset */
    {
      offset = begin;
      if (offset > 0xFF)
        boops(oldbuffer, bufp - buffer, "Invalid subfont offset.");

      bufp = bufp2 + 1;

      while (isspace((unsigned char)*bufp))
        bufp++;

      continue;
    }
    else if (*bufp2 == '_')             /* range */
    {
      bufp = bufp2 + 1;
      if (!isdigit((unsigned char)*bufp))
        boops(oldbuffer, bufp - buffer, "Invalid subfont range entry.");

      end = strtol(bufp, &bufp2, 0);

      if (bufp == bufp2 || end < 0 || end > 0x16FFFFL)
        boops(oldbuffer, bufp - buffer, "Invalid subfont range entry.");
      if (*bufp2 && !isspace((unsigned char)*bufp2))
        boops(oldbuffer, bufp2 - buffer, "Invalid subfont range entry.");
      if (end < begin)
        boops(oldbuffer, bufp - buffer, "End of subfont range too small.");
      if (offset + (end - begin) > 255)
        boops(oldbuffer, bufp3 - buffer,
              "Subfont range too large for current offset (%i).", offset);
    }
    else if (isspace((unsigned char)*bufp2) || !*bufp2)        /* single value */
      end = begin;
    else
      boops(oldbuffer, bufp2 - buffer, "Invalid subfont range entry.");

    for (i = begin; i <= end; i++)
    {
      if (!next && fnt->sf_code[offset] != -1)
        boops(oldbuffer, bufp3 - buffer, "Overlapping subfont ranges.");

      fnt->sf_code[offset++] = i;
    }

    bufp = bufp2;

    while (isspace((unsigned char)*bufp))
      bufp++;

    if (!*bufp)
      break;
  }

  free(buffer);
  free(oldbuffer);

  if (!next)
    while ((rsf = rsf->next))
      (void)really_get_sfd(fnt, True, rsf, new_sfd);

  return True;
}


void
close_sfd(void)
{
  really_close_sfd(real_sfd_name);
  really_close_sfd(real_lig_name);
}


static void
really_close_sfd(realsubfont *real_name)
{
  realsubfont *rsf;


  while (real_name)
  {
    if (real_name->file)
      fclose(real_name->file);
    if (real_name->name)
      free(real_name->name);

    rsf = real_name;
    real_name = real_name->next;
    free(rsf);
  }
}


/*
 *   We extract the subfont definition file name.  The name must
 *   be embedded between two `@' characters.  If there is no sfd file,
 *   `sfd_begin' is set to -1.
 *
 *   The `@' characters will be replaced with null characters.
 */

void
handle_sfd(char *s, int *sfd_begin, int *postfix_begin)
{
  int i;
  Boolean have_atsign;


  have_atsign = False;
  *sfd_begin = -1;
  *postfix_begin = -1;

  for (i = 0; s[i]; i++)
  {
    if (s[i] == '@')
    {
      if (have_atsign)
      {
        *postfix_begin = i + 1;

        s[i] = '\0';
        break;
      }
      have_atsign = True;
      *sfd_begin = i + 1;

      s[i] = '\0';
    }
  }

  if (*sfd_begin != -1 &&
      (*postfix_begin == -1 || *postfix_begin < *sfd_begin + 2))
    oops("Invalid subfont definition file name.");

  if (*postfix_begin > -1)
    for (i = *postfix_begin; s[i]; i++)
      if (s[i] == '/' || s[i] == ':' || s[i] == '\\' || s[i] == '@')
        oops("`/', `:', `\\', and `@' not allowed after second `@'.");
}


/* end */
