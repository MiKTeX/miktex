% omfilter.ch: Running OCP programs
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
@x [1] m.11 l.412 - Omega Filtering
  {string of length |file_name_size|; tells where the string pool appears}
@.TeXformats@>
@y
  {string of length |file_name_size|; tells where the string pool appears}
@.TeXformats@>
@!active_mem_size=2000; {number of words of |active_info| for active ocps}
@z
%---------------------------------------
@x [5] - Omega Translation
procedure overflow(@!s:str_number;@!n:integer); {stop due to finiteness}
begin normalize_selector;
print_err("Omega capacity exceeded, sorry [");
@.TeX capacity exceeded ...@>
print(s); print_char("="); print_int(n); print_char("]");
help2("If you really absolutely need more capacity,")@/
  ("you can ask a wizard to enlarge me.");
succumb;
end;
@y
procedure overflow(@!s:str_number;@!n:integer); {stop due to finiteness}
begin normalize_selector;
print_err("Omega capacity exceeded, sorry [");
@.TeX capacity exceeded ...@>
print(s); print_char("="); print_int(n); print_char("]");
help2("If you really absolutely need more capacity,")@/
  ("you can ask a wizard to enlarge me.");
succumb;
end;

procedure overflow_ocp_buf_size;
begin
overflow("ocp_buf_size",ocp_buf_size);
end;

procedure overflow_ocp_stack_size;
begin
overflow("ocp_stack_size",ocp_stack_size);
end;
@z
%---------------------------------------
@x [22] m.300 l.6425 - Omega Filtering
@!in_state_record = record
  @!state_field, @!index_field: quarterword;
  @!start_field,@!loc_field, @!limit_field, @!name_field: halfword;
  end;
@y
@!in_state_record = record
  @!state_field, @!index_field: quarterword;
  @!start_field,@!loc_field, @!limit_field, @!name_field,
  @!ocp_lstack_field: halfword; {used for omega translation processes}
  @!ocp_no_field: halfword; {used for omega translation processes}
  end;
@z
%---------------------------------------
@x [22] m.300 l.6444 - Omega Filtering
@d limit==cur_input.limit_field {end of current line in |buffer|}
@d name==cur_input.name_field {name of the current file}
@y
@d limit==cur_input.limit_field {end of current line in |buffer|}
@d name==cur_input.name_field {name of the current file}
@d current_ocp_lstack==cur_input.ocp_lstack_field {name of the current ocp}
@d current_ocp_no==cur_input.ocp_no_field {name of the current ocp}
@z
%---------------------------------------
@x [22] m.312 l.6766 - Omega Filtering
  if state<>token_list then
    begin @<Print location of current line@>;
    @<Pseudoprint the line@>;
@y
  if current_ocp_lstack>0 then
  begin print_nl("OCP stack "); print_scaled(current_ocp_lstack); 
        print(" entry "); print_int(current_ocp_no); print(":");
    @<Pseudoprint the line@>;
  end
  else if state<>token_list then
    begin @<Print location of current line@>;
    @<Pseudoprint the line@>;
@z
%---------------------------------------
@x [23] m.331 l.7069 - Omega Filtering
state:=new_line; start:=1; index:=0; line:=0; name:=0;
@y
state:=new_line; start:=1; index:=0; line:=0; name:=0;
current_ocp_lstack:=0; current_ocp_no:=0;
@z
%---------------------------------------
@x [24] m.343 l.7248 - Omega Filtering
else  begin state:=new_line;@/
  @<Move to next line of file,
@y
else  begin
  if current_ocp_lstack>0 then
  begin
     pop_input; goto restart;
  end;
  state:=new_line;
  @<Move to next line of file,
@z
%---------------------------------------
@x [30] m.582 l.11283 - Omega Filtering
char_warning(f,c);
new_character:=null;
exit:end;
@y
char_warning(f,c);
new_character:=null;
exit:end;

@
@<Types...@>=
@!active_index=0..active_mem_size;

