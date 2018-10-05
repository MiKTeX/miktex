% ompar.ch: Local paragraph information
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
%-------------------------
@x [15] m.208 l.4170
@d min_internal=char_ghost+1 {the smallest code that can follow \.{\\the}}
@y
@d assign_local_box=char_ghost+1
   {box for guillemets \.{\\localleftbox} or \.{\\localrightbox}}
@d min_internal=assign_local_box+1 {the smallest code that can follow \.{\\the}}
@z
%-------------------------
@x [16] m.212 l.4306
  @!aux_field: memory_word;
@y
  @!aux_field: memory_word;
  @!local_par_field: pointer;
  @!local_par_bool_field: boolean;
@z
%-------------------------
@x [16] m.213 l.4318
@d mode_line==cur_list.ml_field {source file line number at beginning of list}
@y
@d mode_line==cur_list.ml_field {source file line number at beginning of list}
@d local_par==cur_list.local_par_field
   {pointer to whatsit for local stuff}
@d local_par_bool==cur_list.local_par_bool_field
   {has local stuff been changed in this group?}
@z
%-------------------------
@x [16] m.215 l.4344
prev_graf:=0; shown_mode:=0;
@y
prev_graf:=0; shown_mode:=0;
local_par:=null; local_par_bool:=false;
@z
%-------------------------
@x [16] m.216 l.4351
@ When \TeX's work on one level is interrupted, the state is saved by
calling |push_nest|. This routine changes |head| and |tail| so that
a new (empty) list is begun; it does not change |mode| or |aux|.

@p procedure push_nest; {enter a new semantic level, save the old}
begin if nest_ptr>max_nest_stack then
  begin max_nest_stack:=nest_ptr;
  if nest_ptr=nest_size then overflow("semantic nest size",nest_size);
