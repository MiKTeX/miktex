%% miktex-ppltotf.ch
%%
%% Copyright (C) 2021-2022 Christian Schenk
%% 
%% This file is free software; the copyright holder gives
%% unlimited permission to copy and/or distribute it, with or
%% without modifications, as long as this notice is preserved.

% _____________________________________________________________________________
%
% [1.2]
% _____________________________________________________________________________

@x
@d print_ln(#)==write_ln(#)
@y
@d print_ln(#)==write_ln(#)
@d verbose==miktex_get_verbose_flag
@z

@x
var @<Globals in the outer block@>@/
@y
var @<Globals in the outer block@>@/
function miktex_get_verbose_flag : boolean; forward;
@z

@x
  parse_arguments;
@y
@z

@x
  miktex_print_miktex_banner(output);
@y
  miktex_print_enc_string(output);
  miktex_print_miktex_banner(output);
@z

% _____________________________________________________________________________
%
% [2.6]
% _____________________________________________________________________________

@x
  print_ln (version_string);
  print_ln ('process kanji code is ', conststringcast(get_enc_string), '.');
@y
@z

% _____________________________________________________________________________
%
% [12.148] System-dependent changes
% _____________________________________________________________________________

@x
const n_options = 5; {Pascal won't count array lengths for us.}
      usage_help (PPLTOTF_HELP, 'issue@@texjp.org');
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