@
@d active_ocp(#)==active_info[#].hh.b0
@d active_counter(#)==active_info[#].hh.b1
@d active_lstack_no(#)==active_info[#+1].sc

@<Glob...@>=
@!active_info:array[active_index] of memory_word;
@!active_min_ptr:active_index; {first unused word of |active_info|}
@!active_max_ptr:active_index; {last unused word of |active_info|}
@!active_real:active_index;
@!holding:array[active_index] of ocp_list_index;

@
@<Initialize table...@>=
active_min_ptr:=0;
active_real:=0;

@
@p
function is_last_ocp(llstack_no:scaled; counter:integer):integer;
begin
active_min_ptr:=equiv(ocp_active_min_ptr_base);
active_max_ptr:=equiv(ocp_active_max_ptr_base);
active_real:=active_min_ptr;
while (active_real < active_max_ptr) and
      (active_lstack_no(active_real) < llstack_no) do begin
  active_real:=active_real+2;
  end;
while (active_real < active_max_ptr) and
      (active_lstack_no(active_real) = llstack_no) and
      (active_counter(active_real) <= counter) do begin
  active_real:=active_real+2;
  end;
is_last_ocp := (active_real=active_max_ptr);
end;

procedure print_active_ocps;
var i:integer;
begin
print_nl("Active ocps: [");
i:=active_min_ptr;
while i<active_max_ptr do begin
  print("(");
  print_scaled(active_lstack_no(i));
  print(",");
  print_int(active_counter(i));
  print(",");
  print_esc(ocp_id_text(active_ocp(i)));
  print(")");
  if i<>(active_max_ptr-2) then print(",");
  i:=i+2;
  end;
print("]");
end;

procedure add_ocp_stack(min_index:integer; min_value:scaled);
var p:ocp_lstack_index;
    llstack_no:scaled;
    counter:integer;
begin
p:=ocp_list_lstack(holding[min_index]);
llstack_no:=ocp_list_lstack_no(holding[min_index]);
counter:=0;
while not (is_null_ocp_lstack(p)) do begin
  active_ocp(active_max_ptr):=ocp_lstack_ocp(p);
  active_counter(active_max_ptr):=counter;
  active_lstack_no(active_max_ptr):=llstack_no;
  p:=ocp_lstack_lnext(p);
  active_max_ptr:=active_max_ptr+2;
  incr(counter);  {no overflow problem}
  end;
end;

procedure active_compile;
var i:integer;
    min_index:integer;
    min_stack_ocp:scaled;
    old_min:scaled;
    max_active:integer;
    stack_value:scaled;
begin
active_min_ptr:=active_max_ptr;
min_stack_ocp:=ocp_maxint;
max_active:=equiv(ocp_active_number_base)-1;
for i:=max_active downto 0 do begin
  holding[i]:=ocp_list_list[equiv(ocp_active_base+i)];
  stack_value:=ocp_list_lstack_no(holding[i]);
  if stack_value<min_stack_ocp then begin
    min_index:=i;
    min_stack_ocp:=stack_value;
    end;
  end;
while min_stack_ocp<ocp_maxint do begin
  add_ocp_stack(min_index, min_stack_ocp);
  old_min:=min_stack_ocp;
  min_stack_ocp:=ocp_maxint;
  for i:=max_active downto 0 do begin
    stack_value:=ocp_list_lstack_no(holding[i]);
    while old_min=stack_value do begin
      holding[i]:=ocp_list_lnext(holding[i]);
      stack_value:=ocp_list_lstack_no(holding[i]);
      end;
    if stack_value<min_stack_ocp then begin
      min_index:=i;
      min_stack_ocp:=stack_value;
      end;
    end;
  end;
end;

@z
%---------------------------------------
@x [46] m.1030 l.19984 - Omega Filtering
main_loop:@<Append character |cur_chr| and the following characters (if~any)
  to the current hlist in the current font; |goto reswitch| when
  a non-character has been fetched@>;
@y
main_loop:
if is_last_ocp(current_ocp_lstack,current_ocp_no) then begin
  @<Append character |cur_chr| and the following characters (if~any)
    to the current hlist in the current font; |goto reswitch| when
    a non-character has been fetched@>;
  end
else begin
  @<Create a buffer with character |cur_chr| and the following
    characters (if~any) and then apply the current active OCP filter
    to this buffer@>;
  end;
@z
%---------------------------------------
@x [50] m.1302 l.23862 - Omega Filtering
@<Dump the font information@>;
@y
@<Dump the font information@>;
@<Dump the active ocp information@>;
@z
%---------------------------------------
@x [50] m.1303 l.23711 - Omega Filtering
@<Undump the font information@>;
@y
@<Undump the font information@>;
@<Undump the active ocp information@>;
@z
%---------------------------------------
@x [50] m.1323 l.24907 - Omega Filtering
begin undump_font_table(k);@/
end
@y
begin undump_font_table(k);@/
end

@ @<Dump the active ocp information@>=
dump_int(active_min_ptr);
dump_int(active_max_ptr);
for k:=0 to active_max_ptr-1 do dump_wd(active_info[k]);
print_ln; print_int(active_max_ptr); print(" words of active ocps");

@ @<Undump the active ocp information@>=
undump_size(0)(active_mem_size)('active start point')(active_min_ptr);
undump_size(0)(active_mem_size)('active mem size')(active_max_ptr);
for k:=0 to active_max_ptr-1 do undump_wd(active_info[k]);

@z
%---------------------------------------
@x [54] m.1379 l.24883 - Omega Filtering
@ Here we do the main work required for reading and interpreting
  $\Omega$ Compiled Translation Processes.
@y
@ Here we do the main work required for reading and interpreting
  $\Omega$ Compiled Translation Processes.

@
@<Types...@>=

@ Here are all the instructions in our mini-assembler.
@d otp_right_output=1
@d otp_right_num=2
@d otp_right_char=3
@d otp_right_lchar=4
@d otp_right_some=5

@d otp_pback_output=6
@d otp_pback_num=7
@d otp_pback_char=8
@d otp_pback_lchar=9
@d otp_pback_some=10

@d otp_add=11
@d otp_sub=12
@d otp_mult=13
@d otp_div=14
@d otp_mod=15
@d otp_lookup=16
@d otp_push_num=17
@d otp_push_char=18
@d otp_push_lchar=19

@d otp_state_change=20
@d otp_state_push=21
@d otp_state_pop=22

@d otp_left_start=23
@d otp_left_return=24
@d otp_left_backup=25

@d otp_goto=26
@d otp_goto_ne=27
@d otp_goto_eq=28
@d otp_goto_lt=29
@d otp_goto_le=30
@d otp_goto_gt=31
@d otp_goto_ge=32
@d otp_goto_no_advance=33
@d otp_goto_beg=34
@d otp_goto_end=35

@d otp_stop=36

@
@<Glob...@>=
@!otp_init_input_start:halfword;
@!otp_init_input_last:halfword;
@!otp_init_input_end:halfword;
@!otp_i:halfword;
@!otp_init_input_buf:array[0..20000] of quarterword;

@!otp_input_start:halfword;
@!otp_input_last:halfword;
@!otp_input_end:halfword;
@!otp_input_buf:array[0..20000] of quarterword;

@!otp_output_end:halfword;
@!otp_output_buf:array[0..20000] of quarterword;

@!otp_stack_used:halfword;
@!otp_stack_last:halfword;
@!otp_stack_new:halfword;
@!otp_stack_buf:array[0..1000] of quarterword;

@!otp_pc:halfword;

@!otp_calc_ptr:halfword;
@!otp_calcs:array[0..1000] of halfword;
@!otp_state_ptr:halfword;
@!otp_states:array[0..1000] of halfword;

@!otp_input_char:halfword;
@!otp_calculated_char:halfword;
@!otp_no_input_chars:halfword;

@!otp_instruction:halfword;
@!otp_instr:halfword;
@!otp_arg:halfword;
@!otp_first_arg:halfword;
@!otp_second_arg:halfword;

@!otp_input_ocp:halfword;
@!otp_counter:halfword;

@!otp_finished:boolean;
@!otp_ext_str:integer;
@!otp_ext_str_arg:integer;
@!otp_ext_i:integer;

@
@<Initialize table...@>=

@
@<Create a buffer with character |cur_chr| and the following
  characters (if~any) and then apply the current active OCP filter
  to this buffer@>=
begin
@<Read in the buffer for treatment@>;
@<Apply the OCP to the input buffer@>;
@<Place the output buffer back on the stream@>;
goto big_switch;
end;

@
@<Read in the buffer for treatment@>=
begin
otp_init_input_end:=0;
while (cur_cmd=letter) or (cur_cmd=other_char) or
      (cur_cmd=char_given) or (cur_cmd=spacer) do begin
  incr(otp_init_input_end);
  if otp_init_input_end >ocp_buf_size then
    overflow_ocp_buf_size;
  otp_init_input_buf[otp_init_input_end]:=cur_chr;
  get_token;
  if (cur_cmd<>letter) and (cur_cmd<>other_char) and
     (cur_cmd<>char_given) then begin
    x_token;
    if (cur_cmd=char_num) then begin
      scan_char_num; cur_cmd:=other_char; cur_chr:=cur_val;
      end;
    end;
  end;
back_input;
otp_input_end:=otp_init_input_end;
for otp_i:=0 to otp_init_input_end do begin
  otp_input_buf[otp_i]:=otp_init_input_buf[otp_i];
  end;
end

@
@<Apply the OCP to the input buffer@>=
begin
otp_input_ocp:=active_ocp(active_real);
if otp_input_ocp=0 then begin
  print_nl("Null ocp being used: all input lost");
  otp_output_end:=0;
  end
else if ocp_external(otp_input_ocp)=0 then begin
  otp_input_start:=0;
  otp_input_last:=0;
  otp_stack_used:=0;
  otp_stack_last:=0;
  otp_stack_new:=0;
  otp_output_end:=0;
  otp_pc:=0;
  otp_finished:=false;
  otp_calc_ptr:=0;
  otp_calcs[otp_calc_ptr]:=0;
  otp_state_ptr:=0;
  otp_states[otp_state_ptr]:=0;
  while not otp_finished do
    @<Run the OTP program@>;
  end
else begin
  k:=0;
  if name_of_file then libc_free (name_of_file);
  otp_ext_str:=ocp_external(otp_input_ocp);
  otp_ext_str_arg:=ocp_external_arg(otp_input_ocp);
  name_of_file := xmalloc (4 + length(otp_ext_str) + length(otp_ext_str_arg));
  for otp_ext_i:=str_start(otp_ext_str) to
         str_start(otp_ext_str)+length(otp_ext_str)-1 do
    append_to_name(str_pool[otp_ext_i]);
  append_to_name(" ");
  for otp_ext_i:=str_start(otp_ext_str_arg) to
         str_start(otp_ext_str_arg)+length(otp_ext_str_arg)-1 do
    append_to_name(str_pool[otp_ext_i]);
  name_of_file[length(otp_ext_str)+length(otp_ext_str_arg)+2]:=0;
  run_external_ocp(name_of_file);
  end
end

@
@<Place the output buffer back on the stream@>=
begin
if (first+otp_output_end)>=ocp_buf_size then
  overflow_ocp_buf_size;
push_input; 
current_ocp_lstack:=active_lstack_no(active_real);
current_ocp_no:=active_counter(active_real);
state:=mid_line;
start:=first;
last:=start;
loc:=start;
for otp_counter:=1 to otp_output_end do begin
  buffer[last]:=otp_output_buf[otp_counter];
  incr(last);  {no overflow problem}
  end;
limit:=last-1;
first:=last;
end

@
@d otp_cur_state==otp_states[otp_state_ptr]
@d otp_set_instruction==begin
if otp_pc>=ocp_state_no(otp_input_ocp)(otp_cur_state) then begin
  print_err("bad OCP program -- PC not valid");
  succumb;
  end;
otp_instruction:=ocp_state_entry(otp_input_ocp)(otp_cur_state)(otp_pc);
otp_instr:=otp_instruction div @"1000000;
otp_arg:=otp_instruction mod @"1000000;
end

@<Run the OTP program@>=
begin
otp_set_instruction;
case otp_instr of
  @<Run the |otp_right| instructions@>;
  @<Run the |otp_pback| instructions@>;
  @<Run the arithmetic instructions@>;
  @<Run the |otp_state| instructions@>;
  @<Run the |otp_left| instructions@>;
  @<Run the |otp_goto| instructions@>;
  @<Run the |otp_stop| instruction@>;
  othercases begin
    print_err("bad OCP program -- unknown instruction");
    succumb;
    end
  endcases;
end

@

@d otp_check_char(#)==
if (1>#) or (#>otp_no_input_chars) then begin
  print_err("right hand side of OCP expression is bad");
  succumb;
  end

@d otp_get_char(#)==begin
otp_check_char(#);
if #>otp_stack_last then begin
  otp_calculated_char:=otp_input_buf[otp_input_start+#-otp_stack_last];
  end
else begin
  otp_calculated_char:=otp_stack_buf[#];
  end;
end

@d decr(#)==#:=#-1

@<Run the |otp_right| instructions@>=
  otp_right_output: begin
    incr(otp_output_end);
    if otp_output_end >ocp_buf_size then
      overflow_ocp_buf_size;
    otp_output_buf[otp_output_end]:=otp_calcs[otp_calc_ptr];
    decr(otp_calc_ptr);
    incr(otp_pc);
    end;
  otp_right_num: begin
    incr(otp_output_end);
    if otp_output_end >ocp_buf_size then
      overflow_ocp_buf_size;
    otp_output_buf[otp_output_end]:=otp_arg;
    incr(otp_pc);
    end;
  otp_right_char: begin
    otp_get_char(otp_arg);
    incr(otp_output_end);
    if otp_output_end >ocp_buf_size then
      overflow_ocp_buf_size;
    otp_output_buf[otp_output_end]:=otp_calculated_char;
    incr(otp_pc);
    end;
  otp_right_lchar: begin
    otp_get_char(otp_no_input_chars-otp_arg);
    incr(otp_output_end);
    if otp_output_end >ocp_buf_size then
      overflow_ocp_buf_size;
    otp_output_buf[otp_output_end]:=otp_calculated_char;
    incr(otp_pc);
    end;
  otp_right_some: begin
    otp_first_arg:=otp_arg+1;
    incr(otp_pc);
    otp_set_instruction;
    otp_second_arg:=otp_no_input_chars-otp_arg;
    for otp_counter:=otp_first_arg to otp_second_arg do begin
      otp_get_char(otp_counter);
      incr(otp_output_end);
      if otp_output_end >ocp_buf_size then
        overflow_ocp_buf_size;
      otp_output_buf[otp_output_end]:=otp_calculated_char;
      end;
    incr(otp_pc);
    end

@
@<Run the |otp_pback| instructions@>=
  otp_pback_output: begin
    incr(otp_stack_new);
    if otp_stack_new >= ocp_stack_size then
      overflow_ocp_stack_size;
    otp_stack_buf[otp_stack_new]:=otp_calcs[otp_calc_ptr];
    decr(otp_calc_ptr);
    incr(otp_pc);
    end;
  otp_pback_num: begin
    incr(otp_stack_new);
    if otp_stack_new >= ocp_stack_size then
      overflow_ocp_stack_size;
    otp_stack_buf[otp_stack_new]:=otp_arg;
    incr(otp_pc);
    end;
  otp_pback_char: begin
    otp_get_char(otp_arg);
    incr(otp_stack_new);
    if otp_stack_new >= ocp_stack_size then
      overflow_ocp_stack_size;
    otp_stack_buf[otp_stack_new]:=otp_calculated_char;
    incr(otp_pc);
    end;
  otp_pback_lchar: begin
    otp_get_char(otp_no_input_chars-otp_arg);
    incr(otp_stack_new);
    if otp_stack_new >= ocp_stack_size then
      overflow_ocp_stack_size;
    otp_stack_buf[otp_stack_new]:=otp_calculated_char;
    incr(otp_pc);
    end;
  otp_pback_some: begin
    otp_first_arg:=otp_arg+1;
    incr(otp_pc);
    otp_set_instruction;
    otp_second_arg:=otp_no_input_chars-otp_arg;
    for otp_counter:=otp_first_arg to otp_second_arg do begin
      otp_get_char(otp_counter);
      incr(otp_stack_new);
      if otp_stack_new >= ocp_stack_size then
        overflow_ocp_stack_size;
      otp_stack_buf[otp_stack_new]:=otp_calculated_char;
      end;
    incr(otp_pc);
    end
   
@
@<Run the arithmetic instructions@>=
  otp_add: begin
    otp_calcs[otp_calc_ptr-1] :=
      otp_calcs[otp_calc_ptr-1] + otp_calcs[otp_calc_ptr];
    incr(otp_pc);
    decr(otp_calc_ptr);
    end;
  otp_sub: begin
    otp_calcs[otp_calc_ptr-1] :=
      otp_calcs[otp_calc_ptr-1] - otp_calcs[otp_calc_ptr];
    incr(otp_pc);
    decr(otp_calc_ptr);
    end;
  otp_mult: begin
    otp_calcs[otp_calc_ptr-1] :=
      otp_calcs[otp_calc_ptr-1] * otp_calcs[otp_calc_ptr];
    incr(otp_pc);
    decr(otp_calc_ptr);
    end;
  otp_div: begin
    otp_calcs[otp_calc_ptr-1] :=
      otp_calcs[otp_calc_ptr-1] div otp_calcs[otp_calc_ptr];
    incr(otp_pc);
    decr(otp_calc_ptr);
    end;
  otp_mod: begin
    otp_calcs[otp_calc_ptr-1] :=
      otp_calcs[otp_calc_ptr-1] mod otp_calcs[otp_calc_ptr];
    incr(otp_pc);
    decr(otp_calc_ptr);
    end;
  otp_lookup: begin
    if otp_calcs[otp_calc_ptr]>=
       ocp_table_no(otp_input_ocp)(otp_calcs[otp_calc_ptr-1]) then begin
      print_err("bad OCP program -- table index not valid");
      succumb;
      end;
    otp_calcs[otp_calc_ptr-1]:=
    ocp_table_entry(otp_input_ocp)(otp_calcs[otp_calc_ptr-1])
                   (otp_calcs[otp_calc_ptr]);
    incr(otp_pc);
    decr(otp_calc_ptr);
    end;
  otp_push_num: begin
    incr(otp_calc_ptr);
    if otp_calc_ptr >= ocp_stack_size then
      overflow_ocp_stack_size;
    otp_calcs[otp_calc_ptr]:=otp_arg;
    incr(otp_pc);
    end;
  otp_push_char: begin
    otp_get_char(otp_arg);
    incr(otp_calc_ptr);
    if otp_calc_ptr >= ocp_stack_size then
      overflow_ocp_stack_size;
    otp_calcs[otp_calc_ptr]:=otp_calculated_char;
    incr(otp_pc);
    end;
  otp_push_lchar: begin
    otp_get_char(otp_no_input_chars-otp_arg);
    incr(otp_calc_ptr);
    if otp_calc_ptr >= ocp_stack_size then
      overflow_ocp_stack_size;
    otp_calcs[otp_calc_ptr]:=otp_calculated_char;
    incr(otp_pc);
    end
   
@
@<Run the |otp_state| instructions@>=
  otp_state_change: begin
    otp_input_start:=otp_input_last;
    for otp_counter:=1 to (otp_stack_new-otp_stack_used) do begin
      otp_stack_buf[otp_counter] := 
        otp_stack_buf[otp_counter+otp_stack_used];
      end;
    otp_stack_new:=otp_stack_new-otp_stack_used;
    otp_stack_last:=otp_stack_new;
    otp_stack_used:=0;
    otp_states[otp_state_ptr]:=otp_arg;
    otp_pc:=0;
    end;
  otp_state_push: begin
    otp_input_start:=otp_input_last;
    for otp_counter:=1 to (otp_stack_new-otp_stack_used) do begin
      otp_stack_buf[otp_counter] := 
        otp_stack_buf[otp_counter+otp_stack_used];
      end;
    otp_stack_new:=otp_stack_new-otp_stack_used;
    otp_stack_last:=otp_stack_new;
    otp_stack_used:=0;
    incr(otp_state_ptr);
    if otp_state_ptr >= ocp_stack_size then
      overflow_ocp_stack_size;
    otp_states[otp_state_ptr]:=otp_arg;
    otp_pc:=0;
    end;
  otp_state_pop: begin
    otp_input_start:=otp_input_last;
    for otp_counter:=1 to (otp_stack_new-otp_stack_used) do begin
      otp_stack_buf[otp_counter] := 
        otp_stack_buf[otp_counter+otp_stack_used];
      end;
    otp_stack_new:=otp_stack_new-otp_stack_used;
    otp_stack_last:=otp_stack_new;
    otp_stack_used:=0;
    if otp_state_ptr>0 then decr(otp_state_ptr);
    otp_pc:=0;
    end
   
@
@<Run the |otp_left| instructions@>=
  otp_left_start: begin
    otp_input_start:=otp_input_last;
    otp_input_last:=otp_input_start;
    otp_stack_used:=0;
    if (otp_stack_last=0) and (otp_input_last>=otp_input_end) then
      otp_finished:=true
    else if (otp_stack_used < otp_stack_last) then begin
      incr(otp_stack_used); {no overflow problem}
      otp_input_char:=otp_stack_buf[otp_stack_used];
      otp_no_input_chars:=1;
      incr(otp_pc);
      end
    else begin
      incr(otp_input_last); {no overflow problem}
      otp_input_char:=otp_input_buf[otp_input_last];
      otp_no_input_chars:=1;
      incr(otp_pc);
      end;
    end;
  otp_left_return: begin
    otp_input_last:=otp_input_start;
    otp_stack_used:=0;
    if (otp_stack_used < otp_stack_last) then begin
      incr(otp_stack_used); {no overflow problem}
      otp_input_char:=otp_stack_buf[otp_stack_used];
      otp_no_input_chars:=1;
      incr(otp_pc);
      end
    else begin
      incr(otp_input_last); {no overflow problem}
      otp_input_char:=otp_input_buf[otp_input_last];
      otp_no_input_chars:=1;
      incr(otp_pc);
      end;
    end;
  otp_left_backup: begin
    if otp_input_start < otp_input_last then begin
      decr(otp_input_last); otp_input_char:=otp_input_buf[otp_input_last];
      end
    else begin
      decr(otp_stack_used); otp_input_char:=otp_stack_buf[otp_stack_used];
      end;
    decr(otp_no_input_chars);
    incr(otp_pc);
    end
   
@
@<Run the |otp_goto| instructions@>=
  otp_goto: begin
    otp_pc:=otp_arg;
    end;
  otp_goto_ne: begin
    otp_first_arg:=otp_arg;
    incr(otp_pc);
    otp_set_instruction;
    if otp_input_char <> otp_first_arg then begin
      otp_pc:=otp_arg;
      end
    else begin
      incr(otp_pc);
      end;
    end;
  otp_goto_eq: begin
    otp_first_arg:=otp_arg;
    incr(otp_pc);
    otp_set_instruction;
    if otp_input_char = otp_first_arg then begin
      otp_pc:=otp_arg;
      end
    else begin
      incr(otp_pc);
      end;
    end;
  otp_goto_lt: begin
    otp_first_arg:=otp_arg;
    incr(otp_pc);
    otp_set_instruction;
    if otp_input_char < otp_first_arg then begin
      otp_pc:=otp_arg;
      end
    else begin
      incr(otp_pc);
      end;
   end;
  otp_goto_le: begin
    otp_first_arg:=otp_arg;
    incr(otp_pc);
    otp_set_instruction;
    if otp_input_char <= otp_first_arg then begin
      otp_pc:=otp_arg;
      end
    else begin
      incr(otp_pc);
      end;
    end;
  otp_goto_gt: begin
    otp_first_arg:=otp_arg;
    incr(otp_pc);
    otp_set_instruction;
    if otp_input_char > otp_first_arg then begin
      otp_pc:=otp_arg;
      end
    else begin
      incr(otp_pc);
      end;
    end;
  otp_goto_ge: begin
    otp_first_arg:=otp_arg;
    incr(otp_pc);
    otp_set_instruction;
    if otp_input_char >= otp_first_arg then begin
      otp_pc:=otp_arg;
      end
    else begin
      incr(otp_pc);
      end;
    end;
  otp_goto_no_advance: begin
    if (otp_stack_used < otp_stack_last) then begin
      incr(otp_stack_used); {no overflow problem}
      otp_input_char:=otp_stack_buf[otp_stack_used];
      incr(otp_no_input_chars); {no overflow problem}
      incr(otp_pc);
      end
    else if otp_input_last>=otp_input_end then begin
      otp_pc:=otp_arg;
      end
    else begin
      incr(otp_input_last); {no overflow problem}
      otp_input_char:=otp_input_buf[otp_input_last];
      incr(otp_no_input_chars); {no overflow problem}
      incr(otp_pc);
      end;
    end;
  otp_goto_beg: begin
    if otp_input_last=0 then begin
      otp_pc:=otp_arg;
      end
    else begin
      incr(otp_pc);
      end;
    end;
  otp_goto_end: begin
    if otp_input_last>=otp_input_end then begin
      otp_pc:=otp_arg;
      end
    else begin
      incr(otp_pc);
      end;
    end
   
@
@<Run the |otp_stop| instruction@>=
  otp_stop: begin
    otp_input_start:=otp_input_last;
    for otp_counter:=1 to (otp_stack_new-otp_stack_used) do begin
      otp_stack_buf[otp_counter] := 
        otp_stack_buf[otp_counter+otp_stack_used];
      end;
    otp_stack_new:=otp_stack_new-otp_stack_used;
    otp_stack_last:=otp_stack_new;
    otp_stack_used:=0;
    otp_pc:=0;
    end
   
@z
