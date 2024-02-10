%% suppresserrors.ch: support ``suppressing errors'' primitives in LuaTeX
%%
%% \suppresslongerror    done
%% \suppressoutererror   done
%% \suppressmathparerror done
%% The followings are not implemented to e-(u)pTeX:
%% % \suppressifcsnameerror 
%% % \suppressfontnotfounderror -> we have an error from mktextfm etc. anyway
%% % \suppressprimitiveerror -> e-(u)pTeX does not produce errors in \pdfprimitive

@x
@d eTeX_state_code=etex_int_base+10 {\eTeX\ state variables}
@y
@d suppress_long_error_code=etex_int_base+10
@d suppress_outer_error_code=etex_int_base+11
@d suppress_mathpar_error_code=etex_int_base+12
@d eTeX_state_code=etex_int_base+13 {\eTeX\ state variables}
@z

@x
@d read_papersize_special==int_par(read_papersize_special_code)
@y
@d read_papersize_special==int_par(read_papersize_special_code)
@d suppress_long_error==int_par(suppress_long_error_code)
@d suppress_outer_error==int_par(suppress_outer_error_code)
@d suppress_mathpar_error==int_par(suppress_mathpar_error_code)
@z

@x {Perhaps this change hunk is not needed}
@p procedure check_outer_validity;
var p:pointer; {points to inserted token list}
@!q:pointer; {auxiliary pointer}
begin if scanner_status<>normal then
@y
@p procedure check_outer_validity;
var p:pointer; {points to inserted token list}
@!q:pointer; {auxiliary pointer}
begin if suppress_outer_error=0 then if scanner_status<>normal then
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
      else check_outer_validity;
@y
      if cur_cmd=dont_expand then
        @<Get the next token, suppressing expansion@>
      else if suppress_outer_error=0 then check_outer_validity;
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
@z

@x
read_papersize_special_code:print_esc("readpapersizespecial");
@y
read_papersize_special_code:print_esc("readpapersizespecial");
suppress_long_error_code: print_esc("suppresslongerror");
suppress_outer_error_code: print_esc("suppressoutererror");
suppress_mathpar_error_code: print_esc("suppressmathparerror");
@z
