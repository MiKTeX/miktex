%% miktex-w2c-adapter.ch: MiKTeX to Web2C adapter
%% 
%% Copyright (C) 2021 Christian
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
miktex_allocate_memory;
@y
miktex_allocate_memory;
REMOVE_THIS_BEGIN
  line_stack:=xmalloc_array (integer, max_in_open);
REMOVE_THIS_END
@z

% _____________________________________________________________________________
%
% [54.1391]
% _____________________________________________________________________________

@x
@!eof_seen : array[1..sup_max_in_open] of boolean; {has eof been seen?}
@y
@!eof_seen : array[1..max_in_open] of boolean; {has eof been seen?}
@z


% _____________________________________________________________________________
%
% [54.1508]
% _____________________________________________________________________________

@x
@!grp_stack : array[0..sup_max_in_open] of save_pointer; {initial |cur_boundary|}
@!if_stack : array[0..sup_max_in_open] of pointer; {initial |cond_ptr|}
@y
@!grp_stack : array[0..max_in_open] of save_pointer; {initial |cur_boundary|}
@!if_stack : array[0..max_in_open] of pointer; {initial |cond_ptr|}
@z
