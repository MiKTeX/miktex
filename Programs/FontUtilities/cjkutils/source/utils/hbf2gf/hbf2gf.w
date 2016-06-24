% This is the cweb file hbf2gf.w of the CJK package ver. 4.8.4  18-Apr-2015

% Copyright (C) 1994-2015  Werner Lemberg <wl@@gnu.org>
%
% This program is free software; you can redistribute it and/or modify
% it under the terms of the GNU General Public License as published by
% the Free Software Foundation; either version 2 of the License, or
% (at your option) any later version.
%
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
%
% You should have received a copy of the GNU General Public License
% along with this program in doc/COPYING; if not, write to the Free
% Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
% MA 02110-1301 USA

% To print this CWEB file you should (but not must) use the CWEAVE of the
% c2cweb-package (found at the CTAN archives, e.g. ftp.dante.de) and then say
%
%           cweave +ai hbf2gf.w
%
% This (fully compatible) CWEAVE can transform CWEB-files with alternative
% output rules (look at the position of braces below!) the author (it's me
% too :-) prefer. Otherwise this file will be formatted traditionally.

\font\meta=logo10
\def\mf{{\meta META}\-{\meta FONT}}
\def\Om{\char "0A}                      % Omega symbol

\def\title{hbf2gf (CJK Version 4.8.4)}

\def\topofcontents{
  \null\vfill
  \centerline{\titlefont The {\ttitlefont hbf2gf} program}
  \vskip 20pt
  \centerline{(CJK Version 4.8.4)}
  \vfill}

\def\botofcontents{
  \vfill
  \noindent
  Copyright \copyright~1996-1999 by Werner Lemberg
  \bigskip\noindent
  Permission is granted to make and distribute verbatim copies of this
  document provided that the copyright notice and this permission notice are
  preserved on all copies.

  \smallskip\noindent
  Permission is granted to copy and distribute modified versions of this
  document under the conditions for verbatim copying, provided that the
  entire resulting derived work is distributed under the terms of a
  permission notice identical to this one.}

\pageno=\contentspagenumber \advance\pageno by 1
\let\maybe=\iftrue
\fullpageheight=240mm
\pageheight=223mm
\pagewidth=158mm
\setpage
\frenchspacing


\def\msdos{\.{msdos}}
@s msdos TeX

@s HBF int
@s HBF_CHAR int
@s HBF_BBOX int

@s __inline__ int


@* Introduction.
This is the \.{hbf2gf} program by Werner Lemberg
(\.{wl@@gnu.org}).

The ``banner line'' defined here should be changed whenever \.{hbf2gf} is
modified.

@d banner
"hbf2gf (CJK ver. 4.8.4)"


@
\.{hbf2gf} is intended to convert Hanzi Bitmap Fonts (HBF) into \TeX\
generic font files (\.{GF} files) according to the \\{CJK} package, which
\.{hbf2gf} is part of.

The outline of \.{hbf2gf} is simple: a CJK (Chinese/Japanese/Korean) bitmap
file will be scaled and written in at most |nmb_files| \.{GF} files, each
file containing |256|~characters (except the last and possibly the first
one). In the normal case it's not necessary to compute the right value of
|nmb_files| because \.{hbf2gf} will do this; you should use |-1| instead to
indicate this. See the last section for an example.

Alternatively you can call \.{hbf2gf} similar to \mf, i.e., the program will
compute one font on demand. This mode will be used if two or three input
parameters instead of one are given: the font name, the horizontal
resolution, and optionally a vertical scaling factor or resolution to allow
modes for e.g.\ $300\times600\,\hbox{dpi}$ printers. \.{hbf2gf} will extract
the configuration file name from the font name; if this file isn't found,
the program exits with error code~|2| (this is useful for scripts like
\.{mktexpk}). If the configuration file is found but an error occurs while
computing the font, error code~|1| is returned. In case of success, the exit
code is zero.

The characters in the input font files are completely described by the HBF
header file. This program uses the HBF API implementation of Ross Paterson
(\.{ross@@soi.city.ac.uk}; with small extensions). You will find a
description of the HBF standard at \.{ftp.ifcss.org}.

A batch file created by \.{hbf2gf} too (if the program computes a whole set
of subfonts) will convert the \.{GF} files to \.{PK} files using
\.{GFtoPK}, a part of every \TeX\ package.

@d TRUE  1
@d FALSE 0@#

@d STRING_LENGTH 255
         /* the maximal length of an input string in the configuration file */
@d FILE_NAME_LENGTH 1024
                 /* the maximal length (including the path) of a filename */@#

@<Global variables@>=
int nmb_files = -1; /* create all files by default */
int unicode = FALSE; /* whether a Unicode font should be processed */@#

int testing = FALSE; /* whether we test only the font name */@#

int mf_like = FALSE; /* whether we are in the \mf-like mode */
int file_number = 0; /* the subfont number */@#

double x_resolution = 0.0; /* the second and third input parameter */
double y_scale = 1.0;@#

int pk_files = TRUE; /* command line options */
int tfm_files = TRUE;
int long_extension = TRUE;
int quiet = FALSE;@#

char config_file[FILE_NAME_LENGTH + 4 + 1];
                                         /* we probably must add `\.{.cfg}' */
char output_name[STRING_LENGTH + 1];@#

FILE *config, *out;
HBF *hbf;@#

#ifdef msdos /* if we compile under DOS or OS/2 */
#define WRITE_BIN   "wb"
#define WRITE_TXT   "wt"
#define READ_BIN    "rb"
#define READ_TXT    "rt"
#else
#define WRITE_BIN   "w"
#define WRITE_TXT   "w"
#define READ_BIN    "r"
#define READ_TXT    "r"
#endif@#

int end_of_file = FALSE;


@
One \.{PL} file will be created additionally, which describes the font
metrics in a readable way. Because all CJK characters have identical
bounding boxes, one metrics file is enough---the batch job created by
\.{hbf2gf} calls \.{PLtoTF} to produce this \.{TFM} file and then copies
it into |nmb_files| metrics files. There usually will be a discrepancy
between the number of characters in the last \.{GF} file and the
\.{TFM} file, but this does not harm.

If you specify the \.{ofm\_file} option in the configuration file, an
extended virtual property file (such files have the extension \.{.ovp}) for
the \Om\ system is written; this will be then converted with
\.{ovp2ovf} into an \.{OFM} and an \.{OVF} file to map all the subfonts
into one large virtual font.



@* The main routine.
The main routine takes |file_name|, |x_resolution|, and |y_scale| as command
line parameters if in \mf-like mode, otherwise |config_file| as the only
argument. |read_config()| scans the configuration file and fills the global
variables, |write_file()| writes the \.{GF} files, |write_pl()| and
|write_ovp()| write the \.{PL} and \.{OVP} files respectively, and
|write_job()| the batch file.

@c
@<Include files@>@;
@<Prototypes@>@;
@<Global variables@>@;@#


int main(int argc, char *argv[])
   {char *p;

    @<Initialize \TeX\ file searching@>@;@#

    @<Scan options@>@;@#

    if(!quiet)
        printf("\n%s\n\n", banner);@#

    strncpy(config_file, argv[1], FILE_NAME_LENGTH);
    config_file[FILE_NAME_LENGTH] = '\0';@#

    if(argc > 2 || testing)
       {int l = strlen(config_file);

        if(l > 2)
            config_file[l - 2] = '\0';
                                     /* strip subfont number from file name */
        else
           {if(!quiet)
                printf("`%s' can't be a subfont created by hbf2gf\n",
                       config_file);
            exit(2);
           }@#

        mf_like = TRUE;
       }@#
        
    read_config(); /* will call |exit(1)| on errors */@#

    if(mf_like)
        @<Check other arguments@>@;@#

    @<Initialize variables@>@;@#

    @<Write files@>@;@#

    if(tfm_files)
        write_pl();
    if(ofm_file)
        write_ovp();
    if(!mf_like)
        write_job();@#

    hbfClose(hbf);@#

    exit(0);
    return 0; /* never reached */
   }


@
@d VERSION @/
"\n"@/
"Copyright (C) 1996-1999 Werner Lemberg.\n"@/
"There is NO warranty.  You may redistribute this software\n"@/
"under the terms of the GNU General Public License\n"@/
"and the HBF library copyright.\n"@/
"\n"@/
"For more information about these matters, see the files\n"@/
"named COPYING and hbf.c.\n"@/
"\n"

@<Print version@>=
           {printf("\n");
            printf(banner);
            printf(" (%s)\n", TeX_search_version());
            printf(VERSION);
            exit(0);
           }


@
@d USAGE @/
"\n"@/
"Usage: hbf2gf [-q] configuration_file[.cfg]\n"@/
"       hbf2gf [options] font_name x_resolution [y_scale | y_resolution]\n"@/
"       hbf2gf -t [-q] font_name\n"@/
"\n"@/
"  Convert a font in HBF format to TeX's GF resp. PK format.\n"@/
"\n"@/
"         -q             be silent\n"@/
"         -p             don't produce a PL file\n"@/
"         -g             don't produce a GF file\n"@/
"         -n             use no resolution in extension (only `.gf')\n"@/
"         -t             test for font_name (returns 0 on success)\n"@/
"         --help         print this message and exit\n"@/
"         --version      print version number and exit\n"@/
"\n"

@<Print help information@>=
           {printf(USAGE);
            exit(0);
           }


@
Three options can be specified to the program (\.{-p}, \.{-g}, and \.{-n})
if in \mf-like mode to suppress creation of a \.{PL} resp.\ a
\.{GF} file, and to force a `\.{.gf}' extension (instead of
e.g.\ `\.{.300gf}'). The corresponding setting of a particular switch in the
configuration file is ignored then.

Additionally, the option \.{-t} tests whether the specified subfont name
leads to an \.{hbf2gf} configuration file. It returns 0 on success and prints
out the name of that configuration file (provided the \.{-q} switch isn't
set). This test isn't a thorough one; it only removes the last two characters
and checks whether a configuration file with that name exists.

@<Scan options@>=
    if(argc == 2)
       {if(strcmp(argv[1], "--help") == 0)
            @<Print help information@>@;
        else if(strcmp(argv[1], "--version") == 0)
            @<Print version@>@;
       }@#

    while(argc > 1)
       {p = argv[1];
        if(p[0] != '-')
            break;
        if(p[1] == 'p')
            tfm_files = FALSE;
        else if(p[1] == 'g')
            pk_files = FALSE;
        else if(p[1] == 'n')
            long_extension = FALSE;
        else if(p[1] == 'q')
            quiet = TRUE;
        else if(p[1] == 't')
            testing = TRUE;@#

        argv++;
        argc--;
       }@#

    if(testing)
       {if(argc != 2)
           {fprintf(stderr, "Need exactly one parameter for `-t' option.\n");
            fprintf(stderr, "Try `hbf2gf --help' for more information.\n");
            exit(1);
           }
       }
    else if(argc < 2 || argc > 4)
       {fprintf(stderr, "Invalid number of parameters.\n");
        fprintf(stderr, "Try `hbf2gf --help' for more information.\n");
        exit(1);
       }@#


@
If the (optional) argument is larger than~|10|, we treat it as a value for
the vertical resolution (in dpi), otherwise as a vertical scaling factor.

@<Check other arguments@>=
   {if(unicode)
        file_number = (int)strtol(&argv[1][strlen(argv[1]) - 2],
                                  (char **)NULL, 16);
    else
        file_number = atoi(&argv[1][strlen(argv[1]) - 2]);@#

    x_resolution = atof(argv[2]);
    if(x_resolution < PRINTER_MIN_RES_X)
       {fprintf(stderr, "Invalid horizontal resolution\n");
        exit(1);
       }@#

    if(argc > 3)
       {y_scale = atof(argv[3]);
        if(y_scale < 0.01)
           {fprintf(stderr,
                    "Invalid vertical scaling factor or resolution\n");
            exit(1);
           }
        if(y_scale > 10.0)
            y_scale = (double)x_resolution / y_scale;
       }
   }


@
If |unicode| is |TRUE|, the start value of the running number appended to
the base name of the output font files is taken from the HBF header file,
otherwise it starts with~`\.{01}'. |min_char| represents the lower bound of
the code range.

If we are in \mf-like mode, |file_number| is taken from the command line,
and |max_numb| will be set to~|1|.

@<Write files@>=
   {int j, max_numb;

    if(!mf_like)
       {file_number = (unicode == TRUE ? (min_char >> 8) : 1);
        if(nmb_files == -1)
            max_numb = (unicode == TRUE ? 0x100 : 100);
        else
            max_numb = nmb_files;
       }
    else
        max_numb = 1;@#

    for(j = 0; (j < max_numb) && !end_of_file; file_number++, j++)
        write_file();@#

    nmb_files = j; /* the real number of output font files */
   }


@
@<Include files@>=
#ifdef HAVE_CONFIG_H
#include <c-auto.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef TM_IN_SYS_TIME
#include <sys/time.h>
#endif
#include "hbf.h"



@* The functions.
The first function to be described is |write_file()|. Each \.{GF} file
consists of three sections: a preamble, a data section, and a postamble. The
functions |write_pre()|, |write_data()|, and |write_post()| handle this.

@<Prototypes@>=
static void write_file(void);


@
In \mf-like mode we create font file name extensions similar to \mf\ if
the \.{-n} option isn't specified; otherwise only `\.{.gf}' will be appended.

@c
static void write_file(void)
   {char output_file[FILE_NAME_LENGTH + 1];


    if(pk_files)
       {if(mf_like)
           {if(unicode)
                sprintf(output_file, "%s%02x.%.0igf",@/
                        output_name, file_number,
                         long_extension ? (int)(x_resolution + 0.5) : 0);
            else
                sprintf(output_file, "%s%02i.%.0igf",@/
                        output_name, file_number,
                         long_extension ? (int)(x_resolution + 0.5) : 0);
           }
        else
           {if(unicode)
                sprintf(output_file, "%s%02x.gf", output_name, file_number);
            else
                sprintf(output_file, "%s%02i.gf", output_name, file_number);
           }
        if(!(out = fopen(output_file, WRITE_BIN)))
           {fprintf(stderr, "Couldn't open `%s'\n", output_file);
            exit(1);
           }
        if(!quiet)
            printf("Writing `%s' ", output_file);@#

        write_pre();
        write_data();
        write_post();
        fclose(out);@#

        if(!quiet)
            printf("\n");
       }
    else
        write_data();
   }


@
The preamble has two bytes at the very beginning, |PRE| and |GF_ID|. |PRE|
starts the preamble, and |GF_ID| is the Generic Font Identity Byte. The next
bytes are a string in Pascal format containing a header, the date, and the
time. Strings in Pascal format start with the length of the string and have
no terminating NULL byte.

@d GF_ID 131
@d PRE   247@#

@d header " hbf2gf output "

@<Prototypes@>=
static void write_pre(void);


@
@s tm int

@c
static void write_pre(void)
   {char out_s[40], s[20];
    time_t secs_now;
    struct tm *time_now;


    strcpy(out_s, header);@#

    secs_now = time(NULL); /* system date and time */
    time_now = localtime(&secs_now);
    strftime(s, 20, "%Y.%m.%d:%H.%M", time_now);
    strcat(out_s, s);@#

    fputc(PRE, out);
    fputc(GF_ID, out);
    fputc(strlen(out_s), out);
    fputs(out_s, out);
   }


@
|write_data()| produces the middle part of the \.{GF} file. It first sets
|char_adr_p| equal to the address of |char_adr[]| which will contain file
offsets of the compressed characters.

|input_size_x| and |input_size_y| reflect the original dimensions of the
bitmap font, |pk_output_size_x| and |pk_output_size_y| contain the width and
height of the output character box (in pixels), |pk_offset_x| and
|pk_offset_y| define the baseline of the font. The same names starting with
`\\{tfm\_}' instead of `\\{pk\_}' are used for \.{TFM} files (values are
multiples of design size). |mag_x| and |mag_y| hold the scaling factors
which are needed to reach |design_size|. |slant| defines the slant ($\Delta
x / \Delta y$), |target_size_x| and |target_size_y| will be the final
dimensions; |magstep_x = target_size_x / design_size| is \TeX's
\.{\\magstep}.

