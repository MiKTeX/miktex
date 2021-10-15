% $Id$
% Public domain (from David Jones, tex-implementors 30jun2020).
% Unfortunately this cannot be in pdftex.ch since the ordering
% of changes gets messed up.
%
% If \tracinglostchars >= 3, then making missing characters from a font
% be an error, not just a log message, and always report the character
% code in hex as well as a character.
% 
% Simple example test:
% pdftex -ini
%   \font\x=/usr/local/texlive/2020/texmf-dist/fonts/tfm/public/knuth-lib/logo10.tfm
%   \tracinglostchars=3 \x \char99\end
% should get:
%   ! Missing character: There is no c ("63) in font logo10.

@x [30] If \tracinglostchars > 2, then promote to full error.
@p procedure char_warning(@!f:internal_font_number;@!c:eight_bits);
var old_setting: integer; {saved value of |tracing_online|}
begin if tracing_lost_chars>0 then
 begin old_setting:=tracing_online;
 if eTeX_ex and(tracing_lost_chars>1) then tracing_online:=1;
  begin begin_diagnostic;
  print_nl("Missing character: There is no ");
@.Missing character@>
  print_ASCII(c); print(" in font ");
  slow_print(font_name[f]); print_char("!"); end_diagnostic(false);
  end;
 tracing_online:=old_setting;
 end;
end;
@y
@p procedure char_warning(@!f:internal_font_number;@!c:eight_bits);
var old_setting: integer; {saved value of |tracing_online|}
begin if tracing_lost_chars>0 then
 begin old_setting:=tracing_online;
 if eTeX_ex and(tracing_lost_chars>1) then tracing_online:=1;
 if tracing_lost_chars > 2 then
   print_err("Missing character: There is no ")
 else begin
   begin_diagnostic;
   print_nl("Missing character: There is no ")
 end;
@.Missing character@>
 print_ASCII(c);
 if tracing_lost_chars > 2 then begin
   print(" (");
   print_hex(c);
   print(")");
 end;
 print(" in font ");
 slow_print(font_name[f]);
 if tracing_lost_chars < 3 then print_char("!");
 tracing_online:=old_setting;
 if tracing_lost_chars > 2 then begin
   help0;
   error;
 end else end_diagnostic(false);
 end; {of |tracing_lost_chars>0|}
end;  {of procedure}
@z

