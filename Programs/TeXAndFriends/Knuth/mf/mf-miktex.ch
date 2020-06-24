%% mf-miktex.ch: WEB change file for METAFONT
%% 
%% Copyright (C) 1991-2020 Christian Schenk
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

%% ////////////////////////////////////////////////////////////////////////////
%% //                                                                        //
%% //                       INSPIRED BY WEB2C'S MF.CH                        //
%% //                                                                        //
%% ////////////////////////////////////////////////////////////////////////////

@x
\def\PASCAL{Pascal}
@y
\def\PASCAL{Pascal}
\def\C4P{C4P}
\def\Cplusplus{C++}
\def\MiKTeX{MiK\TeX}
@z

% WEAVE: print changes only.

@x
  \def\?##1]{\hbox to 1in{\hfil##1.\ }}
  }
@y 83
  \def\?##1]{\hbox{Changes to \hbox to 1em{\hfil##1}.\ }}
  }
\let\maybe=\iffalse
@z

% _____________________________________________________________________________
%
% [1.4]
% _____________________________________________________________________________

@x
procedure initialize; {this procedure gets things started properly}
@y
@t\4@>@<Declare \MiKTeX\ procedures@>@/
@#
procedure initialize; {this procedure gets things started properly}
@z

% _____________________________________________________________________________
%
% [1.6]
% _____________________________________________________________________________

