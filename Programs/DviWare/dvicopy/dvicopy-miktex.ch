%% dvicopy-miktex.ch:
%% 
%% Copyright (C) 1996-2016 Christian Schenk
%% 
%% This file is free software; you can redistribute it and/or modify
%% it under the terms of the GNU General Public License as published
%% by the Free Software Foundation; either version 2, or (at your
%% option) any later version.
%% 
%% This file is distributed in the hope that it will be useful, but
%% WITHOUT ANY WARRANTY; without even the implied warranty of
%% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
%% General Public License for more details.
%%
%% You should have received a copy of the GNU General Public License
%% along with this file; if not, write to the Free Software
%% Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
%% USA.

% _____________________________________________________________________________
%
% [1.1] Introduction
% _____________________________________________________________________________

@x
@d comm_length=24 {length of |preamble_comment|}
@y
@d comm_length=24 {length of |preamble_comment|}
@d comm_length_plus_one=25
@z

% _____________________________________________________________________________
%
% [1.3]
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
@<Constants...@>=
@!max_fonts=100; {maximum number of distinct fonts}
@!max_chars=10000; {maximum number of different characters among all fonts}
@!max_widths=3000; {maximum number of different characters widths}
@!max_packets=5000; {maximum number of different characters packets;
  must be less than 65536}
