/* outocp.c: ASCII output for OCP files, mainly for debugging purposes.

This file is part of the Omega project, which
is based in the web2c distribution of TeX.

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
#define P1H(x1) (x1)
#define P1C(t1, v1) (t1 v1)
#define P2C(t1, v1, t2, v2) (t1 v1, t2 v2)
#endif

#if defined(MIKTEX)
#define NO_C4P
#if defined(__cplusplus)
#include <miktex/Core/Core>
#else
#include <miktex/Core/c/api.h>
#endif
#include <miktex/KPSE/Emulation>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define true 1
#else
#include <kpathsea/config.h>
#include <kpathsea/types.h>
#include <kpathsea/c-proto.h>
#include <kpathsea/c-std.h>
#include <kpathsea/c-fopen.h>
#include <kpathsea/tex-file.h>
#endif
#include "otp.h"

char *otp_names[] = {
"                   ",
"OTP_RIGHT_OUTPUT   ",
"OTP_RIGHT_NUM      ",
"OTP_RIGHT_CHAR     ",
"OTP_RIGHT_LCHAR    ",
"OTP_RIGHT_SOME     ",
"OTP_PBACK_OUTPUT   ",
"OTP_PBACK_NUM      ",
"OTP_PBACK_CHAR     ",
"OTP_PBACK_LCHAR    ",
"OTP_PBACK_SOME     ",
"OTP_ADD            ",
"OTP_SUB            ",
"OTP_MULT           ",
"OTP_DIV            ",
"OTP_MOD            ",
"OTP_LOOKUP         ",
"OTP_PUSH_NUM       ",
"OTP_PUSH_CHAR      ",
"OTP_PUSH_LCHAR     ",
"OTP_STATE_CHANGE   ",
"OTP_STATE_PUSH     ",
"OTP_STATE_POP      ",
"OTP_LEFT_START     ",
"OTP_LEFT_RETURN    ",
"OTP_LEFT_BACKUP    ",
"OTP_GOTO           ",
"OTP_GOTO_NE        ",
"OTP_GOTO_EQ        ",
"OTP_GOTO_LT        ",
"OTP_GOTO_LE        ",
"OTP_GOTO_GT        ",
"OTP_GOTO_GE        ",
"OTP_GOTO_NO_ADVANCE",
"OTP_GOTO_BEG       ",
"OTP_GOTO_END       ",
"OTP_STOP           "};


void
ctp_abort P1C(string, s)
{
  fprintf(stderr, s);
  exit(EXIT_FAILURE);
}
 
int no_words_read = 0;

int
ctp_get P1C(FILE *, input_file) {
   int ctpchar;
   ctpchar = getc(input_file);
   if (ctpchar == EOF) {
      ctp_abort("Unexpected end of file\n");
   }
   return ctpchar;
}

int
ctp_read P1C(FILE *, input_file) {
   int ctpword;
   no_words_read++;
   ctpword = ctp_get(input_file);
   if (ctpword>127) {
      ctp_abort("first octet > 127");
   }
   ctpword = ctpword*256 + ctp_get(input_file);
   ctpword = ctpword*256 + ctp_get(input_file);
   ctpword = ctpword*256 + ctp_get(input_file);
   return ctpword;
}

void
ctp_explain P1C(FILE *, input_file) {
   int i,j, arg, entry, instruction;
   int ctp_length, real_ctp_length, ctp_input, ctp_output,
       ctp_no_tables, ctp_room_tables, ctp_no_states, ctp_room_states;
   int room_tables[100], room_states[100];

   ctp_length = ctp_read(input_file);
   fprintf(stdout, "ctp_length     : %3x(%3d)\n",
           ctp_length, ctp_length);
   real_ctp_length=ctp_length-7;
   fprintf(stdout, "real_ctp_length: %3x(%3d)\n",
           real_ctp_length, real_ctp_length);
   ctp_input = ctp_read(input_file);
   fprintf(stdout, "ctp_input      : %3x(%3d)\n",
           ctp_input, ctp_input);
   ctp_output = ctp_read(input_file);
   fprintf(stdout, "ctp_output     : %3x(%3d)\n",
           ctp_output, ctp_output);
   ctp_no_tables = ctp_read(input_file);
   fprintf(stdout, "ctp_no_tables  : %3x(%3d)\n",
           ctp_no_tables, ctp_no_tables);
   ctp_room_tables = ctp_read(input_file);
   fprintf(stdout, "ctp_room_tables: %3x(%3d)\n",
           ctp_room_tables, ctp_room_tables);
   ctp_no_states = ctp_read(input_file);
   fprintf(stdout, "ctp_no_states  : %3x(%3d)\n",
           ctp_no_states, ctp_no_states);
   ctp_room_states = ctp_read(input_file);
   fprintf(stdout, "ctp_room_states: %3x(%3d)\n\n",
           ctp_room_states, ctp_room_states);
   if (ctp_no_tables >= 100) {
      ctp_abort("Too many tables\n");
   }
   if (ctp_no_states >= 100) {
      ctp_abort("Too many states\n");
   }
   if (ctp_no_tables != 0) {
      for(i=0; i<ctp_no_tables; i++) {
         room_tables[i] = ctp_read(input_file);
         fprintf(stdout, "Table %2x(%2d): %3x(%3d)entries\n",
                 i, i, room_tables[i], room_tables[i]);
      }
      fprintf(stdout, "\n");
      for(i=0; i<ctp_no_tables; i++) {
         for(j=0; j<room_tables[i]; j++) {
            entry = ctp_read(input_file);
            fprintf(stdout, "Table %2x(%2d), entry %3x(%3d): %2x(%3d)\n",
                    i, i, j, j, entry, entry);
         }
      }
      fprintf(stdout, "\n");
   }
   if (ctp_no_states != 0) {
      for(i=0; i<ctp_no_states; i++) {
         room_states[i] = ctp_read(input_file);
         fprintf(stdout, "State %2x(%2d): %3x(%3d) entries\n",
                 i, i, room_states[i], room_states[i]);
      }
      fprintf(stdout, "\n");
      for(i=0; i<ctp_no_states; i++) {
         for(j=0; j<room_states[i]; j++) {
	    instruction = ctp_read(input_file);
            arg = instruction & 0xffffff;
            if ((arg>=32) && (arg<127))
               fprintf(stdout,
               "State %2x(%2d), entry %3x(%3d): %s %2x(%3d,`%c')\n",
               i, i, j, j, otp_names[instruction>>24],
               arg, arg, arg);
            else
               fprintf(stdout,
               "State %2x(%2d), entry %3x(%3d): %s %2x(%3d)\n",
               i, i, j, j, otp_names[instruction>>24],
               arg, arg);
         }
      }
   }
   fprintf(stdout, "\nfile length should be: %3x(%3d)\n",
           ctp_length, ctp_length);
   fprintf(stdout, "number words read    : %3x(%3d)\n",
           no_words_read, no_words_read);
}

#if defined(MIKTEX)
#  define main MIKTEXCEECALL Main
#endif
int main P2C(int, argc, string *, argv) {
string input_name, full_input_name;
FILE *input_file;

   kpse_set_progname (argv[0]);
   switch (argc) {
   case 1: fprintf(stderr, "outocp: No file given\n");
         return EXIT_FAILURE;
   case 2: input_name = argv[1];
         break;
   default: fprintf(stderr, "outocp: Too many arguments\n");
         return EXIT_FAILURE;
   }
   full_input_name = kpse_find_file(input_name, kpse_ocp_format, true);
   if (!full_input_name) {
     fprintf(stderr, "outocp: %s not found\n", input_name);
     return EXIT_FAILURE;
   }
   input_file = xfopen(full_input_name, FOPEN_RBIN_MODE);
   ctp_explain(input_file);
   return EXIT_SUCCESS;
}
