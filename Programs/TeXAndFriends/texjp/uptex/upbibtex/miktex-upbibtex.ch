%% miktex-upbibtex.ch
%%
%% Copyright (C) 2021-2022 Christian Schenk
%% 
%% This file is free software; the copyright holder gives
%% unlimited permission to copy and/or distribute it, with or
%% without modifications, as long as this notice is preserved.

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
const n_options = 9; {Pascal won't count array lengths for us.}
      usage_help (UPBIBTEX_HELP, 'issue@@texjp.org');
    end else if argument_is ('kanji') then begin
      if (not set_enc_string(optarg, nil)) then
        write_ln('Bad kanji encoding "', stringcast(optarg), '".');

    end else if argument_is ('guess-input-enc') then begin
        enable_guess_file_enc;

    end else if argument_is ('no-guess-input-enc') then begin
        disable_guess_file_enc;

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
long_options[current_option].name := 'guess-input-enc';
long_options[current_option].has_arg := 0;
long_options[current_option].flag := 0;
long_options[current_option].val := 0;
incr(current_option);
long_options[current_option].name := 'no-guess-input-enc';
long_options[current_option].has_arg := 0;
long_options[current_option].flag := 0;
long_options[current_option].val := 0;
incr(current_option);
@ Kanji-internal option.
@.-kanji-internal@>

@<Define the option...@> =
long_options[current_option].name := 'kanji-internal';
long_options[current_option].has_arg := 1;
long_options[current_option].flag := 0;
long_options[current_option].val := 0;
incr(current_option);

@ An element with all zeros always ends the list.
begin kpse_set_program_name (argv[0], 'upbibtex');
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