The \CEE/ standard specifies that all global values will be automatically
set to~|0| if no initialization value is given.

@<Global...@>=
long char_adr[256];
long *char_adr_p;@#

int pk_offset_x;
      /* horizontal offset (increase character width a bit; will be applied on
         both the left and the right side) */
double tfm_offset_x;
int pk_offset_y;
               /* vertical offset (must be configured to desired font size) */
double tfm_offset_y;@#

int input_size_x;
int input_size_y;
const char *font_encoding; /* taken from the HBF file */
int pk_width; /* without slant */
int pk_output_size_x;
                     /* the output character box dimensions without offsets */
double tfm_output_size_x;
int pk_output_size_y;
double tfm_output_size_y;@#

double design_size = 10.0; /* in points */
double target_size_x; /* in points */
double target_size_y;
double magstep_x;
double magstep_y;
double slant;
int rotation;@#

double mag_x; /* horizontal and vertical magnification values */
double mag_y;@#

int empty_char; /* a flag whether the character does not exist or is empty */
int last_char; /* the last valid character in a \.{GF} file */@#

int dot_count; /* this counts the processed characters;
                  every ten characters a dot is output to the screen */


@
@<Prototypes@>=
static void write_data(void);


@
@c
static void write_data(void)
   {dot_count = 0;
    char_adr_p = char_adr;@#

    for(last_char = 0; (last_char < 256) && !end_of_file; last_char++)
        @<Write character@>@;
   }


@
The code in this section saves the current file position first and calls
|make_pixel_array()|, which expands and scales the character bitmap.

|BOC| (and |BOC1|), the Begin Of Character command byte, must be followed by
the character code and the dimensions of the character as explained in
``\mf---the program'' (corrected by vertical and horizontal offsets).

|write_coding()| compresses and outputs the bitmap; |EOC| (End Of Character)
finishes the current character.

@d BOC  67
@d BOC1 68 /* simplified version of |BOC| */
@d EOC  69

@<Write character@>=
       {if(dot_count++ % 10 == 0) /* a progress report for impatient users */
            if(pk_files && !quiet)
               {printf(".");
                fflush(stdout);
               }@#

        empty_char = FALSE;
        make_pixel_array();
        if(end_of_file)
            return;@#

        if(pk_files)
           {*char_adr_p = ftell(out);
            char_adr_p++;@#

            if(empty_char)
               {fputc(BOC1, out);
                fputc((unsigned char)last_char, out);
                fputc(0, out);
                fputc(0, out);
                fputc(0, out);
                fputc(0, out);
                fputc(EOC, out);
               }
            else
               {fputc(BOC, out);
                fputl(last_char, out);
                fputl(-1L, out);
                fputl(pk_offset_x, out);
                fputl(pk_output_size_x + pk_offset_x, out);
                fputl(pk_offset_y, out);
                fputl(pk_output_size_y + pk_offset_y, out);@#

                write_coding();@#

                fputc(EOC, out);
               }
           }
       }


@
The current \.{GF} file will be completed with data written by
|write_post()|. The end consists of three sections: ``special'', ``post'',
and ``postpost''. The first contains material not used by \TeX\ itself but
which can be used by other programs like \.{GFtoDVI} or for documentary
purposes (|coding[]| and |comment[]|). The second describes the font as a
whole, and the last marks the end of the file.

|pk_total_min_x| up to |pk_total_max_y| define the greatest bounding box of
this file (including offsets); the horizontal character escapement after
drawing the character is |pk_dx|. |tfm_width| is the width in multiples of
the design size ignoring the target size.

@d _2_16   65536.0 /* $2^{16}$ */
@d _2_20 1048576.0 /* $2^{20}$ */

@<Global...@>=
char coding[STRING_LENGTH + 1]; /* a comment describing the font encoding */
char comment[STRING_LENGTH + 1]; /* a comment describing the font */@#

unsigned long checksum;@#

long pk_total_min_x;
long pk_total_max_x;
long pk_total_min_y;
long pk_total_max_y;@#

int dpi_x; /* printer resolution */
int dpi_y;
double ppp_x; /* pixels per point */
double ppp_y;


@
To clarify the meaning of these values see the sections about the metrics
and configuration file also.

\TeX\ defines that 72.27~points are exactly 1~inch.

@<Prototypes@>=
static void write_post(void);


@
@c
static void write_post(void)
   {long special_adr;
    long post_adr;@#

    long designsize = design_size * _2_20; /* design size $* 2^{20}$ */@#

    int pk_dx;
    long tfm_width;@#

    int i;
    long temp;


    ppp_x = dpi_x / 72.27 * magstep_x;
    ppp_y = dpi_y / 72.27 * magstep_y;@#

    pk_total_min_x = pk_offset_x;
    pk_total_max_x = pk_output_size_x + 2 * pk_offset_x;
    pk_total_min_y = pk_offset_y;
    pk_total_max_y = pk_output_size_y + pk_offset_y;@#

    pk_dx = pk_width + 2 * pk_offset_x; /* no slant */
    tfm_width = (tfm_output_size_x + 2 * tfm_offset_x) * _2_20;
                          /* width in multiples of design size $* 2^{20}$ */@#

    @<Special section@>@;
    @<Post section@>@;
    @<Postpost section@>@;
   }