@:TeX capacity exceeded semantic nest size}{\quad semantic nest size@>
  end;
nest[nest_ptr]:=cur_list; {stack the record}
incr(nest_ptr); head:=get_avail; tail:=head; prev_graf:=0; mode_line:=line;
@y
@ @p @<LOCAL: Declare |make_local_par_node|@>;

@ When \TeX's work on one level is interrupted, the state is saved by
calling |push_nest|. This routine changes |head| and |tail| so that
a new (empty) list is begun; it does not change |mode| or |aux|.

@p procedure push_nest; {enter a new semantic level, save the old}
begin if nest_ptr>max_nest_stack then
  begin max_nest_stack:=nest_ptr;
  if nest_ptr=nest_size then overflow("semantic nest size",nest_size);
@:TeX capacity exceeded semantic nest size}{\quad semantic nest size@>
  end;
nest[nest_ptr]:=cur_list; {stack the record}
incr(nest_ptr); head:=get_avail; tail:=head; prev_graf:=0; mode_line:=line;
local_par:=make_local_par_node; local_par_bool:=false;
@z
%-------------------------
@x [16] m.217 l.4366
begin free_avail(head); decr(nest_ptr); cur_list:=nest[nest_ptr];
@y
begin
if local_par<>null then begin
  if local_par_bool then begin end {tail_append(local_par)}
  else free_node(local_par,local_par_size);
  end;
free_avail(head); decr(nest_ptr); cur_list:=nest[nest_ptr];
@z
%-------------------------
@x [17] m.230 l.4860
@d box_base=toks_base+number_regs {table of |number_regs| box registers}
@y
@d local_left_box_base=toks_base+number_regs
@d local_right_box_base=local_left_box_base+1
@d box_base=local_right_box_base+1 {table of |number_regs| box registers}
@z
%-------------------------
@x [17] m.230 l.4888
@d box(#)==equiv(box_base+#)
@y
@d local_left_box==equiv(local_left_box_base)
@d local_right_box==equiv(local_right_box_base)
@d box(#)==equiv(box_base+#)
@z
%-------------------------
@x [17] m.236 l.4954
@d int_pars=55 {total number of integer parameters}
@y
@d local_inter_line_penalty_code=55 {local \.{\\interlinepenalty}}
@d local_broken_penalty_code=56 {local \.{\\brokenpenalty}}
@d no_local_whatsits_code=57 {counts local whatsits}
@d int_pars=58 {total number of integer parameters}
@z
%-------------------------
@x [17] m.236 l.5011
@d new_line_char==int_par(new_line_char_code)
@y
@d new_line_char==int_par(new_line_char_code)
@d local_inter_line_penalty==int_par(local_inter_line_penalty_code)
@d local_broken_penalty==int_par(local_broken_penalty_code)
@d no_local_whatsits==int_par(no_local_whatsits_code)
@z
%-------------------------
@x [19] m.269 l.5860
@d max_group_code=16
@y
@d local_box_group=17 {code for `\.{\\localleftbox...\\localrightbox}'}
@d max_group_code=17
@z
%-------------------------
@x [38] m.814 l.15994
@!just_box:pointer; {the |hlist_node| for the last line of the new paragraph}
@y
@!just_box:pointer; {the |hlist_node| for the last line of the new paragraph}
@!internal_pen_inter:integer; {running \.{\\localinterlinepenalty}}
@!internal_pen_broken:integer; {running \.{\\localbrokenpenalty}}
@!internal_left_box:pointer; {running \.{\\localleftbox}}
@!internal_left_box_width:integer; {running \.{\\localleftbox} width}
@!init_internal_left_box:pointer; {running \.{\\localleftbox}}
@!init_internal_left_box_width:integer; {running \.{\\localleftbox} width}
@!internal_right_box:pointer; {running \.{\\localrightbox}}
@!internal_right_box_width:integer; {running \.{\\localrightbox} width}
@!temp_no_whatsits:integer; {used when closing group}

@ @<Set init...@>=
temp_no_whatsits:=0;
@z
%-------------------------
@x [38] m.815 l.16002
procedure line_break(@!final_widow_penalty:integer);
label done,done1,done2,done3,done4,done5,continue;
var @<Local variables for line breaking@>@;
@y
function get_natural_width(first_ptr:pointer):scaled;
label reswitch, common_ending, exit;
var
@!p:pointer;
@!x:scaled; {height, depth, and natural width}
@!g:pointer; {points to a glue specification}
@!f:internal_font_number; {the font in a |char_node|}
@!i:four_quarters; {font information about a |char_node|}
begin 
x:=0;
p:=first_ptr;
while p<>null do begin
  while is_char_node(p) do begin
    f:=font(p); i:=char_info(f)(character(p));
    x:=x+char_width(f)(i);
    p:=link(p);
    end;
  if p<>null then
    begin case type(p) of
    hlist_node,vlist_node,rule_node,unset_node:
      x:=x+width(p);
    ins_node,mark_node,adjust_node: do_nothing;
    whatsit_node: do_nothing;
    glue_node: begin
      g:=glue_ptr(p); x:=x+width(g);
      end;
    kern_node,math_node: x:=x+width(p);
    ligature_node: begin
      f:=font(p); i:=char_info(f)(character(lig_char(p)));
      x:=x+char_width(f)(i);
      end;
    othercases do_nothing
    endcases;
    p:=link(p);
    end;
  end;
get_natural_width:=x;
end;

procedure line_break(@!final_widow_penalty:integer);
label done,done1,done2,done3,done4,done5,continue;
var new_hsize: scaled;
@<Local variables for line breaking@>@;
@z
%-------------------------
@x [38] m.821 l.16110
@ The passive node for a given breakpoint contains only four fields:

\yskip\hang|link| points to the passive node created just before this one,
if any, otherwise it is |null|.

\yskip\hang|cur_break| points to the position of this breakpoint in the
horizontal list for the paragraph being broken.

\yskip\hang|prev_break| points to the passive node that should precede this
one in an optimal path to this breakpoint.

\yskip\hang|serial| is equal to |n| if this passive node is the |n|th
one created during the current pass. (This field is used only when
printing out detailed statistics about the line-breaking calculations.)

\yskip\noindent
There is a global variable called |passive| that points to the most
recently created passive node. Another global variable, |printed_node|,
is used to help print out the paragraph when detailed information about
the line-breaking computation is being displayed.

@d passive_node_size=2 {number of words in passive nodes}
@d cur_break==rlink {in passive node, points to position of this breakpoint}
@d prev_break==llink {points to passive node that should precede this one}
@d serial==info {serial number for symbolic identification}
@y
@ The passive node for a given breakpoint contains EIGHT fields:

\yskip\hang|link| points to the passive node created just before this one,
if any, otherwise it is |null|.

\yskip\hang|cur_break| points to the position of this breakpoint in the
horizontal list for the paragraph being broken.

\yskip\hang|prev_break| points to the passive node that should precede this
one in an optimal path to this breakpoint.

\yskip\hang|serial| is equal to |n| if this passive node is the |n|th
one created during the current pass. (This field is used only when
printing out detailed statistics about the line-breaking calculations.)

\yskip\hang|passive_pen_inter| holds the current \.{\\localinterlinepenalty}

\yskip\hang|passive_pen_broken| holds the current \.{\\localbrokenpenalty}

\yskip\noindent
There is a global variable called |passive| that points to the most
recently created passive node. Another global variable, |printed_node|,
is used to help print out the paragraph when detailed information about
the line-breaking computation is being displayed.

@d passive_node_size=10 {number of words in passive nodes}
@d cur_break==rlink {in passive node, points to position of this breakpoint}
@d prev_break==llink {points to passive node that should precede this one}
@d serial==info {serial number for symbolic identification}
@d passive_pen_inter(#)==mem[#+2].int
@d passive_pen_broken(#)==mem[#+3].int
@d passive_left_box(#)==mem[#+4].int
@d passive_left_box_width(#)==mem[#+5].int
@d passive_last_left_box(#)==mem[#+6].int
@d passive_last_left_box_width(#)==mem[#+7].int
@d passive_right_box(#)==mem[#+8].int
@d passive_right_box_width(#)==mem[#+9].int
@z
%-------------------------
@x [38] m.835 l.16407
@<If a line number class...@>=
begin l:=line_number(r);
if l>old_l then
  begin {now we are no longer in the inner loop}
@y
@<If a line number class...@>=
begin l:=line_number(r);
if l>old_l then
  begin {now we are no longer in the inner loop}
@z
%-------------------------
@x [38] m.844 l.16595
@<Insert a new active node from |best_place[fit_class]| to |cur_p|@>=
begin q:=get_node(passive_node_size);
link(q):=passive; passive:=q; cur_break(q):=cur_p;
@!stat incr(pass_number); serial(q):=pass_number;@+tats@;@/
prev_break(q):=best_place[fit_class];@/
@y
@<Insert a new active node from |best_place[fit_class]| to |cur_p|@>=
begin q:=get_node(passive_node_size);
link(q):=passive; passive:=q; cur_break(q):=cur_p;
@!stat incr(pass_number); serial(q):=pass_number;@+tats@;@/
prev_break(q):=best_place[fit_class];@/
{Here we keep track of the subparagraph penalties in the break nodes}
passive_pen_inter(q):=internal_pen_inter;
passive_pen_broken(q):=internal_pen_broken;
passive_last_left_box(q):=internal_left_box;
passive_last_left_box_width(q):=internal_left_box_width;
if prev_break(q)<>null then begin
  passive_left_box(q):=passive_last_left_box(prev_break(q));
  passive_left_box_width(q):=passive_last_left_box_width(prev_break(q));
  end
else begin
  passive_left_box(q):=init_internal_left_box;
  passive_left_box_width(q):=init_internal_left_box_width;
  end;
passive_right_box(q):=internal_right_box;
passive_right_box_width(q):=internal_right_box_width;
@z
%-------------------------
@x [38] m.848 l.16595
if par_shape_ptr=null then
  if hang_indent=0 then
    begin last_special_line:=0; second_width:=hsize;
@y
new_hsize := get_natural_width(link(temp_head)) div 7;
if new_hsize>hsize then new_hsize:=hsize;
if new_hsize<(hsize div 4) then new_hsize:=(hsize div 4);
new_hsize := hsize;
if par_shape_ptr=null then
  if hang_indent=0 then
    begin last_special_line:=0; second_width:=new_hsize;
@z
%-------------------------
@x [38] m.849 l.16595
  begin first_width:=hsize-abs(hang_indent);
  if hang_indent>=0 then first_indent:=hang_indent
  else first_indent:=0;
  second_width:=hsize; second_indent:=0;
  end
else  begin first_width:=hsize; first_indent:=0;
@y
  begin first_width:=new_hsize-abs(hang_indent);
  if hang_indent>=0 then first_indent:=hang_indent
  else first_indent:=0;
  second_width:=new_hsize; second_indent:=0;
  end
else  begin first_width:=new_hsize; first_indent:=0;
@z
%-------------------------
@x [38] m.851 l.16706
@<Consider the demerits for a line from |r| to |cur_p|...@>=
begin artificial_demerits:=false;@/
@^inner loop@>
shortfall:=line_width-cur_active_width[1]; {we're this much too short}
@y
@<Consider the demerits for a line from |r| to |cur_p|...@>=
begin artificial_demerits:=false;@/
@^inner loop@>
shortfall:=line_width-cur_active_width[1]; {we're this much too short}
if break_node(r)=null then
  shortfall:=shortfall-init_internal_left_box_width
else shortfall:=shortfall-passive_last_left_box_width(break_node(r));
shortfall:=shortfall-internal_right_box_width;
@z
%-------------------------
@x [39] m.863 l.16932
loop@+  begin if threshold>inf_bad then threshold:=inf_bad;
  if second_pass then @<Initialize for hyphenating a paragraph@>;
  @<Create an active breakpoint representing the beginning of the paragraph@>;
  cur_p:=link(temp_head); auto_breaking:=true;@/
  prev_p:=cur_p; {glue at beginning is not a legal breakpoint}
@y
loop@+  begin if threshold>inf_bad then threshold:=inf_bad;
  if second_pass then @<Initialize for hyphenating a paragraph@>;
  @<Create an active breakpoint representing the beginning of the paragraph@>;
  cur_p:=link(temp_head); auto_breaking:=true;@/
  prev_p:=cur_p; {glue at beginning is not a legal breakpoint}
  @<LOCAL: Initialize with first |local_paragraph| node@>;
@z
%-------------------------
@x [39] m.877 l.17206
procedure post_line_break(@!final_widow_penalty:integer);
label done,done1;
@y
procedure post_line_break(@!final_widow_penalty:integer);
label done,done1;
@z
%-------------------------
@x [39] m.879 l.17257
  if non_discardable(q) then goto done1;
  if type(q)=kern_node then if subtype(q)<>explicit then goto done1;
@y
  if @<LOCAL: local paragraph node@> then begin end
  else begin
    if non_discardable(q) then goto done1;
    if type(q)=kern_node then if subtype(q)<>explicit then goto done1;
    end;
@z
%-------------------------
@x [39] m.881 l.17289
q:=cur_break(cur_p); disc_break:=false; post_disc_break:=false;
if q<>null then {|q| cannot be a |char_node|}
  if type(q)=glue_node then
    begin delete_glue_ref(glue_ptr(q));
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
@<Put the \(r)\.{\\rightskip} glue after node |q|@>;
@y
q:=cur_break(cur_p); disc_break:=false; post_disc_break:=false;
if q<>null then {|q| cannot be a |char_node|}
  if type(q)=glue_node then begin
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
if passive_right_box(cur_p)<>null then begin
  r:=copy_node_list(passive_right_box(cur_p));
  link(r):=link(q);
  link(q):=r;
  q:=r;
  end;
@<Put the \(r)\.{\\rightskip} glue after node |q|@>;
@z
%-------------------------
@x [39] m.887 l.17352
@<Put the \(l)\.{\\leftskip} glue at the left...@>=
r:=link(q); link(q):=null; q:=link(temp_head); link(temp_head):=r;
if left_skip<>zero_glue then
  begin r:=new_param_glue(left_skip_code);
  link(r):=q; q:=r;
  end
@y
@<Put the \(l)\.{\\leftskip} glue at the left...@>=
r:=link(q); link(q):=null; q:=link(temp_head); link(temp_head):=r;
if passive_left_box(cur_p)<>null then begin
  r:=copy_node_list(passive_left_box(cur_p));
  s:=link(q);
  link(r):=q;
  q:=r;
  if (cur_line=prev_graf+1) and (s<>null) then
    if type(s)=hlist_node then
      if list_ptr(s)=null then begin
        q:=link(q);
        link(r):=link(s);
        link(s):=r;
        end;
  end;
if left_skip<>zero_glue then
  begin r:=new_param_glue(left_skip_code);
  link(r):=q; q:=r;
  end
@z
%-------------------------
@x [39] m.890 l.17391
@<Append a penalty node, if a nonzero penalty is appropriate@>=
if cur_line+1<>best_line then
  begin pen:=inter_line_penalty;
  if cur_line=prev_graf+1 then pen:=pen+club_penalty;
  if cur_line+2=best_line then pen:=pen+final_widow_penalty;
  if disc_break then pen:=pen+broken_penalty;
@y
@<Append a penalty node, if a nonzero penalty is appropriate@>=
if cur_line+1<>best_line then begin
  if passive_pen_inter(cur_p)<>0 then
      pen:=passive_pen_inter(cur_p)
  else pen:=inter_line_penalty;
  if cur_line=prev_graf+1 then pen:=pen+club_penalty;
  if cur_line+2=best_line then pen:=pen+final_widow_penalty;
  if disc_break then
    if passive_pen_broken(cur_p)<>0 then
      pen:=pen+passive_pen_broken(cur_p)
    else pen:=pen+broken_penalty;
@z
%-------------------------
@x [47] m.1063 l.20580
non_math(left_brace): new_save_level(simple_group);
any_mode(begin_group): new_save_level(semi_simple_group);
any_mode(end_group): if cur_group=semi_simple_group then unsave
  else off_save;
@y
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
@z
%-------------------------
@x [47] m.1068 l.20653
procedure handle_right_brace;
var p,@!q:pointer; {for short-term use}
@!d:scaled; {holds |split_max_depth| in |insert_group|}
@!f:integer; {holds |floating_penalty| in |insert_group|}
begin case cur_group of
simple_group: unsave;
@y
procedure handle_right_brace;
var p,@!q:pointer; {for short-term use}
@!d:scaled; {holds |split_max_depth| in |insert_group|}
@!f:integer; {holds |floating_penalty| in |insert_group|}
begin case cur_group of
simple_group: begin
  temp_no_whatsits:=no_local_whatsits;
  unsave;
  if temp_no_whatsits<>0 then
    if abs(mode)=hmode then @<LOCAL: Add local paragraph node@>;
  end;
@z
%-------------------------
@x [47] m.1091 l.21055
push_nest; mode:=hmode; space_factor:=1000; set_cur_lang; clang:=cur_lang;
prev_graf:=(norm_min(left_hyphen_min)*@'100+norm_min(right_hyphen_min))
             *@'200000+cur_lang;
if indented then
  begin tail:=new_null_box; link(head):=tail; width(tail):=par_indent;@+
  end;
@y
push_nest; mode:=hmode; space_factor:=1000; set_cur_lang; clang:=cur_lang;
prev_graf:=(norm_min(left_hyphen_min)*@'100+norm_min(right_hyphen_min))
             *@'200000+cur_lang;
@<LOCAL: Add local paragraph node@>;
if indented then begin
  tail:=new_null_box; link(link(head)):=tail; width(tail):=par_indent;@+
  end;
@z
%-------------------------
@x [47] m.1096 l.21116
procedure end_graf;
begin if mode=hmode then
  begin if head=tail then pop_nest {null paragraphs are ignored}
@y
procedure end_graf;
begin if mode=hmode then
  begin if (head=tail) or (link(head)=tail) then pop_nest
        {null paragraphs are ignored, all contain a |local_paragraph| node}
@z
%-------------------------
@x [47] m.1114 l.21322
primitive("-",discretionary,1);
@!@:Single-character primitives -}{\quad\.{\\-}@>
primitive("discretionary",discretionary,0);
@!@:discretionary_}{\.{\\discretionary} primitive@>

@ @<Cases of |print_cmd_chr|...@>=
discretionary: if chr_code=1 then
  print_esc("-")@+else print_esc("discretionary");

@ @<Cases of |main_control| that build...@>=
hmode+discretionary,mmode+discretionary: append_discretionary;
@y
primitive("-",discretionary,1);
@!@:Single-character primitives -}{\quad\.{\\-}@>
primitive("discretionary",discretionary,0);
@!@:discretionary_}{\.{\\discretionary} primitive@>
primitive("localleftbox",assign_local_box,0);
@!@:assign_local_box}{\.{\\localleftbox} primitive@>
primitive("localrightbox",assign_local_box,1);
@!@:assign_local_box}{\.{\\localrightbox} primitive@>

@ @<Cases of |print_cmd_chr|...@>=
discretionary: if chr_code=1 then
  print_esc("-")@+else print_esc("discretionary");
assign_local_box: if chr_code=0 then print_esc("localleftbox")
                  else print_esc("localrightbox");

@ @<Cases of |main_control| that build...@>=
hmode+discretionary,mmode+discretionary: append_discretionary;
any_mode(assign_local_box): append_local_box(cur_chr);
@z
%-------------------------
@x [47] m.1117 l.21338
procedure append_discretionary;
@y
procedure append_local_box(kind:integer);
begin 
incr(save_ptr); saved(-1):=kind; new_save_level(local_box_group);
scan_left_brace; push_nest; mode:=-hmode; space_factor:=1000;
end;

procedure append_discretionary;
@z
%-------------------------
@x [47] m.1118 l.21355
disc_group: build_discretionary;
@y
disc_group: build_discretionary;
local_box_group: build_local_box;
@z
%-------------------------
@x [47] m.1119 l.21358
procedure build_discretionary;
@y
procedure build_local_box;
var p:pointer;
    kind:integer;
begin
unsave;
kind:=saved(-1);
decr(save_ptr);
p:=link(head); pop_nest;
p:=hpack(p,natural);
if kind=0 then
  eq_define(local_left_box_base,box_ref,p)
else
  eq_define(local_right_box_base,box_ref,p);
if abs(mode)=hmode then
  @<LOCAL: Add local paragraph node@>;
eq_word_define(int_base+no_local_whatsits_code,no_local_whatsits+1);
end;

procedure build_discretionary;
@z
%-------------------------
@x [47] m.1200 l.22453
procedure resume_after_display;
begin if cur_group<>math_shift_group then confusion("display");
@:this can't happen display}{\quad display@>
unsave; prev_graf:=prev_graf+3;
push_nest; mode:=hmode; space_factor:=1000; set_cur_lang; clang:=cur_lang;
prev_graf:=(norm_min(left_hyphen_min)*@'100+norm_min(right_hyphen_min))
             *@'200000+cur_lang;
@y
procedure resume_after_display;
begin if cur_group<>math_shift_group then confusion("display");
@:this can't happen display}{\quad display@>
unsave; prev_graf:=prev_graf+3;
push_nest; mode:=hmode; space_factor:=1000; set_cur_lang; clang:=cur_lang;
prev_graf:=(norm_min(left_hyphen_min)*@'100+norm_min(right_hyphen_min))
             *@'200000+cur_lang;
@<LOCAL: Add local paragraph node@>;
@z
%-------------------------
@x [49] m.1228 l.22908
assign_int: begin p:=cur_chr; scan_optional_equals; scan_int;
  word_define(p,cur_val);
@y
assign_int: begin p:=cur_chr; scan_optional_equals; scan_int;
  word_define(p,cur_val);
{If we are defining subparagraph penalty levels while we are
in hmode, then we put out a whatsit immediately, otherwise
we leave it alone.  This mechanism might not be sufficiently
powerful, and some other algorithm, searching down the stack,
might be necessary.  Good first step.}
  if (abs(mode)=hmode) and
     ((p=(int_base+local_inter_line_penalty_code)) or
      (p=(int_base+local_broken_penalty_code))) then begin
    @<LOCAL: Add local paragraph node@>;
    eq_word_define(int_base+no_local_whatsits_code,no_local_whatsits+1);
    local_par_bool:=true;
    end;
@z
%-------------------------
@x [53] m.1344 l.24533
@d set_language_code=5 {command modifier for \.{\\setlanguage}}
@y
@d set_language_code=5 {command modifier for \.{\\setlanguage}}
@d local_par_node=6 {|subtype| in whatsits for local paragraph node}
@z
%-------------------------
@x [53] m.1344 l.24546
primitive("setlanguage",extension,set_language_code);@/
@!@:set_language_}{\.{\\setlanguage} primitive@>
@y
primitive("setlanguage",extension,set_language_code);@/
@!@:set_language_}{\.{\\setlanguage} primitive@>
primitive("localinterlinepenalty",assign_int,local_inter_line_penalty_code);@/
primitive("localbrokenpenalty",assign_int,local_broken_penalty_code);@/
@z
%-------------------------
@x [53] m.1356 l.24660
language_node:begin print_esc("setlanguage");
  print_int(what_lang(p)); print(" (hyphenmin ");
  print_int(what_lhm(p)); print_char(",");
  print_int(what_rhm(p)); print_char(")");
  end;
@y4
language_node:begin print_esc("setlanguage");
  print_int(what_lang(p)); print(" (hyphenmin ");
  print_int(what_lhm(p)); print_char(",");
  print_int(what_rhm(p)); print_char(")");
  end;
@<LOCAL: print out |local_paragraph| node@>;
@z
%-------------------------
@x [53] m.1357 l.24685
othercases confusion("ext2")
@y
local_par_node: begin r:=get_node(local_par_size);
  words:=local_par_size;
  end;
othercases confusion("ext2")
@z
%-------------------------
@x [53] m.1358 l.24696
othercases confusion("ext3")
@y
local_par_node: free_node(p,local_par_size);
othercases confusion("ext3")
@z
%-------------------------
@x [53] m.1362 l.24711
@<Advance \(p)past a whatsit node in the \(l)|line_break| loop@>=@+
adv_past(cur_p)
@y
@<Advance \(p)past a whatsit node in the \(l)|line_break| loop@>=@+
adv_past(cur_p) else @<LOCAL: Advance past a |local_paragraph| node@>
@z
%-------------------------
@x [53] m.1373 l.24820
procedure out_what(@!p:pointer);
var j:small_number; {write stream number}
begin case subtype(p) of
open_node,write_node,close_node:@<Do some work that has been queued up
  for \.{\\write}@>;
special_node:special_out(p);
language_node:do_nothing;
othercases confusion("ext4")
@y
procedure out_what(@!p:pointer);
var j:small_number; {write stream number}
begin case subtype(p) of
open_node,write_node,close_node:@<Do some work that has been queued up
  for \.{\\write}@>;
special_node:special_out(p);
language_node:do_nothing;
local_par_node:do_nothing;
othercases confusion("ext4")
@z
%-------------------------
@x [53] m.1378 l.24900
@ @<Finish the extensions@>=
for k:=0 to 15 do if write_open[k] then a_close(write_file[k])
@y
@ @<Finish the extensions@>=
for k:=0 to 15 do if write_open[k] then a_close(write_file[k])

@ 
@d local_pen_inter(#)==mem[#+1].int {\.{\\localinterlinepenalty}}
@d local_pen_broken(#)==mem[#+2].int {\.{\\localbrokenpenalty}}
@d local_box_left(#)==mem[#+3].int {\.{\\localleftbox}}
@d local_box_left_width(#)==mem[#+4].int
@d local_box_right(#)==mem[#+5].int {\.{\\localleftbox}}
@d local_box_right_width(#)==mem[#+6].int
@d local_par_size==7

@<LOCAL: Declare |make_local_par_node|@>=
function make_local_par_node:pointer;
{This function creates a |local_paragraph| node}
var p:pointer;
begin
p:=get_node(local_par_size); type(p):=whatsit_node;
subtype(p):=local_par_node; link(p):=null;
local_pen_inter(p):=local_inter_line_penalty;
local_pen_broken(p):=local_broken_penalty;
if local_left_box=null then begin
  local_box_left(p):=null;
  local_box_left_width(p):=0;
  end
else begin
  local_box_left(p):=copy_node_list(local_left_box);
  local_box_left_width(p):=width(local_left_box);
  end;
if local_right_box=null then begin
  local_box_right(p):=null;
  local_box_right_width(p):=0;
  end
else begin
  local_box_right(p):=copy_node_list(local_right_box);
  local_box_right_width(p):=width(local_right_box);
  end;
make_local_par_node:=p;
end

@ @<LOCAL: Initialize with first |local_paragraph| node@>=
if subtype(cur_p)=local_par_node then begin
  internal_pen_inter:=local_pen_inter(cur_p);
  internal_pen_broken:=local_pen_broken(cur_p);
  init_internal_left_box:=local_box_left(cur_p);
  init_internal_left_box_width:=local_box_left_width(cur_p);
  internal_left_box:=init_internal_left_box;
  internal_left_box_width:=init_internal_left_box_width;
  internal_right_box:=local_box_right(cur_p);
  internal_right_box_width:=local_box_right_width(cur_p);
  end

@ @<LOCAL: Advance past a |local_paragraph| node@>=
if subtype(cur_p)=local_par_node then begin
  internal_pen_inter:=local_pen_inter(cur_p);
  internal_pen_broken:=local_pen_broken(cur_p);
  internal_left_box:=local_box_left(cur_p);
  internal_left_box_width:=local_box_left_width(cur_p);
  internal_right_box:=local_box_right(cur_p);
  internal_right_box_width:=local_box_right_width(cur_p);
  end

@ @<LOCAL: print out |local_paragraph| node@>=
local_par_node: begin
  print_esc("whatsit");
  append_char(".");
  print_ln; print_current_string;
  print_esc("localinterlinepenalty"); print("=");
  print_int(local_pen_inter(p));
  print_ln; print_current_string;
  print_esc("localbrokenpenalty"); print("=");
  print_int(local_pen_broken(p));
  print_ln; print_current_string;
  print_esc("localleftbox");
  if local_box_left(p)=null then print("=null")
  else begin
    append_char(".");
    show_node_list(local_box_left(p));
    decr(pool_ptr);
    end;
  print_ln; print_current_string;
  print_esc("localrightbox");
  if local_box_right(p)=null then print("=null")
  else begin
    append_char(".");
    show_node_list(local_box_right(p));
    decr(pool_ptr);
    end;
  decr(pool_ptr);
  end

@ @<LOCAL: Add local paragraph node@>=
begin tail_append(make_local_par_node) end

@ @<LOCAL: local paragraph node@>=
((type(q)=whatsit_node) and (subtype(q)=local_par_node))

@z
%-------------------------
