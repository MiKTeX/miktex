%% vftovp-miktex.ch: WEB change file for VFtoVP
%% 
%% Copyright (C) 1991-2019 Christian Schenk
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
%% along with This file; if not, write to the Free Software
%% Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
%% USA.

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
% [1.5]
% _____________________________________________________________________________

@x
@!byte=0..255; {unsigned eight-bit quantity}
@y
@!byte=0..255; {unsigned eight-bit quantity}
@!byte_file=packed file of byte;
@z

% _____________________________________________________________________________
%
% [2.7]
% _____________________________________________________________________________

@x
@!vf_file:packed file of byte;
@y
@!vf_file:byte_file;
@z

% _____________________________________________________________________________
%
% [3.10] Font metric data
% _____________________________________________________________________________

@x
@!tfm_file:packed file of byte;
@y
@!tfm_file:byte_file;
@z

% _____________________________________________________________________________
%
% [3.11]
% _____________________________________________________________________________

@x
@<Set init...@>=
reset(tfm_file); reset(vf_file);
@y
@<Set init...@>=
if (c4pargc <> 4) then begin
  print_ln('Usage: vftovp VFFILE TFMFILE VPLFILE');
  c4p_exit(1);
end;
if (not miktex_open_vf_file(vf_file, c4p_argv[1])) then begin
  print_ln('The VF file could not be found.');
  c4p_exit(1);
end;
if (not miktex_open_tfm_file(tfm_file, c4p_argv[2])) then begin
  print_ln('The TFM file could not be found.');
  c4p_exit(1);
end;
reset(tfm_file);
reset(vf_file);
@z

% _____________________________________________________________________________
%
% [3.21]
% _____________________________________________________________________________

@x
@ @<Set init...@>=
rewrite(vpl_file);
@y
@ @<Set init...@>=
if (c4pargc <> 4) then begin
  print_ln('Usage: vftovp VFFILE TFMFILE VPLFILE');
  c4p_exit(1);
end;
if (not c4p_fopen(vpl_file,c4p_argv[3], c4p_w_mode, false)) then begin
  print_ln('The VPL file could not be written.');
end;
rewrite(vpl_file);
@z

% _____________________________________________________________________________
%
% [4.24]
% _____________________________________________________________________________

