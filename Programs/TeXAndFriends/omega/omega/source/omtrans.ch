% omtrans.ch: Characters sets for input and output
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
@x [3] m.25 l.767 - Omega Translation
initialization.  We shall define a word file later; but it will be possible
for us to specify simple operations on word files before they are defined.
@y
initialization.  We shall define a word file later; but it will be possible
for us to specify simple operations on word files before they are defined.

@d no_mode=0
@d onebyte_mode=1
@d ebcdic_mode=2
@d twobyte_mode=3
@d twobyteLE_mode=4

@d trans_input=0
@d trans_output=1
@d trans_default_input=2
@d trans_default_output=3
@d trans_no_input=4
@d trans_no_output=5
@d trans_no_default_input=6
@d trans_no_default_output=7

@d mode_input=0
@d mode_output=1
@d mode_default_input=2
@d mode_default_output=3
@d mode_no_input=4
@d mode_no_output=5
@d mode_no_default_input=6
@d mode_no_default_output=7
@z
%---------------------------------------
@x [3] m.30 l.890 - Omega Translation
@!max_buf_stack:0..buf_size; {largest index used in |buffer|}
@y
@!max_buf_stack:0..buf_size; {largest index used in |buffer|}
@!term_in_mode:halfword;
@!term_in_translation:halfword;
@z
%---------------------------------------
@x [3] m.37 l.1065 - Omega Translation
  if not input_ln(term_in,true) then {this shouldn't happen}
@y
  if not new_input_ln(term_in,term_in_mode,term_in_translation,true)
then {this shouldn't happen}
@z
%---------------------------------------
@x [5] m.54 l. - Omega Translation
@!log_file : alpha_file; {transcript of \TeX\ session}
@y
@!log_file : alpha_file; {transcript of \TeX\ session}
@!term_out_mode:halfword;
@!term_out_translation:halfword;
@z
%---------------------------------------
@x [5] m.59 l. - Omega Translation
@<Basic printing...@>=
procedure print_char(@!s:ASCII_code); {prints a single character}
label exit;
begin if @<Character |s| is the current new-line character@> then
 if selector<pseudo then
  begin print_ln; return;
  end;
case selector of
term_and_log: begin wterm(xchr[s]); wlog(xchr[s]);
  incr(term_offset); incr(file_offset);
  if term_offset=max_print_line then
    begin wterm_cr; term_offset:=0;
    end;
  if file_offset=max_print_line then
    begin wlog_cr; file_offset:=0;
    end;
  end;
log_only: begin wlog(xchr[s]); incr(file_offset);
  if file_offset=max_print_line then print_ln;
  end;
term_only: begin wterm(xchr[s]); incr(term_offset);
  if term_offset=max_print_line then print_ln;
  end;
no_print: do_nothing;
pseudo: if tally<trick_count then trick_buf[tally mod error_line]:=s;
new_string: begin if pool_ptr<pool_size then append_char(s);
  end; {we drop characters if the string space is full}
othercases write(write_file[selector],xchr[s])
endcases;@/
incr(tally);
exit:end;
@y
@d omega_write(#)==case term_out_mode of
                   no_mode: write(#,xchr[s]);
                   onebyte_mode,ebcdic_mode: write(#,xchr[s]);
                   twobyte_mode: begin write(#,xchr[s div @"100]);
                     write(#,xchr[s mod @"100]);
                     end;
                   twobyteLE_mode: begin write(#,xchr[s mod @"100]);
                     write(#,xchr[s div @"100]);
                     end;
                   end

@d omega_file_write(#)==case write_file_mode[#] of
                   no_mode: write(write_file[#],xchr[s]);
                   onebyte_mode,ebcdic_mode: write(write_file[#],xchr[s]);
                   twobyte_mode: begin write(write_file[#],xchr[s div @"100]);
                     write(write_file[#],xchr[s mod @"100]);
                     end;
                   twobyteLE_mode: begin write(write_file[#],xchr[s mod @"100]);
                     write(write_file[#],xchr[s div @"100]);
                     end;
                   end

@<Basic printing...@>=
procedure print_char(@!s:ASCII_code); {prints a single character}
label exit;
begin if @<Character |s| is the current new-line character@> then
 if selector<pseudo then
  begin print_ln; return;
  end;
case selector of
term_and_log: begin omega_write(term_out); omega_write(log_file);
  incr(term_offset); incr(file_offset);
  if term_offset=max_print_line then
    begin wterm_cr; term_offset:=0;
    end;
  if file_offset=max_print_line then
    begin wlog_cr; file_offset:=0;
    end;
  end;
log_only: begin omega_write(log_file); incr(file_offset);
  if file_offset=max_print_line then print_ln;
  end;
term_only: begin omega_write(term_out); incr(term_offset);
  if term_offset=max_print_line then print_ln;
  end;
no_print: do_nothing;
pseudo: if tally<trick_count then trick_buf[tally mod error_line]:=s;
new_string: begin if pool_ptr<pool_size then append_char(s);
  end; {we drop characters if the string space is full}
othercases omega_file_write(selector)
endcases;@/
incr(tally);
exit:end;
@z
%---------------------------------------
@x [5] m.59 l. - Omega Translation
procedure print(@!s:integer); {prints string |s|}
label exit;
var j:pool_pointer; {current character code position}
@!nl:integer; {new-line character to restore}
@!l:integer; {for printing 16-bit characters}
begin if s>=str_ptr then s:="???" {this can't happen}
@.???@>
else if s<biggest_char then
  if s<0 then s:="???" {can't happen}
  else begin if selector>pseudo then
      begin print_char(s); return; {internal strings are not expanded}
      end;
    if (@<Character |s| is the current new-line character@>) then
      if selector<pseudo then
        begin print_ln; return;
        end;
    nl:=new_line_char;
    @<Set newline character to -1@>;
    if s<@"20 then begin
      print_char(@"5E); print_char(@"5E); print_char(s+@'100);
      end
    else if s<@"7F then
      print_char(s)
    else if s=@"7F then begin
      print_char(@"5E); print_char(@"5E); print_char(s-@'100);
      end
    else if s<@"100 then begin
      print_char(@"5E); print_char(@"5E);
      print_lc_hex((s mod @"100) div @"10); print_lc_hex(s mod @"10);
      end
    else begin {Here is where we generate the strings on the fly.}
      print_char(@"5E); print_char(@"5E);
      print_char(@"5E); print_char(@"5E);
      print_lc_hex(s div @"1000); print_lc_hex((s mod @"1000) div @"100);
      print_lc_hex((s mod @"100) div @"10); print_lc_hex(s mod @"10);
      end;
    @<Set newline character to nl@>;
    return;
    end;
j:=str_start(s);
while j<str_start(s+1) do
  begin print_char(so(str_pool[j])); incr(j);
  end;
exit:end;

@ Control sequence names, file names, and strings constructed with
\.{\\string} might contain |ASCII_code| values that can't
be printed using |print_char|. Therefore we use |slow_print| for them:

@<Basic print...@>=
procedure slow_print(@!s:integer); {prints string |s|}
var j:pool_pointer; {current character code position}
begin if (s>=str_ptr) or (s<=biggest_char) then print(s)
else begin j:=str_start(s);
  while j<str_start(s+1) do
    begin print(so(str_pool[j])); incr(j);
    end;
  end;
end;
@y
procedure print(@!s:integer); {prints string |s|}
label exit;
var j:pool_pointer; {current character code position}
@!nl:integer; {new-line character to restore}
@!l:integer; {for printing 16-bit characters}
begin if s>=str_ptr then s:="???" {this can't happen}
@.???@>
else if s<biggest_char then
  if s<0 then s:="???" {can't happen}
  else begin if selector>pseudo then
      begin print_char(s); return; {internal strings are not expanded}
      end;
    if (@<Character |s| is the current new-line character@>) then
      if selector<pseudo then
        begin print_ln; return;
        end;
    nl:=new_line_char;
    @<Set newline character to -1@>;
    if s<@"20 then begin
      print_char(@"5E); print_char(@"5E); print_char(s+@'100);
      end
    else if s<@"7F then
      print_char(s)
    else if s=@"7F then begin
      print_char(@"5E); print_char(@"5E); print_char(s-@'100);
      end
    else if s<@"100 then begin
      print_char(@"5E); print_char(@"5E);
      print_lc_hex((s mod @"100) div @"10); print_lc_hex(s mod @"10);
      end
    else begin {Here is where we generate the strings on the fly.}
      print_char(@"5E); print_char(@"5E);
      print_char(@"5E); print_char(@"5E);
      print_lc_hex(s div @"1000); print_lc_hex((s mod @"1000) div @"100);
      print_lc_hex((s mod @"100) div @"10); print_lc_hex(s mod @"10);
      end;
    @<Set newline character to nl@>;
    return;
    end;
j:=str_start(s);
while j<str_start(s+1) do
  begin print_char(so(str_pool[j])); incr(j);
  end;
exit:end;

procedure print_nl(@!s:str_number); {prints string |s| at beginning of line}
begin if ((term_offset>0)and(odd(selector)))or@|
  ((file_offset>0)and(selector>=log_only)) then print_ln;
print(s);
end;

procedure overflow_ocp_buf_size; forward;
procedure overflow_ocp_stack_size; forward;

procedure omega_print(s:str_number);
var new_s:str_number;
    om_mode,om_translation:integer;
    j:integer;
begin
om_mode:=no_mode;
om_translation:=0;
case selector of
term_and_log,log_only,term_only: begin
  om_mode:=term_out_mode;
  if om_mode<>no_mode then om_translation:=term_out_translation;
  end;
no_print,pseudo,new_string: do_nothing;
othercases begin
  om_mode:=write_file_mode[selector];
  if om_mode<>no_mode then om_translation:=write_file_translation[selector];
  end
endcases;

if (om_mode=no_mode) or
   ((om_mode=onebyte_mode) and (om_translation=0)) then
  begin
  j:=str_start(s);
  while j<str_start(s+1) do begin
    new_s:=so(str_pool[j]);
    if new_s<@"20 then begin
      print_char(@"5E); print_char(@"5E); print_char(new_s+@'100);
      end
    else if new_s<@"7F then
      print_char(new_s)
    else if new_s=@"7F then begin
      print_char(@"5E); print_char(@"5E); print_char(new_s-@'100);
      end
    else if new_s<@"100 then begin
      print_char(@"5E); print_char(@"5E);
      print_lc_hex((new_s mod @"100) div @"10); print_lc_hex(new_s mod @"10);
      end
    else begin {Here is where we generate the strings on the fly.}
      print_char(@"5E); print_char(@"5E);
      print_char(@"5E); print_char(@"5E);
      print_lc_hex(new_s div @"1000);
      print_lc_hex((new_s mod @"1000) div @"100);
      print_lc_hex((new_s mod @"100) div @"10);
      print_lc_hex(new_s mod @"10);
      end;
    incr(j);
    end;
  end
else begin
  if om_translation=0 then begin
    j:=str_start(s);
    otp_counter:=1;
    while j<str_start(s+1) do begin
      otp_output_buf[otp_counter]:=so(str_pool[j]);
      incr(otp_counter);
      incr(j);
      end;
    otp_output_end:=otp_counter;
    end
  else begin
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
    otp_input_ocp:=om_translation;
    j:=str_start(s);
    otp_counter:=1;
    while j<str_start(s+1) do begin
      otp_input_buf[otp_counter]:=so(str_pool[j]);
      incr(otp_counter);
      incr(j);
      end;
    otp_input_end:=otp_counter;
    while not otp_finished do
      @<Run the OTP program@>;
    end;
  for otp_counter:=1 to (otp_output_end-1) do begin
    new_s:=otp_output_buf[otp_counter];
    if (om_mode<twobyte_mode) and (new_s>255) then
      begin
      print_char(@"5E); print_char(@"5E);
      print_char(@"5E); print_char(@"5E);
      print_lc_hex(new_s div @"1000);
      print_lc_hex((new_s mod @"1000) div @"100);
      print_lc_hex((new_s mod @"100) div @"10);
      print_lc_hex(new_s mod @"10);
      end
    else
      print_char(new_s);
    end;
  end;
end;

@ Control sequence names, file names, and strings constructed with
\.{\\string} might contain |ASCII_code| values that can't
be printed using |print_char|. Therefore we use |slow_print| for them:

@<Basic print...@>=
procedure slow_print(@!s:integer); {prints string |s|}
begin if (s>=str_ptr) or (s<=biggest_char) then print(s)
else omega_print(s);
end;
@z
%---------------------------------------
@x [5] - Omega Translation
procedure print_nl(@!s:str_number); {prints string |s| at beginning of line}
begin if ((term_offset>0)and(odd(selector)))or@|
  ((file_offset>0)and(selector>=log_only)) then print_ln;
print(s);
end;
@y
@z
%---------------------------------------
@x [5] m.71 l.1694 - Omega Translation
if not input_ln(term_in,true) then fatal_error("End of file on the terminal!");
@y
if not new_input_ln(term_in,term_in_mode,term_in_translation,true)
then fatal_error("End of file on the terminal!");
@z
%---------------------------------------
@x [15] m.209 l.4170 - Omega Translation
@d max_command=ocp_trace_level
   {the largest command code seen at |big_switch|}
@y
@d char_trans=ocp_trace_level+1
@d char_mode=char_trans+1
@d max_command=char_mode
   {the largest command code seen at |big_switch|}
@z
%---------------------------------------
@x [17] m.230 l.4722 - Omega Translation
@d toks_base=ocp_active_base+max_active_ocp_lists
   {table of |number_regs| token list registers}
@y
@d ocp_input_mode_base=ocp_active_base+max_active_ocp_lists
@d ocp_input_onebyte_translation_base    =ocp_input_mode_base+ 1
@d ocp_input_ebcdic_translation_base     =ocp_input_mode_base+ 2
@d ocp_input_twobyte_translation_base    =ocp_input_mode_base+ 3
@d ocp_input_twobyteLE_translation_base  =ocp_input_mode_base+ 4

@d ocp_output_mode_base                  =ocp_input_mode_base+ 5
@d ocp_output_onebyte_translation_base   =ocp_input_mode_base+ 6
@d ocp_output_ebcdic_translation_base    =ocp_input_mode_base+ 7
@d ocp_output_twobyte_translation_base   =ocp_input_mode_base+ 8
@d ocp_output_twobyteLE_translation_base =ocp_input_mode_base+ 9

@d toks_base                             =ocp_input_mode_base+10
@z
%---------------------------------------
@x [22] m.304 l.6535 - Omega Translation
@!input_file : array[1..max_in_open] of alpha_file;
@y
@!input_file : array[1..max_in_open] of alpha_file;
@!input_file_mode : array[1..max_in_open] of halfword;
@!input_file_translation : array[1..max_in_open] of halfword;
@z
%---------------------------------------
@x [23] m.331 l.7066 - Omega Translation
param_ptr:=0; max_param_stack:=0;
@y
param_ptr:=0; max_param_stack:=0;
geq_define(ocp_input_mode_base,data,1);
term_in_mode:=equiv(ocp_input_mode_base);
term_out_mode:=equiv(ocp_input_mode_base);
if term_in_mode>0
then term_in_translation:=
     equiv(ocp_input_onebyte_translation_base+term_in_mode-1);
if term_out_mode>0
then term_out_translation:=
     equiv(ocp_output_onebyte_translation_base+term_in_mode-1);
@z
%---------------------------------------
@x [24] m.362 l.7066 - Omega Translation
  begin if input_ln(cur_file,true) then {not end of file}
@y
  begin if new_input_ln(cur_file,input_file_mode[index],
           input_file_translation[index],true) then {not end of file}
@z
%---------------------------------------
@x [27] m.480 l.9411 - Omega Translation
@!read_file:array[0..15] of alpha_file; {used for \.{\\read}}
@y
@!read_file:array[0..15] of alpha_file; {used for \.{\\read}}
@!read_file_mode:array[0..15] of halfword;
@!read_file_translation:array[0..15] of halfword;
@z
%---------------------------------------
@x [27] m.485 l.9475 - Omega Translation
@<Input the first line of |read_file[m]|@>=
if input_ln(read_file[m],false) then read_open[m]:=normal
else  begin a_close(read_file[m]); read_open[m]:=closed;
  end
@y
@<Input the first line of |read_file[m]|@>=
begin read_file_mode[m]:=
  get_file_mode(read_file[m], equiv(ocp_input_mode_base));
if read_file_mode[m]>0
then read_file_translation[m]:=
     equiv(ocp_input_onebyte_translation_base+read_file_mode[m]-1);
if new_input_ln(read_file[m],read_file_mode[m],
   read_file_translation[m],false) then read_open[m]:=normal
else  begin a_close(read_file[m]); read_open[m]:=closed;
  end
end
@z
%---------------------------------------
@x [27] m.486 l.9484 - Omega Translation
begin if not input_ln(read_file[m],true) then
  begin a_close(read_file[m]); read_open[m]:=closed;
@y
begin if not new_input_ln(read_file[m],
read_file_mode[m],read_file_translation[m],true) then
  begin a_close(read_file[m]); read_open[m]:=closed;
@z
%---------------------------------------
@x [29] m.538 l.10371 - Omega Translation
begin line:=1;
if input_ln(cur_file,false) then do_nothing;
@y
begin line:=1;
input_file_mode[index]:=
  get_file_mode(cur_file, equiv(ocp_input_mode_base));
if input_file_mode[index]>0
then input_file_translation[index]:=
     equiv(ocp_input_onebyte_translation_base+input_file_mode[index]-1);
if new_input_ln(cur_file,input_file_mode[index],
   input_file_translation[index],false) then do_nothing;
@z
%---------------------------------------
@x [49] m.1210 l.22629 - Omega Translation
any_mode(ocp_trace_level) : prefixed_command;
@y
any_mode(ocp_trace_level) : prefixed_command;
any_mode(char_mode) : do_char_mode;
any_mode(char_trans) : do_char_translation;
@z
%---------------------------------------
@x [49] m.1343 l.24928 - Omega Translation
@!write_file:array[0..15] of alpha_file;
@y
@!write_file:array[0..15] of alpha_file;
@!write_file_mode:array[0..15] of halfword;
@!write_file_translation:array[0..15] of halfword;
@z
%---------------------------------------
@x
@* \[54] $\Omega$ changes.

@y
@* \[54] $\Omega$ changes.

@ Here we do the main work required for reading and interpreting
  $\Omega$ Input Translation Processes.

@ @<Put each...@>=
primitive("InputTranslation",           char_trans, trans_input);
primitive("OutputTranslation",          char_trans, trans_output);
primitive("DefaultInputTranslation",    char_trans, trans_default_input);
primitive("DefaultOutputTranslation",   char_trans, trans_default_output);
primitive("noInputTranslation",         char_trans, trans_no_input);
primitive("noOutputTranslation",        char_trans, trans_no_output);
primitive("noDefaultInputTranslation",  char_trans, trans_no_default_input);
primitive("noDefaultOutputTranslation", char_trans, trans_no_default_output);
primitive("InputMode",                  char_mode,  mode_input);
primitive("OutputMode",                 char_mode,  mode_output);
primitive("DefaultInputMode",           char_mode,  mode_default_input);
primitive("DefaultOutputMode",          char_mode,  mode_default_output);
primitive("noInputMode",                char_mode,  mode_no_input);
primitive("noOutputMode",               char_mode,  mode_no_output);
primitive("noDefaultInputMode",         char_mode,  mode_no_default_input);
primitive("noDefaultOutputMode",        char_mode,  mode_no_default_output);

geq_define(ocp_input_onebyte_translation_base, data, 0);
geq_define(ocp_input_ebcdic_translation_base, data, 0);
geq_define(ocp_input_twobyte_translation_base, data, 0);
geq_define(ocp_input_twobyteLE_translation_base, data, 0);
geq_define(ocp_input_mode_base, data, 0);

geq_define(ocp_output_onebyte_translation_base, data, 0);
geq_define(ocp_output_ebcdic_translation_base, data, 0);
geq_define(ocp_output_twobyte_translation_base, data, 0);
geq_define(ocp_output_twobyteLE_translation_base, data, 0);
geq_define(ocp_output_mode_base, data, 0);

@ @<Cases of |print_cmd_chr|...@>=
char_trans:
  case chr_code of
  trans_input:             print_esc("InputTranslation");
  trans_output:            print_esc("OutputTranslation");
  trans_default_input:     print_esc("DefaultInputTranslation");
  trans_default_output:    print_esc("DefaultOutputTranslation");
  trans_no_input:          print_esc("noInputTranslation");
  trans_no_output:         print_esc("noOutputTranslation");
  trans_no_default_input:  print_esc("noDefaultInputTranslation");
  trans_no_default_output: print_esc("noDefaultOutputTranslation");
  end;
char_mode:
  case chr_code of
  mode_input:              print_esc("InputMode");
  mode_output:             print_esc("OutputMode");
  mode_default_input:      print_esc("DefaultInputMode");
  mode_default_output:     print_esc("DefaultOutputMode");
  mode_no_input:           print_esc("noInputMode");
  mode_no_output:          print_esc("noOutputMode");
  mode_no_default_input:   print_esc("noDefaultInputMode");
  mode_no_default_output:  print_esc("noDefaultOutputMode");
  end;

@ @<Declare subprocedures for |prefixed_command|@>=
procedure scan_file_referrent;
begin
if scan_keyword("currentfile") then cur_val:=max_halfword
else scan_int;
end;

procedure scan_mode;
begin
if scan_keyword("onebyte") then cur_val:=onebyte_mode
else if scan_keyword("ebcdic") then cur_val:=ebcdic_mode
else if scan_keyword("twobyte") then cur_val:=twobyte_mode
else if scan_keyword("twobyteLE") then cur_val:=twobyteLE_mode
else begin print_err("Invalid input mode"); cur_val:=0; end;
end;

procedure do_char_translation;
var kind:halfword;
    fileref:halfword;
    moderef:halfword;
    ocpref:halfword;
begin
fileref:=0;
moderef:=0;
ocpref:=0;
kind:=cur_chr;
if (kind mod 4) <= 1 then begin
  scan_file_referrent;
  fileref:=cur_val;
  end
else begin
  scan_mode;
  moderef:=cur_val;
  end;
if kind < 4 then begin
  scan_ocp_ident;
  ocpref:=cur_val;
  end;
case (kind mod 4) of
  trans_input: begin
    if (fileref>=0) and (fileref<=15) then begin
      read_file_translation[fileref]:=ocpref;
      end
    else if fileref<>max_halfword then begin
      term_in_translation:=ocpref;
      end
    else begin
      base_ptr:=input_ptr; input_stack[base_ptr]:=cur_input;
      while state = token_list do
        begin
        decr(base_ptr);
        cur_input:=input_stack[base_ptr];
        end;
      if name>17 then input_file_translation[index]:=ocpref;
      cur_input:=input_stack[input_ptr];
      end;
    end;
  trans_output: begin
    if (fileref>=0) and (fileref<=15) then begin
      write_file_translation[fileref]:=ocpref;
      end
    else begin
      term_out_translation:=ocpref;
      end;
    end;
  trans_default_input: begin
    geq_define(ocp_input_mode_base+moderef,data,ocpref);
    end;
  trans_default_output: begin
    geq_define(ocp_output_mode_base+moderef,data,ocpref);
    end;
  end;
end;

procedure do_char_mode;
var kind:halfword;
    fileref:halfword;
    moderef:halfword;
begin
fileref:=0;
moderef:=0;
kind:=cur_chr;
if (kind mod 4) <= 1 then begin
  scan_file_referrent;
  fileref:=cur_val;
  end;
if kind < 4 then begin
  scan_mode;
  moderef:=cur_val;
  end;
case (kind mod 4) of
  trans_input: begin
    if (fileref>=0) and (fileref<=15) then begin
      read_file_mode[fileref]:=moderef;
      end
    else if fileref<>max_halfword then begin
      term_in_mode:=moderef;
      end
    else begin
      base_ptr:=input_ptr; input_stack[base_ptr]:=cur_input;
      while state = token_list do
        begin
        decr(base_ptr);
        cur_input:=input_stack[base_ptr];
        end;
      if name>17 then input_file_mode[index]:=moderef;
      cur_input:=input_stack[input_ptr];
      end;
    end;
  trans_output: begin
    if (fileref>=0) and (fileref<=15) then begin
      write_file_mode[fileref]:=moderef;
      end
    else begin
      term_out_mode:=moderef;
      end;
    end;
  trans_default_input: begin
    geq_define(ocp_input_mode_base,data,moderef);
    end;
  trans_default_output: begin
    geq_define(ocp_output_mode_base,data,moderef);
    end;
  end;
end;

function p_new_input_ln(f:alpha_file;
the_mode,translation:halfword;
bypass_eoln:boolean):boolean;
begin
if (the_mode=no_mode) or ((the_mode=onebyte_mode) and (translation=0))
then p_new_input_ln:=input_ln(f,bypass_eoln)
else if not newinputline(f,the_mode,bypass_eoln)
then p_new_input_ln:=false
else if translation=0 then
begin
   if (first+otp_input_end)>=buf_size then overflow("buffer size", buf_size);
   state:=new_line;
   start:=first;
   last:=start;
   loc:=start;
   for otp_counter:=1 to otp_input_end do
   begin
      buffer[last]:=otp_input_buf[otp_counter];
      incr(last);
   end;
   while buffer[last-1]=' ' do decr(last);
   limit:=last-1;
   p_new_input_ln:=true
end
else
begin
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
   otp_input_ocp:=translation;
   while not otp_finished do
      @<Run the OTP program@>;
   if (first+otp_output_end)>=buf_size then overflow("buffer size", buf_size);
   state:=new_line;
   start:=first;
   last:=start;
   loc:=start;
   for otp_counter:=1 to otp_output_end do
   begin
      buffer[last]:=otp_output_buf[otp_counter];
      incr(last);
   end;
   while buffer[last-1]=' ' do decr(last);
   limit:=last-1;
   p_new_input_ln:=true
end
end;


@z
%---------------------------------------
