%% ovf2ovp-miktex.ch: WEB change file for OVF2OVP
%% 
%% Copyright (C) 1998-2016 Christian Schenk
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
reset(tfm_file); reset(vf_file);
@y
if (c4pargc <> 4) then begin
 print_ln('Usage: ovf2ovp inputfile tfmfile outputfile'); c4p_exit(1);
end;
c4p_fopen(vf_file,c4p_argv[1],c4p_rb_mode,true);
c4p_fopen(tfm_file,c4p_argv[2],c4p_rb_mode,true);
reset(tfm_file); reset(vf_file);
@z

% _____________________________________________________________________________
%
% [3.22]
% _____________________________________________________________________________

@x
rewrite(vpl_file);
@y
if (c4pargc <> 4) then begin
 print_ln('Usage: ovf2ovp inputfile tfmfile outputfile'); c4p_exit(1);
end;
c4p_fopen(vpl_file,c4p_argv[3],c4p_w_mode,true);
rewrite(vpl_file);
@z

% _____________________________________________________________________________
%
% [4.25]
% _____________________________________________________________________________

@x
  print_ln('Sorry, but I can''t go on; are you sure this is a OFM?');
  goto final_end;
@y
  print_ln('Sorry, but I can''t go on; are you sure this is a OFM?');
  c4p_exit(1);
@z

% _____________________________________________________________________________
%
% [5.32]
% _____________________________________________________________________________

@x
  print_ln('Sorry, but I can''t go on; are you sure this is a OVF?');
  goto final_end;
@y
  print_ln('Sorry, but I can''t go on; are you sure this is a OVF?');
  c4p_exit(1);
@z

% _____________________________________________________________________________
%
% [5.37]
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
% [5.40]
% _____________________________________________________________________________

@x
reset(tfm_file,cur_name);
@^system dependencies@>
if eof(tfm_file) then
  print_ln('---not loaded, font metric file can''t be opened!')
@y
if not miktex_open_tfm_file(tfm_file,cur_name) then
  print_ln('---not loaded, font metric file can''t be opened!')
@z

% _____________________________________________________________________________
%
% [5.45]
% _____________________________________________________________________________

@x
@d default_directory_name=='TeXfonts:' {change this to the correct name}
@d default_directory_name_length=9 {change this to the correct length}
@y
@d default_directory_name==''
@d default_directory_name_length=0
@z

@x
@<Glob...@>=
@!default_directory:packed array[1..default_directory_name_length] of char;
@y
@z

% _____________________________________________________________________________
%
% [5.46]
% _____________________________________________________________________________

@x
default_directory:=default_directory_name;
@y
do_nothing;
@z

% _____________________________________________________________________________
%
% [5.47]
% _____________________________________________________________________________

@x
if a=0 then begin
  for k:=1 to default_directory_name_length do
    cur_name[k]:=default_directory[k];
  r:=default_directory_name_length;
  end
else r:=0;
for k:=font_start[font_ptr]+14 to vf_ptr-1 do begin
  incr(r);
  if r+4>name_length then vf_abort('Font name too long for me!');
@.Font name too long for me@>
  if (vf[k]>="a")and(vf[k]<="z") then
      cur_name[r]:=xchr[vf[k]-@'40]
  else cur_name[r]:=xchr[vf[k]];
  end;
cur_name[r+1]:='.'; cur_name[r+2]:='T'; cur_name[r+3]:='F'; cur_name[r+4]:='M'
@y
r:=0;
for k:=font_start[font_ptr]+14 to vf_ptr-1 do
  begin incr(r);
  if r+5>name_length then vf_abort('Font name too long for me!');
  if (vf[k]>="a")and(vf[k]<="z") then
      cur_name[r]:=xchr[vf[k]-@'40]
  else cur_name[r]:=xchr[vf[k]];
  end;
cur_name[r+1]:='.'; cur_name[r+2]:='t'; cur_name[r+3]:='f'; cur_name[r+4]:='m';
cur_name[r+5]:= chr (0);
@z

% _____________________________________________________________________________
%
% [6.53]
% _____________________________________________________________________________

@x
ASCII_04:=' !"#$%&''()*+,-./0123456789:;<=>?';@/
ASCII_10:='@@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_';@/
ASCII_14:='`abcdefghijklmnopqrstuvwxyz{|}~?';@/
HEX:='0123456789ABCDEF';@/
@y
c4p_arrcpy(ASCII_04,' !"#$%&''()*+,-./0123456789:;<=>?');@/
c4p_arrcpy(ASCII_10,'@@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_');@/
c4p_arrcpy(ASCII_14,'`abcdefghijklmnopqrstuvwxyz{|}~?');@/
c4p_arrcpy(HEX,'0123456789ABCDEF');@/
@z

@x
MBL_string:='MBL'; RI_string:='RI '; RCE_string:='RCE';
@y
c4p_arrcpy(MBL_string,'MBL'); c4p_arrcpy(RI_string,'RI '); c4p_arrcpy(RCE_string,'RCE');
@z

% _____________________________________________________________________________
%
% [11.155] System-dependent changes
% _____________________________________________________________________________

@x
which introduce new sections, can be inserted here; then only the index
itself will get a new section number.
@^system dependencies@>
@y
which introduce new sections, can be inserted here; then only the index
itself will get a new section number.
@^system dependencies@>

@ @<Types...@>=
@!byte_file=packed file of byte;
@z
