% $Id$
% Implementation of |\partokenname| and |\partokencontext| primitives.
% Public domain. Originally written by Petr Olsak, 2021.

% TeX inserts (let's say) "par-token" at empty lines and in other situations.
% The primitive meaning of "par-token" is (let's say) "end-paragraph",
%   but it can be changed by the macro programmer (by |\def\par|, for example).
% The name of the auto-emitted "par-token" is fixed as |\par| in
%   classical TeX. 
% The new |\partokenname| primitive defined here allows changing this cs name.
% After |\partokenname <control-sequence>| is given, the specified
%   <control-sequence> will be emitted at empty lines, etc., not |\par|. 
% Moreover, the given <control-sequence> plays the role of |\par| when
%   processing not-\long macros ("runaway error").
% The setting of |\partokenname| is global.
%
% The integer register |\partokencontext| allows controlling the places where
% "par-token" is auto-emitted. The value of |\partokencontext| can be:
%   0 ... "par-token" is emitted as in classical TeX, i.e. at empty lines,
%         before \end, \vskip, \hrule, \unvbox, and \halign (if horizontal mode
%         is current) and in several error recovery situations.
%         This is the default value.
%   1 ... "par-token" is emitted as before and at the end of \vbox,
%         \vtop, and \vcenter if horizontal mode is current here.
%   2 ... "par-token" is emitted as before and at the end of \noalign,
%         \vadjust, \output, \insert, and \valign items, if horizontal
%         mode is current here.
% Classical TeX does a direct call of the "end-paragraph" routine at the
% places mentioned in cases 1 and 2.
% 
% See the user documentation in the pdftex manual for more,
% the tests/partoken-test.tex file for basic examples,
% and the OpTeX format (eventually) for the practical case inspiring
% this idea.

% New internal constant for integer register |\partokencontext|
@x
@d web2c_int_pars=web2c_int_base+4 {total number of web2c's integer parameters}
@y
@d partoken_context_code=web2c_int_base+4 {controlling where |partoken| inserted}
@d web2c_int_pars=web2c_int_base+5 {total number of web2c's integer parameters}
@z

% Allocation of integer register |\partokencontext|
@x
@d tracing_stack_levels==int_par(tracing_stack_levels_code)
@y
@d tracing_stack_levels==int_par(tracing_stack_levels_code)
@d partoken_context==int_par(partoken_context_code)
@z

% Printing |\show\partokencontext|
@x
tracing_stack_levels_code:print_esc("tracingstacklevels");
@y
tracing_stack_levels_code:print_esc("tracingstacklevels");
partoken_context_code:print_esc("partokencontext");
@z

% Declaration of primitives |\partokenname| and |\partokencontext|
@x
  primitive("tracingstacklevels",assign_int,int_base+tracing_stack_levels_code);@/
@!@:tracing_stack_levels_}{\.{\\tracingstacklevels} primitive@>
@y
  primitive("tracingstacklevels",assign_int,int_base+tracing_stack_levels_code);@/
@!@:tracing_stack_levels_}{\.{\\tracingstacklevels} primitive@>
  primitive("partokenname",partoken_name,0);@/
@!@:partoken_name_}{\.{\\partokenname} primitive@>
  primitive("partokencontext",assign_int,int_base+partoken_context_code);@/
@!@:partoken_context_}{\.{\\partokencontext} primitive@>
@z

% Printing |\show\partokenname|
@x
vrule: print_esc("vrule");
@y
vrule: print_esc("vrule");
partoken_name: print_esc("partokenname");
@z

% End of |\vbox| and |\vtop|
@x
vbox_group: begin end_graf; package(0);
  end;
vtop_group: begin end_graf; package(vtop_code);
  end;
@y
vbox_group: if (partoken_context>0) and (mode=hmode) then 
  begin
     back_input; cur_tok := par_token; back_input; token_type := inserted;
  end
  else  begin end_graf; package(0); end;
vtop_group: if (partoken_context>0) and (mode=hmode) then
  begin
     back_input; cur_tok := par_token; back_input; token_type := inserted;
  end 
  else begin end_graf; package(vtop_code); end;
@z

% End of |\insert| or |\vadjust|
@x
insert_group: begin end_graf; q:=split_top_skip; add_glue_ref(q);
@y
insert_group: if (partoken_context>1) and (mode=hmode) then
  begin
     back_input; cur_tok := par_token; back_input; token_type := inserted;
  end 
  else begin end_graf; q:=split_top_skip; add_glue_ref(q);
@z

% End of |\output|
@x
output_group: @<Resume the page builder...@>;
@y
output_group: if (partoken_context>1) and (mode=hmode) then
  begin
     back_input; cur_tok := par_token; back_input; token_type := inserted;
  end 
  else @<Resume the page builder...@>;
@z

% End of item in |\valign|
@x
vmode+endv,hmode+endv: do_endv;
@y
vmode+endv,hmode+endv: if (partoken_context>1) and (mode=hmode) then
  begin
     back_input; cur_tok := par_token; back_input; token_type := inserted;
  end
  else do_endv;
@z

% End of |\noalign| in |\halign|
@x
no_align_group: begin end_graf; unsave; align_peek;
  end;
@y
no_align_group: if (partoken_context>1) and (mode=hmode) then
  begin
     back_input; cur_tok := par_token; back_input; token_type := inserted;
  end 
  else begin end_graf; unsave; align_peek;
  end;
@z

% End of |\vcenter|
@x
vcenter_group: begin end_graf; unsave; save_ptr:=save_ptr-2;
@y
vcenter_group: if (partoken_context>0) and (mode=hmode) then 
  begin
     back_input; cur_tok := par_token; back_input; token_type := inserted;
  end else begin end_graf; unsave; save_ptr:=save_ptr-2;
@z

% Setting new name of the |par-token|
@x
any_mode(after_group):begin get_token; save_for_after(cur_tok);
  end;
@y
any_mode(after_group):begin get_token; save_for_after(cur_tok);
  end;
any_mode(partoken_name):begin get_token;
  if cur_cs > 0 then begin
     par_loc := cur_cs; par_token := cur_tok;
     end; 
  end;
@z

% Undump |par-token| from the format
@x
undump(hash_base)(hash_top)(par_loc);
@y
undump_int(par_loc);
@z
