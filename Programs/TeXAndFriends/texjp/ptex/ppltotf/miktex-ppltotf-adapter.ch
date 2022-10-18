%% miktex-ppltotf-adapter.ch
%%
%% Copyright (C) 2021-2022 Christian Schenk
%% 
%% This file is free software; the copyright holder gives
%% unlimited permission to copy and/or distribute it, with or
%% without modifications, as long as this notice is preserved.

@x
\def\title{PL\lowercase{to}TF}
@y
\def\title{PL$\,$\lowercase{to}$\,$TF changes for C}
@z

% _____________________________________________________________________________
%
% [1.1] Introduction
% _____________________________________________________________________________

@x
@d banner=='This is PLtoTF, Version 3.6' {printed when the program starts}
@y
@d my_name=='pltotf'
@d banner=='This is PLtoTF, Version 3.6' {printed when the program starts}
@z

% _____________________________________________________________________________
%
% [1.2]
% _____________________________________________________________________________

@x
  begin print(banner);
@y
  begin
  parse_arguments;
  print(banner);
@z

% _____________________________________________________________________________
%
% [2.6]
% _____________________________________________________________________________

@x
c4p_fopen(pl_file,c4p_argv[1],c4p_r_mode,true);reset(pl_file);
@y
c4p_fopen(pl_file,c4p_argv[1],c4p_r_mode,true);reset(pl_file);
  print_ln (version_string);
@z

% _____________________________________________________________________________
%
% [12.148] System-dependent changes
% _____________________________________________________________________________

@x
This section should be replaced, if necessary, by changes to the program
that are necessary to make \.{PLtoTF} work at a particular installation.
It is usually best to design your change file so that all changes to
previous sections preserve the section numbering; then everybody's version
will be consistent with the printed program. More extensive changes,
which introduce new sections, can be inserted here; then only the index
itself will get a new section number.
@^system dependencies@>
@y
const n_options = 3; {Pascal won't count array lengths for us.}
      usage_help (PLTOTF_HELP, nil);
    end; {Else it was a flag; |getopt| has already done the assignment.}
@ An element with all zeros always ends the list.
@z
