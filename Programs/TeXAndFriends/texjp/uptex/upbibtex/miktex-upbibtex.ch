%% miktex-upbibtex.ch:
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
% [2.10] The main program
% _____________________________________________________________________________

@x
initialize;
@y
initialize;
init_kanji;
@z

@x
  print (banner);
  print (' (', conststringcast(get_enc_string), ')');
@y
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
% [14.437]
% _____________________________________________________________________________

@x
procedure x_substring;
label exit;
@y
procedure x_substring;
label exit, 4711;
@z

% _____________________________________________________________________________
%
% [14.438]
% _____________________________________________________________________________

@x
        break;
@y
        goto 4711;
@z

@x
    if sp_end<=tpe then break;
@y
    if sp_end<=tpe then goto 4711;
@z

@x
end;
@y
end;
4711:
@z

% _____________________________________________________________________________
%
% [16.467] System-dependent changes
% _____________________________________________________________________________

@x
const n_options = 7; {Pascal won't count array lengths for us.}
      usage_help (UPBIBTEX_HELP, 'issue@@texjp.org');
    end else if argument_is ('kanji') then begin
      if (not set_enc_string(optarg, nil)) then
        write_ln('Bad kanji encoding "', stringcast(optarg), '".');

    end else if argument_is ('kanji-internal') then begin
      if (not (set_enc_string(nil,optarg) and
               (is_internalEUC or is_internalUPTEX))) then
        write_ln('Bad internal kanji encoding "', stringcast(optarg), '".');

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

@ Kanji-internal option.
@.-kanji-internal@>

@<Define the option...@> =
long_options[current_option].name := 'kanji-internal';
long_options[current_option].has_arg := 1;
long_options[current_option].flag := 0;
long_options[current_option].val := 0;
incr(current_option);

@ An element with all zeros always ends the list.
@y
@z

% _____________________________________________________________________________
%
% [16.477]
% _____________________________________________________________________________

@x
function miktex_get_verbose_flag : boolean; forward;
@y
function miktex_get_verbose_flag : boolean; forward;
function is_internalEUC : boolean; forward;
function is_internalUPTEX : boolean; forward;
@z
