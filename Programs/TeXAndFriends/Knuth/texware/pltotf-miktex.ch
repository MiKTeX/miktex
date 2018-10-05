%% pltotf-miktex.ch: WEB change file for PLtoTF
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

% _____________________________________________________________________________
%
% [2.6]
% _____________________________________________________________________________

@x
reset(pl_file);
@y
if (c4pargc < 2) then begin
  write_ln ('Usage: pltotf inputfile outputfile'); c4p_exit(1);
end;
c4p_fopen(pl_file,c4p_argv[1],c4p_r_mode,true);reset(pl_file);
@z

% _____________________________________________________________________________
%
% [2.16]
% _____________________________________________________________________________

@x
@ On some systems you may have to do something special to write a
packed file of bytes. For example, the following code didn't work
when it was first tried at Stanford, because packed files have to be
opened with a special switch setting on the \PASCAL\ that was used.
@^system dependencies@>

@<Set init...@>=
rewrite(tfm_file);
@y
@ On some systems you may have to do something special to write a
packed file of bytes.
@^system dependencies@>

@<Set init...@>=
if (c4pargc <> 3) then begin
  write_ln ('Usage: pltotf inputfile outputfile'); c4p_exit(1);
end;
c4p_fopen(tfm_file,c4p_argv[2],c4p_wb_mode,true);rewrite(tfm_file);
@z