@
\.{XXXn} will be followed by n~bytes representing the length of a string
which follows immediately. |YYY| is a 32~bit integer which is normally
connected with the preceding string (but not used here). |special_adr|
contains the address of the ``special section''. All items here are
optional.

@d XXX1 239 /* these are all special command bytes */
@d XXX2 240 /* not used */
@d XXX3 241 /* not used */
@d XXX4 242 /* not used */@#

@d YYY  243 /* not used */

@<Special section@>=
    special_adr = ftell(out);@#

    if(*coding)
       {fputc(XXX1, out); /* |XXX1| implies a string length |< 256| */
        fputc(strlen(coding), out);
        fputs(coding, out);
       }@#

    if(*comment)
       {fputc(XXX1, out);
        fputc(strlen(comment), out);
        fputs(comment, out);
       }


@
All character offsets collected in |char_adr| will be written to the output
file. |fputl()| writes a 32~bit integer into a file.

|CHAR_LOC0| (and |CHAR_LOC|) is the first byte of a character locator
(i.e., offset, character code, and width information). |POST| starts the
postamble, and |post_adr| points to the beginning byte of the postamble.

@d POST 248@#

@d CHAR_LOC 245
@d CHAR_LOC0 246 /* simplified version of |CHAR_LOC| */

@<Post section@>=
    post_adr = ftell(out);
    fputc(POST, out);
    fputl(special_adr, out);@#

    fputl(designsize, out);
    fputl(checksum, out);
    fputl(ppp_x * _2_16, out);
    fputl(ppp_y * _2_16, out);
    fputl(pk_total_min_x, out);
    fputl(pk_total_max_x, out);
    fputl(pk_total_min_y, out);
    fputl(pk_total_max_y, out);@#

    char_adr_p = char_adr;@#

    if(pk_dx < 256)
       {for(i = 0; i < last_char; i++) /* the character locators */
           {fputc(CHAR_LOC0, out);
            fputc(i, out);
            fputc(pk_dx, out);
            fputl(tfm_width, out);
            fputl(*char_adr_p++, out);
           }
       }
    else /* will only happen if |MAX_CHAR_SIZE >= 256| */
       {for(i = 0; i < last_char; i++)
           {fputc(CHAR_LOC, out);
            fputc(i, out);
            fputl(pk_dx * _2_16, out);
            fputl(0, out);
            fputl(tfm_width, out);
            fputl(*char_adr_p++, out);
           }
       }


@
|POSTPOST| starts the section after the postamble. To get all information in
a \.{GF} file, you must start here. The very last bytes of the file have
the value |POSTPOST_ID| (the file is filled with at least 4 of these bytes
until a file length of a multiple of~4 is reached). Going backwards a
|GF_ID| will be next, then comes the address of the postamble section.

Jumping to the postamble, a |POST| byte comes first, then the address of
the special section, and afterwards all character offsets. These offsets and
addresses describe the whole file.

@d POSTPOST    249
@d POSTPOST_ID 223

@<Postpost section@>=
    fputc(POSTPOST, out);
    fputl(post_adr, out);
    fputc(GF_ID, out);
    temp = ftell(out);
    i = (int)(temp % 4) + 4;
    while(i--)
        fputc(POSTPOST_ID, out);


@
\TeX\ wants the most significant byte first.

@<Prototypes@>=
static void fputl(long, FILE *);


@
@c
static void fputl(long num, FILE *f)
   {fputc(num >> 24, f);
    fputc(num >> 16, f);
    fputc(num >> 8, f);
    fputc(num, f);
   }


@
|make_pixel_array()| scales a character into the array |out_char[]| where
each \\{byte} represents one pixel, contrary to the input file where each
\\{bit} is used to store the character bitmap. |BLACK| indicates a black
pixel.

The scaling routine was modeled after the program \.{pnmscale} of the
\.{pbmplus} package. \.{pbmplus} was designed to handle arbitrary pictures,
and bitmaps are only a special case of a graymap with values from~0 for
white up to |PIXEL_MAXVAL = 255| for black.

If |EOF| is encountered, |end_of_file| is set and the function returns
immediately.

@d BLACK 1
@d WHITE 0@#

@d PIXEL_MAXVAL  255
@d SCALE        4096
@d HALFSCALE    2048@#

@d MAX_CHAR_SIZE 1023

@<Global...@>=
HBF_CHAR code;
const unsigned char *bitmap;
            /* a proper input bitmap array will be allocated by the HBF API */
const unsigned char *bP;@#

unsigned char out_char[MAX_CHAR_SIZE * MAX_CHAR_SIZE + 1];
                                                 /* the output bitmap array */
unsigned char *out_char_p;@#

unsigned char pixelrow[MAX_CHAR_SIZE];
unsigned char temp_pixelrow[MAX_CHAR_SIZE];
unsigned char new_pixelrow[MAX_CHAR_SIZE + 1];
                                 /* we need space to append a white pixel */@#
int curr_row; /* for |read_row()| if we access the glyph rotated */

long grayrow[MAX_CHAR_SIZE];@#

long s_mag_x, s_mag_y, s_slant;


@
We need to initialize the |grayrow[]| array together with some other
variables.

Two steps are necessary to compute the |code| if we are in the \mf-like
mode. Assuming that we search the code \.{0xXXYY}, we first set |code| to
the value \.{0xXXAA}, where \.{0xAA} is equal to |min_2_byte| (getting
|offset| as the number of remaining characters to reach the first character
in our given subfont), then we increment |code| (and decrement |offset|)
until |offset| equals~0.

@<Initialize variables@>=
   {int col, offset;

    if(rotation)
       {int tmp;


        tmp = input_size_x;
        input_size_x = input_size_y;
        input_size_y = tmp;
       }@#

    if(mf_like)
       {target_size_x = design_size * (x_resolution / dpi_x);
        target_size_y = design_size * (x_resolution * y_scale / dpi_y);
       }
    else
       target_size_x = target_size_y = design_size;
    magstep_x = target_size_x / design_size;
    magstep_y = target_size_y / design_size;@#

    pk_offset_x = offset_x * magstep_x + 0.5;
    pk_offset_y = offset_y * magstep_y + 0.5;
    tfm_offset_x = offset_x / (dpi_x / 72.27) / design_size;
    tfm_offset_y = offset_y / (dpi_y / 72.27) / design_size;@#

    pk_width = input_size_x * mag_x * magstep_x + 0.5; /* without slant */
    pk_output_size_x = input_size_x * mag_x * magstep_x + 
                       input_size_y * mag_y * magstep_y * slant + 0.5;
    pk_output_size_y = input_size_y * mag_y * magstep_y + 0.5;
    tfm_output_size_x = input_size_x * mag_x /
                          (dpi_x / 72.27) / design_size;
    tfm_output_size_y = input_size_y * mag_y /
                          (dpi_y / 72.27) / design_size;
    if(pk_output_size_x > MAX_CHAR_SIZE)
       {fprintf(stderr, "Output character box width too big\n");
        exit(1);
       }
    if(pk_output_size_y > MAX_CHAR_SIZE)
       {fprintf(stderr, "Output character box height too big\n");
        exit(1);
       }@#

    for(col = 0; col < input_size_x; ++col)
        grayrow[col] = HALFSCALE;@#

    if(!mf_like)
        code = (min_char & 0xFF00) + min_2_byte;
    else
       {if((file_number < (unicode ? 0 : 1)) || (file_number >= 0x100))
           {fprintf(stderr, "Invalid subfile number\n");
            exit(1);
           }@#

        if(unicode)
           {offset = 0;
            code = file_number * 0x100;
           }
        else
           {offset = (file_number - 1) * 256 % nmb_2_bytes;
            code = (min_char & 0xFF00) + min_2_byte +
                     (file_number - 1) * 256 / nmb_2_bytes * 0x100;
           }@#

        while(offset--)
            while(!b2_codes[code++ & 0xFF])
                                           /* eliminate invalid |b2_codes| */
                ;@#

        if(code > max_char)
           {fprintf(stderr, "Invalid subfile number\n");
            exit(1);
           }
       }

    s_mag_x = mag_x * magstep_x * SCALE;
    s_mag_y = mag_y * magstep_y * SCALE;
    s_slant = slant * SCALE;
   }


@
All arrays of the \\{pixelrow} family contain gray values. While scaling
with non-integer values a pixel of the input bitmap will normally not align
with the pixel grid of the output bitmap (geometrically spoken). In this
case we first compute the fractions of input pixel rows scaled vertically
and add the corresponding gray values until a temporary row is produced.
Then we repeat this procedure horizontally pixel by pixel and write the
result into an output array.


@<Prototypes@>=
static void make_pixel_array(void);


@
@c
static void make_pixel_array(void)
   {unsigned char *prP;
    unsigned char *temp_prP;
    unsigned char *new_prP;
    long *grP;@#

    register unsigned char *xP;
    register unsigned char *nxP;@#

    register int row, col;
    int rows_read = 0;
    register int need_to_read_row = 1;@#

    long frac_row_to_fill = SCALE;
    long frac_row_left = s_mag_y;@#

    int no_code = FALSE;


    prP = pixelrow;
    temp_prP = temp_pixelrow;
    new_prP = new_pixelrow;
    grP = grayrow;
    out_char_p = out_char; /* will be increased by |write_row()| */@#

again:
    if(b2_codes[code & 0xFF]) /* a valid second byte? */
       {if(pk_files)
           {bitmap = hbfGetBitmap(hbf, code);
            bP = bitmap;
                                     /* will be increased by |read_row()| */@#

            if(!bitmap)
                empty_char = TRUE;
            else
                @<Scale row by row@>@;
           }
       }
    else
        no_code = TRUE;@#

    if((code & 0xFF) == max_2_byte)
        code += 0xFF - (max_2_byte - min_2_byte); /* go to next plane */
    if(code >= max_char)
       {end_of_file = TRUE;
        return;
       }@#

    code++;@#

    if(no_code)
       {no_code = FALSE;
        goto again;
       }
   }


@
@<Scale row by row@>=
   {if(pk_output_size_y == input_size_y)  /* shortcut Y~scaling if possible */
        temp_prP = prP;@#

    curr_row = input_size_y - 1; /* only needed for rotated glyphs */
    for(row = 0; row < pk_output_size_y; ++row)
       {@<Scale Y from |pixelrow[]| into |temp_pixelrow[]|@>@;
        @<Scale X from |temp_pixelrow[]| into |new_pixelrow[]|
          and write it into |out_char[]|@>@;
       }
   }


