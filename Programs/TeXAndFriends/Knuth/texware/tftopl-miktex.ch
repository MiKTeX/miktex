%% tftopl-miktex.ch: WEB change file for TFtoPL
%%
%% Copyright (C) 1991-2016 Christian Schenk
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

%% **     7  ******************************************************************

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
% [4.28]
% _____________________________________________________________________________

@x
ASCII_04:=' !"#$%&''()*+,-./0123456789:;<=>?';@/
ASCII_10:='@@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_';@/
ASCII_14:='`abcdefghijklmnopqrstuvwxyz{|}~ ';@/
MBL_string:='MBL'; RI_string:='RI '; RCE_string:='RCE';
@y
c4p_arrcpy(ASCII_04,' !"#$%&''()*+,-./0123456789:;<=>?');@/
c4p_arrcpy(ASCII_10,'@@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_');@/
c4p_arrcpy(ASCII_14,'`abcdefghijklmnopqrstuvwxyz{|}~ ');@/
c4p_arrcpy(MBL_string,'MBL'); c4p_arrcpy(RI_string,'RI '); c4p_arrcpy(RCE_string,'RCE');
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
