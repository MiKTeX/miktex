%% miktex-etex.ch: miktex-etex change file
%% 
%% Copyright (C) 1998-2021 Christian
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
% [51.1332]
% _____________________________________________________________________________

@x
REMOVE_THIS_BEGIN
  line_stack:=xmalloc_array (integer, max_in_open);
  eof_seen:=xmalloc_array (boolean, max_in_open);
  grp_stack:=xmalloc_array (save_pointer, max_in_open);
  if_stack:=xmalloc_array (pointer, max_in_open);
REMOVE_THIS_END
@y
@z

% _____________________________________________________________________________
%
% [54.1379] \[53a] The extended features of \eTeX
% _____________________________________________________________________________

@x
@!init if (etex_p or(buffer[loc]="*"))and(format_ident=" (INITEX)") then
@y
@!init if (miktex_etex_p or(buffer[loc]="*"))and(format_ident=" (INITEX)") then
@z

% _____________________________________________________________________________
%
% [61.1383]
% _____________________________________________________________________________

@x
@!etex_p: boolean; {was the -etex option specified}
@y
@z

% _____________________________________________________________________________
%
% [55.1603]
% _____________________________________________________________________________

@x
function@?miktex_enable_eightbit_chars_p : boolean; forward;@t\2@>@/
@y
function@?miktex_enable_eightbit_chars_p : boolean; forward;@t\2@>@/
function@?miktex_etex_p: boolean; forward;@t\2@>@/
@z
