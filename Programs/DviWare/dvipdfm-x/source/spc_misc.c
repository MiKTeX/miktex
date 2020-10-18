/* This is dvipdfmx, an eXtended version of dvipdfm by Mark A. Wicks.

    Copyright (C) 2002-2020 by Jin-Hwan Cho and Shunsaku Hirata,
    the dvipdfmx project team.
    
    Copyright (C) 1998, 1999 by Mark A. Wicks <mwicks@kettering.edu>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "system.h"
#include "mem.h"
#include "error.h"

#include "mfileio.h"

#include "dpxutil.h"

#include "pdfparse.h"
#include "pdfobj.h"

#include "pdfcolor.h"
#include "pdfdraw.h"
#include "pdfximage.h"
#include "pdfdev.h"

#include "mpost.h"

#include "specials.h"

#include "spc_util.h"
#include "spc_misc.h"

/* pdfcolorstack:
 * This special is provoded as a compatibility feature to pdftex.
 */
struct stack
{
  int       page;
  int       direct;
  dpx_stack stack;
};

#define PDFCOLORSTACK_MAX_STACK 256
struct spc_stack
{
  struct stack stacks[PDFCOLORSTACK_MAX_STACK];
};

static struct spc_stack spc_stack;

static int
pdfcolorstack__get_id (struct spc_env *spe, int *id, struct spc_arg *args)
{
  char *q;

  if (args->curptr >= args->endptr) {
    spc_warn(spe, "Stack ID number expected but not found.");
    return -1;
  }
  q = parse_number(&args->curptr, args->endptr);
  if (!q) {
    spc_warn(spe, "Stack ID number expected but not found.");
    return -1;
  }
  *id = atoi(q);
  RELEASE(q);

  skip_white(&args->curptr, args->endptr);

  return 0;
}

static int
pdfcolorstack__init (void *dp)
{
  struct spc_stack *sd = dp;
  int  i;

  for (i = 0; i < PDFCOLORSTACK_MAX_STACK; i++) {
    sd->stacks[i].page    = 0;
    sd->stacks[i].direct  = 0;
    dpx_stack_init(&sd->stacks[i].stack);
  }

  return 0;
}

static int
pdfcolorstack__clean (void *dp)
{
  struct spc_stack *sd = dp;
  int  i;

  for (i = 0; i < PDFCOLORSTACK_MAX_STACK; i++) {
    pdf_obj   *litstr;
    dpx_stack *stk = &sd->stacks[i].stack;

    while ((litstr = dpx_stack_pop(stk)) != NULL) {
      pdf_release_obj(litstr);
    }
  }

  return 0;
}

static void
pdfcolorstack__set_litstr (pdf_coord cp, pdf_obj *litstr, int direct)
{
  pdf_tmatrix M;

  if (!litstr)
    return;

  if (!direct) {
    M.a = M.d = 1.0; M.b = M.c = 0.0;
    M.e = cp.x; M.f = cp.y;
    pdf_dev_concat(&M);
  }
  pdf_doc_add_page_content(" ", 1);
  pdf_doc_add_page_content(pdf_string_value(litstr), pdf_string_length(litstr));
  if (!direct) {
    M.e = -cp.x; M.f = -cp.y;
    pdf_dev_concat(&M);
  }
}

static int
spc_handler_pdfcolorstackinit (struct spc_env *spe, struct spc_arg *args)
{
  int           id = -1;
  struct stack *st;
  char         *q;
  pdf_coord     cp = {0.0, 0.0};
  pdf_obj      *litstr;

  skip_white(&args->curptr, args->endptr);
  if (args->curptr >= args->endptr)
    return -1;

  if (pdfcolorstack__get_id(spe, &id, args) < 0)
    return -1;
  if (id < 0 || id >= PDFCOLORSTACK_MAX_STACK) {
    spc_warn(spe, "Invalid stack number specified: %d", id);
    return -1;
  }
  skip_white(&args->curptr, args->endptr);

  st = &spc_stack.stacks[id];
  if (dpx_stack_depth(&st->stack) > 0) {
    spc_warn(spe, "Stadk ID=%d already initialized?", id);
    return -1;
  }

  while ((q = parse_c_ident(&args->curptr, args->endptr)) != NULL) {
    if (!strcmp(q, "page")) {
      st->page = 1;
    } else if (!strcmp(q, "direct")) {
      st->direct = 1;
    } else {
      spc_warn(spe, "Ignoring unknown option for pdfcolorstack special (init): %s", q);
    }
    RELEASE(q);
    skip_white(&args->curptr, args->endptr);
  }

  if (args->curptr < args->endptr) {
    litstr = parse_pdf_string(&args->curptr, args->endptr);
    if (litstr) {
      dpx_stack_push(&st->stack, litstr);
      pdfcolorstack__set_litstr(cp, litstr, st->direct);
    }
    skip_white(&args->curptr, args->endptr);
  }  else {
    spc_warn(spe, "No valid PDF literal specified.");
    return -1;
  }

  return 0;
}

