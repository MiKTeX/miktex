%% miktex-pdvitype-adapter.ch: dvitype-miktex.web to pdvitype.ch adapter
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
