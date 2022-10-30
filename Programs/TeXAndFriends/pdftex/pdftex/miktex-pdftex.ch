%% miktex-pdftex.ch
%%
%% Copyright (C) 2021-2022 Christian Schenk
%% 
%% This file is free software; the copyright holder gives
%% unlimited permission to copy and/or distribute it, with or
%% without modifications, as long as this notice is preserved.

% _____________________________________________________________________________
%
% [1.4]
% _____________________________________________________________________________

@x
program TEX; {all file names are defined dynamically}
@y
program PDFTEX; {all file names are defined dynamically}
@z

% _____________________________________________________________________________
%
% [12.181]
% _____________________________________________________________________________

@x
@!k:integer; {index into |mem|, |eqtb|, etc.}
@y
@!k:integer; {index into |mem|, |eqtb|, etc.}
@!font_k:integer; {index into |font_base|, etc.}
@z

% _____________________________________________________________________________
%
% [34.678]
% _____________________________________________________________________________

@x
        pdf_mem := xrealloc_array(pdf_mem, integer, pdf_mem_size);
@y
        pdf_mem := miktex_reallocate(pdf_mem, pdf_mem_size);
@z

% _____________________________________________________________________________
%
% [35.681]
% _____________________________________________________________________________

@x
pdf_os_buf_size := inf_pdf_os_buf_size;
@y
@z

% _____________________________________________________________________________
%
% [35.684]
% _____________________________________________________________________________

@x
        while not b_open_out(pdf_file) do
@y
        while not miktex_open_pdf_file(pdf_file) do
@z

% _____________________________________________________________________________
%
% [35.686]
% _____________________________________________________________________________

@x
        pdf_os_buf := xrealloc_array(pdf_os_buf, eight_bits, pdf_os_buf_size);
@y
        pdf_os_buf := miktex_reallocate(pdf_os_buf, pdf_os_buf_size);
@z

% _____________________________________________________________________________
%
% [36.693]
% _____________________________________________________________________________

@x
               (pdf_font_map[k] = pdf_font_map[f]) and
@y
               (miktex_ptr_equal(pdf_font_map[k], pdf_font_map[f])) and
@z

% _____________________________________________________________________________
%
% [37.698]
% _____________________________________________________________________________

@x
        dest_names := xrealloc_array(dest_names, dest_name_entry, dest_names_size);
@y
        dest_names := miktex_reallocate(dest_names, dest_names_size);
@z

@x
        obj_tab := xrealloc_array(obj_tab, obj_entry, obj_tab_size);
@y
        obj_tab := miktex_reallocate(obj_tab, obj_tab_size);
@z

% _____________________________________________________________________________
%
% [38.712]
% _____________________________________________________________________________

@x
    i := getc(vf_file);
@y
    i := get_byte(vf_file);
@z

% _____________________________________________________________________________
%
% [38.725]
% _____________________________________________________________________________

@x
@ Some functions for processing character packets.
@y
@ Some functions for processing character packets.

@d char_done = 72
@z

% _____________________________________________________________________________
%
% [39.792]
% _____________________________________________________________________________

@x
if pdf_pk_mode <> null then begin
    kpse_init_prog('PDFTEX', fixed_pk_resolution,
                   make_cstring(tokens_to_string(pdf_pk_mode)), nil);
    flush_string;
end else
    kpse_init_prog('PDFTEX', fixed_pk_resolution, nil, nil);
kpse_set_program_enabled (kpse_pk_format, 1, kpse_src_compile);
@y
if pdf_pk_mode <> null then begin
    kpse_init_prog('PDFTEX', fixed_pk_resolution,
                   make_cstring(tokens_to_string(pdf_pk_mode)), 0);
    flush_string;
end else
    kpse_init_prog('PDFTEX', fixed_pk_resolution, 0, 0);
@z

% _____________________________________________________________________________
%
% [39.794]
% _____________________________________________________________________________

@x
    if fixed_pdf_draftmode = 0 then b_close(pdf_file)
@y
    if fixed_pdf_draftmode = 0 then miktex_close_pdf_file(pdf_file)
@z

% _____________________________________________________________________________
%
% [39.806]
% _____________________________________________________________________________

@x
pdf_print("/Producer (pdfTeX-");
@y
pdf_print("/Producer (MiKTeX pdfTeX-");
@z

% _____________________________________________________________________________
%
% [57.1499]
% _____________________________________________________________________________

@x
param_base:=xmalloc_array(integer, font_max);

pdf_char_used:=xmalloc_array(char_used_array, font_max);
pdf_font_size:=xmalloc_array(scaled, font_max);
pdf_font_num:=xmalloc_array(integer, font_max);
pdf_font_map:=xmalloc_array(fm_entry_ptr, font_max);
pdf_font_type:=xmalloc_array(eight_bits, font_max);
pdf_font_attr:=xmalloc_array(str_number, font_max);
pdf_font_blink:=xmalloc_array(internal_font_number, font_max);
pdf_font_elink:=xmalloc_array(internal_font_number, font_max);
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
@y
undump_things(font_check[null_font], font_ptr+1-null_font);
@z

% _____________________________________________________________________________
%
% [57.1503]
% _____________________________________________________________________________

