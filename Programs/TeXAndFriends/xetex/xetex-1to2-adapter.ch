%% xetex-1to2-adapter.ch: xetex-1.web to xetex-2.web adapter
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
% [3.24]
% _____________________________________________________________________________

@x
miktex_initialize_char_tables;
@y
for i:=0 to @'176 do xord[xchr[i]]:=i;
{Set |xprn| for printable ASCII, unless |eight_bit_p| is set.}
for i:=0 to 255 do xprn[i]:=(eight_bit_p or ((i>=" ")and(i<="~")));

{The idea for this dynamic translation comes from the patch by
 Libor Skarvada \.{<libor@@informatics.muni.cz>}
 and Petr Sojka \.{<sojka@@informatics.muni.cz>}. I didn't use any of the
 actual code, though, preferring a more general approach.}

{This updates the |xchr|, |xord|, and |xprn| arrays from the provided
 |translate_filename|.  See the function definition in \.{texmfmp.c} for
 more comments.}
if translate_filename then read_tcx_file;
@z

% _____________________________________________________________________________
%
% [3.26]
% _____________________________________________________________________________

@x
@!name_of_file:packed array[1..file_name_size] of char;@;@/
  {on some systems this may be a \&{record} variable}
@y
@!name_of_file:^text_char;
@z

% _____________________________________________________________________________
%
% [3.30]
% _____________________________________________________________________________

@x
@!buffer:array[0..sup_buf_size] of ASCII_code; {lines of characters being read}
@y
@!buffer:^ASCII_code; {lines of characters being read}
@z

% _____________________________________________________________________________
%
% [3.32]
% _____________________________________________________________________________

@x
is considered an output file the file variable is |term_out|.
@^system dependencies@>

@<Glob...@>=
@!term_in:alpha_file; {the terminal as an input file}
@!term_out:alpha_file; {the terminal as an output file}
@y
is considered an output file the file variable is |term_out|.
@^system dependencies@>

@d term_in==stdin {the terminal as an input file}

@<Glob...@>=
@!term_out:alpha_file; {the terminal as an output file}
@!bound_default:integer; {temporary for setup}
@z

% _____________________________________________________________________________
%
% [5.65]
% _____________________________________________________________________________

