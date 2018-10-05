/*
 *   case.h
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */

#ifndef CASE_H
#define CASE_H

struct _Case
{
  const char *upper;
  const char *lower;
};
typedef struct _Case Case;


extern Case casetable[];

#endif /* CASE_H */


/* end */
