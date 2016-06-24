/*
 * File:    sexpr.c
 * Purpose: evaluate algebraic expression from single argument
 *          (Derived from expr.c from J.T. Conklin <jtc@netbsd.org>.)
 * Version: 1.0 (Nov 1995)
 * Author:  Piet Tutelaers
 */

#include <stdio.h>
#include "strexpr.h"	/* strexpr() */
#include "basics.h"	/* fatal() */

int main(int argc, char *argv[]) {
   int result;

   if (argc < 2) fatal("Usage: sexpr expression ...\n");

   while (--argc > 0) {
      if (strexpr(&result, *++argv))
         fatal("expression error\n");

      printf("%s = %d\n", argv[0] , result);
   }
   exit(0);
}