static int
pdfcolorstack__set (struct spc_env *spe, struct stack *st, pdf_coord cp, struct spc_arg *args)
{
  pdf_obj *litstr;

  skip_white(&args->curptr, args->endptr);
  if (args->curptr >= args->endptr)
    return -1;

  litstr = dpx_stack_pop(&st->stack);
  if (!litstr) {
    spc_warn(spe, "Stack empty!");
    return -1;
  }
  pdf_release_obj(litstr);

  litstr = parse_pdf_string(&args->curptr, args->endptr);
  if (litstr) {
    dpx_stack_push(&st->stack, litstr);
    pdfcolorstack__set_litstr(cp, litstr, st->direct);
    skip_white(&args->curptr, args->endptr);
  }

  return 0;
}

static int
pdfcolorstack__push (struct spc_env *spe, struct stack *st, pdf_coord cp, struct spc_arg *args)
{
  pdf_obj *litstr;

  skip_white(&args->curptr, args->endptr);
  if (args->curptr >= args->endptr)
    return -1;

  litstr = parse_pdf_string(&args->curptr, args->endptr);
  if (litstr) {
    dpx_stack_push(&st->stack, litstr);
    pdfcolorstack__set_litstr(cp, litstr, st->direct);
    skip_white(&args->curptr, args->endptr);
  }

  return 0;
}

static int
pdfcolorstack__current (struct spc_env *spe, struct stack *st, pdf_coord cp, struct spc_arg *args)
{
  pdf_obj *litstr;

  litstr = dpx_stack_top(&st->stack);
  if (litstr) {
    pdfcolorstack__set_litstr(cp, litstr, st->direct);
    skip_white(&args->curptr, args->endptr);
  } else {
    spc_warn(spe, "Stack empty!");
    return -1;
  }

  return 0;
}

static int
pdfcolorstack__pop (struct spc_env *spe, struct stack *st, pdf_coord cp, struct spc_arg *args)
{
  int      error = 0;
  pdf_obj *litstr;

  /* "default" at the bottom */
  if (dpx_stack_depth(&st->stack) < 2) {
    spc_warn(spe, "Stack underflow");
    return -1;
  }
  litstr = dpx_stack_pop(&st->stack);
  if (litstr) {
    pdf_release_obj(litstr);
  }
  litstr = dpx_stack_top(&st->stack);
  if (litstr) {
    pdfcolorstack__set_litstr(cp, litstr, st->direct);
  }

  return error;
}

static int
spc_handler_pdfcolorstack (struct spc_env *spe, struct spc_arg *args)
{
  int           error = 0;
  int           id;
  char          *command;
  struct stack *st;
  pdf_coord     cp;

  skip_white(&args->curptr, args->endptr);
  if (args->curptr >= args->endptr)
    return -1;

  if (pdfcolorstack__get_id(spe, &id, args) < 0)
    return -1;
  if (id < 0 || id >= PDFCOLORSTACK_MAX_STACK) {
    spc_warn(spe, "Invalid stack ID specified: %d", id);
    return -1;
  }
  skip_white(&args->curptr, args->endptr);

  st = &spc_stack.stacks[id];
  if (dpx_stack_depth(&st->stack) < 1) {
    spc_warn(spe, "Stack ID=%d not properly initialized?", id);
    return -1;
  }

  command = parse_c_ident(&args->curptr, args->endptr);
  if (!command)
    return -1;

  spc_get_current_point(spe, &cp);
  if (!strcmp(command, "set")) {
    error = pdfcolorstack__set(spe, st, cp, args);
  } else if (!strcmp(command, "push")) {
    error = pdfcolorstack__push(spe, st, cp, args);
  } else if (!strcmp(command, "pop")) {
    error = pdfcolorstack__pop(spe, st, cp, args);
  } else if (!strcmp(command, "current")) {
    error = pdfcolorstack__current(spe, st, cp, args);
  } else {
    spc_warn(spe, "Unknown action: %s", command);
  }

  if (error) {
    spc_warn(spe, "Error occurred while processing pdfcolorstack: id=%d command=\"%s\"", id, command);
  }

  RELEASE(command);

  return error;
}


int
spc_misc_at_begin_document (void)
{
  struct spc_stack *sd = &spc_stack;
  return  pdfcolorstack__init(sd);
}

int
spc_misc_at_end_document (void)
{
  struct spc_stack *sd = &spc_stack;
  return pdfcolorstack__clean(sd);
}

int
spc_misc_at_begin_page (void)
{
  struct spc_stack *sd = &spc_stack;
  int  i;

  for (i = 0; i < PDFCOLORSTACK_MAX_STACK; i++) {
    dpx_stack *stk = &sd->stacks[i].stack;

    if (sd->stacks[i].page) {
      pdf_obj   *litstr = dpx_stack_top(stk);
      pdf_coord  cp     = {0.0, 0.0};

      if (litstr)
        pdfcolorstack__set_litstr(cp, litstr, sd->stacks[i].direct);
    }
  }

  return 0;
}

