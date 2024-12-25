% unbalanced-braces-eptex.ch
% Fix overrun/underrun of \write and \output. David Fuchs, 2024.
% Public domain.
%
% Changes for full defense against over-running (or under-running)
% an \output or \write and ending up in all sorts of ill-defined
% trouble.  (Including the case of \output=\toks with no braces.)
% 
% For some problematic input files, see tests/unbalanced-braces.test
% (not a runnable test).
%
% The idea is that when it's time to run/evaluate each \output or \write,
% they have to come to an end exactly as expected: at the right brace that
% came from when they were first scanned to begin with; no amount of
% monkey business with "funny braces" or \noexpand or \expandafter or \let
% or whatever other trick you can think of should be able to get around
% it. And you can't get away with going even one token past where you were
% supposed to stop.
%
% To reiterate, fatal errors from this result in the terminal show just:
%   ! Emergency stop.
% with no details on what/why.  You have to look in the .log file for
% the (terse) specifics:
%   Unbalanced output routine
% 
% It's rather draconian, calling fatal_error when there's a problem,
% but users really have no business trying to get anywhere near this
% sort of thing on purpose.  I sure hope nobody has found any use for
% such undefined behaviors.
 
% The changes assume locnull-optimize.ch has been applied. Unfortunately
% the semantically-related changes have to be broken up to apply in
% tex.web order.

% emacs-page
%% Catch extra left braces in \output right when finished scanning it.
%
@x [23.324] l.7000 p.B139
    if token_type=macro then {parameters must be flushed}
      while param_ptr>param_start do
        begin decr(param_ptr);
        flush_list(param_stack[param_ptr]);
        end;
@y
    if token_type=macro then {parameters must be flushed}
      while param_ptr>param_start do
        begin decr(param_ptr);
        flush_list(param_stack[param_ptr]);
        end
    else if (token_type=output_text)and(not output_can_end) then
      fatal_error("Unbalanced output routine");
@.Unbalanced output routine@>
@z

%% Catch extra right braces in the \output routine.
% <Resume the page builder...> tried, but needs to be more robust against
% a backed-up right (funny) brace in the middle of \output masquerading
% as the end-of-\output right brace.
%
% Reorder these so that end_token_list sees output_active=false.
% 1) In back_input:
@x [23.325] l.7025 p.B139
begin while (loc=null)and(token_type<>v_template) do
  end_token_list; {conserve stack space}
@y
begin while (loc=null)and(token_type<>v_template)
            and(token_type<>output_text) do
  end_token_list; {conserve stack space}
@z

%% Catch extra left braces finishing scanning a \write_text.
%
% In <Input from token list...> don't allow end_write while we're
% still scanning through the write_text.
@x [24.357] l.7488 p.B150
      else if suppress_outer_error=0 then check_outer_validity;
@y
      else
        begin
        if (cur_cs=end_write)and(mode=0) then
          fatal_error("Unbalanced write command");
        if suppress_outer_error=0 then check_outer_validity;
        end;
@z

%% Returning to catching extra right braces.
%
% 2) In <Feed the macro body and its parameters to the scanner>:
@x [25.390] l.7983 p.B161
while (loc=null)and(token_type<>v_template) do
  end_token_list; {conserve stack space}
@y
while (loc=null)and(token_type<>v_template)
      and(token_type<>output_text) do
  end_token_list; {conserve stack space}
@z

% We know we've just scanned a right brace that seems to be the end
% of the \output routine.  But maybe it had been backed-up over,
% and we've lost the output_text in the call to back_input. So,
% the checking gets sloppy.  But now we're sure to keep the
% (finished) output_text level around, so we can always check
% that we were just finished with it, so it's where the brace
% came from.

@x [45.989] l.19364 p.B417
@!output_active:boolean; {are we in the midst of an output routine?}
@y
@!output_active:boolean; {are we in the midst of an output routine?}
@!output_can_end:boolean; {is this an auspicious time for it to end?}
@z

@x [45.989] l.19367 p.B417
output_active:=false; insert_penalties:=0;
@y
output_active:=false; output_can_end:=false; insert_penalties:=0;
@z

% In <Resume the page builder after an output routine has come to an end>:
@x [45.1026] l.19938 p.B432
begin if (loc<>null) or
 ((token_type<>output_text)and(token_type<>backed_up)) then
  @<Recover from an unbalanced output routine@>;
@y
begin
  while (state=token_list)and(loc=null)and(token_type=backed_up) do
    end_token_list; {output-ending brace may have been backed-up}
  if (state<>token_list)or(loc<>null)or(token_type<>output_text) then
    @<Recover from an unbalanced output routine@>;
@z

% In <Resume the page builder after an output routine has come to an end>:
@x [45.1026] l.19941 p.B432
end_token_list; {conserve stack space in case more outputs are triggered}
end_graf; unsave; output_active:=false; insert_penalties:=0;@/
@y
output_can_end:=true;
end_token_list; {conserve stack space in case more outputs are triggered}
output_can_end:=false;
end_graf; unsave; output_active:=false; insert_penalties:=0;@/
@z

% <Expand macros in the token list and...> had set mode:=0 while
% expanding the \write token list.

% Reorder these statements so that the final get_token that's supposed to
% scan off the end_write_token will have mode<>0 if everything lined up:
@x [53.1371] l.24884 p.B546
get_token;@+if cur_tok<>end_write_token then
  @<Recover from an unbalanced write command@>;
mode:=old_mode;
@y
mode:=old_mode;
get_token;@+if cur_tok<>end_write_token then
  @<Recover from an unbalanced write command@>;
@z
