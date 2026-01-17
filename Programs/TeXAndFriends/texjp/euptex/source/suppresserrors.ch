%% suppresserrors.ch: support ``suppressing errors'' primitives in LuaTeX
%%
%% \suppresslongerror    done
%% \suppressoutererror   done
%% \suppressmathparerror done
%% The followings are not implemented to e-(u)pTeX:
%% % \suppressifcsnameerror 
%% % \suppressfontnotfounderror -> we have an error from mktextfm etc. anyway
%% % \suppressprimitiveerror -> e-(u)pTeX does not produce errors in \pdfprimitive
%%
%% \ignoreprimitiveerror (from pdfTeX and XeTeX)
%%
%% This change file should be applied after unbalanced-braces.ch.

@x
@d print_err(#)==begin if interaction=error_stop_mode then wake_up_terminal;
  if file_line_error_style_p then print_file_line
  else print_nl("! ");
  print(#);
  end
@y
@d print_err(#)==begin if interaction=error_stop_mode then wake_up_terminal;
  if file_line_error_style_p then print_file_line
  else print_nl("! ");
  print(#);
  end
@d print_ignored_err(#)==begin if interaction=error_stop_mode then
  wake_up_terminal;
  if file_line_error_style_p then print_file_line
  else print_nl("");
  print("ignored: "); print(#);
  end
@z

@x
else  begin xn_over_d:=-u; remainder:=-(v mod d);
  end;
end;
@y
else  begin xn_over_d:=-u; remainder:=-(v mod d);
  end;
end;

function is_bit_set(n: integer; s: small_number): boolean;
{check if $s$-th bit (one-based) of $n$ is set}
var m, i: integer;
begin
  m := 1;
  for i := 1 to s - 1 do
    m := m * 2;
  is_bit_set := (n div m) mod 2;
end;
@z

@x
@d eTeX_state_code=etex_int_base+10 {\eTeX\ state variables}
@y
@d suppress_long_error_code=etex_int_base+10
@d suppress_outer_error_code=etex_int_base+11
@d suppress_mathpar_error_code=etex_int_base+12
@d ignore_primitive_error_code=etex_int_base+13 {ignore some primitive/engine errors}
@d eTeX_state_code=etex_int_base+14 {\eTeX\ state variables}
@z

@x
@d read_papersize_special==int_par(read_papersize_special_code)
@y
@d read_papersize_special==int_par(read_papersize_special_code)
@d suppress_long_error==int_par(suppress_long_error_code)
@d suppress_outer_error==int_par(suppress_outer_error_code)
@d suppress_mathpar_error==int_par(suppress_mathpar_error_code)
@d ignore_primitive_error==int_par(ignore_primitive_error_code)
@z

@x @<Finish line, emit a \.{\\par}@>
if cur_cmd>=outer_call then check_outer_validity;
@y
if (suppress_outer_error=0)and(cur_cmd>=outer_call) then check_outer_validity;
@z

@x @<Process an active-character...@>
if cur_cmd>=outer_call then check_outer_validity;
@y
if (suppress_outer_error=0)and(cur_cmd>=outer_call) then check_outer_validity;
@z

@x @<Scan a control...@>
if cur_cmd>=outer_call then check_outer_validity;
@y
if (suppress_outer_error=0)and(cur_cmd>=outer_call) then check_outer_validity;
@z

@x @<Input from token list, |goto restart|  ...@>
      if cur_cmd=dont_expand then
        @<Get the next token, suppressing expansion@>
      else
        begin
        if (cur_cs=end_write)and(mode=0) then
          fatal_error("Unbalanced write command");
        check_outer_validity;
        end;
@y
      if cur_cmd=dont_expand then
        @<Get the next token, suppressing expansion@>
      else
        begin
        if (cur_cs=end_write)and(mode=0) then
          fatal_error("Unbalanced write command");
        if suppress_outer_error=0 then check_outer_validity;
        end;
@z

@x @<Read next line of file into |buffer|, ...>
  end_file_reading; {resume previous level}
  check_outer_validity; goto restart;
@y
  end_file_reading; {resume previous level}
  if suppress_outer_error=0 then check_outer_validity; goto restart;
@z

@x
if cur_tok=par_token then if long_state<>long_call then
  @<Report a runaway argument and abort@>;
@y
if cur_tok=par_token then if long_state<>long_call then
  if suppress_long_error=0 then @<Report a runaway argument and abort@>;
@z

@x
  if cur_tok=par_token then if long_state<>long_call then
    @<Report a runaway argument and abort@>;
@y
  if cur_tok=par_token then if long_state<>long_call then
    if suppress_long_error=0 then @<Report a runaway argument and abort@>;
@z

@x
    print_err("Infinite glue shrinkage found in box being split");@/
@.Infinite glue shrinkage...@>
    help4("The box you are \vsplitting contains some infinitely")@/
      ("shrinkable glue, e.g., `\vss' or `\vskip 0pt minus 1fil'.")@/
      ("Such glue doesn't belong there; but you can safely proceed,")@/
      ("since the offensive shrinkability has been made finite.");
    error; r:=new_spec(q); shrink_order(r):=normal; delete_glue_ref(q);
@y
    if is_bit_set(ignore_primitive_error, 1) then
      print_ignored_err("Infinite glue shrinkage found in box being split")
    else begin
      print_err("Infinite glue shrinkage found in box being split");@/
@.Infinite glue shrinkage...@>
      help4("The box you are \vsplitting contains some infinitely")@/
        ("shrinkable glue, e.g., `\vss' or `\vskip 0pt minus 1fil'.")@/
        ("Such glue doesn't belong there; but you can safely proceed,")@/
        ("since the offensive shrinkability has been made finite.");
      error;
    end;
    r:=new_spec(q); shrink_order(r):=normal; delete_glue_ref(q);
@z



@x
@<Math-only cases in non-math modes, or vice versa@>: insert_dollar_sign;
@y
@<Math-only cases in non-math modes, or vice versa@>: insert_dollar_sign;
mmode+par_end: if suppress_mathpar_error=0 then insert_dollar_sign;
@z

@x
mmode+endv, mmode+par_end, mmode+stop, mmode+vskip, mmode+un_vbox,
mmode+valign, mmode+hrule
@y
mmode+endv, mmode+stop, mmode+vskip, mmode+un_vbox,
mmode+valign, mmode+hrule
@z

@x after_math
@<Check that another \.\$ follows@>=
begin get_x_token;
@y
@<Check that another \.\$ follows@>=
begin repeat get_x_token;
until (suppress_mathpar_error=0)or(cur_cmd<>par_end);
@z

@x
primitive("readpapersizespecial",assign_int,int_base+read_papersize_special_code);@/
@!@:read_papersize_special_}{\.{\\readpapersizespecial} primitive@>
@y
primitive("readpapersizespecial",assign_int,int_base+read_papersize_special_code);@/
@!@:read_papersize_special_}{\.{\\readpapersizespecial} primitive@>
primitive("suppresslongerror",assign_int,int_base+suppress_long_error_code);@/
@!@:suppress_long_error_}{\.{\\suppresslongerror} primitive@>
primitive("suppressoutererror",assign_int,int_base+suppress_outer_error_code);@/
@!@:suppress_outer_error_}{\.{\\suppressoutererror} primitive@>
primitive("suppressmathparerror",assign_int,int_base+suppress_mathpar_error_code);@/
@!@:suppress_mathpar_error_}{\.{\\suppressmathparerror} primitive@>
primitive("ignoreprimitiveerror",assign_int,int_base+ignore_primitive_error_code);@/
@!@:ignore_primitive_error_}{\.{\\ignoreprimitiveerror} primitive@>
@z

@x
read_papersize_special_code:print_esc("readpapersizespecial");
@y
read_papersize_special_code:print_esc("readpapersizespecial");
suppress_long_error_code: print_esc("suppresslongerror");
suppress_outer_error_code: print_esc("suppressoutererror");
suppress_mathpar_error_code: print_esc("suppressmathparerror");
ignore_primitive_error_code:print_esc("ignoreprimitiveerror");
@z
