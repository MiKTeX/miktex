/*
 *   ftlib.h
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000, 2002, 2003 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 *
 *   Copyright (C) 2012 by
 *     Peter Breitenlohner <tex-live@tug.org>
 */

#ifndef FTLIB_H
#define FTLIB_H

extern FT_Library  engine;
extern FT_Face     face;

extern FT_Matrix   matrix1, matrix2;

extern int dpi, ptsize;

extern Boolean has_gsub;


void FTopen(char *filename, Font *fnt, Boolean do_tfm, Boolean quiet);

int Get_Vert(int Num);

#endif /* FTLIB_H */
