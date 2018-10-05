/*
 * File:    texfiles.h
 * Purpose: headerfile for texfiles.c
 * Version: 1.0 (Nov 1993)
 * Author:  Piet Tutelaers
 */

#ifndef __TEXFILES__
#define __TEXFILES__

#include <stdio.h>

#ifdef MSDOS
typedef long integer ;
#else
typedef int integer ;
#endif

unsigned long num(FILE *fp, int size);
integer snum(FILE *fp, int size);

#define one(fp)		((unsigned char) getc(fp))
#define sone(fp)	((long) one(fp))
#define two(fp)		num (fp, 2)
#define stwo(fp)	snum(fp, 2)
#define four(fp)	num (fp, 4)
#define sfour(fp)	snum(fp, 4)

#endif /* __TEXFILES__ */
