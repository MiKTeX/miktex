#ifndef _IO_H_
#define _IO_H_

#include <stdio.h>
#include <fribidi/fribidi.h>
#include "fribidixetex-defines.h"

int io_read_line(FriBidiChar *text,int encoding,FILE *f);
void io_write_line(FriBidiChar *text,int encoding,FILE *f);

void io_init(void);

extern int io_line_number;

#endif
