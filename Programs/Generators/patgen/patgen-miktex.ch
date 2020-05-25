%% patgen-miktex.ch:
%% 
%% Copyright (C) 2020 Christian Schenk
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
% [1.3]
% _____________________________________________________________________________

@x
  begin print_ln(banner);@/
@y
  begin print(banner); miktex_print_miktex_banner(output); write_ln;@/
@z

% _____________________________________________________________________________
%
% [1.10]
% _____________________________________________________________________________

@x
@d jump_out==goto end_of_PATGEN {terminates \.{PATGEN}}
@y
@d jump_out==c4p_throw(end_of_PATGEN) {terminates \.{PATGEN}}
@z

@x
@d error(#)==begin print_ln(#); jump_out; end
@y
@d error(#)==begin write_ln(c4p_error_output, #); jump_out; end
@z

% _____________________________________________________________________________
%
% [2.12] The character set
% _____________________________________________________________________________

@x
@!text_char=char; {the data type of characters in text files}
@!ASCII_code=0..last_ASCII_code; {internal representation of input characters}
@y
@!ASCII_code=0..last_ASCII_code; {internal representation of input characters}
@!text_char=ASCII_code; {the data type of characters in text files}
@z

% _____________________________________________________________________________
%
% [3.27]
% _____________________________________________________________________________

@x
@!trie_size=55000; {space for pattern trie}
@!triec_size=26000; {space for pattern count trie, must be less than
 |trie_size| and greater than the number of occurrences of any pattern in
 the dictionary}
@y
@!trie_size=10000000; {space for pattern trie}
@!triec_size=5000000; {space for pattern count trie, must be less than
 |trie_size| and greater than the number of occurrences of any pattern in
 the dictionary}
@z

@x
@!max_buf_len=80; {maximum length of input lines, must be at least |max_len|}
@y
@!max_buf_len=3000; {maximum length of input lines, must be at least |max_len|}
@z

% _____________________________________________________________________________
%
% [6.51] Input and output
% _____________________________________________________________________________

@x
@d close_out(#)==close(#) {close an output file}
@d close_in(#)==do_nothing {close an input file}
@y
@d close_out(#)==c4p_fclose(#) {close an output file}
@d close_in(#)==c4p_fclose(#) {close an input file}
@z

 _____________________________________________________________________________
%
% [6.54]
% _____________________________________________________________________________

@x
reset(translate);
@y
c4p_fopen(translate, c4p_argv[4], c4p_r_mode, true);
@z

% _____________________________________________________________________________
%
% [7.67]
% _____________________________________________________________________________

@x
@p procedure collect_count_trie;
@y
@p procedure collect_count_trie;
var miktex_r: real;
@z

@x
  if good_pat_count>0 then
  print_ln(', efficiency = ',
    good_count/(good_pat_count+bad_count/bad_eff):1:2)
@y
  if good_pat_count>0 then begin
    miktex_r := good_count / (good_pat_count + bad_count / bad_eff);
    print_ln(', efficiency = ', miktex_r:1:2)
  end
@z

% _____________________________________________________________________________
%
% [8.87]
% _____________________________________________________________________________

@x
@!filnam: packed array[1..8] of char; {for |pattmp|}
@y
@!filnam: packed array[1..9] of char; {for |pattmp|}
@z

% _____________________________________________________________________________
%
% [8.88]
% _____________________________________________________________________________

@x
  reset(dictionary);@/
@y
  c4p_fopen(dictionary, c4p_argv[1], c4p_w_mode, true);
@z

@x
    begin filnam:='pattmp. ';
@y
    begin c4p_strcpy(filnam, 9, 'pattmp. ');
@z

@x
    rewrite(pattmp,filnam);
@y
    c4p_fopen(pattmp, filnam, c4p_w_mode, true);
@z

% _____________________________________________________________________________
%
% [9.90] Reading patterns
% _____________________________________________________________________________

@x
reset(patterns);
@y
c4p_fopen(patterns, c4p_argv[2], c4p_r_mode, true);
@z

% _____________________________________________________________________________
%
% [10.94] The main program
% _____________________________________________________________________________

@x
@p begin initialize;
@y
@p begin
miktex_process_command_line_options;
c4p_begin_try_block(end_of_PATGEN);
if (c4pargc <> 5) then
 error('Usage: patgen dictionary patterns output translate');
initialize;
@z

@x
rewrite(patout);
@y
c4p_fopen(patout, c4p_argv[3], c4p_w_mode, true);
@z

@x
end_of_PATGEN:
@y
end_of_PATGEN: c4p_end_try_block(end_of_PATGEN)
@z
