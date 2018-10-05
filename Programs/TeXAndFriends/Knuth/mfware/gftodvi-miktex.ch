%% gftodvi-miktex.ch: WEB change file for GFtoDVI
%% 
%% Copyright (C) 1991-2016 Christian Schenk
%% 
%% This file is free software; you can redistribute it and/or modify it
%% under the terms of the GNU General Public License as published by the
%% Free Software Foundation; either version 2, or (at your option) any
%% later version.
%% 
%% This file is distributed in the hope that it will be useful, but
%% WITHOUT ANY WARRANTY; without even the implied warranty of
%% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
%% General Public License for more details.
%% 
%% You should have received a copy of the GNU General Public License
%% along with This file; if not, write to the Free Software Foundation,
%% 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

% _____________________________________________________________________________
%
% [1.3]
% _____________________________________________________________________________

@x
procedure initialize; {this procedure gets things started properly}
@y
@t\4@>@<Declare \MiKTeX\ procedures@>@/
@#
procedure initialize; {this procedure gets things started properly}
@z

@x
  begin print_ln(banner);@/
@y
  begin
  miktex_process_command_line_options;
  print(banner);
  miktex_print_miktex_banner(output);
  write_ln;@/
  term_in:=input;
@z

% _____________________________________________________________________________
%
% [1.8]
% _____________________________________________________________________________

@x
begin goto final_end;
@y
begin c4p_exit(1);
@z

% _____________________________________________________________________________
%
% [2.11]
% _____________________________________________________________________________

@x
@d text_char == char {the data type of characters in text files}
@d first_text_char=0 {ordinal number of the smallest element of |text_char|}
@d last_text_char=255 {ordinal number of the largest element of |text_char|}
@y
@d text_char == char {the data type of characters in text files}
@d first_text_char=-127 {ordinal number of the smallest element of |text_char|}
@d last_text_char=128 {ordinal number of the largest element of |text_char|}
@z

% _____________________________________________________________________________
%
% [7.47]
% _____________________________________________________________________________

@x
begin reset(gf_file,name_of_file);
@y
begin c4p_fopen(gf_file,name_of_file,c4p_rb_mode,true); reset (gf_file);
@z

@x
begin reset(tfm_file,name_of_file);
@y
begin if not miktex_open_tfm_file(tfm_file,name_of_file) then c4p_exit(1);
reset(tfm_file);
@z

@x
begin rewrite(dvi_file,name_of_file);
@y
begin c4p_fopen(dvi_file,name_of_file,c4p_wb_mode,true);
@z

% _____________________________________________________________________________
%
% [9.78]
% _____________________________________________________________________________

@x
@d home_font_area=max_keyword+16 {string number for system-dependent font area}
@y
@z

% _____________________________________________________________________________
%
% [10.87]
% _____________________________________________________________________________

@x
l:=9; init_str9("T")("e")("X")("f")("o")("n")("t")("s")(":")(home_font_area);@/
@y
do_nothing;
@z

% _____________________________________________________________________________
%
% [10.90]
% _____________________________________________________________________________

@x
else  begin if (c=">")or(c=":") then
@y
else  begin if (c="/") then
@z

% _____________________________________________________________________________
%
% [10.92]
% _____________________________________________________________________________

@x
for k:=name_length+1 to file_name_size do name_of_file[k]:=' ';
@y
for k:=name_length+1 to file_name_size do name_of_file[k]:=chr(0);
@z

% _____________________________________________________________________________
%
% [10.94]
% _____________________________________________________________________________

@x
begin loop@+begin print_nl('GF file name: '); input_ln;
@y
begin loop@+begin
  if not miktex_get_input_file_name then begin
    print_nl('GF file name: '); input_ln
  end;
@z

% _____________________________________________________________________________
%
% [10.97]
% _____________________________________________________________________________

@x
font_name[title_font]:=default_title_font;
font_name[label_font]:=default_label_font;
font_name[gray_font]:=default_gray_font;
font_name[slant_font]:=null_string;
font_name[logo_font]:=logo_font_name;
@y
@z

% _____________________________________________________________________________
%
% [10.98]
% _____________________________________________________________________________

@x
  begin if length(font_area[f])=0 then font_area[f]:=home_font_area;
@y
  begin font_area[f]:=null_string;
@z

@x
  if font_area[f]=home_font_area then font_area[f]:=null_string;
@y
  font_area[f]:=null_string;
@z

% _____________________________________________________________________________
%
% [11.107]
% _____________________________________________________________________________

@x
begin for k:=a to b do write(dvi_file,dvi_buf[k]);
@y
begin c4p_buf_write(dvi_file,c4p_ptr(dvi_buf[a]),b-a+1);
@z

% _____________________________________________________________________________
%
% [16.168]
% _____________________________________________________________________________

@x
@!over_col:scaled; {overflow labels start here}
@y
@!over_col:scaled; {overflow labels start here}
@!over_label_offset:scaled;
@z

% _____________________________________________________________________________
%
% [16.170]
% _____________________________________________________________________________

@x
over_col:=over_col+delta_x+10000000;
@y
over_col:=over_col+delta_x+over_label_offset;
@z

% _____________________________________________________________________________
%
% [18.219] The main program
% _____________________________________________________________________________

@x
@p begin initialize; {get all variables initialized}
@<Initialize the strings@>;
@y
@p begin initialize; {get all variables initialized}
@<Initialize the strings@>;
font_name[title_font]:=miktex_get_title_font;
font_name[label_font]:=miktex_get_label_font;
font_name[gray_font]:=miktex_get_gray_font;
font_name[slant_font]:=miktex_get_slant_font;
font_name[logo_font]:=miktex_get_logo_font;
@z

% _____________________________________________________________________________
%
% [19.222] System-dependent changes
% _____________________________________________________________________________

@x
itself will get a new section number.
@^system dependencies@>
@y
itself will get a new section number.
@^system dependencies@>

@ @<Declare \MiKTeX\ procedures@>=

function miktex_get_input_file_name : boolean; forward;
function miktex_get_title_font : str_number; forward;
function miktex_get_label_font : str_number; forward;
function miktex_get_gray_font : str_number; forward;
function miktex_get_slant_font : str_number; forward;
function miktex_get_logo_font : str_number; forward;
@z
