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
reset(tfm_file); reset(vf_file);
@y
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
rewrite(vpl_file);
@y
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
  print_ln('Sorry, but I can''t go on; are you sure this is a TFM?');
  goto final_end;
@y
  print_ln('Sorry, but I can''t go on; are you sure this is a TFM?');
  c4p_exit(1);
@z

@x
  print_ln('Sorry, but I can''t go on; are you sure this is a VF?');
  goto final_end;
@y
  print_ln('Sorry, but I can''t go on; are you sure this is a VF?');
  c4p_exit(1);
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
@y
@d default_directory_name=='./'
@d default_directory_name_length=2
@z

@x
default_directory:=default_directory_name;
@y
c4p_arrcpy(default_directory, default_directory_name);
@z

% _____________________________________________________________________________
%
% [5.44]
% _____________________________________________________________________________

@x
  if r+4>name_length then vf_abort('Font name too long for me!');
@y
  if r+5>name_length then vf_abort('Font name too long for me!');
@z

@x
cur_name[r+1]:='.'; cur_name[r+2]:='T'; cur_name[r+3]:='F'; cur_name[r+4]:='M'
@y
cur_name[r+1]:='.'; cur_name[r+2]:='T'; cur_name[r+3]:='F'; cur_name[r+4]:='M';
cur_name[r+5]:=chr(0);
@z

% _____________________________________________________________________________
%
% [6.50]
% _____________________________________________________________________________

@x
ASCII_04:=' !"#$%&''()*+,-./0123456789:;<=>?';@/
ASCII_10:='@@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_';@/
ASCII_14:='`abcdefghijklmnopqrstuvwxyz{|}~?';@/
@y
c4p_arrcpy(ASCII_04, ' !"#$%&''()*+,-./0123456789:;<=>?');@/
c4p_arrcpy(ASCII_10, '@@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_');@/
c4p_arrcpy(ASCII_14, '`abcdefghijklmnopqrstuvwxyz{|}~?');@/
@z

@x
MBL_string:='MBL'; RI_string:='RI '; RCE_string:='RCE';
@y
c4p_arrcpy(MBL_string, 'MBL');
c4p_arrcpy(RI_string, 'RI ');
c4p_arrcpy(RCE_string, 'RCE');
@z
