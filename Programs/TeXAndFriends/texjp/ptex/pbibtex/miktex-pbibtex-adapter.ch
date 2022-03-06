%% miktex-pbibtex-adapter.ch
%%
%% Copyright (C) 2021-2022 Christian Schenk
%% 
%% This file is free software; the copyright holder gives
%% unlimited permission to copy and/or distribute it, with or
%% without modifications, as long as this notice is preserved.

% _____________________________________________________________________________
%
% [1.1] Introduction
% _____________________________________________________________________________

@x
@d banner=='This is BibTeX, Version 0.99d' {printed when the program starts}
@y
@d my_name=='bibtex'
@d banner=='This is BibTeX, Version 0.99d' {printed when the program starts}
@z

% _____________________________________________________________________________
%
% [2.10] The main program
% _____________________________________________________________________________

@x
  miktex_print_miktex_banner(output);
  print_newline;
@y
  miktex_print_miktex_banner(output);
  print_newline;
  print (banner);
@z

% _____________________________________________________________________________
%
% [8.102]
% _____________________________________________________________________________

@x
if (c4p_argc <> 2) then begin
@y
parse_arguments;
if (c4p_argc <> 2) then begin
@z

% _____________________________________________________________________________
%
% [16.467] System-dependent changes
% _____________________________________________________________________________

@x
This section should be replaced, if necessary, by changes to the program
that are necessary to make \BibTeX\ work at a particular installation.
It is usually best to design your change file so that all changes to
previous sections preserve the section numbering; then everybody's version
will be consistent with the printed program. More extensive changes,
which introduce new sections, can be inserted here; then only the index
itself will get a new section number.
@y
const n_options = 4; {Pascal won't count array lengths for us.}
      usage_help (BIBTEX_HELP, nil);
    end; {Else it was a flag; |getopt| has already done the assignment.}
long_options[current_option].name := 'version';
long_options[current_option].has_arg := 0;
long_options[current_option].flag := 0;
long_options[current_option].val := 0;
incr (current_option);
begin kpse_set_program_name (argv[0], 'bibtex');
@z
