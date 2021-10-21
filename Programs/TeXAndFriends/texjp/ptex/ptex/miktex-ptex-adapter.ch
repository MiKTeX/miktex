%% miktex-ptex-adapter.ch: tex-miktex.web to ptex-base.ch adapter
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
% [3.28]
% _____________________________________________________________________________

@x
@ Files can be closed with the \ph\ routine `|close(f)|', which
@y
@ All of the file opening functions are defined in C.

@ Files can be closed with the \ph\ routine `|close(f)|', which
@z

% _____________________________________________________________________________
%
% [5.62]
% _____________________________________________________________________________

@x
@<Initialize the output...@>=
if (not miktex_get_quiet_flag) then begin
@y
@<Initialize the output...@>=
if src_specials_p or file_line_error_style_p or parse_first_line_p then
  wterm(banner_k)
else
  wterm(banner);
if (not miktex_get_quiet_flag) then begin
@z

% _____________________________________________________________________________
%
% [29.517]
% _____________________________________________________________________________

@x
  if (c="/")or(c="\") then
@y
  if IS_DIR_SEP(c) then
@z

% _____________________________________________________________________________
%
% [29.537]
% _____________________________________________________________________________

@x
if not miktex_is_compatible
@y
if src_specials_p or file_line_error_style_p or parse_first_line_p
@z

% _____________________________________________________________________________
%
% [30.550]
% _____________________________________________________________________________

@x
@!font_info:array[font_index] of memory_word;
  {the big collection of font data}
@y
@!font_info: ^fmemory_word;
@z

% _____________________________________________________________________________
%
% [30.551]
% _____________________________________________________________________________

@x
@!char_base:array[internal_font_number] of integer;
@y
@!char_base: ^integer;
@z

% _____________________________________________________________________________
%
% [46.1035]
% _____________________________________________________________________________

@x
  if (miktex_insert_src_special_auto) then append_src_special;
@y
  if (insert_src_special_auto) then append_src_special;
@z

% _____________________________________________________________________________
%
% [50.1308]
% _____________________________________________________________________________

@x
dump_int(@$);@/
@y
libc_free(format_engine);@/
dump_int(@$);@/
@z

% _____________________________________________________________________________
%
% [50.1309]
% _____________________________________________________________________________

@x
if x<>@"4D694B54 then goto bad_fmt; {not a format file}
@y
if x<>@"4D694B54 then goto bad_fmt; {not a format file}
libc_free(format_engine);
@z

% _____________________________________________________________________________
%
% [50.1322]
% _____________________________________________________________________________

@x
if fmem_ptr>font_mem_size then font_mem_size:=fmem_ptr;
@y
if fmem_ptr>font_mem_size then font_mem_size:=fmem_ptr;
font_info:=xmalloc_array(fmemory_word, font_mem_size);
@z

% _____________________________________________________________________________
%
% [50.1324]
% _____________________________________________________________________________

@x
begin {Allocate the font arrays}
@y
begin {Allocate the font arrays}
char_base:=xmalloc_array(integer, font_max);
@z

% _____________________________________________________________________________
%
% [51.1333]
% _____________________________________________________________________________

@x
miktex_allocate_memory;
@y
miktex_allocate_memory;
  font_info:=xmalloc_array (fmemory_word, font_mem_size);
  font_check:=xmalloc_array(four_quarters, font_max);
  char_base:=xmalloc_array(integer, font_max);
@z
