%% omega-miktex-misc.ch:
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

%% ////////////////////////////////////////////////////////////////////////////
%% //                                                                        //
%% //                      INSPIRED BY WEB2C'S TEX.CH                        //
%% //                                                                        //
%% ////////////////////////////////////////////////////////////////////////////

% _____________________________________________________________________________
%
% [29.539]
% _____________________________________________________________________________

@x
@d ensure_output_open_end(#)==while not b_open_out(#) do
@y
@d ensure_output_open_end(#)==while (not miktex_open_dvi_file(#)) do
@z

% _____________________________________________________________________________
%
% [48.1227]
% _____________________________________________________________________________

@x
      break;
@y
      c4p_break_loop;
@z

% _____________________________________________________________________________
%
% [49.1298]
% _____________________________________________________________________________

@x
        break;
@y
        c4p_break_loop;
@z

@x
      if a_open_in(cur_file,kpse_tex_format) then begin
@y
      if miktex_open_onm_file(cur_file, c4p_ptr(name_of_file[2])) then begin
@z

% _____________________________________________________________________________
%
% [55.1470]
% _____________________________________________________________________________

@x
@d decr(#)==#:=#-1
@y

@z
