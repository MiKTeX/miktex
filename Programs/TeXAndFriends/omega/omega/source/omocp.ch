% omocp.ch: Reading an OCP file
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
@x [1] m.11 l.412 - Omega OCP
  {string of length |file_name_size|; tells where the string pool appears}
@.TeXformats@>
@y
  {string of length |file_name_size|; tells where the string pool appears}
@.TeXformats@>
@!ocp_maxint=@"10000000;
@z
%---------------------------------------
@x [1] m.12 l.436 - Omega OCP
@d hyph_size=307 {another prime; the number of \.{\\hyphenation} exceptions}
@y
@d hyph_size=307 {another prime; the number of \.{\\hyphenation} exceptions}
@d ocp_base=0 {smallest internal ocp number; must not be less
  than |min_quarterword|}
@d ocp_biggest=65535 {the real biggest ocp}
@d number_ocps=ocp_biggest-ocp_base+1
@d ocp_list_base=0 {smallest internal ocp list number; must not be less
  than |min_quarterword|}
@d ocp_list_biggest=65535 {the real biggest ocp list}
@d number_ocp_lists=ocp_list_biggest-ocp_list_base+1
@d max_active_ocp_lists=65536
@z
%---------------------------------------
@x [15] m.209 l.4170 - Omega OCP
@d max_command=set_interaction
   {the largest command code seen at |big_switch|}
@y
@d set_ocp=set_interaction+1
   {Place a translation process in the stream}
@d def_ocp=set_ocp+1
   {Define and load a translation process}
@d set_ocp_list=def_ocp+1
   {Place a list of OCPs in the stream}
@d def_ocp_list=set_ocp_list+1
   {Define a list of OCPs}
@d clear_ocp_lists=def_ocp_list+1
   {Remove all active OCP lists}
@d push_ocp_list=clear_ocp_lists+1
   {Add to the sequence of active OCP lists}
@d pop_ocp_list=push_ocp_list+1
   {Remove from the sequence of active OCP lists}
@d ocp_list_op=pop_ocp_list+1
   {Operations for building a list of OCPs}
@d ocp_trace_level=ocp_list_op+1
   {Tracing of active OCPs, either 0 or 1}
@d max_command=ocp_trace_level
   {the largest command code seen at |big_switch|}
@z
%---------------------------------------
@x [17] m.222 l.4523 - Omega OCP
@d font_id_base=frozen_null_font-font_base
  {begins table of |number_fonts| permanent font identifiers}
@d undefined_control_sequence=frozen_null_font+number_fonts
@y
@d font_id_base=frozen_null_font-font_base
  {begins table of |number_fonts| permanent font identifiers}
