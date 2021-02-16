%%% miktex-adapter-1.ch:
%%%
%%% Derived from:
%%% tex.ch for C compilation with web2c, derived from various other
%%% change files.  By Tim Morgan, UC Irvine ICS Department, and many
%%% others.

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
