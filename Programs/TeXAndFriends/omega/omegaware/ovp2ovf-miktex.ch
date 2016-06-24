%% ovp2ovf-miktex.ch: WEB Change File for OVP2OVF
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
% [2.6]
% _____________________________________________________________________________

@x
reset(vpl_file);
@y
if (c4pargc <> 4) then begin
 print_ln ('Usage: ovp2ovf inputfile outputfile tfmfile'); c4p_exit(1);
end;
c4p_fopen(vpl_file,c4p_argv[1],c4p_r_mode,true);
reset(vpl_file);
@z

% _____________________________________________________________________________
%
% [2.21]
% _____________________________________________________________________________

@x
@!vf_file:packed file of 0..255;
@!tfm_file:packed file of 0..255;
@y
@!vf_file,tfm_file:packed file of 0..255;
@z

% _____________________________________________________________________________
%
% [2.22]
% _____________________________________________________________________________

@x
rewrite(vf_file); rewrite(tfm_file);
@y
if (c4pargc <> 4) then begin
 print_ln ('Usage: ovp2ovf inputfile outputfile tfmfile'); c4p_exit(1);
end;
c4p_fopen(vf_file,c4p_argv[2],c4p_wb_mode,true);
c4p_fopen(tfm_file,c4p_argv[3],c4p_wb_mode,true);
rewrite(vf_file); rewrite(tfm_file);
@z

% _____________________________________________________________________________
%
% [3.24]
% _____________________________________________________________________________

@x
@d first_ord=0 {ordinal number of the smallest element of |char|}
@d last_ord=127 {ordinal number of the largest element of |char|}
@y
@d first_ord=0 {ordinal number of the smallest element of |char|}
@d last_ord=127 {ordinal number of the largest element of |char|}
@z

% _____________________________________________________________________________
%
% [8.141]
% _____________________________________________________________________________

@x
HEX:='0123456789ABCDEF';@/
@y
c4p_arrcpy(HEX,'0123456789ABCDEF');@/
@z
