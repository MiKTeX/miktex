#ifndef _BIDI_H_
#define _BIDI_H_
#include <stdio.h>
#include <fribidi/fribidi.h>

void bidi_init(FILE *f_out);
int bidi_process(FriBidiChar *in,FriBidiChar *out,
					int replace_minus,int tranlate_only,int no_mirroring);
void bidi_finish(void);

#endif
