%% tex-miktex-quiet.ch: implement quiet mode
%% 
%% Copyright (C) 1991-2020 Christian Schenk
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
% [5.61]
% _____________________________________________________________________________

@x
if not miktex_is_compatible then
  wterm(banner_k);
else
  wterm(banner);
miktex_print_miktex_banner(term_out); {\MiKTeX: append the \MiKTeX\ version information}
if format_ident=0 then print_ln {\MiKTeX: eliminate misleading `(no format preloaded)'.}
else  begin slow_print(format_ident); print_ln;
  end;
@y
if (not miktex_get_quiet_flag) then begin
  if not miktex_is_compatible then
    wterm(banner_k)
  else
    wterm(banner);
  miktex_print_miktex_banner(term_out); {\MiKTeX: append the \MiKTeX\ version information}
  if format_ident=0 then print_ln {\MiKTeX: eliminate misleading `(no format preloaded)'.}
  else  begin slow_print(format_ident); print_ln;
    end;
end;
@z

% _____________________________________________________________________________
%
% [54.1379]
% _____________________________________________________________________________

@x
@* \[54/\MiKTeX] System-dependent changes for \MiKTeX.
@y
@* \[54/\MiKTeX] System-dependent changes for \MiKTeX.

@ Forward declaration of \MiKTeX\ functions.

@<Declare \MiKTeX\ functions@>=

function miktex_get_quiet_flag : boolean; forward;@t\2@>@/
@z
