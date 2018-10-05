% omxml.ch: Generate XML, MathML, HTML, etc.
%
% This file is part of the Omega project, which
% is based on the web2c distribution of TeX.
%
% Copyright (c) 1994--2000 John Plaice and Yannis Haralambous
% 
% This library is free software; you can redistribute it and/or
% modify it under the terms of the GNU Library General Public
% License as published by the Free Software Foundation; either
% version 2 of the License, or (at your option) any later version.
% 
% This library is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
% Library General Public License for more details.
% 
% You should have received a copy of the GNU Library General Public
% License along with this library; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
%
%---------------------------------------
@x [1] m.11 l.392 - Omega
@!font_max=65535; {maximum internal font number; must be at most |font_biggest|}
@y
@!font_sort_max=65535; {maximum internal font sort number}
@!font_max=65535; {maximum internal font number; must be at most |font_biggest|}
@z
%---------------------------------------
@x [1] m.12 l.437 - Omega
@d number_fonts=font_biggest-font_base+2
@y
@d number_fonts=font_biggest-font_base+2
@d font_sort_base=0
@d font_sort_biggest=65535 {the biggest font sort}
@d number_font_sorts=font_sort_biggest-font_sort_base+2
@z
%---------------------------------------
@x [5] m.57 l.1446 - Omega
othercases write_ln(write_file[selector])
@y
othercases if selector>max_selector then
    write_ln(output_files[selector-max_selector])
  else
    write_ln(write_file[selector])
@z
%---------------------------------------
@x [5] m.58 l.1465 - Omega
othercases omega_file_write(selector)
@y
othercases if selector>max_selector then
    write(output_files[selector-max_selector],xchr[s])
  else
    omega_file_write(selector)
@z
%---------------------------------------
@x [10] m.160 l.3270 - Omega
@ In fact, there are still more types coming. When we get to math formula
processing we will see that a |style_node| has |type=14|; and a number
of larger type codes will also be defined, for use in math mode only.
@y
@ In fact, there are still more types coming. When we get to math formula
processing we will see that a |style_node| has |type=14|; and a number
of larger type codes will also be defined, for use in math mode only.

@d sgml_text_node=0
@d sgml_math_node=1

