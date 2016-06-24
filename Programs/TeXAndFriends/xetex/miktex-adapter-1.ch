%% miktex-adapter-1.ch:
%% 
%% Copyright (C) 2003-2016 Christian Schenk
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
% [8.114]
% _____________________________________________________________________________

@x
@d max_quarterword==255 {largest allowable value in a |quarterword|}
@y
@d max_quarterword=@"FFFF {largest allowable value in a |quarterword|}
@z

% _____________________________________________________________________________
%
% [29.542]
% _____________________________________________________________________________

@x
var k:1..file_name_size; {index into |name_of_file|}
@y
var k:0..file_name_size; {index into |name_of_file|}
@z

@x
else  begin for k:=1 to name_length do append_char(xord[name_of_file[k]]);
@y
else  begin
  make_utf16_name;
  for k:=0 to name_length16-1 do append_char(name_of_file16[k]);
@z