@d frozen_null_ocp=frozen_null_font+number_fonts
  {permanent `\.{\\nullocp}'}
@d ocp_id_base=frozen_null_ocp-ocp_base
  {begins table of |number_ocps| permanent ocp identifiers}
@d frozen_null_ocp_list=frozen_null_ocp+number_ocps
  {permanent `\.{\\nullocplist}'}
@d ocp_list_id_base=frozen_null_ocp_list-ocp_list_base
  {begins table of |number_ocp_lists| permanent ocp list identifiers}
@d undefined_control_sequence=frozen_null_ocp_list+number_ocp_lists 
@z
%---------------------------------------
@x
@d toks_base=local_base+10 {table of |number_regs| token list registers}
@y
@d ocp_trace_level_base=local_base+10
@d ocp_active_number_base=ocp_trace_level_base+1
@d ocp_active_min_ptr_base = ocp_active_number_base+1
@d ocp_active_max_ptr_base = ocp_active_min_ptr_base+1
@d ocp_active_base = ocp_active_max_ptr_base+1
@d toks_base=ocp_active_base+max_active_ocp_lists 
   {table of |number_regs| token list registers}
@z
%---------------------------------------
@x [17] m.232 l.4799 - Omega OCP
@d null_font==font_base
@y
@d null_font==font_base
@d null_ocp==ocp_base
@d null_ocp_list==ocp_list_base
@z
%---------------------------------------
@x [18] m.256 l.5479 - Omega OCP
@d font_id_text(#) == newtext(font_id_base+#) {a frozen font identifier's name}
@y
@d font_id_text(#) == newtext(font_id_base+#) {a frozen font identifier's name}
@d ocp_id_text(#) == newtext(ocp_id_base+#) {a frozen ocp identifier's name}
@d ocp_list_id_text(#) == newtext(ocp_list_id_base+#) 
   {a frozen ocp list identifier's name}
@z
%---------------------------------------
@x [26] m.409 l.8254 - Omega OCP
@t\4\4@>@<Declare procedures that scan font-related stuff@>
@y
@t\4\4@>@<Declare procedures that scan font-related stuff@>
@t\4\4@>@<Declare procedures that scan ocp-related stuff@>
@z
%---------------------------------------
@x [29] m.514 l.9968 - Omega OCP
|TEX_font_area|.  These system area names will, of course, vary from place
to place.
@y
|TEX_font_area|.  $\Omega$'s compiled translation process files whose areas
are not given explicitly are assumed to appear in a standard system area
called |OMEGA_ocp_area|.  These system area names will, of course, vary
from place to place.
@z
%---------------------------------------
@x [29] m.514 l.9974 - Omega OCP
@d TEX_font_area=="TeXfonts:"
@.TeXfonts@>
@y
@d TEX_font_area=="TeXfonts:"
@.TeXfonts@>
@d OMEGA_ocp_area=="OmegaOCPs:"
@.OmegaOCPs@>
@z
%---------------------------------------
@x [30] m.582 l.10379 - Omega OCP
@* \[30] Font metric data.
@y
@* \[30] Font metric data and OCPs.
@z
%---------------------------------------
@x [30] m.??? l.10928 - Omega OCP
@ @<Read and check...@>=
@y
@ @<Read and check the font data...@>=
@z
%---------------------------------------
@x [30] m.??? l.11010 - Omega OCP
@ @<Apologize for not loading...@>=
@y
@ @<Apologize for not loading the font...@>=
@z
%---------------------------------------
@x [30] m.582 l.11283 - Omega OCP
char_warning(f,c);
new_character:=null;
exit:end;
@y
char_warning(f,c);
new_character:=null;
exit:end;

@ Here we begin the \.{OCP} file handling.

@<Glob...@>=
@!ocp_file:byte_file;

@ So that is what \.{OCP} files hold.

When the user defines \.{\\ocp\\f}, say, \TeX\ assigns an internal number
to the user's ocp~\.{\\f}. Adding this number to |ocp_id_base| gives the
|eqtb| location of a ``frozen'' control sequence that will always select
the ocp.

@<Types...@>=
@!internal_ocp_number=ocp_base..ocp_biggest;
@!ocp_index=integer;

@ Here now is the array of ocp arrays.

@<Glob...@>=
@!ocp_ptr:internal_ocp_number; {largest internal ocp number in use}

@ Besides the arrays just enumerated, we have two directory arrays that
make it easy to get at the individual entries in |ocp_info|.
The beginning of the info for the |j|-th state in the |i|-th ocp is at
location |ocp_info[ocp_state_base[i]+j]| and the |k|-th entry is in
location |ocp_info[ocp_info[ocp_state_base[i]+j]+k]|.
(These formulas assume that |min_quarterword| has already been 
added to |i|, |j| and |k|, since $\Omega$ stores its quarterwords that way.)

@d ocp_info_end(#)==#]
@d ocp_info(#)==ocp_tables[#,ocp_info_end
@d offset_ocp_file_size=0
@d offset_ocp_name=1
@d offset_ocp_area=offset_ocp_name+1
@d offset_ocp_external=offset_ocp_area+1
@d offset_ocp_external_arg=offset_ocp_external+1
@d offset_ocp_input=offset_ocp_external_arg+1
@d offset_ocp_output=offset_ocp_input+1
@d offset_ocp_no_tables=offset_ocp_output+1
@d offset_ocp_no_states=offset_ocp_no_tables+1
@d offset_ocp_table_base=offset_ocp_no_states+1
@d offset_ocp_state_base=offset_ocp_table_base+1
@d offset_ocp_info=offset_ocp_state_base+1
@d ocp_file_size(#)==ocp_info(#)(offset_ocp_file_size)
@d ocp_name(#)==ocp_info(#)(offset_ocp_name)
@d ocp_area(#)==ocp_info(#)(offset_ocp_area)
@d ocp_external(#)==ocp_info(#)(offset_ocp_external)
@d ocp_external_arg(#)==ocp_info(#)(offset_ocp_external_arg)
@d ocp_input(#)==ocp_info(#)(offset_ocp_input)
@d ocp_output(#)==ocp_info(#)(offset_ocp_output)
@d ocp_no_tables(#)==ocp_info(#)(offset_ocp_no_tables)
@d ocp_no_states(#)==ocp_info(#)(offset_ocp_no_states)
@d ocp_table_base(#)==ocp_info(#)(offset_ocp_table_base)
@d ocp_state_base(#)==ocp_info(#)(offset_ocp_state_base)

@ $\Omega$ always knows at least one ocp, namely the null ocp.
It does nothing.

@<Initialize table...@>=
ocp_ptr:=null_ocp; 
allocate_ocp_table(null_ocp,17);
ocp_file_size(null_ocp):=17;
ocp_name(null_ocp):="nullocp"; ocp_area(null_ocp):="";
ocp_external(null_ocp):=0; ocp_external_arg(null_ocp):=0;
ocp_input(null_ocp):=1; ocp_output(null_ocp):=1;
ocp_no_tables(null_ocp):=0;
ocp_no_states(null_ocp):=1;
ocp_table_base(f):=offset_ocp_info;
ocp_state_base(f):=offset_ocp_info;
ocp_info(null_ocp)(offset_ocp_info) := offset_ocp_info+2;  {number of entries}
ocp_info(null_ocp)(offset_ocp_info+1) := offset_ocp_info+5;  {number of entries}
ocp_info(null_ocp)(offset_ocp_info+2) := 23;  {|OTP_LEFT_START|}
ocp_info(null_ocp)(offset_ocp_info+3) := 3;  {|OTP_RIGHT_CHAR|}
ocp_info(null_ocp)(offset_ocp_info+4) := 36;  {|OTP_STOP|}


@ @<Put each...@>=
primitive("nullocp", set_ocp, null_ocp);
settext(frozen_null_ocp,"nullocp");
set_new_eqtb(frozen_null_ocp,new_eqtb(cur_val));
geq_define(ocp_active_number_base, data, 0);
geq_define(ocp_active_min_ptr_base, data, 0);
geq_define(ocp_active_max_ptr_base, data, 0);

@ Of course we want to define macros that suppress the detail of how ocp
information is actually packed, so that we don't have to write things like
$$\hbox{|ocp_info[k+ocp_info[j+ocp_state_base[i]]]|}$$
too often. The \.{WEB} definitions here make |ocp_state_entry(i)(j)(k)| 
(|ocp_table_entry(i)(j)(k)|) the |k|-th word in the |j|-th state (table)
of the |i|-th ocp.
@^inner loop@>

@d ocp_state_end(#)==#]
@d ocp_state_one(#)==#*2]+ocp_state_end
@d ocp_state_entry(#)==ocp_tables[#,ocp_tables[#,ocp_state_base(#)+ocp_state_one

@d ocp_state_no_end(#)==#*2+1]
@d ocp_state_no(#)==ocp_tables[#,ocp_state_base(#)+ocp_state_no_end

@d ocp_table_end(#)==#]
@d ocp_table_one(#)==#*2]+ocp_table_end
@d ocp_table_entry(#)==ocp_tables[#,ocp_tables[#,ocp_table_base(#)+ocp_table_one

@d ocp_table_no_end(#)==#*2+1]
@d ocp_table_no(#)==ocp_tables[#,ocp_table_base(#)+ocp_table_no_end

@ $\Omega$ checks the information of a \.{OCP} file for validity as the
file is being read in, so that no further checks will be needed when
typesetting is going on. The somewhat tedious subroutine that does this
is called |read_ocp_info|. It has three parameters: the user ocp
identifier~|u|, and the file name and area strings |nom| and |aire|.

The subroutine opens and closes a global file variable called |ocp_file|.
It returns the value of the internal ocp number that was just loaded.
If an error is detected, an error message is issued and no ocp
information is stored; |null_ocp| is returned in this case.

@d bad_ocp=11 {label for |read_ocp_info|}
@d ocp_abort(#)==begin print("OCP file error (");
 print(#); print(")"); print_ln; goto bad_ocp end
 {do this when the \.{OCP} data is wrong}

@p function read_ocp_info(@!u:pointer;@!nom,@!aire,@!ext:str_number;
                          @!external_ocp:boolean)
  :internal_ocp_number; {input a \.{OCP} file}
label done,bad_ocp,not_found;
var 
@!file_opened:boolean; {was |ocp_file| successfully opened?}
@!f:internal_ocp_number; {the new ocp's number}
@!g:internal_ocp_number; {the number to return}
@!ocpword:integer;
@!ocpmem_run_ptr:ocp_index;
@!ocp_length,real_ocp_length:integer; {length of ocp file}
@!previous_address:ocp_index;
@!temp_ocp_input:integer;
@!temp_ocp_output:integer;
@!temp_ocp_no_tables:integer;
@!temp_ocp_no_states:integer;
@!i,new_offset,room_for_tables,room_for_states:integer;
begin g:=null_ocp;@/
@<Read and check the ocp data; |ocp_abort| if the \.{OCP} file is
  malformed; if there's no room for this ocp, say so and |goto
  done|; otherwise |incr(ocp_ptr)| and |goto done|@>;
bad_ocp: @<Report that the ocp won't be loaded@>;
done: if file_opened then b_close(ocp_file);
ocp_name(f):=nom; ocp_area(f):=aire;
read_ocp_info:=g;
end;

@ $\Omega$ does not give precise details about why it
rejects a particular \.{OCP} file.

@d start_ocp_error_message==print_err("Translation process "); 
   sprint_cs(u); print_char("="); print_file_name(nom,aire,"");

@<Report that the ocp won't be loaded@>=
start_ocp_error_message;
@.Translation process x=xx not loadable...@>
if file_opened then print(" not loadable: Bad ocp file")
else print(" not loadable: ocp file not found");
help2("I wasn't able to read the data for this ocp,")@/
("so I will ignore the ocp specification.");
error

@ @<Read and check the ocp data...@>=
if external_ocp then 
  @<Check |ocp_file| exists@>
else begin
  @<Open |ocp_file| for input@>;
  @<Read the {\.{OCP}} file@>;
  end;

@ @<Check |ocp_file| exists@>=
begin
file_opened:=false;
pack_file_name(nom,aire,ext);
b_test_in;
if name_length=0 then ocp_abort("opening file");
f :=ocp_ptr+1;
allocate_ocp_table(f,13);
ocp_file_size(f):=13;
for i:=1 to name_length do begin
  append_char(name_of_file[i]);
  end;
ocp_external(f):=make_string;
scan_string_argument;
ocp_external_arg(f):=cur_val;
ocp_name(f):=""; ocp_area(f):="";
ocp_state_base(f):=0; ocp_table_base(f):=0;
ocp_input(f):=1; ocp_output(f):=1;
ocp_info(f)(offset_ocp_info):=0;
ocp_ptr:=f; g:=f;
goto done;
end

@ @<Open |ocp_file| for input@>=
file_opened:=false;
pack_file_name(nom,aire,".ocp");
if not b_open_in(ocp_file) then ocp_abort("opening file");
file_opened:=true

@ Note: A malformed \.{OCP} file might be shorter than it claims to be;
thus |eof(ocp_file)| might be true when |read_ocp_info| refers to
|ocp_file^| or when it says |get(ocp_file)|. If such circumstances
cause system error messages, you will have to defeat them somehow,
for example by defining |ocpget| to be `\ignorespaces|begin get(ocp_file);|
|if eof(ocp_file) then ocp_abort; end|\unskip'.
@^system dependencies@>

@d add_to_ocp_info(#)==begin ocp_tables[f,ocpmem_run_ptr]:=#;
  incr(ocpmem_run_ptr);
  end
@d ocpget==get(ocp_file)
@d ocpbyte==ocp_file^
@d ocp_read(#)==begin ocpword:=ocpbyte;
  if ocpword>127 then ocp_abort("checking first octet");
  ocpget; ocpword:=ocpword*@'400+ocpbyte;
  ocpget; ocpword:=ocpword*@'400+ocpbyte;
  ocpget; ocpword:=ocpword*@'400+ocpbyte;
  #:=ocpword;
  end
@d ocp_read_all(#)==begin ocpget; ocp_read(#); end
@d ocp_read_info==begin ocp_read_all(ocpword);
  add_to_ocp_info(ocpword);
  end

@ @<Read the {\.{OCP}} file@>=
begin
f :=ocp_ptr+1;
ocpmem_run_ptr:=offset_ocp_info;
ocp_read(ocp_length);
real_ocp_length:=ocp_length-7;
ocp_read_all(temp_ocp_input);
ocp_read_all(temp_ocp_output);
ocp_read_all(temp_ocp_no_tables);
ocp_read_all(room_for_tables);
ocp_read_all(temp_ocp_no_states);
ocp_read_all(room_for_states);
if real_ocp_length <>
   (temp_ocp_no_tables + room_for_tables +
    temp_ocp_no_states + room_for_states) then
  ocp_abort("checking size");
real_ocp_length:=real_ocp_length+12+
   temp_ocp_no_states+temp_ocp_no_tables;
allocate_ocp_table(f,real_ocp_length);
ocp_external(f):=0;
ocp_external_arg(f):=0;
ocp_file_size(f):=real_ocp_length;
ocp_input(f):=temp_ocp_input;
ocp_output(f):=temp_ocp_output;
ocp_no_tables(f):=temp_ocp_no_tables;
ocp_no_states(f):=temp_ocp_no_states;
ocp_table_base(f):=ocpmem_run_ptr;
if ocp_no_tables(f) <> 0 then begin
  previous_address:=ocpmem_run_ptr+2*(ocp_no_tables(f));
  for i:=1 to ocp_no_tables(f) do begin
    add_to_ocp_info(previous_address);
    ocp_read_all(new_offset);
    add_to_ocp_info(new_offset);
    previous_address:=previous_address+new_offset;
    end
  end;
if room_for_tables <> 0 then begin
  for i:=1 to room_for_tables do begin
    ocp_read_info;
    end
  end;
ocp_state_base(f):=ocpmem_run_ptr;
if ocp_no_states(f) <> 0 then begin
  previous_address:=ocpmem_run_ptr+2*(ocp_no_states(f));
  for i:=1 to ocp_no_states(f) do begin
    add_to_ocp_info(previous_address);
    ocp_read_all(new_offset);
    add_to_ocp_info(new_offset);
    previous_address:=previous_address+new_offset;
    end;
  end;
if room_for_states <> 0 then begin
  for i:=1 to room_for_states do begin
    ocp_read_info;
    end
  end;
ocp_ptr:=f; g:=f;
goto done;
end

@ Before we forget about the format of these tables, let's deal with 
$\Omega$'s basic scanning routine related to ocp information.

@<Declare procedures that scan ocp-related stuff@>=
procedure scan_ocp_ident;
var f:internal_ocp_number;
begin @<Get the next non-blank non-call...@>;
if cur_cmd=set_ocp then f:=cur_chr
else  begin print_err("Missing ocp identifier");
@.Missing ocp identifier@>
  help2("I was looking for a control sequence whose")@/
  ("current meaning has been defined by \ocp.");
  back_error; f:=null_ocp;
  end;
cur_val:=f;
end;


@ Here we begin the \.{OCP} list handling.


@<Types...@>=
@!internal_ocp_list_number=ocp_list_base..ocp_list_biggest;
@!ocp_list_index=integer; {index into |ocp_list_info|}
@!ocp_lstack_index=integer; {index into |ocp_lstack_info|}

@ Here now is the array of ocp arrays.
@d ocp_list_lnext(#)==ocp_list_info[#].hh.b0
@d ocp_list_lstack(#)==ocp_list_info[#].hh.b1
@d ocp_list_lstack_no(#)==ocp_list_info[#+1].sc
@d ocp_lstack_lnext(#)==ocp_lstack_info[#].hh.b0
@d ocp_lstack_ocp(#)==ocp_lstack_info[#].hh.b1
@d make_null_ocp_list==make_ocp_list_node(0,ocp_maxint,0)
@d is_null_ocp_list(#)==ocp_list_lstack_no(#)=ocp_maxint
@d make_null_ocp_lstack==0
@d is_null_ocp_lstack(#)==#=0
@d add_before_op=1
@d add_after_op=2
@d remove_before_op=3
@d remove_after_op=4

@<Glob...@>=
@!ocp_list_info:array[ocp_list_index] of memory_word;
  {the big collection of ocp list data}
@!ocp_listmem_ptr:ocp_list_index; {first unused word of |ocp_list_info|}
@!ocp_listmem_run_ptr:ocp_list_index; {temp unused word of |ocp_list_info|}
@!ocp_lstack_info:array[ocp_lstack_index] of memory_word;
  {the big collection of ocp lstack data}
@!ocp_lstackmem_ptr:ocp_lstack_index; {first unused word of |ocp_lstack_info|}
@!ocp_lstackmem_run_ptr:ocp_lstack_index; {temp unused word of |ocp_lstack_info|}
@!ocp_list_ptr:internal_ocp_list_number; {largest internal ocp list number in use}
@!ocp_list_list:array[internal_ocp_list_number] of ocp_list_index;

@
@<Initialize table...@>=
ocp_listmem_ptr:=2;
ocp_list_lstack(0):=0;
ocp_list_lstack_no(0):=ocp_maxint;
ocp_list_lnext(0):=0;
ocp_list_ptr:=null_ocp_list;
ocp_list_list[null_ocp_list]:=0;
ocp_lstackmem_ptr:=1;

@ $\Omega$ always knows at least one ocp list, namely the null ocp list.

@ @<Put each...@>=
primitive("nullocplist", set_ocp_list, null_ocp_list);
settext(frozen_null_ocp_list,"nullocplist"); 
set_new_eqtb(frozen_null_ocp_list,new_eqtb(cur_val));

@ @p function make_ocp_list_node(llstack:ocp_lstack_index; 
                                 llstack_no:scaled; 
                                 llnext:ocp_list_index):ocp_list_index;
var p:ocp_list_index;
begin 
p:=ocp_listmem_run_ptr;
ocp_list_lstack(p):=llstack;
ocp_list_lstack_no(p):=llstack_no;
ocp_list_lnext(p):=llnext;
ocp_listmem_run_ptr:=ocp_listmem_run_ptr+2;
make_ocp_list_node:=p;
end;

function make_ocp_lstack_node(locp:internal_ocp_number;
                              llnext:ocp_lstack_index) : ocp_lstack_index;
var p:ocp_lstack_index;
begin 
p:=ocp_lstackmem_run_ptr;
ocp_lstack_ocp(p):=locp;
ocp_lstack_lnext(p):=llnext;
incr(ocp_lstackmem_run_ptr);
make_ocp_lstack_node:=p;
end;

function copy_ocp_lstack(llstack:ocp_lstack_index):ocp_lstack_index;
var result:ocp_lstack_index;
begin
if is_null_ocp_lstack(llstack) then
  result:=make_null_ocp_lstack
else
  result:=make_ocp_lstack_node(ocp_lstack_ocp(llstack),
                               copy_ocp_lstack(ocp_lstack_lnext(llstack)));
copy_ocp_lstack:=result;
end;

function copy_ocp_list(list:ocp_list_index):ocp_list_index;
var result:ocp_list_index;
begin
if is_null_ocp_list(list) then
  result:=make_null_ocp_list
else
  result:=make_ocp_list_node(copy_ocp_lstack(ocp_list_lstack(list)),
                             ocp_list_lstack_no(list),
                             copy_ocp_list(ocp_list_lnext(list)));
copy_ocp_list:=result;
end;

function ocp_ensure_lstack(list:ocp_list_index; llstack_no:scaled):
   ocp_list_index;
var p:ocp_list_index;
    q:ocp_list_index;
begin
p:=list;
if is_null_ocp_list(p) then begin
  ocp_list_lstack_no(p) := llstack_no;
  ocp_list_lnext(p) := make_null_ocp_list;
  end
else if ocp_list_lstack_no(p) > llstack_no then begin
  ocp_list_lnext(p):= 
    make_ocp_list_node(ocp_list_lstack(p),
                       ocp_list_lstack_no(p),
                       ocp_list_lnext(p));
  ocp_list_lstack(p):=0;
  ocp_list_lstack_no(p):=llstack_no;
  end
else begin
  q:=ocp_list_lnext(p);
  while (not (is_null_ocp_list(q))) and 
         ocp_list_lstack_no(q) <= llstack_no do begin
    p:=q; q:=ocp_list_lnext(q);
    end;
  if ocp_list_lstack_no(p) < llstack_no then begin
    ocp_list_lnext(p) := make_ocp_list_node(0, llstack_no, q);
    p := ocp_list_lnext(p);
    end;
  end;
ocp_ensure_lstack := p;
end;

procedure ocp_apply_add(list_entry:ocp_list_index;
                        lbefore:boolean;
                        locp:internal_ocp_number);
var p:ocp_lstack_index;
    q:ocp_lstack_index;
begin
p := ocp_list_lstack(list_entry);
if lbefore or (p=0) then begin
  ocp_list_lstack(list_entry) := make_ocp_lstack_node(locp, p);
  end
else begin
  q:=ocp_lstack_lnext(p);
  while q<>0 do begin
    p:=q; q:=ocp_lstack_lnext(q);
    end;
  ocp_lstack_lnext(p):=make_ocp_lstack_node(locp, null);
  end;
end;

procedure ocp_apply_remove(list_entry:ocp_list_index;
                           lbefore:boolean);
var p:ocp_lstack_index;
    q:ocp_lstack_index;
    r:ocp_lstack_index;
begin
p := ocp_list_lstack(list_entry);
if p=0 then begin
  print_err("warning: stack entry already empty"); print_ln
  end
else begin
  q := ocp_lstack_lnext(p);
  if lbefore or (q=0) then
    ocp_list_lstack(list_entry) := q
  else begin
    r:=ocp_lstack_lnext(q);
    while r <> 0 do begin
      p:=q; q:=r; r:=ocp_lstack_lnext(r);
      end;
    ocp_lstack_lnext(p) := null;
    end
  end;
end;

procedure scan_scaled; {sets |cur_val| to a scaled value}
label done, done1, done2, found, not_found, attach_fraction;
var negative:boolean; {should the answer be negated?}
@!f:integer; {numerator of a fraction whose denominator is $2^{16}$}
@!k,@!kk:small_number; {number of digits in a decimal fraction}
@!p,@!q:pointer; {top of decimal digit stack}
begin f:=0; arith_error:=false; negative:=false;
@<Get the next non-blank non-sign...@>;
back_input;
if cur_tok=continental_point_token then cur_tok:=point_token;
if cur_tok<>point_token then scan_int
else  begin radix:=10; cur_val:=0;
  end;
if cur_tok=continental_point_token then cur_tok:=point_token;
if (radix=10)and(cur_tok=point_token) then @<Scan decimal fraction@>;
if cur_val<0 then {in this case |f=0|}
  begin negative := not negative; negate(cur_val);
  end;
if cur_val>@'40000 then arith_error:=true
else cur_val := cur_val*unity +f;
if arith_error or(abs(cur_val)>=@'10000000000) then
begin print_err("Stack number too large");
end;
if negative then negate(cur_val);
end;

procedure print_ocp_lstack(lstack_entry:ocp_lstack_index);
var p:ocp_lstack_index;
begin
p:=lstack_entry;
while (p<>0) do begin
  print_esc(ocp_id_text(ocp_lstack_ocp(p)));
  p:=ocp_lstack_lnext(p);
  if (p<>0) then print(",");
  end;
end;

procedure print_ocp_list(list_entry:ocp_list_index);
var p:ocp_list_index;
begin
print("["); p:=list_entry;
while not (is_null_ocp_list(p)) do begin
  print("(");
  print_scaled(ocp_list_lstack_no(p));
  print(" : ");
  print_ocp_lstack(ocp_list_lstack(p));
  print(")");
  p:=ocp_list_lnext(p);
  if not (is_null_ocp_list(p)) then print(", ");
  end;
print("]");
end;

function scan_ocp_list: ocp_list_index;
var llstack_no:scaled; 
    lop:quarterword;
    lstack_entry:ocp_list_index;
    other_list:ocp_list_index;
    ocp_ident:internal_ocp_number;
    result:ocp_list_index;
begin
get_r_token; 
if cur_cmd = set_ocp_list then
  result := copy_ocp_list(ocp_list_list[cur_chr])
else if cur_cmd <> ocp_list_op then begin
  print_err("Bad ocp list specification");
@.Bad ocp list specification@>
  help1("I was looking for a ocp list specification.");
  result := make_null_ocp_list;
  end
else begin
  lop:=cur_chr;
  scan_scaled; llstack_no:=cur_val;
  if (llstack_no<=0) or (llstack_no>=ocp_maxint) then begin
    print_err("Stack numbers must be between 0 and 4096 (exclusive)");
    result := make_null_ocp_list;
    end
  else begin
    if lop <= add_after_op then begin
      scan_ocp_ident; ocp_ident:=cur_val;
      end;
    other_list:=scan_ocp_list;
    lstack_entry:=ocp_ensure_lstack(other_list, llstack_no);
    if lop <= add_after_op then
      ocp_apply_add(lstack_entry, (lop=add_before_op), ocp_ident)
    else
      ocp_apply_remove(lstack_entry, (lop=remove_before_op));
    result:=other_list;
    end;
  end;
scan_ocp_list:=result;
end;

function read_ocp_list: internal_ocp_list_number;
var f:internal_ocp_list_number;
    g:internal_ocp_list_number;
begin
g:=null_ocp_list;
f:=ocp_list_ptr+1;
ocp_listmem_run_ptr:=ocp_listmem_ptr;
ocp_lstackmem_run_ptr:=ocp_lstackmem_ptr;
ocp_list_list[f]:=scan_ocp_list;
ocp_list_ptr:=f;
ocp_listmem_ptr:=ocp_listmem_run_ptr;
ocp_lstackmem_ptr:=ocp_lstackmem_run_ptr;
g:=f;
read_ocp_list:=g;
end;

procedure scan_ocp_list_ident;
var f:internal_ocp_list_number;
begin @<Get the next non-blank non-call...@>;
if cur_cmd=set_ocp_list then f:=cur_chr
else  begin print_err("Missing ocp list identifier");
@.Missing ocp list identifier@>
  help2("I was looking for a control sequence whose")@/
  ("current meaning has been defined by \ocplist.");
  back_error; f:=null_ocp_list;
  end;
cur_val:=f;
end;


@z
%---------------------------------------
@x [49] m.1210 l.22629 - Omega OCP
any_mode(set_interaction):prefixed_command;
@y
any_mode(set_interaction),
any_mode(set_ocp),
any_mode(def_ocp),
any_mode(set_ocp_list),
any_mode(def_ocp_list),
any_mode(clear_ocp_lists),
any_mode(push_ocp_list),
any_mode(pop_ocp_list),
any_mode(ocp_list_op),
any_mode(ocp_trace_level) : prefixed_command;
@z
%---------------------------------------
@x [50] m.1302 l.23682 - Omega OCP
@<Dump the font information@>;
@y
@<Dump the font information@>;
@<Dump the ocp information@>;
@<Dump the ocp list information@>;
@z
%---------------------------------------
@x [50] m.1303 l.23711 - Omega OCP
@<Undump the font information@>;
@y
@<Undump the font information@>;
@<Undump the ocp information@>;
@<Undump the ocp list information@>;
@z
%---------------------------------------
@x [50] m.1323 l.24907 - Omega OCP
begin undump_font_table(k);@/
end
@y
begin undump_font_table(k);@/
end

@ @<Dump the ocp information@>=
dump_int(ocp_ptr);
for k:=null_ocp to ocp_ptr do
  @<Dump the array info for internal ocp number |k|@>;
print_ln; print_int(ocp_ptr-ocp_base); print(" preloaded ocp");
if ocp_ptr<>ocp_base+1 then print_char("s")

@ @<Undump the ocp information@>=
undump_size(ocp_base)(ocp_biggest)('ocp max')(ocp_ptr);
for k:=null_ocp to ocp_ptr do
  @<Undump the array info for internal ocp number |k|@>

@ @<Dump the array info for internal ocp number |k|@>=
begin dump_ocp_table(k);
print_nl("\ocp"); print_esc(ocp_id_text(k)); print_char("=");
print_file_name(ocp_name(k),ocp_area(k),"");
end

@ @<Undump the array info for internal ocp number |k|@>=
begin undump_ocp_table(k);
end

@ @<Dump the ocp list information@>=
dump_int(ocp_listmem_ptr);
for k:=0 to ocp_listmem_ptr-1 do dump_wd(ocp_list_info[k]);
dump_int(ocp_list_ptr);
for k:=null_ocp_list to ocp_list_ptr do begin
  dump_int(ocp_list_list[k]);
  print_nl("\ocplist"); 
  print_esc(ocp_list_id_text(k)); 
  print_char("=");
  print_ocp_list(ocp_list_list[k]);
  end;
dump_int(ocp_lstackmem_ptr);
for k:=0 to ocp_lstackmem_ptr-1 do dump_wd(ocp_lstack_info[k])

@ @<Undump the ocp list information@>=
undump_size(1)(1000000)('ocp list mem size')(ocp_listmem_ptr);
for k:=0 to ocp_listmem_ptr-1 do undump_wd(ocp_list_info[k]);
undump_size(ocp_list_base)(ocp_list_biggest)('ocp list max')(ocp_list_ptr);
for k:=null_ocp_list to ocp_list_ptr do
  undump_int(ocp_list_list[k]);
undump_size(1)(1000000)('ocp lstack mem size')(ocp_lstackmem_ptr);
for k:=0 to ocp_lstackmem_ptr-1 do undump_wd(ocp_lstack_info[k])

@z
%---------------------------------------
@x
@* \[54] $\Omega$ changes.

@y
@* \[54] $\Omega$ changes.

@ Here we do the main work required for reading and interpreting
  $\Omega$ Compiled Translation Processes.

@ @<Put each...@>=
primitive("ocp", def_ocp, 0);
primitive("externalocp", def_ocp, 1);
primitive("ocplist", def_ocp_list, 0);
primitive("pushocplist", push_ocp_list, 0);
primitive("popocplist", pop_ocp_list, 0);
primitive("clearocplists", clear_ocp_lists, 0);
primitive("addbeforeocplist", ocp_list_op, add_before_op);
primitive("addafterocplist", ocp_list_op, add_after_op);
primitive("removebeforeocplist", ocp_list_op, remove_before_op);
primitive("removeafterocplist", ocp_list_op, remove_after_op);
primitive("ocptracelevel", ocp_trace_level, 0);
set_equiv(ocp_trace_level_base,0);

@ @<Cases of |print_cmd_chr|...@>=
set_ocp: begin
  print("select ocp "); 
  slow_print(ocp_name(chr_code)); 
  end;
def_ocp: if cur_chr=0 then print_esc("ocp")
  else print_esc("externalocp");
set_ocp_list: print("select ocp list "); 
def_ocp_list:  print_esc("ocplist");
push_ocp_list: print_esc("pushocplist");
pop_ocp_list:  print_esc("popocplist");
clear_ocp_lists: print_esc("clearocplists");
ocp_list_op:
  if chr_code=add_before_op then print_esc("addbeforeocplist")
  else if chr_code=add_after_op then print_esc("addafterocplist")
  else if chr_code=remove_before_op then print_esc("removebeforeocplist")
  else {|chr_code|=|remove_after_op|} print_esc("removeafterocplist");
ocp_trace_level: print_esc("ocptracelevel");

@ @<Assignments@>=
set_ocp: begin
  print_err("To use ocps, use the "); print_esc("pushocplist"); 
  print(" primitive");print_ln
  end;
def_ocp: new_ocp(a);
set_ocp_list: begin
  print_err("To use ocp lists, use the "); 
  print_esc("pushocplist"); print(" primitive");print_ln
  end;
def_ocp_list: new_ocp_list(a);
push_ocp_list: do_push_ocp_list(a);
pop_ocp_list: do_pop_ocp_list(a);
clear_ocp_lists: do_clear_ocp_lists(a);
ocp_list_op: begin
  print_err("To build ocp lists, use the ");
  print_esc("ocplist"); print(" primitive"); print_ln
  end;
ocp_trace_level: begin scan_optional_equals; scan_int;
  if cur_val<>0 then cur_val:=1;
  define(ocp_trace_level_base, data, cur_val);
  end;

@ @<Declare subprocedures for |prefixed_command|@>=
procedure new_ocp(@!a:small_number);
label common_ending;
var u:pointer; {user's ocp identifier}
@!f:internal_ocp_number; {runs through existing ocps}
@!t:str_number; {name for the frozen ocp identifier}
@!old_setting:0..max_selector; {holds |selector| setting}
@!flushable_string:str_number; {string not yet referenced}
@!external_ocp:boolean; {external binary file}
begin if job_name=0 then open_log_file;
  {avoid confusing \.{texput} with the ocp name}
@.texput@>
if cur_chr=1 then external_ocp:=true
else external_ocp:=false;
get_r_token; u:=cur_cs;
if u>=hash_base then t:=newtext(u)
else if u>=single_base then
  if u=null_cs then t:="OCP"@+else t:=u-single_base
else  begin old_setting:=selector; selector:=new_string;
  print("OCP"); print(u-active_base); selector:=old_setting;
@.OCPx@>
  str_room(1); t:=make_string;
  end;
define(u,set_ocp,null_ocp); scan_optional_equals; scan_file_name;
@<If this ocp has already been loaded, set |f| to the internal
  ocp number and |goto common_ending|@>;
f:=read_ocp_info(u,cur_name,cur_area,cur_ext,external_ocp);
common_ending: 
set_equiv(u,f); set_new_eqtb(ocp_id_base+f,new_eqtb(u));
settext(ocp_id_base+f,t);
if equiv(ocp_trace_level_base)=1 then begin
  print_nl(""); print_esc("ocp"); print_esc(t); print("="); print(cur_name);
  end;
end;

@ When the user gives a new identifier to a ocp that was previously loaded,
the new name becomes the ocp identifier of record. OCP names `\.{xyz}' and
`\.{XYZ}' are considered to be different.

@<If this ocp has already been loaded...@>=
flushable_string:=str_ptr-1;
for f:=ocp_base+1 to ocp_ptr do
  if str_eq_str(ocp_name(f),cur_name)and str_eq_str(ocp_area(f),cur_area) then
    begin
    if cur_name=flushable_string then begin
      flush_string; cur_name:=ocp_name(f);
      end;
    goto common_ending
    end

@ @<Declare subprocedures for |prefixed_command|@>=
procedure new_ocp_list(@!a:small_number);
var u:pointer; {user's ocp list identifier}
@!f:internal_ocp_list_number; {runs through existing ocp lists}
@!t:str_number; {name for the frozen ocp list identifier}
@!old_setting:0..max_selector; {holds |selector| setting}
begin if job_name=0 then open_log_file;
  {avoid confusing \.{texput} with the ocp list name}
@.texput@>
get_r_token; u:=cur_cs;
if u>=hash_base then t:=newtext(u)
else if u>=single_base then
  if u=null_cs then t:="OCPLIST"@+else t:=u-single_base
else  begin old_setting:=selector; selector:=new_string;
  print("OCPLIST"); print(u-active_base); selector:=old_setting;
@.OCPx@>
  str_room(1); t:=make_string;
  end;
define(u,set_ocp_list,null_ocp_list); scan_optional_equals; 
f:=read_ocp_list;
    set_equiv(u,f); set_new_eqtb(ocp_list_id_base+f,new_eqtb(u));
    settext(ocp_list_id_base+f,t);
if equiv(ocp_trace_level_base)=1 then begin
  print_nl(""); print_esc("ocplist"); print_esc(t); print("=");
  print_ocp_list(ocp_list_list[f]);
  end;
end;

@ @<Declare subprocedures for |prefixed_command|@>=
procedure do_push_ocp_list(@!a:small_number);
var ocp_list_no:halfword;
    old_number:halfword;
    i:integer;
begin
scan_ocp_list_ident; ocp_list_no:=cur_val;
old_number:=equiv(ocp_active_number_base);
define(ocp_active_base+old_number, data, ocp_list_no);
define(ocp_active_number_base, data, (old_number+1));

if equiv(ocp_trace_level_base)=1 then begin
  print_nl("New active ocp list: {");
  for i:=old_number downto 0 do begin
    print_esc(ocp_list_id_text(equiv(ocp_active_base+i)));
    print("="); print_ocp_list(ocp_list_list[equiv(ocp_active_base+i)]);
    if i<>0 then print(",");
    end;
  print("}");
  end;

active_compile;
define(ocp_active_min_ptr_base, data, active_min_ptr);
define(ocp_active_max_ptr_base, data, active_max_ptr);
end;

@ @<Declare subprocedures for |prefixed_command|@>=
procedure do_pop_ocp_list(@!a:small_number);
var old_number:halfword;
    i:integer;
begin
old_number:=equiv(ocp_active_number_base);
if old_number=0 then begin
  print_err("No active ocp lists to be popped");
  end
else
  define(ocp_active_number_base, data, (old_number-1));

if equiv(ocp_trace_level_base)=1 then begin
  print_nl("New active ocp list: {");
  for i:=(old_number-2) downto 0 do begin
    print_esc(ocp_list_id_text(equiv(ocp_active_base+i)));
    print("="); print_ocp_list(ocp_list_list[equiv(ocp_active_base+i)]);
    if i<>0 then print(",");
    end;
  print("}");
  end;

active_compile;
define(ocp_active_min_ptr_base, data, active_min_ptr);
define(ocp_active_max_ptr_base, data, active_max_ptr);
end;

@ @<Declare subprocedures for |prefixed_command|@>=
procedure do_clear_ocp_lists(@!a:small_number);
begin
define(ocp_active_number_base, data, 0);
active_compile;
define(ocp_active_min_ptr_base, data, active_min_ptr);
define(ocp_active_max_ptr_base, data, active_max_ptr);
end;

@z
%---------------------------------------
