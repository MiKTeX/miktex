%% gftype-miktex.ch: WEB change file for GFtype
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
  begin print_ln(banner);@/
@y
  begin print(banner);
  miktex_print_miktex_banner(output);
  write_ln;@/
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
% [3.19]
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
% [4.22]
% _____________________________________________________________________________

@x
begin reset(gf_file);
@y
begin
if (c4pargc <> 2) then
 abort('Usage: gftype inputfile');
c4p_fopen(gf_file,c4p_argv[1],c4p_rb_mode,true);
reset(gf_file);
@z

% _____________________________________________________________________________
%
% [5.31]
% _____________________________________________________________________________

@x
begin rewrite(term_out); {prepare the terminal for output}
write_ln(term_out,banner);@/
@y
begin term_out:=output; term_in:=input;
@z

% _____________________________________________________________________________
%
% [8.62]
% _____________________________________________________________________________

@x
@!pix_ratio: real; {multiply by this to convert \.{TFM} width to scaled pixels}
@y
@!pix_ratio: longreal;
@z

% _____________________________________________________________________________
%
% [9.66] The main program
% _____________________________________________________________________________

@x
@p begin initialize; {get all variables initialized}
@y
@p begin
c4p_begin_try_block(final_end);
initialize; {get all variables initialized}
@z

@x
final_end:end.
@y
print_nl;
c4p_exit (0);
final_end:
c4p_end_try_block(final_end);
c4p_exit (1);
end.
@z