@x
pdf_mem := xrealloc_array(pdf_mem, integer, pdf_mem_size);
@y
pdf_mem := miktex_reallocate(pdf_mem, pdf_mem_size);
@z

% _____________________________________________________________________________
%
% [58.1510]
% _____________________________________________________________________________

@x
REMOVE_THIS_BEGIN
  setup_bound_var (0)('hash_extra')(hash_extra);
  setup_bound_var (10000)('expand_depth')(expand_depth);
  setup_bound_var (72)('pk_dpi')(pk_dpi);
  const_chk (hash_extra);
  const_chk (obj_tab_size);
  const_chk (pdf_mem_size);
  const_chk (dest_names_size);
  const_chk (pk_dpi);
  if error_line > ssup_error_line then error_line := ssup_error_line;

  line_stack:=xmalloc_array (integer, max_in_open);
  eof_seen:=xmalloc_array (boolean, max_in_open);
  grp_stack:=xmalloc_array (save_pointer, max_in_open);
  if_stack:=xmalloc_array (pointer, max_in_open);

  hyph_link :=xmalloc_array (hyph_pointer, hyph_size);
  obj_tab:=xmalloc_array (obj_entry, inf_obj_tab_size); {will grow dynamically}
  pdf_mem:=xmalloc_array (integer, inf_pdf_mem_size); {will grow dynamically}
  dest_names:=xmalloc_array (dest_name_entry, inf_dest_names_size); {will grow dynamically}
  pdf_op_buf:=xmalloc_array (eight_bits, pdf_op_buf_size);
  pdf_os_buf:=xmalloc_array (eight_bits, inf_pdf_os_buf_size); {will grow dynamically}
  pdf_os_objnum:=xmalloc_array (integer, pdf_os_max_objs);
  pdf_os_objoff:=xmalloc_array (integer, pdf_os_max_objs);
REMOVE_THIS_END
@y
@z

@x
main_control; {come to life}
@y
make_pdftex_banner;
main_control; {come to life}
@z

% _____________________________________________________________________________
%
% [58.1515]
% _____________________________________________________________________________

@x
param_base:=xmalloc_array(integer, font_max);

pdf_char_used:=xmalloc_array(char_used_array,font_max);
pdf_font_size:=xmalloc_array(scaled,font_max);
pdf_font_num:=xmalloc_array(integer,font_max);
pdf_font_map:=xmalloc_array(fm_entry_ptr,font_max);
pdf_font_type:=xmalloc_array(eight_bits,font_max);
pdf_font_attr:=xmalloc_array(str_number,font_max);
pdf_font_blink:=xmalloc_array(internal_font_number,font_max);
pdf_font_elink:=xmalloc_array(internal_font_number,font_max);
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
@y
font_ptr:=null_font; fmem_ptr:=7;
@z

% _____________________________________________________________________________
%
% [61.1645] \[53a] The extended features of \eTeX
% _____________________________________________________________________________

@x
@!init if (etex_p or(buffer[loc]="*"))and(format_ident=" (INITEX)") then
@y
@!init if (miktex_etex_p or (buffer[loc]="*"))and(format_ident=" (INITEX)") then
@z

% _____________________________________________________________________________
%
% [61.1649]
% _____________________________________________________________________________

@x
@!etex_p: boolean; {was the -etex option specified}
@y
@z

% _____________________________________________________________________________
%
% [65.1888] \[54/ML\TeX] System-dependent changes for ML\TeX
% _____________________________________________________________________________

@x
@* \[54/ML\TeX] System-dependent changes for ML\TeX.
@y
@* \[54/miktex] System-dependent changes for \MiKTeX-pdf\TeX.

@ @<Set init...@>=

for font_k := font_base to font_max do begin
    for k := 0 to 31 do begin
        pdf_char_used[font_k, k] := 0;
    end;
    pdf_font_size[font_k] := 0;
    pdf_font_num[font_k] := 0;
    pdf_font_map[font_k] := 0;
    pdf_font_type[font_k] := new_font_type;
    pdf_font_attr[font_k] := "";
    pdf_font_blink[font_k] := null_font;
    pdf_font_elink[font_k] := null_font;
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

@ @<Declare \MiKTeX\ functions@>=

function get_nullstr: str_number;
begin
    get_nullstr := "";
end;

function colorstackused: integer; forward;@t\2@>@/
function miktex_etex_p: boolean; forward;@t\2@>@/
function get_resname_prefix : str_number; forward;@t\2@>@/
function getllx: scaled; forward;@t\2@>@/
function getlly: scaled; forward;@t\2@>@/
function geturx: scaled; forward;@t\2@>@/
function getury: scaled; forward;@t\2@>@/
function is_quote_bad: boolean; forward;@t\2@>@/
function matrixused: boolean; forward;@t\2@>@/
function miktex_halt_on_error_p : boolean; forward;@t\2@>@/
function miktex_ptr_equal:boolean; forward;@t\2@>@/
function packet_byte : eight_bits; forward;@t\2@>@/


@* \[54/ML\TeX] System-dependent changes for ML\TeX.
@z

% _____________________________________________________________________________
%
% [65.1894]
% _____________________________________________________________________________

% TODO: TL sync

@x
found: @<Print character substitution tracing log@>;
@y
found:
@z
