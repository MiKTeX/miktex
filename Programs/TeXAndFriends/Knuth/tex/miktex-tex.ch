%% miktex-tex.ch
%%
%% Derived from:
%% tex.ch for C compilation with web2c, derived from various other
%% change files.  By Tim Morgan, UC Irvine ICS Department, and many
%% others.

@x
\def\PASCAL{Pascal}
@y
\def\PASCAL{Pascal}
\def\CfourP{C4P}
\def\Cplusplus{C++}
\def\MiKTeX{MiK\TeX}
@z

@x
  \def\?##1]{\hbox to 1in{\hfil##1.\ }}
  }
@y
  \def\?##1]{\hbox{Changes to ##1.\ }}
  }
\let\maybe=\iffalse
@z

% _____________________________________________________________________________
%
% [1.2]
% _____________________________________________________________________________

@x
@d banner=='This is TeX, Version 3.141592653' {printed when \TeX\ starts}
@y
@d TeX_banner_k=='This is mikTeX, Version 3.141592653' {printed when \TeX\ starts}
@d TeX_banner=='This is TeX, Version 3.141592653' {printed when \TeX\ starts}
@#
@d banner==TeX_banner
@d banner_k==TeX_banner_k
@z

% _____________________________________________________________________________
%
% [1.4]
% _____________________________________________________________________________

