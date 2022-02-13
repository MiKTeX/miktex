% $Id$
% Implementation of |\showstream|.
% Public domain. Originally written by Marcel Kr\"uger, 2021.

Since we redirect content which is normally written to the terminal/log
to a write stream, print_nl will sometimes get called while selector is a write
stream. Therefore we adapt print_nl to handle that case properly.
@x
procedure print_nl(@!s:str_number); {prints string |s| at beginning of line}
begin if ((term_offset>0)and(odd(selector)))or@|
  ((file_offset>0)and(selector>=log_only)) then print_ln;
print(s);
end;
@y
procedure print_nl(@!s:str_number); {prints string |s| at beginning of line}
begin if (selector<no_print)or((term_offset>0)and(odd(selector)))or@|
  ((file_offset>0)and(selector>=log_only)) then print_ln;
print(s);
end;
@z

Add a integer parameter
@x
@d web2c_int_pars=web2c_int_base+5 {total number of web2c's integer parameters}
@#
@y
@d show_stream_code=web2c_int_base+5 {stream to output xray commands to}
@d web2c_int_pars=web2c_int_base+6 {total number of web2c's integer parameters}
@#
@z

@x
@d partoken_context==int_par(partoken_context_code)
@y
@d partoken_context==int_par(partoken_context_code)
@d show_stream==int_par(show_stream_code)
@z

@x
partoken_context_code:print_esc("partokencontext");
@y
partoken_context_code:print_esc("partokencontext");
show_stream_code:print_esc("showstream");
@z

@x
  primitive("partokencontext",assign_int,int_base+partoken_context_code);@/
@!@:partoken_context_}{\.{\\partokencontext} primitive@>
@y
  primitive("partokencontext",assign_int,int_base+partoken_context_code);@/
@!@:partoken_context_}{\.{\\partokencontext} primitive@>
primitive("showstream",assign_int,int_base+show_stream_code);@/
@!@:show_stream_}{\.{\\showstream} primitive@>
@z

Then assign a default value since it's not the normal 0.
@x Part of the integer section of @<Initialize table entries...@>=, the integer section
del_code("."):=0; {this null delimiter is used in error recovery}
@y
del_code("."):=0; {this null delimiter is used in error recovery}
show_stream:=-1;
@z

Then do the actual change: In all cases of |show_whatever| we adjust the selector
if |show_stream| is set. We don't need any changes
to |begin/end_diagnostic| since they don't do anything interesting if
|selector| isn't |term_and_log|.

For |show_lists_code| this is done directly in |show_whatever|:
@x
show_lists_code: begin begin_diagnostic; show_activities;
  end;
@y
show_lists_code:
  begin @<Adjust |selector| based on |show_stream|@>
  begin_diagnostic; show_activities;
  end;
@z

The ending gets skipped if we changed the selector,
but we have to reset the |selector| based on the current interaction setting.
@x
@<Complete a potentially long \.{\\show} command@>;
common_ending: if interaction<error_stop_mode then
  begin help0; decr(error_count);
  end
else if tracing_online>0 then
  begin@t@>@;@/
  help3("This isn't an error message; I'm just \showing something.")@/
  ("Type `I\show...' to show more (e.g., \show\cs,")@/
  ("\showthe\count10, \showbox255, \showlists).");
  end
else  begin@t@>@;@/
  help5("This isn't an error message; I'm just \showing something.")@/
  ("Type `I\show...' to show more (e.g., \show\cs,")@/
  ("\showthe\count10, \showbox255, \showlists).")@/
  ("And type `I\tracingonline=1\show...' to show boxes and")@/
  ("lists on your terminal as well as in the transcript file.");
  end;
error;
end;
@y
@<Complete a potentially long \.{\\show} command@>;
common_ending: if selector<no_print then
  begin print_ln;
  @<Initialize the print |selector| based on |interaction|@>;
  if log_opened then selector:=selector+2;
  end
else begin if interaction<error_stop_mode then
    begin help0; decr(error_count);
    end
  else if tracing_online>0 then
    begin@t@>@;@/
    help3("This isn't an error message; I'm just \showing something.")@/
    ("Type `I\show...' to show more (e.g., \show\cs,")@/
    ("\showthe\count10, \showbox255, \showlists).");
    end
  else  begin@t@>@;@/
    help5("This isn't an error message; I'm just \showing something.")@/
    ("Type `I\show...' to show more (e.g., \show\cs,")@/
    ("\showthe\count10, \showbox255, \showlists).")@/
    ("And type `I\tracingonline=1\show...' to show boxes and")@/
    ("lists on your terminal as well as in the transcript file.");
    end;
  error;
  end;
end;
@z

While the other cases have separate blocks
@x
@ @<Show the current meaning of a token...@>=
begin get_token;
@y
@ @<Adjust |selector| based on |show_stream|@>=
if (show_stream>=0) and (show_stream<no_print) and write_open[show_stream] then
  selector:=show_stream;

@ @<Show the current meaning of a token...@>=
begin get_token;
  @<Adjust |selector| based on |show_stream|@>
@z

@x
@ @<Show the current contents of a box@>=
begin scan_register_num; fetch_box(p); begin_diagnostic;
@y
@ @<Show the current contents of a box@>=
begin scan_register_num; fetch_box(p);
@<Adjust |selector| based on |show_stream|@>
begin_diagnostic;
@z

@x
@ @<Show the current value of some parameter...@>=
begin p:=the_toks;
@y
@ @<Show the current value of some parameter...@>=
begin p:=the_toks;
@<Adjust |selector| based on |show_stream|@>
@z

@x
show_groups: begin begin_diagnostic; show_save_groups;
@y
show_groups: begin @<Adjust |selector| based on |show_stream|@>
  begin_diagnostic; show_save_groups;
@z

@x
show_ifs: begin begin_diagnostic; print_nl(""); print_ln;
@y
show_ifs: begin @<Adjust |selector| based on |show_stream|@>
  begin_diagnostic; print_nl(""); print_ln;
@z
