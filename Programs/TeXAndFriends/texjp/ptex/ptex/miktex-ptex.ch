%% miktex-ptex.ch
%%
%% Copyright (C) 2021-2022 Christian Schenk
%% 
%% This file is free software; the copyright holder gives
%% unlimited permission to copy and/or distribute it, with or
%% without modifications, as long as this notice is preserved.

% _____________________________________________________________________________
%
% [5.58]
% _____________________________________________________________________________

@x
@d wterm(#)==write(term_out,#)
@y
@d wterm(#)==write(term_out,#)
@d wterm_char(#)==miktex_write_char(term_out,#)
@z

@x
@d wlog(#)==write(log_file,#)
@y
@d wlog(#)==write(log_file,#)
@d wlog_char(#)==miktex_write_char(log_file,#)
@z

% _____________________________________________________________________________
%
% [5.60]
% _____________________________________________________________________________

@x
term_and_log: begin wterm(xchr[s]); incr(term_offset);
@y
term_and_log: begin wterm_char(xchr[s]); incr(term_offset);
@z

@x
  wlog(xchr[s]); incr(file_offset);
@y
  wlog_char(xchr[s]); incr(file_offset);
@z

@x
log_only: begin wlog(xchr[s]); incr(file_offset);
@y
log_only: begin wlog_char(xchr[s]); incr(file_offset);
@z

@x
term_only: begin wterm(xchr[s]); incr(term_offset);
@y
term_only: begin wterm_char(xchr[s]); incr(term_offset);
@z

@x
othercases write(write_file[selector],xchr[s])
@y
othercases miktex_write_char(write_file[selector],xchr[s])
@z

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
% [29.529]
% _____________________________________________________________________________

@x
miktex_reallocate_name_of_file(length(a) + length(n) + length(e) + 1);
name_of_file:= xmalloc_array (ASCII_code, (length(a)+length(n)+length(e))*4+1);
@y
miktex_reallocate_name_of_file((length(a) + length(n) + length(e)) * 4 + 1);
@z

% _____________________________________________________________________________
%
% [29.533]
% _____________________________________________________________________________

@x
miktex_reallocate_name_of_file(n + (b - a + 1) + format_ext_length + 1);
name_of_file := xmalloc_array (ASCII_code, (n+(b-a+1)+format_ext_length)*4+1);
@y
miktex_reallocate_name_of_file((n + (b - a + 1) + format_ext_length) * 4 + 1);
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
miktex_print_miktex_banner(log_file);
@y
if not miktex_is_compatible
then
  wlog(banner_k)
else
  wlog(banner);
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
    fget; cx:=fbyte;
    font_info[k].hh.lhfield:=tonum(cx); {|kchar_type|}
@y
    fget; cx:=fbyte;
    font_info[k].hh.lh:=tonum(cx); {|kchar_type|}
@z

% _____________________________________________________________________________
%
% [46.1040]
% _____________________________________________________________________________

@x
@!ins_kp:boolean; {whether insert kinsoku penalty}
@y
@!ins_kp:integer; {whether insert kinsoku penalty}
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
font_enc:=xmalloc_array(eight_bits, font_max);
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
  buffer:=xmalloc_array (ASCII_code, buf_size);
  buffer2:=xmalloc_array (ASCII_code, buf_size);
  font_info:=xmalloc_array (memory_word, font_mem_size);
  font_dir:=xmalloc_array(eight_bits, font_max);
  font_enc:=xmalloc_array(eight_bits, font_max);
  font_num_ext:=xmalloc_array(integer, font_max);
  font_check:=xmalloc_array(four_quarters, font_max);
  ctype_base:=xmalloc_array(integer, font_max);
  char_base:=xmalloc_array(integer, font_max);
@y
@z

% _____________________________________________________________________________
%
% [53.1382]
% _____________________________________________________________________________

@x
      if name_of_file then libc_free(name_of_file);
      name_of_file := xmalloc(cur_length*4+1);
@y
      miktex_reallocate_name_of_file(cur_length * 4 + 1);
@z

% _____________________________________________________________________________
%
% [54.1400]
% _____________________________________________________________________________

@x
  if eight_bit_p then
@y
  if miktex_enable_eightbit_chars_p then
@z

% _____________________________________________________________________________
%
% [54.1412]
% _____________________________________________________________________________

@x
function@?insert_src_special_auto : boolean; forward;@t\2@>@/
@y
function@?insert_src_special_auto : boolean; forward;@t\2@>@/
function@?isinternalUPTEX : boolean; forward;@t\2@>@/
function@?isterminalUTF8 : boolean; forward;@t\2@>@/
@z
