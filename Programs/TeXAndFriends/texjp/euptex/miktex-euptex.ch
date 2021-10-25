%% miktex-euptex.ch:
%% 
%% Copyright (C) 2021 Christian Schenk
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
% [54.1419]
% _____________________________________________________________________________

@x
    if j>=0 then k:=setinfileenc(input_file[j],stringcast(name_of_file+1))
    else k:=setstdinenc(stringcast(name_of_file+1));
@y
    if j>=0 then k:=setinfileenc(input_file[j],name_of_file)
    else k:=setstdinenc(name_of_file);
@z

@x
      term_and_log: begin wterm(stringcast(name_of_file + 1));
        wlog(stringcast(name_of_file + 1)); end;
      log_only:  wlog(stringcast(name_of_file + 1));
      term_only: wterm(stringcast(name_of_file + 1));
@y
      term_and_log: begin wterm(name_of_file);
        wlog(name_of_file); end;
      log_only:  wlog(name_of_file);
      term_only: wterm(name_of_file);
@z

% _____________________________________________________________________________
%
% [62.1780] \[54/pdf\TeX] System-dependent changes for {\tt\char"5Cpdfstrcmp}
% _____________________________________________________________________________

@x
@!isprint_utf8: boolean;
@y
@z
