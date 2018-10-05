/*
 *   pklib.h
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */

#ifndef PKLIB_H
#define PKLIB_H

typedef unsigned char byte;


void TFMopen(char **filename);

void PKopen(char *filename, char *ident, int resolution);

void PKputglyph(int cc, int llx, int lly, int urx, int ury, 
                int w, int h, byte *b);

void PKclose(void);

#endif /* PKLIB_H */


/* end */
