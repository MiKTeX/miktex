/*
 *	.\" e
 *	.TH MAG 1 UMFT
 *	.SH NAME
 *	mag \- computes fontsizes and magsteps
 *	
 *	.de Ex
 *	.sp
 *	.RS
 *	.nf
 *	.ft C
 *	..
 *	.de Xe
 *	.RE
 *	.sp
 *	.fi
 *	..
 *	.SH SYNOPSIS 
 *	.B
 *	mag
 *	[-\fBR\fIdpi\fP\fP] magstep . . .
 *	
 *	.B
 *	mag
 *	[-\fBr\fP] [-\fBR\fP\fIdpi\fP] fontsize . . .
 *	
 *	.SH DESCRIPTION
 *	.EQ
 *	delim $$
 *	.EN
 *	This tool calculates fontsizes given magsteps.  TeXfonts are provided as
 *	true sized fonts or as magnifications.  The fontsize of a true sized
 *	font equals the resolution of the printer (ex.  300).  The fontsize
 *	of a font magnified $n$ \fImagsteps\fP equals:
 *	.EQ
 *	1.2 sup{n} times 300
 *	delim off
 *	.EN
 *	rounded to its nearest integer value.  Fontnames for TeX fonts normally
 *	consists of the name (\fIcmr\fP), pointsize (\fI10\fP), type (\fIpk\fP)
 *	and fontsize (\fI360\fP), for example: \fIcmr10.360pk\fP. 
 *	
 *	.SH EXAMPLES
 *	The result of \fImag -R240 -2 0 0.5 1\fP will be: 
 *	.Ex 
 *	167 240 263 288 
 *	.Xe
 *	
 *	The inverse function is computed with the \fI-r\fP option. The result of
 *	\fImag -r -R240 167 240 263 288\fP
 *	will be the fontsteps:
 *	.Ex
 *	-2 0 0.5 1
 *	.Xe
 *	
 *	The UNIX shells allow command substitution. One can write:
 *	.Ex
 *	mag -r -R240 `mag -R240 -2 0 0.5 1`
 *	.Xe
 *	
 *	.SH DATE
 *	August, 1992
 *	
 *	.SH AUTHOR
 *	.nf
 *	Piet Tutelaers
 *	University of Technology Eindhoven
 */

#include "basics.h"	/* fatal() */

#include <ctype.h> 
#include <math.h>
#include <stdarg.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#ifdef WIN32
#include <fcntl.h>
#endif

int invert = 0;
float DPI = 300.0;

/* Prototypes */
static int fontsize(double);
static double stepsize(double);

int main(int argc, char *argv[]) {
   float  sz, arg; int c;
   const char *myname = "mag";
   short done;

   while (--argc > 0 && (*++argv)[0] == '-') {
      if (isdigit((int)(*argv)[1])) { /* allow negative numbers as arguments */
         break;
      }
      done=0;
      while ((!done) && (c = *++argv[0]))  /* allow -bcK like options */
      	 switch (c) {
      	 case 'r':
      	    invert = 1; break;
      	 case 'R':
      	    if (*++argv[0] == '\0') {
      	       argc--; argv++;
      	    }
      	    if (argv[0] == NULL) goto invalid;
	    DPI = atof(argv[0]);
	    done = 1;
      	    break;
invalid:
      	 default:
      	    fatal("%s: %c invalid option\n", myname, c);
      	 }
      }

   if (argc < 1) {
      msg  ("mag (ps2pk) version " PACKAGE_VERSION " (" TL_VERSION ")\n");
      msg  ("Usage: %s [-r] [-Rdpi] size . . .\n", myname);
      fatal("\nEmail bug reports to %s.\n", PACKAGE_BUGREPORT);
   }
   
#ifdef WIN32
   setmode(fileno(stdout), _O_BINARY);
#endif

   for ( ; argc; argc--, argv++) {
      arg=atof(argv[0]);
      switch (invert) {
      case 0:
         printf("%d%c", fontsize(arg), argc > 1 ? ' ' : '\n');
         break;
      case 1:
         sz=stepsize(arg);
         if (((int)(10*sz))%10==0)
            printf("%d%c", (int)sz, argc > 1 ? ' ' : '\n');
         else 
            printf("%f%c", sz, argc > 1 ? ' ' : '\n');
      }
   }
   return 0;
}

static int
fontsize(double x)
{
   return(DPI*pow(1.2, x) + 0.5);
}

static double
stepsize(double x)
{
   double s;
   s=(log(x)-log(DPI))/log(1.2);
   if (s>=0) return floor(10*s+0.5)/10;
   return -floor(10*(-s)+0.5)/10;
}
