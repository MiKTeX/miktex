%% tangle-miktex.ch: WEB change file for TANGLE
%% 
%% Copyright (C) 1991-2021 Christian Schenk
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
@d stat==@{ {change this to `$\\{stat}\equiv\null$'
  when gathering usage statistics}
@d tats==@t@>@} {change this to `$\\{tats}\equiv\null$'
  when gathering usage statistics}
 @y
@d stat==
@d tats==
 @z

% _____________________________________________________________________________
%
% [1.8]
% _____________________________________________________________________________

@x
@!buf_size=100; {maximum length of input line}
@!max_bytes=45000; {|1/ww| times the number of bytes in identifiers,
  strings, and module names; must be less than 65536}
@!max_toks=50000; {|1/zz| times the number of bytes in compressed \PASCAL\ code;
  must be less than 65536}
@!max_names=4000; {number of identifiers, strings, module names;
  must be less than 10240}
@!max_texts=2000; {number of replacement texts, must be less than 10240}
@!hash_size=353; {should be prime}
@!longest_name=400; {module names shouldn't be longer than this}
@!line_length=72; {lines of \PASCAL\ output have at most this many characters}
@!out_buf_size=144; {length of output buffer, should be twice |line_length|}
@!stack_size=50; {number of simultaneous levels of macro expansion}
@!max_id_length=12; {long identifiers are chopped to this length, which must
  not exceed |line_length|}
@!unambig_length=7; {identifiers must be unique if chopped to this length}
  {note that 7 is more strict than \PASCAL's 8, but this can be varied}
@y
@!buf_size=5000; {maximum length of input line}
@!max_bytes=65535; {|1/ww| times the number of bytes in identifiers,
  strings, and module names; must be less than 65536}
@!max_toks=65535; {|1/zz| times the number of bytes in compressed \PASCAL\ code;
  must be less than 65536}
@!max_names=10239; {number of identifiers, strings, module names;
  must be less than 10240}
@!max_texts=10239; {number of replacement texts, must be less than 10240}
@!hash_size=353; {should be prime}
@!longest_name=2000; {module names shouldn't be longer than this}
@!line_length=130; {lines of \PASCAL\ output have at most this many characters}
@!out_buf_size=260; {length of output buffer, should be twice |line_length|}
@!stack_size=1000; {number of simultaneous levels of macro expansion}
@!max_id_length=64; {long identifiers are chopped to this length, which must
  not exceed |line_length|}
@!unambig_length=64; {identifiers must be unique if chopped to this length}
@!max_numeric=1073741823;
@z

% _____________________________________________________________________________
%
% [2.12]
% _____________________________________________________________________________

@x
@d text_char == char {the data type of characters in text files}
@d first_text_char=0 {ordinal number of the smallest element of |text_char|}
@d last_text_char=255 {ordinal number of the largest element of |text_char|}
@y
@d text_char == char {the data type of characters in text files}
@d first_text_char=-128 {ordinal number of the smallest element of |text_char|}
@d last_text_char=127 {ordinal number of the largest element of |text_char|}
@z

% _____________________________________________________________________________
%
% [2.17]
% _____________________________________________________________________________

@x
for i:=1 to @'37 do xchr[i]:=' ';
for i:=@'200 to @'377 do xchr[i]:=' ';
@y
for i:=1 to @'37 do xchr[i]:=chr(i);
for i:=@'200 to @'377 do xchr[i]:=chr(i);
@z

% _____________________________________________________________________________
%
% [2.18]
% _____________________________________________________________________________

@x
xord[' ']:=" ";
@y
xord[' ']:=" ";
xord[xchr[carriage_return]]:=" ";
@z

% _____________________________________________________________________________
%
% [3.21]
% _____________________________________________________________________________

@x
rewrite(term_out,'TTY:'); {send |term_out| output to the terminal}
@y
term_out:=output;
@z

% _____________________________________________________________________________
%
% [3.24]
% _____________________________________________________________________________

@x
begin reset(web_file); reset(change_file);
@y
begin if (c4pargc < 3) then
  fatal_error('Usage: tangle inputfile changefile outputfile poolfile');
c4p_fopen (web_file,c4p_argv[1],c4p_r_mode,true); reset(web_file);
c4p_fopen (change_file,c4p_argv[2],c4p_r_mode,true); reset(change_file);
@z

@x
rewrite(Pascal_file); rewrite(pool);
@y
if (c4pargc < 5) then
  fatal_error('Usage: tangle inputfile changefile outputfile poolfile');
c4p_fopen(Pascal_file,c4p_argv[3],c4p_wb_mode,true); rewrite(Pascal_file);
c4p_fopen(pool,c4p_argv[4],c4p_wb_mode,true); rewrite(pool);
@z

% _____________________________________________________________________________
%
% [4.34]
% _____________________________________________________________________________

@x
begin goto end_of_TANGLE;
@y
begin throw(end_of_TANGLE);
@z

% _____________________________________________________________________________
%
% [5.37] Data structures
% _____________________________________________________________________________

@x
@!sixteen_bits=0..65535; {unsigned two-byte quantity}
@y
@!sixteen_bits=0..max_numeric; {unsigned quantity}
@z

% _____________________________________________________________________________
%
% [5.38]
% _____________________________________________________________________________

@x
@d ww=2 {we multiply the byte capacity by approximately this amount}
@d zz=3 {we multiply the token capacity by approximately this amount}
@y
@d ww=5 {we multiply the byte capacity by approximately this amount}
@d zz=5 {we multiply the token capacity by approximately this amount}
@z

% _____________________________________________________________________________
%
% [5.40]
% _____________________________________________________________________________

@x
@!string_ptr:name_pointer; {next number to be given to a string of length |<>1|}
@y
@!string_ptr:integer; {next number to be given to a string of length |<>1|}
@z

% _____________________________________________________________________________
%
% [5.42]
% _____________________________________________________________________________

@x
name_ptr:=1; string_ptr:=256; pool_check_sum:=271828;
@y
name_ptr:=1; string_ptr:=max_char+1; pool_check_sum:=271828;
@z

% _____________________________________________________________________________
%
% [5.47]
% _____________________________________________________________________________

@x
their |equiv| value contains the corresponding numeric value plus $2^{15}$.
@y
their |equiv| value contains the corresponding numeric value plus $2^{30}$.
@z

% _____________________________________________________________________________
%
% [6.63]
% _____________________________________________________________________________

@x
error; q:=0; {only one conflict will be printed, since |equiv[0]=0|}
@y
mark_harmless; q:=0; {only one conflict will be printed, since |equiv[0]=0|}
@z

% _____________________________________________________________________________
%
% [6.64]
% _____________________________________________________________________________

@x
  equiv[p]:=buffer[id_first+1]+@'100000
else  begin equiv[p]:=string_ptr+@'100000;
@y
  equiv[p]:=buffer[id_first+1]+(max_numeric+1)
else  begin equiv[p]:=string_ptr+(max_numeric+1);
@z

% _____________________________________________________________________________
%
% [9.89]
% _____________________________________________________________________________

@x
numeric: begin cur_val:=equiv[a]-@'100000; a:=number;
@y
numeric: begin cur_val:=equiv[a]-(max_numeric+1); a:=number;
@z

% _____________________________________________________________________________
%
% [10.105]
% _____________________________________________________________________________

@x
 (((out_contrib[1]="D")and(out_contrib[2]="I")and(out_contrib[3]="V")) or@|
 ((out_contrib[1]="M")and(out_contrib[2]="O")and(out_contrib[3]="D")) ))or@|
@y
 (((out_contrib[1]="d")and(out_contrib[2]="i")and(out_contrib[3]="v")) or@|
 ((out_contrib[1]="m")and(out_contrib[2]="o")and(out_contrib[3]="d")) ))or@|
@z

% _____________________________________________________________________________
%
% [10.110]
% _____________________________________________________________________________

@x
  if ((out_buf[out_ptr-3]="D")and(out_buf[out_ptr-2]="I")and
    (out_buf[out_ptr-1]="V"))or @/
     ((out_buf[out_ptr-3]="M")and(out_buf[out_ptr-2]="O")and
    (out_buf[out_ptr-1]="D")) then@/ goto bad_case
@y
  if ((out_buf[out_ptr-3]="d")and(out_buf[out_ptr-2]="i")and
    (out_buf[out_ptr-1]="v"))or @/
     ((out_buf[out_ptr-3]="m")and(out_buf[out_ptr-2]="o")and
    (out_buf[out_ptr-1]="d")) then@/ goto bad_case
@z

% _____________________________________________________________________________
%
% [11.114]
% _____________________________________________________________________________

@x
and_sign: begin out_contrib[1]:="A"; out_contrib[2]:="N"; out_contrib[3]:="D";
@^uppercase@>
  send_out(ident,3);
  end;
not_sign: begin out_contrib[1]:="N"; out_contrib[2]:="O"; out_contrib[3]:="T";
  send_out(ident,3);
  end;
set_element_sign: begin out_contrib[1]:="I"; out_contrib[2]:="N";
  send_out(ident,2);
  end;
or_sign: begin out_contrib[1]:="O"; out_contrib[2]:="R"; send_out(ident,2);
  end;
@y
and_sign: begin out_contrib[1]:="a"; out_contrib[2]:="n"; out_contrib[3]:="d";
@^uppercase@>
  send_out(ident,3);
  end;
not_sign: begin out_contrib[1]:="n"; out_contrib[2]:="o"; out_contrib[3]:="t";
  send_out(ident,3);
  end;
set_element_sign: begin out_contrib[1]:="i"; out_contrib[2]:="n";
  send_out(ident,2);
  end;
or_sign: begin out_contrib[1]:="o"; out_contrib[2]:="r"; send_out(ident,2);
  end;
@z

% _____________________________________________________________________________
%
% [11.116]
% _____________________________________________________________________________

@x
"A",up_to("Z"): begin out_contrib[1]:=cur_char; send_out(ident,1);
  end;
"a",up_to("z"): begin out_contrib[1]:=cur_char-@'40; send_out(ident,1);
@y
"A",up_to("Z"): begin out_contrib[1]:=cur_char+@'40; send_out(ident,1);
  end;
"a",up_to("z"): begin out_contrib[1]:=cur_char; send_out(ident,1);
@z

@x
    if out_contrib[k]>="a" then out_contrib[k]:=out_contrib[k]-@'40
@y
    if (out_contrib[k]>="A")and(out_contrib[k]<="Z") then
       out_contrib[k]:=out_contrib[k]+@'40
@z

% _____________________________________________________________________________
%
% [11.119]
% _____________________________________________________________________________

@x
  if cur_char="e" then cur_char:="E";
@^uppercase@>
  if cur_char="E" then goto get_fraction
@y
  if cur_char="E" then cur_char:="e";
@^uppercase@>
  if cur_char="e" then goto get_fraction
@z

% _____________________________________________________________________________
%
% [11.120]
% _____________________________________________________________________________

@x
if (out_contrib[k]="E")and((cur_char="+")or(cur_char="-")) then
@^uppercase@>
  begin if k<line_length then incr(k);
  out_contrib[k]:=cur_char; cur_char:=get_output;
  end
else if cur_char="e" then cur_char:="E";
until (cur_char<>"E")and((cur_char<"0")or(cur_char>"9"));
@y
if (out_contrib[k]="e")and((cur_char="+")or(cur_char="-")) then
@^uppercase@>
  begin if k<line_length then incr(k);
  out_contrib[k]:=cur_char; cur_char:=get_output;
  end
else if cur_char="E" then cur_char:="e";
until (cur_char<>"e")and((cur_char<"0")or(cur_char>"9"));
@z

% _____________________________________________________________________________
%
% [14.157]
% _____________________________________________________________________________

@x
if abs(accumulator)>=@'100000 then
  begin err_print('! Value too big: ',accumulator:1); accumulator:=0;
@.Value too big@>
  end;
equiv[p]:=accumulator+@'100000; {name |p| now is defined to equal |accumulator|}
@y
if abs(accumulator)>=(max_numeric+1) then
  begin err_print('! Value too big: ',accumulator:1); accumulator:=0;
@.Value too big@>
  end;
equiv[p]:=accumulator+(max_numeric+1); {name |p| now is defined to equal |accumulator|}
@z

% _____________________________________________________________________________
%
% [14.158]
% _____________________________________________________________________________

@x
    add_in(equiv[q]-@'100000);
@y
    add_in(equiv[q]-(max_numeric+1));
@z

% _____________________________________________________________________________
%
% [17.180]
% _____________________________________________________________________________

@x
reset(term_in,'TTY:','/I'); {open |term_in| as the terminal, don't do a |get|}
@y
term_in:=input;
@z

% _____________________________________________________________________________
%
% [18.182] The main program
% _____________________________________________________________________________

@x
@p begin initialize;
@<Initialize the input system@>;
print_ln(banner); {print a ``banner line''}
@y
@p begin
c4p_begin_try_block(end_of_TANGLE);
miktex_process_command_line_options;
initialize;
@<Initialize the input system@>;
print(banner);
miktex_print_miktex_banner(term_out);
new_line; {print a ``banner line''}
@z

@x
end_of_TANGLE:
@y
end_of_TANGLE: c4p_end_try_block(end_of_TANGLE);
@z

@x
if string_ptr>256 then @<Finish off the string pool file@>;
@y
if string_ptr>max_char+1 then @<Finish off the string pool file@>;
@z

% _____________________________________________________________________________
%
% [18.184]
% _____________________________________________________________________________

@x
begin print_nl(string_ptr-256:1, ' strings written to string pool file.');
@y
begin print_nl(string_ptr-(max_char+1):1, ' strings written to string pool file.');
@z

% _____________________________________________________________________________
%
% [18.187]
% _____________________________________________________________________________

@x
end {there are no other cases}
@y
end; {there are no other cases}
new_line;
if (history > harmless_message) then
  c4p_exit (history);
@z

% _____________________________________________________________________________
%
% [19.188] System-dependent changes
% _____________________________________________________________________________

@x
which introduce new modules, can be inserted here; then only the index
itself will get a new module number.
@^system dependencies@>
@y
which introduce new modules, can be inserted here; then only the index
itself will get a new module number.
@^system dependencies@>

@ @<Globals...@>=
@!max_char : integer;
@z