int
spc_misc_at_begin_form (void)
{
  return spc_misc_at_begin_page();
}

int
spc_misc_at_end_form (void)
{
  return spc_misc_at_begin_page();
}

static int
spc_handler_postscriptbox (struct spc_env *spe, struct spc_arg *ap)
{
  int            form_id, len;
  transform_info ti;
  load_options   options = {1, 0, NULL};
  char           filename[256], *fullname;
  char           buf[512];
  FILE          *fp;

  ASSERT(spe && ap);

  if (ap->curptr >= ap->endptr) {
    spc_warn(spe, "No width/height/filename given for postscriptbox special.");
    return  -1;
  }

  /* input is not NULL terminated */
  len = (int) (ap->endptr - ap->curptr);
  len = MIN(511, len);
  memcpy(buf, ap->curptr, len);
  buf[len] = '\0';

  transform_info_clear(&ti);

  spc_warn(spe, buf);
  if (sscanf(buf, "{%lfpt}{%lfpt}{%255[^}]}",
      &ti.width, &ti.height, filename) != 3) {
    spc_warn(spe, "Syntax error in postscriptbox special?");
    return  -1;
  }
  ap->curptr = ap->endptr;

  ti.width  *= 72.0 / 72.27;
  ti.height *= 72.0 / 72.27;

  fullname = kpse_find_pict(filename);
  if (!fullname) {
    spc_warn(spe, "Image file \"%s\" not found.", filename);
    return  -1;
  }

  fp = MFOPEN(fullname, FOPEN_R_MODE);
  if (!fp) {
    spc_warn(spe, "Could not open image file: %s", fullname);
    RELEASE(fullname);
    return  -1;
  }
  RELEASE(fullname);

  ti.flags |= (INFO_HAS_WIDTH|INFO_HAS_HEIGHT);

  for (;;) {
    const char *p = mfgets(buf, 512, fp);
    if (!p)
      break;
    if (mps_scan_bbox(&p, p + strlen(p), &ti.bbox) >= 0) {
      ti.flags |= INFO_HAS_USER_BBOX;
      break;
    }
  }
  MFCLOSE(fp);

  form_id = pdf_ximage_load_image(NULL, filename, options);
  if (form_id < 0) {
    spc_warn(spe, "Failed to load image file: %s", filename);
    return  -1;
  }

  spc_put_image(spe, form_id, &ti, spe->x_user, spe->y_user);

  return  0;
}

static int
spc_handler_null (struct spc_env *spe, struct spc_arg *args)
{
  args->curptr = args->endptr;

  return 0;
}

static struct spc_handler misc_handlers[] = {
  {"postscriptbox",     spc_handler_postscriptbox},
  {"pdfcolorstackinit", spc_handler_pdfcolorstackinit},
  {"pdfcolorstack",     spc_handler_pdfcolorstack},
  {"landscape",         spc_handler_null}, /* handled at bop */
  {"papersize",         spc_handler_null}, /* handled at bop */
  {"src:",              spc_handler_null}, /* simply ignore  */
  {"pos:",              spc_handler_null}, /* simply ignore  */
  {"om:",               spc_handler_null}  /* simply ignore  */
};


int
spc_misc_check_special (const char *buffer, int size)
{
  const char *p, *endptr;
  int         i;

  p      = buffer;
  endptr = p + size;

  skip_white(&p, endptr);
  size = (int) (endptr - p);
  for (i = 0; i < sizeof(misc_handlers)/sizeof(struct spc_handler); i++) {
    if (size >= strlen(misc_handlers[i].key) &&
        !strncmp(p, misc_handlers[i].key, strlen(misc_handlers[i].key))) {
      return 1;
    }
  }

  return 0;
}

int
spc_misc_setup_handler (struct spc_handler *handle, struct spc_env *spe, struct spc_arg *args)
{
  const char *key;
  int         i, keylen;

  ASSERT(handle && spe && args);

  skip_white(&args->curptr, args->endptr);

  key = args->curptr;
  while (args->curptr < args->endptr && isalpha((unsigned char)args->curptr[0])) {
    args->curptr++;
  }

  if (args->curptr < args->endptr &&
      args->curptr[0] == ':') {
    args->curptr++;
  }

  keylen = (int) (args->curptr - key);
  if (keylen < 1) {
    return -1;
  }

  for (i = 0; i < sizeof(misc_handlers)/sizeof(struct spc_handler); i++) {
    if (keylen == strlen(misc_handlers[i].key) &&
        !strncmp(key, misc_handlers[i].key, keylen)) {

      skip_white(&args->curptr, args->endptr);

      args->command = misc_handlers[i].key;

      handle->key   = "???:";
      handle->exec  = misc_handlers[i].exec;

      return 0;
    }
  }

  return -1;
}