@d sgml_node=unset_node+1 {|type| for an SGML node}
@d sgml_node_size=5
@d sgml_tag(#)==mem[#+1].int
@d sgml_attrs(#)==mem[#+2].int
@d sgml_singleton(#)==info(#+3)
@d sgml_info(#)==link(#+3)
@d sgml_kind(#)==mem[#+4].int

@d sgml_attr_node=unset_node+2 {|type| for an SGML attribute node}
@d sgml_attr_node_size=4

@d sgml_entity_node=unset_node+3 {|type| for an SGML entity node}
@d sgml_entity_node_size=2
@d sgml_entity_string(#)==mem[#+1].int

@d biggest_ordinary_node=sgml_entity_node

@p
function new_sgml_node:pointer;
var p:pointer;
begin p:=get_node(sgml_node_size);
type(p):=sgml_node;
sgml_tag(p):=0;
sgml_attrs(p):=0;
sgml_info(p):=0;
sgml_singleton(p):=0;
sgml_kind(p):=0;
new_sgml_node:=p;
end;

function new_sgml_attr_node:pointer;
var p:pointer;
begin p:=get_node(sgml_attr_node_size);
type(p):=sgml_attr_node;
sgml_tag(p):=0;
sgml_attrs(p):=0;
sgml_info(p):=0;
sgml_singleton(p):=0;
new_sgml_attr_node:=p;
end;

function new_sgml_entity_node:pointer;
var p:pointer;
begin p:=get_node(sgml_entity_node_size);
type(p):=sgml_entity_node;
sgml_entity_string(p):=0;
new_sgml_entity_node:=p;
end;
@z
%---------------------------------------
@x [12] m.174 l.3520 - Omega
@p procedure short_display(@!p:integer); {prints highlights of list |p|}
var n:integer; {for replacement counts}
begin while p>mem_min do
  begin if is_char_node(p) then
    begin if p<=mem_end then
      begin if font(p)<>font_in_short_display then
        begin if (font(p)<font_base)or(font(p)>font_max) then
          print_char("*")
@.*\relax@>
        else @<Print the font identifier for |font(p)|@>;
        print_char(" "); font_in_short_display:=font(p);
        end;
      print_ASCII(qo(character(p)));
@y
@p procedure short_display(@!p:integer); {prints highlights of list |p|}
var n:integer; {for replacement counts}
    fsort:integer;
begin while p>mem_min do
  begin if is_char_node(p) then
    begin if p<=mem_end then
      begin if font(p)<>font_in_short_display then
        begin if (font(p)<font_base)or(font(p)>font_max) then
          print_char("*")
@.*\relax@>
        else @<Print the font identifier for |font(p)|@>;
        print_char(" "); font_in_short_display:=font(p);
        end;
      if not SGML_show_entities then
        print_ASCII(qo(character(p)))
      else begin
        fsort:=font_name_sort(font(p));
        if fsort<>0 then begin
          if (font_sort_char_entity(fsort)(character(p))<>0) then
            slow_print(font_sort_char_entity(fsort)(character(p)))
          else
            print_ASCII(qo(character(p)));
          end
        else
          print_ASCII(qo(character(p)));
        end;
@z
%---------------------------------------
@x [12] m.176 l.3563 - Omega
@p procedure print_font_and_char(@!p:integer); {prints |char_node| data}
begin if p>mem_end then print_esc("CLOBBERED.")
else  begin if (font(p)<font_base)or(font(p)>font_max) then print_char("*")
@.*\relax@>
  else @<Print the font identifier for |font(p)|@>;
  print_char(" "); print_ASCII(qo(character(p)));
  end;
end;
@y
@p procedure print_font_and_char(@!p:integer); {prints |char_node| data}
var fsort:integer;
begin if p>mem_end then print_esc("CLOBBERED.")
else  begin if (font(p)<font_base)or(font(p)>font_max) then print_char("*")
@.*\relax@>
  else @<Print the font identifier for |font(p)|@>;
  print_char(" ");
  if not SGML_show_entities then
    print_ASCII(qo(character(p)))
  else begin
    fsort:=font_name_sort(font(p));
    if fsort<>0 then begin
      if (font_sort_char_entity(fsort)(character(p))<>0) then
        print(font_sort_char_entity(fsort)(character(p)))
      else
        print_ASCII(qo(character(p)));
      end
    else
      print_ASCII(qo(character(p)));
    end;
  end;
end;
@z
%---------------------------------------
@x [13] m.202 l.3928 - Omega
    adjust_node: flush_node_list(adjust_ptr(p));
@y
    adjust_node: flush_node_list(adjust_ptr(p));
    sgml_node: begin
      if sgml_tag(p)<>0 then flush_node_list(sgml_attrs(p));
      free_node(p,sgml_node_size); goto done;
      end;
    sgml_attr_node: begin
      free_node(p,sgml_attr_node_size); goto done;
      end;
    sgml_entity_node: begin
      free_node(p,sgml_entity_node_size); goto done;
      end;
@z
%---------------------------------------
@x [15] m.209 l.4138 - Omega
@d max_command=char_mode
   {the largest command code seen at |big_switch|}
@y
@d set_show_sgml_entities=char_mode+1
   {printing hex code or SGML-like entity during \.{\\showbox}}
@d set_mml_mode=set_show_sgml_entities+1
   {for entering or leaving MML mode ( \.{\\mmlmode}~)}
@d mml_tag=set_mml_mode+1
   {for various SGML tags}
@d max_command=mml_tag
   {the largest command code seen at |big_switch|}
@z
%---------------------------------------
@x [16] m.212--217 l.4303 - Omega
  @!pg_field,@!ml_field: integer;@+
@y
  @!pg_field,@!ml_field: integer;@+
  @!sgml_field: integer;
  @!sgml_attr_field: pointer;
@z
%---------------------------------------
@x [16] m.212--217 l.4303 - Omega
@d mode_line==cur_list.ml_field {source file line number at beginning of list}
@y
@d mode_line==cur_list.ml_field {source file line number at beginning of list}
@d current_sgml_tag==cur_list.sgml_field {current SGML tag}
@d current_sgml_attrs==cur_list.sgml_attr_field {current SGML tag}
@z
%---------------------------------------
@x [16] m.212--217 l.4303 - Omega
prev_graf:=0; shown_mode:=0;
@y
prev_graf:=0; shown_mode:=0;
current_sgml_tag:=0; current_sgml_attrs:=null;
@z
%---------------------------------------
@x [16] m.212--217 l.4303 - Omega
incr(nest_ptr); head:=get_avail; tail:=head; prev_graf:=0; mode_line:=line;
@y
incr(nest_ptr); head:=get_avail; tail:=head; prev_graf:=0; mode_line:=line;
current_sgml_tag:=0; current_sgml_attrs:=null;
@z
%---------------------------------------
@x [16] m.212--217 l.4303 - Omega
@p procedure pop_nest; {leave a semantic level, re-enter the old}
begin
if local_par<>null then begin
  if local_par_bool then begin end {tail_append(local_par)}
  else free_node(local_par,local_par_size);
  end;
free_avail(head); decr(nest_ptr); cur_list:=nest[nest_ptr];
end;
@y
@p procedure pop_nest; {leave a semantic level, re-enter the old}
var attrs:pointer;
    {p:pointer;}
begin attrs:=null;
if local_par<>null then begin
  if local_par_bool then begin end {tail_append(local_par)}
  else free_node(local_par,local_par_size);
  end;
{
if current_sgml_attrs<>null then
  if current_sgml_tag=null then
    attrs:=current_sgml_attrs
  else
    free_avail(current_sgml_attrs);
}
free_avail(head); decr(nest_ptr); cur_list:=nest[nest_ptr];
{
if attrs<>null then begin
  p:=attrs;
  while link(p)<>null do
    begin
    p:=link(p);
    end;
  link(p):=current_sgml_attrs;
  current_sgml_attrs:=attrs;
  end;
}
end;
@z
%---------------------------------------
@x [17] m.222 l.4525 - Omega
@d frozen_null_ocp=frozen_null_font+number_fonts
@y
@d frozen_null_font_sort=frozen_null_font+number_font_sorts {permanent null sort}
@d frozen_null_ocp=frozen_null_font_sort+number_font_sorts
@z
%---------------------------------------
@x [17] m.232 l.4802 - Omega
@d var_code==@"7000000 {math code meaning ``use the current family''}
@y
@d null_font_sort==font_sort_base
@d var_code==@"7000000 {math code meaning ``use the current family''}
@z
%---------------------------------------
@x [19] m.269 l.5859 - Omega
@d math_left_group=16 {code for `\.{\\left...\\right}'}
@d local_box_group=17 {code for `\.{\\localleftbox...\\localrightbox}'}
@d max_group_code=17
@y
@d math_left_group=16 {code for `\.{\\left...\\right}'}
@d math_mml_group=17 {code for automatic `\.{\\left...\\right}'}
@d text_mml_group=18 {code for `\.{\\text...}'}
@d text_sgml_group=19 {code for SGML tags}
@d font_entity_group=20 
@d empty_tag_group=21 
@d lone_tag_group=22 
@d local_box_group=23 {code for `\.{\\localleftbox...\\localrightbox}'}
@d max_group_code=23
@z
%---------------------------------------
@x [29] m.532 l.10260 - Omega
@ Here's an example of how these conventions are used. Whenever it is time to
ship out a box of stuff, we shall use the macro |ensure_dvi_open|.

@d ensure_dvi_open==if output_file_name=0 then
  begin if job_name=0 then open_log_file;
  pack_job_name(".dvi");
  while not b_open_out(dvi_file) do
    prompt_file_name("file name for output",".dvi");
  output_file_name:=b_make_name_string(dvi_file);
  end

@<Glob...@>=
@!dvi_file: byte_file; {the device-independent output goes here}
@!output_file_name: str_number; {full name of the output file}
@!log_name:str_number; {full name of the log file}
@y
@ Here's an example of how these conventions are used. Whenever it is time to
ship out a box of stuff, we shall use the macro |ensure_dvi_open|.

@d ensure_output_open_end(#)==while not b_open_out(#) do
  prompt_file_name("file name for output",output_m_suffix);
  output_m_name:=b_make_name_string(#);
  end end

@d ensure_output_open_middle(#)==output_m_name:=#; if #=0 then
  begin if job_name=0 then open_log_file;
  pack_job_name(output_m_suffix);
  ensure_output_open_end

@d ensure_output_open(#)==begin output_m_suffix:=#; ensure_output_open_middle

@d ensure_dvi_open==begin
  ensure_output_open(".dvi")(output_file_name)(dvi_file);
  output_file_name:=output_m_name end

@<Glob...@>=
@!dvi_file: byte_file; {the device-independent output goes here}
@!output_file_name: str_number; {full name of the dvi output file}
@!output_m_suffix: str_number; {suffix for the current output file}
@!output_m_name: str_number; {suffix for the current output file}
@!output_file_names:array[1..10] of str_number;
@!output_files:array[1..10] of byte_file;
@!output_file_no:integer; {number of open output files}
@!log_name:str_number; {full name of the log file}
@z
%---------------------------------------
@x [29] m.533 l.10260 - Omega
@ @<Initialize the output...@>=output_file_name:=0;
@y
@ @<Initialize the output...@>=output_file_name:=0;
for output_file_no:=1 to 10 do output_file_names[output_file_no]:=0;
output_file_no:=0;
@z
%---------------------------------------
@x [30] m.549 l.10686 - Omega
@!font_ptr:internal_font_number; {largest internal font number in use}
@y
@!font_ptr:internal_font_number; {largest internal font number in use}
@!font_sort_ptr:integer; {largest internal font sort number in use}
@z
%---------------------------------------
@x [30] m.550 l.10723 - Omega
@d param_base(#)==font_info(#)(offset_param_base).int
@y
@d param_base(#)==font_info(#)(offset_param_base).int

@d font_sort_info_end(#)==#]
@d font_sort_info(#)==font_sort_tables[#,font_sort_info_end
@d font_sort_offset_file_size=0
@d font_sort_offset_name=font_sort_offset_file_size+1
@d font_sort_offset_bc=font_sort_offset_name+1
@d font_sort_offset_ec=font_sort_offset_bc+1
@d font_sort_offset_char_base=font_sort_offset_bc+1
@d font_sort_file_size(#)==font_sort_info(#)(font_sort_offset_file_size).int
@d font_sort_name(#)==font_sort_info(#)(font_sort_offset_name).int
@d font_sort_bc(#)==font_sort_info(#)(font_sort_offset_bc).int
@d font_sort_ec(#)==font_sort_info(#)(font_sort_offset_ec).int
@d font_sort_char_base(#)==font_sort_info(#)(font_sort_offset_char_base).int
@d font_sort_char_entity_end(#)==#*3].int
@d font_sort_char_entity(#)==
   font_sort_tables[#,font_sort_offset_char_base+font_sort_char_entity_end
@d font_sort_char_tag_end(#)==#*3+1].int
@d font_sort_char_tag(#)==
   font_sort_tables[#,font_sort_offset_char_base+font_sort_char_tag_end
@d font_sort_char_attr_end(#)==#*3+2].int
@d font_sort_char_attr(#)==
   font_sort_tables[#,font_sort_offset_char_base+font_sort_char_attr_end
@z
%---------------------------------------
@x [30] m.550 l.10723 - Omega
@!dimen_font:integer;
@y
@!dimen_font:integer;
@!font_sorts:integer; {to run through font sorts}
@!font_sort_c:integer; {character used to define font entities}
@!font_sort_number:integer; {this particular font sort number}
@z
%---------------------------------------
@x [30] m.552 l.10749 - Omega
for k:=1 to 7 do font_info(null_font)(param_base(null_font)+k).sc:=0;
@y
for k:=1 to 7 do font_info(null_font)(param_base(null_font)+k).sc:=0;
font_sort_ptr:=null_font_sort;
allocate_font_sort_table(null_font_sort,font_sort_offset_char_base);
font_sort_file_size(null_font_sort):=font_sort_offset_char_base;
font_sort_name(null_font_sort):="nullfontsort";
@z
%---------------------------------------
@x [32] m.638 l.12656 - Omega
@<Ship box |p| out@>;
@y
if not MML_mode then begin @<Ship box |p| out@>; end;
@z
%---------------------------------------
@x [34] m.682 l.13395 - Omega
@d ord_noad=unset_node+3 {|type| of a noad classified Ord}
@y
@d ord_noad=biggest_ordinary_node+3 {|type| of a noad classified Ord}
@z
%---------------------------------------
@x [34] m.688 l.13515 - Omega
@d style_node=unset_node+1 {|type| of a style node}
@y
@d style_node=biggest_ordinary_node+1 {|type| of a style node}
@z
%---------------------------------------
@x [34] m.688 l.13515 - Omega
@d choice_node=unset_node+2 {|type| of a choice node}
@y
@d choice_node=biggest_ordinary_node+2 {|type| of a choice node}
@z
%---------------------------------------
@x [46] m.1030 
hmode+spacer: if space_factor=1000 then goto append_normal_space
  else app_space;
hmode+ex_space,mmode+ex_space: goto append_normal_space;
@y
hmode+spacer:
  if MML_mode then begin
    make_sgml_entity(" ");
    goto big_switch;
    end
  else if space_factor=1000 then goto append_normal_space
  else app_space;
hmode+ex_space,mmode+ex_space: goto append_normal_space;
@z
%---------------------------------------
@x [47] m.1060 l.20533 - Omega
procedure append_glue;
var s:small_number; {modifier of skip command}
begin s:=cur_chr;
case s of
fil_code: cur_val:=fil_glue;
fill_code: cur_val:=fill_glue;
ss_code: cur_val:=ss_glue;
fil_neg_code: cur_val:=fil_neg_glue;
skip_code: scan_glue(glue_val);
mskip_code: scan_glue(mu_val);
end; {now |cur_val| points to the glue specification}
tail_append(new_glue(cur_val));
if s>=skip_code then
  begin decr(glue_ref_count(cur_val));
  if s>skip_code then subtype(tail):=mu_glue;
  end;
end;
@y
procedure append_glue;
var s:small_number; {modifier of skip command}
begin s:=cur_chr;
case s of
fil_code: cur_val:=fil_glue;
fill_code: cur_val:=fill_glue;
ss_code: cur_val:=ss_glue;
fil_neg_code: cur_val:=fil_neg_glue;
skip_code: scan_glue(glue_val);
mskip_code: scan_glue(mu_val);
end; {now |cur_val| points to the glue specification}
if (abs(mode)=mmode) and MML_mode then begin
  end
else begin
  tail_append(new_glue(cur_val));
  if s>=skip_code then
    begin decr(glue_ref_count(cur_val));
    if s>skip_code then subtype(tail):=mu_glue;
    end;
  end;
end;
@z
%---------------------------------------
@x [47] m.1061 l.20339 - Omega
procedure append_kern;
var s:quarterword; {|subtype| of the kern node}
begin s:=cur_chr; scan_dimen(s=mu_glue,false,false);
tail_append(new_kern(cur_val)); subtype(tail):=s;
end;
@y
procedure append_kern;
var s:quarterword; {|subtype| of the kern node}
begin s:=cur_chr; scan_dimen(s=mu_glue,false,false);
if (abs(mode)=mmode) and MML_mode then begin
  end
else begin
  tail_append(new_kern(cur_val)); subtype(tail):=s;
  end;
end;
@z
%---------------------------------------
@x [47] m.1073 l.20795 - Omega
vmode+hmove,hmode+vmove,mmode+vmove: begin t:=cur_chr;
  scan_normal_dimen;
  if t=0 then scan_box(cur_val)@+else scan_box(-cur_val);
  end;
any_mode(leader_ship): scan_box(leader_flag-a_leaders+cur_chr);
any_mode(make_box): begin_box(0);
@y
vmode+hmove,hmode+vmove,mmode+vmove: begin
  if abs(mode)=mmode and MML_mode then begin
    print_err("Unauthorized entry in math expression: ");
    print_cmd_chr(cur_cmd,cur_chr); print_ln;
    print_nl("The MathML translator cannot continue");
    succumb;
    end
  else begin
    t:=cur_chr; scan_normal_dimen;
    if t=0 then scan_box(cur_val)@+else scan_box(-cur_val);
    end;
  end;
any_mode(leader_ship): begin
  if abs(mode)=mmode and MML_mode then begin
    print_err("Unauthorized entry in math expression: ");
    print_cmd_chr(cur_cmd,cur_chr); print_ln;
    print_nl("The MathML translator cannot continue");
    succumb;
    end
  else
    scan_box(leader_flag-a_leaders+cur_chr);
  end;
any_mode(make_box): begin
  if abs(mode)=mmode and MML_mode then begin
    print_err("Unauthorized entry in math expression: ");
    print_cmd_chr(cur_cmd,cur_chr); print_ln;
    print_nl("The MathML translator cannot continue");
    succumb;
    end
  else
    begin_box(0);
  end;
@z
%---------------------------------------
@x [47] m. l.21055 - Omega
procedure new_graf(@!indented:boolean);
var p,q,dir_graf_tmp:pointer;
begin prev_graf:=0;
if (mode=vmode)or(head<>tail) then
  tail_append(new_param_glue(par_skip_code));
push_nest; mode:=hmode; space_factor:=1000; set_cur_lang; clang:=cur_lang;
prev_graf:=(norm_min(left_hyphen_min)*@'100+norm_min(right_hyphen_min))
             *@'200000+cur_lang;
@<LOCAL: Add local paragraph node@>;
if indented then begin
  p:=new_null_box; box_dir(p):=par_direction;
  width(p):=par_indent;@+
  tail_append(p);
  end;
q:=tail;
dir_rover:=text_dir_ptr;
while dir_rover<>null do
  begin
  if (link(dir_rover)<>null) or (dir_dir(dir_rover)<>par_direction) then
    begin
    dir_graf_tmp:=new_dir(dir_dir(dir_rover));
    link(dir_graf_tmp):=link(q); link(q):=dir_graf_tmp;
    end;
  dir_rover:=link(dir_rover);
  end;
q:=head;
while link(q)<>null do q:=link(q);
tail:=q;
if every_par<>null then begin_token_list(every_par,every_par_text);
if nest_ptr=1 then build_page; {put |par_skip| glue on current page}
end;
@y
procedure sgml_start_graf; forward;

procedure new_graf(@!indented:boolean);
var p,q,dir_graf_tmp:pointer;
begin prev_graf:=0;
if (mode=vmode)or(head<>tail) then
  tail_append(new_param_glue(par_skip_code));
{if MML_mode then begin
  sgml_start_graf
  end
else} begin
push_nest; mode:=hmode; space_factor:=1000; set_cur_lang; clang:=cur_lang;
prev_graf:=(norm_min(left_hyphen_min)*@'100+norm_min(right_hyphen_min))
             *@'200000+cur_lang;
@<LOCAL: Add local paragraph node@>;
if indented then begin
  p:=new_null_box; box_dir(p):=par_direction;
  width(p):=par_indent;@+
  tail_append(p);
  end;
q:=tail;
dir_rover:=text_dir_ptr;
while dir_rover<>null do
  begin
  if (link(dir_rover)<>null) or (dir_dir(dir_rover)<>par_direction) then
    begin
    dir_graf_tmp:=new_dir(dir_dir(dir_rover));
    link(dir_graf_tmp):=link(q); link(q):=dir_graf_tmp;
    end;
  dir_rover:=link(dir_rover);
  end;
q:=head;
while link(q)<>null do q:=link(q);
tail:=q;
if every_par<>null then begin_token_list(every_par,every_par_text);
if nest_ptr=1 then build_page; {put |par_skip| glue on current page}
end;
end;
@z
%---------------------------------------
@x [47] m.1100 l.21156 - Omega
procedure end_graf;
begin if mode=hmode then
  begin if (head=tail) or (link(head)=tail) then pop_nest
        {null paragraphs are ignored, all contain a |local_paragraph| node}
  else line_break(widow_penalty);
  if dir_save<>null then
    begin flush_node_list(dir_save); dir_save:=null;
    end;
  normal_paragraph;
  error_count:=0;
  end;
end;
@y
procedure sgml_end_graf; forward;

procedure end_graf;
begin if mode=hmode then
  begin if (head=tail) or (link(head)=tail) then pop_nest
        {null paragraphs are ignored, all contain a |local_paragraph| node}
  {else if MML_mode then begin sgml_end_graf end}
  else line_break(widow_penalty);
  if dir_save<>null then
    begin flush_node_list(dir_save); dir_save:=null;
    end;
  normal_paragraph;
  error_count:=0;
  end;
end;
@z
%---------------------------------------
@x [47] m.1130 l.21562 - Omega
vmode+halign,hmode+valign:init_align;
mmode+halign: if privileged then
  if cur_group=math_shift_group then init_align
  else off_save;
vmode+endv,hmode+endv: do_endv;
@y
vmode+halign,hmode+valign:init_align;
mmode+halign: if MML_mode then begin
    print_err("Unauthorized entry in math expression: ");
    print_esc("halign"); print_ln;
    print_nl("The MathML translator cannot continue");
    succumb;
    end
  else begin
    if privileged then
    if cur_group=math_shift_group then init_align
    else off_save;
    end;
vmode+endv,hmode+endv: do_endv;
@z
%---------------------------------------
@x [48] m.1136 l.21605 - Omega
@* \[48] Building math lists.
The routines that \TeX\ uses to create mlists are similar to those we have
just seen for the generation of hlists and vlists. But it is necessary to
make ``noads'' as well as nodes, so the reader should review the
discussion of math mode data structures before trying to make sense out of
the following program.

Here is a little routine that needs to be done whenever a subformula
is about to be processed. The parameter is a code like |math_group|.

@<Declare act...@>=
procedure push_math(@!c:group_code);
begin
if math_direction<>text_direction then dir_math_save:=true;
push_nest; mode:=-mmode; incompleat_noad:=null;
@<DIR: |new_save_level| math@>;
end;
@y
@* \[48] Building math lists.
The routines that \TeX\ uses to create mlists are similar to those we have
just seen for the generation of hlists and vlists. But it is necessary to
make ``noads'' as well as nodes, so the reader should review the
discussion of math mode data structures before trying to make sense out of
the following program.

Here is a little routine that needs to be done whenever a subformula
is about to be processed. The parameter is a code like |math_group|.

@<Glob...@>=
@!MML_mode:boolean;
@!SGML_show_entities:boolean;
@!MML_level:integer;
@!mml_file_no:integer; {the \.{MML} output file}
@!mml_indent:integer;
@!mml_depth_level:integer;
@!mml_last_print_char:boolean;

@ @<Set init...@>=
MML_mode:=false;
SGML_show_entities:=true;
MML_level:=0;
mml_indent:=0;
mml_file_no:=0;
mml_depth_level:=0;
mml_last_print_char:=false;

@ 

@d sgml_out_pointer(#)==
  case math_type(#(p)) of
    math_char: begin
      fetch(#(p));
      fsort:=font_name_sort(cur_f);
      if char_exists(cur_i) then begin
        if (font_sort_char_tag(fsort)(cur_c)<>0) then begin
          if not inside_mop then begin
            for i:=1 to mml_indent do print("  ");
            print("<"); print(font_sort_char_tag(fsort)(cur_c));
            if (font_sort_char_attr(fsort)(cur_c)<>null) then begin
              sgml_attrs_out(font_sort_char_attr(fsort)(cur_c));
              end;
            print("> "); 
            end;
          if (font_sort_char_entity(fsort)(cur_c)<>0) then begin
            print(font_sort_char_entity(fsort)(cur_c));
            end;
          if not inside_mop then begin
            print(" </"); print(font_sort_char_tag(fsort)(cur_c));
            print(">"); print(new_line_char);
            back_at_bol:=true;
            end;
          end;
        end;
      end;
    sub_mlist: begin
      case type(p) of
        op_noad,bin_noad,rel_noad,
        open_noad,close_noad,punct_noad,inner_noad: begin
          for i:=1 to mml_indent do print("  ");
          print("<mo");
          if (type(p)=op_noad) and (subtype(p)=limits) then begin
            print(" limits=""true""");
            end
          else if (type(p)=op_noad) and (subtype(p)=no_limits) then begin
            print(" limits=""false""");
            end;
          print("> ");
          q:=p; cur_mlist:=info(#(p));
          if link(cur_mlist)=null then
            if type(cur_mlist)=sgml_node then
              if str_eq_str(sgml_tag(cur_mlist),"mrow") then
                cur_mlist:=sgml_info(cur_mlist);
          sgml_out_mlist(false,false,true);  p:=q;
          print(" </mo>");
          print(new_line_char);
          back_at_bol:=true;
          end;
        othercases begin
          q:=p; cur_mlist:=info(#(p));
          sgml_out_mlist(false,false,false);  p:=q;
          end
        endcases;
      end;
    othercases begin
      for i:=1 to mml_indent do print("  ");
      print("<merror> Unrecognized math stuff </merror>");
      print(new_line_char);
      end
    endcases;

@<Declare act...@>=
procedure push_math(@!c:group_code);
begin
if math_direction<>text_direction then dir_math_save:=true;
push_nest; mode:=-mmode; incompleat_noad:=null;
@<DIR: |new_save_level| math@>;
end;

function sgml_out_on_one_line(p:pointer):boolean;
begin
if (sgml_tag(p)="mi") or (sgml_tag(p)="mo") or (sgml_tag(p)="mn") then
  sgml_out_on_one_line:=true
else sgml_out_on_one_line:=false;
end;

procedure sgml_attrs_out(p:pointer);
var q:pointer;
begin
q:=p;
while q<>null do begin
  print(" ");
  print(sgml_tag(q)); print("=""");
  print(sgml_attrs(q)); print("""");
  q:=link(q);
  end;
end;

procedure sgml_out_mlist(break_line,inside_text,inside_mop:boolean);
var p,q:pointer;
    old_selector:integer;
    i,fsort:integer;
    back_at_bol:boolean;
    first_inside_text:boolean;
begin
old_selector:=selector;
selector:=max_selector+mml_file_no;
p:=cur_mlist;
back_at_bol:=false;
first_inside_text:=inside_text;
while p<>null do begin
  if is_char_node(p) then begin
    fsort:=font_name_sort(font(p));
    if (font_sort_char_entity(fsort)(character(p))<>0) then begin
      if back_at_bol then
        for i:=1 to mml_indent do print("  ");
      print(font_sort_char_entity(fsort)(character(p)));
      back_at_bol:=false;
      mml_last_print_char:=true;
      first_inside_text:=false;
      end;
    end
  else begin
    case type(p) of
      hlist_node,vlist_node,rule_node,
      ins_node,mark_node,adjust_node: do_nothing;
      ligature_node: begin
        fsort:=font_name_sort(font(lig_char(p)));
        if (font_sort_char_entity(fsort)(character(lig_char(p)))<>0) then begin
          if back_at_bol then
            for i:=1 to mml_indent do print("  ");
          print(font_sort_char_entity(fsort)(character(lig_char(p))));
          back_at_bol:=false;
          end;
        mml_last_print_char:=false;
        first_inside_text:=false;
        end;
      disc_node,whatsit_node,math_node,glue_node,
      kern_node,penalty_node,unset_node: do_nothing;
      sgml_node: begin
        if sgml_tag(p)=0 then begin
          print(sgml_attrs(p));
          end
        else if sgml_singleton(p)>=1 then begin
          if not (first_inside_text) then begin
            if inside_text then print(new_line_char);
            for i:=1 to mml_indent do print("  ");
            end;
          print("<"); print(sgml_tag(p));
          sgml_attrs_out(sgml_attrs(p));
          if sgml_singleton(p)=1 then print("/");
          print(">");
          print(new_line_char);
          back_at_bol:=true;
          end
        else begin
          if not (first_inside_text) then begin
            if inside_text and not(back_at_bol) then print(new_line_char);
            for i:=1 to mml_indent do print("  ");
            end;
          print("<"); print(sgml_tag(p));
          sgml_attrs_out(sgml_attrs(p)); print(">");
          if sgml_out_on_one_line(p) then
            print(" ")
          else begin mml_indent:=mml_indent+1;
            print(new_line_char);
            end;
          cur_mlist:=sgml_info(p);
          if cur_mlist<>null then begin
            if (sgml_kind(p)=sgml_text_node) or
               (sgml_tag(p)="mtext") then begin
              for i:=1 to mml_indent do print("  ");
              sgml_out_mlist(false,true,false);
              if mml_last_print_char then print(new_line_char);
              end
            else sgml_out_mlist(false,false,false);
            end;
          if sgml_out_on_one_line(p) then
            print(" ")
          else begin
            mml_indent:=mml_indent-1;
            for i:=1 to mml_indent do print("  ");
            end;
          print("</"); print(sgml_tag(p)); print(">");
          print(new_line_char);
          back_at_bol:=true;
          end;
        mml_last_print_char:=false;
        first_inside_text:=false;
        end;
      sgml_entity_node:
      if not(first_inside_text and 
             (str_eq_str(sgml_entity_string(p)," "))) then begin
          if back_at_bol then
            for i:=1 to mml_indent do print("  ");
          print(sgml_entity_string(p));
          first_inside_text:=false;
          mml_last_print_char:=true;
          back_at_bol:=false;
          if break_line then begin
            print(new_line_char);
            back_at_bol:=true;
          end;
        end;
      fraction_noad: begin
        for i:=1 to mml_indent do print("  ");
        print("<mfrac> Arguments </mfrac>"); print(new_line_char);
        back_at_bol:=true;
        mml_last_print_char:=false;
        first_inside_text:=false;
        end;
      othercases begin
        if (info(subscr(p))<>null) or
           (info(supscr(p))<>null) then begin
          if inside_text then print(new_line_char);
          for i:=1 to mml_indent do print("  ");
          print("<m");
          if info(subscr(p))<>empty then print("sub");
          if info(supscr(p))<>empty then print("sup");
          print(">");
          mml_indent:=mml_indent+1;
          print(new_line_char);
          end;
        sgml_out_pointer(nucleus);
        if (info(subscr(p))<>null) or
           (info(supscr(p))<>null) then begin
          if info(subscr(p))<>null then begin
            sgml_out_pointer(subscr);
            end;
          if info(supscr(p))<>null then begin
            sgml_out_pointer(supscr);
            end;
          mml_indent:=mml_indent-1;
          for i:=1 to mml_indent do print("  ");
          print("</m");
          if info(subscr(p))<>null then print("sub");
          if info(supscr(p))<>null then print("sup");
          print(">");
          print(new_line_char);
          back_at_bol:=true;
          end;
        mml_last_print_char:=false;
        first_inside_text:=false;
        end;
      endcases;
    end;
  p:=link(p);
  end;
selector:=old_selector;
end;

procedure make_sgml_entity(s:str_number);
begin
link(tail):=new_sgml_entity_node;
tail:=link(tail);
sgml_entity_string(tail):=s;
end;

procedure scan_font_entity;
var cv1,cv2,cv3,cv4:integer;
begin
scan_string_argument; cv1:=cur_val;
scan_left_brace; scan_int; cv2:=cur_val; scan_right_brace;
scan_string_argument; cv3:=cur_val;
scan_string_argument; cv4:=cur_val;
save_ptr:=save_ptr+4;
saved(-1):=cv1; saved(-2):=cv2; saved(-3):=cv3; saved(-4):=cv4;
new_save_level(font_entity_group); scan_left_brace; push_nest;
end;

procedure scan_empty_tag;
begin
scan_string_argument;
link(tail):=new_sgml_node;
tail:=link(tail);
sgml_tag(tail):=cur_val;
sgml_kind(tail):=sgml_math_node;
sgml_singleton(tail):=1;
incr(save_ptr); saved(-1):=tail;
new_save_level(empty_tag_group); scan_left_brace; push_nest;
end;

procedure scan_lone_tag;
begin
scan_string_argument;
link(tail):=new_sgml_node;
tail:=link(tail);
sgml_tag(tail):=cur_val;
sgml_kind(tail):=sgml_math_node;
sgml_singleton(tail):=2;
incr(save_ptr); saved(-1):=tail;
new_save_level(lone_tag_group); scan_left_brace; push_nest;
end;

procedure sgml_startmathtag(s:str_number);
begin
push_math(math_group); current_sgml_tag:=s;
end;


procedure sgml_starttexttag(s:str_number);
begin
push_nest; new_save_level(text_sgml_group);
current_sgml_tag:=s;
mode:=-hmode;
end;


procedure sgml_attribute(s,s1:str_number);
var p:pointer;
begin
p:=new_sgml_attr_node;
sgml_tag(p):=s;
sgml_attrs(p):=s1;
sgml_singleton(p):=0;
link(p):=current_sgml_attrs;
current_sgml_attrs:=p;
end;

procedure sgml_endtexttag(s:str_number);
var q:pointer;
begin
if not str_eq_str(s,current_sgml_tag) then begin
  print_err("Tags do not match: ");
  print(current_sgml_tag); print(","); print(s);
  print_ln;
  print_nl("The MathML translator cannot continue");
  succumb;
  end;
q:=new_sgml_node;
sgml_tag(q):=s;
sgml_kind(q):=sgml_text_node;
sgml_info(q):=link(head);
sgml_attrs(q):=current_sgml_attrs;
pop_nest;
unsave;
if mode=vmode then begin
  ensure_mml_open;
  cur_mlist:=q;
  sgml_out_mlist(false,false,false);
  end
else begin
  link(tail):=q;
  tail:=q;
  end;
end;

procedure sgml_write(s:str_number);
var old_selector:integer;
begin
ensure_mml_open;
old_selector:=selector;
selector:=max_selector+mml_file_no;
print(s);
selector:=old_selector;
end;

procedure sgml_endmathtag(s:str_number);
var q:pointer;
begin
if not str_eq_str(s,current_sgml_tag) then begin
  print_err("Tags do not match: ");
  print(current_sgml_tag); print(","); print(s);
  print_ln;
  print_nl("The MathML translator cannot continue");
  succumb;
  end;
q:=new_sgml_node;
sgml_tag(q):=s;
sgml_kind(q):=sgml_math_node;
if s<>"mtext" then sgml_attrs(q):=current_sgml_attrs;
sgml_info(q):=link(head);
if current_sgml_attrs=null then
  if (link(head)<>null) then
    if (str_eq_str(s,"mtr") or str_eq_str(s,"mtd")) and
       (link(link(head))=null) then begin
      q:=sgml_info(q);
      end;
pop_nest;
@<DIR: |unsave| math@>;
link(tail):=q;
tail:=q;
end;

procedure sgml_start_graf;
begin
sgml_starttexttag("par")
end;

procedure sgml_end_graf;
begin
sgml_endtexttag("par")
end;

@ Entering or leaving MML mode 
by using the routine called |primitive|, defined below. Let us enter them
now, so that we don't have to list all those parameter names anywhere else.

@<Put each of \TeX's primitives into the hash table@>=
primitive("showSGMLentities",set_show_sgml_entities,1);
primitive("noshowSGMLentities",set_show_sgml_entities,0);
primitive("MMLmode",set_mml_mode,1);
@!@:MML_mode_}{\.{\\MMLmode} primitive@>
primitive("noMMLmode",set_mml_mode,0);
@!@:noMML_mode_}{\.{\\noMMLmode} primitive@>
primitive("SGMLstartmathtag",mml_tag,0);
primitive("SGMLendmathtag",mml_tag,1);
primitive("SGMLstarttexttag",mml_tag,2);
primitive("SGMLendtexttag",mml_tag,3);
primitive("SGMLattribute",mml_tag,4);
primitive("MMLstarttext",mml_tag,5);
primitive("MMLendtext",mml_tag,6);

primitive("SGMLampersand",other_char,"&");
primitive("SGMLbackslash",other_char,"\");
primitive("SGMLcarret",other_char,"^");
primitive("SGMLdollar",other_char,"$");
primitive("SGMLhash",other_char,"#");
primitive("SGMLleftbrace",other_char,"{");
primitive("SGMLpercent",other_char,"%");
primitive("SGMLrightbrace",other_char,"}");
primitive("SGMLunderscore",other_char,"_");

primitive("SGMLentity",mml_tag,7);
primitive("SGMLemptytag",mml_tag,8);
primitive("SGMLlonetag",mml_tag,9);
primitive("SGMLFontEntity",mml_tag,10);
primitive("SGMLwrite",mml_tag,11);
primitive("SGMLwriteln",mml_tag,12);

@ @<Cases of |main_control| that build boxes and lists@>=
mmode+set_mml_mode: report_illegal_case;
non_math(set_mml_mode):
  if cur_chr=0 then MML_mode:=false else MML_mode:=true;
any_mode(set_show_sgml_entities):
  if cur_chr=0 then SGML_show_entities:=false
               else SGML_show_entities:=true;
any_mode(mml_tag): begin
  case cur_chr of
    0: begin scan_string_argument; sgml_startmathtag(cur_val); end;
    1: begin scan_string_argument; sgml_endmathtag(cur_val); end;
    2: begin scan_string_argument; sgml_starttexttag(cur_val); end;
    3: begin scan_string_argument; sgml_endtexttag(cur_val); end;
    4: begin scan_string_argument; cur_val1:=cur_val;
             scan_string_argument; sgml_attribute(cur_val1,cur_val); end;
    5: begin
       push_nest; new_save_level(text_mml_group);
       current_sgml_tag:="mtext"; mode:=-hmode;
       end;
    6: begin sgml_endtexttag("mtext"); end;
    7: begin scan_string_argument; make_sgml_entity(cur_val); end;
    8: begin scan_empty_tag; end;
    9: begin scan_lone_tag; end;
   10: begin scan_font_entity; end;
   11: begin scan_string_argument; sgml_write(cur_val); end;
   12: begin sgml_write(new_line_char); end;
  end;
  end;

@ The following code opens \.{MML} output file if neccesary.
@p procedure ensure_mml_open;
begin
  if mml_file_no=0 then begin
    incr(output_file_no);
    mml_file_no:=output_file_no;
    ensure_output_open(".mml")
                      (output_file_names[mml_file_no])
                      (output_files[mml_file_no]);
    output_file_names[mml_file_no]:=output_m_name;
    end;
end;

@ @<Declare act...@>=

@z
%---------------------------------------
@x [48] m.1137 l.21620 - Omega
@ We get into math mode from horizontal mode when a `\.\$' (i.e., a
|math_shift| character) is scanned. We must check to see whether this
`\.\$' is immediately followed by another, in case display math mode is
called for.

@<Cases of |main_control| that build...@>=
hmode+math_shift:init_math;
@y
@ We get into math mode from horizontal mode when a `\.\$' (i.e., a
|math_shift| character) is scanned. We must check to see whether this
`\.\$' is immediately followed by another, in case display math mode is
called for.

@<Cases of |main_control| that build...@>=
hmode+math_shift: begin
  if MML_mode then begin
    ensure_mml_open;
    incr(MML_level);
    end;
  init_math;
  end;

@z
%---------------------------------------
@x [48] m.1139 l.21648 - Omega
if every_math<>null then begin_token_list(every_math,every_math_text);
@y
current_sgml_tag:="mrow";
sgml_attribute("displaystyle","false");
if every_math<>null then begin_token_list(every_math,every_math_text);
@z
%---------------------------------------
@x [48] m.1145 l.21703 - Omega
if every_display<>null then begin_token_list(every_display,every_display_text);
@y
current_sgml_tag:="mrow";
sgml_attribute("displaystyle","true");
if every_display<>null then begin_token_list(every_display,every_display_text);
@z
%---------------------------------------
@x [48] m.1155 l.21878 - Omega
  else  type(p):=ord_noad+(c div @"1000000);
  link(tail):=p; tail:=p;
@y
  else  type(p):=ord_noad+(c div @"1000000);
  if not MML_mode then
    tail_append(p)
  else if (c div @"1000000)=4 then begin
    saved(0):=p;
    incr(save_ptr);
    push_math(math_mml_group);
    end
  else if (cur_group=math_mml_group) and
          ((c div @"1000000)=5) then begin
    {unsave;}
    @<DIR: |unsave| math@>;
    decr(save_ptr);
    q:=saved(0);
    r:=fin_mlist(null);
    push_math(math_group);
    tail_append(q); tail_append(r); tail_append(p);
    {unsave;}
    @<DIR: |unsave| math@>;
    p:=fin_mlist(null);
    tail_append(p);
    end
  else tail_append(p);
@z
%---------------------------------------
@x [48] m.1176--1177 l.22109 - Omega
procedure sub_sup;
var t:small_number; {type of previous sub/superscript}
@!p:pointer; {field to be filled by |scan_math|}
begin t:=empty; p:=null;
if tail<>head then if scripts_allowed(tail) then
  begin p:=supscr(tail)+cur_cmd-sup_mark; {|supscr| or |subscr|}
  t:=math_type(p);
  end;
if (p=null)or(t<>empty) then @<Insert a dummy noad to be sub/superscripted@>;
scan_math(p);
end;
@y
procedure sub_sup;
var t:small_number; {type of previous sub/superscript}
@!p,q,r:pointer; {field to be filled by |scan_math|}
begin t:=empty; p:=null;
if tail<>head then begin
  if MML_mode then
    if type(tail)=sgml_node then begin
      q:=head;
      while link(q)<>tail do q:=link(q);
      r:=new_noad;
      info(nucleus(r)):=tail;
      math_type(nucleus(r)):=sub_mlist;
      link(q):=r;
      tail:=r;
      end;
  if scripts_allowed(tail) then
    begin p:=supscr(tail)+cur_cmd-sup_mark; {|supscr| or |subscr|}
    t:=math_type(p);
    end;
  end;
if (p=null)or(t<>empty) then @<Insert a dummy noad to be sub/superscripted@>;
scan_math(p);
end;
@z
%---------------------------------------
@x [48] m.1181 l.22199 - Omega
procedure math_fraction;
var c:small_number; {the type of generalized fraction we are scanning}
begin c:=cur_chr;
if incompleat_noad<>null then
  @<Ignore the fraction operation and complain about this ambiguous case@>
else  begin incompleat_noad:=get_node(fraction_noad_size);
  type(incompleat_noad):=fraction_noad;
  subtype(incompleat_noad):=normal;
  math_type(numerator(incompleat_noad)):=sub_mlist;
  info(numerator(incompleat_noad)):=link(head);
  mem[denominator(incompleat_noad)].hh:=empty_field;
  mem[left_delimiter(incompleat_noad)].qqqq:=null_delimiter;
  mem[right_delimiter(incompleat_noad)].qqqq:=null_delimiter;@/
  link(head):=null; tail:=head;
  @<Use code |c| to distinguish between generalized fractions@>;
  end;
end;
@y
procedure math_fraction;
var c:small_number; {the type of generalized fraction we are scanning}
begin c:=cur_chr;
if incompleat_noad<>null then
  @<Ignore the fraction operation and complain about this ambiguous case@>
else  begin incompleat_noad:=get_node(fraction_noad_size);
  type(incompleat_noad):=fraction_noad;
  subtype(incompleat_noad):=normal;
  math_type(numerator(incompleat_noad)):=sub_mlist;
  info(numerator(incompleat_noad)):=link(head);
  mem[denominator(incompleat_noad)].hh:=empty_field;
  if MML_mode then begin
    mem[left_delimiter(incompleat_noad)].int:=0;
    mem[right_delimiter(incompleat_noad)].int:=0;@/
    end
  else begin
    mem[left_delimiter(incompleat_noad)].qqqq:=null_delimiter;
    mem[right_delimiter(incompleat_noad)].qqqq:=null_delimiter;@/
    end;
  link(head):=null; tail:=head;
  @<Use code |c| to distinguish between generalized fractions@>;
  end;
end;
@z
%---------------------------------------
@x [48] m.1182 l.22199 - Omega
  begin scan_delimiter(left_delimiter(incompleat_noad),false);
  scan_delimiter(right_delimiter(incompleat_noad),false);
@y
  begin if MML_mode then begin
    mem[left_delimiter(incompleat_noad)].int:=new_noad;
    scan_math(nucleus(mem[left_delimiter(incompleat_noad)].int));
    mem[right_delimiter(incompleat_noad)].int:=new_noad;
    scan_math(nucleus(mem[right_delimiter(incompleat_noad)].int));
    end
  else begin
    scan_delimiter(left_delimiter(incompleat_noad),0);
    scan_delimiter(right_delimiter(incompleat_noad),0);
    end;
@z
%---------------------------------------
@x [48] m.1183 l.22212 - Omega
  begin scan_delimiter(garbage,false); scan_delimiter(garbage,false);
@y
  begin if MML_mode then begin
    scan_math(garbage); scan_math(garbage);
    end
  else begin
    scan_delimiter(garbage,0); scan_delimiter(garbage,0);
    end;
@z
%---------------------------------------
@x [48] m.1184 l.22230 - Omega
function fin_mlist(@!p:pointer):pointer;
var q:pointer; {the mlist to return}
begin if incompleat_noad<>null then @<Compleat the incompleat noad@>
else  begin link(tail):=p; q:=link(head);
  end;
pop_nest; fin_mlist:=q;
end;
@y
function fin_mlist(@!p:pointer):pointer;
var q,q1,q2:pointer; {the mlist to return}
    tag:integer;
begin if incompleat_noad<>null then @<Compleat the incompleat noad@>
else  begin link(tail):=p; q:=link(head);
  end;
if current_sgml_tag=0 then
  tag:="mrow"
else tag:=current_sgml_tag;
pop_nest;
if MML_mode then
  if q<>null then
   if link(q)<>null then begin
     q1:=new_sgml_node;
     sgml_tag(q1):=tag;
     sgml_info(q1):=q;
     sgml_kind(q1):=sgml_math_node;
     fin_mlist:=q1
     end
   else fin_mlist:=q
  else fin_mlist:=q
else
  fin_mlist:=q;
end;
@z
%---------------------------------------
@x [48] m.1185 l.22256 - Omega
@ @<Compleat...@>=
begin math_type(denominator(incompleat_noad)):=sub_mlist;
info(denominator(incompleat_noad)):=link(head);
if p=null then q:=incompleat_noad
else  begin q:=info(numerator(incompleat_noad));
  if type(q)<>left_noad then confusion("right");
@:this can't happen right}{\quad right@>
  info(numerator(incompleat_noad)):=link(q);
  link(q):=incompleat_noad; link(incompleat_noad):=p;
  end;
end
@y
@ @<Compleat...@>=
begin math_type(denominator(incompleat_noad)):=sub_mlist;
info(denominator(incompleat_noad)):=link(head);
if p=null then
  if not MML_mode then
    q:=incompleat_noad
  else begin
    if link(info(numerator(incompleat_noad)))<>null then begin
      q1:=new_sgml_node;
      sgml_tag(q1):="mrow";
      sgml_kind(q1):=sgml_math_node;
      sgml_info(q1):=info(numerator(incompleat_noad));
      end
    else q1:=info(numerator(incompleat_noad));
    if link(info(denominator(incompleat_noad)))<>null then begin
      q2:=new_sgml_node;
      sgml_tag(q2):="mrow";
      sgml_kind(q2):=sgml_math_node;
      sgml_info(q2):=info(denominator(incompleat_noad));
      end
    else q2:=info(denominator(incompleat_noad));
    q:=new_sgml_node;
    sgml_tag(q):="mfrac";
    sgml_kind(q):=sgml_math_node;
    sgml_info(q):=q1;
    link(q1):=q2;
    current_sgml_attrs:=null;
    if thickness(incompleat_noad)<>default_code then
      if thickness(incompleat_noad)=0 then
        sgml_attribute("linethickness","0ex")
      else if thickness(incompleat_noad)<default_rule_thickness then
        sgml_attribute("linethickness","thin")
      else if thickness(incompleat_noad)>default_rule_thickness then
        sgml_attribute("linethickness","thick");
    sgml_attrs(q):=current_sgml_attrs;
    if (mem[left_delimiter(incompleat_noad)].int<>0) or
       (mem[right_delimiter(incompleat_noad)].int<>0) then begin
      q1:=q;
      q:=new_sgml_node;
      sgml_tag(q):="mrow";
      sgml_kind(q):=sgml_math_node;
      sgml_info(q):=mem[left_delimiter(incompleat_noad)].int;
      link(mem[left_delimiter(incompleat_noad)].int):=q1;
      link(q1):=mem[right_delimiter(incompleat_noad)].int;
      end
    end
else  begin q:=info(numerator(incompleat_noad));
  if type(q)<>left_noad then confusion("right");
@:this can't happen right}{\quad right@>
  info(numerator(incompleat_noad)):=link(q);
  link(q):=incompleat_noad; link(incompleat_noad):=p;
  end;
end
@z
%---------------------------------------
@x [48] m.1186 l.22256 - Omega
math_group: begin @<DIR: |unsave| math@>; decr(save_ptr);@/
  math_type(saved(0)):=sub_mlist; p:=fin_mlist(null); info(saved(0)):=p;
  if p<>null then if link(p)=null then
   if type(p)=ord_noad then
    begin if math_type(subscr(p))=empty then
     if math_type(supscr(p))=empty then
      begin mem[saved(0)].hh:=mem[nucleus(p)].hh;
      free_node(p,noad_size);
      end;
    end
  else if type(p)=accent_noad then if saved(0)=nucleus(tail) then
   if type(tail)=ord_noad then @<Replace the tail of the list by |p|@>;
  end;
@y
text_mml_group: begin
  unsave; decr(save_ptr);
  p:=saved(0);
  sgml_info(p):=link(head);
  pop_nest;
  tail_append(p);
  end;
text_sgml_group: begin
  print_err("Expecting closing tag </");
  print(current_sgml_tag); print(">.");
  print_ln;
  print_nl("The MathML translator cannot continue");
  succumb;
  end;
math_mml_group: begin
  @<DIR: |unsave| math@>;
  decr(save_ptr);
  link(saved(0)):=link(head);
  p:=saved(0);
  pop_nest;
    if type(p)=sgml_node then
      if str_eq_str(sgml_tag(p),"mrow") then
        if sgml_attrs(p)=null then begin
          if link(sgml_info(p))=null then
          p:=sgml_info(p);
          end;
  tail_append(p);
  back_input;
  end;
math_group: begin @<DIR: |unsave| math@>; decr(save_ptr);@/
  if MML_mode then 
    math_type(saved(0)):=sub_mlist
  else
    math_type(saved(0)):=sub_mlist;
  p:=fin_mlist(null); info(saved(0)):=p;
  if p<>null then if link(p)=null then
   if type(p)=ord_noad then
    begin if math_type(subscr(p))=empty then
     if math_type(supscr(p))=empty then
      begin mem[saved(0)].hh:=mem[nucleus(p)].hh;
      free_node(p,noad_size);
      end;
    end
  else if type(p)=accent_noad then if saved(0)=nucleus(tail) then
   if type(tail)=ord_noad then @<Replace the tail of the list by |p|@>;
  end;
font_entity_group: begin
  unsave;
  font_sort_number:=font_sort_ptr+1;
  for font_sorts:=font_sort_base+1 to font_sort_ptr do
    if str_eq_str(font_sort_name(font_sorts),saved(-1)) then begin
      font_sort_number:=font_sorts;
      break;
      end;
  font_sort_c:=saved(-2);
  font_sort_char_entity(font_sort_number)(font_sort_c):=saved(-3);
  font_sort_char_tag(font_sort_number)(font_sort_c):=saved(-4);
  font_sort_char_attr(font_sort_number)(font_sort_c):=current_sgml_attrs;
  current_sgml_attrs:=null;
  save_ptr:=save_ptr-4; pop_nest;
  end;
empty_tag_group: begin
  unsave; sgml_attrs(saved(-1)):=current_sgml_attrs;
  decr(save_ptr); pop_nest;
  end;
lone_tag_group: begin
  unsave; sgml_attrs(saved(-1)):=current_sgml_attrs;
  decr(save_ptr); pop_nest;
  end;
@z
%---------------------------------------
@x [48] m.1191 l.22301 - Omega
procedure math_left_right;
var t:small_number; {|left_noad| or |right_noad|}
@!p:pointer; {new noad}
begin t:=cur_chr;
if (t=right_noad)and(cur_group<>math_left_group) then
  @<Try to recover from mismatched \.{\\right}@>
else  begin p:=new_noad; type(p):=t;
  scan_delimiter(delimiter(p),false);
  if t=left_noad then
    begin push_math(math_left_group); link(head):=p; tail:=p;
    end
  else  begin p:=fin_mlist(p);
    @<DIR: |unsave| math@>; {end of |math_left_group|}
    tail_append(new_noad); type(tail):=inner_noad;
    math_type(nucleus(tail)):=sub_mlist;
    info(nucleus(tail)):=p;
    end;
  end;
end;
@y
procedure math_left_right;
var t:small_number; {|left_noad| or |right_noad|}
@!p,q,r:pointer; {new noad}
begin t:=cur_chr;
if (t=right_noad)and(cur_group<>math_left_group) then
  @<Try to recover from mismatched \.{\\right}@>
else  begin p:=new_noad;
  if MML_mode then begin
    scan_math(nucleus(p));
    if t=left_noad then begin
      saved(0):=p; incr(save_ptr);
      push_math(math_left_group);
      end
    else begin
      {unsave;}
      @<DIR: |unsave| math@>;
      decr(save_ptr);
      q:=saved(0); r:=fin_mlist(null);
      push_math(math_group);
      tail_append(q); tail_append(r);
      tail_append(p);
      {unsave;}
      @<DIR: |unsave| math@>;
      p:=fin_mlist(null);
      tail_append(p);
      end
    end
  else begin
    type(p):=t;
    scan_delimiter(delimiter(p),false);
    if t=left_noad then
      begin push_math(math_left_group); link(head):=p; tail:=p;
      end
    else  begin p:=fin_mlist(p);
      @<DIR: |unsave| math@>; {end of |math_left_group|}
      tail_append(new_noad); type(tail):=inner_noad;
      math_type(nucleus(tail)):=sub_mlist;
      info(nucleus(tail)):=p;
      end;
    end;
  end;
end;
@z
%---------------------------------------
@x [48] m.1192 l.22284 - Omega
begin if cur_group=math_shift_group then
  begin scan_delimiter(garbage,false);
  print_err("Extra "); print_esc("right");
@.Extra \\right.@>
  help1("I'm ignoring a \right that had no matching \left.");
  error;
  end
else off_save;
end
@y
begin if cur_group=math_shift_group then
  begin if MML_mode then scan_math(garbage)
  else scan_delimiter(garbage,0);
  print_err("Extra "); print_esc("right");
@.Extra \\right.@>
  help1("I'm ignoring a \right that had no matching \left.");
  error;
  end
else off_save;
end
@z
%---------------------------------------
@x [48] m.1193 l.22327 - Omega
mmode+math_shift: if cur_group=math_shift_group then after_math
  else off_save;
@y
mmode+math_shift: if cur_group=math_mml_group then after_mml
  else if cur_group=math_shift_group then after_math
  else off_save;
@z
%---------------------------------------
@x [48] m.1194 l.22284 - Omega
procedure after_math;
var l:boolean; {`\.{\\leqno}' instead of `\.{\\eqno}'}
@!danger:boolean; {not enough symbol fonts are present}
@!m:integer; {|mmode| or |-mmode|}
@!p:pointer; {the formula}
@!a:pointer; {box containing equation number}
@<Local variables for finishing a displayed formula@>@;
begin danger:=false;
@<Check that the necessary fonts for math symbols are present;
  if not, flush the current math lists and set |danger:=true|@>;
m:=mode; l:=false; p:=fin_mlist(null); {this pops the nest}
if mode=-m then {end of equation number}
  begin @<Check that another \.\$ follows@>;
  cur_mlist:=p; cur_style:=text_style; mlist_penalties:=false;
  mlist_to_hlist; a:=hpack(link(temp_head),natural);
  @<DIR: |unsave| math@>;
  decr(save_ptr); {now |cur_group=math_shift_group|}
  if saved(0)=1 then l:=true;
  danger:=false;
  @<Check that the necessary fonts for math symbols are present;
    if not, flush the current math lists and set |danger:=true|@>;
  m:=mode; p:=fin_mlist(null);
  end
else a:=null;
if m<0 then @<Finish math in text@>
else  begin if a=null then @<Check that another \.\$ follows@>;
  @<Finish displayed math@>;
  end;
end;
@y
procedure after_mml;
var p:pointer;
begin
@<DIR: |unsave| math@>;
decr(save_ptr);
link(saved(0)):=link(head);
p:=saved(0);
pop_nest;
tail_append(p);
back_input;
end;

procedure after_math;
var l:boolean; {`\.{\\leqno}' instead of `\.{\\eqno}'}
@!danger:boolean; {not enough symbol fonts are present}
@!m:integer; {|mmode| or |-mmode|}
@!p:pointer; {the formula}
@!a:pointer; {box containing equation number}
@<Local variables for finishing a displayed formula@>@;
begin danger:=false;
@<Check that the necessary fonts for math symbols are present;
  if not, flush the current math lists and set |danger:=true|@>;
m:=mode; l:=false; p:=fin_mlist(null); {this pops the nest}
if MML_mode then decr(MML_level);
if mode=-m then {end of equation number}
  begin @<Check that another \.\$ follows@>;
  cur_mlist:=p; cur_style:=text_style; mlist_penalties:=false;
  if MML_mode then begin
    sgml_out_mlist(true,false,false)
    end
  else begin
    mlist_to_hlist; a:=hpack(link(temp_head),natural);
    end;
  @<DIR: |unsave| math@>;
  decr(save_ptr); {now |cur_group=math_shift_group|}
  if saved(0)=1 then l:=true;
  danger:=false;
  @<Check that the necessary fonts for math symbols are present;
    if not, flush the current math lists and set |danger:=true|@>;
  m:=mode; p:=fin_mlist(null);
  end
else a:=null;
if m<0 then @<Finish math in text@>
else  begin if a=null then @<Check that another \.\$ follows@>;
  @<Finish displayed math@>;
  end;
end;
@z
%---------------------------------------
@x [49] m.1196 l.22388 - Omega
@<Finish math in text@> =
begin tail_append(new_math(math_surround,before));
if dir_math_save then
  @<Append a begin direction to the tail of the current list@>;
cur_mlist:=p; cur_style:=text_style; mlist_penalties:=(mode>0); mlist_to_hlist;
link(tail):=link(temp_head);
while link(tail)<>null do tail:=link(tail);
if dir_math_save then
  @<Append an end direction to the tail of the current list@>;
dir_math_save:=false;
tail_append(new_math(math_surround,after));
space_factor:=1000;
@<DIR: |unsave| math@>;
end
@y
@<Finish math in text@>=
begin
if not MML_mode then begin
  tail_append(new_math(math_surround,before));
  if dir_math_save then
    @<Append a begin direction to the tail of the current list@>;
  end;
cur_mlist:=p; cur_style:=text_style; mlist_penalties:=(mode>0);
if MML_mode then begin
{
  if MML_level=0 then sgml_out_mlist(true,false,false)
  else tail_append(cur_mlist);
}
  sgml_starttexttag("inlinemath");
  sgml_starttexttag("math");
  tail_append(cur_mlist);
  sgml_endtexttag("math");
  sgml_endtexttag("inlinemath");
  end
else begin
  mlist_to_hlist;
  link(tail):=link(temp_head);
  while link(tail)<>null do tail:=link(tail);
  if dir_math_save then
    @<Append an end direction to the tail of the current list@>;
  dir_math_save:=false;
  tail_append(new_math(math_surround,after));
  end;
space_factor:=1000;
@<DIR: |unsave| math@>;
end
@z
%---------------------------------------
@x [49] m.1197 l.22430 - Omega
@<Finish displayed math@>=
cur_mlist:=p; cur_style:=display_style; mlist_penalties:=false;
mlist_to_hlist; p:=link(temp_head);@/
adjust_tail:=adjust_head; b:=hpack(p,natural); p:=list_ptr(b);
t:=adjust_tail; adjust_tail:=null;@/
w:=width(b); z:=display_width; s:=display_indent;
if (a=null)or danger then
  begin e:=0; q:=0;
  end
else  begin e:=width(a); q:=e+math_quad(text_size);
  end;
if w+q>z then
  @<Squeeze the equation as much as possible; if there is an equation
    number that should go on a separate line by itself,
    set~|e:=0|@>;
@<Determine the displacement, |d|, of the left edge of the equation, with
  respect to the line size |z|, assuming that |l=false|@>;
@<Append the glue or equation number preceding the display@>;
@<Append the display and perhaps also the equation number@>;
@<Append the glue or equation number following the display@>;
resume_after_display
@y
@<Finish displayed math@>=
cur_mlist:=p; cur_style:=display_style; mlist_penalties:=false;
if MML_mode then begin
{
  if MML_level=0 then sgml_out_mlist(true,false,false)
  else tail_append(cur_mlist);
}
  sgml_starttexttag("displaymath");
  sgml_starttexttag("math");
  tail_append(cur_mlist);
  sgml_endtexttag("math");
  sgml_endtexttag("displaymath");
  end
else begin
  mlist_to_hlist; p:=link(temp_head);@/
  adjust_tail:=adjust_head; b:=hpack(p,natural); p:=list_ptr(b);
  t:=adjust_tail; adjust_tail:=null;@/
  w:=width(b); z:=display_width; s:=display_indent;
  if (a=null)or danger then
    begin e:=0; q:=0;
    end
  else  begin e:=width(a); q:=e+math_quad(text_size);
    end;
  if w+q>z then
    @<Squeeze the equation as much as possible; if there is an equation
      number that should go on a separate line by itself,
      set~|e:=0|@>;
  @<Determine the displacement, |d|, of the left edge of the equation, with
    respect to the line size |z|, assuming that |l=false|@>;
  @<Append the glue or equation number preceding the display@>;
  @<Append the display and perhaps also the equation number@>;
  @<Append the glue or equation number following the display@>;
  end;
resume_after_display
@z
%---------------------------------------
@x [49] m.1257 l.23269 - Omega
@!flushable_string:str_number; {string not yet referenced}
begin if job_name=0 then open_log_file;
  {avoid confusing \.{texput} with the font name}
@.texput@>
get_r_token; u:=cur_cs;
@y
@!flushable_string:str_number; {string not yet referenced}
@!cur_font_sort_name:str_number; {the name without the digits at the end}
@!new_length:integer; {length of font name, to become font sort name}
@!last_character:integer; {last character in font name}
@!i:integer; {to run through characters of name}
@!this_is_a_new_font:boolean;
begin if job_name=0 then open_log_file;
  {avoid confusing \.{texput} with the font name}
@.texput@>
get_r_token; u:=cur_cs;
this_is_a_new_font:=false;
@z
%---------------------------------------
@x [49] m.1257 l.23290 - Omega
common_ending: set_equiv(u,f);
set_new_eqtb(font_id_base+f,new_eqtb(u)); settext(font_id_base+f,t);
@y
this_is_a_new_font:=true;
common_ending: set_equiv(u,f);
set_new_eqtb(font_id_base+f,new_eqtb(u)); settext(font_id_base+f,t);
if this_is_a_new_font then
  begin
  if cur_name>=@"10000 then begin
    new_length:=length(cur_name);
    last_character:=str_pool[str_start(cur_name)+new_length-1];
    while (last_character>="0") and (last_character<="9") do begin
      decr(new_length);
      last_character:=str_pool[str_start(cur_name)+new_length-1];
      end;
    for i:=1 to new_length do begin
      append_char(str_pool[str_start(cur_name)+i-1]);
      end;
    cur_font_sort_name:=make_string;
    font_sort_number:=font_sort_ptr+1;
    for font_sorts:=font_sort_base+1 to font_sort_ptr do
      if str_eq_str(font_sort_name(font_sorts),cur_font_sort_name) then begin
        font_sort_number:=font_sorts;
        break;
        end;
    font_name_sort(f):=font_sort_number;
    if font_sort_number=(font_sort_ptr+1) then begin
      incr(font_sort_ptr);
      allocate_font_sort_table
        (font_sort_ptr,
         font_sort_offset_char_base+3*(font_ec(f)-font_bc(f)+1));
      font_sort_file_size(font_sort_ptr):=
         font_sort_offset_char_base+3*(font_ec(f)-font_bc(f)+1);
      font_sort_name(font_sort_ptr):=cur_font_sort_name;
      font_sort_ec(font_sort_ptr):=font_ec(f);
      font_sort_bc(font_sort_ptr):=font_bc(f);
      cur_name:=cur_font_sort_name;
      cur_ext:=".onm";
      pack_cur_name;
      begin_file_reading;
      if a_open_in(cur_file,kpse_tex_format) then begin
        name:=a_make_name_string(cur_file);
        print_ln; print("("); print(name);
        @<Read the first line of the new file@>;
        end
      else end_file_reading;
      end;
    end;
  end;
@z
%---------------------------------------
@x [50] m.1320 l.23983 - Omega
@ @<Dump the font information@>=
@y
@ @<Dump the font information@>=
dump_int(font_sort_ptr);
for k:=null_font_sort to font_sort_ptr do
  dump_font_sort_table(k,font_sort_file_size(k));
@z
%---------------------------------------
@x [50] m.1321 l.23993 - Omega
@ @<Undump the font information@>=
@y
@ @<Undump the font information@>=
undump_size(font_base)(font_max)('font sort max')(font_sort_ptr);
for k:=null_font_sort to font_sort_ptr do
  undump_font_sort_table(k);
@z
%---------------------------------------
@x [51] m.1333 l.24244 - Omega
procedure close_files_and_terminate;
var k:integer; {all-purpose index}
begin @<Finish the extensions@>;
@!stat if tracing_stats>0 then @<Output statistics about this job@>;@;@+tats@/
wake_up_terminal; @<Finish the \.{DVI} file@>;
@y
procedure close_files_and_terminate;
var k:integer; {all-purpose index}
begin @<Finish the extensions@>;
@!stat if tracing_stats>0 then @<Output statistics about this job@>;@;@+tats@/
wake_up_terminal;
if not MML_mode then begin @<Finish the \.{DVI} file@>; end;
@z
%---------------------------------------
