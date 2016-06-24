%% omega-miktex.ch:
%% 
%% Copyright (C) 1998-2016 Christian Schenk
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

%% ////////////////////////////////////////////////////////////////////////////
%% //                                                                        //
%% //                      INSPIRED BY WEB2C'S TEX.CH                        //
%% //                                                                        //
%% ////////////////////////////////////////////////////////////////////////////

@x
\def\PASCAL{Pascal}
@y
\def\PASCAL{Pascal}
\def\C4P{C4P}
\def\Cplusplus{C++}
\def\MiKTeX{MiK\TeX}
@z

% WEAVE: print changes only.

@x
  \def\?##1]{\hbox to 1in{\hfil##1.\ }}
  }
@y 83
  \def\?##1]{\hbox{Changes to \hbox to 1em{\hfil##1}.\ }}
  }
\let\maybe=\iffalse
@z

% _____________________________________________________________________________
%
% [1.4]
% _____________________________________________________________________________

@x
procedure initialize; {this procedure gets things started properly}
@y
@t\4@>@<Declare \MiKTeX\ procedures@>@/
@#
procedure initialize; {this procedure gets things started properly}
@z

% _____________________________________________________________________________
%
% [1.6]
% _____________________________________________________________________________

@x
@d final_end=9999 {this label marks the ending of the program}
@y
@d final_end=9999 {this label marks the ending of the program}
@d goto_end_of_TEX==c4p_throw(end_of_TEX) {\MiKTeX: throw \Cplusplus\ exception}
@d goto_final_end==c4p_throw(final_end) {\MiKTeX: throw \Cplusplus\ exception}
@z

% _____________________________________________________________________________
%
% [1.8]
% _____________________________________________________________________________

