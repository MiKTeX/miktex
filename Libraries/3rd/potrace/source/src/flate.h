/* Copyright (C) 2001-2015 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */


#ifndef FLATE_H
#define FLATE_H

int dummy_xship(FILE *f, int filter, char *s, int len);
int flate_xship(FILE *f, int filter, char *s, int len);
int pdf_xship(FILE *f, int filter, char *s, int len);
int lzw_xship(FILE *f, int filter, char *s, int len);
int a85_xship(FILE *f, int filter, char *s, int len);

#endif /* FLATE_H */
