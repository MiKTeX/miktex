%% miktex-ptex-adapter.ch
%%
%% Copyright (C) 2021-2022 Christian Schenk
%% 
%% This file is free software; the copyright holder gives
%% unlimited permission to copy and/or distribute it, with or
%% without modifications, as long as this notice is preserved.

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
if not miktex_get_quiet_flag then begin
@y
@<Initialize the output...@>=
if src_specials_p or file_line_error_style_p or parse_first_line_p then
  wterm(banner_k)
else
  wterm(banner);
if not miktex_get_quiet_flag then begin
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
% [29.519]
% _____________________________________________________________________________

@x
miktex_reallocate_name_of_file(length(a) + length(n) + length(e) + 1);
@y
miktex_reallocate_name_of_file(length(a) + length(n) + length(e) + 1);
name_of_file:= xmalloc_array (ASCII_code, length(a)+length(n)+length(e)+1);
@z

% _____________________________________________________________________________
%
% [29.523]
% _____________________________________________________________________________

@x
miktex_reallocate_name_of_file(n + (b - a + 1) + format_ext_length + 1);
@y
miktex_reallocate_name_of_file(n + (b - a + 1) + format_ext_length + 1);
name_of_file := xmalloc_array (ASCII_code, n+(b-a+1)+format_ext_length+1);
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
  if miktex_insert_src_special_auto then append_src_special;
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
font_bc:=xmalloc_array(eight_bits, font_max);
font_ec:=xmalloc_array(eight_bits, font_max);
@z

% _____________________________________________________________________________
%
% [51.1333]
% _____________________________________________________________________________

@x
miktex_allocate_memory;
@y
miktex_allocate_memory;
  buffer:=xmalloc_array (ASCII_code, buf_size);
  font_info:=xmalloc_array (fmemory_word, font_mem_size);
@z

% _____________________________________________________________________________
%
% [51.1338]
% _____________________________________________________________________________

@x
fix_date_and_time;@/
@y
fix_date_and_time;@/
@z

@x
  trie_root:=0; trie_c[0]:=si(0); trie_ptr:=0;
@y
  trie_root:=0; trie_c[0]:=si(0); trie_ptr:=0;

  {Allocate and initialize font arrays}
  font_check:=xmalloc_array(four_quarters, font_max);
  font_bc:=xmalloc_array(eight_bits, font_max);
  font_ec:=xmalloc_array(eight_bits, font_max);
  char_base:=xmalloc_array(integer, font_max);
@z

% _____________________________________________________________________________
%
% [54.1387]
% _____________________________________________________________________________

@x
  if miktex_enable_eightbit_chars_p then
@y
  if eight_bit_p then
@z
