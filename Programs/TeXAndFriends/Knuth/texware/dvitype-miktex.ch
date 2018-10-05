%% dvitype-miktex.ch: WEB change file for DVItype
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
% [1.2]
% _____________________________________________________________________________

@x
  begin print_ln(banner);@/
@y
  begin print(banner);
  miktex_print_miktex_banner(output);
  write_ln;@/
@z

% _____________________________________________________________________________
%
% [1.5]
% _____________________________________________________________________________

@x
@!name_length=50; {a file name shouldn't be longer than this}
@y
@!name_length=260;
@!namez_length=261;
@z

% _____________________________________________________________________________
%
% [1.7]
% _____________________________________________________________________________

@x
begin goto final_end;
@y
begin c4p_throw(final_end);
@z

% _____________________________________________________________________________
%
% [2.9]
% _____________________________________________________________________________

@x
@d text_char == char {the data type of characters in text files}
@d first_text_char=0 {ordinal number of the smallest element of |text_char|}
@d last_text_char=127 {ordinal number of the largest element of |text_char|}
@y
@d text_char == char {the data type of characters in text files}
@d first_text_char=0 {ordinal number of the smallest element of |text_char|}
@d last_text_char=127 {ordinal number of the largest element of |text_char|}
@z

% _____________________________________________________________________________
%
% [4.23]
% _____________________________________________________________________________

@x
begin reset(dvi_file);
@y
begin
if (c4pargc <> 2) then
 abort('Usage: dvitype inputfile');
c4p_fopen(dvi_file,c4p_argv[1],c4p_rb_mode,true);
reset(dvi_file);
@z

@x
begin reset(tfm_file,cur_name);
@y
begin miktex_open_tfm_file(tfm_file, cur_name);
@z

% _____________________________________________________________________________
%
% [4.24]
% _____________________________________________________________________________

@x
@!cur_name:packed array[1..name_length] of char; {external name,
@y
@!cur_name:packed array[1..namez_length] of char; {external name,
@z

% _____________________________________________________________________________
%
% [4.28]
% _____________________________________________________________________________

@x
begin set_pos(dvi_file,-1); dvi_length:=cur_pos(dvi_file);
@y
begin c4p_fseek(dvi_file,0,c4p_end); dvi_length:=c4p_ftell(dvi_file);
@z

@x
begin set_pos(dvi_file,n); cur_loc:=n;
@y
begin c4p_fseek(dvi_file,n,c4p_set); cur_loc:=n;
@z

% _____________________________________________________________________________
%
% [5.33]
% _____________________________________________________________________________

@x
@!tfm_conv:real; {\.{DVI} units per absolute \.{TFM} unit}
@y
@!tfm_conv:longreal;
@z

% _____________________________________________________________________________
%
% [5.39]
% _____________________________________________________________________________

@x
@!conv:real; {converts \.{DVI} units to pixels}
@!true_conv:real; {converts unmagnified \.{DVI} units to pixels}
@y
@!conv:longreal;
@!true_conv:longreal;
@z

% _____________________________________________________________________________
%
% [6.41] Optional modes of output
% _____________________________________________________________________________

@x
@!resolution:real; {pixels per inch}
@y
@!resolution:longreal;
@z

% _____________________________________________________________________________
%
% [6.46]
% _____________________________________________________________________________

@x
begin update_terminal; reset(term_in);
@y
begin update_terminal; get(term_in);
@z

% _____________________________________________________________________________
%
% [6.50]
% _____________________________________________________________________________

@x
begin rewrite(term_out); {prepare the terminal for output}
write_ln(term_out,banner);
@<Determine the desired |out_mode|@>;
@<Determine the desired |start_count| values@>;
@<Determine the desired |max_pages|@>;
@<Determine the desired |resolution|@>;
@<Determine the desired |new_mag|@>;
@y
begin term_out:=output; term_in:=input;
{|write_ln(term_out,banner);|}
if 0 then begin
  @<Determine the desired |out_mode|@>;
  @<Determine the desired |start_count| values@>;
  @<Determine the desired |max_pages|@>;
  @<Determine the desired |resolution|@>;
  @<Determine the desired |new_mag|@>;
end;
@z

% _____________________________________________________________________________
%
% [7.64]
% _____________________________________________________________________________

@x
@d default_directory_name=='TeXfonts:' {change this to the correct name}
@d default_directory_name_length=9 {change this to the correct length}
@y
@d default_directory_name==''
@d default_directory_name_length=0
@z

@x
@<Glob...@>=
@!default_directory:packed array[1..default_directory_name_length] of char;
@y
@z

% _____________________________________________________________________________
%
% [7.65]
% _____________________________________________________________________________

@x
default_directory:=default_directory_name;
@y
do_nothing;
@z

% _____________________________________________________________________________
%
% [7.66]
% _____________________________________________________________________________

@x
if p=0 then
  begin for k:=1 to default_directory_name_length do
    cur_name[k]:=default_directory[k];
  r:=default_directory_name_length;
  end
else r:=0;
@y
r:=0;
@z

@x
  if (names[k]>="a")and(names[k]<="z") then
      cur_name[r]:=xchr[names[k]-@'40]
  else cur_name[r]:=xchr[names[k]];
  end;
cur_name[r+1]:='.'; cur_name[r+2]:='T'; cur_name[r+3]:='F'; cur_name[r+4]:='M'
@y
  cur_name[r]:=xchr[names[k]];
  end;
cur_name[r+1]:='.'; cur_name[r+2]:='t'; cur_name[r+3]:='f'; cur_name[r+4]:='m';
cur_name[r+5]:=chr(0);
@z

% _____________________________________________________________________________
%
% [13.107] The main program
% _____________________________________________________________________________

@x
@p begin initialize; {get all variables initialized}
@y
@p begin
c4p_begin_try_block(final_end);
initialize; {get all variables initialized}
miktex_process_command_line_options;
@z

@x
final_end:end.
@y
final_end:
c4p_end_try_block(final_end);
end.
@z
