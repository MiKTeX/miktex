% Copyright 1996-2025 Han The Thanh, <thanh@pdftex.org>
%
% This file is part of pdfTeX.
%
% pdfTeX is free software; you can redistribute it and/or modify
% it under the terms of the GNU General Public License as published by
% the Free Software Foundation; either version 2 of the License, or
% (at your option) any later version.
%
% pdfTeX is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
%
% You should have received a copy of the GNU General Public License along with
% this program.  If not, see <http://www.gnu.org/licenses/>.

@x tex.ch l.60 - tell weave we want the whole file
\let\maybe=\iffalse
@y 83
\let\maybe=\iftrue
@z

@x tex.ch l.76 - pdftex banner
@d banner==TeX_banner
@d banner_k==TeX_banner_k
@y
@d banner==pdfTeX_banner
@d banner_k==pdfTeX_banner
@z

@x pdftex.web l.1891 - have print_ignored_error respect --file-line-error
  print_nl("ignored error: "); print(#);
@y
  if file_line_error_style_p then print_file_line
  else print_nl("");
  print("ignored error: "); print(#);
@z

% See also tex.ch l.1204, etex.ch l.2226. Not sure what's going on here.
@x pdftex.web l.28025 - restore e-TeX last_node_type removed in tex.ch
last_glue:=max_halfword; last_penalty:=0; last_kern:=0;
@y
last_glue:=max_halfword; last_penalty:=0; last_kern:=0;
last_node_type:=-1;
@z

@x tex.ch l.1228 - table sizes [see also pdftex.web l.5194]
@d frozen_null_font=frozen_control_sequence+11
  {permanent `\.{\\nullfont}'}
@y
@d frozen_null_font=frozen_control_sequence+12+prim_size
  {permanent `\.{\\nullfont}'}
@z

@x tex.ch l.1274 - web2c, then e-TeX, then pdftex integer parameters
@d int_pars=web2c_int_pars {total number of integer parameters}
@#
@d etex_int_base=tex_int_pars {base for \eTeX's integer parameters}
@y
@d pdftex_first_integer_code = web2c_int_pars {base for \pdfTeX's integer parameters}
@z

% start of tex.pch
@x (MLTeX) l.12945
      begin i := char_tag(char_info(f)(c));
@y
      begin i := char_tag(orig_char_info(f)(c));
@z

@x (MLTeX) l.14640
@d is_valid_char(#)==((font_bc[f] <= #) and (# <= font_ec[f]) and
                      char_exists(char_info(f)(#)))
@y
@d is_valid_char(#)==((font_bc[f] <= #) and (# <= font_ec[f]) and
                      char_exists(orig_char_info(f)(#)))
@z

@x (MLTeX) l.14678
function get_charwidth(f: internal_font_number; c: eight_bits): scaled;
begin
    if is_valid_char(c) then
        get_charwidth := char_width(f)(char_info(f)(c))
    else
        get_charwidth := 0;
end;

function get_charheight(f: internal_font_number; c: eight_bits): scaled;
begin
    if is_valid_char(c) then
        get_charheight := char_height(f)(height_depth(char_info(f)(c)))
    else
        get_charheight := 0;
end;

function get_chardepth(f: internal_font_number; c: eight_bits): scaled;
begin
    if is_valid_char(c) then
        get_chardepth := char_depth(f)(height_depth(char_info(f)(c)))
    else
        get_chardepth := 0;
end;
@y
function get_charwidth(f: internal_font_number; c: eight_bits): scaled;
begin
    if is_valid_char(c) then
        get_charwidth := char_width(f)(orig_char_info(f)(c))
    else
        get_charwidth := 0;
end;

function get_charheight(f: internal_font_number; c: eight_bits): scaled;
begin
    if is_valid_char(c) then
        get_charheight := char_height(f)(height_depth(orig_char_info(f)(c)))
    else
        get_charheight := 0;
end;

function get_chardepth(f: internal_font_number; c: eight_bits): scaled;
begin
    if is_valid_char(c) then
        get_chardepth := char_depth(f)(height_depth(orig_char_info(f)(c)))
    else
        get_chardepth := 0;
end;
@z


@x (MLTeX) l.17413
label reswitch, move_past, fin_rule, next_p;
@y
label reswitch, move_past, fin_rule, next_p, found, continue;
@z

@x (MLTeX) l.17462
  if is_valid_char(c) then
      output_one_char(c)
  else
      char_warning(f, c);
  cur_h:=cur_h+char_width(f)(char_info(f)(c));
@y
  if is_valid_char(c) then begin
      output_one_char(c);
      cur_h:=cur_h+char_width(f)(char_info(f)(c));
      goto continue;
  end;
  if mltex_enabled_p then
    @<(\pdfTeX) Output a substitution, |goto continue| if not possible@>;
continue:
@z

% this @x code is modified from tex.web by tex.ch.
@x tex.ch l.3447 - omit block if pdf_font_step[f] nonzero; preserve indentation
    begin if s>0 then
      begin if s=font_size[f] then goto common_ending;
      end
    else begin arith_error:=false;
      if font_size[f]=xn_over_d(font_dsize[f],-s,1000)
      then if not arith_error
        then goto common_ending;
      end;
    end
@y
  begin {preserved}
  if pdf_font_step[f] = 0 then begin
    if s>0 then
      begin if s=font_size[f] then goto common_ending;
      end
    else begin arith_error:=false;
      if font_size[f]=xn_over_d(font_dsize[f],-s,1000)
      then if not arith_error
        then goto common_ending;
      end;
    end
  end
@z

@x tex.ch l.3989 - more things to dump
param_base:=xmalloc_array(integer, font_max);

undump_things(font_check[null_font], font_ptr+1-null_font);
@y
param_base:=xmalloc_array(integer, font_max);

pdf_char_used:=xmalloc_array(char_used_array, font_max);
pdf_font_size:=xmalloc_array(scaled, font_max);
pdf_font_num:=xmalloc_array(integer, font_max);
pdf_font_map:=xmalloc_array(fm_entry_ptr, font_max);
pdf_font_type:=xmalloc_array(eight_bits, font_max);
pdf_font_attr:=xmalloc_array(str_number, font_max);
pdf_font_blink:=xmalloc_array(internal_font_number, font_max);
pdf_font_elink:=xmalloc_array(internal_font_number, font_max);
pdf_font_has_space_char:=xmalloc_array(boolean, font_max);
pdf_font_stretch:=xmalloc_array(integer, font_max);
pdf_font_shrink:=xmalloc_array(integer, font_max);
pdf_font_step:=xmalloc_array(integer, font_max);
pdf_font_expand_ratio:=xmalloc_array(integer, font_max);
pdf_font_auto_expand:=xmalloc_array(boolean, font_max);
pdf_font_lp_base:=xmalloc_array(integer, font_max);
pdf_font_rp_base:=xmalloc_array(integer, font_max);
pdf_font_ef_base:=xmalloc_array(integer, font_max);
pdf_font_kn_bs_base:=xmalloc_array(integer, font_max);
pdf_font_st_bs_base:=xmalloc_array(integer, font_max);
pdf_font_sh_bs_base:=xmalloc_array(integer, font_max);
pdf_font_kn_bc_base:=xmalloc_array(integer, font_max);
pdf_font_kn_ac_base:=xmalloc_array(integer, font_max);
vf_packet_base:=xmalloc_array(integer, font_max);
vf_default_font:=xmalloc_array(internal_font_number, font_max);
vf_local_font_num:=xmalloc_array(internal_font_number, font_max);
vf_e_fnts:=xmalloc_array(integer, font_max);
vf_i_fnts:=xmalloc_array(internal_font_number, font_max);
pdf_font_nobuiltin_tounicode:=xmalloc_array(boolean, font_max);

for font_k := font_base to font_max do begin
    for k := 0 to 31 do
        pdf_char_used[font_k, k] := 0;
    pdf_font_size[font_k] := 0;
    pdf_font_num[font_k] := 0;
    pdf_font_map[font_k] := 0;
    pdf_font_type[font_k] := new_font_type;
    pdf_font_attr[font_k] := "";
    pdf_font_blink[font_k] := null_font;
    pdf_font_elink[font_k] := null_font;
    pdf_font_has_space_char[font_k] := false;
    pdf_font_stretch[font_k] := null_font;
    pdf_font_shrink[font_k] := null_font;
    pdf_font_step[font_k] := 0;
    pdf_font_expand_ratio[font_k] := 0;
    pdf_font_auto_expand[font_k] := false;
    pdf_font_lp_base[font_k] := 0;
    pdf_font_rp_base[font_k] := 0;
    pdf_font_ef_base[font_k] := 0;
    pdf_font_kn_bs_base[font_k] := 0;
    pdf_font_st_bs_base[font_k] := 0;
    pdf_font_sh_bs_base[font_k] := 0;
    pdf_font_kn_bc_base[font_k] := 0;
    pdf_font_kn_ac_base[font_k] := 0;
    pdf_font_nobuiltin_tounicode[font_k] := false;
end;

make_pdftex_banner;
undump_things(font_check[null_font], font_ptr+1-null_font);
@z

@x tex.ch l.4196 - more bounds to set
  setup_bound_var (0)('hash_extra')(hash_extra);
  setup_bound_var (10000)('expand_depth')(expand_depth);

@y
  setup_bound_var (0)('hash_extra')(hash_extra);
  setup_bound_var (10000)('expand_depth')(expand_depth);
  setup_bound_var (72)('pk_dpi')(pk_dpi);
@z

@x tex.ch l.4229 - more constants to check
  const_chk (hash_extra);
  if error_line > ssup_error_line then error_line := ssup_error_line;
@y
  const_chk (hash_extra);
  const_chk (obj_tab_size);
  const_chk (pdf_mem_size);
  const_chk (dest_names_size);
  const_chk (pk_dpi);
  if error_line > ssup_error_line then error_line := ssup_error_line;
@z

@x tex.ch l.4238 - more arrays to xmalloc
  line_stack:=xmalloc_array (integer, max_in_open);
@y
  line_stack:=xmalloc_array (integer, max_in_open);
  eof_seen:=xmalloc_array (boolean, max_in_open);
  grp_stack:=xmalloc_array (save_pointer, max_in_open);
  if_stack:=xmalloc_array (pointer, max_in_open);
@z

@x tex.ch l.4245 - still more arrays to xmalloc
  hyph_link :=xmalloc_array (hyph_pointer, hyph_size);
@y
  hyph_link :=xmalloc_array (hyph_pointer, hyph_size);
  obj_tab:=xmalloc_array (obj_entry, inf_obj_tab_size); {will grow dynamically}
  pdf_mem:=xmalloc_array (integer, inf_pdf_mem_size); {will grow dynamically}
  dest_names:=xmalloc_array (dest_name_entry, inf_dest_names_size); {will grow dynamically}
  pdf_op_buf:=xmalloc_array (eight_bits, pdf_op_buf_size);
  pdf_os_buf:=xmalloc_array (eight_bits, inf_pdf_os_buf_size); {will grow dynamically}
  pdf_os_objnum:=xmalloc_array (integer, pdf_os_max_objs);
  pdf_os_objoff:=xmalloc_array (integer, pdf_os_max_objs);
@z

@x tex.ch l.4356 - more hyphenation initializations
  trie_root:=0; trie_c[0]:=si(0); trie_ptr:=0;
@y
  trie_root:=0; trie_c[0]:=si(0); trie_ptr:=0;
  hyph_root:=0; hyph_start:=0;
@z


@x tex.ch l.4381 - more arrays to xmallox
  param_base:=xmalloc_array(integer, font_max);

  font_ptr:=null_font; fmem_ptr:=7;
@y
param_base:=xmalloc_array(integer, font_max);

pdf_char_used:=xmalloc_array(char_used_array,font_max);
pdf_font_size:=xmalloc_array(scaled,font_max);
pdf_font_num:=xmalloc_array(integer,font_max);
pdf_font_map:=xmalloc_array(fm_entry_ptr,font_max);
pdf_font_type:=xmalloc_array(eight_bits,font_max);
pdf_font_attr:=xmalloc_array(str_number,font_max);
pdf_font_blink:=xmalloc_array(internal_font_number,font_max);
pdf_font_elink:=xmalloc_array(internal_font_number,font_max);
pdf_font_has_space_char:=xmalloc_array(boolean,font_max);
pdf_font_stretch:=xmalloc_array(integer,font_max);
pdf_font_shrink:=xmalloc_array(integer,font_max);
pdf_font_step:=xmalloc_array(integer,font_max);
pdf_font_expand_ratio:=xmalloc_array(integer,font_max);
pdf_font_auto_expand:=xmalloc_array(boolean,font_max);
pdf_font_lp_base:=xmalloc_array(integer,font_max);
pdf_font_rp_base:=xmalloc_array(integer,font_max);
pdf_font_ef_base:=xmalloc_array(integer,font_max);
pdf_font_kn_bs_base:=xmalloc_array(integer, font_max);
pdf_font_st_bs_base:=xmalloc_array(integer, font_max);
pdf_font_sh_bs_base:=xmalloc_array(integer, font_max);
pdf_font_kn_bc_base:=xmalloc_array(integer, font_max);
pdf_font_kn_ac_base:=xmalloc_array(integer, font_max);
vf_packet_base:=xmalloc_array(integer,font_max);
vf_default_font:=xmalloc_array(internal_font_number,font_max);
vf_local_font_num:=xmalloc_array(internal_font_number,font_max);
vf_e_fnts:=xmalloc_array(integer,font_max);
vf_i_fnts:=xmalloc_array(internal_font_number,font_max);
pdf_font_nobuiltin_tounicode:=xmalloc_array(boolean,font_max);

for font_k := font_base to font_max do begin
    for k := 0 to 31 do
        pdf_char_used[font_k, k] := 0;
    pdf_font_size[font_k] := 0;
    pdf_font_num[font_k] := 0;
    pdf_font_map[font_k] := 0;
    pdf_font_type[font_k] := new_font_type;
    pdf_font_attr[font_k] := "";
    pdf_font_blink[font_k] := null_font;
    pdf_font_elink[font_k] := null_font;
    pdf_font_has_space_char[font_k] := false;
    pdf_font_stretch[font_k] := null_font;
    pdf_font_shrink[font_k] := null_font;
    pdf_font_step[font_k] := 0;
    pdf_font_expand_ratio[font_k] := 0;
    pdf_font_auto_expand[font_k] := false;
    pdf_font_lp_base[font_k] := 0;
    pdf_font_rp_base[font_k] := 0;
    pdf_font_ef_base[font_k] := 0;
    pdf_font_kn_bs_base[font_k] := 0;
    pdf_font_st_bs_base[font_k] := 0;
    pdf_font_sh_bs_base[font_k] := 0;
    pdf_font_kn_bc_base[font_k] := 0;
    pdf_font_kn_ac_base[font_k] := 0;
    pdf_font_nobuiltin_tounicode[font_k] := false;
end;

font_ptr:=null_font; fmem_ptr:=7;
make_pdftex_banner;
@z

@x tex.ch l.4438 (do_extension) - i, j, q, r unused by TeX, needed by pdfTeX
var k:integer; {all-purpose integers}
@!p:pointer; {all-purpose pointers}
@y
var i,@!j,@!k:integer; {all-purpose integers}
@!p,@!q,@!r:pointer; {all-purpose pointers}
@z

@x etex.ch l.3182 -etex command line xswitch
@!init if (buffer[loc]="*")and(format_ident=" (INITEX)") then
@y
@!init if (etex_p or(buffer[loc]="*"))and(format_ident=" (INITEX)") then
@z

@x etex.ch l.3185 -etex command line switch
  incr(loc); eTeX_mode:=1; {enter extended mode}
@y
  if (buffer[loc]="*") then incr(loc);
  eTeX_mode:=1; {enter extended mode}
@z

@x etex.ch l.3224 -etex command line switch
@!eTeX_mode: 0..1; {identifies compatibility and extended mode}
@y
@!eTeX_mode: 0..1; {identifies compatibility and extended mode}
@!etex_p: boolean; {was the -etex option specified}
@z

@x etex.ch l.3302 - use pointer for array
@!eof_seen : array[1..max_in_open] of boolean; {has eof been seen?}
@y
@!eof_seen : ^boolean; {has eof been seen?}
@z

@x etex.ch l.4842 - use pointers for arrays
@!grp_stack : array[0..max_in_open] of save_pointer; {initial |cur_boundary|}
@!if_stack : array[0..max_in_open] of pointer; {initial |cond_ptr|}
@y
@!grp_stack : ^save_pointer; {initial |cur_boundary|}
@!if_stack : ^pointer; {initial |cond_ptr|}
@z

@x etex.ch l.6244 - hyphenation initializations done elsewhere
hyph_root:=0; hyph_start:=0;
@y
@z

@x tex.ch l.4858 - effective_char function needed earlier
replacement, but always existing character |font_bc[f]|.
@^inner loop@>

@<Declare additional functions for ML\TeX@>=
function effective_char(@!err_p:boolean;
@y
replacement, but always existing character |font_bc[f]|.
@^inner loop@>

@<Declare \eTeX\ procedures for sc...@>=
function effective_char(@!err_p:boolean;
@z

@x tex.ch l.4958 - pdftex character substitutions
@ The global variables for the code to substitute a virtual character
@y
@ \pdfTeX's |pdf_hlist_out| uses a similar, but slightly modified code
section of the (almost) same name.

@<(\pdfTeX) Output a substitution, |goto continue| if not possible@>=
  begin
  @<Get substitution information, check it, goto |found|
  if all is ok, otherwise goto |continue|@>;
found: @<Print character substitution tracing log@>;
  @<(\pdfTeX) Rebuild character using substitution information@>;
  end

@ The global variables for the code to substitute a virtual character
@z

@x tex.ch l.5071 - more pdftex substitution
@ Dumping ML\TeX-related material.  This is just the flag in the
@y
@ \pdfTeX's |pdf_hlist_out| uses a similar, but slightly modified code
section of the (almost) same name.

@<(\pdfTeX) Rebuild character using substitution information@>=
  base_x_height:=x_height(f);
  base_slant:=slant(f)/float_constant(65536);
@^real division@>
  accent_slant:=base_slant; {slant of accent character font}
  base_width:=char_width(f)(ib_c);
  base_height:=char_height(f)(height_depth(ib_c));
  accent_width:=char_width(f)(ia_c);
  accent_height:=char_height(f)(height_depth(ia_c));
  @/{compute necessary horizontal shift (don't forget slant)}@/
  delta:=round((base_width-accent_width)/float_constant(2)+
            base_height*base_slant-base_x_height*accent_slant);
@^real multiplication@>
@^real addition@>
  @/{1. For centering/horizontal shifting insert a kern node.}@/
  cur_h:=cur_h+delta;
  @/{2. Then insert the accent character possibly shifted up or down.}@/
  if ((base_height<>base_x_height) and (accent_height>0)) then
    begin {the accent must be shifted up or down}
    cur_v:=base_line+(base_x_height-base_height);
    output_one_char(accent_c);
    cur_v:=base_line;
    end
  else begin
    output_one_char(accent_c);
    end;
  cur_h:=cur_h+accent_width;
  @/{3. For centering/horizontal shifting insert another kern node.}@/
  cur_h:=cur_h+(-accent_width-delta);
  @/{4. Output the base character.}@/
  output_one_char(base_c);
  cur_h:=cur_h+base_width;

@ Dumping ML\TeX-related material.  This is just the flag in the
@z
