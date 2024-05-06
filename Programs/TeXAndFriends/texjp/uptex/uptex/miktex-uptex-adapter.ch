%% miktex-uptex-adapter.ch
%%
%% Copyright (C) 2021-2024 Christian Schenk
%% 
%% This file is free software; the copyright holder gives
%% unlimited permission to copy and/or distribute it, with or
%% without modifications, as long as this notice is preserved.

% _____________________________________________________________________________
%
% [25.366]
% _____________________________________________________________________________

@x
@!p,@!q,@!r:pointer; {for list manipulation}
@!j:0..sup_buf_size; {index into |buffer|}
@y
@!p,@!q,@!r:pointer; {for list manipulation}
@!j:0..buf_size; {index into |buffer|}
@z

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
