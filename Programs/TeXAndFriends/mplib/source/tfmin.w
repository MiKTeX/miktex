% Copyright 2008-2009 Taco Hoekwater.
%
% This program is free software: you can redistribute it and/or modify
% it under the terms of the GNU Lesser General Public License as published by
% the Free Software Foundation, either version 3 of the License, or
% (at your option) any later version.
%
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU Lesser General Public License for more details.
%
% You should have received a copy of the GNU Lesser General Public License
% along with this program.  If not, see <http://www.gnu.org/licenses/>.
%
% TeX is a trademark of the American Mathematical Society.
% METAFONT is a trademark of Addison-Wesley Publishing Company.
% PostScript is a trademark of Adobe Systems Incorporated.

% Here is TeX material that gets inserted after \input webmac

\font\tenlogo=logo10 % font used for the METAFONT logo
\font\logos=logosl10
\def\MF{{\tenlogo META}\-{\tenlogo FONT}}
\def\MP{{\tenlogo META}\-{\tenlogo POST}}

\def\title{Reading TEX metrics files}
\pdfoutput=1

@ Introduction.

@ Needed headers and macros

@d qi(A) (quarterword)(A) /* to store eight bits in a quarterword */
@d null_font 0 /* the |font_number| for an empty font */
@d false 0
@d true 1
@d hlp1(A) mp->help_line[0]=A; }
@d hlp2(A,B) mp->help_line[1]=A; hlp1(B)
@d hlp3(A,B,C) mp->help_line[2]=A; hlp2(B,C)
@d help3  { mp->help_ptr=3; hlp3 /* use this with three help lines */

@c 
#include <w2c/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mplib.h"
#include "mpmp.h" /* internal header */
#include "mpmath.h" /* internal header */
#include "mpstrings.h" /* internal header */
@<Declarations@>;
@h

@ The |font_ps_name| for a built-in font should be what PostScript expects.
A preliminary name is obtained here from the \.{TFM} name as given in the
|fname| argument.  This gets updated later from an external table if necessary.

@<Declarations@>=
font_number mp_read_font_info (MP mp, char *fname);

@ @c
font_number mp_read_font_info (MP mp, char *fname) {
  boolean file_opened; /* has |tfm_infile| been opened? */
  font_number n; /* the number to return */
  halfword lf,tfm_lh,bc,ec,nw,nh,nd; /* subfile size parameters */
  size_t whd_size; /* words needed for heights, widths, and depths */
  int i,ii; /* |font_info| indices */
  int jj; /* counts bytes to be ignored */
  int z; /* used to compute the design size */
  int d; /* height, width, or depth as a fraction of design size times $2^{-8}$ */
  int h_and_d; /* height and depth indices being unpacked */
  int tfbyte = 0; /* a byte read from the file */
  n=null_font;
  @<Open |tfm_infile| for input@>;
  @<Read data from |tfm_infile|; if there is no room, say so and |goto done|;
    otherwise |goto bad_tfm| or |goto done| as appropriate@>;
BAD_TFM:
  @<Complain that the \.{TFM} file is bad@>;
DONE:
  if ( file_opened ) (mp->close_file)(mp,mp->tfm_infile);
  if ( n!=null_font ) { 
    mp->font_ps_name[n]=mp_xstrdup(mp,fname);
    mp->font_name[n]=mp_xstrdup(mp,fname);
  }
  return n;
}

@ \MP\ doesn't bother to check the entire \.{TFM} file for errors or explain
precisely what is wrong if it does find a problem.  Programs called \.{TFtoPL}
@.TFtoPL@> @.PLtoTF@>
and \.{PLtoTF} can be used to debug \.{TFM} files.

@<Complain that the \.{TFM} file is bad@>=
{
   char msg[256];
   const char *hlp[] = {
     "I wasn't able to read the size data for this font so this",
     "`infont' operation won't produce anything. If the font name",
     "is right, you might ask an expert to make a TFM file",
     NULL };
   if ( file_opened )
     hlp[2]="is right, try asking an expert to fix the TFM file";
   mp_snprintf(msg, 256, "Font %s not usable: TFM file %s", fname,
              ( file_opened ? "is bad" : "not found"));
   mp_error(mp, msg, hlp, true);
}

@ @<Read data from |tfm_infile|; if there is no room, say so...@>=
@<Read the \.{TFM} size fields@>;
@<Use the size fields to allocate space in |font_info|@>;
@<Read the \.{TFM} header@>;
@<Read the character data and the width, height, and depth tables and
  |goto done|@>

@ A bad \.{TFM} file can be shorter than it claims to be.  The code given here
might try to read past the end of the file if this happens.  Changes will be
needed if it causes a system error to refer to |tfm_infile^| or call
|get_tfm_infile| when |eof(tfm_infile)| is true.  For example, the definition
@^system dependencies@>
of |tfget| could be changed to
``|begin get(tfm_infile); if eof(tfm_infile) then goto bad_tfm; end|.''

@d tfget do { 
  size_t wanted=1;
  unsigned char abyte=0;
  void *tfbyte_ptr = &abyte;
  (mp->read_binary_file)(mp,mp->tfm_infile, &tfbyte_ptr,&wanted); 
  if (wanted==0) goto BAD_TFM; 
  tfbyte = (int)abyte;
} while (0)
@d read_two(A) { (A)=tfbyte;
  if ( (A)>127 ) goto BAD_TFM;
  tfget; (A)=(A)*0400+tfbyte;
}
@d tf_ignore(A) { for (jj=(A);jj>=1;jj--) tfget; }

@<Read the \.{TFM} size fields@>=
tfget; read_two(lf);
tfget; read_two(tfm_lh);
tfget; read_two(bc);
tfget; read_two(ec);
if ( (bc>1+ec)||(ec>255) ) goto BAD_TFM;
tfget; read_two(nw);
tfget; read_two(nh);
tfget; read_two(nd);
whd_size=(size_t)((ec+1-bc)+nw+nh+nd);
if ( lf<(int)(6+(size_t)tfm_lh+whd_size) ) goto BAD_TFM;
tf_ignore(10)

@ Offsets are added to |char_base[n]| and |width_base[n]| so that is not
necessary to apply the |so|  and |qo| macros when looking up the width of a
character in the string pool.  In order to ensure nonnegative |char_base|
values when |bc>0|, it may be necessary to reserve a few unused |font_info|
elements.

@<Use the size fields to allocate space in |font_info|@>=
if ( mp->next_fmem<(size_t)bc) 
  mp->next_fmem=(size_t)bc; /* ensure nonnegative |char_base| */
if (mp->last_fnum==mp->font_max)
  mp_reallocate_fonts(mp,(mp->font_max+(mp->font_max/4)));
while (mp->next_fmem+whd_size>=mp->font_mem_size) {
  size_t l = mp->font_mem_size+(mp->font_mem_size/4);
  font_data *font_info;
  font_info = mp_xmalloc (mp,(l+1),sizeof(font_data));
  memset (font_info,0,sizeof(font_data)*(l+1));
  memcpy (font_info,mp->font_info,sizeof(font_data)*(mp->font_mem_size+1));
  mp_xfree(mp->font_info);
  mp->font_info = font_info;
  mp->font_mem_size = l;
}
mp->last_fnum++;
n=mp->last_fnum;
mp->font_bc[n]=(eight_bits)bc;
mp->font_ec[n]=(eight_bits)ec;
mp->char_base[n]=(int)(mp->next_fmem-(size_t)bc);
mp->width_base[n]=(int)(mp->next_fmem+(size_t)(ec-bc)+1);
mp->height_base[n]=mp->width_base[n]+nw;
mp->depth_base[n]=mp->height_base[n]+nh;
mp->next_fmem=mp->next_fmem+whd_size;


@ This macro is a bit odd, but it works.

@d integer_as_fraction(A) (int)(A)

@<Read the \.{TFM} header@>=
if ( tfm_lh<2 ) goto BAD_TFM;
tf_ignore(4);
tfget; read_two(z);
tfget; z=z*0400+tfbyte;
tfget; z=z*0400+tfbyte; /* now |z| is 16 times the design size */
mp->font_dsize[n]=mp_take_fraction(mp, z,integer_as_fraction(267432584));
  /* times ${72\over72.27}2^{28}$ to convert from \TeX\ points */
tf_ignore(4*(tfm_lh-2))

@ @<Read the character data and the width, height, and depth tables...@>=
ii=mp->width_base[n];
i=mp->char_base[n]+bc;
while ( i<ii ) { 
  tfget; mp->font_info[i].qqqq.b0=qi(tfbyte);
  tfget; h_and_d=tfbyte;
  mp->font_info[i].qqqq.b1=qi(h_and_d / 16);
  mp->font_info[i].qqqq.b2=qi(h_and_d % 16);
  tfget; tfget;
  i++;
}
while ( i<(int)mp->next_fmem ) {
  @<Read a four byte dimension, scale it by the design size, store it in
    |font_info[i]|, and increment |i|@>;
}
goto DONE

@ The raw dimension read into |d| should have magnitude at most $2^{24}$ when
interpreted as an integer, and this includes a scale factor of $2^{20}$.  Thus
we can multiply it by sixteen and think of it as a |fraction| that has been
divided by sixteen.  This cancels the extra scale factor contained in
|font_dsize[n|.

@<Read a four byte dimension, scale it by the design size, store it in...@>=
{ 
tfget; d=tfbyte;
if ( d>=0200 ) d=d-0400;
tfget; d=d*0400+tfbyte;
tfget; d=d*0400+tfbyte;
tfget; d=d*0400+tfbyte;
mp->font_info[i].sc=mp_take_fraction(mp, d*16,integer_as_fraction(mp->font_dsize[n]));
i++;
}

@ This function does no longer use the file name parser, because |fname| is
a C string already.

@<Open |tfm_infile| for input@>=
file_opened=false;
mp_ptr_scan_file(mp, fname);
if ( strlen(mp->cur_area)==0 ) { mp_xfree(mp->cur_area); mp->cur_area=NULL; }
if ( strlen(mp->cur_ext)==0 )  { 
    mp_xfree(mp->cur_ext); 
    mp->cur_ext=mp_xstrdup(mp,".tfm"); 
}
mp_pack_file_name(mp, mp->cur_name,mp->cur_area,mp->cur_ext);
mp->tfm_infile = (mp->open_file)(mp, mp->name_of_file, "r",mp_filetype_metrics);
if ( !mp->tfm_infile  ) goto BAD_TFM;
file_opened=true