@
@<Scale Y from |pixelrow[]| into |temp_pixelrow[]|@>=
        if(pk_output_size_y == input_size_y)
                                          /* shortcut Y~scaling if possible */
            read_row(prP);
        else
           {while(frac_row_left < frac_row_to_fill)
               {if(need_to_read_row)
                    if(rows_read < input_size_y)
                       {read_row(prP);
                        ++rows_read;
                       }@#

                for(col = 0, xP = prP; col < input_size_x; ++col, ++xP)
                    grP[col] += frac_row_left * (*xP);@#

                frac_row_to_fill -= frac_row_left;
                frac_row_left = s_mag_y;
                need_to_read_row = 1;
               }@#

            @<Produce a temporary row@>@;
           }


@
Now |frac_row_left >= frac_row_to_fill|, so we can produce a row.

@<Produce a temporary row@>=
            if(need_to_read_row)
                if(rows_read < input_size_y)
                   {read_row(prP);
                    ++rows_read;
                    need_to_read_row = 0;
                   }@#

            for(col = 0, xP = prP, nxP = temp_prP;
                col < input_size_x; ++col, ++xP, ++nxP)
               {register long g;


                g = grP[col] + frac_row_to_fill * (*xP);
                g /= SCALE;
                if(g > PIXEL_MAXVAL)
                    g = PIXEL_MAXVAL;@#

                *nxP = g;
                grP[col] = HALFSCALE;
               }@#

            frac_row_left -= frac_row_to_fill;
            if(frac_row_left == 0)
               {frac_row_left = s_mag_y;
                need_to_read_row = 1;
               }
            frac_row_to_fill = SCALE;


@
To implement the slant we move the starting point |nxP| to the right
according to the corresponding |y|~value. To simplify life only positive
shift values are allowed.

We always append a white pixel to avoid artefacts at the end of the line
produced by the last line. This rule sets the second condition that the
slant must not be greater than~|1|---such a large slant would be unusable
anyway for typesetting purposes.

@<Scale X from |temp_pixelrow[]| into |new_pixelrow[]|
  and write it into |out_char[]|@>=
        if(pk_width == input_size_x && s_slant == 0)
                                          /* shortcut X~scaling if possible */
            write_row(temp_prP);
        else
           {register long g = HALFSCALE;
            register long frac_col_to_fill = SCALE;
            register long frac_col_left;
            register int need_col = 0;


            nxP = new_prP;@#

            frac_col_left = (pk_output_size_y - row) * s_slant;
            while(frac_col_left >= frac_col_to_fill)
               {*(nxP++) = 0;
                frac_col_left -= frac_col_to_fill;
               }@#

            if(frac_col_left > 0)
                frac_col_to_fill -= frac_col_left;@#

            for(col = 0, xP = temp_prP; col < input_size_x; ++col, ++xP)
               {frac_col_left = s_mag_x;
                while(frac_col_left >= frac_col_to_fill)
                   {if(need_col)
                       {++nxP;
                        g = HALFSCALE;
                       }@#

                    g += frac_col_to_fill * (*xP);
                    g /= SCALE;
                    if(g > PIXEL_MAXVAL)
                        g = PIXEL_MAXVAL;@#

                    *nxP = g;
                    frac_col_left -= frac_col_to_fill;
                    frac_col_to_fill = SCALE;
                    need_col = 1;
                   }@#

                if(frac_col_left > 0)
                   {if(need_col)
                       {++nxP;
                        g = HALFSCALE;
                        need_col = 0;
                       }@#

                    g += frac_col_left * (*xP);
                    frac_col_to_fill -= frac_col_left;
                   }
               }@#

            @<Write out a row@>@;
           }


@
@<Write out a row@>=
            if(frac_col_to_fill > 0)
               {--xP;
                g += frac_col_to_fill * (*xP);
               }@#

            if(!need_col)
               {g /= SCALE;
                if(g > PIXEL_MAXVAL)
                    g = PIXEL_MAXVAL;
                *nxP = g;
               }@#

            *(++nxP) = 0; /* append a blank pixel */@#

            write_row(new_prP);


@
|read_row()| reads a row from |bitmap[]| and converts it into a graymap row.
If the |rotation| flag has been set, we get the proper column instead (note
that in this case |input_size_x| already reflects the width of the rotated
glyph).

@<Prototypes@>=
static
#ifdef __GNUC__
__inline__
#endif
void read_row(unsigned char *);


@
@c
static
#ifdef __GNUC__
__inline__
#endif
void read_row(unsigned char *pixelrow)
   {register int col, bitshift, offset;
    register unsigned char *xP;
    register unsigned char item = 0;

    if(rotation)
       {bitshift = 7 - (curr_row % 8);
        offset = (input_size_y + 7) / 8;
        bP = bitmap + curr_row / 8;
        for(col = 0, xP = pixelrow; col < input_size_x; ++col, ++xP)
           {*xP = ((*bP >> bitshift) & 1) == 1 ? PIXEL_MAXVAL : 0;
            bP += offset;
           }
        curr_row--;
       }
    else
       {bitshift = -1;
        for(col = 0, xP = pixelrow; col < input_size_x; ++col, ++xP)
           {if(bitshift == -1)
               {item = *(bP++); /* increase input bitmap pointer */
                bitshift = 7;
               }
            *xP = ((item >> bitshift) & 1) == 1 ? PIXEL_MAXVAL : 0;
            --bitshift;
           }
       }
   }


@
|write_row()| converts the graymap back into a bitmap using a simple
threshold.

@<Global...@>=
int threshold = 128;


@
@<Prototypes@>=
static
#ifdef __GNUC__
__inline__
#endif
void write_row(unsigned char *);


@
@c
static
#ifdef __GNUC__
__inline__
#endif
void write_row(unsigned char *pixelrow)
   {register int col;
    register unsigned char *xP;

    for(col = 0, xP = pixelrow; col < pk_output_size_x; ++col, ++xP)
        *(out_char_p++) = (*xP >= threshold) ? 1 : 0;
                                          /* increase output bitmap pointer */
   }


@
Now comes the most interesting routine. The pixel array will be compressed
in sequences of black and white pixels.

|SKIP0|, |SKIP1|, and |SKIP2| indicate how many blank lines will be skipped.
|PAINT_(x)| means that the next x~pixels will have the same color, then the
color changes. |NEW_ROW_(x)| is the first black pixel in the next row.

An example: the pixel sequence 111100011001 [new row] 000111011110 will be
output as 4 3 2 2 1 77 3 1 4 1.

Commands with an ending~`n' in its name indicate that the next n~bytes
should be read as the counter. Example: |SKIP1|~26 means `skip the next
26~rows'.

For further details please refer to ``\mf---the program''.

@d PAINT_(x)    (x)        /* $0 \le x \le 63$ */
@d PAINT1       64
@d PAINT2       65
@d PAINT3       66         /* not used */@#

@d SKIP0        70
@d SKIP1        71
@d SKIP2        72
@d SKIP3        73         /* not used */@#

@d NEW_ROW_(x)  ((x) + 74) /* $0 \le x \le 164$ */@#

@d NOOP        244         /* not used */

@<Prototypes@>=
static void write_coding(void);


@
The |goto start| instruction causes some compilers to complain about
``Unreachable code $\ldots$'' or something similar.

@c
static void write_coding(void)
   {register int count, skip;
    register unsigned char paint;
    register int x, y;
    register unsigned char *cp;

    x = 0;
    y = 0;
    cp = out_char + y * pk_output_size_x + x;
    count = skip = 0;
    paint = WHITE;
    goto start;@#

    while (y < pk_output_size_y)
       {@<Search blank lines@>@;
start:
        @<Process rest of line@>@;
        y++;
       }
   }@#


@
@<Search blank lines@>=
        count = 0;
        x = 0;
        cp = out_char + y * pk_output_size_x + x;@#

        while(x < pk_output_size_x)
           {if(*cp == paint)
                count++;
            else
               {if(skip == 0)
                   {if(count <= 164)
                        fputc(NEW_ROW_(count), out);
                    else
                       {fputc(SKIP0, out);@#

                        if(count < 256)
                           {fputc(PAINT1, out);
                            fputc(count, out);
                           }
                        else
                           {fputc(PAINT2, out);
                            fputc(count >> 8, out);
                            fputc(count & 0xFF, out);
                           }
                       }
                   }
                else
                   {if(skip == 1)
                        fputc(SKIP0, out);
                    else
                       {if(skip < 256)
                           {fputc(SKIP1, out);
                            fputc(skip, out);
                           }
                        else
                           {fputc(SKIP2, out);
                            fputc(skip >> 8, out);
                            fputc(skip & 0xFF, out);
                           }
                       }
                    skip = 0;
                    if(count < 64)
                        fputc(PAINT_(count), out);
                    else if(count < 256)
                       {fputc(PAINT1, out);
                        fputc(count, out);
                       }
                    else
                       {fputc(PAINT2, out);
                        fputc(count >> 8, out);
                        fputc(count & 0xFF, out);
                       }
                   }
                count = 0;
                paint = BLACK;
                break;
               }
            x++;
            cp++;
           }
        if(x >= pk_output_size_x)
           {skip++;
            y++;
            continue;
           }


@
@<Process rest of line@>=
        while(x < pk_output_size_x)
           {if(*cp == paint)
                count++;
            else
               {if(count < 64)
                    fputc(PAINT_(count), out);
                else if(count < 256)
                   {fputc(PAINT1, out);
                    fputc(count, out);
                   }
                else
                   {fputc(PAINT2, out);
                    fputc(count >> 8, out);
                    fputc(count & 0xFF, out);
                   }
                count = 1;
                paint = BLACK - paint;
               }
            x++;
            cp++;
           }
        if(paint == BLACK)
           {if(count < 64)
                fputc(PAINT_(count), out);
            else if(count < 256)
               {fputc(PAINT1, out);
                fputc(count, out);
               }
            else
               {fputc(PAINT2, out);
                fputc(count >> 8, out);
                fputc(count & 0xFF, out);
               }
            paint = WHITE;
           }



@* The font metrics file.
This routine creates one \.{PL} file with the font properties. None of the
\TeX\ font dimensions are needed because you never will use CJK fonts
directly, and intercharacter stretching is handled by the \\{CJK} macro
\.{\\CJKglue}. (Other packages may define similar commands.)

The name of the \.{PL} file will contain the running two digits in
\mf-like mode only.

