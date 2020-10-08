%% tie-miktex.ch: WEB change file for Tie
%% 
%% Copyright (C) 2000-2020 Christian Schenk
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
% [1.5]
% _____________________________________________________________________________

@x
#define max_file_index 9
@y
#define max_file_index 100
@z

% _____________________________________________________________________________
%
% [2.11]
% _____________________________________________________________________________

@x
@d nl_mark    @'\n' /* \ASCII{} code used as line end marker */
@y
@d cr_mark    @'\r' /* \ASCII{} code used as carriage return marker */
@d nl_mark    @'\n' /* \ASCII{} code used as line end marker */
@z

% _____________________________________________________________________________
%
% [2.13]
% _____________________________________________________________________________

@x
xchr[nl_mark]='\n';
@y
xchr[cr_mark]='\r';
xchr[nl_mark]='\n';
@z

% _____________________________________________________________________________
%
% [5.27]
% _____________________________________________________________________________

@x
   if (c!=@' ' && c!=tab_mark)
@y
   if (c!=@' ' && c!=tab_mark && c!=cr_mark)
@z

% _____________________________________________________________________________
%
% [7.34]
% _____________________________________________________________________________

@x
    out_file=fopen(out_name,"w");
@y
    if(out_name[0] == '-' && out_name[1] == 0) out_file=stdout;
    else
    out_file=fopen(out_name,"w");
@z
