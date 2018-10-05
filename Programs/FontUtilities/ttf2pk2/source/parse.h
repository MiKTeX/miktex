/*
 *   parse.h
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */

#ifndef PARSE_H
#define PARSE_H

#include "ttf2tfm.h"

encoding *readencoding(char **enc, Font *fnt, Boolean ignoreligkern);
void get_replacements(Font *fnt);

#endif /* PARSE_H */


/* end */