It makes sense \\{not} to compute the check sum automatically for two
reasons. Firstly, since \TeX's checksum algorithm is based on the character
width, the number of valid characters, and the designsize, there is a much
higher chance that two subfonts from different HBF fonts have the same check
sum than it is for ordinary fonts, because all characters have the same
width, usually 256~characters in a subfont, and very often the same design
size. Secondly, and this is more important, we create just one
\.{TFM} file for all subfonts regardless of the real number of characters
in a particular subfont.

To have an identification string in the \.{TFM} file, we split it into
single bytes and use the \.{HEADER} command repeatedly.

@<Prototypes@>=
static void write_pl(void);


@
@c
static void write_pl(void)
   {int i, pos;
    char output_file[FILE_NAME_LENGTH + 1];
    long t, sc;
    char *s;
    char tfm_header[] = "Created by hbf2gf";

    file_number--; /* for \mf-like mode */@#

    if(mf_like)
       {if(unicode)
            sprintf(output_file, "%s%02x.pl", output_name, file_number);
        else
            sprintf(output_file, "%s%02i.pl", output_name, file_number);
       }
    else
        sprintf(output_file, "%s.pl", output_name);@#

    if(!(out = fopen(output_file, WRITE_TXT)))
       {fprintf(stderr, "Couldn't open `%s'\n", output_file);
        exit(1);
       }
    if(!quiet)
        printf("\nWriting `%s'\n", output_file);@#

    fprintf(out,@/
            "\n(FAMILY %s%d)"@/
            "\n(CODINGSCHEME CJK-%s)",
            output_name, file_number, font_encoding);@#

    fprintf(out,@/
            "\n(DESIGNSIZE R %.6f)"@/
            "\n(COMMENT DESIGNSIZE IS IN POINTS)"@/
            "\n(COMMENT OTHER SIZES ARE MULTIPLES OF DESIGNSIZE)"@/
            "\n(CHECKSUM O %lo)"@/
            "\n(FONTDIMEN"@/
            "\n   (SLANT R %.6f)"@/
            "\n   (SPACE R 0.0)"@/
            "\n   (STRETCH R 0.0)"@/
            "\n   (SHRINK R 0.0)"@/
            "\n   (XHEIGHT R 1.0)"@/
            "\n   (QUAD R 1.0)"@/
            "\n   (EXTRASPACE R 0.0)"@/
            "\n   )", design_size, checksum, slant);@#

    s = tfm_header;
    i = strlen(s);
    t = ((long)i) << 24;
    sc = 16;
    pos = 18;@#

    fprintf(out, "\n");
    while(i > 0)
       {t |= ((long)(*(unsigned char *)s++)) << sc;
        sc -= 8;
        if(sc < 0)
           {fprintf(out, "\n(HEADER D %d O %lo)", pos, t);
            t = 0;
            sc = 24;
            pos++;
           }
        i--;
       }
    if(t)
        fprintf(out, "\n(HEADER D %d O %lo)", pos, t);
    fprintf(out, "\n");@#

    for(i = 0; i < 256; i++)
       {fprintf(out,@/
                "\n(CHARACTER O %o"@/
                "\n   (CHARWD R %.6f)"@/
                "\n   (CHARHT R %.6f)"@/
                "\n   (CHARDP R %.6f)"@/
                "\n   (CHARIC R %.6f)"@/
                "\n   )",@/
                i,
                tfm_output_size_x + 2 * tfm_offset_x,
                tfm_output_size_y + tfm_offset_y,
                -tfm_offset_y,
                slant * (tfm_output_size_y + tfm_offset_y));
       }@#

    fclose(out);
   }



@* The extended virtual font file for \Om.
The following is very similar to |write_pl()|; we simply map the glyphs
of the subfonts back to the original encoding positions.

@<Prototypes@>=
static void write_ovp(void);


@
@c
static void write_ovp(void)
   {int c, i, nmb_subfonts, remainder, count, pos;
    char output_file[FILE_NAME_LENGTH + 1];
    long t, sc;
    char *s;
    char ofm_header[] = "Created by hbf2gf";

    nmb_subfonts = ((max_char - (min_char & 0xFF00)) / 256 * nmb_2_bytes)
                     / 256 + 1;
    remainder = ((max_char - (min_char & 0xFF00)) / 256 * nmb_2_bytes)
                  % 256;@#

    /* correction for the last incomplete second byte range */
    for(count = 0; count < (max_char & 0xFF); count++)
        if(b2_codes[count])
            remainder++;
    if(remainder >= 256)
        nmb_subfonts++;@#

    sprintf(output_file, "%s.ovp", output_name);@#

    if(!(out = fopen(output_file, WRITE_TXT)))
       {fprintf(stderr, "Couldn't open `%s'\n", output_file);
        exit(1);
       }
    if(!quiet)
        printf("\nWriting `%s'\n", output_file);@#

    fprintf(out,@/
            "\n(VTITLE Omega virtual font created by hbf2gf)"@/
            "\n(DESIGNSIZE R %.6f)"@/
            "\n(COMMENT DESIGNSIZE IS IN POINTS)"@/
            "\n(COMMENT OTHER SIZES ARE MULTIPLES OF DESIGNSIZE)"@/
            "\n(CHECKSUM O %lo)"@/
            "\n(FONTDIMEN"@/
            "\n   (SLANT R %.6f)"@/
            "\n   (SPACE R 0.0)"@/
            "\n   (STRETCH R 0.0)"@/
            "\n   (SHRINK R 0.0)"@/
            "\n   (XHEIGHT R 1.0)"@/
            "\n   (QUAD R 1.0)"@/
            "\n   (EXTRASPACE R 0.0)"@/
            "\n   )", design_size, checksum, slant);@#

    s = ofm_header;
    i = strlen(s);
    t = ((long)i) << 24;
    sc = 16;
    pos = 18;@#

    fprintf(out, "\n");
    while(i > 0)
       {t |= ((long)(*(unsigned char *)s++)) << sc;
        sc -= 8;
        if(sc < 0)
           {fprintf(out, "\n(HEADER D %d O %lo)", pos, t);
            t = 0;
            sc = 24;
            pos++;
           }
        i--;
       }
    if(t)
        fprintf(out, "\n(HEADER D %d O %lo)", pos, t);
    fprintf(out, "\n");@#

    for(i = 0; i < nmb_subfonts; i++)
       {fprintf(out,@/
                "\n(MAPFONT D %i"@/
                "\n   (FONTNAME %s%02i)"@/
                "\n   (FONTCHECKSUM O %lo)"@/
                "\n   (FONTAT R 1.0)"@/
                "\n   (FONTDSIZE R %.6f)"@/
                "\n   )", i, output_name, i + 1, checksum, design_size);
       }@#

    for(c = min_char, i = 0, count = 0; c <= max_char; c++)
       {if(b2_codes[c & 0xFF] == VALID_SUBCODE)
           {fprintf(out,@/
                    "\n(CHARACTER O %o"@/
                    "\n   (CHARWD R %.6f)"@/
                    "\n   (CHARHT R %.6f)"@/
                    "\n   (CHARDP R %.6f)"@/
                    "\n   (CHARIC R %.6f)"@/
                    "\n   (MAP"@/
                    "\n      (SELECTFONT D %i)"@/
                    "\n      (SETCHAR O %o)"@/
                    "\n      )"@/
                    "\n   )",@/
                    c,
                    tfm_output_size_x + 2 * tfm_offset_x,
                    tfm_output_size_y + tfm_offset_y,
                    -tfm_offset_y,
                    slant * (tfm_output_size_y + tfm_offset_y),
                    i,
                    count);@#

            count++;
            if(count == 256)
               {count = 0;
                i++;
               }
           }
        else
            continue;
       }@#

    fclose(out);
   }



@* The job file.
This routine is the most system specific one. If your operating system needs
a different outline, make appropriate changes here.

You have to call this batch file after \.{hbf2gf} has finished (if not in
\mf-like mode). It will transform the \.{GF} files into \.{PK} files and
delete the now unnecessary \.{GF} files, then transform the \.{PL} file into
a \.{TFM} file and copy it |nmb_files| times. The name of the job file is
|output_name|.

@d EXTENSION_LENGTH 8 /* the maximal length of a file extension */@#

@d GFTOPK_NAME "gftopk"
@d PLTOTF_NAME "pltotf"
@d OVP2OVF_NAME "ovp2ovf"

@<Global...@>=
char job_extension[EXTENSION_LENGTH + 1];
char rm_command[STRING_LENGTH + 1];
char cp_command[STRING_LENGTH + 1];
char pk_directory[STRING_LENGTH + 1];
char tfm_directory[STRING_LENGTH + 1];@#

int ofm_file = FALSE;


@
@<Prototypes@>=
static void write_job(void);


@
@c
static void write_job(void)
   {FILE *out;
    int i, j;
    char buffer[FILE_NAME_LENGTH + 1];


    strcpy(buffer, output_name);
    strcat(buffer, job_extension);
    if(!(out = fopen(buffer, WRITE_TXT)))
       {fprintf(stderr, "Couldn't open `%s'\n", buffer);
        exit(1);
       }
    if(!quiet)
        printf("\nWriting `%s'\n", buffer);@#

    if(pk_files)
       {if(unicode)
           {for(i = (min_char >> 8), j = 0; j < nmb_files; i++, j++)
                fprintf(out,@/
                       "%s %s%02x.gf %s%s%02x.%.0ipk\n"@/
                       "%s %s%02x.gf\n",@/
                       GFTOPK_NAME, output_name, i,@/
                       pk_directory, output_name, i,
                        long_extension ? (int)(dpi_x * magstep_x + 0.5) : 0,@/
                       rm_command, output_name, i);
           }
        else
           {for(i = 1; i <= nmb_files; i++)
                fprintf(out,@/
                       "%s %s%02i.gf %s%s%02i.%.0ipk\n"@/
                       "%s %s%02i.gf\n",@/
                       GFTOPK_NAME, output_name, i,@/
                       pk_directory, output_name, i,
                        long_extension ? (int)(dpi_x * magstep_x + 0.5) : 0,@/
                       rm_command, output_name, i);
           }
       }@#

    if(tfm_files)
       {fprintf(out,@/
                "\n"@/
                "%s %s.pl %s.tfm\n"@/
                "%s %s.pl\n"@/
                "\n",@/
                PLTOTF_NAME, output_name, output_name,@/
                rm_command, output_name);@#

        if(unicode)
           {for(i = (min_char >> 8), j = 0; j < nmb_files; i++, j++)
                fprintf(out,@/
                        "%s %s.tfm %s%s%02x.tfm\n",@/
                        cp_command, output_name,
                        tfm_directory, output_name, i);
           }
        else
           {for(i = 1; i <= nmb_files; i++)
                fprintf(out,@/
                        "%s %s.tfm %s%s%02i.tfm\n",@/
                        cp_command, output_name,
                        tfm_directory, output_name, i);
           }@#

        fprintf(out,@/
                "\n"@/
                "%s %s.tfm",@/
                rm_command, output_name);
       }@#

    if(ofm_file)
       {fprintf(out,@/
                "\n"@/
                "%s %s.ovp %s.ovf %s.ofm\n"@/
                "%s %s.ovp\n"@/
                "\n",@/
                OVP2OVF_NAME, output_name, output_name, output_name,@/
                rm_command, output_name);
       }@#

    fclose(out);
   }



