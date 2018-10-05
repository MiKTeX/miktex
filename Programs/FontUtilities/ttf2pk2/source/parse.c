/*
 *   parse.c
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000 by
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

#include "ttf2tfm.h"
#include "errormsg.h"
#include "newobj.h"
#include "ligkern.h"
#include "texenc.h"
#include "parse.h"
#include "filesrch.h"


/*
 *   Here we get a token from the encoding file.  We parse just as much
 *   PostScript as we expect to find in an encoding file.  We allow
 *   commented lines and names like 0, .notdef, _foo_.  We do not allow
 *   //abc.
 *
 *   `bufferp' is a pointer to the current line; the offset of the beginning
 *   of the token to be parsed relative to `bufferp' will be returned in
 *   `offsetp'.  On the first call of gettoken() `init' must be set to 1 and
 *   to 0 on the following calls.
 *
 *   If `ignoreligkern' is `True', no LIGKERN data will be extracted from the
 *   encoding file.
 *
 *   Don't modify `bufferp'!
 *
 *   The memory management of `bufferp' will be done by gettoken() itself;
 *   nevertheless, it returns a pointer to the current token which should be
 *   freed after it has been used.
 */

static char *
gettoken(char **bufferp, size_t *offsetp, FILE *f, Font *fnt,
         Boolean ignoreligkern, Boolean init)
{
  char *p, *q;
  char tempchar;
  static char *curp;


  if (init)
    curp = NULL;

  while (1)
  {
    while (curp == NULL || *curp == '\0')
    {
      if (*bufferp)
        free(*bufferp);

      if (texlive_getline(bufferp, f) == False)
        oops("Premature end in encoding file.");

      curp = *bufferp;

      for (p = *bufferp; *p; p++)
        if (*p == '%')
        {
          if (ignoreligkern == False)
            checkligkern(p, fnt);
          *p = '\0';
          break;
        }
    }

    while (isspace((unsigned char)*curp))
      curp++;

    *offsetp = curp - *bufferp;

    if (*curp)
    {
      if (*curp == '[' || *curp == ']' ||
          *curp == '{' || *curp == '}')
        q = curp++;
      else if (*curp == '/' ||
               *curp == '-' || *curp == '_' || *curp == '.' ||
               ('0' <= *curp && *curp <= '9') ||
               ('a' <= *curp && *curp <= 'z') ||
               ('A' <= *curp && *curp <= 'Z'))
      {
        q = curp++;
        while (*curp == '-' || *curp == '_' || *curp == '.' ||
               ('0' <= *curp && *curp <= '9') ||
               ('a' <= *curp && *curp <= 'z') ||
               ('A' <= *curp && *curp <= 'Z'))
          curp++;
      }
      else
        q = curp;

      tempchar = *curp;
      *curp = '\0';
      p = newstring(q);
      *curp = tempchar;
      return p;
    }
  }
}


/*
 *   This routine reads in an encoding file, given the name.  It returns
 *   the final total structure.  It performs a number of consistency checks.
 */

