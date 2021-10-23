%% miktex-uptftopl.ch:
%% 
%% Copyright (C) 2021 Christian Schenk
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
procedure initialize; {this procedure gets things started properly}
@y
function is_internalEUC : boolean; forward;
function is_internalUPTEX : boolean; forward;
procedure initialize; {this procedure gets things started properly}
@z

@x
    tfm_file_array := xmalloc_array (byte, 1002 + 4);
    init_kanji;
    parse_arguments;
@y
    init_kanji;
@z

@x
  miktex_print_miktex_banner(output);
@y
  miktex_print_enc_string(output);
  miktex_print_miktex_banner(output);
@z

% _____________________________________________________________________________
%
% [2.7]
% _____________________________________________________________________________

@x
  print_ln (version_string);
  print_ln ('process kanji code is ', conststringcast(get_enc_string), '.');
@y
@z

% _____________________________________________________________________________
%
% [3.19]
% _____________________________________________________________________________

@x
@!tfm_file_array: ^byte; {the input data all goes here}
@y
@z

% _____________________________________________________________________________
%
% [8.100] System-dependent changes
% _____________________________________________________________________________

@x
const n_options = 6; {Pascal won't count array lengths for us.}
      usage_help (UPTFTOPL_HELP, 'issue@@texjp.org');
    end else if argument_is ('kanji') then begin
      if (not set_enc_string(optarg,optarg)) then
        print_ln('Bad kanji encoding "', stringcast(optarg), '".');

    end; {Else it was a flag; |getopt| has already done the assignment.}
@ Kanji option.
@.-kanji@>

@<Define the option...@> =
long_options[current_option].name := 'kanji';
long_options[current_option].has_arg := 1;
long_options[current_option].flag := 0;
long_options[current_option].val := 0;
incr(current_option);

@ An element with all zeros always ends the list.
@y
@z