@* The configuration file.
Here is a list with all necessary keywords (and parameters):
\medskip
\halign{\quad\.{#}\hfil&\quad#\hfil\cr
        hbf\_header   & the HBF header file name of the input font(s). \cr
        output\_name  & the name stem of the output files. \cr
                      & Should be equal to the name of the configuration file
                        in most cases.\cr
                      & A running two digit decimal number starting with
                        \.{01} will be appended. \cr
                      & (For Unicode fonts see the keyword \.{unicode}
                        below.) \cr
       }
\bigskip
And now all optional keywords:
\medskip
\halign{\quad\.{#}\hfil&\quad#\hfil\cr
        x\_offset            & increases the character width. \cr
                             & Will be applied on both sides; \cr 
                             & default is the value given in the HBF header
                               (\.{HBF\_BITMAP\_BOUNDING\_BOX}) \cr
                             & scaled to \\{design\/}size (in pixels). \cr
        y\_offset            & shifts all characters up or down; \cr
                             & default is the value given in the HBF header
                               (\.{HBF\_BITMAP\_BOUNDING\_BOX}) \cr
                             & scaled to \\{design\/}size (in pixels). \cr
        design\_size         & the design size (in points) of the font. \cr
                             & \.{x\_offset} and \.{y\_offset} refer to this
                               size. \cr
                             & Default is |10.0| \cr
        target\_size         & This command is obsolete now and will be
                               ignored.\cr
                             & \cr
        slant                & the slant of the font (given as $\Delta x /
                               \Delta y$). \cr
                             & Only values in the range |0 <= slant <= 1|
                               are allowed. \cr
                             & Default is |0.0| \cr
        rotation             & if set to `yes'. the glyphs are rotated
                               90~degrees counter-clockwise. \cr
                             & The default offsets as given in the HBF header
                               will be ignored (and set to~|0|). \cr
                             & Default is `no'. \cr
                             & \cr
        mag\_x               & \cr
        mag\_y               & scaling values of the characters to reach
                               design size. \cr
                             & If only one magnification is given, x and y
                               values are assumed to be equal. \cr
                             & Default is \.{mag\_x} $=$ \.{mag\_y} $=$ |1.0|
                               \cr
        threshold            & A value between |1| and |254| defining a
                               threshold for converting the internal \cr
                             & graymap into the output bitmap; lower values
                               cut more pixels. \cr
                             & Default value is |128|. \cr
                             & \cr
        comment              & a comment describing the font; \cr
                             & default is none. \cr
                             & \cr
        nmb\_fonts           & the number of the fonts. \cr
                             & Default value is~|-1| for creating all
                               fonts. \cr
        unicode              & if `yes', a two digit hexadecimal number will
                               be used as a running number, \cr
                             & starting with the value of the first byte of
                               the first code range. \cr
                             & Default is `no'. \cr
        min\_char            & the minimum of the encoding range. \cr
                             & Specify this value if it is not identical to
                               the lowest code value \cr
                             & in the HBF file (to which it defaults). \cr
                             & \cr
        dpi\_x               & \cr
        dpi\_y               & the horizontal and vertical resolution (in
                               dpi) of the printer. \cr
                             & If only one resolution is given, x and y
                               values are assumed to be equal. \cr
                             & Default is |300|. \cr
        checksum             & a checksum to identify the \.{GF} files
                               with the appropriate \.{TFM} files. \cr
                             & The default of this 32~bit unsigned integer
                               is |0|. \cr
        coding               & a comment describing the coding scheme; \cr
                             & default is none. \cr
                             & \cr
        pk\_directory        & the destination directory of the
                               \.{PK} files; \cr
                             & default: none. \cr
                             & Attention! The batch file will not check
                               whether this directory exists. \cr
        tfm\_directory       & the destination directory of the
                               \.{TFM} files; \cr
                             & default: none. \cr
                             & Attention! The batch file will not check
                               whether this directory exists. \cr
        pk\_files            & whether to create \.{PK} files or not; \cr
                             & default is `yes'. \cr
        tfm\_files           & whether to create \.{TFM} files or not; \cr
                             & default is `yes'. \cr
        ofm\_file            & whether to create an \.{OFM} and an
                               \.{OVF} file or not; \cr
                             & default is `no'. \cr
        long\_extension      & if `yes', \.{PK} files will include the
                               resolution in the extension \cr
                             & (e.g. \.{gsso1201.300pk}). \cr
                             & This affects the batch file only (default is
                               `yes'). \cr
        rm\_command          & this shell command removes files; \cr
                             & default: `rm'. \cr
        cp\_command          & this shell command copies files; \cr
                             & default: `cp'. \cr
        job\_extension       & the extension of the batch file which calls
                               \.{GFtoPK} and \.{PLtoTF} \cr
                             & to convert the \.{GF} and the
                               \.{PL} files into \.{PK} and
                               \.{TFM} files; \cr
                             & default is none. \cr
       }
\bigskip

The searching algorithm (for the keywords) of \.{hbf2gf} is case
insensitive; it makes no difference whether you write for example \.{comment}
or \.{CommenT}. The keywords must start a line (be in the first column), and
the corresponding parameters must be on the same line with the keyword and
separated by at least one space or tabulator stop. Lines starting not with a
keyword are ignored.

Key values \\{are} case sensitive (except \.{yes} and \.{no}).

The default system dependent values are for \UNIX/-like operating systems;
if you use for example DOS, you must write
\medskip
\halign{\quad\.{#}\hfil&\quad\.{#}\hfil\cr
        long\_extension     & no  \cr
        rm\_command         & del  \cr
        cp\_command         & copy \cr
        job\_extension      & .bat \cr
       }
\medskip

Both the values |pk_output_size_x| and |pk_output_size_y| must not exceed
|MAX_CHAR_SIZE|; \.{x\_offset} and \.{y\_offset} are related to the
design size (and not to the input size).

In \mf-like mode, one \.{GF} file and one \.{PL} file will be computed
(depending on the command line options \.{-g} and \.{-p}), taking
|x_resolution| and |y_scale| from the command line. \.{nmb\_fonts} will
always be set to~|1|; no job file will be created.


@
@d PRINTER_MIN_RES_X    50
@d PRINTER_MIN_RES_Y    50

@<Global...@>=
char Buffer[STRING_LENGTH + 1];


@
@<Prototypes@>=
static void read_config(void);


@
If |config_file| isn't found in \mf-like mode we assume that the font
isn't a HBF font at all.

@c
static void read_config(void)
   {HBF_BBOX *boxp;
    char *real_config_file;


    @<Handle extension@>@;@#

    real_config_file = TeX_search_cfg_file(config_file);
    if(!real_config_file)
       {if(mf_like)
           {if(!quiet)
                printf("Couldn't find `%s'\n", config_file);
            exit(2);
           }
        else
           {fprintf(stderr, "Couldn't find `%s'\n", config_file);
            exit(1);
           }
       }@#

    if(!(config = fopen(real_config_file, READ_TXT)))
       {if(!testing)
           {fprintf(stderr, "Couldn't open `%s'\n", config_file);
            exit(1);
           }
        else /* We reach this point only if no searching library is used */
           {if(!quiet)
                fprintf(stderr, "Couldn't find or open `%s'\n", config_file);
            exit(2);
           }
       }@#

    if(testing)
       {if(!quiet)
            printf("%s\n", real_config_file);
        exit(0);
       }

    @<Necessary parameters@>@;
    @<Optional parameters@>@;@#

    @<Get code range@>@;
    @<Get sub code range@>@;@#

    fclose(config);
   }


@
Here we check whether we have to add an extension.

@<Handle extension@>=
       {int i, lastext = -1;

        for(i = 0; config_file[i]; i++)
            if(config_file[i] == '.')
                lastext = i;
            else if (config_file[i] == '/' ||
                     config_file[i] == ':' ||
                     config_file[i] == '\\')
                lastext = -1;@#

        if(lastext == -1)
            strcat(config_file, ".cfg");
       }


@
@<Necess...@>=
   {char hbf_header[STRING_LENGTH + 1];
    char *real_hbf_header;

    if(!fsearch("hbf_header"))
        config_error("hbf_header");
    else
        strcpy(hbf_header, Buffer);@#

    real_hbf_header = TeX_search_hbf_file(hbf_header);
    if(!real_hbf_header)
       {fprintf(stderr, "Couldn't find `%s'\n", hbf_header);
        exit(1);
       }@#

    hbfDebug = 1;            /* we activate error messages of the HBF API
                                       while scanning the HBF header file */@#

    if(!(hbf = hbfOpen(real_hbf_header)))
        exit(1);@#

    hbfDebug = 0;@#

    boxp = hbfBitmapBBox(hbf);
    input_size_x = boxp->hbf_height; /* will be checked later for rotation */
    input_size_y = boxp->hbf_width;
    font_encoding = hbfProperty(hbf, "HBF_CODE_SCHEME");@#

    if(!fsearch("output_name"))
        config_error("output_name");
    else
        strcpy(output_name, Buffer);
   }


@
@<Global...@>=
int offset_x;
int offset_y;@#

HBF_CHAR user_min_char;
int have_min_char = FALSE;

@
@<Opt...@>=
       {if(fsearch("nmb_files"))
            nmb_files = atoi(Buffer);
        if(fsearch("unicode"))
            if(Buffer[0] == 'y' || Buffer[0] == 'Y')
                unicode = TRUE;
        if(fsearch("min_char"))
           {user_min_char = strtoul(Buffer, (char **)NULL, 0);
            have_min_char = TRUE;
           }@#

        if(!mf_like)
           {if(fsearch("pk_files"))
                if(Buffer[0] == 'n' || Buffer[0] == 'N')
                    pk_files = FALSE;
            if(fsearch("tfm_files"))
                if(Buffer[0] == 'n' || Buffer[0] == 'N')
                    tfm_files = FALSE;
            if(fsearch("ofm_file"))
                if(Buffer[0] == 'y' || Buffer[0] == 'Y')
                    ofm_file = TRUE;
            if(fsearch("long_extension"))
                if(Buffer[0] == 'n' || Buffer[0] == 'N')
                    long_extension = FALSE;
           }@#

        if(fsearch("slant"))
            slant = atof(Buffer);
        if(slant < 0.0 || slant > 1.0)
           {fprintf(stderr, "Invalid slant\n");
            exit(1);
           }
        if(fsearch("rotation"))
            if(Buffer[0] == 'y' || Buffer[0] == 'Y')
                rotation = TRUE;@#

        if(fsearch("mag_x"))
            mag_x = atof(Buffer);
        if(fsearch("mag_y"))
            mag_y = atof(Buffer);
        if(!mag_x && !mag_y)
           {mag_x = 1.0;
            mag_y = 1.0;
           }
        if(mag_x && !mag_y)
            mag_y = mag_x;
        if(mag_y && !mag_x)
            mag_x = mag_y;
        if(mag_x <= 0.0)
           {fprintf(stderr, "Invalid horizontal magnification\n");
            exit(1);
           }
        if(mag_y <= 0.0)
           {fprintf(stderr, "Invalid vertical magnification\n");
            exit(1);
           }@#

        if(fsearch("dpi_x"))
            dpi_x = atoi(Buffer);
        if(fsearch("dpi_y"))
            dpi_y = atoi(Buffer);
        if(!dpi_x && !dpi_y)
           {dpi_x = 300;
            dpi_y = 300;
           }
        if(dpi_x && !dpi_y)
            dpi_y = dpi_x;
        if(dpi_y && !dpi_x)
            dpi_x = dpi_y;
        if(dpi_x <= PRINTER_MIN_RES_X)
           {fprintf(stderr, "Invalid horizontal printer resolution\n");
            exit(1);
           }
        if(dpi_y <= PRINTER_MIN_RES_Y)
           {fprintf(stderr, "Invalid vertical printer resolution\n");
            exit(1);
           }@#

        if(fsearch("design_size"))
            design_size = atof(Buffer);@#

        if(fsearch("x_offset"))
            offset_x = atoi(Buffer);
        else
            offset_x = rotation ? 0 : (boxp->hbf_xDisplacement * mag_x + 0.5);
        if(fsearch("y_offset"))
            offset_y = atoi(Buffer);
        else
            offset_y = rotation ? 0 : (boxp->hbf_yDisplacement * mag_y + 0.5);
        if(!fsearch("comment"))
            comment[0] = '\0';
        else
            strcpy(comment, Buffer);@#

        if(fsearch("threshold"))
            threshold = atoi(Buffer);
        if(threshold <= 0 || threshold >= 255)
           {fprintf(stderr, "Invalid threshold\n");
            exit(1);
           }@#

        if(!fsearch("checksum"))
            checksum = 0;
        else
            checksum = strtoul(Buffer, (char **)NULL, 0);@#

        if(!fsearch("coding"))
            coding[0] = '\0';
        else
            strcpy(coding, Buffer);@#

        if(!fsearch("pk_directory"))
            pk_directory[0] = '\0';
        else
            strcpy(pk_directory, Buffer);@#

        if(!fsearch("tfm_directory"))
            tfm_directory[0] = '\0';
        else
            strcpy(tfm_directory, Buffer);@#

        if(fsearch("rm_command"))
            strcpy(rm_command, Buffer);
        else
            strcpy(rm_command, "rm");@#

        if(fsearch("cp_command"))
            strcpy(cp_command, Buffer);
        else
            strcpy(cp_command, "cp");@#

        if(!fsearch("job_extension"))
            job_extension[0] = '\0';
        else
           {strncpy(job_extension, Buffer, EXTENSION_LENGTH);
            job_extension[EXTENSION_LENGTH] = '\0';
           }
       }


@
The function |hbfGetCodeRange()| is an extension to the HBF API.

Successive calls return the code ranges in ascending order; we only need the
extrema of the whole code range.

In case |min_char| has been supplied in the configuration file, we use that
value instead.

@<Global...@>=
HBF_CHAR min_char, max_char;


@
@<Get code range@>=
   {const void *cp;
    HBF_CHAR dummy;

    cp = hbfGetCodeRange(hbf, NULL, &min_char, &max_char);
    for(; cp != NULL; cp = hbfGetCodeRange(hbf, cp, &dummy, &max_char))
        ;@#

    if(have_min_char)
        min_char = user_min_char;
   }


@
The function |hbfGetByte2Range()| is an extension to the HBF API.

Successive calls return the byte~2 ranges in ascending order. We raise
|VALID_SUBCODE| in the array |b2_codes[]| for all characters in subcode
ranges.

@d VALID_SUBCODE    1

@<Global...@>=
char b2_codes[256];
unsigned char min_2_byte, max_2_byte;
int nmb_2_bytes = 0;


@
@<Get sub code range@>=
   {const void *b2r;
    unsigned char dummy;
    int i;

    for(i = 0; i < 256; i++)
        b2_codes[i] = 0;@#

    b2r = hbfGetByte2Range(hbf, NULL, &min_2_byte, &max_2_byte);
    dummy = min_2_byte;
    for(; b2r != NULL; b2r = hbfGetByte2Range(hbf, b2r, &dummy, &max_2_byte))
       {for(i = dummy; i <= max_2_byte; i++)
            b2_codes[i] = VALID_SUBCODE;
       }@#

    for(i = 0; i < 256; i++)    /* compute the number of valid |b2_codes| */
        if(b2_codes[i] == VALID_SUBCODE)
            nmb_2_bytes++;
   }


@
This search routine is case insignificant. Each keyword must start a line;
the function checks whether the character before the keyword is a newline
character (|'\n'|). It also checks the presence of a parameter and fills
|Buffer| if existent. |fsearch()| returns~1 on success.

@<Prototypes@>=
static int fsearch(const char *);


@
@c
static int fsearch(const char *search_string)
   {char *P, p;
    const char *Q;
    char temp_buffer[STRING_LENGTH + 1];
    char env_name[STRING_LENGTH + 1];
    char *env_p;
    char *env_value;
    char *Buf_p;
    int Ch, ch, old_ch = '\n';
    int count = STRING_LENGTH;

    rewind(config); /* we start at offset~0 */@#

    do
       {Q = search_string;
        p = tolower((unsigned char)*Q);
        Ch = fgetc(config);
        ch = tolower(Ch);
        while(!(ch == p && old_ch == '\n') && Ch != EOF)
                              /* search first character of |search_string|;
                                        |'\n'| must be the character before */
           {old_ch = ch;
            Ch = fgetc(config);
            ch = tolower(Ch);
           }@#

        for(;;)
           {if(*(++Q) == '\0')
                if((Ch = fgetc(config)) == ' ' || Ch == '\t')
                   /* there must be a space or a tab stop after the keyword */
                    goto success;
            Ch = fgetc(config);
            if(tolower(Ch) != tolower((unsigned char)*Q))
                break;
           }
       }
    while(Ch != EOF);@#

    return 0;@#

success:
    P = temp_buffer;@#

    while((Ch = fgetc(config)) == ' ' || Ch == '\t')
                                          /* remove leading blanks and tabs */
        ;
    while(Ch != '\n' && --count > 0 && Ch != EOF) /* fill |Buffer| */
       {*P++ = Ch;
        Ch = fgetc(config);
       }
    *P = '\0';@#

    if (*temp_buffer)
        @<Check for environment variables@>@;
    else
        *Buffer = '\0';@#

    return (*Buffer) ? 1 : 0; /* is there something in the buffer? */
   }


@
To make the configuration file more flexible we allow environment variables
in the arguments. We scan the parameter stored in |temp_buffer| whether it
contains a `\.{\$}' character. If yes, the following code fragment tries to
get an environment variable name whose value will be then fetched with
|getenv()|. An environment variable name recognized by \.{hbf2gf} must start
with a letter or underscore; the other characters may be alphanumeric or an
underscore. You can surround the environment variable name with braces to
indicate where the name ends, e.g. \.{\$\{FOO\}}. The interpolated
configuration parameter will be truncated to |STRING_LENGTH| characters. If
you want to have `\.{\$}' you must write `\.{\$\$}'.

Note that you should avoid to use such environment variables for specifying
the location of the configuration file in case you have support for a file
searching library like kpathsea. Its primary aim is to specify the target
directories for the \.{pk\_directory} and the \.{tfm\_directory} keywords.

@<Check for environment variables@>=
       {P = temp_buffer;
        Buf_p = Buffer;
        count = STRING_LENGTH - 1;@#

        while(*P && count > 0)
           {env_p = env_name;@#

            if(*P == '$')
               {P++;
                if(*P == '$')
                   {*(Buf_p++) = *(P++);
                    count--;
                    continue;
                   }
                while(*P == '{') @q } @>
                    P++;
                if(!(isalpha((unsigned char)*P) || *P == '_'))
                   {fprintf(stderr,@/
                "Invalid environment variable name in configuration file\n");
                    exit(1);
                   }
                *(env_p++) = *(P++);
                while(*P)
                   {if(isalnum((unsigned char)*P) || *P == '_')
                        *(env_p++) = *(P++);
                    else
                       {@q { @> while(*P == '}')
                            P++;
                        *env_p = '\0';
                        break;
                       }
                   }@#

                env_value = getenv(env_name);
                if(env_value) /* append the environment value to |Buffer| */
                   {while(*env_value && count > 0)
                       {*(Buf_p++) = *(env_value++);
                        count--;
                       }
                   }
               }
            else
               {*(Buf_p++) = *(P++);
                count--;
               }
           }
        *Buf_p = '\0';
       }


@
If an error occurs, |config_error()| will leave the program with an error
message.

@<Prototypes@>=
static void config_error(const char *);


@
@c
static void config_error(const char *message)
   {fprintf(stderr, "Couldn't find `%s' entry in configuration file\n",
            message);
    exit(1);
   }



@* File searching.

We support three searching engines: emtexdir, kpathsea, and MiKTeX (which is
a Win32 port of kpathsea). For emtexdir, define |HAVE_EMTEXDIR| while
compiling. For kpathsea, define |HAVE_LIBKPATHSEA|. For MikTeX, define
|HAVE_MIKTEX|. If none of these macros is defined, a simple |fopen()| will
be used instead.

@<Include files@>=
#if defined(HAVE_LIBKPATHSEA)
#include "kpathsea/kpathsea.h"

#elif defined(HAVE_EMTEXDIR)
#include "emtexdir.h"

#elif defined(HAVE_MIKTEX)
#include "miktex.h"
#endif

@
@<Global variables@>=
#if defined(HAVE_EMTEXDIR)
char emtex_version_string[] = "emTeXdir";
#elif !defined(HAVE_MIKTEX)
char no_version_string[] = "no search library";
#endif


@
@<Prototypes@>=
static const char *TeX_search_version(void);


@
@c
static const char *TeX_search_version(void)
   {
#if defined(HAVE_LIBKPATHSEA)
    return kpathsea_version_string;
#elif defined(HAVE_EMTEXDIR)
    return emtex_version_string;
#elif defined(HAVE_MIKTEX)
    char buf[200];

    strcpy(buf, "MiKTeX ");
    miktex_get_miktex_version_string_ex(buf + 7, sizeof (buf) - 7);

    return buf;
#else
    return no_version_string;
#endif
   }


@
@s emtex_dir int

@<Global variables@>=
#ifdef HAVE_EMTEXDIR
struct emtex_dir cfg_path, hbf_path;
#endif


@
@<Prototypes@>=
#ifdef HAVE_EMTEXDIR
extern int setup_list(struct emtex_dir *, char *, const char *, unsigned);
static int dir_setup(struct emtex_dir *, const char *, const char *, unsigned);
static char *file_find(char *, struct emtex_dir *);
#endif


@
We slightly modify |emtex_dir_setup()| (from the file \.{emtexdir.c}) to
output a warning in case the environment variable |env| isn't set properly.

@c
#ifdef HAVE_EMTEXDIR
static int dir_setup(ed, env, dir, flags)
  struct emtex_dir *ed;
  const char *env;
  const char *dir;
  unsigned flags;

   {const char *val;
    char path[260];

    ed->alloc = 0;
    ed->used = 0;
    ed->list = NULL;@#

    if(env != NULL && (val = getenv(env)) != NULL)
        return setup_list(ed, path, val, flags);
    else
        fprintf(stderr,
                "Environment variable `%s' not set; use current directory\n",
                env);@#

    return TRUE;
   }
#endif


@
@<Global variables@>=
#ifdef HAVE_EMTEXDIR
char name_buffer[FILE_NAME_LENGTH + 1];
#endif


@
@c
#ifdef HAVE_EMTEXDIR
static char *file_find(name, list)
  char *name;
  struct emtex_dir *list;

   {if(emtex_dir_find(name_buffer, sizeof(name_buffer), list, name, EDF_CWD))
        return name_buffer;@#

    return NULL;
   }
#endif


@
For emtexdir we use the environment variables \.{HBFCFG} and \.{HBFONTS} for
configuration resp.\ HBF header files.

@<Initialize \TeX\ file searching@>=
#if defined(HAVE_LIBKPATHSEA)
    kpse_set_program_name(argv[0], "hbf2gf");
    kpse_init_prog("HBF2GF", 300, "cx", "cmr10");@#

#elif defined(HAVE_EMTEXDIR)
    if(!dir_setup(&cfg_path, "HBFCFG", NULL, EDS_BANG))
       {fprintf(stderr,
                "Couldn't setup search path for configuration files\n");
        exit(1);
       }
    if(!dir_setup(&hbf_path, "HBFONTS", NULL, EDS_BANG))
       {fprintf(stderr,
                "Couldn't setup search path for HBF header files\n");
        exit(1);
       }
#endif


@
Finally, here are the searching routines. A special format in the kpathsea
library for fonts which are neither PostScript nor TrueType (\.{MISCFONTS})
is available with version~3.3 and newer. For older versions we use the path
for PostScript fonts (\.{T1FONTS}) to find HBF files. Configuration files
are searched in the path specified within \.{TEXCONFIG} for old kpathsea
versions, and within \.{HBF2GFINPUTS} for new versions.

@<Prototypes@>=
static char *TeX_search_cfg_file(char *);
static char *TeX_search_hbf_file(char *);


@
@c
#if defined(HAVE_LIBKPATHSEA)
static char *TeX_search_cfg_file(char *name)
   {
    return kpse_find_file(name, kpse_program_text_format, TRUE);
   }@#


static char *TeX_search_hbf_file(char *name)
   {
    return kpse_find_file(name, kpse_miscfonts_format, TRUE);
   }@#


#elif defined(HAVE_EMTEXDIR)
static char *TeX_search_cfg_file(char *name)
   {return file_find(name, &cfg_path);
   }@#


static char *TeX_search_hbf_file(char *name)
   {return file_find(name, &hbf_path);
   }@#


#elif defined(HAVE_MIKTEX)
static char *TeX_search_cfg_file(char *name)
   {char result[_MAX_PATH];

    if (!miktex_find_input_file("hbf2gf", *name, result))
      return 0;
    return strdup(result);
   }@#


static char *TeX_search_hbf_file(char *name)
   {char result[_MAX_PATH];


    if (!miktex_find_miscfont_file(*name, result))
      return 0;
    return strdup(result);
   }@#


#else
static char *TeX_search_cfg_file(char *name)
   {return name;
   }@#


static char *TeX_search_hbf_file(char *name)
   {return name;
   }
#endif



@* An example.
This is the example configuration file \.{b5so12.cfg} (for use with DOS or
OS/2 and the emtexdir searching engine):
\bigskip
\halign{\quad\.{#}\hfil&\quad\.{#}\hfil\cr
        hbf\_header          & et24.hbf \cr
        mag\_x               & 2.076 \cr
        x\_offset            & 3 \cr
        y\_offset            & -8 \cr
        comment              & fanti songti 24x24 pixel font scaled and
                               adapted to 12 pt \cr
                             & \cr
        design\_size         & 12.0 \cr
                             & \cr
        nmb\_fonts           & -1 \cr
                             & \cr
        output\_name         & b5so12 \cr
                             & \cr
        dpi\_x               & 300 \cr
        checksum             & 123456789 \cr
        coding               & codingscheme Big 5 encoded TeX text \cr
                             & \cr
        long\_extension      & no \cr
        job\_extension       & .cmd \cr
        rm\_command          & del \cr
        cp\_command          & copy \cr
        pk\_directory        & \$HBF\_TARGET\\pk\\360dpi\\ \cr
        tfm\_directory       & \$HBF\_TARGET\\tfm\\ \cr
       }
\bigskip
If you say e.g.
\medskip
\quad \.{set HBF\_TARGET=c:\\emtex\\texfonts}
\medskip
on your DOS prompt (or in your \.{autoexec.bat} file), then the interpolated
value of the \.{tfm\_directory} keyword is \.{c:\\emtex\\texfonts\\tfm\\}.
The HBF header file \.{et24.hbf} will be searched in the path specified by
the \.{HBFCFG} environment variable.
\bigskip
The call
\medskip
\quad \.{hbf2gf b5so12.cfg}
\medskip
creates the files
\medskip
\quad \.{b5so1201.gf}, \.{b5so1202.gf}, $\ldots$ , \.{b5so1255.gf},
\.{b5so12.pl}, and \.{b5so12.cmd}
\bigskip
After calling
\medskip
\quad \.{b5so12.cmd}
\medskip
you will find the \.{PK} files in the
\.{c:\\emtex\\texfonts\\pk\\360dpi} directory and the \.{TFM} files in
the \.{c:\\emtex\\texfonts\\tfm} directory; all \.{GF} files and
\.{b5so12.pl} will be deleted.
\bigskip
The call
\medskip
\quad \.{hbf2gf -n b5so1220 417}
\medskip
creates two files:
\medskip
\quad \.{b5so1220.gf} and \.{b5so1220.pl}
\medskip
using the configuration file \.{b5so12.cfg}. The \.{GF} file would be
named \.{b5so1220.417gf} if the flag \.{-n} had not been used.

\medskip

It is possible to convert bitmap fonts to \.{PK} files almost
automatically. The HBF header file already has the entry
\.{HBF\_BITMAP\_BOUNDING\_BOX} which defines vertical and horizontal offsets
(in pixels), but these values are not in all cases optimal. If you omit
\.{x\_offset} and \.{y\_offset} in the configuration file, the third and
fourth parameter of \.{HBF\_BITMAP\_BOUNDING\_BOX} is used, scaled to design
size (to say it in other words: \.{x\_offset} and \.{y\_offset} will always
apply to the design size to be synchronous with the \.{TFM} files).

Don't confuse scaling and magnification: Scaling here means that you choose
a (arbitrary) design size and compute scaling values (\.{mag\_x} and
\.{mag\_y}) which scales the bitmap to this particular design size at a
certain (arbitrarily chosen) resolution (\.{dpi\_x} and \.{dpi\_y}).
Magnification means that the scaled bitmap will be then magnified to a
certain target size while still using the font parameters (i.e., the
\.{TFM} file) of the design size.

In the sample, you have a $24 \times 24$ bitmap font which will be scaled to
$12\,\hbox{pt}$ having a resolution of $300\,\hbox{dpi}$:
\medskip
\quad $1\,\hbox{pt}$ are $300 / 72.27 = 4.1511$ pixel;

\quad $12\,\hbox{pt}$ are $4.1511 * 12 = 49.813$ pixel;

\quad thus the theoretical scaling value is $49.813 / 24 = 2.076$.

\medskip

But especially for small sizes, this may not be the best value if the font
should harmonize with, say, Knuth's Computer Modern fonts. I recommend to
compute, say, 5~\.{PK} fonts, then check the CJK font with different \TeX\
fonts to see whether the offsets and/or the scaling value is good. The
greater the design size the finer you can control the offsets---as an
example you could use a design size of $30\,\hbox{pt}$ (nevertheless there
is a compile-time constant |MAX_CHAR_SIZE| which limits the maximal
character size; default is 255~pixels).

If you have found optimal offsets, you can produce many different
magnifications of the CJK font using the same set of \.{TFM} files
analogous to ordinary \TeX\ fonts; as a simplification, we assume that
\.{PK} files with a resolution of $300\,\hbox{dpi}$ and a design size of
$10\,\hbox{pt}$ have the extension `\.{.300pk}' (respectively come into a
`\.{300dpi}' subdirectory)---this is the reason why in the above example for
the $12\,\hbox{pt}$ design size a `\.{360dpi}' target directory has been
used. Now we can use the following formula:

    $$ \\{needed\_dpi} = \\{your\_horizontal\_resolution} * 
                          {\\{your\_target\_size} \over 10.0} $$

Example: assuming that your printer has a resolution of
$300\times400\,\hbox{dpi}$, and you want $14.4\,\hbox{pt}$:

    $$ 300 * {14.4 \over 10.0} = 432 $$

The vertical scaling value is $400 / 300 = 1.3333$. Use these values now to
call \.{hbf2gf} in \mf-like mode:

\quad \.{hbf2gf b5so1220 432 1.3333}



@* Index.
