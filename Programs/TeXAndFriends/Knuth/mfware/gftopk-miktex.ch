%% gftopk-miktex.ch: WEB change file for GFtoPK
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
% [1.4]
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
% [1.6]
% _____________________________________________________________________________

@x
@!max_row=16000; {largest index in the main |row| array}
@y
@!max_row=128000;
@z

% _____________________________________________________________________________
%
% [1.8]
% _____________________________________________________________________________

@x
begin goto final_end;
@y
begin c4p_throw(final_end);
@z

% _____________________________________________________________________________
%
% [2.10]
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
% [5.39]
% _____________________________________________________________________________

@x
begin reset(gf_file);
@y
begin
if (c4pargc <> 3) then
 abort('Usage: gftopk inputfile outputpfile');
c4p_fopen(gf_file,c4p_argv[1],c4p_rb_mode,true); reset(gf_file);
@z

@x
begin rewrite(pk_file);
@y
begin
if (c4pargc <> 3) then
 abort('Usage: gftopk inputfile outputfile');
c4p_fopen(pk_file,c4p_argv[2],c4p_wb_mode,true);
@z

% _____________________________________________________________________________
%
% [5.46]
% _____________________________________________________________________________

@x
   set_pos(gf_file, -1) ; gf_len := cur_pos(gf_file) ;
@y
   c4p_fseek(gf_file,0,c4p_end); gf_len:=c4p_ftell(gf_file);
@z

@x
   set_pos(gf_file, n); gf_loc := n ;
@y
   c4p_fseek(gf_file,n,c4p_set); gf_loc:=n;
@z

% _____________________________________________________________________________
%
% [8.81]
% _____________________________________________________________________________

@x
@d comm_length = 23 {length of |preamble_comment|}
@y
@d comm_length = 23 {length of |preamble_comment|}
@d comm_length_plus_one = 24 {one more for the string terminator}
@z

% _____________________________________________________________________________
%
% [8.82]
% _____________________________________________________________________________

@x
@!comment : packed array[1..comm_length] of char ;
@y
@!comment : packed array[1..comm_length_plus_one] of char ;
@z

% _____________________________________________________________________________
%
% [8.82]
% _____________________________________________________________________________

@x
comment := preamble_comment ;
@y
c4p_strcpy (comment, comm_length_plus_one, preamble_comment);
@z

% _____________________________________________________________________________
%
% [8.86]
% _____________________________________________________________________________

@x
  initialize ;
@y
  c4p_begin_try_block(final_end);
  initialize ;
@z

@x
final_end : end .
@y
  c4p_exit (0);
final_end :
  c4p_end_try_block(final_end);
  c4p_exit (1);
end .
@z
