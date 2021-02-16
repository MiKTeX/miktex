% $Id$
% Public domain. From Petr Olsak, tex-implementors 2feb2021,
% with adaptations by Akira Kakuto.
%
% If new parameter \tracingstacklevels>0, and \tracingmacros>0,
% output a prefix indicating depth of macro nesting. Also output a
% similar prefix for each \input (regardless of \tracingmacros), which
% counts as a level of input for this.
% 
% Furthermore, if macro expansion is >= the \tracingstacklevels
% value, abbreviate the output.
% 
% A simple test file is in web2c/tests; running:
%   TEXINPUTS=/.../web2c/tests pdftex -ini tracingstacklevels.tex
% should result in log output like:
%   ~.\b ->\a 
%   ~~\a 
%   ...
%   ~.INPUT tracingstacklevel2
%   ~~\b
%   ...

@x
@d web2c_int_pars=web2c_int_base+3 {total number of web2c's integer parameters}
@y
@d tracing_stack_levels_code=web2c_int_base+3 {tracing |input_stack| level if |tracingmacros| positive}
@d web2c_int_pars=web2c_int_base+4 {total number of web2c's integer parameters}
@z

@x
@d tracing_char_sub_def==int_par(tracing_char_sub_def_code)
@y
@d tracing_char_sub_def==int_par(tracing_char_sub_def_code)
@d tracing_stack_levels==int_par(tracing_stack_levels_code)
@z

@x
tracing_char_sub_def_code:print_esc("tracingcharsubdef");
@y
tracing_char_sub_def_code:print_esc("tracingcharsubdef");
tracing_stack_levels_code:print_esc("tracingstacklevels");
@z

@x
  primitive("tracingcharsubdef",assign_int,int_base+tracing_char_sub_def_code);@/
@!@:tracing_char_sub_def_}{\.{\\tracingcharsubdef} primitive@>
  end;
@y
  primitive("tracingcharsubdef",assign_int,int_base+tracing_char_sub_def_code);@/
@!@:tracing_char_sub_def_}{\.{\\tracingcharsubdef} primitive@>
  end;
  primitive("tracingstacklevels",assign_int,int_base+tracing_stack_levels_code);@/
@!@:tracing_stack_levels_}{\.{\\tracingstacklevels} primitive@>
@z

@x This is in <Tidy up the parameter...>.
incr(n);
if tracing_macros>0 then
@y
incr(n);
if tracing_macros>0 then
  if (tracing_stack_levels=0) or (input_ptr<tracing_stack_levels) then
@z

@x \tracingstacklevels for macro expansion.
begin begin_diagnostic; print_ln; print_cs(warning_index);
token_show(ref_count); end_diagnostic(false);
@y
begin begin_diagnostic;
  if tracing_stack_levels > 0 then
    if input_ptr < tracing_stack_levels then
      begin
        v := input_ptr;
        print_ln;  print_char("~");
        while v > 0 do
          begin print_char("."); decr(v);
          end;
        print_cs(warning_index); token_show(ref_count);
      end
    else begin
           print_char("~"); print_char("~"); print_cs(warning_index);
         end
  else begin print_ln; print_cs(warning_index); token_show(ref_count); end;
end_diagnostic(false);
@z

@x New variable in |start_input|.
var temp_str: str_number;
begin scan_file_name; {set |cur_name| to desired file name}
@y
var temp_str: str_number;
v: pointer;
begin scan_file_name; {set |cur_name| to desired file name}
@z

% \tracingstacklevels for \input.
% Still in |start_input|, after other changes from the general \.{tex.ch}.
@x
else if (term_offset>0)or(file_offset>0) then print_char(" ");
print_char("("); incr(open_parens);
slow_print(full_source_filename_stack[in_open]); update_terminal;
@y
else if (term_offset>0)or(file_offset>0) then print_char(" ");
print_char("("); incr(open_parens);
slow_print(full_source_filename_stack[in_open]); update_terminal;
if tracing_stack_levels > 0 then
begin
  begin_diagnostic; print_ln;
  print_char("~");
  v := input_ptr-1;
  if v < tracing_stack_levels then
    while v > 0 do begin print_char("."); decr(v); end
  else print_char("~");
  slow_print("INPUT "); slow_print(cur_name); slow_print(cur_ext); print_ln;
  end_diagnostic(false);
end;
@z