@x
Actually the heading shown here is not quite normal: The |program| line
does not mention any |output| file, because \ph\ would ask the \TeX\ user
to specify a file name if |output| were specified here.
@:PASCAL H}{\ph@>
@^system dependencies@>
@y
@z

@x
procedure initialize; {this procedure gets things started properly}
@y
@t\4@>@<Declare \MiKTeX\ functions@>@/
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
% [2.20]
% _____________________________________________________________________________

@x
@!xchr: array [ASCII_code] of text_char;
  {specifies conversion of output characters}
@y
xchr: array [ASCII_code] of text_char;
   { specifies conversion of output characters }
xprn: array [ASCII_code] of ASCII_code;
   { non zero iff character is printable }
@z

% _____________________________________________________________________________
%
% [2.23]
% _____________________________________________________________________________

@x [2.23]
for i:=0 to @'37 do xchr[i]:=' ';
for i:=@'177 to @'377 do xchr[i]:=' ';
@y
{Initialize |xchr| to the identity mapping.}
for i:=0 to @'37 do xchr[i]:=i;
for i:=@'177 to @'377 do xchr[i]:=i;
@z

% _____________________________________________________________________________
%
% [2.24]
% _____________________________________________________________________________

@x
codes below @'40 in case there is a coincidence.
@y
codes below @'40 in case there is a coincidence.

We use |miktex_initialize_char_tables| to initialize |xchr|, |xord| and |xprn|.
@z

@x
@<Set init...@>=
for i:=first_text_char to last_text_char do xord[chr(i)]:=invalid_code;
for i:=@'200 to @'377 do xord[xchr[i]]:=i;
for i:=0 to @'176 do xord[xchr[i]]:=i;
@y
@<Set init...@>=
miktex_initialize_char_tables;
@z

% _____________________________________________________________________________
%
% [3.27]
% _____________________________________________________________________________

@x
@ The \ph\ compiler with which the present version of \TeX\ was prepared has
extended the rules of \PASCAL\ in a very convenient way. To open file~|f|,
we can write
$$\vbox{\halign{#\hfil\qquad&#\hfil\cr
|reset(f,@t\\{name}@>,'/O')|&for input;\cr
|rewrite(f,@t\\{name}@>,'/O')|&for output.\cr}}$$
The `\\{name}' parameter, which is of type `{\bf packed array
$[\langle\\{any}\rangle]$ of \\{char}}', stands for the name of
the external file that is being opened for input or output.
Blank spaces that might appear in \\{name} are ignored.

The `\.{/O}' parameter tells the operating system not to issue its own
error messages if something goes wrong. If a file of the specified name
cannot be found, or if such a file cannot be opened for some other reason
(e.g., someone may already be trying to write the same file), we will have
|@!erstat(f)<>0| after an unsuccessful |reset| or |rewrite|.  This allows
\TeX\ to undertake appropriate corrective action.
@:PASCAL H}{\ph@>
@^system dependencies@>

\TeX's file-opening procedures return |false| if no file identified by
|name_of_file| could be opened.

@d reset_OK(#)==erstat(#)=0
@d rewrite_OK(#)==erstat(#)=0
@y
@ We use \MiKTEX\ functions to open and close files.
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
@!buffer:^ASCII_code; {lines of characters being read}
@!first:0..sup_buf_size; {the first unused position in |buffer|}
@!last:0..sup_buf_size; {end of the line just input to |buffer|}
@!max_buf_stack:0..sup_buf_size; {largest index used in |buffer|}
@z

% _____________________________________________________________________________
%
% [3.31]
% _____________________________________________________________________________

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
    buffer[last]:=xord[f^]; get(f); incr(last);
    if buffer[last-1]<>" " then last_nonblank:=last;
    end;
  last:=last_nonblank; input_ln:=true;
  end;
end;
@y
We define |input_ln| in C, for efficiency. Nevertheless we quote the module
`Report overflow of the input buffer, and abort' here in order to make
\.{WEAVE} happy, since part of that module is needed by e-TeX.

@p function input_ln(var f:alpha_file;@!bypass_eoln:boolean):boolean; forward;@t\2@>@/
@{ @<Report overflow of the input buffer, and abort@> @}
@z

% _____________________________________________________________________________
%
% [3.33]
% _____________________________________________________________________________

@x
@ Here is how to open the terminal files
in \ph. The `\.{/I}' switch suppresses the first |get|.
@:PASCAL H}{\ph@>
@^system dependencies@>

@d t_open_in==reset(term_in,'TTY:','/O/I') {open the terminal for text input}
@d t_open_out==rewrite(term_out,'TTY:','/O') {open the terminal for text output}
@y
@ Here is how to open the terminal files.

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
@:PASCAL H}{\ph@>
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
@<Report overflow of the input buffer, and abort@>=
if format_ident=0 then
  begin write_ln(term_out,'Buffer size exceeded!'); goto final_end;
@.Buffer size exceeded@>
  end
else begin cur_input.loc_field:=first; cur_input.limit_field:=last-1;
@y
Routine is implemented in C; part of module is, however, needed for e-TeX.

@<Report overflow of the input buffer, and abort@>=
  begin cur_input.loc_field:=first; cur_input.limit_field:=last-1;
@z

% _____________________________________________________________________________
%
% [3.37]
% _____________________________________________________________________________

@x
@ The following program does the required initialization
without retrieving a possible command line.
It should be clear how to modify this routine to deal with command lines,
if the system permits them.
@^system dependencies@>

@p function init_terminal:boolean; {gets the terminal input started}
label exit;
begin t_open_in;
@y
@ The following program does the required initialization.
If anything has been specified on the command line, then we
use the routine |miktex_initialize_buffer| to get the
first input line which returns with |last > first|.
@^system dependencies@>

@p function init_terminal:boolean; {gets the terminal input started}
label exit;
begin t_open_in;
if c4p_plen > 0 then
  begin miktex_initialize_buffer; loc := first;
  while (loc < last) and (buffer[loc]=' ') do incr(loc);
  if loc < last then
    begin init_terminal := true; goto exit;
    end;
  end;
@z

@x
    write(term_out,'! End of file on the terminal... why?');
@y
    write_ln(term_out,'! End of file on the terminal... why?');
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
@!term_offset : integer;
  {the number of characters on the current terminal line}
@!file_offset : integer;
  {the number of characters on the current file line}
@!trick_buf:array[0..ssup_error_line] of ASCII_code; {circular buffer for
@z

% _____________________________________________________________________________
%
% [5.61]
% _____________________________________________________________________________

@x
wterm(banner);
@y
if not miktex_is_compatible then
  wterm(banner_k)
else
  wterm(banner);
@z

@x
if format_ident=0 then wterm_ln(' (no format preloaded)')
else  begin slow_print(format_ident); print_ln;
  end;
@y
miktex_print_miktex_banner(term_out);
if format_ident=0 then wterm_ln(' (preloaded format=',TEX_format_default,')')
else  begin slow_print(format_ident); print_ln;
  end;
if miktex_write18_p then begin
  wterm(' ');
  if restrictedshell then begin
    wterm('restricted ');
  end;
  wterm_ln('\write18 enabled.');
end;
if miktex_source_specials_p then begin
  wterm_ln(' Source specials enabled.')
end;
if miktex_have_tcx_file_name then begin
  miktex_print_tcx_file_name(term_out);
  print_ln;
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

@x
if not input_ln(term_in,true) then fatal_error("End of file on the terminal!");
@y
if not input_ln(term_in,true) then begin
  limit:=0; fatal_error("End of file on the terminal!"); end;
@z

% _____________________________________________________________________________
%
% [6.73]
% _____________________________________________________________________________

@x
  print_nl("! "); print(#);
@y
  if miktex_c_style_error_messages_p then print_file_line
  else print_nl("! ");
  print(#);
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
print_char("."); show_context;
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
"E": if base_ptr>0 then if input_stack[base_ptr].name_field>=256 then
  begin print_nl("You want to edit file ");
@.You want to edit file x@>
  slow_print(input_stack[base_ptr].name_field);
  print(" at line "); print_int(line);
  interaction:=scroll_mode; jump_out;
@y
"E": if base_ptr>0 then if input_stack[base_ptr].name_field>=256 then
    begin edit_name_start:=str_start[edit_file.name_field];
    edit_name_length:=str_start[edit_file.name_field+1] -
                      str_start[edit_file.name_field];
    edit_line:=line;
    jump_out;
@z

% _____________________________________________________________________________
%
% [6.96]
% _____________________________________________________________________________

@x
@d check_interrupt==begin if interrupt<>0 then pause_for_instructions;
  end
@y
@d check_interrupt==begin
  miktex_check_memory_if_debug;
  if interrupt<>0 then pause_for_instructions;
end
@z

% _____________________________________________________________________________
%
% [7.109]
% _____________________________________________________________________________

@x
@!glue_ratio=real; {one-word representation of a glue expansion factor}
@y
@!glue_ratio=longreal; {one-word representation of a glue expansion factor;
\MiKTeX: we use |longreal| instead of |real|}
@z

% _____________________________________________________________________________
%
% [8.110]
% _____________________________________________________________________________

@x
@d min_halfword==0 {smallest allowable value in a |halfword|}
@d max_halfword==65535 {largest allowable value in a |halfword|}
@y
@d min_halfword==-@"FFFFFFF {smallest allowable value in a |halfword|}
@d max_halfword==@"FFFFFFF {largest allowable value in a |halfword|}
@d min_halfhalfword==-@"8000
@d max_halfhalfword==@"7FFF
@z

% _____________________________________________________________________________
%
% [8.111]
% _____________________________________________________________________________

@x
if (mem_min<min_halfword)or(mem_max>=max_halfword)or@|
  (mem_bot-mem_min>max_halfword+1) then bad:=14;
@y
if (mem_bot-sup_main_memory<min_halfword)or@|
  (mem_top+sup_main_memory>=max_halfword) then bad:=14;
@z

@x
if (font_base<min_quarterword)or(font_max>max_quarterword) then bad:=15;
if font_max>font_base+256 then bad:=16;
@y
if (max_font_max<min_halfword)or(max_font_max>max_halfword) then bad:=15;
if font_max>font_base+max_font_max then bad:=16;
@z

% _____________________________________________________________________________
%
% [8.112]
%
% _____________________________________________________________________________

@x
macros are simplified in the obvious way when |min_quarterword=0|.
@^inner loop@>@^system dependencies@>

@d qi(#)==#+min_quarterword
  {to put an |eight_bits| item into a quarterword}
@d qo(#)==#-min_quarterword
  {to take an |eight_bits| item out of a quarterword}
@d hi(#)==#+min_halfword
  {to put a sixteen-bit item into a halfword}
@d ho(#)==#-min_halfword
  {to take a sixteen-bit item from a halfword}
@y
macros are simplified in the obvious way when |min_quarterword=0|.
So they have been simplified here in the obvious way.
@^inner loop@>@^system dependencies@>

The \.{WEB} source for \TeX\ defines |hi(#)==#+min_halfword| which can be
simplified when |min_halfword=0|.  The \MiKTeX\ implementation of \TeX\ can use
|hi(#)==#| together with |min_halfword<0| as long as |max_halfword| is
sufficiently large.

@d qi(#)==# {to put an |eight_bits| item into a quarterword}
@d qo(#)==# {to take an |eight_bits| item from a quarterword}
@d hi(#)==# {to put a sixteen-bit item into a halfword}
@d ho(#)==# {to take a sixteen-bit item from a halfword}
@z

% _____________________________________________________________________________
%
% [8.113]
% _____________________________________________________________________________

@x
@!quarterword = min_quarterword..max_quarterword; {1/4 of a word}
@!halfword=min_halfword..max_halfword; {1/2 of a word}
@!two_choices = 1..2; {used when there are two variants in a record}
@!four_choices = 1..4; {used when there are four variants in a record}
@y
@!quarterword = min_quarterword..max_quarterword;
@!halfhalfword = min_halfhalfword..max_halfhalfword;
@!halfword = min_halfword..max_halfword;
@!two_choices = 1..2; {used when there are two variants in a record}
@!four_choices = 1..4; {used when there are four variants in a record}
@z

@x
  2: (@!b0:quarterword; @!b1:quarterword);
@y
  2: (@!b0:halfhalfword; @!b1:halfhalfword);
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
% [10.144]
% _____________________________________________________________________________

@x
@p function new_ligature(@!f,@!c:quarterword; @!q:pointer):pointer;
@y
@p function new_ligature(@!f:internal_font_number; @!c:quarterword;
                         @!q:pointer):pointer;
@z

% _____________________________________________________________________________
%
% [12.174]
% _____________________________________________________________________________

@x
        begin if (font(p)<font_base)or(font(p)>font_max) then
@y
        begin if (font(p)>font_max) then
@z

% _____________________________________________________________________________
%
% [12.176]
% _____________________________________________________________________________

@x
@p procedure print_font_and_char(@!p:integer); {prints |char_node| data}
begin if p>mem_end then print_esc("CLOBBERED.")
else  begin if (font(p)<font_base)or(font(p)>font_max) then print_char("*")
@y
@p procedure print_font_and_char(@!p:integer); {prints |char_node| data}
begin if p>mem_end then print_esc("CLOBBERED.")
else  begin if (font(p)>font_max) then print_char("*")
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
% [16.211] \[16] The semantic nest
% _____________________________________________________________________________

@x
begin if m>0 then
  case m div (max_command+1) of
  0:print("vertical");
  1:print("horizontal");
  2:print("display math");
  end
else if m=0 then print("no")
else  case (-m) div (max_command+1) of
  0:print("internal vertical");
  1:print("restricted horizontal");
  2:print("math");
  end;
print(" mode");
end;
@y
begin if m>0 then
  case m div (max_command+1) of
  0:print("vertical mode");
  1:print("horizontal mode");
  2:print("display math mode");
  end
else if m=0 then print("no mode")
else  case (-m) div (max_command+1) of
  0:print("internal vertical mode");
  1:print("restricted horizontal mode");
  2:print("math mode");
  end;
end;

procedure print_in_mode(@!m:integer); {prints the mode represented by |m|}
begin if m>0 then
  case m div (max_command+1) of
  0:print("' in vertical mode");
  1:print("' in horizontal mode");
  2:print("' in display math mode");
  end
else if m=0 then print("' in no mode")
else  case (-m) div (max_command+1) of
  0:print("' in internal vertical mode");
  1:print("' in restricted horizontal mode");
  2:print("' in math mode");
  end;
end;
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
@!nest:^list_state_record;
@!nest_ptr:0..sup_nest_size; {first unused location of |nest|}
@!max_nest_stack:0..sup_nest_size; {maximum of |nest_ptr| when pushing}
@z

% _____________________________________________________________________________
%
% [16.215]
% _____________________________________________________________________________

@x
prev_graf:=0; shown_mode:=0;
@<Start a new current page@>;
@y
prev_graf:=0; shown_mode:=0;
@/{The following piece of code is a copy of module 991:}
page_contents:=empty; page_tail:=page_head; {|link(page_head):=null;|}@/
last_glue:=max_halfword; last_penalty:=0; last_kern:=0;
page_depth:=0; page_max_depth:=0;
@z

% _____________________________________________________________________________
%
% [16.218]
% _____________________________________________________________________________

@x
var p:0..nest_size; {index into |nest|}
@y
var p:0..sup_nest_size; {index into |nest|}
@z

% _____________________________________________________________________________
%
% [17.222]
% _____________________________________________________________________________

@x
@d undefined_control_sequence=frozen_null_font+257 {dummy location}
@y
@d undefined_control_sequence=frozen_null_font+max_font_max+1 {dummy location}
@z

% _____________________________________________________________________________
%
% [17.241]
% _____________________________________________________________________________

@x
@ The following procedure, which is called just before \TeX\ initializes its
input and output, establishes the initial values of the date and time.
@^system dependencies@>
Since standard \PASCAL\ cannot provide such information, something special
is needed. The program here simply assumes that suitable values appear in
the global variables \\{sys\_time}, \\{sys\_day}, \\{sys\_month}, and
\\{sys\_year} (which are initialized to noon on 4 July 1776,
in case the implementor is careless).

@p procedure fix_date_and_time;
begin sys_time:=12*60;
sys_day:=4; sys_month:=7; sys_year:=1776;  {self-evident truths}
@y
@ The following procedure, which is called just before \TeX\ initializes
its input and output, establishes the initial values of the date and
time. It calls \CfourP\ functions.

We have to initialize the |sys_| variables because that is what gets
output on the first line of the log file. (New in 2021.)
@^system dependencies@>

@p procedure fix_date_and_time;
begin sys_time:=c4p_hour*60+c4p_minute;
sys_day:=c4p_day; sys_month:=c4p_month; sys_year:=c4p_year;
@z

% _____________________________________________________________________________
%
% [19.271]
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
% [22.301]
% _____________________________________________________________________________

@x
@!input_stack : array[0..stack_size] of in_state_record;
@!input_ptr : 0..stack_size; {first unused location of |input_stack|}
@!max_in_stack: 0..stack_size; {largest value of |input_ptr| when pushing}
@y
@!input_stack : ^in_state_record;
@!input_ptr : 0..sup_stack_size; {first unused location of |input_stack|}
@!max_in_stack: 0..sup_stack_size; {largest value of |input_ptr| when pushing}
@z

% _____________________________________________________________________________
%
% [22.304]
% _____________________________________________________________________________

@x
@!in_open : 0..max_in_open; {the number of lines in the buffer, less one}
@!open_parens : 0..max_in_open; {the number of open text files}
@!input_file : array[1..max_in_open] of alpha_file;
@!line : integer; {current line number in the current source file}
@!line_stack : array[1..max_in_open] of integer;
@y
@!in_open : 0..sup_max_in_open; {the number of lines in the buffer, less one}
@!open_parens : 0..sup_max_in_open; {the number of open text files}
@!input_file : ^alpha_file;
@!line : integer; {current line number in the current source file}
@!line_stack : ^integer;
@!source_filename_stack : ^str_number;
@!full_source_filename_stack : ^str_number;
@z

% _____________________________________________________________________________
%
% [22.306]
% _____________________________________________________________________________

@x
  begin print_nl("Runaway ");
@.Runaway...@>
  case scanner_status of
  defining: begin print("definition"); p:=def_ref;
    end;
  matching: begin print("argument"); p:=temp_head;
    end;
  aligning: begin print("preamble"); p:=hold_head;
    end;
  absorbing: begin print("text"); p:=def_ref;
    end;
  end; {there are no other cases}
@y
  begin
@.Runaway...@>
  case scanner_status of
  defining: begin print_nl("Runaway definition"); p:=def_ref;
    end;
  matching: begin print_nl("Runaway argument"); p:=temp_head;
    end;
  aligning: begin print_nl("Runaway preamble"); p:=hold_head;
    end;
  absorbing: begin print_nl("Runaway text"); p:=def_ref;
    end;
  end; {there are no other cases}
@z

% _____________________________________________________________________________
%
% [22.308]
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
% [22.310]
% _____________________________________________________________________________

@x
@!base_ptr:0..stack_size; {shallowest level shown by |show_context|}
@y
@!base_ptr:0..sup_stack_size; {shallowest level shown by |show_context|}
@z

% _____________________________________________________________________________
%
% [22.315]
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
@!l:0..ssup_error_line; {length of descriptive information on line 1}
@!m:integer; {context information gathered for line 2}
@!n:0..ssup_error_line; {length of line 1}
@z

% _____________________________________________________________________________
%
% [23.328]
% _____________________________________________________________________________

@x
incr(in_open); push_input; index:=in_open;
@y
incr(in_open); push_input; index:=in_open;
source_filename_stack[index]:=0;full_source_filename_stack[index]:=0;
@z

% _____________________________________________________________________________
%
% [23.331]
% _____________________________________________________________________________

@x
begin input_ptr:=0; max_in_stack:=0;
@y
begin input_ptr:=0; max_in_stack:=0;
source_filename_stack[0]:=0;full_source_filename_stack[0]:=0;
@z

% _____________________________________________________________________________
%
% [24.338]
% _____________________________________________________________________________

@x
print(" while scanning ");
@y
@z

% _____________________________________________________________________________
%
% [24.339]
% _____________________________________________________________________________

@x
defining:begin print("definition"); info(p):=right_brace_token+"}";
  end;
matching:begin print("use"); info(p):=par_token; long_state:=outer_call;
  end;
aligning:begin print("preamble"); info(p):=right_brace_token+"}"; q:=p;
  p:=get_avail; link(p):=q; info(p):=cs_token_flag+frozen_cr;
  align_state:=-1000000;
  end;
absorbing:begin print("text"); info(p):=right_brace_token+"}";
@y
defining:begin print(" while scanning definition"); info(p):=right_brace_token+"}";
  end;
matching:begin print(" while scanning use"); info(p):=par_token; long_state:=outer_call;
  end;
aligning:begin print(" while scanning preamble"); info(p):=right_brace_token+"}"; q:=p;
  p:=get_avail; link(p):=q; info(p):=cs_token_flag+frozen_cr;
  align_state:=-1000000;
  end;
absorbing:begin print(" while scanning text"); info(p):=right_brace_token+"}";
@z

% _____________________________________________________________________________
%
% [25.366]
% _____________________________________________________________________________

@x
@!j:0..buf_size; {index into |buffer|}
@y
@!j:0..sup_buf_size; {index into |buffer|}
@z

@x
begin cv_backup:=cur_val; cvl_backup:=cur_val_level; radix_backup:=radix;
@y
begin
incr(expand_depth_count);
if expand_depth_count>=expand_depth then overflow("expansion depth",expand_depth);
cv_backup:=cur_val; cvl_backup:=cur_val_level; radix_backup:=radix;
@z

@x
cur_order:=co_backup; link(backup_head):=backup_backup;
@y
cur_order:=co_backup; link(backup_head):=backup_backup;
decr(expand_depth_count);
@z

% _____________________________________________________________________________
%
% [26.413]
% _____________________________________________________________________________

@x
@!p:0..nest_size; {index into |nest|}
@y
@!p:0..sup_nest_size; {index into |nest|}
@z

% _____________________________________________________________________________
%
% [27.484]
% _____________________________________________________________________________

@x
else fatal_error("*** (cannot \read from terminal in nonstop modes)")
@y
else begin
  limit:=0;
  fatal_error("*** (cannot \read from terminal in nonstop modes)");
  end
@z

% _____________________________________________________________________________
%
% [28.501]
% _____________________________________________________________________________

@x
if_eof_code: begin scan_four_bit_int; b:=(read_open[cur_val]=closed);
  end;
@y
if_eof_code: begin scan_four_bit_int_or_18;
  if cur_val=18 then b:=not miktex_write18_p
  else b:=(read_open[cur_val]=closed);
  end;
@z

% _____________________________________________________________________________
%
% [29.513]
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
% [29.514]
% _____________________________________________________________________________

@x
@d TEX_area=="TeXinputs:"
@.TeXinputs@>
@d TEX_font_area=="TeXfonts:"
@.TeXfonts@>
@y
In C, the default paths are specified separately.
@z

% _____________________________________________________________________________
%
% [29.515]
% _____________________________________________________________________________

@x
begin area_delimiter:=0; ext_delimiter:=0;
@y
begin area_delimiter:=0; ext_delimiter:=0; quoted_filename:=false;
@z

% _____________________________________________________________________________
%
% [29.516]
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
  if (c="/")or(c="\") then
@z

@x
  else if (c=".")and(ext_delimiter=0) then ext_delimiter:=cur_length;
@y
  else if c="." then ext_delimiter:=cur_length;
@z

% _____________________________________________________________________________
%
% [29.517]
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
  s:=str_start[str_ptr];
  t:=str_start[str_ptr]+area_delimiter;
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
s:=str_start[str_ptr]+area_delimiter;
if ext_delimiter=0 then t:=pool_ptr else t:=str_start[str_ptr]+ext_delimiter-1;
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
  s:=str_start[str_ptr]+ext_delimiter-1;
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
  end;
if ext_delimiter=0 then
  begin cur_ext:=""; cur_name:=make_string;
@y
  temp_str:=search_string(cur_area);
  if temp_str>0 then
    begin cur_area:=temp_str;
    decr(str_ptr);  {no |flush_string|, |pool_ptr| will be wrong!}
    for j:=str_start[str_ptr+1] to pool_ptr-1 do
      begin str_pool[j-area_delimiter]:=str_pool[j];
      end;
    pool_ptr:=pool_ptr-area_delimiter; {update |pool_ptr|}
    end;
  end;
if ext_delimiter=0 then
  begin cur_ext:=""; cur_name:=slow_make_string;
@z

@x
  incr(str_ptr); cur_ext:=make_string;
@y
  incr(str_ptr); cur_ext:=make_string;
  decr(str_ptr); {undo extension string to look at name part}
  temp_str:=search_string(cur_name);
  if temp_str>0 then
    begin cur_name:=temp_str;
    decr(str_ptr);  {no |flush_string|, |pool_ptr| will be wrong!}
    for j:=str_start[str_ptr+1] to pool_ptr-1 do
      begin str_pool[j-ext_delimiter+area_delimiter+1]:=str_pool[j];
      end;
    pool_ptr:=pool_ptr-ext_delimiter+area_delimiter+1;  {update |pool_ptr|}
    end;
  cur_ext:=slow_make_string;  {remake extension string}
@z

% _____________________________________________________________________________
%
% [29.518]
% _____________________________________________________________________________

@x
some operating systems put the file area last instead of first.)
@^system dependencies@>
@y
some operating systems put the file area last instead of first.)
@^system dependencies@>

@d check_quoted(#) == {check if string |#| needs quoting}
if #<>0 then begin
  j:=str_start[#];
  while (not must_quote) and (j<str_start[#+1]) do begin
    must_quote:=str_pool[j]=" "; incr(j);
  end;
end
@#
@d print_quoted(#) == {print string |#|, omitting quotes}
if #<>0 then
  for j:=str_start[#] to str_start[#+1]-1 do
    if so(str_pool[j])<>"""" then
      print(so(str_pool[j]))
@z

@x
begin slow_print(a); slow_print(n); slow_print(e);
@y
var must_quote: boolean; {whether to quote the filename}
@!j:pool_pointer; {index into |str_pool|}
begin
must_quote:=false;
check_quoted(a); check_quoted(n); check_quoted(e);
{FIXME: Alternative is to assume that any filename that has to be quoted has
 at least one quoted component...if we pick this, a number of insertions
 of |print_file_name| should go away.
|must_quote|:=((|a|<>0)and(|str_pool|[|str_start|[|a|]]=""""))or
              ((|n|<>0)and(|str_pool|[|str_start|[|n|]]=""""))or
              ((|e|<>0)and(|str_pool|[|str_start|[|e|]]=""""));}
if must_quote then print_char("""");
print_quoted(a); print_quoted(n); print_quoted(e);
if must_quote then print_char("""");
@z

% _____________________________________________________________________________
%
% [29.519]
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
begin k:=0;
@y
begin k:=0;
miktex_reallocate_name_of_file(length(a) + length(n) + length(e) + 1);
@z

@x
for k:=name_length+1 to file_name_size do name_of_file[k]:=' ';
@y
name_of_file[name_length + 1]:= chr(0); {\MiKTeX: 0-terminate the file name}
@z

% _____________________________________________________________________________
%
% [29.520]
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
% [29.521]
% _____________________________________________________________________________

@x
@!TEX_format_default:packed array[1..format_default_length] of char;

@ @<Set init...@>=
TEX_format_default:='TeXformats:plain.fmt';
@y
@!format_default_length: integer;
@!TEX_format_default:packed array[1..260] of char; {FIXME}

@ @<Set init...@>=
miktex_get_default_dump_file_name(TEX_format_default);
format_default_length:=c4pstrlen(TEX_format_default);
@z

% _____________________________________________________________________________
%
% [29.522]
% _____________________________________________________________________________

@x
if format_default_length>file_name_size then bad:=31;
@y
do_nothing;
@z

% _____________________________________________________________________________
%
% [29.523]
% _____________________________________________________________________________

@x
for j:=1 to n do append_to_name(xord[TEX_format_default[j]]);
@y
miktex_reallocate_name_of_file(n + (b - a + 1) + format_ext_length + 1);
for j:=1 to n do append_to_name(xord[TEX_format_default[j]]);
@z

@x
for k:=name_length+1 to file_name_size do name_of_file[k]:=' ';
@y
name_of_file[name_length + 1]:= chr(0); {\MiKTeX: 0-terminate the file name}
@z

% _____________________________________________________________________________
%
% [29.524]
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
% [29.525]
% _____________________________________________________________________________

@x
begin if (pool_ptr+name_length>pool_size)or(str_ptr=max_strings)or
@y
save_area_delimiter, save_ext_delimiter: pool_pointer;
save_name_in_progress, save_stop_at_space: boolean;
begin if (pool_ptr+name_length>pool_size)or(str_ptr=max_strings)or
@z

@x
  make_name_string:=make_string;
@y
  make_name_string:=make_string;
  {At this point we also set |cur_name|, |cur_ext|, and |cur_area| to
   match the contents of |name_of_file|.}
  save_area_delimiter:=area_delimiter; save_ext_delimiter:=ext_delimiter;
  save_name_in_progress:=name_in_progress; save_stop_at_space:=stop_at_space;
  name_in_progress:=true;
  begin_name;
  stop_at_space:=false;
  k:=1;
  while (k<=name_length)and(more_name(name_of_file[k])) do
    incr(k);
  stop_at_space:=save_stop_at_space;
  end_name;
  name_in_progress:=save_name_in_progress;
  area_delimiter:=save_area_delimiter; ext_delimiter:=save_ext_delimiter;
@z

% _____________________________________________________________________________
%
% [29.526]
% _____________________________________________________________________________

@x
@p procedure scan_file_name;
label done;
@y
@p @t\4@>@<Define procedure |scan_file_name_braced|@>@/
procedure scan_file_name;
label done;
var
  @!save_warning_index: pointer;
begin
  save_warning_index := warning_index;
  warning_index := cur_cs; {store |cur_cs| here to remember until later}
  @<Get the next non-blank non-relax non-call...@>; {here the program expands
    tokens and removes spaces and \.{\\relax}es from the input. The \.{\\relax}
    removal follows LuaTeX''s implementation, and other cases of
    balanced text scanning.}
  back_input; {return the last token to be read by either code path}
  if cur_cmd=left_brace then
    scan_file_name_braced
  else
@z

@x
  if not more_name(cur_chr) then goto done;
@y
  {If |cur_chr| is a space and we're not scanning a token list, check
   whether we're at the end of the buffer. Otherwise we end up adding
   spurious spaces to file names in some cases.}
  if (cur_chr=" ") and (state<>token_list) and (loc>limit) then goto done;
  if not more_name(cur_chr) then goto done;
@z

@x
done: end_name; name_in_progress:=false;
end;
@y
  end;
done: end_name; name_in_progress:=false;
warning_index := save_warning_index; {restore |warning_index|}
end;
@z

% _____________________________________________________________________________
%
% [29.530]
% _____________________________________________________________________________

@x
var k:0..buf_size; {index into |buffer|}
@y
var k:0..sup_buf_size; {index into |buffer|}
@!saved_cur_name:str_number; {to catch empty terminal input}
@!saved_cur_ext:str_number; {to catch empty terminal input}
@!saved_cur_area:str_number; {to catch empty terminal input}
@z

@x
if e=".tex" then show_context;
@y
if (e=".tex") or (e="") then show_context;
print_ln; print("(Press Enter to retry, or Control-C to exit");
if (e<>"") then
  begin
    print("; default file extension is `"); print(e); print("'");
  end;
print(")"); print_ln;
@z

@x
clear_terminal; prompt_input(": "); @<Scan file name in the buffer@>;
if cur_ext="" then cur_ext:=e;
@y
saved_cur_name:=cur_name;
saved_cur_ext:=cur_ext;
saved_cur_area:=cur_area;
clear_terminal; prompt_input(": "); @<Scan file name in the buffer@>;
if (length(cur_name)=0) and (cur_ext="") and (cur_area="") then
  begin
    cur_name:=saved_cur_name;
    cur_ext:=saved_cur_ext;
    cur_area:=saved_cur_area;
  end
else
  if cur_ext="" then cur_ext:=e;
@z

% _____________________________________________________________________________
%
% [29.532]
% _____________________________________________________________________________

@x
  while not b_open_out(dvi_file) do
@y
  while (not miktex_open_dvi_file (dvi_file)) do
@z

% _____________________________________________________________________________
%
% [29.534]
% _____________________________________________________________________________

@x
@!k:0..buf_size; {index into |months| and |buffer|}
@!l:0..buf_size; {end of first input line}
@y
@!k:0..sup_buf_size; {index into |months| and |buffer|}
@!l:0..sup_buf_size; {end of first input line}
@z

@x
if job_name=0 then job_name:="texput";
@y
if job_name=0 then job_name:=miktex_get_job_name("texput");
@z

% _____________________________________________________________________________
%
% [29.536]
% _____________________________________________________________________________

@x
begin wlog(banner);
@y
begin
if not miktex_is_compatible
then
  wlog(banner_k)
else
  wlog(banner);
@z

@x
slow_print(format_ident); print("  ");
print_int(sys_day); print_char(" ");
months:='JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC';
@y
miktex_print_miktex_banner(log_file);
slow_print(format_ident); print("  ");
print_int(sys_day); print_char(" ");
c4p_arrcpy(months,'JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC');
@z

@x
end
@y
if miktex_write18_p then begin
  wlog_cr;
  wlog(' ');
  if restrictedshell then begin
    wlog('restricted ');
  end;
  wlog('\write18 enabled.')
  end;
if miktex_source_specials_p then begin
  wlog_cr;
  wlog(' Source specials enabled.')
  end;
if miktex_c_style_error_messages_p then begin
  wlog_cr;
  wlog(' file:line:error style messages enabled.')
  end;
if miktex_parse_first_line_p then begin
  wlog_cr;
  wlog(' %&-line parsing enabled.');
  end;
if miktex_have_tcx_file_name then begin
  miktex_print_tcx_file_name(log_file);
  end;
end
@z

% _____________________________________________________________________________
%
% [29.537]
% _____________________________________________________________________________

@x
begin scan_file_name; {set |cur_name| to desired file name}
if cur_ext="" then cur_ext:=".tex";
pack_cur_name;
loop@+  begin begin_file_reading; {set up |cur_file| and new level of input}
  if a_open_in(cur_file) then goto done;
  if cur_area="" then
    begin pack_file_name(cur_name,TEX_area,cur_ext);
    if a_open_in(cur_file) then goto done;
    end;
@y
var temp_str: str_number;
begin scan_file_name; {set |cur_name| to desired file name}
pack_cur_name;
loop@+begin
  begin_file_reading; {set up |cur_file| and new level of input}
  if a_open_in(cur_file) then
    goto done;
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
  begin job_name:=cur_name; open_log_file;
@y
  begin job_name:=miktex_get_job_name(cur_name); open_log_file;
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
slow_print(full_source_filename_stack[in_open]); update_terminal;
@z

@x
if name=str_ptr-1 then {conserve string pool space (but see note above)}
  begin flush_string; name:=cur_name;
  end;
@y
@z

% _____________________________________________________________________________
%
% [30.548]
% _____________________________________________________________________________

@x
@!internal_font_number=font_base..font_max; {|font| in a |char_node|}
@!font_index=0..font_mem_size; {index into |font_info|}
@y
@!internal_font_number=font_base..max_font_max; {|font| in a |char_node|}
@!font_index=0..sup_font_mem_size; {index into |font_info|}
@!nine_bits=min_quarterword..non_char;
@z

% _____________________________________________________________________________
%
% [30.549]
% _____________________________________________________________________________

@x
@!font_bc:array[internal_font_number] of eight_bits;
  {beginning (smallest) character code}
@!font_ec:array[internal_font_number] of eight_bits;
  {ending (largest) character code}
@y
@!font_bc: ^eight_bits;
  {beginning (smallest) character code}
@!font_ec: ^eight_bits;
  {ending (largest) character code}
@z

@x
@!font_false_bchar:array[internal_font_number] of min_quarterword..non_char;
  {|font_bchar| if it doesn't exist in the font, otherwise |non_char|}
@y
@!font_false_bchar: ^nine_bits;
  {|font_bchar| if it doesn't exist in the font, otherwise |non_char|}
@z

% _____________________________________________________________________________
%
% [30.551]
% _____________________________________________________________________________

@x
for k:=font_base to font_max do font_used[k]:=false;
@y
@z

% _____________________________________________________________________________
%
% [30.552]
% _____________________________________________________________________________

@x
font_ptr:=null_font; fmem_ptr:=7;
font_name[null_font]:="nullfont"; font_area[null_font]:="";
hyphen_char[null_font]:="-"; skew_char[null_font]:=-1;
bchar_label[null_font]:=non_address;
font_bchar[null_font]:=non_char; font_false_bchar[null_font]:=non_char;
font_bc[null_font]:=1; font_ec[null_font]:=0;
font_size[null_font]:=0; font_dsize[null_font]:=0;
char_base[null_font]:=0; width_base[null_font]:=0;
height_base[null_font]:=0; depth_base[null_font]:=0;
italic_base[null_font]:=0; lig_kern_base[null_font]:=0;
kern_base[null_font]:=0; exten_base[null_font]:=0;
font_glue[null_font]:=null; font_params[null_font]:=7;
param_base[null_font]:=-1;
for k:=0 to 6 do font_info[k].sc:=0;
@y
@z

% _____________________________________________________________________________
%
% [30.560]
% _____________________________________________________________________________

@x
@!file_opened:boolean; {was |tfm_file| successfully opened?}
@y
@!name_too_long:boolean; {|nom| or |aire| exceeds 255 bytes?}
@!file_opened:boolean; {was |tfm_file| successfully opened?}
@z

% _____________________________________________________________________________
%
% [30.561]
% _____________________________________________________________________________

@x
else print(" not loadable: Metric (TFM) file not found");
@y
else if name_too_long then print(" not loadable: Metric (TFM) file name too long")
else print(" not loadable: Metric (TFM) file not found");
@z

% _____________________________________________________________________________
%
% [30.563]
% _____________________________________________________________________________

@x
if aire="" then pack_file_name(nom,TEX_font_area,".tfm")
else pack_file_name(nom,aire,".tfm");
@y
name_too_long:=(length(nom)>255)or(length(aire)>255);
if name_too_long then abort;
pack_file_name(nom,aire,"");
@z

@x
if not b_open_in(tfm_file) then abort;
@y
if not miktex_open_tfm_file(tfm_file,name_of_file) then abort;
@z

% _____________________________________________________________________________
%
% [32.592]
% _____________________________________________________________________________

@x
@!c,@!f:quarterword; {character and font in current |char_node|}
@y
 {character and font in current |char_node|}
@!c:quarterword;
@!f:internal_font_number;
@z

% _____________________________________________________________________________
%
% [32.594]
% _____________________________________________________________________________

@x
@!dvi_index=0..dvi_buf_size; {an index into the output buffer}
@y
@!dvi_index=0..sup_dvi_buf_size; {an index into the output buffer}
@z

% _____________________________________________________________________________
%
% [32.597]
% _____________________________________________________________________________

@x
var k:dvi_index;
begin for k:=a to b do write(dvi_file,dvi_buf[k]);
@y
begin c4p_buf_write(dvi_file,c4p_ptr(dvi_buf[a]),b-a+1);
@z

% _____________________________________________________________________________
%
% [32.601]
% _____________________________________________________________________________

@x
each time, we use the macro |dvi_out|.
@y
each time, we use the macro |dvi_out|.

The length of |dvi_file| should not exceed |@"7FFFFFFF|; we set |cur_s:=-2|
to prevent further \.{DVI} output causing infinite recursion.
@z

@x
begin if dvi_limit=dvi_buf_size then
@y
begin if dvi_ptr>(@"7FFFFFFF-dvi_offset) then
  begin cur_s:=-2;
  fatal_error("dvi length exceeds ""7FFFFFFF");
@.dvi length exceeds...@>
  end;
if dvi_limit=dvi_buf_size then
@z

% _____________________________________________________________________________
%
% [32.602]
% _____________________________________________________________________________

@x
if dvi_ptr>0 then write_dvi(0,dvi_ptr-1)
@y
if dvi_ptr>(@"7FFFFFFF-dvi_offset) then
  begin cur_s:=-2;
  fatal_error("dvi length exceeds ""7FFFFFFF");
@.dvi length exceeds...@>
  end;
if dvi_ptr>0 then write_dvi(0,dvi_ptr-1)
@z

@x
begin dvi_out(fnt_def1);
dvi_out(f-font_base-1);@/
@y
begin if f<=256+font_base then
  begin dvi_out(fnt_def1);
  dvi_out(f-font_base-1);
  end
else begin dvi_out(fnt_def1+1);
  dvi_out((f-font_base-1) div @'400);
  dvi_out((f-font_base-1) mod @'400);
  end;
@z

% _____________________________________________________________________________
%
% [32.617]
%
% TODO: set output comment
% _____________________________________________________________________________

% _____________________________________________________________________________
%
% [32.621]
% _____________________________________________________________________________

@x
else  begin dvi_out(fnt1); dvi_out(f-font_base-1);
  end;
@y
else if(f<=256+font_base) then
  begin dvi_out (fnt1); dvi_out(f-font_base-1);
  end
else begin dvi_out(fnt1+1);
  dvi_out((f-font_base-1) div @'400);
  dvi_out((f-font_base-1) mod @'400);
  end;
@z

% _____________________________________________________________________________
%
% [32.642]
% _____________________________________________________________________________

@x
else  begin dvi_out(post); {beginning of the postamble}
@y
else if cur_s<>-2 then
  begin dvi_out(post); {beginning of the postamble}
@z

@x
  print_nl("Output written on "); slow_print(output_file_name);
@y
  print_nl("Output written on "); print_file_name(0, output_file_name, 0);
@z

@x
  print(" ("); print_int(total_pages); print(" page");
  if total_pages<>1 then print_char("s");
@y
  print(" ("); print_int(total_pages);
  if total_pages<>1 then print(" pages")
  else print(" page");
@z

@x
  b_close(dvi_file);
@y
  miktex_close_dvi_file(dvi_file);
@z

% _____________________________________________________________________________
%
% [40.891]
%
% FIXME: is this necessary?
% _____________________________________________________________________________

@x
begin @!init if trie_not_ready then init_trie;@+tini@;@/
@y
begin @!Init if trie_not_ready then init_trie;@+Tini@;@/
@z

% _____________________________________________________________________________
%
% [46.1049]
% _____________________________________________________________________________

@x
print("' in "); print_mode(mode);
@y
print_in_mode(mode);
@z

% _____________________________________________________________________________
%
% [49.1244]
% _____________________________________________________________________________

@x
var p:0..nest_size; {index into |nest|}
@y
var p:0..sup_nest_size; {index into |nest|}
@z

% _____________________________________________________________________________
%
% [49.1252]
% _____________________________________________________________________________

@x
    begin @!init new_patterns; goto done;@;@+tini@/
@y
    begin @!Init new_patterns; goto done;@;@+Tini@/
@z

% _____________________________________________________________________________
%
% [49.1257]
% _____________________________________________________________________________

@x
@!flushable_string:str_number; {string not yet referenced}
@y
@z

% _____________________________________________________________________________
%
% [49.1260]
% _____________________________________________________________________________

@x
flushable_string:=str_ptr-1;
@y
@z

@x
    begin if cur_name=flushable_string then
      begin flush_string; cur_name:=font_name[f];
      end;
    if s>0 then
@y
    begin if s>0 then
@z

% _____________________________________________________________________________
%
% [49.1275]
% _____________________________________________________________________________

@x
  if cur_ext="" then cur_ext:=".tex";
  pack_cur_name;
  if a_open_in(read_file[n]) then read_open[n]:=just_open;
@y
  pack_cur_name;
  if a_open_in(read_file[n]) then
    read_open[n]:=just_open;
@z

% _____________________________________________________________________________
%
% [50.1301]
% _____________________________________________________________________________

@x
format_ident:=" (INITEX)";
@y
if miktex_is_init_program then format_ident:=" (INITEX)";
@z

% _____________________________________________________________________________
%
% [50.1302]
% _____________________________________________________________________________

@x
@!w: four_quarters; {four ASCII codes}
@y
@!format_engine: ^text_char;
@z

% _____________________________________________________________________________
%
% [50.1303]
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
  if allow_makefmt then begin
    allow_makefmt := false;
    w_close(fmt_file);
    if miktex_open_format_file(fmt_file, true) then goto start_loading;
  end
  wake_up_terminal;
@z

% _____________________________________________________________________________
%
% [50.1305]
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
% [50.1306]
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

@x
@d undump_size_end_end(#)==too_small(#)@+else undump_end_end
@y
@d format_debug_end(#)==
    write_ln (c4p_error_output, ' = ', #);
  end;
@d format_debug(#)==
  if debug_format_file then begin
    write (c4p_error_output, 'fmtdebug:', #);
    format_debug_end
@d undump_size_end_end(#)==
  too_small(#)@+else format_debug (#)(x); undump_end_end
@z

% _____________________________________________________________________________
%
% [50.1307]
%
% TODO: Dump engine name.
% _____________________________________________________________________________

@x
dump_int(@$);@/
@y
dump_int(@"4D694B54);  {\MiKTeX's magic constant: "TKiM"}
dump_int(@$);@/
@<Dump |xord|, |xchr|, and |xprn|@>;
dump_int(max_halfword);@/
@z

% _____________________________________________________________________________
%
% [50.1308]
%
% TODO: Undump/check engine name.
% _____________________________________________________________________________


@x
x:=fmt_file^.int;
if x<>@$ then goto bad_fmt; {check that strings are the same}
@y
undump_int(x);
format_debug('format magic number')(x);
if x<>@"4D694B54 then goto bad_fmt; {not a format file}
undump_int(x);
format_debug('string pool checksum')(x);
if x<>@$ then begin {check that strings are the same}
  wake_up_terminal;
  wterm_ln('---! ', name_of_file,
           ' made by different executable version, strings are different');
  goto bad_fmt;
end;
@<Undump |xord|, |xchr|, and |xprn|@>;
undump_int(x);
if x<>max_halfword then goto bad_fmt; {check |max_halfword|}
@z

@x
undump_int(x);
if x<>mem_bot then goto bad_fmt;
undump_int(x);
if x<>mem_top then goto bad_fmt;
@y
undump_int(x); format_debug ('mem_bot')(x);
if x<>mem_bot then goto bad_fmt;
undump_int(mem_top); format_debug ('mem_top')(mem_top);
if mem_bot+1100>mem_top then goto bad_fmt;


head:=contrib_head; tail:=contrib_head;
     page_tail:=page_head;  {page initialization}

mem_min := mem_bot - extra_mem_bot;
mem_max := mem_top + extra_mem_top;

yzmem:=miktex_reallocate(yzmem, mem_max - mem_min + 1);
zmem := yzmem - mem_min;
mem := zmem;
@z

% _____________________________________________________________________________
%
% [50.1309]
% _____________________________________________________________________________

@x
for k:=0 to str_ptr do dump_int(str_start[k]);
k:=0;
while k+4<pool_ptr do
  begin dump_four_ASCII; k:=k+4;
  end;
k:=pool_ptr-4; dump_four_ASCII;
@y
dump_things(str_start[0], str_ptr+1);
dump_things(str_pool[0], pool_ptr);
@z

% _____________________________________________________________________________
%
% [50.1310]
% _____________________________________________________________________________

@x
undump_size(0)(pool_size)('string pool size')(pool_ptr);
undump_size(0)(max_strings)('max strings')(str_ptr);
for k:=0 to str_ptr do undump(0)(pool_ptr)(str_start[k]);
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
str_start:=miktex_reallocate(str_start, max_strings);
undump_checked_things(0, pool_ptr, str_start[0], str_ptr+1);@/
str_pool:=miktex_reallocate(str_pool, pool_size);
undump_things(str_pool[0], pool_ptr);
@z

% _____________________________________________________________________________
%
% [50.1311]
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
% [50.1312]
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
% [50.1320]
% _____________________________________________________________________________

@x
for k:=0 to fmem_ptr-1 do dump_wd(font_info[k]);
dump_int(font_ptr);
for k:=null_font to font_ptr do
  @<Dump the array info for internal font number |k|@>;
@y
dump_things(font_info[0], fmem_ptr);
dump_int(font_ptr);
@<Dump the array info for internal font number |k|@>;
@z

@x
print_int(font_ptr-font_base); print(" preloaded font");
if font_ptr<>font_base+1 then print_char("s")
@y
print_int(font_ptr-font_base);
if font_ptr<>font_base+1 then print(" preloaded fonts")
else print(" preloaded font")
@z

% _____________________________________________________________________________
%
% [50.1321]
% _____________________________________________________________________________

@x
undump_size(7)(font_mem_size)('font mem size')(fmem_ptr);
for k:=0 to fmem_ptr-1 do undump_wd(font_info[k]);
undump_size(font_base)(font_max)('font max')(font_ptr);
for k:=null_font to font_ptr do
  @<Undump the array info for internal font number |k|@>
@y
undump_size(7)(sup_font_mem_size)('font mem size')(fmem_ptr);
if fmem_ptr>font_mem_size then font_mem_size:=fmem_ptr;
font_info:=miktex_reallocate(font_info, font_mem_size);
undump_things(font_info[0], fmem_ptr);@/
undump_size(font_base)(font_base+max_font_max)('font max')(font_ptr);
{This undumps all of the font info, despite the name.}
@<Undump the array info for internal font number |k|@>;
@z

% _____________________________________________________________________________
%
% [50.1322]
% _____________________________________________________________________________

@x
@ @<Dump the array info for internal font number |k|@>=
begin dump_qqqq(font_check[k]);
dump_int(font_size[k]);
dump_int(font_dsize[k]);
dump_int(font_params[k]);@/
dump_int(hyphen_char[k]);
dump_int(skew_char[k]);@/
dump_int(font_name[k]);
dump_int(font_area[k]);@/
dump_int(font_bc[k]);
dump_int(font_ec[k]);@/
dump_int(char_base[k]);
dump_int(width_base[k]);
dump_int(height_base[k]);@/
dump_int(depth_base[k]);
dump_int(italic_base[k]);
dump_int(lig_kern_base[k]);@/
dump_int(kern_base[k]);
dump_int(exten_base[k]);
dump_int(param_base[k]);@/
dump_int(font_glue[k]);@/
dump_int(bchar_label[k]);
dump_int(font_bchar[k]);
dump_int(font_false_bchar[k]);@/
print_nl("\font"); print_esc(font_id_text(k)); print_char("=");
print_file_name(font_name[k],font_area[k],"");
if font_size[k]<>font_dsize[k] then
  begin print(" at "); print_scaled(font_size[k]); print("pt");
  end;
end
@y
@ @<Dump the array info for internal font number |k|@>=
begin
dump_things(font_check[null_font], font_ptr+1-null_font);
dump_things(font_size[null_font], font_ptr+1-null_font);
dump_things(font_dsize[null_font], font_ptr+1-null_font);
dump_things(font_params[null_font], font_ptr+1-null_font);
dump_things(hyphen_char[null_font], font_ptr+1-null_font);
dump_things(skew_char[null_font], font_ptr+1-null_font);
dump_things(font_name[null_font], font_ptr+1-null_font);
dump_things(font_area[null_font], font_ptr+1-null_font);
dump_things(font_bc[null_font], font_ptr+1-null_font);
dump_things(font_ec[null_font], font_ptr+1-null_font);
dump_things(char_base[null_font], font_ptr+1-null_font);
dump_things(width_base[null_font], font_ptr+1-null_font);
dump_things(height_base[null_font], font_ptr+1-null_font);
dump_things(depth_base[null_font], font_ptr+1-null_font);
dump_things(italic_base[null_font], font_ptr+1-null_font);
dump_things(lig_kern_base[null_font], font_ptr+1-null_font);
dump_things(kern_base[null_font], font_ptr+1-null_font);
dump_things(exten_base[null_font], font_ptr+1-null_font);
dump_things(param_base[null_font], font_ptr+1-null_font);
dump_things(font_glue[null_font], font_ptr+1-null_font);
dump_things(bchar_label[null_font], font_ptr+1-null_font);
dump_things(font_bchar[null_font], font_ptr+1-null_font);
dump_things(font_false_bchar[null_font], font_ptr+1-null_font);
for k:=null_font to font_ptr do
  begin print_nl("\font"); print_esc(font_id_text(k)); print_char("=");
  print_file_name(font_name[k],font_area[k],"");
  if font_size[k]<>font_dsize[k] then
    begin print(" at "); print_scaled(font_size[k]); print("pt");
    end;
  end;
end
@z

% _____________________________________________________________________________
%
% [50.1323]
% _____________________________________________________________________________

@x
@ @<Undump the array info for internal font number |k|@>=
begin undump_qqqq(font_check[k]);@/
undump_int(font_size[k]);
undump_int(font_dsize[k]);
undump(min_halfword)(max_halfword)(font_params[k]);@/
undump_int(hyphen_char[k]);
undump_int(skew_char[k]);@/
undump(0)(str_ptr)(font_name[k]);
undump(0)(str_ptr)(font_area[k]);@/
undump(0)(255)(font_bc[k]);
undump(0)(255)(font_ec[k]);@/
undump_int(char_base[k]);
undump_int(width_base[k]);
undump_int(height_base[k]);@/
undump_int(depth_base[k]);
undump_int(italic_base[k]);
undump_int(lig_kern_base[k]);@/
undump_int(kern_base[k]);
undump_int(exten_base[k]);
undump_int(param_base[k]);@/
undump(min_halfword)(lo_mem_max)(font_glue[k]);@/
undump(0)(fmem_ptr-1)(bchar_label[k]);
undump(min_quarterword)(non_char)(font_bchar[k]);
undump(min_quarterword)(non_char)(font_false_bchar[k]);
end
@y
@ This module should now be named `Undump all the font arrays'.

@<Undump the array info for internal font number |k|@>=
begin {Allocate the font arrays}
undump_things(font_check[null_font], font_ptr+1-null_font);
undump_things(font_size[null_font], font_ptr+1-null_font);
undump_things(font_dsize[null_font], font_ptr+1-null_font);
undump_checked_things(min_halfword, max_halfword,
                      font_params[null_font], font_ptr+1-null_font);
undump_things(hyphen_char[null_font], font_ptr+1-null_font);
undump_things(skew_char[null_font], font_ptr+1-null_font);
undump_upper_check_things(str_ptr, font_name[null_font], font_ptr+1-null_font);
undump_upper_check_things(str_ptr, font_area[null_font], font_ptr+1-null_font);
{There's no point in checking these values against the range $[0,255]$,
 since the data type is |unsigned char|, and all values of that type are
 in that range by definition.}
undump_things(font_bc[null_font], font_ptr+1-null_font);
undump_things(font_ec[null_font], font_ptr+1-null_font);
undump_things(char_base[null_font], font_ptr+1-null_font);
undump_things(width_base[null_font], font_ptr+1-null_font);
undump_things(height_base[null_font], font_ptr+1-null_font);
undump_things(depth_base[null_font], font_ptr+1-null_font);
undump_things(italic_base[null_font], font_ptr+1-null_font);
undump_things(lig_kern_base[null_font], font_ptr+1-null_font);
undump_things(kern_base[null_font], font_ptr+1-null_font);
undump_things(exten_base[null_font], font_ptr+1-null_font);
undump_things(param_base[null_font], font_ptr+1-null_font);
undump_checked_things(min_halfword, lo_mem_max,
                     font_glue[null_font], font_ptr+1-null_font);
undump_checked_things(0, fmem_ptr-1,
                     bchar_label[null_font], font_ptr+1-null_font);
undump_checked_things(min_quarterword, non_char,
                     font_bchar[null_font], font_ptr+1-null_font);
undump_checked_things(min_quarterword, non_char,
                     font_false_bchar[null_font], font_ptr+1-null_font);
end
@z

% _____________________________________________________________________________
%
% [50.1325]
% _____________________________________________________________________________

@x
@!init trie_not_ready:=false @+tini
@y
@!Init trie_not_ready:=false @+Tini
@z

% _____________________________________________________________________________
%
% [50.1326]
% _____________________________________________________________________________

@x
dump_int(interaction); dump_int(format_ident); dump_int(69069);
@y
if miktex_get_interaction >= 0 then dump_int(error_stop_mode)
else dump_int(interaction);
dump_int(format_ident); dump_int(69069);
@z

% _____________________________________________________________________________
%
% [50.1327]
% _____________________________________________________________________________

@x
undump(batch_mode)(error_stop_mode)(interaction);
@y
undump(batch_mode)(error_stop_mode)(interaction);
if miktex_get_interaction>=0 then interaction:=miktex_get_interaction;
@z

@x
if (x<>69069)or eof(fmt_file) then goto bad_fmt
@y
if x<>69069 then goto bad_fmt
@z

% _____________________________________________________________________________
%
% [51.1332]
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
if error_line > ssup_error_line then error_line := ssup_error_line;
@z

@x
    '---case ',bad:1);
@y
    '---case ',bad);
@z

@x
@!init if not get_strings_started then goto final_end;
init_prim; {call |primitive| for each primitive}
init_str_ptr:=str_ptr; init_pool_ptr:=pool_ptr; fix_date_and_time;
tini@/
@y
@!Init if not get_strings_started then goto final_end;
init_prim; {call |primitive| for each primitive}
init_str_ptr:=str_ptr; init_pool_ptr:=pool_ptr; fix_date_and_time;
Tini@/
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
% [51.1333]
% _____________________________________________________________________________

@x
    slow_print(log_name); print_char(".");
    end;
  end;
@y
    print_file_name(0, log_name, 0); print_char(".");
    end;
  end;
print_ln;
if (edit_name_start<>0) and (interaction>batch_mode) then begin
  if log_opened then
    miktex_invoke_editor(edit_name_start,edit_name_length,edit_line,
                         str_start[log_name], length(log_name))
  else
    miktex_invoke_editor(edit_name_start,edit_name_length,edit_line)
end;
@z

% _____________________________________________________________________________
%
% [51.1335]
% _____________________________________________________________________________

@x
  begin @!init for c:=top_mark_code to split_bot_mark_code do
@y
  begin @!Init for c:=top_mark_code to split_bot_mark_code do
@z

    if cur_mark[c]<>null then delete_token_ref(cur_mark[c]);
@x
  store_fmt_file; return;@+tini@/
@y
  store_fmt_file; return;@+Tini@/
@z

% _____________________________________________________________________________
%
% [51.1337]
% _____________________________________________________________________________

@x
fix_date_and_time;@/
@y
fix_date_and_time;@/

@!init
if trie_not_ready then begin {initex without format loaded}
  trie_root:=0; trie_c[0]:=si(0); trie_ptr:=0;

  font_ptr:=null_font; fmem_ptr:=7;
  font_name[null_font]:="nullfont"; font_area[null_font]:="";
  hyphen_char[null_font]:="-"; skew_char[null_font]:=-1;
  bchar_label[null_font]:=non_address;
  font_bchar[null_font]:=non_char; font_false_bchar[null_font]:=non_char;
  font_bc[null_font]:=1; font_ec[null_font]:=0;
  font_size[null_font]:=0; font_dsize[null_font]:=0;
  char_base[null_font]:=0; width_base[null_font]:=0;
  height_base[null_font]:=0; depth_base[null_font]:=0;
  italic_base[null_font]:=0; lig_kern_base[null_font]:=0;
  kern_base[null_font]:=0; exten_base[null_font]:=0;
  font_glue[null_font]:=null; font_params[null_font]:=7;
  param_base[null_font]:=-1;
  for font_k:=0 to 6 do font_info[font_k].sc:=0;
  end;
  tini@/

  for font_k:=font_base to font_max do font_used[font_k]:=false;
@z

@x
@<Initialize the print |selector|...@>;
@y
if miktex_get_interaction >= 0 then
  interaction:=miktex_get_interaction;
@<Initialize the print |selector|...@>;
@z

% _____________________________________________________________________________
%
% [52.1339]
% _____________________________________________________________________________

@x
5: print_word(font_info[n]);
@y 24397
5: begin print_scaled(font_info[n].sc); print_char(" ");@/
  print_int(font_info[n].qqqq.b0); print_char(":");@/
  print_int(font_info[n].qqqq.b1); print_char(":");@/
  print_int(font_info[n].qqqq.b2); print_char(":");@/
  print_int(font_info[n].qqqq.b3);
  end;
@z

% _____________________________________________________________________________
%
% [51.1348]
% _____________________________________________________________________________

@x
var i,@!j,@!k:integer; {all-purpose integers}
@!p,@!q,@!r:pointer; {all-purpose pointers}
@y
var k:integer; {all-purpose integers}
@!p:pointer; {all-purpose pointers}
@z

% _____________________________________________________________________________
%
% [54.1373]
% _____________________________________________________________________________

@x
procedure out_what(@!p:pointer);
var j:small_number; {write stream number}
@y
procedure out_what(@!p:pointer);
var j:small_number; {write stream number}
    @!old_setting:0..max_selector;
@z

% _____________________________________________________________________________
%
% [54.1374]
% _____________________________________________________________________________

@x
  else  begin if write_open[j] then a_close(write_file[j]);
    if subtype(p)=close_node then write_open[j]:=false
@y
  else  begin if write_open[j] then begin a_close(write_file[j]);
                                          write_open[j]:=false; end;
    if subtype(p)=close_node then do_nothing {already closed}
@z

@x
      while not a_open_out(write_file[j]) do
        prompt_file_name("output file name",".tex");
      write_open[j]:=true;
@y
      while not miktex_allow_name_of_file(false)
            or not a_open_out(write_file[j]) do
        prompt_file_name("output file name",".tex");
      write_open[j]:=true;
      {If on first line of input, log file is not ready yet, so don't log.}
      if log_opened then begin
        old_setting:=selector;
        if (tracing_online<=0) then
          selector:=log_only  {Show what we're doing in the log file.}
        else selector:=term_and_log;  {Show what we're doing.}
        print_nl("\openout");
        print_int(j);
        print(" = `");
        print_file_name(cur_name,cur_area,cur_ext);
        print("'."); print_nl(""); print_ln;
        selector:=old_setting;
      end;
@z

% _____________________________________________________________________________
%
% [54.1379] \[54] System-dependent changes
% _____________________________________________________________________________

@x
@* \[54/ML\TeX] System-dependent changes for ML\TeX.
@y
@* \[54/miktex] System-dependent changes for \MiKTeX.
Here are extra variables for \MiKTeX.  (This numbering of the
system-dependent section allows easy integration of \MiKTeX\ and e-\TeX, etc.)
@^system dependencies@>

@<Glob...@>=
@!edit_name_start: pool_pointer; {where the filename to switch to starts}
@!edit_name_length,@!edit_line: integer; {what line to start editing at}
@!stop_at_space: boolean; {whether |more_name| returns false for space}

@ The |edit_name_start| will be set to point into |str_pool| somewhere after
its beginning if \TeX\ is supposed to switch to an editor on exit.

@<Set init...@>=
edit_name_start:=0;
stop_at_space:=true;

@ These are used when we regenerate the representation of the first 256
strings.

@<Global...@> =
@!save_str_ptr: str_number;
@!save_pool_ptr: pool_pointer;
@!output_comment: ^char;
@!k,l: 0..255; {used by `Make the first 256 strings', etc.}

@ When debugging a macro package, it can be useful to see the exact
control sequence names in the format file.  For example, if ten new
csnames appear, it's nice to know what they are, to help pinpoint where
they came from.  (This isn't a truly ``basic'' printing procedure, but
that's a convenient module in which to put it.)

@<Basic printing procedures@> =
procedure print_csnames (hstart:integer; hfinish:integer);
var c,h:integer;
begin
  write_ln(c4p_error_output, 'fmtdebug:csnames from ', hstart, ' to ', hfinish, ':');
  for h := hstart to hfinish do begin
    if text(h) > 0 then begin {if have anything at this position}
      for c := str_start[text(h)] to str_start[text(h) + 1] - 1
      do begin
        write(str_pool[c], c4p_error_output); {print the characters}
      end;
      write_ln(c4p_error_output, '|');
    end;
  end;
end;

@ Are we printing extra info as we read the format file?

@<Glob...@> =
@!debug_format_file: boolean;


@ A helper for printing file:line:error style messages.  Look for a
filename in |full_source_filename_stack|, and if we fail to find
one fall back on the non-file:line:error style.

@<Basic print...@>=
procedure print_file_line;
var level: 0..sup_max_in_open;
begin
  level:=in_open;
  while (level>0) and (full_source_filename_stack[level]=0) do
    decr(level);
  if level=0 then
    print_nl("! ")
  else begin
    print_nl (""); print (full_source_filename_stack[level]); print (":");
    if level=in_open then print_int (line)
    else print_int (line_stack[level+1]);
    print (": ");
  end;
end;

@ To be able to determine whether \.{\\write18} is enabled from within
\TeX\ we also implement \.{\\eof18}.  We sort of cheat by having an
additional route |scan_four_bit_int_or_18| which is the same as
|scan_four_bit_int| except it also accepts the value 18.

@<Declare procedures that scan restricted classes of integers@>=
procedure scan_four_bit_int_or_18;
begin scan_int;
if (cur_val<0)or((cur_val>15)and(cur_val<>18)) then
  begin print_err("Bad number");
@.Bad number@>
  help2("Since I expected to read a number between 0 and 15,")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;

@ Dumping the |xord|, |xchr|, and |xprn| arrays.  We dump these always
in the format, so a TCX file loaded during format creation can set a
default for users of the format.

@<Dump |xord|, |xchr|, and |xprn|@>=
dump_things(xord[0], 256);
dump_things(xchr[0], 256);
dump_things(xprn[0], 256);

@ Undumping the |xord|, |xchr|, and |xprn| arrays.  This code is more
complicated, because we want to ensure that a TCX file specified on
the command line will override whatever is in the format.  Since the
tcx file has already been loaded, that implies throwing away the data
in the format.  Also, if no |translate_filename| is given, but
|eight_bit_p| is set we have to make all characters printable.

@<Undump |xord|, |xchr|, and |xprn|@>=
if miktex_have_tcx_file_name then begin
  for k:=0 to 255 do undump_things(dummy_xord, 1);
  for k:=0 to 255 do undump_things(dummy_xchr, 1);
  for k:=0 to 255 do undump_things(dummy_xprn, 1);
  end
else begin
  undump_things(xord[0], 256);
  undump_things(xchr[0], 256);
  undump_things(xprn[0], 256);
  if miktex_enable_eightbit_chars_p then
    for k:=0 to 255 do
      xprn[k]:=1;
end;


@* \[54/miktex-string] The string recycling routines.
\TeX{} uses 2 upto 4 {\it new\/} strings when scanning a filename in an
\.{\\input}, \.{\\openin}, or \.{\\openout} operation.  These strings are
normally lost because the reference to them are not saved after finishing
the operation.  |search_string| searches through the string pool for the
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
  while s>255 do  {first 256 strings depend on implementation!!}
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


@* \[54/miktex] More changes for \MiKTeX.
% Related to [25.366] expansion depth check
Sometimes, recursive calls to the |expand| routine may
cause exhaustion of the run-time calling stack, resulting in
forced execution stops by the operating system. To diminish the chance
of this happening, a counter is used to keep track of the recursion
depth, in conjunction with a constant called |expand_depth|.

This does not catch all possible infinite recursion loops, just the ones
that exhaust the application calling stack. The actual maximum value of
|expand_depth| is outside of our control, but the initial setting of
|10000| should be enough to prevent problems.
@^system dependencies@>

@<Global...@>=
expand_depth_count:integer;

@ @<Set init...@>=
expand_depth_count:=0;

@ % Related to [29.526] expansion depth check
When |scan_file_name| starts it looks for a |left_brace|
(skipping \.{\\relax}es, as other \.{\\toks}-like primitives).
If a |left_brace| is found, then the procedure scans a file
name contained in a balanced token list, expanding tokens as
it goes. When the scanner finds the balanced token list, it
is converted into a string and fed character-by-character to
|more_name| to do its job the same as in the ``normal'' file
name scanning.

@ @<Define procedure |scan_file_name_braced|@>=
procedure scan_file_name_braced;
var
  @!save_scanner_status: small_number; {|scanner_status| upon entry}
  @!save_def_ref: pointer; {|def_ref| upon entry, important if inside `\.{\\message}}
  @!save_cur_cs: pointer;
  @!s: str_number; {temp string}
  @!p: pointer; {temp pointer}
  @!i: integer; {loop tally}
  @!save_stop_at_space: boolean; {this should be in tex.ch}
  @!dummy: boolean;
    {Initializing}
begin save_scanner_status := scanner_status; {|scan_toks| sets |scanner_status| to |absorbing|}
  save_def_ref := def_ref; {|scan_toks| uses |def_ref| to point to the token list just read}
  save_cur_cs := cur_cs; {we set |cur_cs| back a few tokens to use in runaway errors}
    {Scanning a token list}
  cur_cs := warning_index; {for possible runaway error}
  {mimick |call_func| from pdfTeX}
  if scan_toks(false, true) <> 0 then do_nothing; {actually do the scanning}
  {|s := tokens_to_string(def_ref);|}
  old_setting := selector; selector:=new_string;
  show_token_list(link(def_ref),null,pool_size-pool_ptr);
  selector := old_setting;
  s := make_string;
  {turns the token list read in a string to input}
    {Restoring some variables}
  delete_token_ref(def_ref); {remove the token list from memory}
  def_ref := save_def_ref; {and restore |def_ref|}
  cur_cs := save_cur_cs; {restore |cur_cs|}
  scanner_status := save_scanner_status; {restore |scanner_status|}
    {Passing the read string to the input machinery}
  save_stop_at_space := stop_at_space; {save |stop_at_space|}
  stop_at_space := false; {set |stop_at_space| to false to allow spaces in file names}
  begin_name;
  for i:=str_start[s] to str_start[s+1]-1 do
    dummy := more_name(str_pool[i]); {add each read character to the current file name}
  stop_at_space := save_stop_at_space; {restore |stop_at_space|}
end;


@* \[54/miktex] Even more changes for \MiKTeX.

@ @<Declare \MiKTeX\ functions@>=
function@?miktex_c_style_error_messages_p : boolean; forward;@t\2@>@/
function@?miktex_enable_eightbit_chars_p : boolean; forward;@t\2@>@/
function@?miktex_get_interaction : integer; forward;@t\2@>@/
function@?miktex_get_quiet_flag : boolean; forward;@t\2@>@/
function@?miktex_halt_on_error_p : boolean; forward;@t\2@>@/
function@?miktex_have_tcx_file_name : boolean; forward;@t\2@>@/
function@?miktex_is_compatible : boolean; forward;@t\2@>@/
function@?miktex_is_init_program : boolean; forward;@t\2@>@/
function@?miktex_make_full_name_string : str_number; forward;@t\2@>@/
function@?miktex_parse_first_line_p : boolean; forward;@t\2@>@/
function@?miktex_source_specials_p : boolean; forward;@t\2@>@/
function@?miktex_write18_p : boolean; forward;@t\2@>@/

@ @<Constants in the outer block@>=
@!const_font_base=font_base;

@ @<Global variables@>=
@!hyph_size : integer; {maximum number of hyphen exceptions}
@!trie_size : integer; {space for hyphenation patterns; should be larger for
  \.{INITEX} than it is in production versions of \TeX}
@!buf_size:integer; {maximum number of characters simultaneously present in
  current lines of open files and in control sequences between
  \.{\\csname} and \.{\\endcsname}; must not exceed |max_halfword|}
@!error_line:integer; {width of context lines on terminal error messages}
@!extra_mem_bot:integer; {|mem_min:=mem_bot-extra_mem_bot| except in \.{INITEX}}
@!extra_mem_top:integer; {|mem_max:=mem_top+extra_mem_top| except in \.{INITEX}}
@!font_k:integer; {loop variable for initialization}
@!font_max:integer; {maximum internal font number; must not exceed |max_quarterword|
  and must be at most |font_base+256|}
@!font_mem_size:integer; {number of words of |font_info| for all fonts}
@!half_error_line:integer; {width of first lines of contexts in te
  error messages; should be between 30 and |error_line-15|}
@!max_in_open:integer; {maximum number of input files and error insertions that
  can be going on simultaneously}
@!main_memory:integer; {total memory words allocated in initex}
@!max_print_line:integer; {width of longest text lines output; should be at least 60}
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
@!strings_free:integer; {strings available after format loaded}
@!string_vacancies:integer; {the minimum number of characters that should be
  available for the user's control sequences and font names,
  after \TeX's own error messages are stored}
@!expand_depth:integer; {limits recursive calls to the |expand| procedure}


@* \[54/ML\TeX] System-dependent changes for ML\TeX.
@z