@!max_bytes=30000; {maximum number of bytes for characters packets}
@!max_recursion=10; {\.{VF} files shouldn't recurse beyond this level}
@!stack_size=100; {\.{DVI} files shouldn't |push| beyond this depth}
@!terminal_line_length=150; {maximum number of characters input in a single
  line of input from the terminal}
@!name_length=50; {a file name shouldn't be longer than this}
@y
@<Constants...@>=
@!max_selections=max_select;
@!max_fonts=5000; {maximum number of distinct fonts}
@!max_chars=1300000; {maximum number of different characters among all fonts}
@!max_widths=1300000; {maximum number of different characters widths}
@!max_packets=65535; {maximum number of different characters packets;
  must be less than 65536}
@!max_bytes=1000000; {maximum number of bytes for characters packets}
@!max_recursion=100; {\.{VF} files shouldn't recurse beyond this level}
@!stack_size=100; {\.{DVI} files shouldn't |push| beyond this depth}
@!terminal_line_length=150; {maximum number of characters input in a single
  line of input from the terminal}
@!name_length=259; {a file name shouldn't be longer than this}
@!name_length_plus_one=260; {one more for the string terminator}
@z

% _____________________________________________________________________________
%
% [4.23]
% _____________________________________________________________________________

@x
@d abort(#)==begin print_ln(' ',#,'.'); jump_out;
    end
@y
@d abort(#)==begin print_ln(' ',#,'.'); jump_out;
    end
@d goto_final_end==c4p_throw(final_end)
@z

@x
goto final_end;
@y
goto_final_end;
@z

% _____________________________________________________________________________
%
% [6.62]
% _____________________________________________________________________________

@x
@!cur_name:packed array[1..name_length] of char; {external name,
@y
@!cur_name:packed array[1..name_length_plus_one] of char; {external name,
@z

% _____________________________________________________________________________
%
% [6.63]
% _____________________________________________________________________________

@x
@d make_font_name_end(#)==
  append_to_name(#[l]); make_name
@d make_font_name(#)==
  l_cur_name:=0; for l:=1 to # do make_font_name_end
@y
@d make_font_name==l_cur_name:=0; make_name
@z

% _____________________________________________________________________________
%
% [6.67]
% _____________________________________________________________________________

@x
  if (b>="a")and(b<="z") then Decr(b)(("a"-"A")); {convert to upper case}
@y
@z

@x
  begin incr(l_cur_name); cur_name[l_cur_name]:=' ';
  end;
@y
  begin incr(l_cur_name); cur_name[l_cur_name]:=chr(0);
  end;
cur_name[name_length]:=chr(0);
@z

% _____________________________________________________________________________
%
% [8.91]
% _____________________________________________________________________________

@x
id4(".")("T")("F")("M")(tfm_ext); {file name extension for \.{TFM} files}
@y
id4(".")("t")("f")("m")(tfm_ext); {file name extension for \.{TFM} files}
@z

% _____________________________________________________________________________
%
% [8.92]
% _____________________________________________________________________________

@x
@d TFM_default_area_name=='TeXfonts:' {change this to the correct name}
@d TFM_default_area_name_length=9 {change this to the correct length}
@y
@d TFM_default_area_name==''
@d TFM_default_area_name_length=0
@z

@x
@<Glob...@>=
@!TFM_default_area:packed array[1..TFM_default_area_name_length] of char;
@y
@z

% _____________________________________________________________________________
%
% [8.93]
% _____________________________________________________________________________

@x
@ @<Set init...@>=
TFM_default_area:=TFM_default_area_name;
@y
@ @<Set init...@>=
do_nothing;
@z

% _____________________________________________________________________________
%
% [8.95]
% _____________________________________________________________________________

@x
make_font_name(TFM_default_area_name_length)(TFM_default_area)(tfm_ext);
reset(tfm_file,cur_name);
if eof(tfm_file) then
@^system dependencies@>
  abort('---not loaded, TFM file can''t be opened!')
@y
make_font_name(tfm_ext);
if not miktex_open_tfm_file (tfm_file, cur_name) then
  abort('---not loaded, TFM file can''t be opened!')
@z

% _____________________________________________________________________________
%
% [9.110]
% _____________________________________________________________________________

@x
@<Open input file(s)@>=
reset(dvi_file); {prepares to read packed bytes from |dvi_file|}
dvi_loc:=0;
@y
@<Open input file(s)@>=
if (c4pargc <> 3) then
 abort('Usage: dvicopy inputfile outfile');
c4p_fopen(dvi_file,c4p_argv[1],c4p_rb_mode,true);
reset(dvi_file);
cur_loc:=0;
@z

% _____________________________________________________________________________
%
% [9.112]
% _____________________________________________________________________________

@x
begin set_pos(dvi_file,-1); dvi_length:=cur_pos(dvi_file);
@y
begin c4p_fseek(dvi_file,0,c4p_end); dvi_length:=c4p_ftell(dvi_file);
@z

@x
begin set_pos(dvi_file,n); dvi_loc:=n;
@y
begin c4p_fseek(dvi_file,n,c4p_set); dvi_loc:=n;
@z

% _____________________________________________________________________________
%
% [10.135]
% _____________________________________________________________________________

@x
id3(".")("V")("F")(vf_ext); {file name extension for \.{VF} files}
@y
id3(".")("v")("f")(vf_ext); {file name extension for \.{VF} files}
@z

% _____________________________________________________________________________
%
% [10.137]
% _____________________________________________________________________________

@x
@d VF_default_area_name=='TeXvfonts:' {change this to the correct name}
@d VF_default_area_name_length=10 {change this to the correct length}
@y
@d VF_default_area_name=='' {change this to the correct name}
@d VF_default_area_name_length=0 {change this to the correct length}
@z

@x
@<Glob...@>=
@!VF_default_area:packed array[1..VF_default_area_name_length] of char;
@y
@z

% _____________________________________________________________________________
%
% [10.138]
% _____________________________________________________________________________

@x
@ @<Set init...@>=
VF_default_area:=VF_default_area_name;
@y
@ @<Set init...@>=
do_nothing;
@z

% _____________________________________________________________________________
%
% [10.139]
% _____________________________________________________________________________

@x
make_font_name(VF_default_area_name_length)(VF_default_area)(vf_ext);
reset(vf_file,cur_name);
if eof(vf_file) then
@y
make_font_name(vf_ext);
if not miktex_open_vf_file (vf_file,cur_name) then
@z

% _____________________________________________________________________________
%
% [11.163]
% _____________________________________________________________________________

@x
append_one(push);
vf_move[vf_ptr]:=vf_move[vf_ptr-1];
vf_push_loc[vf_ptr]:=byte_ptr;
vf_last_end[vf_ptr]:=byte_ptr;
vf_last[vf_ptr]:=vf_other
@y
append_one(push);
c4p_arrcpy (vf_move[vf_ptr], vf_move[vf_ptr-1]);
vf_last[vf_ptr]:=vf_other
@z

% _____________________________________________________________________________
%
% [11.170]
% _____________________________________________________________________________

@x
  vf_move[vf_ptr]:=vf_move[vf_ptr-1];
@y
  c4p_arrcpy (vf_move[vf_ptr], vf_move[vf_ptr-1]);
@z

% _____________________________________________________________________________
%
% [12.175] Terminal communication
% _____________________________________________________________________________

@x
n_opt:=0; {change this to indicate the presence of command line options}
k_opt:=0; {just in case}
@y
n_opt:=0;
k_opt:=0;
setup_options;
@z

% _____________________________________________________________________________
%
% [12.176]
% _____________________________________________________________________________

@x
begin if n_opt=0 then
  begin print('Enter option: '); update_terminal; reset(input);
  if eoln(input) then read_ln(input);
  k:=0; pckt_room(terminal_line_length);
  while (k<terminal_line_length)and not eoln(input) do
    begin append_byte(xord[input^]); incr(k); get(input);
    end;
  end
else if k_opt<n_opt then
  begin incr(k_opt);
  {Copy command line option number |k_opt| into |byte_mem| array!}
  end;
end;
@y
begin
  if k_opt<n_opt then begin
    k:=0; pckt_room(c4pstrlen(options[k_opt]));
    while (options[k_opt][k] <> 0) do begin
      append_byte(xord[options[k_opt][k]]); incr(k);
    end;
    incr(k_opt);
  end;
end;
@z

% _____________________________________________________________________________
%
% [12.177]
% _____________________________________________________________________________

@x
@!n_opt:int_16; {number of options found in command line}
@y
@!n_opt:int_16; {number of options found in command line}
@!options:array[0..max_select,0..99] of char;
@z

% _____________________________________________________________________________
%
% [16.241]
% _____________________________________________________________________________

@x
@p begin initialize; {get all variables initialized}
@y
@p begin
miktex_process_command_line_options;
c4p_begin_try_block(final_end);
initialize; {get all variables initialized}
@z

@x
final_end:end.
@y
final_end: c4p_end_try_block(final_end);
end.
@z

% _____________________________________________________________________________
%
% [17.246]
% _____________________________________________________________________________

@x
@<Open output file(s)@>=
rewrite(out_file); {prepares to write packed bytes to |out_file|}
@y
@<Open output file(s)@>=
if (c4pargc <> 3) then
 abort('Usage: dvicopy inputfile outputfile');
c4p_fopen(out_file,c4p_argv[2],c4p_wb_mode,true);
 {prepares to write packed bytes to |out_file|}
@z

% _____________________________________________________________________________
%
% [18.260]
% _____________________________________________________________________________

@x
@!comment:packed array[1..comm_length] of char; {preamble comment prefix}
@y
@!comment:packed array[1..comm_length_plus_one] of char; {preamble comment prefix}
@z

% _____________________________________________________________________________
%
% [18.261]
% _____________________________________________________________________________

@x
comment:=preamble_comment; pckt_room(comm_length);
@y
c4p_strcpy(comment, comm_length + 1, preamble_comment);
pckt_room(comm_length);
@z
