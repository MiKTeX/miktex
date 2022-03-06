%% miktex-uptex.ch
%%
%% Copyright (C) 2021-2022 Christian Schenk
%% 
%% This file is free software; the copyright holder gives
%% unlimited permission to copy and/or distribute it, with or
%% without modifications, as long as this notice is preserved.

% _____________________________________________________________________________
%
% [9.113] \[8] Packed data
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
% [54.1412]
% _____________________________________________________________________________

@x
function@?insert_src_special_auto : boolean; forward;@t\2@>@/
@y
function@?insert_src_special_auto : boolean; forward;@t\2@>@/
function@?isinternalUPTEX : boolean; forward;@t\2@>@/
@z
