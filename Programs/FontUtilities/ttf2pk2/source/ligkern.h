/*
 *   ligkern.h
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */

#ifndef LIGKERN_H
#define LIGKERN_H

#include "ttf2tfm.h"


void checkligkern(char *s, Font *fnt);
void getligkerndefaults(Font *fnt);

#endif /* LIGKERN_H */


/* end */
