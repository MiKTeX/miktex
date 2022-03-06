%% xetex-miktex-adapter.ch: xetex.web to MiKTeX change file adapter
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
% [8.114]
% _____________________________________________________________________________

@x
@d max_quarterword=@"FFFF {largest allowable value in a |quarterword|}
@d min_halfword==-@"FFFFFFF {smallest allowable value in a |halfword|}
@d max_halfword==@"3FFFFFFF {largest allowable value in a |halfword|}
@y
@d max_quarterword=255 {largest allowable value in a |quarterword|}
@d min_halfword==0 {smallest allowable value in a |halfword|}
@d max_halfword==65535 {largest allowable value in a |halfword|}
@z

% _____________________________________________________________________________
%
% [15.217]
% _____________________________________________________________________________

@x
@d shorthand_def=97 {code definition ( \.{\\chardef}, \.{\\countdef}, etc.~)}
@y
@d shorthand_def=95 {code definition ( \.{\\chardef}, \.{\\countdef}, etc.~)}
@z

% _____________________________________________________________________________
%
% [17.238]
% _____________________________________________________________________________

@x
@d int_base=math_code_base+number_usvs {beginning of region 5}
@y
@d int_base=math_code_base+256 {beginning of region 5}
@z

@x [17.236] l.4960 - first web2c, then e-TeX additional integer parameters
@d tex_int_pars=55 {total number of \TeX's integer parameters}
@y
@d int_pars=55 {total number of integer parameters}
@z

% _____________________________________________________________________________
%
% [29.541]
% _____________________________________________________________________________

@x
var k:0..file_name_size; {index into |name_of_file|}
@y
var k:1..file_name_size; {index into |name_of_file|}
@z

@x
else  begin
  make_utf16_name;
  for k:=0 to name_length16-1 do append_char(name_of_file16[k]);
@y
else  begin for k:=1 to name_length do append_char(xord[name_of_file[k]]);
@z

% _____________________________________________________________________________
%
% [29.548]
% _____________________________________________________________________________

@x
  while not dvi_open_out(dvi_file) do
@y
  while not b_open_out(dvi_file) do
@z

% _____________________________________________________________________________
%
% [30.579]
% _____________________________________________________________________________

@x
check_for_tfm_font_mapping;
if b_open_in(tfm_file) then begin
  file_opened:=true
@y
if not b_open_in(tfm_file) then abort;
@z

% _____________________________________________________________________________
%
% [36.773]
% _____________________________________________________________________________

@x
      begin c:=rem_byte(cur_i); i:=char_info(cur_f)(c);
@y
    begin c:=rem_byte(cur_i); i:=char_info(cur_f)(c);
@z
