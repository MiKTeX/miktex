@x
@d my_name=='ptftopl'
@d banner=='This is pTFtoPL, Version 3.3-p2.0'
@y
@d my_name=='uptftopl'
@d banner=='This is upTFtoPL, Version 3.3-p2.0-u1.28'
@z

@x
      usage_help (PTFTOPL_HELP, 'issue@@texjp.org');
@y
      usage_help (UPTFTOPL_HELP, 'issue@@texjp.org');
@z

@x
@d max_kanji=7237 {number of the kanji characters - 1}
@y
@d max_kanji=1114111 {number of the kanji characters - 1}
@z

@x procedure out_kanji
i:0..3; {index of array}
@y
i:0..5; {index of array}
@z
@x
  begin cx:=jis_code; out('J '); {specify jiscode format}
  dig[0]:=Hi(cx) div 16; dig[1]:=Hi(cx) mod 16;
  dig[2]:=Lo(cx) div 16; dig[3]:=Lo(cx) mod 16;
  for i:=0 to 3 do
@y
  begin cx:=jis_code;
  if (isinternalUPTEX) then out('U ')
  else out('J '); {specify jiscode format}
  dig[0]:=(cx div 65536) div 16; dig[1]:=(cx div 65536) mod 16;
  dig[2]:=(cx div 4096) mod 16; dig[3]:=(cx div 256) mod 16;
  dig[4]:=(cx div 16) mod 16; dig[5]:=cx mod 16;
  for i:=0 to 1 do
    if (dig[i]<>0)or(dig[0]<>0) then begin { if dig[0]<>0, dig[1] should be always printed }
      if dig[i]<10 then out(dig[i]) else
      case dig[i] of
         10: out('A'); 11: out('B'); 12: out('C');
         13: out('D'); 14: out('E'); 15: out('F');
      end;
    end;
  for i:=2 to 5 do
@z

@x
  out(xchr[Hi(cx)]); out(xchr[Lo(cx)]);
@y
  if BYTE1(cx)<>0 then out(xchr[BYTE1(cx)]);
  if BYTE2(cx)<>0 then out(xchr[BYTE2(cx)]);
  if BYTE3(cx)<>0 then out(xchr[BYTE3(cx)]);
                       out(xchr[BYTE4(cx)]);
@z

@x function valid_jis_code
var first_byte, second_byte:integer; { jis code bytes }
begin valid_jis_code:=true;
first_byte:=cx div @'400; second_byte:=cx mod @'400;
if (first_byte<@"21)
     or((first_byte>@"28)and(first_byte<@"30))
     or(first_byte>@"74) then valid_jis_code:=false;
if (second_byte<@"21)or(second_byte>@"7E) then valid_jis_code:=false;
@y
begin valid_jis_code:=true;
if (cx>@"10FFFF)or(not is_char_kanji(fromDVI(cx)))
  or(toDVI(fromDVI(cx))<>cx) then valid_jis_code:=false;
@z

@x function index_to_jis
if ix<=8*94-1 then
  index_to_jis:=(ix div 94 + @"21) * @'400 + (ix mod 94 + @"21)
else
  index_to_jis:=((ix+7 * 94) div 94 + @"21) * @'400 + ((ix+7*94) mod 94 + @"21);
@y
index_to_jis:=ix;
@z

@x function jis_to_index
var first_byte,second_byte:integer; { jis code bytes }
begin
first_byte:=cx div @'400 - @"21;
second_byte:=cx mod @'400 - @"21;
if first_byte<8 then
  jis_to_index:=first_byte*94+second_byte
else
  jis_to_index:=(first_byte-7)*94+second_byte;
@y
begin
jis_to_index:=cx;
@z