encoding *
readencoding(char **enc, Font *fnt, Boolean ignoreligkern)
{
  char *real_encname;
  FILE *enc_file;
  char *p, *q, c;
  char *buffer;
  char numbuf[11];
  size_t offset;
  int i;
  long l;
  encoding *e = (encoding *)mymalloc(sizeof (encoding));


  if (enc && *enc)
  {
    real_encname = TeX_search_encoding_file(enc);
    if (!real_encname)
      oops("Cannot find encoding file `%s'.", *enc);

    enc_file = fopen(real_encname, "rt");
    if (enc_file == NULL)
      oops("Cannot open encoding file `%s'.", real_encname);

    buffer = NULL;
    p = gettoken(&buffer, &offset, enc_file, fnt, ignoreligkern, True);
    if (*p != '/' || p[1] == '\0')
      boops(buffer, offset,
        "First token in encoding must be literal encoding name.");
    e->name = newstring(p + 1);
    free(p);

    p = gettoken(&buffer, &offset, enc_file, fnt, ignoreligkern, False);
    if (strcmp(p, "["))
      boops(buffer, offset,
        "Second token in encoding must be mark ([) token.");
    free(p);

    for (i = 0; i < 256; i++)
    {
      p = gettoken(&buffer, &offset, enc_file, fnt, ignoreligkern, False);
      if (*p != '/' || p[1] == 0)
        boops(buffer, offset,
          "Tokens 3 to 257 in encoding must be literal names.");

      /* now we test for a generic code point resp. glyph index value */

      c = p[2];
      if (p[1] == '.' && (c == 'c' || c == 'g') && '0' <= p[3] && p[3] <= '9')
      {
        l = strtol(p + 3, &q, 0);
        if (*q != '\0' || l < 0 || l > 0x16FFFF)
          boops(buffer, offset, "Invalid encoding token.");
        sprintf(numbuf, ".%c0x%lx", c, l);
        e->vec[i] = newstring(numbuf);
      }
      else
        e->vec[i] = newstring(p + 1);

      free(p);
    }

    p = gettoken(&buffer, &offset, enc_file, fnt, ignoreligkern, False);
    if (strcmp(p, "]"))
      boops(buffer, offset,
        "Token 258 in encoding must be make-array (]).");
    free(p);

    while (texlive_getline(&buffer, enc_file))
    {
      for (p = buffer; *p; p++)
        if (*p == '%')
        {
          if (ignoreligkern == False)
            checkligkern(p, fnt);
          *p = '\0';
          break;
        }
    }

    fclose(enc_file);

    if (ignoreligkern == False && fnt->sawligkern == False)
      getligkerndefaults(fnt);
  }
  else
  {
    if (ignoreligkern == False)
    {
      e = &staticencoding;
      getligkerndefaults(fnt);
    }
    else
      e = NULL;
  }

  return e;
}


/*
 *   We scan a glyph replacement file.
 *   `%' is the comment character.
 */

void
get_replacements(Font *fnt)
{
  char *real_replacement_name;
  FILE *replacement_file;
  char *buffer = NULL, *oldbuffer = NULL;
  char *p;
  char *old_name, *new_name;
  stringlist *sl;


  if (!fnt->replacementname)
    return;

  real_replacement_name = TeX_search_replacement_file(&fnt->replacementname);
  if (!real_replacement_name)
    oops("Cannot find replacement file `%s'.", fnt->replacementname);

  replacement_file = fopen(real_replacement_name, "rt");
  if (replacement_file == NULL)
    oops("Cannot open replacement file `%s'.", real_replacement_name);

  while (texlive_getline(&buffer, replacement_file))
  {
    for (p = buffer; *p; p++)
      if (*p == '%')
      {
        *p = '\0';
        break;
      }

    if (oldbuffer)
      free(oldbuffer);
    oldbuffer = newstring(buffer);

    p = buffer;

    while (isspace((unsigned char)*p))
      p++;
    if (!*p)
      continue;

    old_name = p;

    while (*p && !isspace((unsigned char)*p))
      p++;
    if (*p)
      *p++ = '\0';      

    while (*p && isspace((unsigned char)*p))
      p++;
    if (!*p)
      boops(oldbuffer, old_name - oldbuffer, "Replacement glyph missing.");

    new_name = p;

    while (*p && !isspace((unsigned char)*p))
      p++;
    if (*p)
      *p++ = '\0';      

    while (*p && isspace((unsigned char)*p))
      p++;
    if (*p)
      boops(oldbuffer, p - oldbuffer, "Invalid replacement syntax.");

    sl = newstringlist();
    sl->new_name = newstring(new_name);
    sl->old_name = newstring(old_name);
    sl->next = fnt->replacements;
    fnt->replacements = sl;
  }

  fclose(replacement_file);
}


/* end */
