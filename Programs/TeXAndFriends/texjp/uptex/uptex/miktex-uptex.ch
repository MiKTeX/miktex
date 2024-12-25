%% miktex-uptex.ch
%%
%% Copyright (C) 2021-2024 Christian Schenk
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
% [30.565]
% _____________________________________________________________________________

@x
@p procedure scan_file_name;
@y
@p @t\4@>@<Define procedure |scan_file_name_braced|@>@/
procedure scan_file_name;
@z

% _____________________________________________________________________________
%
% [31.602]
% _____________________________________________________________________________

@x
if not ofm_open_in(tfm_file) then
  if not b_open_in(tfm_file) then abort;
@y
if not miktex_open_ofm_file(tfm_file,name_of_file) then
  if not miktex_open_tfm_file(tfm_file,name_of_file) then abort;
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
