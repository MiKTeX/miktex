%% miktex-eptex.ch:
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
% [9.131] \[8] Packed data
% _____________________________________________________________________________

@x
@d min_halfhalfword==-@"8000
@d max_halfhalfword==@"7FFF
@y
@d min_halfhalfword==0
@d max_halfhalfword==@"FFFF
@z

% _____________________________________________________________________________
%
% [49.1200]
% _____________________________________________________________________________

@x
procedure scan_delimiter(@!p:pointer;@!r:boolean);
@y
procedure scan_delimiter(@!p:pointer;@!r:integer);
@z

% _____________________________________________________________________________
%
% [54.1419]
% _____________________________________________________________________________

@x
    if j>=0 then k:=setinfileenc(input_file[j],stringcast(name_of_file+1))
    else k:=setstdinenc(stringcast(name_of_file+1));
@y
    if j>=0 then k:=setinfileenc(input_file[j],name_of_file)
    else k:=setstdinenc(name_of_file);
@z

% _____________________________________________________________________________
%
% [62.1786]
% _____________________________________________________________________________

@x
@!isprint_utf8: boolean;
@y
@z
