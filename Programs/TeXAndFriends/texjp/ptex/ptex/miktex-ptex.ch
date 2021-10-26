%% miktex-ptex.ch:
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
% [5.63]
% _____________________________________________________________________________

@x
if src_specials_p or file_line_error_style_p or parse_first_line_p then
  wterm(banner_k)
else
  wterm(banner);
  wterm(' (');
  wterm(conststringcast(get_enc_string));
  wterm(')');
@y
@z

@x
miktex_print_miktex_banner(term_out);
@y
miktex_print_enc_string(term_out);
miktex_print_miktex_banner(term_out);
@z

% _____________________________________________________________________________
%
% [24.349]
% _____________________________________________________________________________

@x
@!l:0..buf_size; {temporary index into |buffer|}
@y
@!l:0..sup_buf_size; {temporary index into |buffer|}
@z

% _____________________________________________________________________________
%
% [29.526]
% _____________________________________________________________________________

@x
  if (IS_DIR_SEP(c)and(not_kanji_char_seq(prev_char,c))) then
@y
  if ((c="/")or(c="\")and(not_kanji_char_seq(prev_char,c))) then
@z


% _____________________________________________________________________________
%
% [29.546]
% _____________________________________________________________________________

@x
if src_specials_p or file_line_error_style_p or parse_first_line_p
then
  wlog(banner_k)
else
  wlog(banner);
  wlog(' (');
  wlog(conststringcast(get_enc_string));
  wlog(')');
@y
@z

@x
miktex_print_miktex_banner(log_file);
@y
wlog(banner_k);
miktex_print_enc_string(log_file);
miktex_print_miktex_banner(log_file);
@z

% _____________________________________________________________________________
%
% [30.564]
% _____________________________________________________________________________

@x
@d kchar_type_end(#)==#].hh.lhfield
@y
@d kchar_type_end(#)==#].hh.lh
@z

% _____________________________________________________________________________
%
% [30.579]
% _____________________________________________________________________________

@x
    fget; cx:=fbyte; font_info[k].hh.lhfield:=tonum(cx); {|kchar_type|}
@y
    fget; cx:=fbyte; font_info[k].hh.lh:=tonum(cx); {|kchar_type|}
@z

% _____________________________________________________________________________
%
% [46.1044]
% _____________________________________________________________________________

@x
  if (insert_src_special_auto) then append_src_special;
@y
  if (miktex_insert_src_special_auto) then append_src_special;
@z

% _____________________________________________________________________________
%
% [50.1319]
% _____________________________________________________________________________

@x
libc_free(format_engine);@/
@y
@z

% _____________________________________________________________________________
%
% [50.1320]
% _____________________________________________________________________________

@x
libc_free(format_engine);
@y
@z

% _____________________________________________________________________________
%
% [50.1333]
% _____________________________________________________________________________

@x
font_info:=xmalloc_array(memory_word, font_mem_size);
@y
@z

% _____________________________________________________________________________
%
% [50.1335]
% _____________________________________________________________________________

@x
font_dir:=xmalloc_array(eight_bits, font_max);
font_num_ext:=xmalloc_array(integer, font_max);
ctype_base:=xmalloc_array(integer, font_max);
char_base:=xmalloc_array(integer, font_max);
@y
@z

% _____________________________________________________________________________
%
% [51.1344]
% _____________________________________________________________________________

@x
  font_info:=xmalloc_array (memory_word, font_mem_size);
  font_dir:=xmalloc_array(eight_bits, font_max);
  font_num_ext:=xmalloc_array(integer, font_max);
  font_check:=xmalloc_array(four_quarters, font_max);
  ctype_base:=xmalloc_array(integer, font_max);
  char_base:=xmalloc_array(integer, font_max);
@y
@z
