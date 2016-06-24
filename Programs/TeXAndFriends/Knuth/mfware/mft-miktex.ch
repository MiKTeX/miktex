%% mft-miktex.ch: WEB change file for MFT
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
@d end_of_MFT = 9999 {go here to wrap it up}
@y
@d end_of_MFT = 9999 {go here to wrap it up}
@d goto_end_of_MFT==c4p_throw(end_of_MFT)
@z

% _____________________________________________________________________________
%
% [2.14]
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
% [3.21]
% _____________________________________________________________________________

@x
rewrite(term_out,'TTY:'); {send |term_out| output to the terminal}
@y
term_out:=output; {send |term_out| output to the terminal}
@z

% _____________________________________________________________________________
%
% [3.24]
% _____________________________________________________________________________

@x
begin reset(mf_file); reset(change_file); reset(style_file);
@y
begin
if c4p_argc <> 5 then
      fatal_error ('Usage: mft inputfile changefile stylefile outputfile');
c4p_fopen(mf_file, c4p_argv[1], c4p_r_mode, true);
c4p_fopen(change_file, c4p_argv[2], c4p_r_mode, true);
c4p_fopen(style_file, c4p_argv[3], c4p_r_mode, true);
reset(mf_file); reset(change_file); reset(style_file);
@z

@x
rewrite(tex_file);
@y
if c4p_argc <> 5 then
      fatal_error ('Usage: mft inputfile changefile stylefile outputfile');
c4p_fopen(tex_file, c4p_argv[4], c4p_w_mode, true);
@z

% _____________________________________________________________________________
%
% [4.31]
% _____________________________________________________________________________

@x
begin goto end_of_MFT;
@y
begin goto_end_of_MFT;
@z

% _____________________________________________________________________________
%
% [11.112] The main program
% _____________________________________________________________________________

@x
@p begin initialize; {beginning of the main program}
print_ln(banner); {print a ``banner line''}
@y
@p begin initialize; {beginning of the main program}
c4p_begin_try_block(end_of_MFT);
print(banner);
miktex_print_miktex_banner(output);
write_ln; {print a ``banner line''}
@z

@x
end_of_MFT:{here files should be closed if the operating system requires it}
@y
end_of_MFT:
c4p_end_try_block(end_of_MFT);
{here files should be closed if the operating system requires it}
@z

% _____________________________________________________________________________
%
% [11.113]
% _____________________________________________________________________________

@x
end {there are no other cases}
@y
end; {there are no other cases}
new_line;
c4p_exit(history);
@z
