/* omegabis.c: C routines to support external OCPs

This file is part of the Omega project, which
is based on the web2c distribution of TeX.

Copyright (c) 1994--2000 John Plaice and Yannis Haralambous

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#if defined(MIKTEX)
#define C4PEXTERN extern
#include "omega-miktex.h"
#else
#define EXTERN extern
#endif
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#if !defined(MIKTEX)
#include "omegad.h"
#endif
#undef read

void
runexternalocp P1C(string, external_ocp_name)
{
  char *in_file_name;
  char *out_file_name;
  FILE *in_file;
  FILE *out_file;
  char command_line[400];
  int i;
  unsigned c;
  int c_in;

#if defined(MIKTEX_WINDOWS)
  char *tempenv;
#define null_string(s) ((s == NULL) || (*s == '\0'))

  tempenv = getenv("TMPDIR");
  if (null_string(tempenv))
    tempenv = getenv("TEMP");
  if (null_string(tempenv))
    tempenv = getenv("TMP");
  if (null_string(tempenv))
    tempenv = "c:/tmp";	/* "/tmp" is not good if we are on a CD-ROM */
  in_file_name = concat(tempenv, "/__omega__in__XXXXXX");
  mktemp(in_file_name);
  in_file = fopen(in_file_name, FOPEN_WBIN_MODE);
#else
  in_file_name = strdup("/tmp/__omega__in__XXXXXX");
  mkstemp(in_file_name);
  in_file = fopen(in_file_name, "w");
#endif
  
  for (i=1; i<=otpinputend; i++) {
      c = otpinputbuf[i];
      if (c>0xffff) {
          fprintf(stderr, "Omega does not currently support 31-bit chars\n");
          exit(1);
      }
      if (c>0x4000000) {
          fputc(0xfc | ((c>>30) & 0x1), in_file);
          fputc(0x80 | ((c>>24) & 0x3f), in_file);
          fputc(0x80 | ((c>>18) & 0x3f), in_file);
          fputc(0x80 | ((c>>12) & 0x3f), in_file);
          fputc(0x80 | ((c>>6) & 0x3f), in_file);
          fputc(0x80 | (c & 0x3f), in_file);
      } else if (c>0x200000) {
          fputc(0xf8 | ((c>>24) & 0x3), in_file);
          fputc(0x80 | ((c>>18) & 0x3f), in_file);
          fputc(0x80 | ((c>>12) & 0x3f), in_file);
          fputc(0x80 | ((c>>6) & 0x3f), in_file);
          fputc(0x80 | (c & 0x3f), in_file);
      } else if (c>0x10000) {
          fputc(0xf0 | ((c>>18) & 0x7), in_file);
          fputc(0x80 | ((c>>12) & 0x3f), in_file);
          fputc(0x80 | ((c>>6) & 0x3f), in_file);
          fputc(0x80 | (c & 0x3f), in_file);
      } else if (c>0x800) {
          fputc(0xe0 | ((c>>12) & 0xf), in_file);
          fputc(0x80 | ((c>>6) & 0x3f), in_file);
          fputc(0x80 | (c & 0x3f), in_file);
      } else if (c>0x80) {
          fputc(0xc0 | ((c>>6) & 0x1f), in_file);
          fputc(0x80 | (c & 0x3f), in_file);
      } else {
          fputc(c & 0x7f, in_file);
      }
  }
  fclose(in_file);
  
#define advance_cin if ((c_in = fgetc(out_file)) == -1) { \
                         fprintf(stderr, "File contains bad char\n"); \
                         goto end_of_while; \
                    }
                     
#if defined(MIKTEX_WINDOWS)
  out_file_name = concat(tempenv, "/__omega__out__XXXXXX");
  mktemp(out_file_name);
#else
  out_file_name = strdup("/tmp/__omega__out__XXXXXX");
  mkstemp(out_file_name);
#endif

  sprintf(command_line, "%s <%s >%s\n",
          external_ocp_name+1, in_file_name, out_file_name);
  system(command_line);
#if defined(MIKTEX_WINDOWS)
  out_file = fopen(out_file_name, FOPEN_RBIN_MODE);
#else
  out_file = fopen(out_file_name, "r");
#endif
  otpoutputend = 0;
  otpoutputbuf[otpoutputend] = 0;
  while ((c_in = fgetc(out_file)) != -1) {
     if (c_in>=0xfc) {
         c = (c_in & 0x1)   << 30;
         {advance_cin}
         c |= (c_in & 0x3f) << 24;
         {advance_cin}
         c |= (c_in & 0x3f) << 18;
         {advance_cin}
         c |= (c_in & 0x3f) << 12;
         {advance_cin}
         c |= (c_in & 0x3f) << 6;
         {advance_cin}
         c |= c_in & 0x3f;
     } else if (c_in>=0xf8) {
         c = (c_in & 0x3) << 24;
         {advance_cin}
         c |= (c_in & 0x3f) << 18;
         {advance_cin}
         c |= (c_in & 0x3f) << 12;
         {advance_cin}
         c |= (c_in & 0x3f) << 6;
         {advance_cin}
         c |= c_in & 0x3f;
     } else if (c_in>=0xf0) {
         c = (c_in & 0x7) << 18;
         {advance_cin}
         c |= (c_in & 0x3f) << 12;
         {advance_cin}
         c |= (c_in & 0x3f) << 6;
         {advance_cin}
         c |= c_in & 0x3f;
     } else if (c_in>=0xe0) {
         c = (c_in & 0xf) << 12;
         {advance_cin}
         c |= (c_in & 0x3f) << 6;
         {advance_cin}
         c |= c_in & 0x3f;
     } else if (c_in>=0x80) {
         c = (c_in & 0x1f) << 6;
         {advance_cin}
         c |= c_in & 0x3f;
     } else {
         c = c_in & 0x7f;
     }
     otpoutputbuf[++otpoutputend] = c;
  }

end_of_while:
  remove(in_file_name);
  remove(out_file_name);
}

