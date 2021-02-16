% $Id$
% enctex1.ch: implementation of enc\TeX, to be applied on top of tex.ch.
% This is the first set of changes, used by all engines supporting encTeX
% (i.e., everything but XeTeX and LuaTeX).

@x [2.20] l.579 - encTeX: global declarations
xprn: array [ASCII_code] of ASCII_code;
   { non zero iff character is printable }
@y
xprn: array [ASCII_code] of ASCII_code;
   { non zero iff character is printable }
mubyte_read: array [ASCII_code] of pointer;
   { non zero iff character begins the multi byte code }
mubyte_write: array [ASCII_code] of str_number;
   { non zero iff character expands to multi bytes in log and write files }
mubyte_cswrite: array [0..127] of pointer;
   { non null iff cs mod 128 expands to multi bytes in log and write files }
mubyte_skip: integer;  { the number of bytes to skip in |buffer| }
mubyte_keep: integer; { the number of chars we need to keep unchanged }
mubyte_skeep: integer; { saved |mubyte_keep| }
mubyte_prefix: integer; { the type of mubyte prefix }
mubyte_tablein: boolean; { the input side of table will be updated }
mubyte_tableout: boolean; { the output side of table will be updated }
mubyte_relax: boolean; { the relax prefix is used }
mubyte_start: boolean; { we are making the token at the start of the line }
mubyte_sstart: boolean; { saved |mubyte_start| }
mubyte_token: pointer; { the token returned by |read_buffer| }
mubyte_stoken: pointer; { saved first token in mubyte primitive }
mubyte_sout: integer; { saved value of |mubyte_out| }
mubyte_slog: integer; { saved value of |mubyte_log| }
spec_sout: integer; { saved value of |spec_out| }
no_convert: boolean; { conversion supressed by noconvert primitive }
active_noconvert: boolean; { true if noconvert primitive is active }
write_noexpanding: boolean; { true only if we need not write expansion }
cs_converting: boolean; { true only if we need csname converting }
special_printing: boolean; { true only if we need converting in special }
message_printing: boolean; { true if message or errmessage prints to string }
@z

@x [2.23] l.723 - encTeX
for i:=@'177 to @'377 do xchr[i]:=i;
@y
for i:=@'177 to @'377 do xchr[i]:=i;
{Initialize enc\TeX\ data.}
for i:=0 to 255 do mubyte_read[i]:=null;
for i:=0 to 255 do mubyte_write[i]:=0;
for i:=0 to 127 do mubyte_cswrite[i]:=null;
mubyte_keep := 0; mubyte_start := false;
write_noexpanding := false; cs_converting := false;
special_printing := false; message_printing := false;
no_convert := false; active_noconvert := false;
@z

@x [5.59] l.1508 FIXME -- enc\TeX\ modifications of |print|.
  else begin if selector>pseudo then
      begin print_char(s); return; {internal strings are not expanded}
      end;
    if (@<Character |s| is the current new-line character@>) then
      if selector<pseudo then
        begin print_ln; return;
        end;
@y
  else begin if (selector>pseudo) and (not special_printing)
                 and (not message_printing) then
      begin print_char(s); return; {internal strings are not expanded}
      end;
    if (@<Character |s| is the current new-line character@>) then
      if selector<pseudo then
        begin print_ln; no_convert := false; return;
        end
      else if message_printing then
        begin print_char(s); no_convert := false; return;
        end;
    if (mubyte_log>0) and (not no_convert) and (mubyte_write[s]>0) then
      s := mubyte_write[s]
    else if xprn[s] or special_printing then
      begin print_char(s); no_convert := false; return; end;
    no_convert := false;
@z

@x [5.71] encTeX - native buffer printing
if last<>first then for k:=first to last-1 do print(buffer[k]);
@y
k:=first; while k < last do begin print_buffer(k) end;
@z

@x [17.230] l.4725 - encTeX: xord_code_base, xchr_code_base, prn_code_base,
@d math_font_base=cur_font_loc+1 {table of 48 math font numbers}
@y
@d xord_code_base=cur_font_loc+1
@d xchr_code_base=xord_code_base+1
@d xprn_code_base=xchr_code_base+1
@d math_font_base=xprn_code_base+1
@z
