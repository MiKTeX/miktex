/* Copyright (C) 2000-2012 by George Williams */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.

 * The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "fontforge.h"
#include "sd.h"
#ifdef HAVE_IEEEFP_H
# include <ieeefp.h>		/* Solaris defines isnan in ieeefp rather than math.h */
#endif

typedef struct _io {
    const char *macro;
    char *start;
    FILE *ps, *fog;
    char fogbuf[60];
    int backedup, cnt, isloop, isstopped, fogns;
    struct _io *prev;
} _IO;

typedef struct io {
    struct _io *top;
    int endedstopped;
    int advance_width;		/* Can be set from a PS comment by MF2PT1 */
} IO;

#define GARBAGE_MAX	64

/* length (of string)
    fill eofill stroke
    gsave grestore
*/

void MatMultiply(real m1[6], real m2[6], real to[6]) {
    real trans[6];

    trans[0] = m1[0]*m2[0] +
		m1[1]*m2[2];
    trans[1] = m1[0]*m2[1] +
		m1[1]*m2[3];
    trans[2] = m1[2]*m2[0] +
		m1[3]*m2[2];
    trans[3] = m1[2]*m2[1] +
		m1[3]*m2[3];
    trans[4] = m1[4]*m2[0] +
		m1[5]*m2[2] +
		m2[4];
    trans[5] = m1[4]*m2[1] +
		m1[5]*m2[3] +
		m2[5];
    memcpy(to,trans,sizeof(trans));
}
