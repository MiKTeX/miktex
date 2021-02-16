%%% miktex-adapter-0.ch:
%%%
%%% Derived from:
%%% tex.ch for C compilation with web2c, derived from various other
%%% change files.  By Tim Morgan, UC Irvine ICS Department, and many
%%% others.

% _____________________________________________________________________________
%
% [2.24]
% _____________________________________________________________________________

@x
for i:=0 to @'176 do xord[xchr[i]]:=i;
@y
for i:=first_text_char to last_text_char do xord[chr(i)]:=invalid_code;
for i:=@'200 to @'377 do xord[xchr[i]]:=i;
for i:=0 to @'176 do xord[xchr[i]]:=i;
@z

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

% _____________________________________________________________________________
%
% [29.536]
% _____________________________________________________________________________

 @x
@!TEX_format_default:packed array[1..format_default_length] of char;
 @y
@!TEX_format_default:packed array[1..format_default_length] of char;

@ @<Set init...@>=
TEX_format_default:='TeXformats:plain.fmt';
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
