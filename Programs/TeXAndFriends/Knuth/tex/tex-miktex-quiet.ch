%%% tex-miktex-quiet.ch:
%%%
%%% Derived from:
%%% tex.ch for C compilation with web2c, derived from various other
%%% change files.  By Tim Morgan, UC Irvine ICS Department, and many
%%% others.

% _____________________________________________________________________________
%
% [5.61]
% _____________________________________________________________________________

@x
if not miktex_is_compatible then
  wterm(banner_k);
else
  wterm(banner);
miktex_print_miktex_banner(term_out); {\MiKTeX: append the \MiKTeX\ version information}
if format_ident=0 then print_ln {\MiKTeX: eliminate misleading `(no format preloaded)'.}
else  begin slow_print(format_ident); print_ln;
  end;
@y
if (not miktex_get_quiet_flag) then begin
  if not miktex_is_compatible then
    wterm(banner_k)
  else
    wterm(banner);
  miktex_print_miktex_banner(term_out); {\MiKTeX: append the \MiKTeX\ version information}
  if format_ident=0 then print_ln {\MiKTeX: eliminate misleading `(no format preloaded)'.}
  else  begin slow_print(format_ident); print_ln;
    end;
end;
@z

% _____________________________________________________________________________
%
% [54.1379]
% _____________________________________________________________________________

@x
@* \[54/\MiKTeX] System-dependent changes for \MiKTeX.
@y
@* \[54/\MiKTeX] System-dependent changes for \MiKTeX.

@ Forward declaration of \MiKTeX\ functions.

@<Declare \MiKTeX\ functions@>=

function miktex_get_quiet_flag : boolean; forward;@t\2@>@/
@z
