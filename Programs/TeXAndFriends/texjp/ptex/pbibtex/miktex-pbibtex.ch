%% miktex-pbibtex.ch:
%% 
%% Copyright (C) 2021-2022 Christian Schenk
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
% [2.10] The main program
% _____________________________________________________________________________

@x
initialize;
@y
initialize;
init_kanji;
@z

@x
  miktex_print_miktex_banner(output);
  print_newline;
  print (banner);
  print (' (', conststringcast(get_enc_string), ')');
@y
  miktex_print_enc_string(output);
  miktex_print_miktex_banner(output);
  print_newline;
@z

@x
  miktex_print_miktex_banner(log_file);
@y
  miktex_print_enc_string(log_file);
  miktex_print_miktex_banner(log_file);
@z

% _____________________________________________________________________________
%
% [8.100]
% _____________________________________________________________________________

@x
  if (not set_enc_string (nil,'EUC')) then uexit(1);
@y
  if (not set_enc_string (0,'EUC')) then uexit(1);
@z

% _____________________________________________________________________________
%
% [8.102]
% _____________________________________________________________________________

@x
init_kanji;
parse_arguments;
@y
@z

% _____________________________________________________________________________
%
% [14.438]
% _____________________________________________________________________________

@x
            break;
@y
            c4p_break_loop;
@z

@x
            break;
@y
            c4p_break_loop;
@z

@x
    if sp_end<=tpe then break;
@y
    if sp_end<=tpe then c4p_break_loop;
@z

% _____________________________________________________________________________
%
% [16.467] System-dependent changes
% _____________________________________________________________________________

@x
const n_options = 6; {Pascal won't count array lengths for us.}
      usage_help (PBIBTEX_HELP, 'issue@@texjp.org');
    end else if argument_is ('kanji') then begin
      if (not set_enc_string(optarg, nil)) then
        write_ln('Bad kanji encoding "', stringcast(optarg), '".');

    end; {Else it was a flag; |getopt| has already done the assignment.}
long_options[current_option].name := 'version';
long_options[current_option].has_arg := 0;
long_options[current_option].flag := 0;
long_options[current_option].val := 0;
incr (current_option);

@ Kanji option.
@.-kanji@>

@<Define the option...@> =
long_options[current_option].name := 'kanji';
long_options[current_option].has_arg := 1;
long_options[current_option].flag := 0;
long_options[current_option].val := 0;
incr(current_option);
begin kpse_set_program_name (argv[0], 'pbibtex');
@y
@z