@x
  if format_ident=0 then print_ln {\MiKTeX: eliminate misleading `(no format preloaded)'.}
  else  begin slow_print(format_ident); print_ln;
    end;
@y
  if format_ident=0 then print_ln {\MiKTeX: eliminate misleading `(no format preloaded)'.}
  else  begin slow_print(format_ident); print_ln;
    end;
if translate_filename then begin
  wterm(' (');
  fputs(translate_filename, stdout);
  wterm_ln(')');
end;
@z

% _____________________________________________________________________________
%
% [8.114]
% _____________________________________________________________________________

@x
@d max_quarterword=255 {largest allowable value in a |quarterword|}
@y
@d max_quarterword=@"FFFF {largest allowable value in a |quarterword|}
@z

% _____________________________________________________________________________
%
% [18.276]
% _____________________________________________________________________________

@x
@!j:0..sup_buf_size; {index into |buffer|}
@y
@!j:0..buf_size; {index into |buffer|}
@z

% _____________________________________________________________________________
%
% [22.316]
% _____________________________________________________________________________

@x
@!input_file : array[1..sup_max_in_open] of alpha_file;
@y
@!input_file : ^alpha_file;
@z

% _____________________________________________________________________________
%
% [29.535]
% _____________________________________________________________________________

@x
begin k:=0;
@y
begin k:=0;
name_of_file:= xmalloc_array (ASCII_code, length(a)+length(n)+length(e)+1);
@z

% _____________________________________________________________________________
%
% [29.540]
% _____________________________________________________________________________

@x
for j:=1 to n do append_to_name(xord[TEX_format_default[j]]);
@y
name_of_file := xmalloc_array (ASCII_code, n+(b-a+1)+format_ext_length+1);
for j:=1 to n do append_to_name(xord[ucharcast(TEX_format_default[j])]);
@z

@x
  append_to_name(xord[TEX_format_default[j]]);
@y
  append_to_name(xord[ucharcast(TEX_format_default[j])]);
@z

% _____________________________________________________________________________
%
% [29.542]
% _____________________________________________________________________________

@x
var k:1..file_name_size; {index into |name_of_file|}
@y
var k:0..file_name_size; {index into |name_of_file|}
@z

@x
else  begin for k:=1 to name_length do append_char(xord[name_of_file[k]]);
@y
else  begin
  make_utf16_name;
  for k:=0 to name_length16-1 do append_char(name_of_file16[k]);
@z

% _____________________________________________________________________________
%
% [29.549]
% _____________________________________________________________________________

@x
  while (not miktex_open_dvi_file (dvi_file)) do
@y
  while not dvi_open_out(dvi_file) do
@z

% _____________________________________________________________________________
%
% [29.553]
% _____________________________________________________________________________

@x
if eTeX_ex then
  begin; wlog_cr; wlog('entering extended mode');
  end;
end
@y
if eTeX_ex then
  begin; wlog_cr; wlog('entering extended mode');
  end;
end
if translate_filename then begin
  wlog_cr;
  wlog(' (');
  fputs(translate_filename, log_file);
  wlog(')');
  end;
end
@z

% _____________________________________________________________________________
%
% [29.554]
% _____________________________________________________________________________

@x
  if a_open_in(cur_file) then goto done;
@y
  if kpse_in_name_ok(stringcast(name_of_file+1))
     and a_open_in(cur_file, kpse_tex_format) then
    goto done;
@z

% _____________________________________________________________________________
%
% [30.580]
% _____________________________________________________________________________

@x
if not miktex_open_tfm_file(tfm_file,name_of_file) then abort;
@y
if not b_open_in(tfm_file) then abort;
@z

% _____________________________________________________________________________
%
% [32.662]
% _____________________________________________________________________________

@x
  miktex_close_dvi_file(dvi_file);
@y
  b_close(dvi_file);
@z

% _____________________________________________________________________________
%
% [46.1067]
% _____________________________________________________________________________

@x
  if (miktex_insert_src_special_auto) then append_src_special;
@y
  if (insert_src_special_auto) then append_src_special;
@z

% _____________________________________________________________________________
%
% [47.1168]
% _____________________________________________________________________________

@x
begin print_err("Extra "); print_esc("endcsname");
@.Extra \\endcsname@>
help1("I'm ignoring this, since I wasn't doing a \csname.");
@y
begin
if cur_chr = 10 then 
begin
  print_err("Extra "); print_esc("endmubyte");
@.Extra \\endmubyte@>
  help1("I'm ignoring this, since I wasn't doing a \mubyte.");
end else begin
  print_err("Extra "); print_esc("endcsname");
@.Extra \\endcsname@>
  help1("I'm ignoring this, since I wasn't doing a \csname.");
end;  
@z

% _____________________________________________________________________________
%
% [49.1308]
% _____________________________________________________________________________

@x
  if a_open_in(read_file[n]) then read_open[n]:=just_open;
@y
  if open_in_name_ok(stringcast(name_of_file+1))
     and a_open_in(read_file[n], kpse_tex_format) then
    read_open[n]:=just_open;
@z

% _____________________________________________________________________________
%
% [49.1335]
% _____________________________________________________________________________

@x
@!w: four_quarters; {four ASCII codes}
@y
@!format_engine: ^text_char;
@z

% _____________________________________________________________________________
%
% [50.1340]
% _____________________________________________________________________________

@x
dump_int(@$);@/
@y
format_engine:=xmalloc_array(text_char,x+4);
dump_int(@$);@/
@z

% _____________________________________________________________________________
%
% [50.1341]
% _____________________________________________________________________________

@x
undump_int(x);
if x<>@"4D694B54 then goto bad_fmt; {not a format file}
undump_int(x);
if x<>@$ then goto bad_fmt; {check that strings are the same}
@y
undump_int(x);
if x<>@"4D694B54 then goto bad_fmt; {not a format file}
format_engine:=xmalloc_array(text_char, x);
undump_int(x);
if x<>@$ then goto bad_fmt; {check that strings are the same}
@z

% _____________________________________________________________________________
%
% [50.1356]
% _____________________________________________________________________________

@x
begin {Allocate the font arrays}
@y
begin {Allocate the font arrays}
font_bc:=xmalloc_array(eight_bits, font_max);
font_ec:=xmalloc_array(eight_bits, font_max);
@z

% _____________________________________________________________________________
%
% [50.1360]
% _____________________________________________________________________________

@x
undump_int(x);
if (x<>69069)or not eof(fmt_file) then goto bad_fmt
@y
undump_int(x);
if (x<>69069)or feof(fmt_file) then goto bad_fmt
@z

% _____________________________________________________________________________
%
% [51.1365]
% _____________________________________________________________________________

@x
miktex_process_command_line_options;
@y
miktex_process_command_line_options;
  setup_bound_var (15000)('max_strings')(max_strings);
@z

@x
miktex_allocate_memory;
@y
miktex_allocate_memory;
  buffer:=xmalloc_array (ASCII_code, buf_size);
  input_file:=xmalloc_array (alpha_file, max_in_open);
  line_stack:=xmalloc_array (integer, max_in_open);
@z

% _____________________________________________________________________________
%
% [51.1370]
% _____________________________________________________________________________

@x
@!init
if trie_not_ready then begin {initex without format loaded}
  trie_root:=0; trie_c[0]:=si(0); trie_ptr:=0;
end;
tini@/
@y
@!init
if trie_not_ready then begin {initex without format loaded}
  trie_root:=0; trie_c[0]:=si(0); trie_ptr:=0;
  {Allocate and initialize font arrays}
  font_bc:=xmalloc_array(eight_bits, font_max);
  font_ec:=xmalloc_array(eight_bits, font_max);
  param_base[null_font]:=-1;
end;
tini@/
@z

% _____________________________________________________________________________
%
% [54.1445]
% _____________________________________________________________________________

@x
@!eof_seen : array[1..sup_max_in_open] of boolean; {has eof been seen?}
@y
@!eof_seen : array[1..max_in_open] of boolean; {has eof been seen?}
@z

% _____________________________________________________________________________
%
% [54.1559]
% _____________________________________________________________________________

@x
@!grp_stack : array[0..sup_max_in_open] of save_pointer; {initial |cur_boundary|}
@!if_stack : array[0..sup_max_in_open] of pointer; {initial |cond_ptr|}
@y
@!grp_stack : array[0..max_in_open] of save_pointer; {initial |cur_boundary|}
@!if_stack : array[0..max_in_open] of pointer; {initial |cond_ptr|}
@z

% _____________________________________________________________________________
%
% [58.1707]
% _____________________________________________________________________________

@x
if miktex_have_tcx_file_name then begin
@y
if translate_filename then begin
@z

@x
  if miktex_enable_eightbit_chars_p then
@y
  if eight_bit_p then
@z
