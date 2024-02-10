% $Id$
% Public domain. See ../pdftexdir/char-warning-pdftex.ch for basic info.
% In addition to the basic change, we also define a new procedure
% print_ucs_code to print in the format U+.... for system fonts.
%
% To test:
% xetex -ini
%   \font\x=/usr/local/texlive/2020/texmf-dist/fonts/tfm/public/knuth-lib/logo10.tfm
%   \tracinglostchars=3 \x \char99\end
% should get:
%   ! Missing character: There is no c ("63) in font logo10.
% And:
%   \font\y="Nimbus Mono" \tracinglostchars=3 \y \char43981\end
% should get:
%   ! Missing character: There is no ꯍ (U+ABCD) in font Nimbus Mono Regular!.
% Assuming that font is findable as a system font, e.g., shown by
%   fc-list :scalable=true family

@x [30] If \tracinglostchars > 2, then promote to full error.
procedure char_warning(@!f:internal_font_number;@!c:integer);
var old_setting: integer; {saved value of |tracing_online|}
begin if tracing_lost_chars>0 then
 begin old_setting:=tracing_online;
 if eTeX_ex and(tracing_lost_chars>1) then tracing_online:=1;
  begin begin_diagnostic;
  print_nl("Missing character: There is no ");
@.Missing character@>
  if c < @"10000 then print_ASCII(c)
  else print_char(c); {non-Plane 0 Unicodes can't be sent through |print_ASCII|}
  print(" in font ");
  slow_print(font_name[f]); print_char("!"); end_diagnostic(false);
  end;
 tracing_online:=old_setting;
 end;
end;
@y
procedure print_ucs_code(@!n: UnicodeScalar); { cf.~|print_hex| }
    var k: 0..22; {index to current digit; we assume that $0\L n<16^{22}$}
    begin
        k := 0;
        print("U+"); { prefix with U+ instead of " }
        repeat
            dig[k] := n mod 16;
            n      := n div 16;
            incr(k);
        until n = 0;
        { pad to at least 4 hex digits }
        while k < 4 do
        begin
            dig[k] := 0;
            incr(k);
        end;
        print_the_digs(k);
end;

procedure char_warning(@!f:internal_font_number;@!c:integer);
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
 if c < @"10000 then print_ASCII(c)
 else print_char(c); {non-Plane 0 Unicodes can't be sent through |print_ASCII|}
 print(" (");
 if is_native_font(f) then print_ucs_code(c)
 else print_hex(c);
 print(")");
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
