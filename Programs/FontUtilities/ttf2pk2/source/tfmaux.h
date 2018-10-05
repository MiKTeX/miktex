/*
 *   tfmaux.h
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000, 2002 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */

#ifndef TFMAUX_H
#define TFMAUX_H

#include "ttf2tfm.h"


#define FIXFACTOR (0x100000L)   /* 2^{20}, the unit fixnum */


int transform(int x, int y, float ef, float sl);

int buildtfm(Font *fnt);
void writetfm(Font *fnt);
void writeenc(Font *fnt);

#endif /* TFMAUX_H */


/* end */
