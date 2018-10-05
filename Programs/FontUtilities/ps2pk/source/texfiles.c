/*
 * File:    texfiles.c
 * Purpose: basic binary TeX reading functions
 * Version: 1.0 (Nov 1993)
 * Author:  Piet Tutelaers (derived from xdvi/pl16)
 */

#include <stdio.h>
#include "texfiles.h"

#define one(fp)		((unsigned char) getc(fp))

unsigned long num(FILE *fp, int size)
{  unsigned long x = 0;

   while (size--) x = (x << 8) | one(fp);
   return x;
}

integer snum(FILE *fp, int size)
{  integer x;

   x = getc(fp);
   while (--size) x = (x << 8) | one(fp);
   return x;
}
