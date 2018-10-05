/*
 *   subfont.h
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000, 2002 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */

#ifndef SUBFONT_H
#define SUBFONT_H

#include "ttf2tfm.h"


Boolean init_sfd(Font *fnt, Boolean fatal);
Boolean get_sfd(Font *fnt, Boolean is_sfd);
void close_sfd(void);

void handle_sfd(char *s, int *sfd_begin, int *postfix_begin);

#endif /* SUBFONT_H */


/* end */