@x
@d abort(#)==begin print_ln(#);
  print_ln('Sorry, but I can''t go on; are you sure this is a TFM?');
  goto final_end;
@y
@d abort(#)==begin print_ln(#);
  print_ln('Sorry, but I can''t go on; are you sure this is a TFM?');
  c4p_exit(1);
@z

@x
@d vf_abort(#)==
  begin print_ln(#);
  print_ln('Sorry, but I can''t go on; are you sure this is a VF?');
  goto final_end;
  end
@y
@d vf_abort(#)==
  begin print_ln(#);
  print_ln('Sorry, but I can''t go on; are you sure this is a VF?');
  c4p_exit(1);
  end
@z

% _____________________________________________________________________________
%
% [5.36]
% _____________________________________________________________________________

@x
@<Print the name...@>=
print('MAPFONT ',font_ptr:1,': ');
for k:=font_start[font_ptr]+14 to vf_ptr-1 do print(xchr[vf[k]]);
k:=font_start[font_ptr]+5;
print_ln(' at ',(((vf[k]*256+vf[k+1])*256+vf[k+2])/@'4000000)*real_dsize:2:2,
  'pt')
@y
@<Print the name...@>=
print('MAPFONT ',font_ptr:1,': ');
for k:=font_start[font_ptr]+14 to vf_ptr-1 do print(xchr[vf[k]]);
k:=font_start[font_ptr]+5;
print_ln(' at ',(((vf[k]*256+vf[k+1])*256+vf[k+2])/@'4000000)*real_dsize:2:2,
  'pt')
@z

% _____________________________________________________________________________
%
% [5.39]
% _____________________________________________________________________________

@x
reset(tfm_file,cur_name);
@y
if (not miktex_open_tfm_file(tfm_file, cur_name)) then begin
  print_ln('A TFM file could not be found.');
  c4p_exit(1);
end;
reset(tfm_file);
@z

% _____________________________________________________________________________
%
% [5.42]
% _____________________________________________________________________________

@x
@d default_directory_name=='TeXfonts:' {change this to the correct name}
@d default_directory_name_length=9 {change this to the correct length}

@<Glob...@>=
@!default_directory:packed array[1..default_directory_name_length] of char;

@ @<Set init...@>=
default_directory:=default_directory_name;
@y
@d default_directory_name=='./' {change this to the correct name}
@d default_directory_name_length=2 {change this to the correct length}

@<Glob...@>=
@!default_directory:packed array[1..default_directory_name_length] of char;

@ @<Set init...@>=
c4p_arrcpy(default_directory,default_directory_name);
@z

% _____________________________________________________________________________
%
% [5.44]
% _____________________________________________________________________________

@x
@ The string |cur_name| is supposed to be set to the external name of the
\.{TFM} file for the current font. This usually means that we need to
prepend the name of the default directory, and
to append the suffix `\.{.TFM}'. Furthermore, we change lower case letters
to upper case, since |cur_name| is a \PASCAL\ string.
@^system dependencies@>

@<Move font name into the |cur_name| string@>=
for k:=1 to name_length do cur_name[k]:=' ';
if a=0 then
  begin for k:=1 to default_directory_name_length do
    cur_name[k]:=default_directory[k];
  r:=default_directory_name_length;
  end
else r:=0;
for k:=font_start[font_ptr]+14 to vf_ptr-1 do
  begin incr(r);
  if r+4>name_length then vf_abort('Font name too long for me!');
@.Font name too long for me@>
  if (vf[k]>="a")and(vf[k]<="z") then
      cur_name[r]:=xchr[vf[k]-@'40]
  else cur_name[r]:=xchr[vf[k]];
  end;
cur_name[r+1]:='.'; cur_name[r+2]:='T'; cur_name[r+3]:='F'; cur_name[r+4]:='M'
@y
@ The string |cur_name| is supposed to be set to the external name of the
\.{TFM} file for the current font. This usually means that we need to
prepend the name of the default directory, and
to append the suffix `\.{.TFM}'. Furthermore, we change lower case letters
to upper case, since |cur_name| is a \PASCAL\ string.
@^system dependencies@>

@<Move font name into the |cur_name| string@>=
for k:=1 to name_length do cur_name[k]:=' ';
if a=0 then
  begin for k:=1 to default_directory_name_length do
    cur_name[k]:=default_directory[k];
  r:=default_directory_name_length;
  end
else r:=0;
for k:=font_start[font_ptr]+14 to vf_ptr-1 do
  begin incr(r);
  if r+5>name_length then vf_abort('Font name too long for me!');
@.Font name too long for me@>
  if (vf[k]>="a")and(vf[k]<="z") then
      cur_name[r]:=xchr[vf[k]-@'40]
  else cur_name[r]:=xchr[vf[k]];
  end;
cur_name[r+1]:='.'; cur_name[r+2]:='T'; cur_name[r+3]:='F'; cur_name[r+4]:='M';
cur_name[r+5]:= chr (0);
@z

% _____________________________________________________________________________
%
% [6.50]
% _____________________________________________________________________________

@x
@ @<Set init...@>=
ASCII_04:=' !"#$%&''()*+,-./0123456789:;<=>?';@/
ASCII_10:='@@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_';@/
ASCII_14:='`abcdefghijklmnopqrstuvwxyz{|}~?';@/
for k:=0 to 255 do xchr[k]:='?';
for k:=0 to @'37 do
  begin xchr[k+@'40]:=ASCII_04[k+1];
  xchr[k+@'100]:=ASCII_10[k+1];
  xchr[k+@'140]:=ASCII_14[k+1];
  end;
MBL_string:='MBL'; RI_string:='RI '; RCE_string:='RCE';
@y
@ @<Set init...@>=
c4p_arrcpy(ASCII_04,' !"#$%&''()*+,-./0123456789:;<=>?');@/
c4p_arrcpy(ASCII_10,'@@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_');@/
c4p_arrcpy(ASCII_14,'`abcdefghijklmnopqrstuvwxyz{|}~?');@/
for k:=0 to 255 do xchr[k]:='?';
for k:=0 to @'37 do
  begin xchr[k+@'40]:=ASCII_04[k+1];
  xchr[k+@'100]:=ASCII_10[k+1];
  xchr[k+@'140]:=ASCII_14[k+1];
  end;
c4p_arrcpy(MBL_string,'MBL');
c4p_arrcpy(RI_string,'RI ');
c4p_arrcpy(RCE_string,'RCE');
@z