@x
@d start_of_MF=1 {go here when \MF's variables are initialized}
@d end_of_MF=9998 {go here to close files and terminate gracefully}
@d final_end=9999 {this label marks the ending of the program}
@y
@d start_of_MF=1 {go here when \MF's variables are initialized}
@d end_of_MF=9998 {go here to close files and terminate gracefully}
@d final_end=9999 {this label marks the ending of the program}
@d goto_fin_end==c4p_throw(final_end)
@d goto_eof_MF==c4p_throw(end_of_MF)
@z

% _____________________________________________________________________________
%
% [1.6]
% _____________________________________________________________________________

@x
@d stat==@{ {change this to `$\\{stat}\equiv\null$' when gathering
  usage statistics}
@d tats==@t@>@} {change this to `$\\{tats}\equiv\null$' when gathering
  usage statistics}
@y
@d stat==
@d tats==
@z

% _____________________________________________________________________________
%
% [1.8]
% _____________________________________________________________________________

@x
the codewords `$|init|\ldots|tini|$'.

@d init== {change this to `$\\{init}\equiv\.{@@\{}$' in the production version}
@d tini== {change this to `$\\{tini}\equiv\.{@@\}}$' in the production version}
@y
the codewords `$|init|\ldots|tini|$' for declarations and by the codewords
`$|Init|\ldots|Tini|$' for executable code.  This distinction is helpful for
implementations where a run-time switch differentiates between the two
versions of the program.

@d init=={ifdef('INIMF')}
@d tini=={endif('INIMF')}
@d Init==init if miktex_is_init_program then begin
@d Tini==end;@+tini
@f Init==begin
@f Tini==end
@z

% _____________________________________________________________________________
%
% [1.11]
% _____________________________________________________________________________

@x
@!mem_max=30000; {greatest index in \MF's internal |mem| array;
  must be strictly less than |max_halfword|;
  must be equal to |mem_top| in \.{INIMF}, otherwise |>=mem_top|}
@y
@z

@x
@!max_internal=100; {maximum number of internal quantities}
@y
@!max_internal=300; {maximum number of internal quantities}
@z

@x
@!buf_size=500; {maximum number of characters simultaneously present in
  current lines of open files; must not exceed |max_halfword|}
@y
@!inf_buf_size=500;
@!sup_buf_size=30000000;
@z

@x
@!error_line=72; {width of context lines on terminal error messages}
@y
@!inf_error_line=40;
@!sup_error_line=65535;
@z

@x
@!half_error_line=42; {width of first lines of contexts in terminal
  error messages; should be between 30 and |error_line-15|}
@y
@!inf_half_error_line=30;
@!sup_half_error_line=65535;
@z

@x
@!max_print_line=79; {width of longest text lines output; should be at least 60}
@y
@!inf_max_print_line=40;
@!sup_max_print_line=65535;
@z

@x
@!screen_width=768; {number of pixels in each row of screen display}
@y
@!screen_width=768; {number of pixels in each row of screen display}
@z

@x
@!screen_depth=1024; {number of pixels in each column of screen display}
@y
@!screen_depth=1024; {number of pixels in each column of screen display}
@z

@x
@!stack_size=30; {maximum number of simultaneous input sources}
@y
@!inf_stack_size=30;
@!sup_stack_size=65535;
@z

@x
@!max_strings=2000; {maximum number of strings; must not exceed |max_halfword|}
@y
@!inf_max_strings=2000;
@!sup_max_strings=2097151;
@z

@x
@!string_vacancies=8000; {the minimum number of characters that should be
  available for the user's identifier names and strings,
  after \MF's own error messages are stored}
@y
@!inf_string_vacancies=8000;
@!sup_string_vacancies=40000000;
@z

@x
@!pool_size=32000; {maximum number of characters in strings, including all
  error messages and help texts, and the names of all identifiers;
  must exceed |string_vacancies| by the total
  length of \MF's own strings, which is currently about 22000}
@y
@!inf_pool_size=32000;
@!sup_pool_size=40000000;
@z

@x
@!move_size=5000; {space for storing moves in a single octant}
@y
@!inf_move_size=5000;
@!sup_move_size=200000;
@z

@x
@!max_wiggle=300; {number of autorounded points per cycle}
@y
@!inf_max_wiggle=300;
@!sup_max_wiggle=200000;
@z

@x
@!gf_buf_size=800; {size of the output buffer, must be a multiple of 8}
@y
@!gf_buf_size=8192; {size of the output buffer, must be a multiple of 8}
@z

@x
@!file_name_size=40; {file names shouldn't be longer than this}
@y
@!file_name_size=9999999; {file names shouldn't be longer than this}
@z

@x
@!path_size=300; {maximum number of knots between breakpoints of a path}
@y
@!inf_path_size=300;
@!sup_path_size=300000;
@z

@x
@!bistack_size=785; {size of stack for bisection algorithms;
  should probably be left at this value}
@y
@!inf_bistack_size=785;
@!sup_bistack_size=300000;
@z

@x
@!header_size=100; {maximum number of \.{TFM} header words, times~4}
@y
@!header_size=100; {maximum number of \.{TFM} header words, times~4}
@z

@x
@!lig_table_size=5000; {maximum number of ligature/kern steps, must be
  at least 255 and at most 32510}
@y
@!inf_lig_table_size=5000;
@!sup_lig_table_size=32510;
@z

@x
@!max_kerns=500; {maximum number of distinct kern amounts}
@y
@!max_kerns=2500; {maximum number of distinct kern amounts}
@z

@x
@!max_font_dimen=50; {maximum number of \&{fontdimen} parameters}
@y
@!max_font_dimen=60; {maximum number of \&{fontdimen} parameters}
@z

% _____________________________________________________________________________
%
% [1.12]
% _____________________________________________________________________________

@x
@d mem_top==30000 {largest index in the |mem| array dumped by \.{INIMF};
  must be substantially larger than |mem_min|
  and not greater than |mem_max|}
@y
@z

@x
@d hash_size=2100 {maximum number of symbolic tokens,
  must be less than |max_halfword-3*param_size|}
@y
@d hash_size=9500 {maximum number of symbolic tokens,
  must be less than |max_halfword-3*param_size|}
@z

@x
@d hash_prime=1777 {a prime number equal to about 85\pct! of |hash_size|}
@y
@d hash_prime=7919 {a prime number equal to about 85\pct! of |hash_size|}
@z

@x
@d max_in_open=6 {maximum number of input files and error insertions that
  can be going on simultaneously}
@y
@d max_in_open=20 {maximum number of input files and error insertions that
  can be going on simultaneously}
@z

@x
@d param_size=150 {maximum number of simultaneous macro parameters}
@y
@z

% _____________________________________________________________________________
%
% [2.20]
% _____________________________________________________________________________

@x
@!xord: array [text_char] of ASCII_code;
@y
@!xord: array [0..255] of ASCII_code;
@z

% _____________________________________________________________________________
%
% [2.22]
% _____________________________________________________________________________

@x
@^character set dependencies@>
@^system dependencies@>

@<Set init...@>=
for i:=0 to @'37 do xchr[i]:=' ';
for i:=@'177 to @'377 do xchr[i]:=' ';
@y
@^character set dependencies@>
@^system dependencies@>

@d tab = @'11 { ASCII horizontal tab }
@d form_feed = @'14 { ASCII form feed }

@<Set init...@>=
{Initialize |xchr| to the identity mapping.}
for i:=0 to @'37 do xchr[i]:=i;
for i:=@'177 to @'377 do xchr[i]:=i;
@z

% _____________________________________________________________________________
%
% [2.23]
% _____________________________________________________________________________

@x
for i:=first_text_char to last_text_char do xord[chr(i)]:=@'177;
for i:=@'200 to @'377 do xord[xchr[i]]:=i;
for i:=0 to @'176 do xord[xchr[i]]:=i;
@y
miktex_initialize_char_tables;
@z

% _____________________________________________________________________________
%
% [3.26]
% _____________________________________________________________________________

@x
|name_of_file| could be opened.
@y
|name_of_file| could be opened.

\MiKTeX: we use our own functions to open files.
@z

@x
begin reset(f,name_of_file,'/O'); a_open_in:=reset_OK(f);
@y
begin if not miktex_open_input_file(f) then a_open_in:=false
   else a_open_in:=true;
@z

@x
begin rewrite(f,name_of_file,'/O'); a_open_out:=rewrite_OK(f);
@y
begin a_open_out:=miktex_open_output_file(f, true);
@z

@x
begin rewrite(f,name_of_file,'/O'); b_open_out:=rewrite_OK(f);
@y
begin b_open_out:=miktex_open_output_file(f, false);
@z

@x
begin reset(f,name_of_file,'/O'); w_open_in:=reset_OK(f);
@y
begin w_open_in:=false
@z

@x
begin rewrite(f,name_of_file,'/O'); w_open_out:=rewrite_OK(f);
@y
begin w_open_out:=miktex_open_output_file(f, false);
@z

% _____________________________________________________________________________
%
% [3.27]
% _____________________________________________________________________________

@x
begin close(f);
@y
begin miktex_close_file(f);
@z

@x
begin close(f);
@y
begin miktex_close_file(f);
@z

@x
begin close(f);
@y
begin miktex_close_file(f);
@z

% _____________________________________________________________________________
%
% [3.29]
% _____________________________________________________________________________

@x
@!buffer:array[0..buf_size] of ASCII_code; {lines of characters being read}
@!first:0..buf_size; {the first unused position in |buffer|}
@!last:0..buf_size; {end of the line just input to |buffer|}
@!max_buf_stack:0..buf_size; {largest index used in |buffer|}
@y
@!buffer:array[0..sup_buf_size] of ASCII_code; {lines of characters being read}
@!first:0..sup_buf_size; {the first unused position in |buffer|}
@!last:0..sup_buf_size; {end of the line just input to |buffer|}
@!max_buf_stack:0..sup_buf_size; {largest index used in |buffer|}
@z

% _____________________________________________________________________________
%
% [3.30]
% _____________________________________________________________________________

@x
@p function input_ln(var @!f:alpha_file;@!bypass_eoln:boolean):boolean;
  {inputs the next line or returns |false|}
var @!last_nonblank:0..buf_size; {|last| with trailing blanks removed}
begin if bypass_eoln then if not eof(f) then get(f);
  {input the first character of the line into |f^|}
last:=first; {cf.\ Matthew 19\thinspace:\thinspace30}
if eof(f) then input_ln:=false
else  begin last_nonblank:=first;
  while not eoln(f) do
    begin if last>=max_buf_stack then
      begin max_buf_stack:=last+1;
      if max_buf_stack=buf_size then
        @<Report overflow of the input buffer, and abort@>;
      end;
    buffer[last]:=xord[f^]; get(f); incr(last);
    if buffer[last-1]<>" " then last_nonblank:=last;
    end;
  last:=last_nonblank; input_ln:=true;
  end;
end;
@y
@p function input_ln(var f:alpha_file;@!bypass_eoln:boolean):boolean;forward;
@z

% _____________________________________________________________________________
%
% [3.32]
% _____________________________________________________________________________

@x
@d t_open_in==reset(term_in,'TTY:','/O/I') {open the terminal for text input}
@d t_open_out==rewrite(term_out,'TTY:','/O')
@y
@d t_open_in==term_in:=i@&nput
@d t_open_out==term_out:=output
@z

% _____________________________________________________________________________
%
% [3.33]
% _____________________________________________________________________________

@x
@d clear_terminal == break_in(term_in,true) {clear the terminal input buffer}
@y
@d clear_terminal == break(term_in) {clear the terminal input buffer}
@z

% _____________________________________________________________________________
%
% [3.34]
% _____________________________________________________________________________

@x
  begin write_ln(term_out,'Buffer size exceeded!'); goto final_end;
@y
  begin write_ln('Buffer size exceeded!'); goto_fin_end;
@z

% _____________________________________________________________________________
%
% [3.36]
% _____________________________________________________________________________

@x
loop@+begin wake_up_terminal; write(term_out,'**'); update_terminal;
@y
loop@+begin
  if (c4p_plen > 0) then miktex_initialize_buffer
  else begin
    wake_up_terminal; write(term_out,'**'); update_terminal;
@z

@x
  loc:=first;
@y
    end;
  loc:=first;
@z

% _____________________________________________________________________________
%
% [4.37]
% _____________________________________________________________________________

@x
@!pool_pointer = 0..pool_size; {for variables that point into |str_pool|}
@!str_number = 0..max_strings; {for variables that point into |str_start|}
@y
@!pool_pointer = 0..sup_pool_size; {for variables that point into |str_pool|}
@!str_number = 0..sup_max_strings; {for variables that point into |str_start|}
@z

% _____________________________________________________________________________
%
% [5.54]
% _____________________________________________________________________________

@x
@!term_offset : 0..max_print_line;
  {the number of characters on the current terminal line}
@!file_offset : 0..max_print_line;
  {the number of characters on the current file line}
@!trick_buf:array[0..error_line] of ASCII_code; {circular buffer for
@y
@!term_offset : 0..sup_max_print_line;
  {the number of characters on the current terminal line}
@!file_offset : 0..sup_max_print_line;
  {the number of characters on the current file line}
@!trick_buf:array[0..sup_error_line] of ASCII_code; {circular buffer for
@z

% _____________________________________________________________________________
%
% [5.61]
% _____________________________________________________________________________

@x
wterm(banner);
if base_ident=0 then wterm_ln(' (no base preloaded)')
else  begin slow_print(base_ident); print_ln;
  end;
@y
if (not miktex_get_quiet_flag) then begin
  wterm(banner);
  miktex_print_miktex_banner(term_out);
  if base_ident=0 then print_ln {|wterm_ln(' (no base preloaded)')|}
  else  begin slow_print(base_ident); print_ln;
    end;
end;
@z

% _____________________________________________________________________________
%
% [5.66]
% _____________________________________________________________________________

@x
var @!k:0..buf_size; {index into |buffer|}
@y
var @!k:0..sup_buf_size; {index into |buffer|}
@z

% _____________________________________________________________________________
%
% [6.68]
% _____________________________________________________________________________

@x
  print_nl("! "); print(#);
@y
  if (miktex_c_style_error_messages_p and not terminal_input) then
    print_c_style_error_message (#)
  else begin print_nl("! "); print(#) end;
@z

% _____________________________________________________________________________
%
% [6.69]
% _____________________________________________________________________________

@x
@ @<Set init...@>=interaction:=error_stop_mode;
@y
@ @<Set init...@>=
if miktex_get_interaction >= 0 then
  interaction:=miktex_get_interaction
else
  interaction:=error_stop_mode;
@z

% _____________________________________________________________________________
%
% [6.76]
% _____________________________________________________________________________

@x
begin goto end_of_MF;
@y
begin goto_eof_MF;
@z

% _____________________________________________________________________________
%
% [6.77]
% _____________________________________________________________________________

@x
print_char("."); show_context;
@y
if not miktex_c_style_error_messages_p then begin
  print_char("."); show_context
end;
if (miktex_halt_on_error_p) then begin
  history:=fatal_error_stop; jump_out;
end;
@z

% _____________________________________________________________________________
%
% [6.79]
% _____________________________________________________________________________

@x
line ready to be edited. But such an extension requires some system
wizardry, so the present implementation simply types out the name of the
file that should be
edited and the relevant line number.
@^system dependencies@>

There is a secret `\.D' option available when the debugging routines haven't
been commented~out.
@^debugging@>
@y
line ready to be edited.
We do this by calling the external procedure |call_edit| with a pointer to
the filename, its length, and the line number.
However, here we just set up the variables that will be used as arguments,
since we don't want to do the switch-to-editor until after \MF\ has closed
its files.
@^system dependencies@>

There is a secret `\.D' option available when the debugging routines have
not been commented out.
@^debugging@>
@d edit_file==input_stack[file_ptr]
@z

@x
"E": if file_ptr>0 then
  begin print_nl("You want to edit file ");
@.You want to edit file x@>
  slow_print(input_stack[file_ptr].name_field);
  print(" at line "); print_int(line);@/
  interaction:=scroll_mode; jump_out;
@y
"E": if file_ptr>0 then
    begin
    edit_name_start:=str_start[edit_file.name_field];
    edit_name_length:=str_start[edit_file.name_field+1] -
                      str_start[edit_file.name_field];
    edit_line:=line;
    jump_out;
@z

% _____________________________________________________________________________
%
% [7.107]
% _____________________________________________________________________________

@x
@p function make_fraction(@!p,@!q:integer):fraction;
@y
@p function make_fraction_orig(@!p,@!q:integer):fraction;
@z

% _____________________________________________________________________________
%
% [7.109]
% _____________________________________________________________________________

@x
@p function take_fraction(@!q:integer;@!f:fraction):integer;
@y
@p function take_fraction_orig(@!q:integer;@!f:fraction):integer;
@z

% _____________________________________________________________________________
%
% [7.112]
% _____________________________________________________________________________

@x
@p function take_scaled(@!q:integer;@!f:scaled):integer;
@y
@p function take_scaled_orig(@!q:integer;@!f:scaled):integer;
@z

% _____________________________________________________________________________
%
% [7.114]
% _____________________________________________________________________________

@x
@p function make_scaled(@!p,@!q:integer):scaled;
@y
@p function make_scaled_orig(@!p,@!q:integer):scaled;
@z

% _____________________________________________________________________________
%
% [9.153]
% _____________________________________________________________________________

@x
@d min_quarterword=0 {smallest allowable value in a |quarterword|}
@d max_quarterword=255 {largest allowable value in a |quarterword|}
@d min_halfword==0 {smallest allowable value in a |halfword|}
@d max_halfword==65535 {largest allowable value in a |halfword|}
@y
@d min_quarterword=0 {smallest allowable value in a |quarterword|}
@d max_quarterword=255 {largest allowable value in a |quarterword|}
@d min_halfword==0 {smallest allowable value in a |halfword|}
@d max_halfword==@"FFFFFFF {largest allowable value in a |halfword|}
@z

% _____________________________________________________________________________
%
% [10.159]
% _____________________________________________________________________________

@x
@!mem : array[mem_min..mem_max] of memory_word; {the big dynamic storage area}
@y
@!mem : ^memory_word; {the big dynamic storage area}
@z

% _____________________________________________________________________________
%
% [11.178]
% _____________________________________________________________________________

@x
@!debug @!free: packed array [mem_min..mem_max] of boolean; {free cells}
@t\hskip1em@>@!was_free: packed array [mem_min..mem_max] of boolean;
@y
@!debug @!free: packed array [0..1] of boolean; {free cells}
@t\hskip1em@>@!was_free: packed array [0..1] of boolean;
@z

% _____________________________________________________________________________
%
% [12.194]
% _____________________________________________________________________________

@x
begin internal[time]:=12*60*unity; {minutes since midnight}
internal[day]:=4*unity; {fourth day of the month}
internal[month]:=7*unity; {seventh month of the year}
internal[year]:=1776*unity; {Anno Domini}
@y
begin internal[time]:=(c4p_hour*60+c4p_minute)*unity; {minutes since midnight}
internal[day]:=c4p_day*unity; {day of month}
internal[month]:=c4p_month*unity; {month of year}
internal[year]:=c4p_year*unity; {Anno Domini}
@z

% _____________________________________________________________________________
%
% [12.199]
% _____________________________________________________________________________

@x
for k:=127 to 255 do char_class[k]:=invalid_class;
@y
for k:=127 to 255 do char_class[k]:=invalid_class;
char_class[tab]:=space_class;
char_class[form_feed]:=space_class;
@z

% _____________________________________________________________________________
%
% [18.279]
% _____________________________________________________________________________

@x
@!delta_x,@!delta_y,@!delta:array[0..path_size] of scaled; {knot differences}
@!psi:array[1..path_size] of angle; {turning angles}
@y
@!delta_x:array[0..sup_path_size] of scaled; {knot differences}
@!delta_y:array[0..sup_path_size] of scaled; {knot differences}
@!delta:array[0..sup_path_size] of scaled; {knot differences}
@!psi:array[1..sup_path_size] of angle; {turning angles}
@z

% _____________________________________________________________________________
%
% [18.280]
% _____________________________________________________________________________

@x
@!k,@!n:0..path_size; {current and final knot numbers}
@y
@!k,@!n:0..sup_path_size; {current and final knot numbers}
@z

% _____________________________________________________________________________
%
% [18.283]
% _____________________________________________________________________________

@x
@!theta:array[0..path_size] of angle; {values of $\theta_k$}
@!uu:array[0..path_size] of fraction; {values of $u_k$}
@!vv:array[0..path_size] of angle; {values of $v_k$}
@!ww:array[0..path_size] of fraction; {values of $w_k$}
@y
@!theta:array[0..sup_path_size] of angle; {values of $\theta_k$}
@!uu:array[0..sup_path_size] of fraction; {values of $u_k$}
@!vv:array[0..sup_path_size] of angle; {values of $v_k$}
@!ww:array[0..sup_path_size] of fraction; {values of $w_k$}
@z

% _____________________________________________________________________________
%
% [18.284]
% _____________________________________________________________________________

@x
var @!k:0..path_size; {current knot number}
@y
var @!k:0..sup_path_size; {current knot number}
@z

% _____________________________________________________________________________
%
% [19.308]
% _____________________________________________________________________________

@x
@!move:array[0..move_size] of integer; {the recorded moves}
@!move_ptr:0..move_size; {the number of items in the |move| list}
@y
@!move:array[0..sup_move_size] of integer; {the recorded moves}
@!move_ptr:0..sup_move_size; {the number of items in the |move| list}
@z

@!move:array[0..move_size] of integer; {the recorded moves}
@!move_ptr:0..move_size; {the number of items in the |move| list}

% _____________________________________________________________________________
%
% [19.309]
% _____________________________________________________________________________

@x
@!bisect_stack:array[0..bistack_size] of integer;
@!bisect_ptr:0..bistack_size;
@y
@!bisect_stack:array[0..sup_bistack_size] of integer;
@!bisect_ptr:0..sup_bistack_size;
@z

% _____________________________________________________________________________
%
% [19.321]
% _____________________________________________________________________________

@x
var@!k:1..move_size; {index into |move|}
@y
var@!k:1..sup_move_size; {index into |move|}
@z

% _____________________________________________________________________________
%
% [20.357]
% _____________________________________________________________________________

@x
@!j,@!jj:0..move_size; {indices into |move|}
@y
@!j,@!jj:0..sup_move_size; {indices into |move|}
@z

% _____________________________________________________________________________
%
% [20.378]
% _____________________________________________________________________________

@x
var @!delta:0..move_size; {extent of |move| data}
@!k:0..move_size; {index into |move|}
@y
var @!delta:0..sup_move_size; {extent of |move| data}
@!k:0..sup_move_size; {index into |move|}
@z

% _____________________________________________________________________________
%
% [21.426]
% _____________________________________________________________________________

@x
var @!k:0..max_wiggle; {runs through the list of inputs}
@y
var @!k:0..sup_max_wiggle; {runs through the list of inputs}
@z

% _____________________________________________________________________________
%
% [21.427]
% _____________________________________________________________________________

@x
@!before,@!after:array[0..max_wiggle] of scaled; {data for |make_safe|}
@!node_to_round:array[0..max_wiggle] of pointer; {reference back to the path}
@!cur_rounding_ptr:0..max_wiggle; {how many are being used}
@!max_rounding_ptr:0..max_wiggle; {how many have been used}
@y
@!before:array[0..sup_max_wiggle] of scaled; {data for |make_safe|}
@!after:array[0..sup_max_wiggle] of scaled; {data for |make_safe|}
@!node_to_round:array[0..sup_max_wiggle] of pointer; {reference back to the path}
@!cur_rounding_ptr:0..sup_max_wiggle; {how many are being used}
@!max_rounding_ptr:0..sup_max_wiggle; {how many have been used}
@z

% _____________________________________________________________________________
%
% [21.440]
% _____________________________________________________________________________

@x
@!k:0..max_wiggle; {runs through before-and-after values}
@y
@!k:0..sup_max_wiggle; {runs through before-and-after values}
@z

% _____________________________________________________________________________
%
% [24.507]
% _____________________________________________________________________________

@x
@!env_move:array[0..move_size] of integer;
@y
@!env_move:array[0..sup_move_size] of integer;
@z

% _____________________________________________________________________________
%
% [24.511]
% _____________________________________________________________________________

@x
@!smooth_bot,@!smooth_top:0..move_size; {boundaries of smoothing}
@y
@!smooth_bot,@!smooth_top:0..sup_move_size; {boundaries of smoothing}
@z

% _____________________________________________________________________________
%
% [26.557]
% _____________________________________________________________________________

@x
@!uv,@!xy:0..bistack_size; {pointers to the current packets of interest}
@y
@!uv,@!xy:0..sup_bistack_size; {pointers to the current packets of interest}
@z

% _____________________________________________________________________________
%
% [27.564]
% _____________________________________________________________________________

@x
begin init_screen:=false;
@y
begin init_screen:=miktex_init_screen(screen_width, screen_depth);
@z

@x
begin @!init wlog_ln('Calling UPDATESCREEN');@+tini {for testing only}
@y
begin @!Init wlog_ln('Calling UPDATESCREEN');@+Tini {for testing only}
miktex_update_screen;
@z

% _____________________________________________________________________________
%
% [27.567]
% _____________________________________________________________________________

@x
var @!r:screen_row;
@!c:screen_col;
begin @{@+for r:=top_row to bot_row-1 do
  for c:=left_col to right_col-1 do
    screen_pixel[r,c]:=white;@+@}@/
@!init wlog_cr; {this will be done only after |init_screen=true|}
@y
begin
  miktex_blank_rectangle(left_col, right_col, top_row, bot_row);
@!Init wlog_cr; {this will be done only after |init_screen=true|}
wlog_ln('Calling BLANKRECTANGLE(',left_col:1,',',
  right_col:1,',',top_row:1,',',bot_row:1,')');@+Tini
@z

% _____________________________________________________________________________
%
% [27.568]
% _____________________________________________________________________________

@x
var @!k:screen_col; {an index into |a|}
@!c:screen_col; {an index into |screen_pixel|}
begin @{ k:=0; c:=a[0];
repeat incr(k);
  repeat screen_pixel[r,c]:=b; incr(c);
  until c=a[k];
  b:=black-b; {$|black|\swap|white|$}
  until k=n;@+@}@/
@!init wlog('Calling PAINTROW(',r:1,',',b:1,';');
  {this is done only after |init_screen=true|}
for k:=0 to n do
  begin wlog(a[k]:1); if k<>n then wlog(',');
  end;
wlog_ln(')');@+tini
@y
var @!k:screen_col; {an index into |a|}
begin
  miktex_paint_row(r, b, a, n);
@!Init wlog('Calling PAINTROW(',r:1,',',b:1,';');
  {this is done only after |init_screen=true|}
for k:=0 to n do
  begin wlog(a[k]:1); if k<>n then wlog(',');
  end;
wlog_ln(')');@+Tini
@z

% _____________________________________________________________________________
%
% [31.628]
% _____________________________________________________________________________

@x
@!input_stack : array[0..stack_size] of in_state_record;
@!input_ptr : 0..stack_size; {first unused location of |input_stack|}
@!max_in_stack: 0..stack_size; {largest value of |input_ptr| when pushing}
@y
@!input_stack : array[0..sup_stack_size] of in_state_record;
@!input_ptr : 0..sup_stack_size; {first unused location of |input_stack|}
@!max_in_stack: 0..sup_stack_size; {largest value of |input_ptr| when pushing}
@z

% _____________________________________________________________________________
%
% [31.631]
% _____________________________________________________________________________

@x
@!line_stack : array[1..max_in_open] of integer;
@y
@!line_stack : array[1..max_in_open] of integer;
@!source_filename_stack : array[1..max_in_open] of str_number;
@!full_source_filename_stack : array[1..max_in_open] of str_number;
@z

% _____________________________________________________________________________
%
% [31.633]
% _____________________________________________________________________________

@x
@!param_stack:array [0..param_size] of pointer;
  {token list pointers for parameters}
@!param_ptr:0..param_size; {first unused entry in |param_stack|}
@y
@!param_stack:array [0..0] of pointer;
  {token list pointers for parameters}
@!param_ptr:integer; {first unused entry in |param_stack|}
@z

% _____________________________________________________________________________
%
% [31.634]
% _____________________________________________________________________________

@x
@!file_ptr:0..stack_size; {shallowest level shown by |show_context|}
@y
@!file_ptr:0..sup_stack_size; {shallowest level shown by |show_context|}
@z

% _____________________________________________________________________________
%
% [31.641]
% _____________________________________________________________________________

@x
@!i:0..buf_size; {index into |buffer|}
@y
@!i:0..sup_buf_size; {index into |buffer|}
@z

@x
@!n:0..error_line; {length of line 1}
@y
@!n:0..sup_error_line; {length of line 1}
@z

% _____________________________________________________________________________
%
% [33.667]
% _____________________________________________________________________________

@x
var @!k:0..buf_size; {an index into |buffer|}
@y
var @!k:0..sup_buf_size; {an index into |buffer|}
@z

% _____________________________________________________________________________
%
% [33.682]
% _____________________________________________________________________________

@x
var @!k:0..buf_size; {an index into |buffer|}
@y
var @!k:0..sup_buf_size; {an index into |buffer|}
@z

% _____________________________________________________________________________
%
% [34.697]
% _____________________________________________________________________________

@x
@!k:0..param_size; {the total number of parameters}
@y
@!k:integer; {the total number of parameters}
@z

% _____________________________________________________________________________
%
% [38.768]
% _____________________________________________________________________________

@x
@ The file names we shall deal with for illustrative purposes have the
following structure:  If the name contains `\.>' or `\.:', the file area
consists of all characters up to and including the final such character;
otherwise the file area is null.  If the remaining file name contains
`\..', the file extension consists of all such characters from the first
remaining `\..' to the end, otherwise the file extension is null.
@^system dependencies@>

We can scan such file names easily by using two global variables that keep track
of the occurrences of area and extension delimiters:

@<Glob...@>=
@!area_delimiter:pool_pointer; {the most recent `\.>' or `\.:', if any}
@!ext_delimiter:pool_pointer; {the relevant `\..', if any}
@y
@ The file names we shall deal with for illustrative purposes have the
following structure:  If the name contains `\./', the file area
consists of all characters up to and including the final such character;
otherwise the file area is null.  If the remaining file name contains
`\..', the file extension consists of all such characters from the first
remaining `\..' to the end, otherwise the file extension is null.
@^system dependencies@>

We can scan such file names easily by using two global variables that keep
track of the occurrences of area and extension delimiters:

@<Glob...@>=
@!area_delimiter:pool_pointer; {the most recent `\./', if any}
@!ext_delimiter:pool_pointer; {the most recent `\..', if any}
@z

% _____________________________________________________________________________
%
% [38.769]
% _____________________________________________________________________________

@x
@d MF_area=="MFinputs:"
@.MFinputs@>
@y
In C, the default paths are specified separately.
@z

% _____________________________________________________________________________
%
% [38.770]
% _____________________________________________________________________________

@x
begin area_delimiter:=0; ext_delimiter:=0;
@y
begin area_delimiter:=0; ext_delimiter:=0; quoted_filename:=false;
@z

% _____________________________________________________________________________
%
% [38.771]
% _____________________________________________________________________________

@x
begin if c=" " then more_name:=false
else  begin if (c=">")or(c=":") then
@y
begin
if c="""" then begin
  quoted_filename:=not quoted_filename;
  more_name:=true;
  end
else if ((c=" ")or(c=tab)) and stop_at_space and (not quoted_filename) then
  more_name:=false
else  begin
  if c="/" then
@z

@x
  else if (c=".")and(ext_delimiter=0) then ext_delimiter:=pool_ptr;
@y
  else if c="." then ext_delimiter:=pool_ptr;
@z

% _____________________________________________________________________________
%
% [38.772]
% _____________________________________________________________________________

@x
@p procedure end_name;
begin if str_ptr+3>max_str_ptr then
  begin if str_ptr+3>max_strings then
    overflow("number of strings",max_strings-init_str_ptr);
@:METAFONT capacity exceeded number of strings}{\quad number of strings@>
  max_str_ptr:=str_ptr+3;
  end;
@y
@p procedure end_name;
var must_quote:boolean; {whether we need to quote a string}
@!j,@!s,@!t: pool_pointer; {running indices}
begin if str_ptr+3>max_str_ptr then
  begin if str_ptr+3>max_strings then
    overflow("number of strings",max_strings-init_str_ptr);
@:METAFONT capacity exceeded number of strings}{\quad number of strings@>
  max_str_ptr:=str_ptr+3;
  end;
str_room(6); {room for quotes, if they are needed}
if area_delimiter<>0 then begin
  {maybe quote |cur_area|}
  must_quote:=false;
  s:=str_start[str_ptr];
  t:=area_delimiter+1;
  j:=s;
  while (not must_quote) and (j<>t) do begin
    must_quote:=str_pool[j]=' '; incr(j);
    end;
  if must_quote then begin
    for j:=pool_ptr-1 downto t do str_pool[j+2]:=str_pool[j];
    str_pool[t+1]:="""";
    for j:=t-1 downto s do str_pool[j+1]:=str_pool[j];
    str_pool[s]:="""";
    if ext_delimiter<>0 then ext_delimiter:=ext_delimiter+2;
    area_delimiter:=area_delimiter+2;
    pool_ptr:=pool_ptr+2;
    end;
  s:=area_delimiter+1;
  end
else begin
  s:=str_start[str_ptr];
  end;
{maybe quote |cur_name|}
if ext_delimiter=0 then t:=pool_ptr else t:=ext_delimiter;
must_quote:=false;
j:=s;
while (not must_quote) and (j<>t) do begin
  must_quote:=str_pool[j]=" "; incr(j);
  end;
if must_quote then begin
  for j:=pool_ptr-1 downto t do str_pool[j+2]:=str_pool[j];
  str_pool[t+1]:="""";
  for j:=t-1 downto s do str_pool[j+1]:=str_pool[j];
  str_pool[s]:="""";
  if ext_delimiter<>0 then ext_delimiter:=ext_delimiter+2;
  pool_ptr:=pool_ptr+2;
  end;
if ext_delimiter<>0 then begin
  {maybe quote |cur_ext|}
  s:=ext_delimiter;
  t:=pool_ptr;
  must_quote:=false;
  j:=s;
  while (not must_quote) and (j<>t) do begin
    must_quote:=str_pool[j]=' '; incr(j);
    end;
  if must_quote then begin
    str_pool[t+1]:="""";
    for j:=t-1 downto s do str_pool[j+1]:=str_pool[j];
    str_pool[s]:="""";
    pool_ptr:=pool_ptr+2;
    end;
  end;
@z

% _____________________________________________________________________________
%
% [38.773]
% _____________________________________________________________________________

@x
begin slow_print(a); slow_print(n); slow_print(e);
@y
var must_quote: boolean; {whether to quote the filename}
@!j:pool_pointer; {index into |str_pool|}
begin
must_quote:=false;
if a<>0 then begin
  j:=str_start[a];
  while (not must_quote) and (j<>str_start[a+1]) do begin
    must_quote:=str_pool[j]=" "; incr(j);
  end;
end;
if n<>0 then begin
  j:=str_start[n];
  while (not must_quote) and (j<>str_start[n+1]) do begin
    must_quote:=str_pool[j]=" "; incr(j);
  end;
end;
if e<>0 then begin
  j:=str_start[e];
  while (not must_quote) and (j<>str_start[e+1]) do begin
    must_quote:=str_pool[j]=" "; incr(j);
  end;
end;
if must_quote then slow_print("""");
if a<>0 then
  for j:=str_start[a] to str_start[a+1]-1 do
    if so(str_pool[j])<>"""" then
      print(so(str_pool[j]));
if n<>0 then
  for j:=str_start[n] to str_start[n+1]-1 do
    if so(str_pool[j])<>"""" then
      print(so(str_pool[j]));
if e<>0 then
  for j:=str_start[e] to str_start[e+1]-1 do
    if so(str_pool[j])<>"""" then
      print(so(str_pool[j]));
if must_quote then slow_print("""");
@z

% _____________________________________________________________________________
%
% [38.774]
% _____________________________________________________________________________

@x
@d append_to_name(#)==begin c:=#; incr(k);
  if k<=file_name_size then name_of_file[k]:=xchr[c];
  end
@y
@d append_to_name(#)==begin c:=#; if not (c="""") then begin incr(k);
  if k<=file_name_size then name_of_file[k]:=xchr[c];
  end end
@z

@x
for j:=str_start[a] to str_start[a+1]-1 do append_to_name(so(str_pool[j]));
@y
name_of_file := miktex_reallocate(name_of_file, length(a) + length(n) + length(e) + 1);
for j:=str_start[a] to str_start[a+1]-1 do append_to_name(so(str_pool[j]));
@z

@x
for k:=name_length+1 to file_name_size do name_of_file[k]:=' ';
@y
name_of_file[name_length + 1]:= chr(0); {\MiKTeX: 0-terminate the file name}
@z

% _____________________________________________________________________________
%
% [38.775]
% _____________________________________________________________________________

@x
@d base_default_length=18 {length of the |MF_base_default| string}
@d base_area_length=8 {length of its area part}
@y
@d base_area_length=0 {no fixed area in C}
@z

@x
@!MF_base_default:packed array[1..base_default_length] of char;
@y
@!base_default_length: integer;
@!MF_base_default:packed array[1..260] of char; {FIXME}
@z

% _____________________________________________________________________________
%
% [38.776]
% _____________________________________________________________________________

@x
MF_base_default:='MFbases:plain.base';
@y
miktex_get_default_dump_file_name(MF_base_default);
base_default_length:=c4pstrlen(MF_base_default);
@z

% _____________________________________________________________________________
%
% [38.777]
% _____________________________________________________________________________

@x
if base_default_length>file_name_size then bad:=41;
@y
do_nothing;
@z

% _____________________________________________________________________________
%
% [38.778]
% _____________________________________________________________________________

@x
for j:=1 to n do append_to_name(xord[MF_base_default[j]]);
@y
name_of_file := miktex_reallocate(name_of_file, n + (b - a + 1) + base_ext_length + 1);
for j:=1 to n do append_to_name(xord[MF_base_default[j]]);
@z

@x
for k:=name_length+1 to file_name_size do name_of_file[k]:=' ';
@y
name_of_file[name_length + 1]:= chr(0); {\MiKTeX: 0-terminate the file name}
@z

% _____________________________________________________________________________
%
% [38.779]
% _____________________________________________________________________________

@x
var @!j:0..buf_size; {the first space after the file name}
@y
var @!j:0..sup_buf_size; {the first space after the file name}
@z

@x
  pack_buffered_name(0,loc,j-1); {try first without the system file area}
  if w_open_in(base_file) then goto found;
  pack_buffered_name(base_area_length,loc,j-1);
    {now try the system base file area}
  if w_open_in(base_file) then goto found;
@y
  pack_buffered_name(0,loc,j-1);
  if miktex_open_base_file(base_file) then goto found;
@z

@x
  wterm_ln('Sorry, I can''t find that base;',' will try PLAIN.');
@y
  wterm_ln('Sorry, I can''t find that base;',' will try default.');
@z

@x
pack_buffered_name(base_default_length-base_ext_length,1,0);
if not w_open_in(base_file) then
@y
pack_buffered_name(base_default_length-base_ext_length,1,0);
if not miktex_open_base_file(base_file) then
@z

@x
  wterm_ln('I can''t find the PLAIN base file!');
@y
  wterm_ln('I can''t find the default base file!');
@z

% _____________________________________________________________________________
%
% [38.780]
% _____________________________________________________________________________

@x
else  begin for k:=1 to name_length do append_char(xord[name_of_file[k]]);
  make_name_string:=make_string;
  end;
@y
else  begin for k:=1 to name_length do append_char(xord[name_of_file[k]]);
  make_name_string:=make_string;
  end;
  {At this point we also set |cur_name|, |cur_ext|, and |cur_area| to
   match the contents of |name_of_file|.}
  k:=1;
  begin_name;
  stop_at_space:=false;
  while (k<=name_length)and(more_name(name_of_file[k])) do
    incr(k);
  stop_at_space:=true;
  end_name;  
@z

% _____________________________________________________________________________
%
% [38.781]
% _____________________________________________________________________________

@x
while buffer[loc]=" " do incr(loc);
@y
while (buffer[loc]=" ")or(buffer[loc]=tab) do incr(loc);
@z

% _____________________________________________________________________________
%
% [38.786]
% _____________________________________________________________________________

@x
var @!k:0..buf_size; {index into |buffer|}
@y
var @!k:0..sup_buf_size; {index into |buffer|}
@z

% _____________________________________________________________________________
%
% [38.788]
% _____________________________________________________________________________

@x
@!k:0..buf_size; {index into |months| and |buffer|}
@!l:0..buf_size; {end of first input line}
@y
@!k:0..sup_buf_size; {index into |months| and |buffer|}
@!l:0..sup_buf_size; {end of first input line}
@z

@x
if job_name=0 then job_name:="mfput";
@y
if job_name=0 then job_name:=miktex_get_job_name("mfput");
@z

% _____________________________________________________________________________
%
% [38.790]
% _____________________________________________________________________________

@x
slow_print(base_ident); print("  ");
print_int(round_unscaled(internal[day])); print_char(" ");
months:='JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC';
@y
miktex_print_miktex_banner(log_file);
slow_print(base_ident); print("  ");
print_int(round_unscaled(internal[day])); print_char(" ");
c4p_arrcpy(months,'JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC');
@z

% _____________________________________________________________________________
%
% [38.793]
% _____________________________________________________________________________

@x
begin @<Put the desired file name in |(cur_name,cur_ext,cur_area)|@>;
if cur_ext="" then cur_ext:=".mf";
pack_cur_name;
loop@+  begin begin_file_reading; {set up |cur_file| and new level of input}
  if a_open_in(cur_file) then goto done;
  if cur_area="" then
    begin pack_file_name(cur_name,MF_area,cur_ext);
    if a_open_in(cur_file) then goto done;
    end;
@y
var temp_str: str_number; k: integer;
begin @<Put the desired file name in |(cur_name,cur_ext,cur_area)|@>;
pack_cur_name;
loop@+begin
  begin_file_reading; {set up |cur_file| and new level of input}
  if cur_ext = ".mf" then begin
    cur_ext := "";
    pack_cur_name;
    end;
  if a_open_in(cur_file) then
    goto done;
@z

@x
done: name:=a_make_name_string(cur_file); str_ref[cur_name]:=max_str_ref;
@y
done: name:=a_make_name_string(cur_file); str_ref[cur_name]:=max_str_ref;
full_source_filename_stack[in_open]:=miktex_make_full_name_string;
@z

@x
  begin job_name:=cur_name; open_log_file;
@y
  begin job_name:=miktex_get_job_name(cur_name); open_log_file;
@z

@x
if term_offset+length(name)>max_print_line-2 then print_ln
else if (term_offset>0)or(file_offset>0) then print_char(" ");
print_char("("); incr(open_parens); slow_print(name); update_terminal;
@y
if term_offset+length(full_source_filename_stack[in_open])>max_print_line-2 then print_ln
else if (term_offset>0)or(file_offset>0) then print_char(" ");
print_char("("); incr(open_parens); slow_print(full_source_filename_stack[in_open]); update_terminal;
@z

@x
if name=str_ptr-1 then {we can conserve string pool space now}
  begin flush_string(name); name:=cur_name;
  end;
@y
@z

% _____________________________________________________________________________
%
% [39.807]
% _____________________________________________________________________________

@x
if s<>"" then
  begin print_nl("! "); print(s);
@.!\relax@>
  end;
@y
if s<>"" then
  begin
  if (miktex_c_style_error_messages_p and not terminal_input) then
    print_c_style_error_message (s)
  else begin print_nl("! "); print(s);end;
@.!\relax@>
  end;
@z

% _____________________________________________________________________________
%
% [44.1046]
% _____________________________________________________________________________

@x
@!n:0..max_print_line; {amount of macro text to show}
@y
@!n:0..sup_max_print_line; {amount of macro text to show}
@z

% _____________________________________________________________________________
%
% [45.1096]
% _____________________________________________________________________________

@x
@!char_remainder:array[eight_bits] of 0..lig_table_size; {the |remainder| byte}
@y
@!char_remainder:array[eight_bits] of 0..sup_lig_table_size; {the |remainder| byte}
@z

@x
@!lig_kern:array[0..lig_table_size] of four_quarters; {the ligature/kern table}
@y
@!lig_kern:array[0..sup_lig_table_size] of four_quarters; {the ligature/kern table}
@z

@x
@!skip_table:array[eight_bits] of 0..lig_table_size; {local label status}
@y
@!skip_table:array[eight_bits] of 0..sup_lig_table_size; {local label status}
@z

@x
@!bch_label:0..lig_table_size; {left boundary starting location}
@!ll,@!lll:0..lig_table_size; {registers used for lig/kern processing}
@!label_loc:array[0..256] of -1..lig_table_size; {lig/kern starting addresses}
@y
@!bch_label:0..sup_lig_table_size; {left boundary starting location}
@!ll,@!lll:0..sup_lig_table_size; {registers used for lig/kern processing}
@!label_loc:array[0..256] of -1..sup_lig_table_size; {lig/kern starting addresses}
@z

% _____________________________________________________________________________
%
% [45.1134]
% _____________________________________________________________________________

@x
print_nl("Font metrics written on "); slow_print(metric_file_name);
@y
print_nl("Font metrics written on "); print_file_name(0,metric_file_name,0);
@z

% _____________________________________________________________________________
%
% [47.1154]
% _____________________________________________________________________________

@x
var k:gf_index;
begin for k:=a to b do write(gf_file,gf_buf[k]);
@y
begin c4p_buf_write(gf_file,c4p_ptr(gf_buf[a]),b-a+1);
@z

% _____________________________________________________________________________
%
% [47.1182]
% _____________________________________________________________________________

@x
print_nl("Output written on "); slow_print(output_file_name);
@y
print_nl("Output written on "); print_file_name(0,output_file_name,0);
@z

% _____________________________________________________________________________
%
% [48.1185]
% _____________________________________________________________________________

@x
base_ident:=" (INIMF)";
@y
if miktex_is_init_program then base_ident:=" (INIMF)";
@z

% _____________________________________________________________________________
%
% [48.1187]
% _____________________________________________________________________________

@x
@d off_base=6666 {go here if the base file is unacceptable}
@y
@d off_base=6666 {go here if the base file is unacceptable}
@d start_loading=7777
@z

@x
label off_base,exit;
var @!k:integer; {all-purpose index}
@y
label off_base,exit,start_loading;
var @!k:integer; {all-purpose index}
allow_makebase:boolean;
@z

@x
begin @<Undump constants for consistency check@>;
@y
begin
  allow_makebase := true;
start_loading:
  @<Undump constants for consistency check@>;
@z

@x
off_base: wake_up_terminal;
@y
off_base:
  if (allow_makebase) then begin
    allow_makebase := false;
    w_close (base_file);
    { <fixme>asserting that |name_of_file| wasn't overridden</fixme> }
    if (miktex_open_base_file(base_file, true)) then goto start_loading;
  end
  wake_up_terminal;
@z

% _____________________________________________________________________________
%
% [48.1188]
% _____________________________________________________________________________

@x
@d dump_wd(#)==begin base_file^:=#; put(base_file);@+end
@d dump_int(#)==begin base_file^.int:=#; put(base_file);@+end
@d dump_hh(#)==begin base_file^.hh:=#; put(base_file);@+end
@d dump_qqqq(#)==begin base_file^.qqqq:=#; put(base_file);@+end
@y
@d dump_things(#)==miktex_dump(base_file, #)
@d dump_wd(#)==miktex_dump(base_file, #)
@d dump_int(#)==miktex_dump_int(base_file, #)
@d dump_hh(#)==miktex_dump(base_file, #)
@d dump_qqqq(#)==miktex_dump(base_file, #)
@z

% _____________________________________________________________________________
%
% [48.1189]
% _____________________________________________________________________________

@x
@d undump_wd(#)==begin get(base_file); #:=base_file^;@+end
@d undump_int(#)==begin get(base_file); #:=base_file^.int;@+end
@d undump_hh(#)==begin get(base_file); #:=base_file^.hh;@+end
@d undump_qqqq(#)==begin get(base_file); #:=base_file^.qqqq;@+end
@y
@d undump_things(#)==miktex_undump(base_file, #)
@d undump_checked_things(#)==miktex_undump(base_file, #)
@d undump_upper_check_things(#)==miktex_undump(base_file, #)
@d undump_wd(#)==miktex_undump(base_file, #)
@d undump_int(#)==miktex_undump_int(base_file, #)
@d undump_hh(#)==miktex_undump(base_file, #)
@d undump_qqqq(#)==miktex_undump(base_file, #)
@z

% _____________________________________________________________________________
%
% [48.1190]
% _____________________________________________________________________________

@x
dump_int(@$);@/
@y
dump_int(@"4D694B4D); {"MKiM"}
dump_int(@$);@/
@<Dump |xord|, |xchr|, and |xprn|@>;
@z

% _____________________________________________________________________________
%
% [48.1191]
% _____________________________________________________________________________

@x
x:=base_file^.int;
if x<>@$ then goto off_base; {check that strings are the same}
undump_int(x);
if x<>mem_min then goto off_base;
undump_int(x);
if x<>mem_top then goto off_base;
@y
undump_int(x);
if x<>@"4D694B4D then goto off_base; {not a base file}
undump_int(x);
if x<>@$ then goto off_base;
@<Undump |xord|, |xchr|, and |xprn|@>;
undump_int(x);
if x<>mem_min then goto off_base;
{Now we deal with dynamically allocating the memory. We don't provide
 all the fancy features \.{tex.ch} does---all that matters is enough to
 run the trap test with a memory size of 3000.}
undump_int (mem_top); {Overwrite whatever we had.}
if mem_max < mem_top then mem_max:=mem_top; {Use at least what we dumped.}
if mem_min+1100>mem_top then goto off_base;
mem:=miktex_reallocate(mem, mem_max - mem_min + 2);
@z

% _____________________________________________________________________________
%
% [48.1199]
% _____________________________________________________________________________

@x
undump_int(x);@+if (x<>69069)or eof(base_file) then goto off_base
@y
undump_int(x);@+if (x<>69069)or not eof(base_file) then goto off_base
@z

% _____________________________________________________________________________
%
% [49.1204]
% _____________________________________________________________________________

@x
@p begin @!{|start_here|}
@y
@p begin @!{|start_here|}
c4p_begin_try_block(final_end);
c4p_begin_try_block(end_of_MF);
miktex_on_texmf_start_job;
miktex_process_command_line_options;
miktex_allocate_memory;
@z

@x
    '---case ',bad:1);
@y
    '---case ',bad);
@z

@x
@!init if not get_strings_started then goto final_end;
init_tab; {initialize the tables}
init_prim; {call |primitive| for each primitive}
init_str_ptr:=str_ptr; init_pool_ptr:=pool_ptr;@/
max_str_ptr:=str_ptr; max_pool_ptr:=pool_ptr; fix_date_and_time;
tini@/
@y  22833
@!init
if miktex_is_init_program then begin
if not get_strings_started then goto final_end;
init_tab; {initialize the tables}
init_prim; {call |primitive| for each primitive}
init_str_ptr:=str_ptr; init_pool_ptr:=pool_ptr;@/
max_str_ptr:=str_ptr; max_pool_ptr:=pool_ptr; fix_date_and_time;
end;
tini@/
@z

@x
ready_already:=314159;
@y
ready_already:=314159;
miktex_on_texmf_initialize;
@z

@x
end_of_MF: close_files_and_terminate;
final_end: ready_already:=0;
@y
end_of_MF: c4p_end_try_block(end_of_MF); close_files_and_terminate;
final_end: c4p_end_try_block(final_end); ready_already:=0;
miktex_free_memory;
miktex_on_texmf_finish_job;
if (history = error_message_issued) or (history = fatal_error_stop) then
begin
  c4p_exit (1);
end;
@z

% _____________________________________________________________________________
%
% [49.1205]
% _____________________________________________________________________________

@x
    slow_print(log_name); print_char(".");
    end;
  end;
@y
    print_file_name(0,log_name,0); print_char(".");
    end;
  end;
print_ln;
if (edit_name_start<>0) and (interaction>batch_mode) then begin
  if log_opened then
    miktex_invoke_editor(edit_name_start,edit_name_length,edit_line,
                         str_start[log_name], length(log_name))
  else
    miktex_invoke_editor(edit_name_start,edit_name_length,edit_line)
end;
@z

% _____________________________________________________________________________
%
% [49.1209]
% _____________________________________________________________________________

@x
  begin @!init store_base_file; return;@+tini@/
@y
  begin
    @!init if miktex_is_init_program then begin store_base_file; return;end;@+tini@/
@z

% _____________________________________________________________________________
%
% [50.1212]
% _____________________________________________________________________________

@x
@<Initialize the print |selector|...@>;
@y
if miktex_get_interaction >= 0 then
  interaction:=miktex_get_interaction;
@<Initialize the print |selector|...@>;
@z

% _____________________________________________________________________________
%
% [51.1214]
% _____________________________________________________________________________

@x
@* \[52] Index.
@y
@ @<Declare \MiKTeX\ procedures@>=

function make_fraction(@!p,@!q:integer):fraction; forward;@t\2@>@/
function miktex_c_style_error_messages_p : boolean; forward;@t\2@>@/
function miktex_enable_eightbit_chars_p : boolean; forward;@t\2@>@/
function miktex_get_interaction : integer; forward;@t\2@>@/
function miktex_get_quiet_flag : boolean; forward;@t\2@>@/
function miktex_halt_on_error_p : boolean; forward;@t\2@>@/
function miktex_have_tcx_file_name : boolean; forward;@t\2@>@/
function miktex_init_screen : boolean; forward;@t\2@>@/
function miktex_is_init_program : boolean; forward;@t\2@>@/
function miktex_make_full_name_string : str_number; forward;@t\2@>@/
function take_fraction(@!p,@!q:integer):fraction; forward;@t\2@>@/
function take_scaled(@!p,@!q:integer):scaled; forward;@t\2@>@/
function make_scaled(@!p,@!q:integer):scaled; forward;@t\2@>@/

@ @<Error handling procedures@>=

procedure print_c_style_error_message (@!s:str_number);
  var k : integer;
      old_interaction : batch_mode..error_stop_mode;
begin
  old_interaction := interaction;
  interaction := nonstop_mode;
  @<Initialize the print |selector| based on |interaction|@>;
  if log_opened then selector:=selector+2;
  print_nl ("");
  print (full_source_filename_stack[in_open]);
  print (":");
  print_int (line);
  print (": ");
  print (s);
  interaction := old_interaction;
  @<Initialize the print |selector| based on |interaction|@>;
  if log_opened then selector:=selector+2;
end;

@ @<Global variables@>=
@!bistack_size:integer; {size of stack for bisection algorithms;
  should probably be left at this value}
@!buf_size:integer; {maximum number of characters simultaneously present in
  current lines of open files; must not exceed |max_halfword|}
@!dummy_xord:ASCII_code;
@!dummy_xchr:text_char;
@!dummy_xprn:ASCII_code;
@!edit_line:integer; {what line to start editing at}
@!edit_name_length:integer;
@!edit_name_start:pool_pointer; {where the filename to switch to starts}
@!error_line:integer; {width of context lines on terminal error messages}
@!half_error_line:integer; {width of first lines of contexts in terminal
  error messages; should be between 30 and |error_line-15|}
@!lig_table_size:integer; {maximum number of ligature/kern steps, must be
  at least 255 and at most 32510}
@!main_memory:integer; {total memory words allocated in initex}
@!max_print_line:integer; {width of longest text lines output; should be at least 60}
@!max_wiggle:integer; {number of autorounded points per cycle}
@!max_strings:integer; {maximum number of strings; must not exceed |max_halfword|}
@!mem_max:integer; {greatest index in \MF's internal |mem| array;
  must be strictly less than |max_halfword|;
  must be equal to |mem_top| in \.{INIMF}, otherwise |>=mem_top|}
@!mem_top:integer; {largest index in the |mem| array dumped by \.{INIMF};
  must be substantially larger than |mem_bot|,
  equal to |mem_max| in \.{INIMF}, else not greater than |mem_max|}
@!move_size:integer; {space for storing moves in a single octant}
@!quoted_filename:boolean; {current filename is quoted}
@!param_size:integer; {maximum number of simultaneous macro parameters}
@!path_size:integer; {maximum number of knots between breakpoints of a path}
@!pool_size:integer; {maximum number of characters in strings, including all
  error messages and help texts, and the names of all identifiers;
  must exceed |string_vacancies| by the total
  length of \MF's own strings, which is currently about 22000}
@!stack_size:integer; {maximum number of simultaneous input sources}
@!stop_at_space:boolean; {whether |more_name| returns false for space}
@!string_vacancies:integer; {the minimum number of characters that should be
  available for the user's identifier names and strings,
  after \MF's own error messages are stored}
@!xprn:array[ASCII_code] of ASCII_code; {use \.{\^\^} notation?}

@ @<Set init...@>=
edit_name_start:=0;
stop_at_space:=true;

@ Dumping the |xord|, |xchr|, and |xprn| arrays.  We dump these always
in the format, so a TCX file loaded during format creation can set a
default for users of the format.

@<Dump |xord|, |xchr|, and |xprn|@>=
dump_things(xord[0], 256);
dump_things(xchr[0], 256);
dump_things(xprn[0], 256);

@ Undumping the |xord|, |xchr|, and |xprn| arrays.  This code is more
complicated, because we want to ensure that a TCX file specified on
the command line will override whatever is in the format.  Since the
tcx file has already been loaded, that implies throwing away the data
in the format.  Also, if no |translate_filename| is given, but
|eight_bit_p| is set we have to make all characters printable.

@<Undump |xord|, |xchr|, and |xprn|@>=
if miktex_have_tcx_file_name then begin
  for k:=0 to 255 do undump_things(dummy_xord, 1);
  for k:=0 to 255 do undump_things(dummy_xchr, 1);
  for k:=0 to 255 do undump_things(dummy_xprn, 1);
  end
else begin
  undump_things(xord[0], 256);
  undump_things(xchr[0], 256);
  undump_things(xprn[0], 256);
  if miktex_enable_eightbit_chars_p then
    for k:=0 to 255 do
      xprn[k]:=1;
end;

@* \[52] Index.
@z