@x
the codewords `$|init|\ldots|tini|$'.

@d init== {change this to `$\\{init}\equiv\.{@@\{}$' in the production version}
@d tini== {change this to `$\\{tini}\equiv\.{@@\}}$' in the production version}
@y
the codewords `$|init|\ldots|tini|$' for declarations and by the codewords
`$|Init|\ldots|Tini|$' for executable code.  This distinction is helpful for
implementations where a run-time switch differentiates between the two
versions of the program.

@d init=={ifdef('INITEX')}
@d tini=={endif('INITEX')}
@d Init==init if miktex_is_init_program then begin
@d Tini==end;@+tini
@f Init==begin
@f Tini==end
@z

@x
@!init @<Initialize table entries (done by \.{INITEX} only)@>@;@+tini
@y
@!Init @<Initialize table entries (done by \.{INITEX} only)@>@;@+Tini
@z

% _____________________________________________________________________________
%
% [1.11]
% _____________________________________________________________________________

@x
@!mem_max=30000; {greatest index in \TeX's internal |mem| array;
  must be strictly less than |max_halfword|;
  must be equal to |mem_top| in \.{INITEX}, otherwise |>=mem_top|}
@y
@z

@x
@!mem_min=0; {smallest index in \TeX's internal |mem| array;
  must be |min_halfword| or more;
  must be equal to |mem_bot| in \.{INITEX}, otherwise |<=mem_bot|}
@y
@z

@x
@!buf_size=500; {maximum number of characters simultaneously present in
  current lines of open files and in control sequences between
  \.{\\csname} and \.{\\endcsname}; must not exceed |max_halfword|}
@y
@!inf_buf_size = 500;
@!sup_buf_size = 30000000;
@z

@x
@!error_line=72; {width of context lines on terminal error messages}
@y
@!inf_error_line=40;
@!sup_error_line=65535;
@z

@x
@!half_error_line=42; {width of first lines of contexts in terminal
  error messages; should be between 30 and |error_line-15|}
@y
@!inf_half_error_line=30;
@!sup_half_error_line=65535;
@z

@x
@!max_print_line=79; {width of longest text lines output; should be at least 60}
@y
@!inf_max_print_line=40;
@!sup_max_print_line=65535;
@z

@x
@!stack_size=200; {maximum number of simultaneous input sources}
@y
@!inf_stack_size=30;
@!sup_stack_size=65535;
@z

@x
@!max_in_open=6; {maximum number of input files and error insertions that
  can be going on simultaneously}
@y
@!inf_max_in_open=6;
@!sup_max_in_open=255;
@z

@x
@!font_max=65535; {maximum internal font number; must be at most |font_biggest|}
@y
@!font_max=65535; {maximum internal font number; must be at most |font_biggest|}
@z

@x
@!param_size=60; {maximum number of simultaneous macro parameters}
@y
@!inf_param_size=60;
@!sup_param_size=600000;
@z

@x
@!nest_size=40; {maximum number of semantic levels simultaneously active}
@y
@!inf_nest_size=40;
@!sup_nest_size=4000;
@z

@x
@!max_strings=3000; {maximum number of strings; must not exceed |max_halfword|}
@y
@!inf_max_strings=3000;
@!sup_max_strings=2097151;
@!inf_strings_free=100;
@!sup_strings_free=262143;
@z

@x
@!string_vacancies=8000; {the minimum number of characters that should be
  available for the user's control sequences and font names,
  after \TeX's own error messages are stored}
@y
@!inf_string_vacancies=8000;
@!sup_string_vacancies=40000000;
@z

@x
@!pool_size=32000; {maximum number of characters in strings, including all
  error messages and help texts, and the names of all fonts and
  control sequences; must exceed |string_vacancies| by the total
  length of \TeX's own strings, which is currently about 23000}
@y
@!inf_pool_size=32000;
@!sup_pool_size=40000000;
@!inf_pool_free=1000;
@!sup_pool_free=40000000;
@z

@x
@!save_size=600; {space for saving values outside of current group; must be
  at most |max_halfword|}
@y
@!inf_save_size=600;
@!sup_save_size=80000;
@z

@x
@!trie_size=8000; {space for hyphenation patterns; should be larger for
@y
@!trie_size_def=80000; {space for hyphenation patterns; should be larger for
@z

@x
@!trie_op_size=500; {space for ``opcodes'' in the hyphenation patterns}
@y
@!trie_op_size_def=2048; {space for ``opcodes'' in the hyphenation patterns}
@z

@x
@!dvi_buf_size=800; {size of the output buffer; must be a multiple of 8}
@y
@!dvi_buf_size=8192; {size of the output buffer; must be a multiple of 8}
@z

@x
@!file_name_size=40; {file names shouldn't be longer than this}
@y
@!file_name_size=259; {file names shouldn't be longer than this}
@!file_name_size_plus_one=260; {one more}
@z

@x
@!active_mem_size=2000; {number of words of |active_info| for active ocps}
@y
@!active_mem_size=2000; {number of words of |active_info| for active ocps}
@z

@x
@!ocp_maxint=@"10000000;
@y
@!ocp_maxint=@"10000000;
@z

% _____________________________________________________________________________
%
% [1.12]
% _____________________________________________________________________________

@x
@d mem_bot=0 {smallest index in the |mem| array dumped by \.{INITEX};
  must not be less than |mem_min|}
@d mem_top==30000 {largest index in the |mem| array dumped by \.{INITEX};
  must be substantially larger than |mem_bot|
  and not greater than |mem_max|}
@y
@z

@x
@d font_base=0 {smallest internal font number; must not be less
@y
@d font_base=0 {smallest internal font number; must not be less
@z

@x
@d hash_size=65536 {maximum number of control sequences; it should be at most
@y
@d hash_size=65536 {maximum number of control sequences; it should be at most
@z

@x
@d hash_prime=55711 {a prime number equal to about 85\pct! of |hash_size|}
@y
@d hash_prime=55711 {a prime number equal to about 85\pct! of |hash_size|}
@z

@x
@d hyph_size=307 {another prime; the number of \.{\\hyphenation} exceptions}
@y
@d hyph_size=307 {another prime; the number of \.{\\hyphenation} exceptions}
@z

% _____________________________________________________________________________
%
% [3.27]
% _____________________________________________________________________________

@x
|name_of_file| could be opened.
@y
|name_of_file| could be opened.

\MiKTeX: we use our own functions to open files.
@z

@x
begin reset(f,name_of_file,'/O'); a_open_in:=reset_OK(f);
@y
begin a_open_in:=miktex_open_input_file(f);
@z

@x
begin rewrite(f,name_of_file,'/O'); a_open_out:=rewrite_OK(f);
@y
begin a_open_out:=miktex_open_output_file(f, true);
@z

@x
begin reset(f,name_of_file,'/O'); b_open_in:=reset_OK(f);
@y
begin b_open_in:=false {\MiKTeX\ does not need this function}
@z

@x
begin rewrite(f,name_of_file,'/O'); b_open_out:=rewrite_OK(f);
@y
begin b_open_out:=miktex_open_output_file(f, false);
@z

@x
begin reset(f,name_of_file,'/O'); w_open_in:=reset_OK(f);
@y
begin w_open_in:=false {\MiKTeX\ does not need this function}
@z

@x
begin rewrite(f,name_of_file,'/O'); w_open_out:=rewrite_OK(f);
@y
begin w_open_out:=miktex_open_output_file(f, false);
@z

% _____________________________________________________________________________
%
% [3.28]
% _____________________________________________________________________________

@x
begin close(f);
@y
begin miktex_close_file(f);
@z

@x
begin close(f);
@y
begin miktex_close_file(f);
@z

@x
begin close(f);
@y
begin miktex_close_file(f);
@z

% _____________________________________________________________________________
%
% [3.30]
% _____________________________________________________________________________

@x
@!buffer:array[0..buf_size] of ASCII_code; {lines of characters being read}
@!first:0..buf_size; {the first unused position in |buffer|}
@!last:0..buf_size; {end of the line just input to |buffer|}
@!max_buf_stack:0..buf_size; {largest index used in |buffer|}
@y
@!buffer:array[0..sup_buf_size] of ASCII_code; {lines of characters being read}
@!first:0..sup_buf_size; {the first unused position in |buffer|}
@!last:0..sup_buf_size; {end of the line just input to |buffer|}
@!max_buf_stack:0..sup_buf_size; {largest index used in |buffer|}
@z

% _____________________________________________________________________________
%
% [3.31]
% _____________________________________________________________________________

@x
finer tuning is often possible at well-developed \PASCAL\ sites.
@^inner loop@>
@y
finer tuning is often possible at well-developed \PASCAL\ sites.
@^inner loop@>

\MiKTeX: we use our own line-reader.
@z

@x
@p function input_ln(var f:alpha_file;@!bypass_eoln:boolean):boolean;
  {inputs the next line or returns |false|}
var last_nonblank:0..buf_size; {|last| with trailing blanks removed}
begin if bypass_eoln then if not eof(f) then get(f);
  {input the first character of the line into |f^|}
last:=first; {cf.\ Matthew 19\thinspace:\thinspace30}
if eof(f) then input_ln:=false
else  begin last_nonblank:=first;
  while not eoln(f) do
    begin if last>=max_buf_stack then
      begin max_buf_stack:=last+1;
      if max_buf_stack=buf_size then
        @<Report overflow of the input buffer, and abort@>;
      end;
    buffer[last]:=f^; get(f); incr(last);
    if buffer[last-1]<>" " then last_nonblank:=last;
    end;
  last:=last_nonblank; input_ln:=true;
  end;
end;
@y
@p function input_ln(var f:alpha_file;@!bypass_eoln:boolean):boolean;forward;
@z

% _____________________________________________________________________________
%
% [3.33]
% _____________________________________________________________________________

@x
in \ph. The `\.{/I}' switch suppresses the first |get|.
@^system dependencies@>
@y
in \ph. The `\.{/I}' switch suppresses the first |get|.
@^system dependencies@>

\MiKTeX: standard input and output streams were prepared
by the \C4P\ runtime library.
@z

@x
@d t_open_in==reset(term_in,'TTY:','/O/I') {open the terminal for text input}
@d t_open_out==rewrite(term_out,'TTY:','/O') {open the terminal for text output}
@y
@d t_open_in==term_in:=i@&nput
@d t_open_out==term_out:=output
@z

% _____________________________________________________________________________
%
% [3.34]
% _____________________________________________________________________________

@x
some instruction to the operating system.  The following macros show how
these operations can be specified in \ph:
@^system dependencies@>
@y
some instruction to the operating system.
@^system dependencies@>
@z

@x
@d clear_terminal == break_in(term_in,true) {clear the terminal input buffer}
@y
@d clear_terminal == break(term_in) {clear the terminal input buffer}
@z

% _____________________________________________________________________________
%
% [3.35]
% _____________________________________________________________________________

@x
  begin write_ln(term_out,'Buffer size exceeded!'); goto final_end;
@y
  begin write_ln(term_out,'Buffer size exceeded!');
  goto_final_end; {\MiKTeX: throw a \Cplusplus\ exception}
@z

% _____________________________________________________________________________
%
% [3.37]
% _____________________________________________________________________________

@x
if the system permits them.
@^system dependencies@>
@y
if the system permits them.
@^system dependencies@>

\MiKTeX: we possibly use the routine |miktex_initialize_buffer| to get the
first input line.
@z

@x
loop@+begin wake_up_terminal; write(term_out,'**'); update_terminal;
@y
loop@+begin
  if (c4p_plen > 0) then miktex_initialize_buffer
  else begin
    wake_up_terminal; write(term_out,'**'); update_terminal;
@z

@x
  loc:=first;
@y
    end;
  loc:=first;
@z

% _____________________________________________________________________________
%
% [4.38]
% _____________________________________________________________________________

@x
@!pool_pointer = 0..pool_size; {for variables that point into |str_pool|}
@!str_number = 0..max_strings; {for variables that point into |str_start|}
@y
@!pool_pointer = 0..sup_pool_size; {for variables that point into |str_pool|}
@!str_number = 0..sup_max_strings; {for variables that point into |str_start|}
@z

% _____________________________________________________________________________
%
% [4.47]
% _____________________________________________________________________________

@x
@p @!init function get_strings_started:boolean; {initializes the string pool,
@y
@p @t\4@>@<Declare additional routines for string recycling@>@/

@!init function get_strings_started:boolean; {initializes the string pool,
@z

% _____________________________________________________________________________
%
% [5.54]
% _____________________________________________________________________________

@x
@!term_offset : 0..max_print_line;
  {the number of characters on the current terminal line}
@!file_offset : 0..max_print_line;
  {the number of characters on the current file line}
@!trick_buf:array[0..error_line] of ASCII_code; {circular buffer for
@y
@!term_offset : 0..sup_max_print_line;
  {the number of characters on the current terminal line}
@!file_offset : 0..sup_max_print_line;
  {the number of characters on the current file line}
@!trick_buf:array[0..sup_error_line] of ASCII_code; {circular buffer for
@z

% _____________________________________________________________________________
%
% [5.61]
% _____________________________________________________________________________

@x
if format_ident=0 then wterm_ln(' (no format preloaded)')
else  begin slow_print(format_ident); print_ln;
  end;
@y
miktex_print_miktex_banner(term_out); {\MiKTeX: append the \MiKTeX\ version information}
if format_ident=0 then print_ln {\MiKTeX: eliminate misleading `(no format preloaded)'.}
else  begin slow_print(format_ident); print_ln;
  end;
@z

% _____________________________________________________________________________
%
% [5.71]
% _____________________________________________________________________________

@x
var k:0..buf_size; {index into |buffer|}
@y
var k:0..sup_buf_size; {index into |buffer|}
@z

% _____________________________________________________________________________
%
% [6.73]
% _____________________________________________________________________________

@x
  print_nl("! "); print(#);
@y
  if (miktex_c_style_error_messages_p and not terminal_input) then
    print_c_style_error_message (#)
  else begin print_nl("! "); print(#) end;
@z

% _____________________________________________________________________________
%
% [6.74]
% _____________________________________________________________________________

@x
@ @<Set init...@>=interaction:=error_stop_mode;
@y
@ @<Set init...@>=
if miktex_get_interaction >= 0 then
  interaction:=miktex_get_interaction
else
  interaction:=error_stop_mode;
@z

% _____________________________________________________________________________
%
% [6.81]
% _____________________________________________________________________________

@x
begin goto end_of_TEX;
@y
begin goto_end_of_TEX; {\MiKTeX: throw a \Cplusplus\ exception}
@z

% _____________________________________________________________________________
%
% [6.82]
% _____________________________________________________________________________

@x
print_char("."); show_context;
@y
if not miktex_c_style_error_messages_p then begin
  print_char("."); show_context
end;
if (miktex_halt_on_error_p) then begin
  history:=fatal_error_stop; jump_out;
end;
@z

% _____________________________________________________________________________
%
% [6.84]
% _____________________________________________________________________________

@x
line ready to be edited. But such an extension requires some system
wizardry, so the present implementation simply types out the name of the
file that should be
edited and the relevant line number.
@^system dependencies@>

There is a secret `\.D' option available when the debugging routines haven't
been commented~out.
@^debugging@>
@y
line ready to be edited.
We do this by calling the external procedure |miktex_invoke_editor|
with a pointer to
the filename, its length, and the line number.
However, here we just set up the variables that will be used as arguments,
since we don't want to do the switch-to-editor until after \TeX\ has closed
its files.
@^system dependencies@>

There is a secret `\.D' option available when the debugging routines haven't
been commented~out.
@^debugging@>
@d edit_file==input_stack[base_ptr]
@z

@x
"E": if base_ptr>0 then
  begin print_nl("You want to edit file ");
@.You want to edit file x@>
  slow_print(input_stack[base_ptr].name_field);
  print(" at line "); print_int(line);
  interaction:=scroll_mode; jump_out;
@y
"E": if base_ptr>0 then
    begin edit_name_start:=str_start(edit_file.name_field);
    edit_name_length:=str_start(edit_file.name_field+1) -
                      str_start(edit_file.name_field);
    edit_line:=line;
    jump_out;
@z

% _____________________________________________________________________________
%
% [7.109]
% _____________________________________________________________________________

@x
@!glue_ratio=real; {one-word representation of a glue expansion factor}
@y
@!glue_ratio=longreal; {one-word representation of a glue expansion factor}
@z

% _____________________________________________________________________________
%
% [8.111]
% _____________________________________________________________________________

@x
if font_max>font_base+@"10000 then bad:=16;
@y
if font_max>font_base+@"10000 then bad:=16;
@z

% _____________________________________________________________________________
%
% [9.116]
% _____________________________________________________________________________

@x
@!mem : array[mem_min..mem_max] of memory_word; {the big dynamic storage area}
@y
@!yzmem : ^memory_word; {the big dynamic storage area}
@!zmem : ^memory_word; {the big dynamic storage area}
@!mem : ^memory_word;
@z

% _____________________________________________________________________________
%
% [9.126]
% _____________________________________________________________________________

@x
begin if hi_mem_min-lo_mem_max>=1998 then t:=lo_mem_max+1000
@y
begin if hi_mem_min-lo_mem_max>=1022 then t:=lo_mem_max+512
@z

% _____________________________________________________________________________
%
% [12.186]
% _____________________________________________________________________________

@x
  if abs(mem[p+glue_offset].int)<@'4000000 then print("?.?")
  else if abs(g)>float_constant(20000) then
@y
  { The Unix |pc| folks removed this restriction with a remark that
    invalid bit patterns were vanishingly improbable, so we follow
    their example without really understanding it.
  |if abs(mem[p+glue_offset].int)<@'4000000 then print('?.?')|
  |else| }
  if fabs(g)>float_constant(20000) then
@z

% _____________________________________________________________________________
%
% [16.213]
% _____________________________________________________________________________

@x
@!nest:array[0..nest_size] of list_state_record;
@!nest_ptr:0..nest_size; {first unused location of |nest|}
@!max_nest_stack:0..nest_size; {maximum of |nest_ptr| when pushing}
@y
@!nest:array[0..sup_nest_size] of list_state_record;
@!nest_ptr:0..sup_nest_size; {first unused location of |nest|}
@!max_nest_stack:0..sup_nest_size; {maximum of |nest_ptr| when pushing}
@z

% _____________________________________________________________________________
%
% [16.215]
% _____________________________________________________________________________

@x
prev_graf:=0; shown_mode:=0;
current_sgml_tag:=0; current_sgml_attrs:=null;
dir_save:=null; dir_math_save:=false;
local_par:=null; local_par_bool:=false;
@<Start a new current page@>;
@y
prev_graf:=0; shown_mode:=0;
current_sgml_tag:=0; current_sgml_attrs:=null;
dir_save:=null; dir_math_save:=false;
local_par:=null; local_par_bool:=false;
@/{The following piece of code is a copy of module 991:}
page_contents:=empty; page_tail:=page_head; {|link(page_head):=null;|}@/
last_glue:=max_halfword; last_penalty:=0; last_kern:=0;
page_depth:=0; page_max_depth:=0;
@z

% _____________________________________________________________________________
%
% [16.220]
% _____________________________________________________________________________

@x
var p:0..nest_size; {index into |nest|}
@y
var p:0..sup_nest_size; {index into |nest|}
@z

% _____________________________________________________________________________
%
% [17.232]
% _____________________________________________________________________________

@x
@d cur_font_loc=box_base+number_regs {internal font number outside math mode}
@y
@d cur_font_loc=box_base+number_regs {internal font number outside math mode}
@z

% _____________________________________________________________________________
%
% [17.245]
% _____________________________________________________________________________

@x
begin time:=12*60; {minutes since midnight}
day:=4; {fourth day of the month}
month:=7; {seventh month of the year}
year:=1776; {Anno Domini}
@y
begin
set_new_eqtb_int(int_base + time_code,c4p_hour*60+c4p_minute);
set_new_eqtb_int(int_base + day_code,c4p_day);
set_new_eqtb_int(int_base + month_code,c4p_month);
set_new_eqtb_int(int_base + year_code,c4p_year);
@z

% _____________________________________________________________________________
%
% [19.276]
% _____________________________________________________________________________

@x
@!save_stack : array[0..save_size] of memory_word;
@!save_ptr : 0..save_size; {first unused entry on |save_stack|}
@!max_save_stack:0..save_size; {maximum usage of save stack}
@y
@!save_stack : array[0..sup_save_size] of memory_word;
@!save_ptr : 0..sup_save_size; {first unused entry on |save_stack|}
@!max_save_stack:0..sup_save_size; {maximum usage of save stack}
@z

@x
@!cur_boundary: 0..save_size; {where the current level begins}
@y
@!cur_boundary: 0..sup_save_size; {where the current level begins}
@z

% _____________________________________________________________________________
%
% [22.306]
% _____________________________________________________________________________

@x
@!input_stack : array[0..stack_size] of in_state_record;
@!input_ptr : 0..stack_size; {first unused location of |input_stack|}
@!max_in_stack: 0..stack_size; {largest value of |input_ptr| when pushing}
@y
@!input_stack : array[0..sup_stack_size] of in_state_record;
@!input_ptr : 0..sup_stack_size; {first unused location of |input_stack|}
@!max_in_stack: 0..sup_stack_size; {largest value of |input_ptr| when pushing}
@z

% _____________________________________________________________________________
%
% [22.309]
% _____________________________________________________________________________


@x
@!in_open : 0..max_in_open; {the number of lines in the buffer, less one}
@!open_parens : 0..max_in_open; {the number of open text files}
@!input_file : array[1..max_in_open] of alpha_file;
@!input_file_mode : array[1..max_in_open] of halfword;
@!input_file_translation : array[1..max_in_open] of halfword;
@!line : integer; {current line number in the current source file}
@!line_stack : array[1..max_in_open] of integer;
@y
@!in_open : 0..sup_max_in_open; {the number of lines in the buffer, less one}
@!open_parens : 0..sup_max_in_open; {the number of open text files}
@!input_file : array[1..sup_max_in_open] of alpha_file;
@!input_file_mode : array[1..sup_max_in_open] of halfword;
@!input_file_translation : array[1..sup_max_in_open] of halfword;
@!line : integer; {current line number in the current source file}
@!line_stack : array[1..sup_max_in_open] of integer;
@!source_filename_stack : array[1..sup_max_in_open] of str_number;
@!full_source_filename_stack : array[1..sup_max_in_open] of str_number;
@z

% _____________________________________________________________________________
%
% [22.313]
% _____________________________________________________________________________

@x
@!param_stack:array [0..param_size] of pointer;
  {token list pointers for parameters}
@!param_ptr:0..param_size; {first unused entry in |param_stack|}
@y
@!param_stack:array [0..sup_param_size] of pointer;
  {token list pointers for parameters}
@!param_ptr:0..sup_param_size; {first unused entry in |param_stack|}
@z

% _____________________________________________________________________________
%
% [22.315]
% _____________________________________________________________________________

@x
@!base_ptr:0..stack_size; {shallowest level shown by |show_context|}
@y
@!base_ptr:0..sup_stack_size; {shallowest level shown by |show_context|}
@z

% _____________________________________________________________________________
%
% [22.320]
% _____________________________________________________________________________

@x
@!i:0..buf_size; {index into |buffer|}
@!j:0..buf_size; {end of current line in |buffer|}
@!l:0..half_error_line; {length of descriptive information on line 1}
@!m:integer; {context information gathered for line 2}
@!n:0..error_line; {length of line 1}
@y
@!i:0..sup_buf_size; {index into |buffer|}
@!j:0..sup_buf_size; {end of current line in |buffer|}
@!l:0..sup_half_error_line; {length of descriptive information on line 1}
@!m:integer; {context information gathered for line 2}
@!n:0..sup_error_line; {length of line 1}
@z

% _____________________________________________________________________________
%
% [23.333]
% _____________________________________________________________________________

@x
line_stack[index]:=line; start:=first; state:=mid_line;
@y
source_filename_stack[index]:=0;
full_source_filename_stack[index]:=0;
line_stack[index]:=line; start:=first; state:=mid_line;
@z

% _____________________________________________________________________________
%
% [23.336]
% _____________________________________________________________________________

@x
state:=new_line; start:=1; index:=0; line:=0; name:=0;
@y
state:=new_line; start:=1; index:=0; line:=0; name:=0;
source_filename_stack[1]:=0;
full_source_filename_stack[1]:=0;
@z

% _____________________________________________________________________________
%
% [24.346]
% _____________________________________________________________________________

@x
var k:0..buf_size; {an index into |buffer|}
@y
var k:0..sup_buf_size; {an index into |buffer|}
@z

% _____________________________________________________________________________
%
% [24.368]
% _____________________________________________________________________________

@x
var k:0..buf_size; {an index into |buffer|}
@y
var k:0..sup_buf_size; {an index into |buffer|}
@z

% _____________________________________________________________________________
%
% [25.371]
% _____________________________________________________________________________

@x
@!j:0..buf_size; {index into |buffer|}
@y
@!j:0..sup_buf_size; {index into |buffer|}
@z

% _____________________________________________________________________________
%
% [26.419]
% _____________________________________________________________________________

@x
@!p:0..nest_size; {index into |nest|}
@y
@!p:0..sup_nest_size; {index into |nest|}
@z

% _____________________________________________________________________________
%
% [29.520]
% _____________________________________________________________________________

@x
@ The file names we shall deal with for illustrative purposes have the
following structure:  If the name contains `\.>' or `\.:', the file area
consists of all characters up to and including the final such character;
otherwise the file area is null.  If the remaining file name contains
`\..', the file extension consists of all such characters from the first
remaining `\..' to the end, otherwise the file extension is null.
@y
@ The file names we shall deal with have the
following structure:  If the name contains `\./' or `\.:'
(for Amiga only), the file area
consists of all characters up to and including the final such character;
otherwise the file area is null.  If the remaining file name contains
`\..', the file extension consists of all such characters from the last
`\..' to the end, otherwise the file extension is null.
@z

@x
@!area_delimiter:pool_pointer; {the most recent `\.>' or `\.:', if any}
@!ext_delimiter:pool_pointer; {the relevant `\..', if any}
@y
@!area_delimiter:pool_pointer; {the most recent `\./', if any}
@!ext_delimiter:pool_pointer; {the most recent `\..', if any}
@z

% _____________________________________________________________________________
%
% [29.521]
% _____________________________________________________________________________

@x
@d TEX_area=="TeXinputs:"
@.TeXinputs@>
@d TEX_font_area=="TeXfonts:"
@.TeXfonts@>
@d OMEGA_ocp_area=="OmegaOCPs:"
@.OmegaOCPs@>
@y
In C, the default paths are specified separately.
@z

% _____________________________________________________________________________
%
% [29.522]
% _____________________________________________________________________________

@x
begin area_delimiter:=0; ext_delimiter:=0;
@y
begin area_delimiter:=0; ext_delimiter:=0; quoted_filename:=false;
@z

% _____________________________________________________________________________
%
% [29.523]
% _____________________________________________________________________________

@x
begin if c=" " then more_name:=false
@y
begin if (c=" ") and stop_at_space and (not quoted_filename) then
  more_name:=false
else  if c="""" then begin
  quoted_filename:=not quoted_filename;
  more_name:=true;
  end
@z

@x
  if (c=">")or(c=":") then
@y
  if (c="/") then
@z

@x
  else if (c=".")and(ext_delimiter=0) then ext_delimiter:=cur_length;
@y
  else if c="." then ext_delimiter:=cur_length;
@z

% _____________________________________________________________________________
%
% [29.524]
% _____________________________________________________________________________

@x
@ The third.
@^system dependencies@>

@p procedure end_name;
@y
@ The third.
@^system dependencies@>
If a string is already in the string pool, the function
|slow_make_string| does not create a new string but returns this string
number, thus saving string space.  Because of this new property of the
returned string number it is not possible to apply |flush_string| to
these strings.

@p procedure end_name;
var temp_str: str_number; {result of file name cache lookups}
@!j,@!s,@!t: pool_pointer; {running indices}
@!must_quote:boolean; {whether we need to quote a string}
@z

@x
@:TeX capacity exceeded number of strings}{\quad number of strings@>
@y
@:TeX capacity exceeded number of strings}{\quad number of strings@>
str_room(6); {Room for quotes, if needed.}
{add quotes if needed}
if area_delimiter<>0 then begin
  {maybe quote |cur_area|}
  must_quote:=false;
  s:=str_start(str_ptr);
  t:=str_start(str_ptr)+area_delimiter;
  j:=s;
  while (not must_quote) and (j<t) do begin
    must_quote:=str_pool[j]=" "; incr(j);
    end;
  if must_quote then begin
    for j:=pool_ptr-1 downto t do str_pool[j+2]:=str_pool[j];
    str_pool[t+1]:="""";
    for j:=t-1 downto s do str_pool[j+1]:=str_pool[j];
    str_pool[s]:="""";
    if ext_delimiter<>0 then ext_delimiter:=ext_delimiter+2;
    area_delimiter:=area_delimiter+2;
    pool_ptr:=pool_ptr+2;
    end;
  end;
{maybe quote |cur_name|}
s:=str_start(str_ptr)+area_delimiter;
if ext_delimiter=0 then t:=pool_ptr else t:=str_start(str_ptr)+ext_delimiter-1;
must_quote:=false;
j:=s;
while (not must_quote) and (j<t) do begin
  must_quote:=str_pool[j]=" "; incr(j);
  end;
if must_quote then begin
  for j:=pool_ptr-1 downto t do str_pool[j+2]:=str_pool[j];
  str_pool[t+1]:="""";
  for j:=t-1 downto s do str_pool[j+1]:=str_pool[j];
  str_pool[s]:="""";
  if ext_delimiter<>0 then ext_delimiter:=ext_delimiter+2;
  pool_ptr:=pool_ptr+2;
  end;
if ext_delimiter<>0 then begin
  {maybe quote |cur_ext|}
  s:=str_start(str_ptr)+ext_delimiter-1;
  t:=pool_ptr;
  must_quote:=false;
  j:=s;
  while (not must_quote) and (j<t) do begin
    must_quote:=str_pool[j]=" "; incr(j);
    end;
  if must_quote then begin
    str_pool[t+1]:="""";
    for j:=t-1 downto s do str_pool[j+1]:=str_pool[j];
    str_pool[s]:="""";
    pool_ptr:=pool_ptr+2;
    end;
  end;
@z

@x
  str_start(str_ptr+1):=str_start(str_ptr)+area_delimiter; incr(str_ptr);
  end;
if ext_delimiter=0 then
  begin cur_ext:=""; cur_name:=make_string;
@y
  str_start(str_ptr+1):=str_start(str_ptr)+area_delimiter; incr(str_ptr);
  temp_str:=search_string(cur_area);
  if temp_str>0 then
    begin cur_area:=temp_str;
    decr(str_ptr);  {no |flush_string|, |pool_ptr| will be wrong!}
    for j:=str_start(str_ptr+1) to pool_ptr-1 do
      begin str_pool[j-area_delimiter]:=str_pool[j];
      end;
    pool_ptr:=pool_ptr-area_delimiter; {update |pool_ptr|}
    end;
  end;
if ext_delimiter=0 then
  begin cur_ext:=""; cur_name:=slow_make_string;
@z

@x
else  begin cur_name:=str_ptr;
  str_start(str_ptr+1):=str_start(str_ptr)+ext_delimiter-area_delimiter-1;
  incr(str_ptr); cur_ext:=make_string;
@y
else  begin cur_name:=str_ptr;
  str_start(str_ptr+1):=str_start(str_ptr)+ext_delimiter-area_delimiter-1;
  incr(str_ptr); cur_ext:=make_string;
  decr(str_ptr); {undo extension string to look at name part}
  temp_str:=search_string(cur_name);
  if temp_str>0 then
    begin cur_name:=temp_str;
    decr(str_ptr);  {no |flush_string|, |pool_ptr| will be wrong!}
    for j:=str_start(str_ptr+1) to pool_ptr-1 do
      begin str_pool[j-ext_delimiter+area_delimiter+1]:=str_pool[j];
      end;
    pool_ptr:=pool_ptr-ext_delimiter+area_delimiter+1;  {update |pool_ptr|}
    end;
  cur_ext:=slow_make_string;  {remake extension string}
@z

% _____________________________________________________________________________
%
% [29.525]
% _____________________________________________________________________________

@x
begin slow_print(a); slow_print(n); slow_print(e);
@y
var must_quote: boolean; {whether to quote the filename}
@!j:pool_pointer; {index into |str_pool|}
begin
must_quote:=false;
if a<>0 then begin
  j:=str_start(a);
  while (not must_quote) and (j<str_start(a+1)) do begin
    must_quote:=str_pool[j]=" "; incr(j);
  end;
end;
if n<>0 then begin
  j:=str_start(n);
  while (not must_quote) and (j<str_start(n+1)) do begin
    must_quote:=str_pool[j]=" "; incr(j);
  end;
end;
if e<>0 then begin
  j:=str_start(e);
  while (not must_quote) and (j<str_start(e+1)) do begin
    must_quote:=str_pool[j]=" "; incr(j);
  end;
end;
{FIXME: Alternative is to assume that any filename that has to be quoted has
 at least one quoted component...if we pick this, a number of insertions
 of |print_file_name| should go away.
|must_quote|:=((|a|<>0)and(|str_pool|[|str_start|[|a|]]=""""))or
              ((|n|<>0)and(|str_pool|[|str_start|[|n|]]=""""))or
              ((|e|<>0)and(|str_pool|[|str_start|[|e|]]=""""));}
if must_quote then print_char("""");
if a<>0 then
  for j:=str_start(a) to str_start(a+1)-1 do
    if so(str_pool[j])<>"""" then
      print(so(str_pool[j]));
if n<>0 then
  for j:=str_start(n) to str_start(n+1)-1 do
    if so(str_pool[j])<>"""" then
      print(so(str_pool[j]));
if e<>0 then
  for j:=str_start(e) to str_start(e+1)-1 do
    if so(str_pool[j])<>"""" then
      print(so(str_pool[j]));
if must_quote then print_char("""");
@z

% _____________________________________________________________________________
%
% [29.526]
% _____________________________________________________________________________

@x
@d append_to_name(#)==begin c:=#; incr(k);
  if k<=file_name_size then name_of_file[k]:=xchr[c];
  end
@y
@d append_to_name(#)==begin c:=#; if not (c="""") then begin incr(k);
  if k<=file_name_size then name_of_file[k]:=xchr[c];
  end end
@z

@x
for k:=name_length+1 to file_name_size do name_of_file[k]:=' ';
@y
name_of_file[ name_length + 1 ]:= chr(0); {\MiKTeX: 0-terminate the file name}
@z

% _____________________________________________________________________________
%
% [29.527]
% _____________________________________________________________________________

@x
@d format_default_length=20 {length of the |TEX_format_default| string}
@d format_area_length=11 {length of its area part}
@d format_ext_length=4 {length of its `\.{.fmt}' part}
@y
Under {\mc UNIX} we don't give the area part, instead depending
on the path searching that will happen during file opening.  Also, the
length will be set in the main program.

@d format_area_length=0 {length of its area part}
@d format_ext_length=4 {length of its `\.{.fmt}' part}
@z

% _____________________________________________________________________________
%
% [29.528]
% _____________________________________________________________________________

@x
@!TEX_format_default:packed array[1..format_default_length] of char;

@ @<Set init...@>=
TEX_format_default:='TeXformats:plain.fmt';
@y
@!format_default_length: integer;
@!TEX_format_default:packed array[1..file_name_size_plus_one] of char;

@ @<Set init...@>=
miktex_get_default_dump_file_name (TEX_format_default);
format_default_length:=c4pstrlen(TEX_format_default);
@z

% _____________________________________________________________________________
%
% [29.529]
% _____________________________________________________________________________

@x
if format_default_length>file_name_size then bad:=31;
@y
do_nothing;
@z

% _____________________________________________________________________________
%
% [29.530]
% _____________________________________________________________________________

@x
for k:=name_length+1 to file_name_size do name_of_file[k]:=' ';
@y
name_of_file[ name_length + 1 ]:= chr(0); {\MiKTeX: 0-terminate the file name}
@z

% _____________________________________________________________________________
%
% [29.531]
% _____________________________________________________________________________

@x
var j:0..buf_size; {the first space after the format file name}
@y
var j:0..sup_buf_size; {the first space after the format file name}
@z

@x
  pack_buffered_name(0,loc,j-1); {try first without the system file area}
  if w_open_in(fmt_file) then goto found;
  pack_buffered_name(format_area_length,loc,j-1);
    {now try the system format file area}
  if w_open_in(fmt_file) then goto found;
@y
  pack_buffered_name(0,loc,j-1);
  if miktex_open_format_file(fmt_file) then goto found;
@z

@x
  wterm_ln('Sorry, I can''t find that format;',' will try PLAIN.');
@y
  wterm_ln('Sorry, I can''t find that format;',' will try default.');
@z

@x
pack_buffered_name(format_default_length-format_ext_length,1,0);
if not w_open_in(fmt_file) then
@y
pack_buffered_name(format_default_length-format_ext_length,1,0);
if not miktex_open_format_file(fmt_file) then
@z

@x
  wterm_ln('I can''t find the PLAIN format file!');
@y
  wterm_ln('I can''t find the default format file!');
@z

% _____________________________________________________________________________
%
% [29.532]
% _____________________________________________________________________________

@x
else  begin for k:=1 to name_length do append_char(name_of_file[k]);
  make_name_string:=make_string;
  end;
@y
else  begin for k:=1 to name_length do append_char(name_of_file[k]);
  make_name_string:=make_string;
  end;
  {At this point we also set |cur_name|, |cur_ext|, and |cur_area| to
   match the contents of |name_of_file|.}
  k:=1;
  name_in_progress:=true;
  begin_name;
  stop_at_space:=false;
  while (k<=name_length)and(more_name(name_of_file[k])) do
    incr(k);
  stop_at_space:=true;
  end_name;
  name_in_progress:=false;
@z

% _____________________________________________________________________________
%
% [29.533]
% _____________________________________________________________________________

@x
  if not more_name(cur_chr) then goto done;
@y
  {If |cur_chr| is a space and we're not scanning a token list, check
   whether we're at the end of the buffer. Otherwise we end up adding
   spurious spaces to file names in some cases.}
  if (cur_chr=" ") and (state<>token_list) and (loc>limit) then goto done;
  if not more_name(cur_chr) then goto done;
@z

% _____________________________________________________________________________
%
% [29.537]
% _____________________________________________________________________________

@x
var k:0..buf_size; {index into |buffer|}
@y
var k:0..sup_buf_size; {index into |buffer|}
@z

@x
if e=".tex" then show_context;
@y
if (e=".tex") or (e="") then show_context;
@z

% _____________________________________________________________________________
%
% [29.541]
% _____________________________________________________________________________

@x
@!k:0..buf_size; {index into |months| and |buffer|}
@!l:0..buf_size; {end of first input line}
@y
@!k:0..sup_buf_size; {index into |months| and |buffer|}
@!l:0..sup_buf_size; {end of first input line}
@z

% _____________________________________________________________________________
%
% [29.543]
% _____________________________________________________________________________

@x
slow_print(format_ident); print_nl("");
print_int(day); print_char(" ");
months:='JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC';
@y
miktex_print_miktex_banner(log_file);
slow_print(format_ident); print_nl("");
print_int(day); print_char(" ");
c4p_arrcpy(months,'JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC');
@z

% _____________________________________________________________________________
%
% [29.544]
% _____________________________________________________________________________

@x
begin scan_file_name; {set |cur_name| to desired file name}
if cur_ext="" then cur_ext:=".tex";
@y
var temp_str: str_number;
begin scan_file_name; {set |cur_name| to desired file name}
@z

@x
  if cur_area="" then
    begin pack_file_name(cur_name,TEX_area,cur_ext);
    if a_open_in(cur_file) then goto done;
    end;
@y
@z

@x
  prompt_file_name("input file name",".tex");
@y
  prompt_file_name("input file name","");
@z

@x
done: name:=a_make_name_string(cur_file);
@y
done: name:=a_make_name_string(cur_file);
source_filename_stack[in_open]:=name;
full_source_filename_stack[in_open]:=miktex_make_full_name_string;
if name=str_ptr-1 then {we can try to conserve string pool space now}
  begin temp_str:=search_string(name);
  if temp_str>0 then
    begin name:=temp_str; flush_string;
    end;
  end;
@z

@x
if job_name=0 then
  begin job_name:=cur_name; open_log_file;
@y
if job_name=0 then
  begin job_name:=miktex_get_job_name; open_log_file;
@z

@x
if term_offset+length(name)>max_print_line-2 then print_ln
else if (term_offset>0)or(file_offset>0) then print_char(" ");
print_char("("); incr(open_parens); slow_print(name); update_terminal;
@y
if term_offset+length(full_source_filename_stack[in_open])>max_print_line-2
then print_ln
else if (term_offset>0)or(file_offset>0) then print_char(" ");
print_char("("); incr(open_parens);
print_file_name(0,full_source_filename_stack[in_open],0); update_terminal;
@z

@x
if name=str_ptr-1 then {we can conserve string pool space now}
  begin flush_string; name:=cur_name;
  end;
@y
@z

% _____________________________________________________________________________
%
% [30.570]
% _____________________________________________________________________________

@x
if aire="" then pack_file_name(nom,TEX_font_area,".ofm")
else pack_file_name(nom,aire,".ofm");
if not b_open_in(tfm_file) then abort;
@y
pack_file_name(nom,aire,"");
if not miktex_open_xfm_file(tfm_file,name_of_file) then abort;
@z

% _____________________________________________________________________________
%
% [30.576]
% _____________________________________________________________________________

@x
    fget; read_sixteen(font_info(f)(k).hh.lhfield);
@y
    fget; read_sixteen(font_info(f)(k).hh.lh);
@z

% _____________________________________________________________________________
%
% [30.605]
% _____________________________________________________________________________

@x
pack_file_name(nom,aire,".ocp");
if not b_open_in(ocp_file) then ocp_abort("opening file");
@y
pack_file_name(nom,aire,".ocp");
if not miktex_open_ocp_file(ocp_file,name_of_file) then ocp_abort("opening file");
@z

% _____________________________________________________________________________
%
% [30.610]
% _____________________________________________________________________________

@x
@!ocp_list_info:array[ocp_list_index] of memory_word;
@y
@!ocp_list_info:array[0..0] of memory_word;
@z

@x
@!ocp_lstack_info:array[ocp_lstack_index] of memory_word;
@y
@!ocp_lstack_info:array[0..0] of memory_word;
@z

% _____________________________________________________________________________
%
% [32.629]
% _____________________________________________________________________________

@x
var k:dvi_index;
begin for k:=a to b do write(dvi_file,dvi_buf[k]);
@y
begin c4p_buf_write(dvi_file,c4p_ptr(dvi_buf[a]),b-a+1);
@z

% _____________________________________________________________________________
%
% [37.806]
% _____________________________________________________________________________

 @x
if every_cr<>null then begin_token_list(every_cr,every_cr_text);
 @y
if (insert_src_special_every_cr and head<>tail) then insert_src_special;
if every_cr<>null then begin_token_list(every_cr,every_cr_text);
 @z

% _____________________________________________________________________________
%
% [37.831]
% _____________________________________________________________________________

 @x
if every_cr<>null then begin_token_list(every_cr,every_cr_text);
 @y
if (insert_src_special_every_cr) then insert_src_special;
if every_cr<>null then begin_token_list(every_cr,every_cr_text);
 @z

% _____________________________________________________________________________
%
% [40.924]
% _____________________________________________________________________________

@x
begin @!init if trie_not_ready then init_trie;@+tini@;@/
@y
begin @!Init if trie_not_ready then init_trie;@+Tini@;@/
@z

% _____________________________________________________________________________
%
% [42.954]
% _____________________________________________________________________________

@x
@!trie_pointer=0..trie_size; {an index into |trie|}
@y
@!trie_pointer=0..trie_size_def; {an index into |trie|}
@z

% _____________________________________________________________________________
%
% [42.955]
% _____________________________________________________________________________

@x
@!hyf_distance:array[1..trie_op_size] of small_number; {position |k-j| of $n_j$}
@!hyf_num:array[1..trie_op_size] of small_number; {value of $n_j$}
@!hyf_next:array[1..trie_op_size] of quarterword; {continuation code}
@!op_start:array[0..biggest_lang] of 0..trie_op_size;
           {offset for current language}
@y
@!hyf_distance:array[1..trie_op_size_def] of small_number; {position |k-j| of $n_j$}
@!hyf_num:array[1..trie_op_size_def] of small_number; {value of $n_j$}
@!hyf_next:array[1..trie_op_size_def] of quarterword; {continuation code}
@!op_start:array[0..biggest_lang] of 0..trie_op_size_def;
           {offset for current language}
@z

% _____________________________________________________________________________
%
% [43.977]
% _____________________________________________________________________________

@x
@!init@! trie_op_hash:array[-trie_op_size..trie_op_size] of 0..trie_op_size;
@y
@!init@! trie_op_hash:array[0..0] of 0..trie_op_size_def;
@z

@x
@!trie_op_lang:array[1..trie_op_size] of 0..biggest_lang;
  {language part of a hashed quadruple}
@!trie_op_val:array[1..trie_op_size] of quarterword;
  {opcode corresponding to a hashed quadruple}
@!trie_op_ptr:0..trie_op_size; {number of stored ops so far}
@y
@!trie_op_lang:array[1..trie_op_size_def] of 0..biggest_lang;
  {language part of a hashed quadruple}
@!trie_op_val:array[1..trie_op_size_def] of quarterword;
  {opcode corresponding to a hashed quadruple}
@!trie_op_ptr:0..trie_op_size_def; {number of stored ops so far}
@z

% _____________________________________________________________________________
%
% [43.978]
% _____________________________________________________________________________

@x
var h:-trie_op_size..trie_op_size; {trial hash location}
@!u:quarterword; {trial op code}
@!l:0..trie_op_size; {pointer to stored data}
@y
var h:-trie_op_size_def..trie_op_size_def; {trial hash location}
@!u:quarterword; {trial op code}
@!l:0..trie_op_size_def; {pointer to stored data}
@z

@x
loop@+  begin l:=trie_op_hash[h];
@y
loop@+  begin l:=trie_op_hash[h + trie_op_size];
@z

@x
    trie_op_lang[trie_op_ptr]:=cur_lang; trie_op_hash[h]:=trie_op_ptr;
@y
    trie_op_lang[trie_op_ptr]:=cur_lang; trie_op_hash[h + trie_op_size]:=trie_op_ptr;
@z

% _____________________________________________________________________________
%
% [43.979]
% _____________________________________________________________________________

@x
  trie_op_hash[j]:=op_start[trie_op_lang[j]]+trie_op_val[j]; {destination}
for j:=1 to trie_op_ptr do while trie_op_hash[j]>j do
  begin k:=trie_op_hash[j];@/
@y
  trie_op_hash[j + trie_op_size]:=op_start[trie_op_lang[j]]+trie_op_val[j]; {destination}
for j:=1 to trie_op_ptr do while trie_op_hash[j + trie_op_size]>j do
  begin k:=trie_op_hash[j + trie_op_size];@/
@z

@x
  trie_op_hash[j]:=trie_op_hash[k]; trie_op_hash[k]:=k;
@y
  trie_op_hash[j + trie_op_size]:=trie_op_hash[k + trie_op_size]; trie_op_hash[k + trie_op_size]:=k;
@z

% _____________________________________________________________________________
%
% [43.980]
% _____________________________________________________________________________

@x
for k:=-trie_op_size to trie_op_size do trie_op_hash[k]:=0;
@y
for k:=-trie_op_size to trie_op_size do trie_op_hash[k + trie_op_size]:=0;
@z

% _____________________________________________________________________________
%
% [43.984]
% _____________________________________________________________________________

@x
@!init@!trie_taken:packed array[1..trie_size] of boolean;
@y
@!init@!trie_taken:packed array[1..trie_size_def] of boolean;
@z

% _____________________________________________________________________________
%
% [46.1068]
% _____________________________________________________________________________

 @x
@<Append character |cur_chr|...@>=
adjust_space_factor;@/
 @y
@<Append character |cur_chr|...@>=
if (head=tail and mode>0) then begin
  if (insert_src_special_auto) then append_src_special;
end;
adjust_space_factor;@/
 @z

% _____________________________________________________________________________
%
% [47.1118]
% _____________________________________________________________________________

 @x
  if every_vbox<>null then begin_token_list(every_vbox,every_vbox_text);
 @y
  if (insert_src_special_every_vbox) then insert_src_special;
  if every_vbox<>null then begin_token_list(every_vbox,every_vbox_text);
 @z

 @x
  if every_hbox<>null then begin_token_list(every_hbox,every_hbox_text);
 @y
  if (insert_src_special_every_hbox) then insert_src_special;
  if every_hbox<>null then begin_token_list(every_hbox,every_hbox_text);
 @z

% _____________________________________________________________________________
%
% [47.1126]
% _____________________________________________________________________________

 @x
if indented then begin
  tail:=new_null_box; link(link(head)):=tail; width(tail):=par_indent;@+
  end;
 @y
if indented then begin
  tail:=new_null_box; link(link(head)):=tail; width(tail):=par_indent;
  if (insert_src_special_every_par) then insert_src_special;@+
  end;
 @z

% _____________________________________________________________________________
%
% [47.1131]
% _____________________________________________________________________________

 @x fixme: etex conflict
  else line_break(widow_penalty);
 @y
  else begin
    if (insert_src_special_every_parend) then insert_src_special;
    line_break(widow_penalty);
  end;
 @z

% _____________________________________________________________________________
%
% [48.1180]
% _____________________________________________________________________________

 @x
if every_math<>null then begin_token_list(every_math,every_math_text);
 @y
if (insert_src_special_every_math) then insert_src_special;
if every_math<>null then begin_token_list(every_math,every_math_text);
 @z

% _____________________________________________________________________________
%
% [48.1186]
% _____________________________________________________________________________

 @x
if every_display<>null then begin_token_list(every_display,every_display_text);
 @y
if (insert_src_special_every_display) then append_src_special;
if every_display<>null then begin_token_list(every_display,every_display_text);
 @z

% _____________________________________________________________________________
%
% [48.1208]
% _____________________________________________________________________________

 @x
  if every_vbox<>null then begin_token_list(every_vbox,every_vbox_text);
 @y
  if (insert_src_special_every_vbox) then insert_src_special;
  if every_vbox<>null then begin_token_list(every_vbox,every_vbox_text);
 @z

% _____________________________________________________________________________
%
% [49.1285]
% _____________________________________________________________________________

@x
var p:0..nest_size; {index into |nest|}
@y
var p:0..sup_nest_size; {index into |nest|}
@z

% _____________________________________________________________________________
%
% [49.1293]
% _____________________________________________________________________________

@x
    begin @!init new_patterns; goto done;@;@+tini@/
@y
    begin @!Init new_patterns; goto done;@;@+Tini@/
@z

% _____________________________________________________________________________
%
% [49.1301]
% _____________________________________________________________________________

@x
flushable_string:=str_ptr-1;
@y
@z

@x
    begin if cur_name=flushable_string then
      begin flush_string; cur_name:=font_name(f);
      end;
    if s>0 then
@y
    begin if s>0 then
@z

% _____________________________________________________________________________
%
% [49.1316]
% _____________________________________________________________________________

@x
  if cur_ext="" then cur_ext:=".tex";
@y
  pack_cur_name;
  if a_open_in(read_file[n]) then
     begin k:=1;
     name_in_progress:=true;
     begin_name;
     while (k<=name_length)and(more_name(name_of_file[k])) do
       incr(k);
     end_name;
     name_in_progress:=false;
     read_open[n]:=just_open;
     end;
@z

% _____________________________________________________________________________
%
% [50.1344]
% _____________________________________________________________________________

@x
@d bad_fmt=6666 {go here if the format file is unacceptable}
@y
@d bad_fmt=6666 {go here if the format file is unacceptable}
@d start_loading=7777
@z

@x
label bad_fmt,exit;
var j,@!k:integer; {all-purpose indices}
@y
label bad_fmt,exit,start_loading;
var j,@!k:integer; {all-purpose indices}
allow_makefmt:boolean;
@z

@x
begin @<Undump constants for consistency check@>;
@y
begin
  allow_makefmt := true;
start_loading:
  @<Undump constants for consistency check@>;
@z

@x
bad_fmt: wake_up_terminal;
@y
bad_fmt:
  if (allow_makefmt) then begin
    allow_makefmt := false;
    w_close (fmt_file);
    { <fixme>asserting that |name_of_file| wasn't overridden</fixme> }
    if (miktex_open_format_file(fmt_file, true)) then goto start_loading;
  end
  wake_up_terminal;
@z

% _____________________________________________________________________________
%
% [50.1346]
% _____________________________________________________________________________

@x
@d dump_wd(#)==begin fmt_file^:=#; put(fmt_file);@+end
@d dump_int(#)==begin fmt_file^.int:=#; put(fmt_file);@+end
@d dump_hh(#)==begin fmt_file^.hh:=#; put(fmt_file);@+end
@d dump_qqqq(#)==begin fmt_file^.qqqq:=#; put(fmt_file);@+end
@y
@d dump_things(#)==miktex_dump(fmt_file, #)
@d dump_wd(#)==miktex_dump(fmt_file, #)
@d dump_int(#)==miktex_dump_int(fmt_file, #)
@d dump_hh(#)==miktex_dump(fmt_file, #)
@d dump_qqqq(#)==miktex_dump(fmt_file, #)
@z

% _____________________________________________________________________________
%
% [50.1347]
% _____________________________________________________________________________

@x
@d undump_wd(#)==begin get(fmt_file); #:=fmt_file^;@+end
@d undump_int(#)==begin get(fmt_file); #:=fmt_file^.int;@+end
@d undump_hh(#)==begin get(fmt_file); #:=fmt_file^.hh;@+end
@d undump_qqqq(#)==begin get(fmt_file); #:=fmt_file^.qqqq;@+end
@y
@d undump_things(#)==miktex_undump(fmt_file, #)
@d undump_checked_things(#)==miktex_undump(fmt_file, #)
@d undump_upper_check_things(#)==miktex_undump(fmt_file, #)
@d undump_wd(#)==miktex_undump(fmt_file, #)
@d undump_int(#)==miktex_undump_int(fmt_file, #)
@d undump_hh(#)==miktex_undump(fmt_file, #)
@d undump_qqqq(#)==miktex_undump(fmt_file, #)
@z

% _____________________________________________________________________________
%
% [50.1348]
% _____________________________________________________________________________

@x
dump_int(@$);@/
@y
dump_int(@$);@/
dump_int(max_halfword);@/
@z


% _____________________________________________________________________________
%
% [50.1349]
% _____________________________________________________________________________

@x
x:=fmt_file^.int;
if x<>@$ then goto bad_fmt; {check that strings are the same}
undump_int(x);
if x<>mem_bot then goto bad_fmt;
undump_int(x);
if x<>mem_top then goto bad_fmt;
@y
undump_int(x);
if x<>@$ then goto bad_fmt; {check that strings are the same}
undump_int(x);
if x<>max_halfword then goto bad_fmt; {check |max_halfword|}
undump_int(x);
if x<>mem_bot then goto bad_fmt;
undump_int(mem_top);
if mem_bot+1100>mem_top then goto bad_fmt;

 
head:=contrib_head; tail:=contrib_head;
     page_tail:=page_head;  {page initialization}

mem_min := mem_bot - extra_mem_bot;
mem_max := mem_top + extra_mem_top;

yzmem:=miktex_reallocate(yzmem, mem_max - mem_min + 2);
zmem := yzmem - mem_min;
mem := zmem;
@z

% _____________________________________________________________________________
%
% [50.1350]
% _____________________________________________________________________________

@x
for k:=too_big_char to str_ptr do dump_int(str_start(k));
k:=0;
while k+4<pool_ptr do
  begin dump_four_ASCII; k:=k+4;
  end;
k:=pool_ptr-4; dump_four_ASCII;
@y
dump_things(str_start(too_big_char), str_ptr-too_big_char+1);
dump_things(str_pool[0], pool_ptr);
@z

% _____________________________________________________________________________
%
% [50.1351]
% _____________________________________________________________________________

@x
undump_size(0)(pool_size)('string pool size')(pool_ptr);
undump_size(0)(max_strings)('max strings')(str_ptr);
for k:=too_big_char to str_ptr do undump(0)(pool_ptr)(str_start(k));
k:=0;
while k+4<pool_ptr do
  begin undump_four_ASCII; k:=k+4;
  end;
k:=pool_ptr-4; undump_four_ASCII;
@y
undump_size(0)(sup_pool_size-pool_free)('string pool size')(pool_ptr);
if pool_size<pool_ptr+pool_free then
  pool_size:=pool_ptr+pool_free;
undump_size(0)(sup_max_strings-strings_free)('sup strings')(str_ptr);@/
if max_strings<str_ptr+strings_free then
  max_strings:=str_ptr+strings_free;
str_start_ar:=miktex_reallocate(str_start_ar, max_strings-biggest_char);
undump_things(str_start(too_big_char), str_ptr-too_big_char+1);
str_pool:=miktex_reallocate(str_pool, pool_size);
undump_things(str_pool[0], pool_ptr);
@z

% _____________________________________________________________________________
%
% [50.1352]
% _____________________________________________________________________________

@x
repeat for k:=p to q+1 do dump_wd(mem[k]);
@y
repeat dump_things(mem[p], q+2-p);
@z

@x
for k:=p to lo_mem_max do dump_wd(mem[k]);
@y
dump_things(mem[p], lo_mem_max+1-p);
@z

@x
for k:=hi_mem_min to mem_end do dump_wd(mem[k]);
@y
dump_things(mem[hi_mem_min], mem_end+1-hi_mem_min);
@z

% _____________________________________________________________________________
%
% [50.1353]
% _____________________________________________________________________________

@x
repeat for k:=p to q+1 do undump_wd(mem[k]);
@y
repeat undump_things(mem[p], q+2-p);
@z

@x
for k:=p to lo_mem_max do undump_wd(mem[k]);
@y
undump_things(mem[p], lo_mem_max+1-p);
@z

@x
for k:=hi_mem_min to mem_end do undump_wd(mem[k]);
@y
undump_things (mem[hi_mem_min], mem_end+1-hi_mem_min);
@z

% _____________________________________________________________________________
%
% [50.1354]
% _____________________________________________________________________________

@x
dump_int(par_loc); dump_int(write_loc);@/
@y
dump_int(par_loc); dump_int(write_loc); dump_int(special_loc);@/
@z

% _____________________________________________________________________________
%
% [50.1355]
% _____________________________________________________________________________

@x
undump(hash_base)(frozen_control_sequence)(write_loc);@/
@y
undump(hash_base)(frozen_control_sequence)(write_loc);@/
undump(hash_base)(frozen_control_sequence)(special_loc);
special_token:=cs_token_flag+special_loc;@/
@z

% _____________________________________________________________________________
%
% [50.1371]
% _____________________________________________________________________________

@x
@!init trie_not_ready:=false @+tini
@y
@!Init trie_not_ready:=false @+Tini
@z

% _____________________________________________________________________________
%
% [50.1373]
% _____________________________________________________________________________

@x
if (x<>69069)or eof(fmt_file) then goto bad_fmt
@y
if (x<>69069)or not eof(fmt_file) then goto bad_fmt
@z

% _____________________________________________________________________________
%
% [51.1378]
% _____________________________________________________________________________

@x
@p begin @!{|start_here|}
@y
@p begin @!{|start_here|}
c4p_begin_try_block(final_end);
c4p_begin_try_block(end_of_TEX);
miktex_on_texmf_start_job;
miktex_process_command_line_options;
miktex_allocate_memory;
@z

@x
    '---case ',bad:1);
@y
    '---case ',bad);
@z

@x [51.1332] l.24215 - INI = VIR, so pool init needs runtime test
@!init if not get_strings_started then goto final_end;
init_prim; {call |primitive| for each primitive}
init_str_ptr:=str_ptr; init_pool_ptr:=pool_ptr; fix_date_and_time;
tini@/
@y
@+init if miktex_is_init_program then
  begin if not get_strings_started then goto final_end;
  init_prim; {call |primitive| for each primitive}
  init_str_ptr:=str_ptr; init_pool_ptr:=pool_ptr; fix_date_and_time;
  end;
@+tini@/
@z

@x
ready_already:=314159;
@y
ready_already:=314159;
miktex_on_texmf_initialize;
@z

@x
end_of_TEX: close_files_and_terminate;
final_end: ready_already:=0;
@y
end_of_TEX: c4p_end_try_block(end_of_TEX); close_files_and_terminate;
final_end: c4p_end_try_block(final_end); ready_already:=0;
miktex_free_memory;
miktex_on_texmf_finish_job;
if (history = error_message_issued) or (history = fatal_error_stop) then
begin
  c4p_exit (1); {\MiKTeX: throw an exception}
end;
@z

% _____________________________________________________________________________
%
% [51.1379]
% _____________________________________________________________________________

@x
    slow_print(log_name); print_char(".");
    end;
  end;
@y
    slow_print(log_name); print_char(".");
    end;
  end;
print_ln;
if (edit_name_start<>0) and (interaction>batch_mode) then begin
  if log_opened then
    miktex_invoke_editor(edit_name_start,edit_name_length,edit_line,
                         str_start(log_name), length(log_name))
  else
    miktex_invoke_editor(edit_name_start,edit_name_length,edit_line)
end;
@z

% _____________________________________________________________________________
%
% [51.1381]
% _____________________________________________________________________________

@x
  begin @!init for c:=top_mark_code to split_bot_mark_code do
    if cur_mark[c]<>null then delete_token_ref(cur_mark[c]);
  store_fmt_file; return;@+tini@/
@y
  begin @!Init for c:=top_mark_code to split_bot_mark_code do
    if cur_mark[c]<>null then delete_token_ref(cur_mark[c]);
  store_fmt_file; return;@+Tini@/
@z

% _____________________________________________________________________________
%
% [53.1390]
% _____________________________________________________________________________

@x
primitive("special",extension,special_node);@/
@y
primitive("special",extension,special_node);@/
special_loc:=cur_val; special_token:=cs_token_flag+special_loc;@/
@z

% _____________________________________________________________________________
%
% [53.1420]
% _____________________________________________________________________________

@x
      while not a_open_out(write_file[j]) do
        prompt_file_name("output file name",".tex");
@y
      while not miktex_allow_name_of_file(false)
            or not a_open_out(write_file[j]) do
        prompt_file_name("output file name",".tex");
@z

% _____________________________________________________________________________
%
% [54.1454]
% _____________________________________________________________________________

@x
@* \[54] System-dependent changes.
@y
@* \[54/MiKTeX] System-dependent changes for MiKTeX.

@ @<Declare action procedures for use by |main_control|@>=

procedure insert_src_special;
var toklist, p, q : pointer;
begin
  if (source_filename_stack[in_open] > 0 and miktex_is_new_source (source_filename_stack[in_open], line)) then begin
    toklist := get_avail;
    p := toklist;
    info(p) := special_token;
    link(p) := get_avail; p := link(p);
    info(p) := left_brace_token+"{";
    q := str_toks (miktex_make_src_special (source_filename_stack[in_open], line));
    link(p) := link(temp_head);
    p := q;
    link(p) := get_avail; p := link(p);
    info(p) := right_brace_token+"}";
    ins_list (toklist);
    miktex_remember_source_info (source_filename_stack[in_open], line);
  end;
end;

procedure append_src_special;
var p : pointer;
begin
  if (source_filename_stack[in_open] > 0 and miktex_is_new_source (source_filename_stack[in_open], line)) then begin
    new_whatsit (special_node, write_node_size);
    write_stream(tail) := null;
    def_ref := get_avail;
    token_ref_count(def_ref) := null;
    str_toks (miktex_make_src_special (source_filename_stack[in_open], line));
    link(def_ref) := link(temp_head);
    write_tokens(tail) := def_ref;
    miktex_remember_source_info (source_filename_stack[in_open], line);
  end;
end;

@ @<Error handling procedures@>=

procedure print_c_style_error_message (@!s:str_number);
  var k : integer;
begin
  print_nl ("");
  print (full_source_filename_stack[in_open]);
  print (":");
  print_int (line);
  print (": ");
  print (s);
end;

@ @<Declare \MiKTeX\ procedures@>=

function miktex_c_style_error_messages_p : boolean; forward;@t\2@>@/
function miktex_get_interaction : integer; forward;@t\2@>@/
function miktex_get_job_name : str_number; forward;@t\2@>@/
function miktex_halt_on_error_p : boolean; forward;@t\2@>@/
function miktex_is_init_program : boolean; forward;@t\2@>@/
function miktex_make_full_name_string : str_number; forward;@t\2@>@/

@ @<Constants in the outer block@>=

@!const_font_base=font_base;

@ @<Global variables@>=

@!buf_size :integer; {maximum number of characters simultaneously present in
  current lines of open files and in control sequences between
  \.{\\csname} and \.{\\endcsname}; must not exceed |max_halfword|}
@!edit_line:integer; {what line to start editing at}
@!edit_name_length:integer;
@!edit_name_start:pool_pointer; {where the filename to switch to starts}
@!error_line :integer; {width of context lines on terminal error messages}
@!extra_mem_bot:integer; {|mem_min:=mem_bot-extra_mem_bot| except in \.{INITEX}}
@!extra_mem_top:integer; {|mem_max:=mem_top+extra_mem_top| except in \.{INITEX}}
@!half_error_line :integer; {width of first lines of contexts in te
  error messages; should be between 30 and |error_line-15|}
@!main_memory:integer; {total memory words allocated in initex}
@!max_in_open:integer; {maximum number of input files and error insertions that
  can be going on simultaneously}
@!max_print_line :integer; {width of longest text lines output; should be at least 60}
@!max_strings:integer; {maximum number of strings; must not exceed |max_halfword|}
@!mem_bot:integer;{smallest index in the |mem| array dumped by \.{INITEX};
  must not be less than |mem_min|}
@!mem_max:integer; {greatest index in \TeX's internal |mem| array;
  must be strictly less than |max_halfword|;
  must be equal to |mem_top| in \.{INITEX}, otherwise |>=mem_top|}
@!mem_min:integer; {smallest index in \TeX's internal |mem| array;
  must be |min_halfword| or more;
  must be equal to |mem_bot| in \.{INITEX}, otherwise |<=mem_bot|}
@!mem_top:integer; {largest index in the |mem| array dumped by \.{INITEX};
  must be substantially larger than |mem_bot|,
  equal to |mem_max| in \.{INITEX}, else not greater than |mem_max|}
@!nest_size:integer; {maximum number of semantic levels simultaneously active}
@!param_size:integer; {maximum number of simultaneous macro parameters}
@!pool_free:integer;{pool space free after format loaded}
@!pool_size:integer; {maximum number of characters in strings, including all
  error messages and help texts, and the names of all fonts and
  control sequences; must exceed |string_vacancies| by the total
  length of \TeX's own strings, which is currently about 23000}
@!quoted_filename:boolean;
@!save_size:integer; {space for saving values outside of current group; must be
  at most |max_halfword|}
@!stack_size:integer; {maximum number of simultaneous input sources}
@!stop_at_space:boolean; {whether |more_name| returns false for space}
@!string_vacancies:integer; {the minimum number of characters that should be
  available for the user's control sequences and font names,
  after \TeX's own error messages are stored}
@!strings_free:integer; {strings available after format loaded}

@!trie_size:integer;
@!trie_op_size:integer;

@!special_loc:pointer;
@!special_token:halfword;

@!insert_src_special_auto:boolean;
@!insert_src_special_every_par:boolean;
@!insert_src_special_every_parend:boolean;
@!insert_src_special_every_cr:boolean;
@!insert_src_special_every_math:boolean;
@!insert_src_special_every_hbox:boolean;
@!insert_src_special_every_vbox:boolean;
@!insert_src_special_every_display:boolean;
	
@!ocp_buf_size:integer;
@!ocp_stack_size:integer;

@!ocp_list_info_size:integer;
@!ocp_lstack_size:integer;
@!ocp_list_list_size:integer;

@!font_tables:array[0..0, 0..0] of memory_word;
@!font_sort_tables:array[0..0, 0..0] of memory_word;
@!ocp_tables:array[0..0, 0..0] of integer;

@!k,l:0..65535;

@ Initialize \MiKTeX\ variables.

@<Set init...@>=
edit_name_start:=0;
stop_at_space:=true;

@* \[54/MiKTeX] The string recycling routines.  \TeX{} uses 2
upto 4 {\it new\/} strings when scanning a filename in an \.{\\input},
\.{\\openin}, or \.{\\openout} operation.  These strings are normally
lost because the reference to them are not saved after finishing the
operation.  |search_string| searches through the string pool for the
given string and returns either 0 or the found string number.

@<Declare additional routines for string recycling@>=
function search_string(@!search:str_number):str_number;
label found;
var result: str_number;
@!s: str_number; {running index}
@!len: integer; {length of searched string}
begin result:=0; len:=length(search);
if len=0 then  {trivial case}
  begin result:=""; goto found;
  end
else  begin s:=search-1;  {start search with newest string below |s|; |search>1|!}
  while s>65535 do  {first 65536 strings depend on implementation!!}
    begin if length(s)=len then
      if str_eq_str(s,search) then
        begin result:=s; goto found;
        end;
    decr(s);
    end;
  end;
found:search_string:=result;
end;

@ The following routine is a variant of |make_string|.  It searches
the whole string pool for a string equal to the string currently built
and returns a found string.  Otherwise a new string is created and
returned.  Be cautious, you can not apply |flush_string| to a replaced
string!

@<Declare additional routines for string recycling@>=
function slow_make_string : str_number;
label exit;
var s: str_number; {result of |search_string|}
@!t: str_number; {new string}
begin t:=make_string; s:=search_string(t);
if s>0 then
  begin flush_string; slow_make_string:=s; return;
  end;
slow_make_string:=t;
exit:end;


@* \[54] System-dependent changes.
@z

% _____________________________________________________________________________
%
% [55.1467]
% _____________________________________________________________________________

@x
  if name_of_file then libc_free (name_of_file);
  otp_ext_str:=ocp_external(otp_input_ocp);
  otp_ext_str_arg:=ocp_external_arg(otp_input_ocp);
  name_of_file := xmalloc (4 + length(otp_ext_str) + length(otp_ext_str_arg));
@y
  otp_ext_str:=ocp_external(otp_input_ocp);
  otp_ext_str_arg:=ocp_external_arg(otp_input_ocp);
@z
