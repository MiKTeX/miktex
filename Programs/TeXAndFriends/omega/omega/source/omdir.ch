% omdir.ch: Primitives to deal with multiple-direction text
%
% This file is part of the Omega projet, which
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
%-------------------------
@x [10] m.135
@d hlist_node=0 {|type| of hlist nodes}
@d box_node_size=7 {number of words to allocate for a box node}
@d width_offset=1 {position of |width| field in a box node}
@d depth_offset=2 {position of |depth| field in a box node}
@d height_offset=3 {position of |height| field in a box node}
@d width(#) == mem[#+width_offset].sc {width of the box, in sp}
@d depth(#) == mem[#+depth_offset].sc {depth of the box, in sp}
@d height(#) == mem[#+height_offset].sc {height of the box, in sp}
@d shift_amount(#) == mem[#+4].sc {repositioning distance, in sp}
@d list_offset=5 {position of |list_ptr| field in a box node}
@d list_ptr(#) == link(#+list_offset) {beginning of the list inside the box}
@d glue_order(#) == subtype(#+list_offset) {applicable order of infinity}
@d glue_sign(#) == type(#+list_offset) {stretching or shrinking}
@d normal=0 {the most common case when several cases are named}
@d stretching = 1 {glue setting applies to the stretch components}
@d shrinking = 2 {glue setting applies to the shrink components}
@d glue_offset = 6 {position of |glue_set| in a box node}
@d glue_set(#) == mem[#+glue_offset].gr
  {a word of type |glue_ratio| for glue setting}
@y
@d hlist_node=0 {|type| of hlist nodes}
@d box_node_size=8 {number of words to allocate for a box node}
@d width_offset=1 {position of |width| field in a box node}
@d depth_offset=2 {position of |depth| field in a box node}
@d height_offset=3 {position of |height| field in a box node}
@d width(#) == mem[#+width_offset].sc {width of the box, in sp}
@d depth(#) == mem[#+depth_offset].sc {depth of the box, in sp}
@d height(#) == mem[#+height_offset].sc {height of the box, in sp}
@d shift_amount(#) == mem[#+4].sc {repositioning distance, in sp}
@d list_offset=5 {position of |list_ptr| field in a box node}
@d list_ptr(#) == link(#+list_offset) {beginning of the list inside the box}
@d glue_order(#) == subtype(#+list_offset) {applicable order of infinity}
@d glue_sign(#) == type(#+list_offset) {stretching or shrinking}
@d normal=0 {the most common case when several cases are named}
@d stretching = 1 {glue setting applies to the stretch components}
@d shrinking = 2 {glue setting applies to the shrink components}
@d glue_offset = 6 {position of |glue_set| in a box node}
@d glue_set(#) == mem[#+glue_offset].gr
  {a word of type |glue_ratio| for glue setting}
@d dir_offset = 7 {position of |box_dir| in a box node}
@d box_dir(#) == mem[#+dir_offset].int
@z
%-------------------------
@x [10] m.136
@p function new_null_box:pointer; {creates a new box node}
var p:pointer; {the new node}
begin p:=get_node(box_node_size); type(p):=hlist_node;
subtype(p):=min_quarterword;
width(p):=0; depth(p):=0; height(p):=0; shift_amount(p):=0; list_ptr(p):=null;
glue_sign(p):=normal; glue_order(p):=normal; set_glue_ratio_zero(glue_set(p));
new_null_box:=p;
@y
@p function new_null_box:pointer; {creates a new box node}
var p:pointer; {the new node}
begin p:=get_node(box_node_size); type(p):=hlist_node;
subtype(p):=min_quarterword;
width(p):=0; depth(p):=0; height(p):=0; shift_amount(p):=0; list_ptr(p):=null;
glue_sign(p):=normal; glue_order(p):=normal; set_glue_ratio_zero(glue_set(p));
box_dir(p):=text_direction; 
new_null_box:=p;
@z
%-------------------------
@x [10] m.138
@d rule_node=2 {|type| of rule nodes}
@d rule_node_size=4 {number of words to allocate for a rule node}
@d null_flag==-@'10000000000 {$-2^{30}$, signifies a missing item}
@d is_running(#) == (#=null_flag) {tests for a running dimension}
@y
@d rule_node=2 {|type| of rule nodes}
@d rule_node_size=5 {number of words to allocate for a rule node}
@d null_flag==-@'10000000000 {$-2^{30}$, signifies a missing item}
@d is_running(#) == (#=null_flag) {tests for a running dimension}
@d rule_dir(#)==info(#+4)
@z
%-------------------------
@x [10] m.139
@p function new_rule:pointer;
var p:pointer; {the new node}
begin p:=get_node(rule_node_size); type(p):=rule_node;
subtype(p):=0; {the |subtype| is not used}
width(p):=null_flag; depth(p):=null_flag; height(p):=null_flag;
new_rule:=p;
end;
@y
@p function new_rule:pointer;
var p:pointer; {the new node}
begin p:=get_node(rule_node_size); type(p):=rule_node;
subtype(p):=0; {the |subtype| is not used}
width(p):=null_flag; depth(p):=null_flag; height(p):=null_flag;
rule_dir(p):=-1;
new_rule:=p;
end;
@z
%-------------------------
@x [10] m.176
procedure print_rule_dimen(@!d:scaled); {prints dimension in rule node}
begin if is_running(d) then print_char("*") else print_scaled(d);
@.*\relax@>
end;
@y
procedure print_rule_dimen(@!d:scaled); {prints dimension in rule node}
begin if is_running(d) then print_char("*") else print_scaled(d);
@.*\relax@>
end;
@#
procedure print_dir(d:integer);
begin
print(dir_names[dir_primary[d]]);
print(dir_names[dir_secondary[d]]);
print(dir_names[dir_tertiary[d]]);
end;
@z
%-------------------------
@x [12] m.184
@ @<Display box |p|@>=
begin if type(p)=hlist_node then print_esc("h")
else if type(p)=vlist_node then print_esc("v")
else print_esc("unset");
print("box("); print_scaled(height(p)); print_char("+");
print_scaled(depth(p)); print(")x"); print_scaled(width(p));
if type(p)=unset_node then
  @<Display special fields of the unset node |p|@>
else  begin @<Display the value of |glue_set(p)|@>;
  if shift_amount(p)<>0 then
    begin print(", shifted "); print_scaled(shift_amount(p));
    end;
  end;
node_list_display(list_ptr(p)); {recursive call}
end
@y
@ @<Display box |p|@>=
begin if type(p)=hlist_node then print_esc("h")
else if type(p)=vlist_node then print_esc("v")
else print_esc("unset");
print("box("); print_scaled(height(p)); print_char("+");
print_scaled(depth(p)); print(")x"); print_scaled(width(p));
if type(p)=unset_node then
  @<Display special fields of the unset node |p|@>
else  begin @<Display the value of |glue_set(p)|@>;
  if shift_amount(p)<>0 then
    begin print(", shifted "); print_scaled(shift_amount(p));
    end;
  print(", direction "); print_dir(box_dir(p));
  end;
node_list_display(list_ptr(p)); {recursive call}
end
@z
%-------------------------
@x [10] m.206
hlist_node,vlist_node,unset_node: begin r:=get_node(box_node_size);
  mem[r+6]:=mem[p+6]; mem[r+5]:=mem[p+5]; {copy the last two words}
@y
hlist_node,vlist_node,unset_node: begin r:=get_node(box_node_size);
  mem[r+7]:=mem[p+7]; mem[r+6]:=mem[p+6]; mem[r+5]:=mem[p+5];
  {copy the last three words}
@z
%-------------------------
@x [15] m.209
@d max_internal=register
   {the largest code that can follow \.{\\the}}
@y
@d assign_dir=register+1 {(\.{\\pagedir}, \.{\\textdir})}
@d max_internal=assign_dir
   {the largest code that can follow \.{\\the}}
@z
%-------------------------
@x [16] m.212
  @!aux_field: memory_word;
@y
  @!aux_field: memory_word;
  @!dirs_field: halfword;
  @!math_field: integer;
@z
%-------------------------
@x [16] m.212
@d mode_line==cur_list.ml_field {source file line number at beginning of list}
@y
@d mode_line==cur_list.ml_field {source file line number at beginning of list}
@d dir_save==cur_list.dirs_field {dir stack when a paragraph is interrupted}
@d dir_math_save==cur_list.math_field
   {should begin/end dir nodes be placed around mathematics?}
@z
%-------------------------
@x [16] m.215
prev_graf:=0; shown_mode:=0;
@y
prev_graf:=0; shown_mode:=0;
dir_save:=null; dir_math_save:=false;
@z
%-------------------------
@x [16] m.216 l.4351
@ @p @<LOCAL: Declare |make_local_par_node|@>;
@y
@ @p @<LOCAL: Declare |make_local_par_node|@>;

@ @p @<DIR: Declare |new_dir|@>;
@z
%-------------------------
@x [16] m.216
incr(nest_ptr); head:=get_avail; tail:=head; prev_graf:=0; mode_line:=line;
@y
incr(nest_ptr); head:=get_avail; tail:=head; prev_graf:=0; mode_line:=line;
dir_save:=null; dir_math_save:=false;
@z
%-------------------------
@x [16] m.217
free_avail(head); decr(nest_ptr); cur_list:=nest[nest_ptr];
@y
free_avail(head); decr(nest_ptr); cur_list:=nest[nest_ptr];
@z
%-------------------------
@x [17] m.236
@d int_pars=58 {total number of integer parameters}
@d count_base=int_base+int_pars {|number_regs| user \.{\\count} registers}
@y
@d no_local_dirs_code=58
@d level_local_dir_code=59
@d int_pars=61 {total number of integer parameters}
@d dir_base=int_base+int_pars
@d page_direction_code=0
@d body_direction_code=1
@d par_direction_code=2
@d text_direction_code=3
@d math_direction_code=4
@d dir_pars=5
@d count_base=dir_base+dir_pars {|number_regs| user \.{\\count} registers}
@z
%-------------------------
@x [17] m.238
@d no_local_whatsits==int_par(no_local_whatsits_code)
@y
@d no_local_whatsits==int_par(no_local_whatsits_code)
@d no_local_dirs==int_par(no_local_dirs_code)
@d level_local_dir==int_par(level_local_dir_code)
@d dir_par(#)==new_eqtb_int(dir_base+#) {a direction parameter}
@d page_direction==dir_par(page_direction_code)
@d body_direction==dir_par(body_direction_code)
@d par_direction==dir_par(par_direction_code)
@d text_direction==dir_par(text_direction_code)
@d math_direction==dir_par(math_direction_code)
@z
%-------------------------
@x [17] m.240
set_new_eqtb_int(int_base+end_line_char_code,carriage_return);
@y
set_new_eqtb_int(int_base+end_line_char_code,carriage_return);
set_new_eqtb_int(int_base+level_local_dir_code,level_one);
@z
%-------------------------
@x [17] m.247
@d dimen_pars=21 {total number of dimension parameters}
@y
@d page_width_code=21
@d page_height_code=22
@d page_right_offset_code=23
@d page_bottom_offset_code=24
@d dimen_pars=25 {total number of dimension parameters}
@z
%-------------------------
@x [17] m.247
@d emergency_stretch==dimen_par(emergency_stretch_code)
@y
@d emergency_stretch==dimen_par(emergency_stretch_code)
@d page_width==dimen_par(page_width_code)
@d page_height==dimen_par(page_height_code)
@d page_right_offset==dimen_par(page_right_offset_code)
@d page_bottom_offset==dimen_par(page_bottom_offset_code)
@z
%-------------------------
@x [26] m.410
@d int_val=0 {integer values}
@d dimen_val=1 {dimension values}
@d glue_val=2 {glue specifications}
@d mu_val=3 {math glue specifications}
@d ident_val=4 {font identifier}
@d tok_val=5 {token lists}

@<Glob...@>=
@!cur_val:integer; {value returned by numeric scanners}
@!cur_val1:integer; {delcodes are now 51 digits}
@!cur_val_level:int_val..tok_val; {the ``level'' of this value}
@y
@d int_val=0 {integer values}
@d dimen_val=1 {dimension values}
@d glue_val=2 {glue specifications}
@d mu_val=3 {math glue specifications}
@d dir_val=4 {directions}
@d ident_val=5 {font identifier}
@d tok_val=6 {token lists}

@<Glob...@>=
@!cur_val:integer; {value returned by numeric scanners}
@!cur_val1:integer; {delcodes are now 51 digits}
@!cur_val_level:int_val..dir_val; {the ``level'' of this value}
@z
%-------------------------
@x [26] m.413
assign_int: scanned_result(new_eqtb_int(m))(int_val);
@y
assign_int: scanned_result(new_eqtb_int(m))(int_val);
assign_dir: scanned_result(new_eqtb_int(m))(dir_val);
@z
%-------------------------
@x [26] m.437
@ @<Declare procedures that scan restricted classes of integers@>=
procedure scan_eight_bit_int;
@y
@
@d dir_T=0
@d dir_L=1
@d dir_B=2
@d dir_R=3
@d dir_eq_end(#)==(#)
@d dir_eq(#)==(#)=dir_eq_end
@d dir_ne_end(#)==(#)
@d dir_ne(#)==(#)<>dir_ne_end
@d dir_opposite_end(#)==((#) mod 4)
@d dir_opposite(#)==(((#)+2) mod 4)=dir_opposite_end
@d dir_parallel_end(#)==((#) mod 2)
@d dir_parallel(#)==((#) mod 2)=dir_parallel_end
@d dir_orthogonal_end(#)==(# mod 2)
@d dir_orthogonal(#)==((#) mod 2)<>dir_orthogonal_end
@d dir_next_end(#)==((#) mod 4)
@d dir_next(#)==(((#)+3) mod 4)=dir_next_end
@d dir_prev_end(#)==((#) mod 4)
@d dir_prev(#)==(((#)+1) mod 4)=dir_prev_end

{box directions}
@d dir_TL_=0
@d dir_TR_=1
@d dir_LT_=2
@d dir_LB_=3
@d dir_BL_=4
@d dir_BR_=5
@d dir_RT_=6
@d dir_RB_=7

{font directions}
@d dir__LT= 0
@d dir__LL= 1
@d dir__LB= 2
@d dir__LR= 3
@d dir__RT= 4
@d dir__RL= 5
@d dir__RB= 6
@d dir__RR= 7
@d dir__TT= 8
@d dir__TL= 9
@d dir__TB=10
@d dir__TR=11
@d dir__BT=12
@d dir__BL=13
@d dir__BB=14
@d dir__BR=15

@d is_mirrored(#)==dir_opposite(dir_primary[#])(dir_tertiary[#])
@d is_rotated(#)==dir_parallel(dir_secondary[#])(dir_tertiary[#])
@d font_direction(#)==(# mod 16)
@d box_direction(#)==(# div 4)

@d scan_single_dir(#)==begin
if scan_keyword("T") then #:=dir_T
else if scan_keyword("L") then #:=dir_L
else if scan_keyword("B") then #:=dir_B
else if scan_keyword("R") then #:=dir_R
else begin
  print_err("Bad direction"); cur_val:=0;
  goto exit;
  end
end

@<Declare procedures that scan restricted classes of integers@>=
procedure scan_dir;
var d1,d2,d3: integer;
begin
get_x_token;
if cur_cmd=assign_dir then begin
  cur_val:=new_eqtb_int(cur_chr);
  goto exit;
  end
else back_input;
scan_single_dir(d1);
scan_single_dir(d2);
if dir_parallel(d1)(d2) then begin
  print_err("Bad direction"); cur_val:=0;
  goto exit;
  end;
scan_single_dir(d3);
cur_val:=d1*8+dir_rearrange[d2]*4+d3;
exit:
end;

@ @<Declare procedures that scan restricted classes of integers@>=
procedure scan_eight_bit_int;
@z
%-------------------------
@x [27] m.463
@p function scan_rule_spec:pointer;
label reswitch;
var q:pointer; {the rule node being created}
begin q:=new_rule; {|width|, |depth|, and |height| all equal |null_flag| now}
if cur_cmd=vrule then width(q):=default_rule
else  begin height(q):=default_rule; depth(q):=0;
  end;
@y
@p function scan_rule_spec:pointer;
label reswitch;
var q:pointer; {the rule node being created}
begin q:=new_rule; {|width|, |depth|, and |height| all equal |null_flag| now}
if cur_cmd=vrule then begin
  width(q):=default_rule;
  rule_dir(q):=body_direction;
  end
else begin
  height(q):=default_rule; depth(q):=0;
  rule_dir(q):=text_direction;
end;
@z
%-------------------------
@x [27] m.465
  int_val:print_int(cur_val);
@y
  int_val:print_int(cur_val);
  dir_val:print_dir(cur_val);
@z
%-------------------------
@x [30] m.550
@d offset_false_bchar=offset_bchar+1
@d offset_ivalues_start=offset_false_bchar+1
@y
@d offset_false_bchar=offset_bchar+1
@d offset_natural_dir=offset_false_bchar+1
@d offset_ivalues_start=offset_natural_dir+1
@z
%-------------------------
@x [30] m.550
@d font_false_bchar(#)==font_info(#)(offset_false_bchar).int
@y
@d font_false_bchar(#)==font_info(#)(offset_false_bchar).int
@d font_natural_dir(#)==font_info(#)(offset_natural_dir).int
@z
%-------------------------
@x [30] m.560
@p function read_font_info(@!u:pointer;@!nom,@!aire:str_number;
   @!s:scaled;offset:quarterword):internal_font_number; {input a \.{TFM} file}
@y
@p function read_font_info(@!u:pointer;@!nom,@!aire:str_number;
   @!s:scaled;offset:quarterword;natural_dir:integer):internal_font_number;
   {input a \.{TFM} file}
@z
%-------------------------
@x [30] m.560
font_offset(f):=offset;
@y
font_offset(f):=offset;
font_natural_dir(f):=natural_dir;
@z
%-------------------------
@x [31] m.586
@d set_rule=132 {typeset a rule and move right}
@y
@d set_rule=132 {typeset a rule and move right}
@d put1==133 {typeset a character without moving}
@z
%-------------------------
@x [31] m.586
@d right1=143 {move right}
@y
@d right1==143 {move right}
@d right4==146 {move right, 4 bytes}
@z
%-------------------------
@x [31] m.586
@d down1=157 {move down}
@y
@d down1=157 {move down}
@d down4=160 {move down, 4 bytes}
@z
%-------------------------
@x [32] m.607
@p procedure movement(@!w:scaled;@!o:eight_bits);
label exit,found,not_found,2,1;
var mstate:small_number; {have we seen a |y| or |z|?}
@!p,@!q:pointer; {current and top nodes on the stack}
@!k:integer; {index into |dvi_buf|, modulo |dvi_buf_size|}
begin q:=get_node(movement_node_size); {new node for the top of the stack}
width(q):=w; location(q):=dvi_offset+dvi_ptr;
if o=down1 then
  begin link(q):=down_ptr; down_ptr:=q;
  end
else  begin link(q):=right_ptr; right_ptr:=q;
  end;
@<Look at the other stack entries until deciding what sort of \.{DVI} command
  to generate; |goto found| if node |p| is a ``hit''@>;
@<Generate a |down| or |right| command for |w| and |return|@>;
found: @<Generate a |y0| or |z0| command in order to reuse a previous
  appearance of~|w|@>;
exit:end;
@y
@d dvi_set(#)==oval:=#-font_offset(f); ocmd:=set1; out_cmd
@d dvi_put(#)==oval:=#-font_offset(f); ocmd:=put1; out_cmd
@d dvi_set_rule_end(#)==dvi_four(#)
@d dvi_set_rule(#)==dvi_out(set_rule); dvi_four(#); dvi_set_rule_end
@d dvi_put_rule_end(#)==dvi_four(#)
@d dvi_put_rule(#)==dvi_out(put_rule); dvi_four(#); dvi_put_rule_end
@d dvi_right(#)==dvi_out(right4); dvi_four(#);
@d dvi_left(#)==dvi_out(right4); dvi_four(-#);
@d dvi_down(#)==dvi_out(down4); dvi_four(#);
@d dvi_up(#)==dvi_out(down4); dvi_four(-#);

@p procedure movement(@!w:scaled;@!o:eight_bits);
label exit,found,not_found,2,1;
var mstate:small_number; {have we seen a |y| or |z|?}
@!p,@!q:pointer; {current and top nodes on the stack}
@!k:integer; {index into |dvi_buf|, modulo |dvi_buf_size|}
begin
case box_direction(dvi_direction) of
  dir_TL_: begin
    end;
  dir_TR_: begin
    if o=right1 then negate(w);
    end;
  dir_LT_: begin
    if o=right1 then o:=down1
    else o:=right1;
    end;
  dir_LB_: begin
    if o=right1 then begin
      o:=down1; negate(w);
      end
    else o:=right1;
    end;
  dir_BL_: begin
    if o=down1 then negate(w);
    end;
  dir_BR_: begin
    negate(w);
    end;
  dir_RT_: begin
    if o=right1 then o:=down1
    else begin o:=right1; negate(w);
      end;
    end;
  dir_RB_: begin
    if o=right1 then o:=down1 else o:=right1;
    negate(w);
    end;
  end;
if false then begin
q:=get_node(movement_node_size); {new node for the top of the stack}
width(q):=w; location(q):=dvi_offset+dvi_ptr;
if o=down1 then
  begin link(q):=down_ptr; down_ptr:=q;
  end
else  begin link(q):=right_ptr; right_ptr:=q;
  end;
@<Look at the other stack entries until deciding what sort of \.{DVI} command
  to generate; |goto found| if node |p| is a ``hit''@>;
end;
@<Generate a |down| or |right| command for |w| and |return|@>;
found: @<Generate a |y0| or |z0| command in order to reuse a previous
  appearance of~|w|@>;
exit:end;
@z
%-------------------------
@x [32] m.610
info(q):=yz_OK;
@y
@z
%-------------------------
@x [32] m.617
dvi_h:=0; dvi_v:=0; cur_h:=h_offset; dvi_f:=null_font;
@y
dvi_h:=0; dvi_v:=0; cur_h:=0; cur_v:=0; dvi_f:=null_font;
@z
%-------------------------
@x [32] m.619
procedure hlist_out; {output an |hlist_node| box}
label reswitch, move_past, fin_rule, next_p;
var base_line: scaled; {the baseline coordinate for this box}
@y
procedure hlist_out; {output an |hlist_node| box}
label reswitch, move_past, fin_rule, next_p;
var base_line: scaled; {the baseline coordinate for this box}
c_wd,c_ht,c_dp: scaled;
  {the real width, height and depth of the character}
c_htdp: quarterword; {height-depth entry in |char_info|}
c_info: four_quarters; {|char_info| entry}
edge_v: scaled;
effective_horizontal: scaled;
basepoint_horizontal: scaled;
basepoint_vertical: scaled;
saving_h: scaled;
saving_v: scaled;
save_direction: integer;
dir_tmp,dir_ptr:pointer;
dvi_dir_h,dvi_dir_ptr,dvi_temp_ptr:integer;
@z
%-------------------------
@x [32] m.619
incr(cur_s);
if cur_s>0 then dvi_out(push);
if cur_s>max_push then max_push:=cur_s;
save_loc:=dvi_offset+dvi_ptr; base_line:=cur_v; left_edge:=cur_h;
while p<>null do @<Output node |p| for |hlist_out| and move to the next node,
  maintaining the condition |cur_v=base_line|@>;
prune_movements(save_loc);
if cur_s>0 then dvi_pop(save_loc);
decr(cur_s);
@y
save_direction:=dvi_direction;
dvi_direction:=box_dir(this_box);
@<DIR: Initialize |dir_ptr| for |ship_out|@>;
saving_h:=dvi_h; saving_v:=dvi_v;
incr(cur_s);
if cur_s>0 then dvi_out(push);
if cur_s>max_push then max_push:=cur_s;
save_loc:=dvi_offset+dvi_ptr; base_line:=cur_v; left_edge:=cur_h;
while p<>null do @<Output node |p| for |hlist_out| and move to the next node,
  maintaining the condition |cur_v=base_line|@>;
cur_h:=saving_h; cur_v:=saving_v;
synch_h; synch_v;
prune_movements(save_loc);
if cur_s>0 then dvi_pop(save_loc);
decr(cur_s);
dvi_direction:=save_direction;
@<DIR: Reset |dir_ptr|@>;
@z
%-------------------------
@x [32] m.620
  oval:=c-font_offset(f); ocmd:=set1; out_cmd;@/
  cur_h:=cur_h+char_width(f)(char_info(f)(c));
@y
  c_info:=char_info(f)(c);
  c_htdp:=height_depth(c_info);
  if is_rotated(dvi_direction) then begin
    c_ht:=char_width(f)(c_info) div 2;
    c_wd:=char_height(f)(c_htdp)+char_depth(f)(c_htdp);
    end
  else begin
    c_ht:=char_height(f)(c_htdp);
    c_dp:=char_depth(f)(c_htdp);
    c_wd:=char_width(f)(c_info);
    end;
  cur_h:=cur_h+c_wd;
  if (font_natural_dir(f)<>-1) then
    case font_direction(dvi_direction) of
    dir__LT,dir__LB: begin
      dvi_set(c);
      end;
    dir__RT,dir__RB: begin
      dvi_put(c);
      dvi_left(c_wd);
      end;
    dir__TL,dir__TR: begin
      dvi_put(c);
      dvi_down(c_wd);
      end;
    dir__BL,dir__BR: begin
      dvi_put(c);
      dvi_up(c_wd);
      end;
    dir__LL,dir__LR: begin
      dvi_put(c);
      dvi_right(c_wd);
      end;
    dir__RL,dir__RR: begin
      dvi_put(c);
      dvi_left(c_wd);
      end;
    dir__TT,dir__TB: begin
      dvi_put(c);
      dvi_down(c_wd);
      end;
    dir__BT,dir__BB: begin
      dvi_put(c);
      dvi_up(c_wd);
      end;
    end
  else
    case font_direction(dvi_direction) of
    dir__LT: begin
      dvi_set(c);
      end;
    dir__LB: begin
      dvi_down(c_ht);
      dvi_set(c);
      dvi_up(c_ht);
      end;
    dir__RT: begin
      dvi_left(c_wd);
      dvi_put(c);
      end;
    dir__RB: begin
      dvi_left(c_wd);
      dvi_down(c_ht);
      dvi_put(c);
      dvi_up(c_ht);
      end;
    dir__TL: begin
      dvi_down(c_wd);
      dvi_left(c_ht);
      dvi_put(c);
      dvi_right(c_ht);
      end;
    dir__TR: begin
      dvi_down(c_wd);
      dvi_left(c_dp);
      dvi_put(c);
      dvi_right(c_dp);
      end;
    dir__BL: begin
      dvi_left(c_ht);
      dvi_put(c);
      dvi_right(c_ht);
      dvi_up(c_wd);
      end;
    dir__BR: begin
      dvi_left(c_dp);
      dvi_put(c);
      dvi_right(c_dp);
      dvi_up(c_wd);
      end;
    dir__LL,dir__LR: begin
      dvi_down(c_ht);
      dvi_put(c);
      dvi_up(c_ht);
      dvi_right(c_wd);
      end;
    dir__RL,dir__RR: begin
      dvi_left(c_wd);
      dvi_down(c_ht);
      dvi_put(c);
      dvi_up(c_ht);
      end;
    dir__TT,dir__TB: begin
      dvi_down(c_wd);
      dvi_left(c_ht);
      dvi_put(c);
      dvi_right(c_ht);
      end;
    dir__BT,dir__BB: begin
      dvi_left(c_ht);
      dvi_put(c);
      dvi_right(c_ht);
      dvi_up(c_wd);
      end;
    end;
@z
%-------------------------
@x [32] m.622
rule_node: begin rule_ht:=height(p); rule_dp:=depth(p); rule_wd:=width(p);
  goto fin_rule;
  end;
@y
rule_node: begin
  if not (dir_orthogonal(dir_primary[rule_dir(p)])(dir_primary[dvi_direction]))
    then begin
    rule_ht:=height(p); rule_dp:=depth(p); rule_wd:=width(p);
    end
  else begin
    rule_ht:=width(p) div 2;
    rule_dp:=width(p) div 2;
    rule_wd:=height(p)+depth(p);
    end;
  goto fin_rule;
  end;
@z
%-------------------------
@x [32] m.623
if list_ptr(p)=null then cur_h:=cur_h+width(p)
else  begin save_h:=dvi_h; save_v:=dvi_v;
  cur_v:=base_line+shift_amount(p); {shift the box down}
  temp_ptr:=p; edge:=cur_h;
  if type(p)=vlist_node then vlist_out@+else hlist_out;
  dvi_h:=save_h; dvi_v:=save_v;
  cur_h:=edge+width(p); cur_v:=base_line;
  end
@y
begin
if not (dir_orthogonal(dir_primary[box_dir(p)])(dir_primary[dvi_direction]))
  then begin
  effective_horizontal:=width(p);
  basepoint_vertical:=0;
  if dir_opposite(dir_secondary[box_dir(p)])(dir_secondary[dvi_direction]) then
    basepoint_horizontal:=width(p)
  else
    basepoint_horizontal:=0;
  end
else begin
  effective_horizontal:=height(p)+depth(p);
  if not (is_mirrored(box_dir(p))) then
    if dir_eq(dir_primary[box_dir(p)])(dir_secondary[dvi_direction]) then
      basepoint_horizontal:=height(p)
    else
      basepoint_horizontal:=depth(p)
  else
    if dir_eq(dir_primary[box_dir(p)])(dir_secondary[dvi_direction]) then
      basepoint_horizontal:=depth(p)
    else
      basepoint_horizontal:=height(p);
  if dir_eq(dir_secondary[box_dir(p)])(dir_primary[dvi_direction]) then
    basepoint_vertical:= -(width(p) div 2)
  else
    basepoint_vertical:= (width(p) div 2);
  end;
if not (is_mirrored(dvi_direction)) then
  basepoint_vertical := basepoint_vertical + shift_amount(p)
    {shift the box `down'}
else
  basepoint_vertical := basepoint_vertical - shift_amount(p);
    {shift the box `up'}
if list_ptr(p)=null then cur_h:=cur_h + effective_horizontal
else begin
  temp_ptr:=p; edge:=cur_h; cur_h:=cur_h + basepoint_horizontal;
  edge_v:=cur_v; cur_v:=base_line + basepoint_vertical;
  synch_h; synch_v; save_h:=dvi_h; save_v:=dvi_v;
  if type(p)=vlist_node then vlist_out@+else hlist_out;
  dvi_h:=save_h; dvi_v:=save_v;
  cur_h:=edge+effective_horizontal; cur_v:=base_line;
  end
end
@z
%-------------------------
@x [32] m.624
  dvi_out(set_rule); dvi_four(rule_ht); dvi_four(rule_wd);
@y
  case font_direction(dvi_direction) of
  dir__LT: begin
    dvi_set_rule(rule_ht)(rule_wd);
    end;
  dir__LB: begin
    dvi_down(rule_ht);
    dvi_set_rule(rule_ht)(rule_wd);
    dvi_up(rule_ht);
    end;
  dir__RT: begin
    dvi_left(rule_wd);
    dvi_put_rule(rule_ht)(rule_wd);
    end;
  dir__RB: begin
    dvi_left(rule_wd);
    dvi_down(rule_ht);
    dvi_put_rule(rule_ht)(rule_wd);
    dvi_up(rule_ht);
    end;
  dir__TL: begin
    dvi_down(rule_wd);
    dvi_left(rule_ht);
    dvi_set_rule(rule_wd)(rule_ht);
    end;
  dir__TR: begin
    dvi_down(rule_wd);
    dvi_put_rule(rule_wd)(rule_ht);
    end;
  dir__BL: begin
    dvi_left(rule_ht);
    dvi_set_rule(rule_wd)(rule_ht);
    dvi_up(rule_wd);
    end;
  dir__BR: begin
    dvi_put_rule(rule_wd)(rule_ht);
    dvi_up(rule_wd);
    end;
  end;
@z
%-------------------------
@x [32] m.626
leader_wd:=width(leader_box);
@y
if not (dir_orthogonal(dir_primary[box_dir(leader_box)])(dir_primary[dvi_direction]))
  then leader_wd:=width(leader_box)
else
  leader_wd:=height(leader_box)+depth(leader_box);
@z
%-------------------------
%@x [32] m.628
%@<Output a leader box at |cur_h|, ...@>=
%begin cur_v:=base_line+shift_amount(leader_box); synch_v; save_v:=dvi_v;@/
%synch_h; save_h:=dvi_h; temp_ptr:=leader_box;
%outer_doing_leaders:=doing_leaders; doing_leaders:=true;
%if type(leader_box)=vlist_node then vlist_out@+else hlist_out;
%doing_leaders:=outer_doing_leaders;
%dvi_v:=save_v; dvi_h:=save_h; cur_v:=base_line;
%cur_h:=save_h+leader_wd+lx;
%end
%@y
%@<Output a leader box at |cur_h|, ...@>=
%begin
%if not (dir_orthogonal(dir_primary[box_dir(leader_box)])(dir_primary[dvi_direction]))
%  then begin
%  basepoint_vertical:=0;
% if dir_opposite(dir_secondary[box_dir(leader_box)])(dir_secondary[dvi_direction]) then
%    basepoint_horizontal:=width(leader_box)
%  else
%    basepoint_horizontal:=0;
%  end
%else begin
%  if not (is_mirrored(box_dir(leader_box))) then
%    if dir_eq(dir_primary[box_dir(leader_box)])(dir_secondary[dvi_direction]) then
%      basepoint_horizontal:=height(leader_box)
%    else
%      basepoint_horizontal:=depth(leader_box)
%  else
%    if dir_eq(dir_primary[box_dir(leader_box)])(dir_secondary[dvi_direction]) then
%      basepoint_horizontal:=depth(leader_box)
%    else
%      basepoint_horizontal:=height(leader_box);
%  if dir_eq(dir_secondary[box_dir(leader_box)])(dir_primary[dvi_direction]) then
%    basepoint_vertical:= -(width(leader_box) div 2)
%  else
%    basepoint_vertical:= (width(leader_box) div 2);
%  end;
%if not (is_mirrored(dvi_direction)) then
%  basepoint_vertical := basepoint_vertical + shift_amount(p)
%    {shift the box `down'}
%else
%  basepoint_vertical := basepoint_vertical - shift_amount(p);
%    {shift the box `up'}
%temp_ptr:=leader_box;
%edge:=cur_h; cur_h:=cur_h + basepoint_horizontal;
%edge_v:=cur_v; cur_v:=base_line + basepoint_vertical;
%synch_h; synch_v; save_h:=dvi_h; save_v:=dvi_v;
%outer_doing_leaders:=doing_leaders; doing_leaders:=true;
%if type(leader_box)=vlist_node then vlist_out@+else hlist_out;
%doing_leaders:=outer_doing_leaders;
%dvi_h:=save_h; dvi_v:=save_v;
%cur_h:=edge+leader_wd+lx; cur_v:=base_line;
%end
%@z
%-------------------------
@x [32] m.629
@!glue_temp:real; {glue value before rounding}
begin this_box:=temp_ptr; g_order:=glue_order(this_box);
g_sign:=glue_sign(this_box); p:=list_ptr(this_box);
incr(cur_s);
if cur_s>0 then dvi_out(push);
if cur_s>max_push then max_push:=cur_s;
save_loc:=dvi_offset+dvi_ptr; left_edge:=cur_h; cur_v:=cur_v-height(this_box);
top_edge:=cur_v;
while p<>null do @<Output node |p| for |vlist_out| and move to the next node,
  maintaining the condition |cur_h=left_edge|@>;
prune_movements(save_loc);
if cur_s>0 then dvi_pop(save_loc);
decr(cur_s);
@y
@!glue_temp:real; {glue value before rounding}
@!save_direction: integer;
@!effective_vertical: scaled;
@!basepoint_horizontal: scaled;
@!basepoint_vertical: scaled;
@!edge_v: scaled;
@!saving_v: scaled;
@!saving_h: scaled;
begin this_box:=temp_ptr; g_order:=glue_order(this_box);
g_sign:=glue_sign(this_box); p:=list_ptr(this_box);
saving_v:=dvi_v; saving_h:=dvi_h;
save_direction:=dvi_direction;
dvi_direction:=box_dir(this_box);
incr(cur_s);
if cur_s>0 then dvi_out(push);
if cur_s>max_push then max_push:=cur_s;
save_loc:=dvi_offset+dvi_ptr; left_edge:=cur_h; cur_v:=cur_v-height(this_box);
top_edge:=cur_v;
while p<>null do @<Output node |p| for |vlist_out| and move to the next node,
  maintaining the condition |cur_h=left_edge|@>;
cur_v:=saving_v; cur_h:=saving_h;
synch_v; synch_h;
prune_movements(save_loc);
if cur_s>0 then dvi_pop(save_loc);
decr(cur_s);
dvi_direction:=save_direction;
@z
%-------------------------
@x [32] m.631
rule_node: begin rule_ht:=height(p); rule_dp:=depth(p); rule_wd:=width(p);
  goto fin_rule;
  end;
@y
rule_node: begin
  if not (dir_orthogonal(dir_primary[rule_dir(p)])(dir_primary[dvi_direction]))
    then begin
    rule_ht:=height(p); rule_dp:=depth(p); rule_wd:=width(p);
    end
  else begin
    rule_ht:=width(p) div 2;
    rule_dp:=width(p) div 2;
    rule_wd:=height(p)+depth(p);
    end;
  goto fin_rule;
  end;
@z
%-------------------------
@x [32] m.632
@<Output a box in a vlist@>=
if list_ptr(p)=null then cur_v:=cur_v+height(p)+depth(p)
else  begin cur_v:=cur_v+height(p); synch_v;
  save_h:=dvi_h; save_v:=dvi_v;
  cur_h:=left_edge+shift_amount(p); {shift the box right}
  temp_ptr:=p;
  if type(p)=vlist_node then vlist_out@+else hlist_out;
  dvi_h:=save_h; dvi_v:=save_v;
  cur_v:=save_v+depth(p); cur_h:=left_edge;
  end
@y
@<Output a box in a vlist@>=
begin
if not (dir_orthogonal(dir_primary[box_dir(p)])(dir_primary[dvi_direction]))
  then begin
  effective_vertical:=height(p)+depth(p);
  if (type(p)=hlist_node) and (is_mirrored(box_dir(p))) then
    basepoint_vertical:=depth(p)
  else
    basepoint_vertical:=height(p);
  if dir_opposite(dir_secondary[box_dir(p)])(dir_secondary[dvi_direction]) then
    basepoint_horizontal:=width(p)
  else
    basepoint_horizontal:=0;
  end
else begin
  effective_vertical:=width(p);
  if not (is_mirrored(box_dir(p))) then
    if dir_eq(dir_primary[box_dir(p)])(dir_secondary[dvi_direction]) then
      basepoint_horizontal:=height(p)
    else
      basepoint_horizontal:=depth(p)
  else
    if dir_eq(dir_primary[box_dir(p)])(dir_secondary[dvi_direction]) then
      basepoint_horizontal:=depth(p)
    else
      basepoint_horizontal:=height(p);
  if dir_eq(dir_secondary[box_dir(p)])(dir_primary[dvi_direction]) then
    basepoint_vertical:=0
  else
    basepoint_vertical:=width(p);
  end;
basepoint_horizontal := basepoint_horizontal + shift_amount(p);
  {shift the box `right'}
if list_ptr(p)=null then begin
  cur_v:=cur_v+effective_vertical;
  end
else  begin
  synch_h; synch_v; edge_v:=cur_v;
  cur_h:=left_edge + basepoint_horizontal;
  cur_v:=cur_v + basepoint_vertical;
  synch_h; synch_v; save_h:=dvi_h; save_v:=dvi_v;
  temp_ptr:=p;
  if type(p)=vlist_node then vlist_out@+else hlist_out;
  dvi_h:=save_h; dvi_v:=save_v;
  cur_h:=left_edge; cur_v:=edge_v + effective_vertical;
  end
end
@z
%-------------------------
@x [32] m.633
  dvi_out(put_rule); dvi_four(rule_ht); dvi_four(rule_wd);
@y
  case font_direction(dvi_direction) of
  dir__LT: begin
    dvi_put_rule(rule_ht)(rule_wd);
    end;
  dir__LB: begin
    dvi_down(rule_ht);
    dvi_put_rule(rule_ht)(rule_wd);
    dvi_up(rule_ht);
    end;
  dir__RT: begin
    dvi_left(rule_wd);
    dvi_set_rule(rule_ht)(rule_wd);
    end;
  dir__RB: begin
    dvi_down(rule_ht);
    dvi_left(rule_wd);
    dvi_set_rule(rule_ht)(rule_wd);
    dvi_up(rule_ht);
    end;
  dir__TL: begin
    dvi_down(rule_wd);
    dvi_left(rule_ht);
    dvi_set_rule(rule_wd)(rule_ht);
    dvi_up(rule_wd);
    end;
  dir__TR: begin
    dvi_down(rule_wd);
    dvi_put_rule(rule_wd)(rule_ht);
    dvi_up(rule_wd);
    end;
  dir__BL: begin
    dvi_left(rule_ht);
    dvi_set_rule(rule_wd)(rule_ht);
    end;
  dir__BR: begin
    dvi_put_rule(rule_wd)(rule_ht);
    end;
  end;
@z
%-------------------------
@x [32] m.635
leader_ht:=height(leader_box)+depth(leader_box);
@y
if not (dir_orthogonal(dir_primary[box_dir(leader_box)])(dir_primary[dvi_direction]))
  then leader_ht:=height(leader_box)+depth(leader_box)
else
  leader_ht:=width(leader_box);
@z
%-------------------------
@x [32] m.637
@<Output a leader box at |cur_v|, ...@>=
begin cur_h:=left_edge+shift_amount(leader_box); synch_h; save_h:=dvi_h;@/
cur_v:=cur_v+height(leader_box); synch_v; save_v:=dvi_v;
temp_ptr:=leader_box;
outer_doing_leaders:=doing_leaders; doing_leaders:=true;
if type(leader_box)=vlist_node then vlist_out@+else hlist_out;
doing_leaders:=outer_doing_leaders;
dvi_v:=save_v; dvi_h:=save_h; cur_h:=left_edge;
cur_v:=save_v-height(leader_box)+leader_ht+lx;
end
@y
@<Output a leader box at |cur_v|, ...@>=
begin
if not (dir_orthogonal(dir_primary[box_dir(p)])(dir_primary[dvi_direction]))
  then begin
  effective_vertical:=height(p)+depth(p);
  if (type(p)=hlist_node) and (is_mirrored(box_dir(p))) then
    basepoint_vertical:=depth(p)
  else
    basepoint_vertical:=height(p);
  if dir_opposite(dir_secondary[box_dir(p)])(dir_secondary[dvi_direction]) then
    basepoint_horizontal:=width(p)
  else
    basepoint_horizontal:=0;
  end
else begin
  effective_vertical:=width(p);
  if not (is_mirrored(box_dir(p))) then
    if dir_eq(dir_primary[box_dir(p)])(dir_secondary[dvi_direction]) then
      basepoint_horizontal:=height(p)
    else
      basepoint_horizontal:=depth(p)
  else
    if dir_eq(dir_primary[box_dir(p)])(dir_secondary[dvi_direction]) then
      basepoint_horizontal:=depth(p)
    else
      basepoint_horizontal:=height(p);
  if dir_eq(dir_secondary[box_dir(p)])(dir_primary[dvi_direction]) then
    basepoint_vertical:= width(p)
  else
    basepoint_vertical:= 0;
  end;
basepoint_horizontal := basepoint_horizontal + shift_amount(p);
  {shift the box `right'}
temp_ptr:=leader_box;
cur_h:=left_edge + basepoint_horizontal;
cur_v:=cur_v + basepoint_vertical;
synch_h; synch_v; save_h:=dvi_h; save_v:=dvi_v;
outer_doing_leaders:=doing_leaders; doing_leaders:=true;
if type(leader_box)=vlist_node then vlist_out@+else hlist_out;
doing_leaders:=outer_doing_leaders;
dvi_h:=save_h; dvi_v:=save_v;
cur_h:=left_edge; cur_v:=edge_v+leader_ht+lx;
end
@z
%-------------------------
@x [32] m.640
cur_v:=height(p)+v_offset; temp_ptr:=p;
if type(p)=vlist_node then vlist_out@+else hlist_out;
@y
dvi_direction:=page_direction;
case box_direction(dvi_direction) of
dir_TL_,dir_LT_: begin
  end;
dir_TR_,dir_RT_: begin
  dvi_right(page_right_offset);
  end;
dir_RB_,dir_BR_: begin
  dvi_right(page_right_offset);
  dvi_down(page_bottom_offset);
  end;
dir_BL_,dir_LB_: begin
  dvi_down(page_bottom_offset);
  end;
end;
cur_h:=h_offset;
cur_v:=height(p)+v_offset;
case box_direction(dvi_direction) of
dir_TL_: begin
  dvi_down(cur_v);
  dvi_right(cur_h);
  end;
dir_TR_: begin
  dvi_down(cur_v);
  dvi_right(-cur_h);
  end;
dir_LT_: begin
  dvi_right(cur_v);
  dvi_down(cur_h);
  end;
dir_LB_: begin
  dvi_right(cur_v);
  dvi_down(-cur_h);
  end;
dir_BL_: begin
  dvi_down(-cur_v);
  dvi_right(cur_h);
  end;
dir_BR_: begin
  dvi_down(-cur_v);
  dvi_right(-cur_h);
  end;
dir_RT_: begin
  dvi_right(-cur_v);
  dvi_down(cur_h);
  end;
dir_RB_: begin
  dvi_right(-cur_v);
  dvi_down(-cur_h);
  end;
end;
dvi_h:=cur_h;
dvi_v:=cur_v;
temp_ptr:=p;
if type(p)=vlist_node then vlist_out@+else hlist_out;
@z
%-------------------------
@x [33] m.644
@d exactly=0 {a box dimension is pre-specified}
@d additional=1 {a box dimension is increased from the natural one}
@d natural==0,additional {shorthand for parameters to |hpack| and |vpack|}
@y
@d exactly=0 {a box dimension is pre-specified}
@d additional=1 {a box dimension is increased from the natural one}
@d natural==0,additional {shorthand for parameters to |hpack| and |vpack|}

@<Glob...@>=
pack_direction:integer;
spec_direction:integer;

@z
%-------------------------
@x [33] m.645
@p procedure scan_spec(@!c:group_code;@!three_codes:boolean);
  {scans a box specification and left brace}
label found;
var @!s:integer; {temporarily saved value}
@!spec_code:exactly..additional;
begin if three_codes then s:=saved(0);
if scan_keyword("to") then spec_code:=exactly
@.to@>
else if scan_keyword("spread") then spec_code:=additional
@.spread@>
else  begin spec_code:=additional; cur_val:=0;
  goto found;
  end;
scan_normal_dimen;
found: if three_codes then
  begin saved(0):=s; incr(save_ptr);
  end;
saved(0):=spec_code; saved(1):=cur_val; save_ptr:=save_ptr+2;
new_save_level(c); scan_left_brace;
end;
@y
@p procedure scan_spec(@!c:group_code;@!three_codes:boolean);
  {scans a box specification and left brace}
label found;
var @!s:integer; {temporarily saved value}
@!spec_code:exactly..additional;
begin if three_codes then s:=saved(0);
if (c<>align_group) and (c<>vcenter_group) then begin
  if scan_keyword("dir") then begin
    scan_dir; spec_direction:=cur_val;
    end;
  end;
if scan_keyword("to") then spec_code:=exactly
@.to@>
else if scan_keyword("spread") then spec_code:=additional
@.spread@>
else  begin spec_code:=additional; cur_val:=0;
  goto found;
  end;
scan_normal_dimen;
found: if three_codes then
  begin saved(0):=s; incr(save_ptr);
  end;
saved(0):=spec_code; saved(1):=cur_val;
if (c<>align_group) and (c<>vcenter_group) then begin
  saved(2):=spec_direction;
  @<DIR: Adjust |text_dir_ptr| for |scan_spec|@>;
  save_ptr:=save_ptr+4;
  new_save_level(c); scan_left_brace;
  eq_word_define(dir_base+body_direction_code,spec_direction);
  eq_word_define(dir_base+par_direction_code,spec_direction);
  eq_word_define(dir_base+text_direction_code,spec_direction);
  eq_word_define(int_base+level_local_dir_code,cur_level);
  end
else begin
  save_ptr:=save_ptr+2;
  new_save_level(c); scan_left_brace;
  end;
spec_direction:=-1;
end;
@z
%-------------------------
@x [33] m.649
@p function hpack(@!p:pointer;@!w:scaled;@!m:small_number):pointer;
label reswitch, common_ending, exit;
var r:pointer; {the box node that will be returned}
@!q:pointer; {trails behind |p|}
@!h,@!d,@!x:scaled; {height, depth, and natural width}
@!s:scaled; {shift amount}
@!g:pointer; {points to a glue specification}
@!o:glue_ord; {order of infinity}
@!f:internal_font_number; {the font in a |char_node|}
@!i:four_quarters; {font information about a |char_node|}
@!hd:eight_bits; {height and depth indices for a character}
begin last_badness:=0; r:=get_node(box_node_size); type(r):=hlist_node;
@y
@p function hpack(@!p:pointer;@!w:scaled;@!m:small_number):pointer;
label reswitch, common_ending, exit;
var r:pointer; {the box node that will be returned}
@!q:pointer; {trails behind |p|}
@!h,@!d,@!x:scaled; {height, depth, and natural width}
@!s:scaled; {shift amount}
@!g:pointer; {points to a glue specification}
@!o:glue_ord; {order of infinity}
@!f:internal_font_number; {the font in a |char_node|}
@!i:four_quarters; {font information about a |char_node|}
@!hd:eight_bits; {height and depth indices for a character}
@!dir_tmp,@!dir_ptr:pointer; {for managing the direction stack}
@!hpack_dir:integer; {the current direction}
begin last_badness:=0; r:=get_node(box_node_size); type(r):=hlist_node;
if pack_direction=-1 then
  box_dir(r):=text_direction
else begin
  box_dir(r):=pack_direction; pack_direction:=-1;
  end;
hpack_dir:=box_dir(r);
@<DIR: Initialize |dir_ptr| for |hpack|@>;
@z
%-------------------------
@x [33] m.649
exit: hpack:=r;
@y
exit:
@<DIR: Reset |dir_ptr|@>;
hpack:=r;
@z
%-------------------------
@x [33] m.653
@<Incorporate box dimensions into the dimensions of the hbox...@>=
begin x:=x+width(p);
if type(p)>=rule_node then s:=0 @+else s:=shift_amount(p);
if height(p)-s>h then h:=height(p)-s;
if depth(p)+s>d then d:=depth(p)+s;
end
@y
@<Incorporate box dimensions into the dimensions of the hbox...@>=
begin
if (type(p)=hlist_node) or (type(p)=vlist_node) then begin
  if dir_orthogonal(dir_primary[box_dir(p)])(dir_primary[hpack_dir]) then begin
    x:=x+height(p)+depth(p);
    s:=shift_amount(p);
    if (width(p) div 2)-s>h then h:=(width(p) div 2)-s;
    if (width(p) div 2)+s>d then d:=(width(p) div 2)+s;
    end
  else if (type(p)=hlist_node) and (is_mirrored(hpack_dir)) then begin
    x:=x+width(p);
    s:=shift_amount(p);
    if depth(p)-s>h then h:=depth(p)-s;
    if height(p)+s>d then d:=height(p)+s;
    end
  else begin
    x:=x+width(p);
    s:=shift_amount(p);
    if height(p)-s>h then h:=height(p)-s;
    if depth(p)+s>d then d:=depth(p)+s;
    end
  end
else begin
  x:=x+width(p);
  if type(p)>=rule_node then s:=0 @+else s:=shift_amount(p);
  if height(p)-s>h then h:=height(p)-s;
  if depth(p)+s>d then d:=depth(p)+s;
  end;
end
@z
%-------------------------
@x [33] m.654
@<Incorporate character dimensions into the dimensions of the hbox...@>=
begin f:=font(p); i:=char_info(f)(character(p)); hd:=height_depth(i);
x:=x+char_width(f)(i);@/
s:=char_height(f)(hd);@+if s>h then h:=s;
s:=char_depth(f)(hd);@+if s>d then d:=s;
p:=link(p);
end
@y
@<Incorporate character dimensions into the dimensions of the hbox...@>=
begin f:=font(p); i:=char_info(f)(character(p)); hd:=height_depth(i);
if is_rotated(hpack_dir) then begin
  x:=x+char_height(f)(hd)+char_depth(f)(hd);@/
  s:=char_width(f)(i) div 2;@+if s>h then h:=s;
  s:=char_width(f)(i) div 2;@+if s>d then d:=s;
  end
else if dir_opposite(dir_tertiary[hpack_dir])(dir_tertiary[box_dir(r)])
  then begin
  x:=x+char_width(f)(i);@/
  s:=char_depth(f)(hd);@+if s>h then h:=s;
  s:=char_height(f)(hd);@+if s>d then d:=s;
  end
else begin
  x:=x+char_width(f)(i);@/
  s:=char_height(f)(hd);@+if s>h then h:=s;
  s:=char_depth(f)(hd);@+if s>d then d:=s;
  end;
p:=link(p);
end
@z
%-------------------------
@x [33] m.666
@ @<Report an overfull hbox and |goto common_ending|, if...@>=
if (-x-total_shrink[normal]>hfuzz)or(hbadness<100) then
  begin if (overfull_rule>0)and(-x-total_shrink[normal]>hfuzz) then
    begin while link(q)<>null do q:=link(q);
    link(q):=new_rule;
    width(link(q)):=overfull_rule;
    end;
@y
@ @<Report an overfull hbox and |goto common_ending|, if...@>=
if (-x-total_shrink[normal]>hfuzz)or(hbadness<100) then
  begin if (overfull_rule>0)and(-x-total_shrink[normal]>hfuzz) then
    begin while link(q)<>null do q:=link(q);
    link(q):=new_rule; rule_dir(q):=box_dir(r);
    width(link(q)):=overfull_rule;
    end;
@z
%-------------------------
@x [33] m.668
@p function vpackage(@!p:pointer;@!h:scaled;@!m:small_number;@!l:scaled):
  pointer;
label common_ending, exit;
var r:pointer; {the box node that will be returned}
@!w,@!d,@!x:scaled; {width, depth, and natural height}
@!s:scaled; {shift amount}
@!g:pointer; {points to a glue specification}
@!o:glue_ord; {order of infinity}
begin last_badness:=0; r:=get_node(box_node_size); type(r):=vlist_node;
@y
@p function vpackage(@!p:pointer;@!h:scaled;@!m:small_number;@!l:scaled):
  pointer;
label common_ending, exit;
var r:pointer; {the box node that will be returned}
@!w,@!d,@!x:scaled; {width, depth, and natural height}
@!s:scaled; {shift amount}
@!g:pointer; {points to a glue specification}
@!o:glue_ord; {order of infinity}
begin last_badness:=0; r:=get_node(box_node_size); type(r):=vlist_node;
if pack_direction=-1 then
  box_dir(r):=body_direction
else begin
  box_dir(r):=pack_direction; pack_direction:=-1;
  end;
@z
%-------------------------
@x [33] m.670
@ @<Incorporate box dimensions into the dimensions of the vbox...@>=
begin x:=x+d+height(p); d:=depth(p);
if type(p)>=rule_node then s:=0 @+else s:=shift_amount(p);
if width(p)+s>w then w:=width(p)+s;
end
@y
@ @<Incorporate box dimensions into the dimensions of the vbox...@>=
begin
if (type(p)=hlist_node) or (type(p)=vlist_node) then begin
  if dir_orthogonal(dir_primary[box_dir(p)])(dir_primary[box_dir(r)]) then begin
    x:=x+d+(width(p) div 2); d:=width(p) div 2;
    s:=shift_amount(p);
    if depth(p)+height(p)+s>w then w:=depth(p)+height(p)+s;
    end
  else if (type(p)=hlist_node) and (is_mirrored(box_dir(p))) then begin
    x:=x+d+depth(p); d:=height(p);
    s:=shift_amount(p);
    if width(p)+s>w then w:=width(p)+s;
    end
  else begin
    x:=x+d+height(p); d:=depth(p);
    s:=shift_amount(p);
    if width(p)+s>w then w:=width(p)+s;
    end
  end  
else
  begin x:=x+d+height(p); d:=depth(p);
  if type(p)>=rule_node then s:=0 @+else s:=shift_amount(p);
  if width(p)+s>w then w:=width(p)+s;
  end;
end
@z
%-------------------------
@x [33] m.679
  begin d:=width(baseline_skip)-prev_depth-height(b);
  if d<line_skip_limit then p:=new_param_glue(line_skip_code)
  else  begin p:=new_skip_param(baseline_skip_code);
    width(temp_ptr):=d; {|temp_ptr=glue_ptr(p)|}
    end;
  link(tail):=p; tail:=p;
  end;
link(tail):=b; tail:=b; prev_depth:=depth(b);
@y
  begin
  if (type(b)=hlist_node) and (is_mirrored(box_dir(b))) then
    d:=width(baseline_skip)-prev_depth-depth(b)
  else
    d:=width(baseline_skip)-prev_depth-height(b);
  if d<line_skip_limit then p:=new_param_glue(line_skip_code)
  else  begin p:=new_skip_param(baseline_skip_code);
    width(temp_ptr):=d; {|temp_ptr=glue_ptr(p)|}
    end;
  link(tail):=p; tail:=p;
  end;
link(tail):=b; tail:=b;
if (type(b)=hlist_node) and (is_mirrored(box_dir(b))) then 
  prev_depth:=height(b)
else
  prev_depth:=depth(b);
@z
%-------------------------
@x [35] m.704
@p function fraction_rule(@!t:scaled):pointer;
  {construct the bar for a fraction}
var p:pointer; {the new node}
begin p:=new_rule; height(p):=t; depth(p):=0; fraction_rule:=p;
end;
@y
@p function fraction_rule(@!t:scaled):pointer;
  {construct the bar for a fraction}
var p:pointer; {the new node}
begin p:=new_rule; rule_dir(p):=math_direction;
height(p):=t; depth(p):=0; fraction_rule:=p;
end;
@z
%-------------------------
@x [35] m.705
@p function overbar(@!b:pointer;@!k,@!t:scaled):pointer;
var p,@!q:pointer; {nodes being constructed}
begin p:=new_kern(k); link(p):=b; q:=fraction_rule(t); link(q):=p;
p:=new_kern(t); link(p):=q; overbar:=vpack(p,natural);
end;
@y
@p function overbar(@!b:pointer;@!k,@!t:scaled):pointer;
var p,@!q:pointer; {nodes being constructed}
begin p:=new_kern(k); link(p):=b; q:=fraction_rule(t); link(q):=p;
p:=new_kern(t); link(p):=q;
pack_direction:=math_direction; overbar:=vpack(p,natural);
end;
@z
%-------------------------
@x [36] m.735
procedure make_under(@!q:pointer);
var p,@!x,@!y: pointer; {temporary registers for box construction}
@!delta:scaled; {overall height plus depth}
begin x:=clean_box(nucleus(q),cur_style);
p:=new_kern(3*default_rule_thickness); link(x):=p;
link(p):=fraction_rule(default_rule_thickness);
y:=vpack(x,natural);
delta:=height(y)+depth(y)+default_rule_thickness;
height(y):=height(x); depth(y):=delta-height(y);
info(nucleus(q)):=y; math_type(nucleus(q)):=sub_box;
end;
@y
procedure make_under(@!q:pointer);
var p,@!x,@!y: pointer; {temporary registers for box construction}
@!delta:scaled; {overall height plus depth}
begin x:=clean_box(nucleus(q),cur_style);
p:=new_kern(3*default_rule_thickness); link(x):=p;
link(p):=fraction_rule(default_rule_thickness);
pack_direction:=math_direction; y:=vpack(x,natural);
delta:=height(y)+depth(y)+default_rule_thickness;
height(y):=height(x); depth(y):=delta-height(y);
info(nucleus(q)):=y; math_type(nucleus(q)):=sub_box;
end;
@z
%-------------------------
@x [36] m.738
  y:=vpack(y,natural); width(y):=width(x);
@y
  pack_direction:=math_direction;
  y:=vpack(y,natural); width(y):=width(x);
@z
%-------------------------
@x [36] m.759
x:=vpack(x,natural); shift_amount(x):=shift_down;
@y
pack_direction:=math_direction;
x:=vpack(x,natural); shift_amount(x):=shift_down;
@z
%-------------------------
@x [38] m.814
@!internal_right_box_width:integer; {running \.{\\localrightbox} width}
@!temp_no_whatsits:integer; {used when closing group}

@ @<Set init...@>=
@y
@!internal_right_box_width:integer; {running \.{\\localrightbox} width}
@!paragraph_dir:integer; {main direction of paragraph}
@!line_break_dir:integer; {current direction within paragraph}
@!break_c_htdp:quarterword; {height-depth entry in |char_info|}
@!temp_no_whatsits:integer; {used when closing group}
@!temp_no_dirs:integer; {used when closing group}
@!temporary_dir:integer;
@!dir_ptr,@!dir_tmp,@!dir_rover:pointer;

@ @<Set init...@>=
temp_no_whatsits:=0; temp_no_dirs:=0; temporary_dir:=0;
dir_ptr:=null; dir_tmp:=null; dir_rover:=null;
@z
%-------------------------
@x [39] m.816
link(tail):=new_param_glue(par_fill_skip_code);
@y
final_par_glue:=new_param_glue(par_fill_skip_code);
link(tail):=final_par_glue;
@z
%-------------------------
@x [38] m.816
init_cur_lang:=prev_graf mod @'200000;
init_l_hyf:=prev_graf div @'20000000;
init_r_hyf:=(prev_graf div @'200000) mod @'100;
@y
init_cur_lang:=prev_graf mod @'200000;
init_l_hyf:=prev_graf div @'20000000;
init_r_hyf:=(prev_graf div @'200000) mod @'100;
paragraph_dir:=local_par_dir(link(head));
line_break_dir:=paragraph_dir;
@<DIR: Initialize |dir_ptr| for |line_break|@>;
@z
%-------------------------
@x [39] m.821
@!pass_number:halfword; {the number of passive nodes allocated on this pass}
@y
@!pass_number:halfword; {the number of passive nodes allocated on this pass}
@!final_par_glue:pointer;
@z
%-------------------------
@x [38] m.841
@<Subtract the width of node |v|...@>=
if is_char_node(v) then
  begin f:=font(v);
  break_width[1]:=break_width[1]-char_width(f)(char_info(f)(character(v)));
  end
else  case type(v) of
  ligature_node: begin f:=font(lig_char(v));@/
    break_width[1]:=@|break_width[1]-
      char_width(f)(char_info(f)(character(lig_char(v))));
    end;
  hlist_node,vlist_node,rule_node,kern_node:
    break_width[1]:=break_width[1]-width(v);
@y
@<Subtract the width of node |v|...@>=
if is_char_node(v) then
  begin f:=font(v);
  if is_rotated(line_break_dir) then begin
    break_c_htdp:=height_depth(char_info(f)(character(v)));
    break_width[1]:=break_width[1]-char_height(f)(break_c_htdp)
                                  -char_depth(f)(break_c_htdp);
    end
  else
    break_width[1]:=break_width[1]-char_width(f)(char_info(f)(character(v)));
  end
else  case type(v) of
  ligature_node: begin f:=font(lig_char(v));@/
   if is_rotated(line_break_dir) then begin
     break_c_htdp:=height_depth(char_info(f)(character(lig_char(v))));
     break_width[1]:=break_width[1]-char_height(f)(break_c_htdp)
                                   -char_depth(f)(break_c_htdp);
     end
   else
     break_width[1]:=@|break_width[1]-
       char_width(f)(char_info(f)(character(lig_char(v))));
   end;
  hlist_node,vlist_node:
    if not (dir_orthogonal(dir_primary[box_dir(v)])(dir_primary[line_break_dir]))
      then break_width[1]:=break_width[1]-width(v)
    else
      break_width[1]:=break_width[1]-(depth(v)+height(v));
  rule_node,kern_node:
    break_width[1]:=break_width[1]-width(v);
@z
%-------------------------
@x [38] m.842
@ @<Add the width of node |s| to |b...@>=
if is_char_node(s) then
  begin f:=font(s);
  break_width[1]:=@|break_width[1]+char_width(f)(char_info(f)(character(s)));
  end
else  case type(s) of
  ligature_node: begin f:=font(lig_char(s));
    break_width[1]:=break_width[1]+
      char_width(f)(char_info(f)(character(lig_char(s))));
    end;
  hlist_node,vlist_node,rule_node,kern_node:
    break_width[1]:=break_width[1]+width(s);
@y
@ @<Add the width of node |s| to |b...@>=
if is_char_node(s) then
  begin f:=font(s);
  if is_rotated(line_break_dir) then begin
    break_c_htdp:=height_depth(char_info(f)(character(s)));
    break_width[1]:=break_width[1]+char_height(f)(break_c_htdp)
                                  +char_depth(f)(break_c_htdp);
    end
  else
    break_width[1]:=@|break_width[1]+char_width(f)(char_info(f)(character(s)));
  end
else  case type(s) of
  ligature_node: begin f:=font(lig_char(s));
   if is_rotated(line_break_dir) then begin
     break_c_htdp:=height_depth(char_info(f)(character(lig_char(s))));
     break_width[1]:=break_width[1]+char_height(f)(break_c_htdp)
                                   +char_depth(f)(break_c_htdp);
     end
   else
     break_width[1]:=break_width[1]+
       char_width(f)(char_info(f)(character(lig_char(s))));
   end;
  hlist_node,vlist_node:
    if not (dir_orthogonal(dir_primary[box_dir(s)])(dir_primary[line_break_dir]))
      then break_width[1]:=break_width[1]+width(s)
    else
      break_width[1]:=break_width[1]+(depth(s)+height(s));
  rule_node,kern_node:
    break_width[1]:=break_width[1]+width(s);
@z
%-------------------------
@x [38] m.844
passive_right_box_width(q):=internal_right_box_width;
@y
passive_right_box_width(q):=internal_right_box_width;
@z
%-------------------------
@x [38] m.866
hlist_node,vlist_node,rule_node: act_width:=act_width+width(cur_p);
@y
hlist_node,vlist_node:
if not (dir_orthogonal(dir_primary[box_dir(cur_p)])(dir_primary[line_break_dir]))
then act_width:=act_width+width(cur_p)
else act_width:=act_width+(depth(cur_p)+height(cur_p));
rule_node: act_width:=act_width+width(cur_p);
@z
%-------------------------
@x [38] m.866
  act_width:=act_width+char_width(f)(char_info(f)(character(lig_char(cur_p))));
@y
  if is_rotated(line_break_dir) then begin
    break_c_htdp:=height_depth(char_info(f)(character(lig_char(cur_p))));
    act_width:=act_width+char_height(f)(break_c_htdp)
                        +char_depth(f)(break_c_htdp);
    end
  else
    act_width:=act_width+char_width(f)(char_info(f)(character(lig_char(cur_p))));
@z
%-------------------------
@x [38] m.867
act_width:=act_width+char_width(f)(char_info(f)(character(cur_p)));
@y
if is_rotated(line_break_dir) then begin
    break_c_htdp:=height_depth(char_info(f)(character(cur_p)));
    act_width:=act_width+char_height(f)(break_c_htdp)
                        +char_depth(f)(break_c_htdp);
  end
else
  act_width:=act_width+char_width(f)(char_info(f)(character(cur_p)));
@z
%-------------------------
@x [38] m.870
@ @<Add the width of node |s| to |disc_width|@>=
if is_char_node(s) then
  begin f:=font(s);
  disc_width:=disc_width+char_width(f)(char_info(f)(character(s)));
  end
else  case type(s) of
  ligature_node: begin f:=font(lig_char(s));
    disc_width:=disc_width+
      char_width(f)(char_info(f)(character(lig_char(s))));
    end;
  hlist_node,vlist_node,rule_node,kern_node:
    disc_width:=disc_width+width(s);
@y
@ @<Add the width of node |s| to |disc_width|@>=
if is_char_node(s) then
  begin f:=font(s);
  if is_rotated(line_break_dir) then begin
      break_c_htdp:=height_depth(char_info(f)(character(s)));
      disc_width:=disc_width+char_height(f)(break_c_htdp)
                            +char_depth(f)(break_c_htdp);
    end
  else
    disc_width:=disc_width+char_width(f)(char_info(f)(character(s)));
  end
else  case type(s) of
  ligature_node: begin f:=font(lig_char(s));
    if is_rotated(line_break_dir) then begin
        break_c_htdp:=height_depth(char_info(f)(character(lig_char(s))));
        disc_width:=disc_width+char_height(f)(break_c_htdp)
                              +char_depth(f)(break_c_htdp);
      end
    else
      disc_width:=disc_width+
        char_width(f)(char_info(f)(character(lig_char(s))));
    end;
  hlist_node,vlist_node:
   if not (dir_orthogonal(dir_primary[box_dir(s)])(dir_primary[line_break_dir]))
    then disc_width:=disc_width+width(s)
    else disc_width:=disc_width+(depth(s)+height(s));
  rule_node,kern_node:
    disc_width:=disc_width+width(s);
@z
%-------------------------
@x [38] m.871
@ @<Add the width of node |s| to |act_width|@>=
if is_char_node(s) then
  begin f:=font(s);
  act_width:=act_width+char_width(f)(char_info(f)(character(s)));
  end
else  case type(s) of
  ligature_node: begin f:=font(lig_char(s));
    act_width:=act_width+
      char_width(f)(char_info(f)(character(lig_char(s))));
    end;
  hlist_node,vlist_node,rule_node,kern_node:
    act_width:=act_width+width(s);
@y
@ @<Add the width of node |s| to |act_width|@>=
if is_char_node(s) then
  begin f:=font(s);
  if is_rotated(line_break_dir) then begin
      break_c_htdp:=height_depth(char_info(f)(character(s)));
      act_width:=act_width+char_height(f)(break_c_htdp)
                          +char_depth(f)(break_c_htdp);
    end
  else
    act_width:=act_width+char_width(f)(char_info(f)(character(s)));
  end
else  case type(s) of
  ligature_node: begin f:=font(lig_char(s));
    if is_rotated(line_break_dir) then begin
        break_c_htdp:=height_depth(char_info(f)(character(lig_char(s))));
        act_width:=act_width+char_height(f)(break_c_htdp)
                            +char_depth(f)(break_c_htdp);
      end
    else
      act_width:=act_width+
        char_width(f)(char_info(f)(character(lig_char(s))));
    end;
  hlist_node,vlist_node:
   if not (dir_orthogonal(dir_primary[box_dir(s)])(dir_primary[line_break_dir]))
    then act_width:=act_width+width(s)
    else act_width:=act_width+(depth(s)+height(s));
  rule_node,kern_node:
    act_width:=act_width+width(s);
@z
%-------------------------
@x [39] m.877
@ The total number of lines that will be set by |post_line_break|
is |best_line-prev_graf-1|. The last breakpoint is specified by
|break_node(best_bet)|, and this passive node points to the other breakpoints
via the |prev_break| links. The finishing-up phase starts by linking the
relevant passive nodes in forward order, changing |prev_break| to
|next_break|. (The |next_break| fields actually reside in the same memory
space as the |prev_break| fields did, but we give them a new name because
of their new significance.) Then the lines are justified, one by one.

@d next_break==prev_break {new name for |prev_break| after links are reversed}

@<Declare subprocedures for |line_break|@>=
procedure post_line_break(@!final_widow_penalty:integer);
label done,done1;
var q,@!r,@!s:pointer; {temporary registers for list manipulation}
@!disc_break:boolean; {was the current break at a discretionary node?}
@!post_disc_break:boolean; {and did it have a nonempty post-break part?}
@!cur_width:scaled; {width of line number |cur_line|}
@!cur_indent:scaled; {left margin of line number |cur_line|}
@!t:quarterword; {used for replacement counts in discretionary nodes}
@!pen:integer; {use when calculating penalties between lines}
@!cur_line: halfword; {the current line number being justified}
begin @<Reverse the links of the relevant passive nodes, setting |cur_p| to the
  first breakpoint@>;
cur_line:=prev_graf+1;
repeat @<Justify the line ending at breakpoint |cur_p|, and append it to the
  current vertical list, together with associated penalties and other
  insertions@>;
incr(cur_line); cur_p:=next_break(cur_p);
if cur_p<>null then if not post_disc_break then
  @<Prune unwanted nodes at the beginning of the next line@>;
until cur_p=null;
if (cur_line<>best_line)or(link(temp_head)<>null) then
  confusion("line breaking");
@:this can't happen line breaking}{\quad line breaking@>
prev_graf:=best_line-1;
end;
@y
@ The total number of lines that will be set by |post_line_break|
is |best_line-prev_graf-1|. The last breakpoint is specified by
|break_node(best_bet)|, and this passive node points to the other breakpoints
via the |prev_break| links. The finishing-up phase starts by linking the
relevant passive nodes in forward order, changing |prev_break| to
|next_break|. (The |next_break| fields actually reside in the same memory
space as the |prev_break| fields did, but we give them a new name because
of their new significance.) Then the lines are justified, one by one.

The |post_line_break| must also keep an dir stack, so that it can
output end direction instructions at the ends of lines
and begin direction instructions at the beginnings of lines.

@d next_break==prev_break {new name for |prev_break| after links are reversed}

@<Declare subprocedures for |line_break|@>=
procedure post_line_break(@!final_widow_penalty:integer);
label done,done1;
var q,@!r,@!s:pointer; {temporary registers for list manipulation}
@!disc_break:boolean; {was the current break at a discretionary node?}
@!post_disc_break:boolean; {and did it have a nonempty post-break part?}
@!cur_width:scaled; {width of line number |cur_line|}
@!cur_indent:scaled; {left margin of line number |cur_line|}
@!t:quarterword; {used for replacement counts in discretionary nodes}
@!pen:integer; {use when calculating penalties between lines}
@!cur_line: halfword; {the current line number being justified}
begin dir_ptr:=dir_save;
@<Reverse the links of the relevant passive nodes, setting |cur_p| to the
  first breakpoint@>;
cur_line:=prev_graf+1;
repeat @<Justify the line ending at breakpoint |cur_p|, and append it to the
  current vertical list, together with associated penalties and other
  insertions@>;
incr(cur_line); cur_p:=next_break(cur_p);
if cur_p<>null then if not post_disc_break then
  @<Prune unwanted nodes at the beginning of the next line@>;
until cur_p=null;
if (cur_line<>best_line)or(link(temp_head)<>null) then
  confusion("line breaking");
@:this can't happen line breaking}{\quad line breaking@>
prev_graf:=best_line-1; dir_save:=dir_ptr;
end;
@z
%-------------------------
@x [39] m.880
@<Justify the line ending at breakpoint |cur_p|, and append it...@>=
@<Modify the end of the line to reflect the nature of the break and to include
  \.{\\rightskip}; also set the proper value of |disc_break|@>;
@<Put the \(l)\.{\\leftskip} glue at the left and detach this line@>;
@y
@<Justify the line ending at breakpoint |cur_p|, and append it...@>=
@<DIR: Insert dir nodes at the beginning of the current line@>;
@<DIR: Adjust the dir stack based on dir nodes in this line@>;
@<Modify the end of the line to reflect the nature of the break and to include
  \.{\\rightskip}; also set the proper value of |disc_break|@>;
@<Put the \(l)\.{\\leftskip} glue at the left and detach this line@>;
@z
%-------------------------
@x [39] m.881
    if passive_right_box(cur_p)<>null then begin
      r:=temp_head;
      while link(r)<>q do r:=link(r);
      s:=copy_node_list(passive_right_box(cur_p));
      link(r):=s;
      link(s):=q;
      end;
    delete_glue_ref(glue_ptr(q));
    glue_ptr(q):=right_skip;
    subtype(q):=right_skip_code+1; add_glue_ref(right_skip);
    goto done;
    end
  else  begin if type(q)=disc_node then
      @<Change discretionary to compulsory and set
        |disc_break:=true|@>
    else if (type(q)=math_node)or(type(q)=kern_node) then width(q):=0;
    end
else  begin q:=temp_head;
  while link(q)<>null do q:=link(q);
  end;
@y
     if passive_right_box(cur_p)<>null then begin
      r:=temp_head;
      while link(r)<>q do r:=link(r);
      @<DIR: Insert dir nodes at the end of the current line@>;
      s:=copy_node_list(passive_right_box(cur_p));
      link(r):=s;
      link(s):=q;
      end
     else begin
      r:=temp_head;
      while link(r)<>q do r:=link(r);
      @<DIR: Insert dir nodes at the end of the current line@>;
      end;
    delete_glue_ref(glue_ptr(q));
    glue_ptr(q):=right_skip;
    subtype(q):=right_skip_code+1; add_glue_ref(right_skip);
    goto done;
    end
  else  begin if type(q)=disc_node then
      @<Change discretionary to compulsory and set
        |disc_break:=true|@>
    else if (type(q)=math_node)or(type(q)=kern_node) then width(q):=0;
    end
else  begin q:=temp_head;
  while link(q)<>null do q:=link(q);
  end;
r:=q;
@<DIR: Insert dir nodes at the end of the current line@>;
@z
%-------------------------
@x [39] m.888
append_to_vlist(just_box);
if adjust_head<>adjust_tail then
  begin link(tail):=link(adjust_head); tail:=adjust_tail;
   end;
adjust_tail:=null
@y
append_to_vlist(just_box);
if adjust_head<>adjust_tail then
  begin link(tail):=link(adjust_head); tail:=adjust_tail;
   end;
adjust_tail:=null
@z
%-------------------------
@x [39] m.888
adjust_tail:=adjust_head; just_box:=hpack(q,cur_width,exactly);
@y
adjust_tail:=adjust_head;
pack_direction:=paragraph_dir;
just_box:=hpack(q,cur_width,exactly);
@z
%-------------------------
@x [44] m.977
@p function vsplit(@!n:eight_bits; @!h:scaled):pointer;
  {extracts a page of height |h| from box |n|}
label exit,done;
var v:pointer; {the box to be split}
p:pointer; {runs through the vlist}
q:pointer; {points to where the break occurs}
begin v:=box(n);
if split_first_mark<>null then
  begin delete_token_ref(split_first_mark); split_first_mark:=null;
  delete_token_ref(split_bot_mark); split_bot_mark:=null;
  end;
@<Dispense with trivial cases of void or bad boxes@>;
q:=vert_break(list_ptr(v),h,split_max_depth);
@<Look at all the marks in nodes before the break, and set the final
  link to |null| at the break@>;
q:=prune_page_top(q); p:=list_ptr(v); free_node(v,box_node_size);
@y
@p function vsplit(@!n:eight_bits; @!h:scaled):pointer;
  {extracts a page of height |h| from box |n|}
label exit,done;
var v:pointer; {the box to be split}
vdir:integer; {the direction of the box to be split}
p:pointer; {runs through the vlist}
q:pointer; {points to where the break occurs}
begin v:=box(n); vdir:=box_dir(v);
if split_first_mark<>null then
  begin delete_token_ref(split_first_mark); split_first_mark:=null;
  delete_token_ref(split_bot_mark); split_bot_mark:=null;
  end;
@<Dispense with trivial cases of void or bad boxes@>;
q:=vert_break(list_ptr(v),h,split_max_depth);
@<Look at all the marks in nodes before the break, and set the final
  link to |null| at the break@>;
q:=prune_page_top(q); p:=list_ptr(v); free_node(v,box_node_size);
pack_direction:=vdir;
@z
%-------------------------
@x [45] m.1001
if width(temp_ptr)>height(p) then width(temp_ptr):=width(temp_ptr)-height(p)
else width(temp_ptr):=0;
@y
if (type(p)=hlist_node) and (is_mirrored(body_direction)) then begin
  if width(temp_ptr)>depth(p) then width(temp_ptr):=width(temp_ptr)-depth(p)
  else width(temp_ptr):=0;
  end
else begin
  if width(temp_ptr)>height(p) then width(temp_ptr):=width(temp_ptr)-height(p)
  else width(temp_ptr):=0;
  end;
@z
%-------------------------
@x [45] m.1002
begin page_total:=page_total+page_depth+height(p);
page_depth:=depth(p);
@y
begin
if (type(p)=hlist_node) and (is_mirrored(body_direction)) then begin
  page_total:=page_total+page_depth+depth(p);
  page_depth:=height(p);
  end
else begin
  page_total:=page_total+page_depth+height(p);
  page_depth:=depth(p);
  end;
@z
%-------------------------
@x [45] m.1017
set_equiv(box_base+255,
          vpackage(link(page_head),best_size,exactly,page_max_depth));
@y
pack_direction:=body_direction;
set_equiv(box_base+255,
          vpackage(link(page_head),best_size,exactly,page_max_depth));
@z
%-------------------------
@x [45] m.1021
temp_ptr:=list_ptr(box(n));
free_node(box(n),box_node_size);
@y
temp_ptr:=list_ptr(box(n));
free_node(box(n),box_node_size);
pack_direction:=body_direction;
@z
%-------------------------
@x [47] m.1063
non_math(left_brace): begin
  new_save_level(simple_group);
  eq_word_define(int_base+no_local_whatsits_code,0);
  end;
any_mode(begin_group): begin
  new_save_level(semi_simple_group);
  eq_word_define(int_base+no_local_whatsits_code,0);
  end;
any_mode(end_group):
  if cur_group=semi_simple_group then begin
    temp_no_whatsits:=no_local_whatsits;
    unsave;
    if temp_no_whatsits<>0 then
      if abs(mode)=hmode then @<LOCAL: Add local paragraph node@>;
    end
  else off_save;
@y
non_math(left_brace): begin
  new_save_level(simple_group);
  eq_word_define(int_base+no_local_whatsits_code,0);
  eq_word_define(int_base+no_local_dirs_code,0);
  end;
any_mode(begin_group): begin
  new_save_level(semi_simple_group);
  eq_word_define(int_base+no_local_whatsits_code,0);
  eq_word_define(int_base+no_local_dirs_code,0);
  end;
any_mode(end_group):
  if cur_group=semi_simple_group then begin
    temp_no_whatsits:=no_local_whatsits;
    temp_no_dirs:=no_local_dirs;
    temporary_dir:=text_direction;
    if dir_level(text_dir_ptr)=cur_level then
      @<DIR: Remove from |text_dir_ptr|@>;
    unsave;
    if abs(mode)=hmode then begin
      if temp_no_dirs<>0 then begin
        @<DIR: Add local dir node@>;
        dir_dir(tail):=temporary_dir-64;
        end;
      if temp_no_whatsits<>0 then
        @<LOCAL: Add local paragraph node@>;
      end
    end
  else off_save;
@z
%-------------------------
@x [47] m.1068
simple_group: begin
  temp_no_whatsits:=no_local_whatsits;
  unsave;
  if temp_no_whatsits<>0 then
    if abs(mode)=hmode then @<LOCAL: Add local paragraph node@>;
  end;
@y
simple_group: begin
  temp_no_whatsits:=no_local_whatsits;
  temp_no_dirs:=no_local_dirs;
  temporary_dir:=text_direction;
  if dir_level(text_dir_ptr)=cur_level then
    @<DIR: Remove from |text_dir_ptr|@>;
  unsave;
  if abs(mode)=hmode then begin
    if temp_no_dirs<>0 then begin
      @<DIR: Add local dir node@>;
      dir_dir(tail):=temporary_dir-64;
      end;
    if temp_no_whatsits<>0 then
      @<LOCAL: Add local paragraph node@>;
    end
  end;
@z
%-------------------------
@x [47] m.1083
begin k:=cur_chr-vtop_code; saved(0):=box_context;
if k=hmode then
  if (box_context<box_flag)and(abs(mode)=vmode) then
    scan_spec(adjusted_hbox_group,true)
  else scan_spec(hbox_group,true)
else  begin if k=vmode then scan_spec(vbox_group,true)
  else  begin scan_spec(vtop_group,true); k:=vmode;
    end;
  normal_paragraph;
  end;
push_nest; mode:=-k;
@y
begin k:=cur_chr-vtop_code; saved(0):=box_context;
case abs(mode) of
vmode: spec_direction:=body_direction;
hmode: spec_direction:=text_direction;
mmode: spec_direction:=math_direction;
end;
if k=hmode then
  if (box_context<box_flag)and(abs(mode)=vmode) then
    scan_spec(adjusted_hbox_group,true)
  else scan_spec(hbox_group,true)
else  begin if k=vmode then scan_spec(vbox_group,true)
  else  begin scan_spec(vtop_group,true); k:=vmode;
    end;
  normal_paragraph;
  end;
push_nest; mode:=-k;
@z
%-------------------------
@x [47] m.1086
procedure package(@!c:small_number);
var h:scaled; {height of box}
@!p:pointer; {first node in a box}
@!d:scaled; {max depth}
begin d:=box_max_depth; unsave; save_ptr:=save_ptr-3;
if mode=-hmode then cur_box:=hpack(link(head),saved(2),saved(1))
else  begin cur_box:=vpackage(link(head),saved(2),saved(1),d);
  if c=vtop_code then @<Readjust the height and depth of |cur_box|,
    for \.{\\vtop}@>;
  end;
pop_nest; box_end(saved(0));
end;
@y
procedure package(@!c:small_number);
var h:scaled; {height of box}
@!p:pointer; {first node in a box}
@!d:scaled; {max depth}
begin d:=box_max_depth; unsave; save_ptr:=save_ptr-5;
pack_direction:=saved(3);
if mode=-hmode then cur_box:=hpack(link(head),saved(2),saved(1))
else  begin cur_box:=vpackage(link(head),saved(2),saved(1),d);
  if c=vtop_code then @<Readjust the height and depth of |cur_box|,
    for \.{\\vtop}@>;
  end;
if saved(4)<>null then
  @<DIR: Adjust back |text_dir_ptr| for |scan_spec|@>;
pop_nest; box_end(saved(0));
end;
@z
%-------------------------
@x [47] m.1091
procedure new_graf(@!indented:boolean);
begin prev_graf:=0;
if (mode=vmode)or(head<>tail) then
  tail_append(new_param_glue(par_skip_code));
push_nest; mode:=hmode; space_factor:=1000; set_cur_lang; clang:=cur_lang;
prev_graf:=(norm_min(left_hyphen_min)*@'100+norm_min(right_hyphen_min))
             *@'200000+cur_lang;
@<LOCAL: Add local paragraph node@>;
if indented then begin
  tail:=new_null_box; link(link(head)):=tail; width(tail):=par_indent;@+
  end;
@y
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
@z
%-------------------------
@x [47] m.1096
procedure end_graf;
begin if mode=hmode then
  begin if (head=tail) or (link(head)=tail) then pop_nest
        {null paragraphs are ignored, all contain a |local_paragraph| node}
  else line_break(widow_penalty);
  normal_paragraph;
  error_count:=0;
  end;
end;
@y
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
@z
%-------------------------
@x [48] m.1136
procedure push_math(@!c:group_code);
begin push_nest; mode:=-mmode; incompleat_noad:=null; new_save_level(c);
end;
@y
procedure push_math(@!c:group_code);
begin
if math_direction<>text_direction then dir_math_save:=true;
push_nest; mode:=-mmode; incompleat_noad:=null;
@<DIR: |new_save_level| math@>;
end;
@z
%-------------------------
@x [48] m.1174
begin unsave; p:=fin_mlist(null);
@y
begin @<DIR: |unsave| math@>; p:=fin_mlist(null);
@z
%-------------------------
@x [48] m.1186
math_group: begin unsave; decr(save_ptr);@/
@y
math_group: begin @<DIR: |unsave| math@>; decr(save_ptr);@/
@z
%-------------------------
@x [48] m.1191
  else  begin p:=fin_mlist(p); unsave; {end of |math_left_group|}
@y
  else  begin p:=fin_mlist(p);
    @<DIR: |unsave| math@>; {end of |math_left_group|}
@z
%-------------------------
@x [48] m.1194
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
  unsave; decr(save_ptr); {now |cur_group=math_shift_group|}
  if saved(0)=1 then l:=true;
@y
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
@z
%-------------------------
@x [48] m.1196
@<Finish math in text@>=
begin tail_append(new_math(math_surround,before));
cur_mlist:=p; cur_style:=text_style; mlist_penalties:=(mode>0); mlist_to_hlist;
link(tail):=link(temp_head);
while link(tail)<>null do tail:=link(tail);
tail_append(new_math(math_surround,after));
space_factor:=1000; unsave;
end
@y
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
@z
%-------------------------
@x [48] m.1200
procedure resume_after_display;
begin if cur_group<>math_shift_group then confusion("display");
@:this can't happen display}{\quad display@>
unsave; prev_graf:=prev_graf+3;
@y
procedure resume_after_display;
begin if cur_group<>math_shift_group then confusion("display");
@:this can't happen display}{\quad display@>
@<DIR: |unsave| math@>;
prev_graf:=prev_graf+3;
@z
%-------------------------
@x [49] m.1210
any_mode(assign_int),
@y
any_mode(assign_int),
any_mode(assign_dir),
@z
%-------------------------
@x [49] m.1228
assign_dimen: begin p:=cur_chr; scan_optional_equals;
  scan_normal_dimen; word_define(p,cur_val);
  end;
@y
assign_dir: begin
  @<DIR: Assign direction codes@>
  end;
assign_dimen: begin p:=cur_chr; scan_optional_equals;
  scan_normal_dimen; word_define(p,cur_val);
  end;
@z
%-------------------------
@x [49] m.1257
@!flushable_string:str_number; {string not yet referenced}
@y
@!natural_dir:integer;{the natural direction of the font}
@!flushable_string:str_number; {string not yet referenced}
@z
%-------------------------
@x [49] m.1257
if scan_keyword("offset") then begin
  scan_int;
  offset:=cur_val;
  if (cur_val<0) then begin
    print_err("Illegal offset has been changed to 0");
    help1("The offset must be bigger than 0."); int_error(cur_val);
    offset:=0;
    end
  end
else offset:=0;
name_in_progress:=false;
@<If this font has already been loaded, set |f| to the internal
  font number and |goto common_ending|@>;
f:=read_font_info(u,cur_name,cur_area,s,offset);
@y
if scan_keyword("offset") then begin
  scan_int;
  offset:=cur_val;
  if (cur_val<0) then begin
    print_err("Illegal offset has been changed to 0");
    help1("The offset must be bigger than 0."); int_error(cur_val);
    offset:=0;
    end
  end
else offset:=0;
if scan_keyword("naturaldir") then begin
  scan_dir;
  natural_dir:=cur_val;
  end
else natural_dir:=-1;
name_in_progress:=false;
@<If this font has already been loaded, set |f| to the internal
  font number and |goto common_ending|@>;
f:=read_font_info(u,cur_name,cur_area,s,offset,natural_dir);
@z
%-------------------------
@x [51] m.1337
if (loc<limit)and(cat_code(buffer[loc])<>escape) then start_input;
  {\.{\\input} assumed}
@y
if (loc<limit)and(cat_code(buffer[loc])<>escape) then start_input;
  {\.{\\input} assumed}
@<DIR: Initialize |text_dir_ptr|@>;
@z
%-------------------------
@x [53] m.1341
@d language_node=4 {|subtype| in whatsits that change the current language}
@y
@d language_node=4 {|subtype| in whatsits that change the current language}
@d dir_node_size=4 {number of words in change direction node}
@d dir_dir(#)==info(#+1) {keep the direction to be taken}
@d dir_level(#)==link(#+1) {the save level at which the direction was defined}
@d dir_dvi_ptr(#)==info(#+2) {for reflecting dvi output}
@d dir_dvi_h(#)==info(#+3) {for reflecting dvi output}
@z
%-------------------------
@x [53] m.1342
@!write_open:array[0..17] of boolean;
@y
@!write_open:array[0..17] of boolean;
@!neg_wd:scaled;
@!pos_wd:scaled;
@!neg_ht:scaled;
@!dvi_direction:integer;
@!dir_primary:array[0..31] of 0..31;
@!dir_secondary:array[0..31] of 0..31;
@!dir_tertiary:array[0..31] of 0..31;
@!dir_rearrange:array[0..3] of 0..31;
@!dir_names:array[0..3] of str_number;
@!text_dir_ptr:pointer;
@!text_dir_tmp:pointer;
@z
%-------------------------
@x [53] m.1343
for k:=0 to 17 do write_open[k]:=false;
@y
for k:=0 to 17 do write_open[k]:=false;
set_new_eqtb_int(dir_base+page_direction_code,0);
set_new_eqtb_int(dir_base+body_direction_code,0);
set_new_eqtb_int(dir_base+par_direction_code,0);
set_new_eqtb_int(dir_base+text_direction_code,0);
set_new_eqtb_int(dir_base+math_direction_code,0);
pack_direction:=-1;
{
set_new_eqtb_sc(dimen_base+page_height_code,55380984); {29.7cm}
set_new_eqtb_sc(dimen_base+page_width_code,39158276); {21cm}
}
set_new_eqtb_sc(dimen_base+page_height_code,15961652); {for talk}
set_new_eqtb_sc(dimen_base+page_width_code,23718717); {for talk}
set_new_eqtb_sc(dimen_base+page_bottom_offset_code,page_height-9472573);
                {-2 inches}
set_new_eqtb_sc(dimen_base+page_right_offset_code,page_width-9472573);
                {-2 inches}
for k:= 0 to 7 do begin
  dir_primary  [k   ]:=dir_T;
  dir_primary  [k+ 8]:=dir_L;
  dir_primary  [k+16]:=dir_B;
  dir_primary  [k+24]:=dir_R;
  end;
for k:= 0 to 3 do begin
  dir_secondary[k   ]:=dir_L;
  dir_secondary[k+ 4]:=dir_R;
  dir_secondary[k+ 8]:=dir_T;
  dir_secondary[k+12]:=dir_B;

  dir_secondary[k+16]:=dir_L;
  dir_secondary[k+20]:=dir_R;
  dir_secondary[k+24]:=dir_T;
  dir_secondary[k+28]:=dir_B;
  end;
for k:=0 to 7 do begin
  dir_tertiary[k*4  ]:=dir_T;
  dir_tertiary[k*4+1]:=dir_L;
  dir_tertiary[k*4+2]:=dir_B;
  dir_tertiary[k*4+3]:=dir_R;
  end;
dir_rearrange[0]:=0;
dir_rearrange[1]:=0;
dir_rearrange[2]:=1;
dir_rearrange[3]:=1;
dir_names[0]:="T";
dir_names[1]:="L";
dir_names[2]:="B";
dir_names[3]:="R";
@z
%-------------------------
@x [53] m.1344
@d local_par_node=6 {|subtype| in whatsits for local paragraph node}
@y
@d local_par_node=6 {|subtype| in whatsits for local paragraph node}
@d dir_node=7 {|subtype| in whatsits for change direction node}
@z
%-------------------------
@x [53] m.1344
primitive("localinterlinepenalty",assign_int,local_inter_line_penalty_code);@/
primitive("localbrokenpenalty",assign_int,local_broken_penalty_code);@/
@y
primitive("localinterlinepenalty",assign_int,local_inter_line_penalty_code);@/
primitive("localbrokenpenalty",assign_int,local_broken_penalty_code);@/
primitive("pagedir",assign_dir,dir_base+page_direction_code);@/
primitive("bodydir",assign_dir,dir_base+body_direction_code);@/
primitive("pardir",assign_dir,dir_base+par_direction_code);@/
primitive("textdir",assign_dir,dir_base+text_direction_code);@/
primitive("mathdir",assign_dir,dir_base+math_direction_code);@/
primitive("pageheight",assign_dimen,dimen_base+page_height_code);@/
primitive("pagewidth",assign_dimen,dimen_base+page_width_code);@/
{
primitive("pagerightoffset",assign_dimen,dimen_base+page_right_offset_code);@/
primitive("pagebottomoffset",assign_dimen,dimen_base+page_bottom_offset_code);@/
}
@z
%-------------------------
@x [53] m.1346
  set_language_code:print_esc("setlanguage");
  othercases print("[unknown extension!]")
  endcases;
@y
  set_language_code:print_esc("setlanguage");
  othercases print("[unknown extension!]")
  endcases;
@z
%-------------------------
@x [53] m.1348
set_language_code:@<Implement \.{\\setlanguage}@>;
@y
set_language_code:@<Implement \.{\\setlanguage}@>;
@z
%-------------------------
@x [53] m.1356
language_node:begin print_esc("setlanguage");
  print_int(what_lang(p)); print(" (hyphenmin ");
  print_int(what_lhm(p)); print_char(",");
  print_int(what_rhm(p)); print_char(")");
  end;
@y
language_node:begin print_esc("setlanguage");
  print_int(what_lang(p)); print(" (hyphenmin ");
  print_int(what_lhm(p)); print_char(",");
  print_int(what_rhm(p)); print_char(")");
  end;
dir_node:
  if dir_dir(p)<0 then begin
    print_esc("enddir"); print(" "); print_dir(dir_dir(p)+64);
    end
  else begin
    print_esc("begindir"); print(" "); print_dir(dir_dir(p));
    end;
@z
%-------------------------
@x [53] m.1357
close_node,language_node: begin r:=get_node(small_node_size);
  words:=small_node_size;
  end;
@y
close_node,language_node: begin r:=get_node(small_node_size);
  words:=small_node_size;
  end;
dir_node: begin r:=get_node(dir_node_size);
  words:=small_node_size;
  end;
@z
%-------------------------
@x [53] m.1358
close_node,language_node: free_node(p,small_node_size);
@y
close_node,language_node: free_node(p,small_node_size);
dir_node: free_node(p,dir_node_size);
@z
%-------------------------
@x [53] m.1360
@ @<Incorporate a whatsit node into an hbox@>=do_nothing
@y
@ @<Incorporate a whatsit node into an hbox@>=
begin
if subtype(p)=dir_node then 
  begin
  @<DIR: Adjust the dir stack for the |hpack| routine@>;
  end;
end
@z
%-------------------------
@x [53] m.1362
@<Advance \(p)past a whatsit node in the \(l)|line_break| loop@>=@+
adv_past(cur_p) else @<LOCAL: Advance past a |local_paragraph| node@>
@y
@<Advance \(p)past a whatsit node in the \(l)|line_break| loop@>=@+
adv_past(cur_p) else @<LOCAL: Advance past a |local_paragraph| node@>
else @<DIR: Adjust the dir stack for the |line_break| routine@>
@z
%-------------------------
@x [53] m.1366
@ @<Output the whatsit node |p| in an hlist@>=
out_what(p)
@y
@ @<Output the whatsit node |p| in an hlist@>= 
if subtype(p)<>dir_node then out_what(p)
else @<Output a reflection instruction if the direction has changed@>
@z
%-------------------------
@x
@d local_par_size==7
@y
@d local_par_dir(#)==mem[#+7].int
@d local_par_size==8
@z
%-------------------------
@x
make_local_par_node:=p;
@y
local_par_dir(p):=par_direction;
make_local_par_node:=p;
@z
%-------------------------
@x
@ @<LOCAL: local paragraph node@>=
((type(q)=whatsit_node) and (subtype(q)=local_par_node))

@y
@ @<LOCAL: local paragraph node@>=
((type(q)=whatsit_node) and (subtype(q)=local_par_node))

@ @<DIR: Assign direction codes@>=
case cur_chr of
dir_base+page_direction_code: begin
  if its_all_over then begin
    scan_dir;
    eq_word_define(dir_base+page_direction_code,cur_val);
    end;
  end;
dir_base+body_direction_code: begin
  if its_all_over then begin
    scan_dir;
    eq_word_define(dir_base+body_direction_code,cur_val);
    end;
  end;
dir_base+par_direction_code: begin
  scan_dir;
  eq_word_define(dir_base+par_direction_code,cur_val);
  end;
dir_base+text_direction_code: begin scan_dir;
  if (no_local_dirs>0) and (abs(mode)=hmode) then begin
    @<DIR: Add local dir node@>;
    dir_dir(tail):=dir_dir(tail)-64;
    end;
  if dir_level(text_dir_ptr)=cur_level then
    @<DIR: Remove from |text_dir_ptr|@>;
  eq_word_define(dir_base+text_direction_code,cur_val);
  @<DIR: Add to |text_dir_ptr|@>;
  if abs(mode)=hmode then
    begin
    @<DIR: Add local dir node@>;
    dir_level(tail):=cur_level;
    end;
  eq_word_define(int_base+no_local_dirs_code,no_local_dirs+1);
  eq_word_define(int_base+level_local_dir_code,cur_level);
  end;
dir_base+math_direction_code: begin scan_dir;
  eq_word_define(dir_base+math_direction_code,cur_val);
  end;
end;

@
@d push_dir(#)==
begin dir_tmp:=new_dir(#);
link(dir_tmp):=dir_ptr; dir_ptr:=dir_tmp;
dir_ptr:=dir_tmp;
end

@d push_dir_node(#)==
begin dir_tmp:=get_node(dir_node_size);
type(dir_tmp):=whatsit_node; subtype(dir_tmp):=dir_node;
dir_dir(dir_tmp):=dir_dir(#);
dir_level(dir_tmp):=dir_level(#);
dir_dvi_h(dir_tmp):=dir_dvi_h(#);
dir_dvi_ptr(dir_tmp):=dir_dvi_ptr(#);
link(dir_tmp):=dir_ptr; dir_ptr:=dir_tmp;
end

@d pop_dir_node==
begin dir_tmp:=dir_ptr;
dir_ptr:=link(dir_tmp);
free_node(dir_tmp,dir_node_size);
end

@ @<DIR: Declare |new_dir|@>=
function new_dir(s:small_number): pointer;
var p:pointer; {the new node}
begin p:=get_node(dir_node_size); type(p):=whatsit_node;
subtype(p):=dir_node; dir_dir(p):=s;
dir_dvi_h(p):=0; dir_dvi_ptr(p):=-1;
dir_level(p):=cur_level; new_dir:=p;
end

@ @<Append a begin direction to the tail of the current list@>=
tail_append(new_dir(math_direction))

@ @<Append an end direction to the tail of the current list@>=
tail_append(new_dir(math_direction-64))

@ @<DIR: Add local dir node@>=
tail_append(new_dir(text_direction))

@ @<DIR: Adjust the dir stack for the |line_break| routine@>=
if subtype(cur_p)=dir_node then
begin
if dir_dir(cur_p)>=0 then begin
  line_break_dir:=dir_dir(cur_p);
  push_dir_node(cur_p);
  end
else begin
  pop_dir_node;
  line_break_dir:=dir_dir(dir_ptr);
  end
end

@ @<DIR: Adjust the dir stack for the |hpack| routine@>=
begin
if dir_dir(p)>=0 then begin
  hpack_dir:=dir_dir(p);
  push_dir_node(p);
  end
else begin
  pop_dir_node;
  hpack_dir:=dir_dir(dir_ptr);
  end
end

@ @<DIR: Insert dir nodes at the beginning of the current line@>=
begin
dir_rover:=dir_ptr;
while dir_rover<>null do
  begin
  dir_tmp:=new_dir(dir_dir(dir_rover)); link(dir_tmp):=link(temp_head);
  link(temp_head):=dir_tmp; dir_rover:=link(dir_rover);
  end
end

@ @<DIR: Adjust the dir stack based on dir nodes in this line@>=
begin flush_node_list(dir_ptr); dir_ptr:=null;
q:=link(temp_head);
while q<>cur_break(cur_p) do
  begin if not is_char_node(q) then
    if type(q)=whatsit_node then
      if subtype(q)=dir_node then begin
        if dir_dir(q)>=0 then begin
          push_dir_node(q)
          end
        else if dir_ptr<>null then
          if dir_dir(dir_ptr)=(dir_dir(q)+64) then begin
            pop_dir_node;
            end;
        end;
  q:=link(q);
  end
end

@ @<DIR: Insert dir nodes at the end of the current line@>=
if dir_ptr<>null then
  begin
  if link(r)=q then begin
    dir_rover:=dir_ptr;
    while dir_rover<>null do
      begin
      dir_tmp:=new_dir(dir_dir(dir_rover)-64);
      link(dir_tmp):=q;
      link(r):=dir_tmp;
      r:=dir_tmp;
      dir_rover:=link(dir_rover);
      end;
    end
  else if r=final_par_glue then begin
    s:=temp_head;
    while link(s)<>r do
      s:=link(s);
    dir_rover:=dir_ptr;
    while dir_rover<>null do
      begin
      dir_tmp:=new_dir(dir_dir(dir_rover)-64);
      link(dir_tmp):=q;
      link(s):=dir_tmp;
      s:=dir_tmp;
      dir_rover:=link(dir_rover);
      end;
    end
  else begin
    dir_rover:=dir_ptr;
    s:=link(q);
    while dir_rover<>null do
      begin
      dir_tmp:=new_dir(dir_dir(dir_rover)-64);
      link(dir_tmp):=s;
      link(q):=dir_tmp;
      q:=dir_tmp;
      dir_rover:=link(dir_rover);
      end;
    r:=q;
    end
  end

@ @<DIR: Reset |dir_ptr|@>=
begin
 while dir_ptr<>null do
  pop_dir_node
end

@ @<DIR: |unsave| math@>=
begin
unsave;
decr(save_ptr);
flush_node_list(text_dir_ptr);
text_dir_ptr:=saved(0);
end

@ @<DIR: |new_save_level| math@>=
begin
saved(0):=text_dir_ptr;
text_dir_ptr:=new_dir(math_direction);
incr(save_ptr); 
new_save_level(c);
eq_word_define(dir_base+body_direction_code,math_direction);
eq_word_define(dir_base+par_direction_code,math_direction);
eq_word_define(dir_base+text_direction_code,math_direction);
eq_word_define(int_base+level_local_dir_code,cur_level);
end

@ @<DIR: Adjust |text_dir_ptr| for |scan_spec|@>=
begin
if spec_direction<>-1 then begin
  saved(3):=text_dir_ptr;
  text_dir_ptr:=new_dir(spec_direction);
  end
else saved(3):=null;
end

@ @<DIR: Adjust back |text_dir_ptr| for |scan_spec|@>=
begin
flush_node_list(text_dir_ptr);
text_dir_ptr:=saved(4);
end

@ @<DIR: Add to |text_dir_ptr|@>=
begin
text_dir_tmp:=new_dir(text_direction);
link(text_dir_tmp):=text_dir_ptr;
text_dir_ptr:=text_dir_tmp;
end

@ @<DIR: Remove from |text_dir_ptr|@>=
begin
text_dir_tmp:=link(text_dir_ptr);
free_node(text_dir_ptr,dir_node_size);
text_dir_ptr:=text_dir_tmp;
end

@ @<DIR: Initialize |text_dir_ptr|@>=
begin
text_dir_ptr:=new_dir(0);
end

@ @<DIR: Initialize |dir_ptr| for |ship_out|@>=
begin dir_ptr:=null; push_dir(dvi_direction);
dir_dvi_ptr(dir_ptr):=dvi_ptr;
end

@ @<DIR: Initialize |dir_ptr| for |hpack|@>=
begin dir_ptr:=null; push_dir(hpack_dir) end

@ @<DIR: Initialize |dir_ptr| for |line_break|@>=
begin dir_ptr:=null; push_dir(paragraph_dir); end

@ @<Output a reflection instruction if the direction has changed@>=
begin
if dir_dir(p)>=0 then
  begin synch_h; synch_v;
  push_dir_node(p);
  if dir_opposite(dir_secondary[dir_dir(dir_ptr)])(dir_secondary[dvi_direction])
  then begin
    if (dir_secondary[dvi_direction]=0) or
       (dir_secondary[dvi_direction]=2) then
      dvi_out(down4)
    else
      dvi_out(right4);
    dir_dvi_ptr(dir_ptr):=dvi_ptr;
    dir_dvi_h(dir_ptr):=dvi_h;
    dvi_four(0);
    cur_h:=0; dvi_h:=0;
    end
  else begin
    dir_dvi_ptr(dir_ptr):=dvi_ptr;
    dir_dvi_h(dir_ptr):=dvi_h;
    end;
  dvi_direction:=dir_dir(dir_ptr);
  end
else begin
  dvi_dir_h:=dir_dvi_h(dir_ptr);
  dvi_dir_ptr:=dir_dvi_ptr(dir_ptr);
  pop_dir_node;
  synch_h; synch_v;
  dvi_direction:=dir_dir(dir_ptr);
 if dir_opposite(dir_secondary[dir_dir(p)+64])(dir_secondary[dvi_direction])
  then begin
    dvi_temp_ptr:=dvi_ptr;
    dvi_ptr:=dvi_dir_ptr;
    if (dir_secondary[dvi_direction]=0) or
       (dir_secondary[dvi_direction]=1) then
      dvi_four(dvi_h)
    else
      dvi_four(-dvi_h);
    dvi_ptr:=dvi_temp_ptr;
    if (dir_secondary[dvi_direction]=0) or
       (dir_secondary[dvi_direction]=2) then
      dvi_out(down4)
    else
      dvi_out(right4);
    if (dir_secondary[dvi_direction]=0) or
       (dir_secondary[dvi_direction]=1) then
      dvi_four(dvi_h)
    else
      dvi_four(-dvi_h);
    end;
    cur_h:=dvi_dir_h+dvi_h; dvi_h:=cur_h;
  end
end
@z
