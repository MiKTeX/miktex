%% tftopl-miktex.ch: WEB change file for TFtoPL
%%
%% Copyright (C) 1991-2021 Christian Schenk
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
% [1.2]
% _____________________________________________________________________________

@x
  begin print_ln(banner);@/
@y
  begin print(banner);
  miktex_print_miktex_banner(output);
  write_ln;@/
@z

% _____________________________________________________________________________
%
% [1.4]
% _____________________________________________________________________________

@x
@!tfm_size=30000; {maximum length of |tfm| data, in bytes}
@!lig_size=5000; {maximum length of |lig_kern| program, in words}
@!hash_size=5003; {preferably a prime number, a bit larger than the number
  of character pairs in lig/kern steps}
@y
@!tfm_size=100000; {maximum length of |tfm| data, in bytes}
@!lig_size=32510; {maximum length of |lig_kern| program, in words ($<2^{15}$)}
@!hash_size=32579; {preferably a prime number, a bit larger than the number
  of character pairs in lig/kern steps}
@z

% _____________________________________________________________________________
%
% [2.7]
% _____________________________________________________________________________

@x
reset(tfm_file);
@y
if (c4pargc < 2) then begin
  write_ln ('Usage: tftopl inputfile outputfile');
  c4p_exit (1);
end;
if (not miktex_open_tfm_file(tfm_file, c4p_argv[1])) then begin
  write_ln ('tftopl: cannot open input file');
  c4p_exit (1);
end;
@z

% _____________________________________________________________________________
%
% [2.17]
% _____________________________________________________________________________

@x
rewrite(pl_file);
@y
if (c4pargc <> 3) then begin
  write_ln ('Syntax: tftopl TFM-file PL-file'); c4p_exit(1);
end;
c4p_fopen(pl_file,c4p_argv[2],c4p_w_mode,true); rewrite(pl_file);
@z

% _____________________________________________________________________________
%
% [3.20]
% _____________________________________________________________________________

@x
  goto final_end;
@y
  c4p_throw(final_end);
@z

% _____________________________________________________________________________
%
% [4.27]
% _____________________________________________________________________________

@x
@!ASCII_04,@!ASCII_10,@!ASCII_14: packed array [1..32] of char;
  {strings for output in the user's external character set}
@y
@!ASCII_04,@!ASCII_10,@!ASCII_14: packed array [1..32] of char;
  {strings for output in the user's external character set}
@!ASCII_all: packed array[0..256] of char;
@z

% _____________________________________________________________________________
%
% [4.28]
% _____________________________________________________________________________

@x
ASCII_04:=' !"#$%&''()*+,-./0123456789:;<=>?';@/
ASCII_10:='@@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_';@/
ASCII_14:='`abcdefghijklmnopqrstuvwxyz{|}~ ';@/
MBL_string:='MBL'; RI_string:='RI '; RCE_string:='RCE';
@y
c4p_arrcpy(ASCII_04,'  !"#$%&''()*+,-./0123456789:;<=>?');@/
c4p_arrcpy(ASCII_10,' @@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_');@/
c4p_arrcpy(ASCII_14,' `abcdefghijklmnopqrstuvwxyz{|}~ ');@/
c4p_arrcpy(ASCII_all,'  !"#$%&''()*+,-./0123456789:;<=>?@@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~');@/
c4p_arrcpy(MBL_string,'MBL'); c4p_arrcpy(RI_string,'RI '); c4p_arrcpy(RCE_string,'RCE');
@z

% _____________________________________________________________________________
%
% [4.38]
% _____________________________________________________________________________

@x
begin if font_type>vanilla then
  begin tfm[0]:=c; out_octal(0,1)
  end
else if (c>="0")and(c<="9") then
  out(' C ',c-"0":1)
else if (c>="A")and(c<="Z") then
  out(' C ',ASCII_10[c-"A"+2])
else if (c>="a")and(c<="z") then
  out(' C ',ASCII_14[c-"a"+2])
else  begin tfm[0]:=c; out_octal(0,1);
@y
begin if (font_type > vanilla) or (charcode_format = charcode_octal) then
  begin tfm[0]:=c; out_octal(0,1)
  end
else if (charcode_format = charcode_ascii) and (c > " ") and (c <= "~")
        and (c <> "(") and (c <> ")") then
  out(' C ', ASCII_all[c - " " + 1])
{default case, use \.C only for letters and digits}
else if (c>="0")and(c<="9") then
  out(' C ',c-"0":1)
else if (c>="A")and(c<="Z") then
  out(' C ',ASCII_10[c-"A"+2])
else if (c>="a")and(c<="z") then
  out(' C ',ASCII_14[c-"a"+2])
else  begin tfm[0]:=c; out_octal(0,1);
@z

% _____________________________________________________________________________
%
% [7.99]
% _____________________________________________________________________________

@x
@p begin initialize;@/
@y
@p begin
c4p_begin_try_block(final_end);
initialize;@/
@z

@x
final_end:end.
@y
final_end:
c4p_end_try_block(final_end);
end.
@z

% _____________________________________________________________________________
%
% [8.100] System-dependent changes
% _____________________________________________________________________________

@x
@* System-dependent changes.
This section should be replaced, if necessary, by changes to the program
that are necessary to make \.{TFtoPL} work at a particular installation.
It is usually best to design your change file so that all changes to
previous sections preserve the section numbering; then everybody's version
will be consistent with the printed program. More extensive changes,
which introduce new sections, can be inserted here; then only the index
itself will get a new section number.
@^system dependencies@>
@y
@ We use an ``enumerated'' type to store the information.

@<Type...@> =
@!charcode_format_type = charcode_ascii..charcode_default;

@ @<Const...@> =
@!charcode_ascii = 0;
@!charcode_octal = 1;
@!charcode_default = 2;

@ @<Global...@> =
@!charcode_format: charcode_format_type;

@ It starts off as the default, that is, we output letters and digits as
ASCII characters, everything else in octal.

@<Set initial values@> =
charcode_format := charcode_default;
@z
