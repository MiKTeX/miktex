%% miktex-uptex-adapter.ch
%%
%% Copyright (C) 2021-2024 Christian Schenk
%% 
%% This file is free software; the copyright holder gives
%% unlimited permission to copy and/or distribute it, with or
%% without modifications, as long as this notice is preserved.

% _____________________________________________________________________________
%
% [29.526]
% _____________________________________________________________________________

@x
@p @t\4@>@<Define procedure |scan_file_name_braced|@>@/
procedure scan_file_name;
@y
@p procedure scan_file_name;
@z

% _____________________________________________________________________________
%
% [30.563]
% _____________________________________________________________________________

@x
pack_file_name(nom,aire,"");
if not miktex_open_tfm_file(tfm_file,name_of_file) then abort;
@y
pack_file_name(nom,aire,"");
if not b_open_in(tfm_file) then abort;
@z
