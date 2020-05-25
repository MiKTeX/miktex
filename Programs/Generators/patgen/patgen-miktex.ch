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

% _____________________________________________________________________________
%
% [6.51] Input and output
% _____________________________________________________________________________

@x
@d close_out(#)==close(#) {close an output file}
@y
@d close_out(#)==c4p_fclose(#) {close an output file}
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
% [10.94] The main program
% _____________________________________________________________________________

@x
@p begin initialize;
@y
@p begin
miktex_process_command_line_options;
c4p_begin_try_block(end_of_PATGEN);
initialize;
@z

@x
end_of_PATGEN:
@y
end_of_PATGEN: c4p_end_try_block(end_of_PATGEN)
@z
