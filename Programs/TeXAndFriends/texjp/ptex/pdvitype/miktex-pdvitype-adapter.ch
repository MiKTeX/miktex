%% miktex-pdvitype-adapter.ch
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
@d banner=='This is DVItype, Version 3.6' {printed when the program starts}
@y
@d my_name=='dvitype'
@d banner=='This is DVItype, Version 3.6' {printed when the program starts}
@z

% _____________________________________________________________________________
%
% [1.3]
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
% [14.112] System-dependent changes
% _____________________________________________________________________________

@x
This section should be replaced, if necessary, by changes to the program
that are necessary to make \.{DVItype} work at a particular installation.
It is usually best to design your change file so that all changes to
previous sections preserve the section numbering; then everybody's version
will be consistent with the printed program. More extensive changes,
which introduce new sections, can be inserted here; then only the index
itself will get a new section number.
@^system dependencies@>
@y
const n_options = 8; {Pascal won't count array lengths for us.}
      usage_help (DVITYPE_HELP, nil);
    end; {Else it was a flag; |getopt| has already done the assignment.}
@ An element with all zeros always ends the list.
@z
