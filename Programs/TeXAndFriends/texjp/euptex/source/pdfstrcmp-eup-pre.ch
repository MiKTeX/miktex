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
if BYTE1(s)<>0 then print_char(@"100+BYTE1(s));
if BYTE2(s)<>0 then print_char(@"100+BYTE2(s));
if BYTE3(s)<>0 then print_char(@"100+BYTE3(s));
                    print_char(@"100+BYTE4(s));
end;
@y
procedure print_kanji(@!s:KANJI_code); {prints a single character}
begin
if s>@"FF then
  begin print_char(@"100+Hi(s)); print_char(@"100+Lo(s));
  end else print_char(s);
end;
@z
