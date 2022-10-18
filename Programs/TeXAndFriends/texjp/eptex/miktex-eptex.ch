%% miktex-eptex.ch
%%
%% Copyright (C) 2021-2022 Christian Schenk
%% 
%% This file is free software; the copyright holder gives
%% unlimited permission to copy and/or distribute it, with or
%% without modifications, as long as this notice is preserved.

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
