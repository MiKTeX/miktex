@x
    if t=" " then t:=space_token
    else t:=other_token+t;
@y
  else if t=" " then t:=space_token
  else t:=other_token+t;
@z

@x
procedure print_kanji(@!s:KANJI_code); {prints a single character}
begin
s:=toBUFF(s mod max_cjk_val);
if BYTE1(s)<>0 then print_char(BYTE1(s));
if BYTE2(s)<>0 then print_char(BYTE2(s));
if BYTE3(s)<>0 then print_char(BYTE3(s));
                    print_char(BYTE4(s));
end;
@y
procedure print_kanji(@!s:KANJI_code); {prints a single character}
begin
if s>255 then
  begin print_char(Hi(s)); print_char(Lo(s));
  end else print_char(s);
end;
@z
