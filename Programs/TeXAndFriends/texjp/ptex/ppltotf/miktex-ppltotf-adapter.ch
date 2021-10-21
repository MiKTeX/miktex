%% miktex-ppltotf-adapter.ch: pltotf-miktex.web to ppltotf.ch adapter
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
