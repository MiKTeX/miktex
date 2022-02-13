% $Id$
% This is a change file for upTeX u1.27
% By Takuji Tanaka.
%
% (02/26/2007) TTK  upTeX u0.01
% (03/18/2007) TTK  upTeX u0.03
% (04/08/2007) TTK  upTeX u0.05
% (04/15/2007) TTK  upTeX u0.06
% (05/13/2007) TTK  upTeX u0.07
% (05/28/2007) TTK  upTeX u0.08
% (06/16/2007) TTK  upTeX u0.09
% (07/28/2007) TTK  upTeX u0.10
% (08/07/2007) TTK  upTeX u0.11
% (08/25/2007) TTK  upTeX u0.12
% (09/15/2007) TTK  upTeX u0.13
% (09/25/2007) TTK  upTeX u0.14
% (10/18/2007) TTK  upTeX u0.15
% (11/10/2007) TTK  upTeX u0.16
% (11/13/2007) TTK  upTeX u0.17
% (12/08/2007) TTK  upTeX u0.18
% (12/26/2007) TTK  upTeX u0.19
% (01/12/2008) TTK  upTeX u0.20
% (03/13/2008) TTK  upTeX u0.21
% (08/03/2008) TTK  upTeX u0.22
% (09/04/2008) TTK  upTeX u0.23
% (10/18/2008) TTK  upTeX u0.24
% (12/13/2008) TTK  upTeX u0.25
% (02/21/2009) TTK  upTeX u0.26
% (03/12/2009) TTK  upTeX u0.27
% (08/23/2009) TTK  upTeX u0.28
% (01/31/2010) TTK  upTeX u0.29
% (04/10/2010) TTK  upTeX u0.30
% (01/15/2012) TTK  upTeX u1.00
% (04/29/2012) TTK  upTeX u1.10
% (08/13/2012) TTK  upTeX u1.11
% (12/29/2014) TTK  upTeX u1.20
% (02/20/2016) TTK  upTeX u1.21
% (01/15/2017) TTK  upTeX u1.22
% (04/09/2017) TTK  Hironori Kitagawa fixed a bug in \endlinechar.
% (2018-01-21) HK   Added \uptexversion primitive and co.
% (2018-02-24) TTK  upTeX u1.23
% (2019-02-23) TTK  upTeX u1.24
% (2019-05-06) HK   Hironori Kitagawa fixed a bug in \if.
% (2019-05-06) TTK  upTeX u1.25
% (2020-02-22) TTK  upTeX u1.26
% (2020-10-25) TTK  upTeX u1.27
% (2022-01-23) TTK  upTeX u1.28

@x upTeX: banner
  {printed when \pTeX\ starts}
@y
  {printed when \pTeX\ starts}
@#
@d upTeX_version=1
@d upTeX_revision==".28"
@d upTeX_version_string=='-u1.28' {current u\pTeX\ version}
@#
@d upTeX_banner=='This is upTeX, Version 3.141592653',pTeX_version_string,upTeX_version_string
@d upTeX_banner_k==upTeX_banner
  {printed when u\pTeX\ starts}
@z

@x upTeX: banner
@d banner==pTeX_banner
@d banner_k==pTeX_banner_k
@y
@d banner==upTeX_banner
@d banner_k==upTeX_banner_k
@z

@x
@!KANJI_code=0..65535; {sixteen-bit numbers}
@y
@!KANJI_code=0..@"FFFFFF; {0..0x10FFFF: Unicode, 0x110000..0xFFFFFF: special}
@z

@x
@!trick_buf2:array[0..ssup_error_line] of 0..2; {pTeX: buffer for KANJI}
@!kcode_pos: 0..2; {pTeX: denotes whether first byte or second byte of KANJI}
@y
@!trick_buf2:array[0..ssup_error_line] of 0..@'24; {pTeX: buffer for KANJI}
@!kcode_pos: 0..@'24; {pTeX: denotes whether first byte or second byte of KANJI
  1..2:2byte-char, 11..13:3byte-char, 21..24:4byte-char}
@!kcp: 0..@'24; {temporary |kcode_pos|}
@z

@x
procedure print_ln; {prints an end-of-line}
@y
procedure print_ln; {prints an end-of-line}
var @!ii: integer;
@z

@x
  if kcode_pos=1 then begin wterm(' '); wlog(' '); end;
@y
  if nrestmultichr(kcode_pos)>0 then
  for ii:=0 to nrestmultichr(kcode_pos)-1 do
    begin wterm(' '); wlog(' '); end;
@z

@x
log_only: begin if kcode_pos=1 then wlog(' ');
@y
log_only: begin
  if nrestmultichr(kcode_pos)>0 then
  for ii:=0 to nrestmultichr(kcode_pos)-1 do wlog(' ');
@z

@x
term_only: begin if kcode_pos=1 then wterm(' ');
@y
term_only: begin
  if nrestmultichr(kcode_pos)>0 then
  for ii:=0 to nrestmultichr(kcode_pos)-1 do wterm(' ');
@z

@x
else if kcode_pos=1 then kcode_pos:=2
@y
else if (kcode_pos=1)or((kcode_pos>=@'11)and(kcode_pos<=@'12))
   or((kcode_pos>=@'21)and(kcode_pos<=@'23)) then incr(kcode_pos)
@z

@x
  begin kcode_pos:=1;
@y
  begin
  if (ismultichr(4,1,xchr[s])) then kcode_pos:=@'21
  else if (ismultichr(3,1,xchr[s])) then kcode_pos:=@'11
  else kcode_pos:=1;
@z

@x
    if file_offset>=max_print_line-1 then
@y
    if file_offset>=max_print_line-nrestmultichr(kcode_pos) then
@z

@x
    if term_offset>=max_print_line-1 then
@y
    if term_offset>=max_print_line-nrestmultichr(kcode_pos) then
@z

@x
@d min_quarterword=0 {smallest allowable value in a |quarterword|}
@d max_quarterword=255 {largest allowable value in a |quarterword|}
@d min_halfword==-@"FFFFFFF {smallest allowable value in a |halfword|}
@d max_halfword==@"FFFFFFF {largest allowable value in a |halfword|}
@d max_cjk_val=@"10000
@y
@d min_quarterword=0 {smallest allowable value in a |quarterword|}
@d max_quarterword=@"FFFF {largest allowable value in a |quarterword|}
@d min_halfword=-@"3FFFFFFF {smallest allowable value in a |halfword|}
@d max_halfword=@"3FFFFFFF {largest allowable value in a |halfword|}
@d max_cjk_val=@"1000000 {to separate wchar and kcatcode}
@z

@x
@d kanji=16 {kanji}
@d kana=17 {hiragana, katakana, alphabet}
@d other_kchar=18 {kanji codes}
@d max_char_code=18 {largest catcode for individual characters}
@y
@d not_cjk=15 {is not cjk characters}
@d kanji=16 {kanji}
@d kana=17 {hiragana, katakana, alphabet}
@d other_kchar=18 {cjk symbol codes}
@d hangul=19 {hangul codes}
@d max_char_code=19 {largest catcode for individual characters}
@z

@x
@d math_comp=left_right+1 {component of formula ( \.{\\mathbin}, etc.~)}
@y
@d kchar_num=left_right+1 {cjk character specified numerically ( \.{\\kchar} )}
@d math_comp=kchar_num+1 {component of formula ( \.{\\mathbin}, etc.~)}
@z

@x
@d math_given=char_given+1 {math code defined by \.{\\mathchardef}}
@y
@d kchar_given=char_given+1 {cjk character code defined by \.{\\kchardef}}
@d math_given=kchar_given+1 {math code defined by \.{\\mathchardef}}
@z

@x
@d max_command=partoken_name {the largest command code seen at |big_switch|}
@y
@d set_enable_cjk_token=partoken_name+1 {set cjk mode ( \.{\\enablecjktoken}, \.{\\disablecjktoken}, \.{\\forcecjktoken} )}
@d max_command=set_enable_cjk_token {the largest command code seen at |big_switch|}
@z

@x
@d cat_code_base=auto_xspacing_code+1
  {table of 256 command codes (the ``catcodes'')}
@d kcat_code_base=cat_code_base+256
  {table of 256 command codes for the wchar's catcodes }
@d auto_xsp_code_base=kcat_code_base+256 {table of 256 auto spacer flag}
@y
@d enable_cjk_token_code=auto_xspacing_code+1
@d cat_code_base=enable_cjk_token_code+1
  {table of 256 command codes (the ``catcodes'')}
@d kcat_code_base=cat_code_base+256
  {table of 512 command codes for the wchar's catcodes }
@d auto_xsp_code_base=kcat_code_base+512 {table of 256 auto spacer flag}
@z

@x
@d auto_xspacing==equiv(auto_xspacing_code)
@y
@d auto_xspacing==equiv(auto_xspacing_code)
@d enable_cjk_token==equiv(enable_cjk_token_code)
@z

@x
eqtb[auto_xspacing_code]:=eqtb[cat_code_base];
for k:=0 to 255 do
  begin cat_code(k):=other_char; kcat_code(k):=other_kchar;
  math_code(k):=hi(k); sf_code(k):=1000;
  auto_xsp_code(k):=0; inhibit_xsp_code(k):=0; inhibit_xsp_type(k):=0;
  kinsoku_code(k):=0; kinsoku_type(k):=0;
  end;
@y
eqtb[auto_xspacing_code]:=eqtb[cat_code_base];
eqtb[enable_cjk_token_code]:=eqtb[cat_code_base];
for k:=0 to 255 do
  begin cat_code(k):=other_char;
  math_code(k):=hi(k); sf_code(k):=1000;
  auto_xsp_code(k):=0; inhibit_xsp_code(k):=0; inhibit_xsp_type(k):=0;
  kinsoku_code(k):=0; kinsoku_type(k):=0;
  end;
for k:=0 to 511 do
  begin kcat_code(k):=other_kchar;
  end;
@z

@x
@t\hskip10pt@>kcat_code(@"20+1):=other_kchar; {1 ku}
@t\hskip10pt@>kcat_code(@"20+2):=other_kchar; {2 ku}
@+@t\1@>for k:=3 to 6 do kcat_code(@"20+k):=kana; {3 ku ... 6 ku}
@+@t\1@>for k:=7 to 8 do kcat_code(@"20+k):=other_kchar; {7 ku ... 8 ku}
@+@t\1@>for k:=16 to 84 do kcat_code(@"20+k):=kanji; {16 ku ... 84 ku}
{ $\.{@@"20}+|k| = |kcatcodekey|(|fromKUTEN|(|HILO|(k,1))$ }
@y
if (isinternalUPTEX) then begin
  { default: |other_kchar| }
  @t\hskip10pt@>kcat_code(@"0):=not_cjk;
  @+@t\1@>for k:=@"2 to @"3 do kcat_code(k):=not_cjk; { Latin Extended-A, Latin Extended-B }
  @t\hskip10pt@>kcat_code(@"25):=hangul; { Hangul Jamo }
  @t\hskip10pt@>kcat_code(@"46):=not_cjk; { Latin Extended Additional }
  @+@t\1@>for k:=@"68 to @"6A do kcat_code(k):=kanji; { CJK Radicals Supplement .. Ideographic Description Characters }
  @+@t\1@>for k:=@"6C to @"6D do kcat_code(k):=kana;  { Hiragana, Katakana }
  @t\hskip10pt@>kcat_code(@"6E):=kanji; { Bopomofo }
  @t\hskip10pt@>kcat_code(@"6F):=hangul; { Hangul Compatibility Jamo }
  @+@t\1@>for k:=@"70 to @"72 do kcat_code(k):=kanji; { Kanbun .. CJK Strokes }
  @t\hskip10pt@>kcat_code(@"73):=kana; { Katakana Phonetic Extensions }
  @t\hskip10pt@>kcat_code(@"76):=kanji; { CJK Unified Ideographs Extension A }
  @t\hskip10pt@>kcat_code(@"78):=kanji; { CJK Unified Ideographs }
  @t\hskip10pt@>kcat_code(@"88):=hangul; { Hangul Jamo Extended-A }
  @t\hskip10pt@>kcat_code(@"93):=hangul; { Hangul Syllables }
  @t\hskip10pt@>kcat_code(@"94):=hangul; { Hangul Jamo Extended-B }
  @t\hskip10pt@>kcat_code(@"99):=kanji; { CJK Compatibility Ideographs }
  { \hskip10pt|kcat_code(@"A2):=other_kchar;| Halfwidth and Fullwidth Forms }
  @+@t\1@>for k:=@"10A to @"10D do kcat_code(k):=kana; { Kana Extended-B .. Small Kana Extension }
  @+@t\1@>for k:=@"135 to @"13B do kcat_code(k):=kanji; { CJK Unified Ideographs Extension B .. G }
  @t\hskip10pt@>kcat_code(@"1FD):=not_cjk; { Latin-1 Letters }
  @t\hskip10pt@>kcat_code(@"1FE):=kana; { Fullwidth digit and latin alphabet }
  @t\hskip10pt@>kcat_code(@"1FF):=kana; { Halfwidth katakana }
end else begin
  @t\hskip10pt@>kcat_code(@"20+1):=other_kchar; {1 ku}
  @t\hskip10pt@>kcat_code(@"20+2):=other_kchar; {2 ku}
  @+@t\1@>for k:=3 to 6 do kcat_code(@"20+k):=kana; {3 ku ... 6 ku}
  @+@t\1@>for k:=7 to 13 do kcat_code(@"20+k):=other_kchar; {7 ku ... 13 ku}
  @+@t\1@>for k:=14 to 120 do kcat_code(@"20+k):=kanji; {14 ku ... 120 ku}
  { $\.{@@"20}+|k| = |kcatcodekey|(|fromKUTEN|(|HILO|(k,1))$ }
  @+@t\1@>for k:=16 to 94 do kcat_code(@"A0+k):=kanji; {2 men 16 ku ... 94 ku}
end;
@z

@x l.5897 - upTeX
primitive("char",char_num,0);@/
@!@:char_}{\.{\\char} primitive@>
@y
primitive("char",char_num,0);@/
@!@:char_}{\.{\\char} primitive@>
primitive("kchar",kchar_num,0);@/
@!@:kchar_}{\.{\\kchar} primitive@>
@z

@x
ital_corr: print_esc("/");
@y
ital_corr: print_esc("/");
kchar_num: print_esc("kchar");
@z

@x
@d cs_token_flag==@"FFFF {amount added to the |eqtb| location in a
  token that stands for a control sequence; is a multiple of~256, less~1}
@d left_brace_token=@'0400 {$2^8\cdot|left_brace|$}
@d left_brace_limit=@'1000 {$2^8\cdot(|left_brace|+1)$}
@d right_brace_token=@'1000 {$2^8\cdot|right_brace|$}
@d right_brace_limit=@'1400 {$2^8\cdot(|right_brace|+1)$}
@d math_shift_token=@'1400 {$2^8\cdot|math_shift|$}
@d tab_token=@'2000 {$2^8\cdot|tab_mark|$}
@d out_param_token=@'2400 {$2^8\cdot|out_param|$}
@d space_token=@'5040 {$2^8\cdot|spacer|+|" "|$}
@d letter_token=@'5400 {$2^8\cdot|letter|$}
@d other_token=@'6000 {$2^8\cdot|other_char|$}
@d match_token=@'6400 {$2^8\cdot|match|$}
@d end_match_token=@'7000 {$2^8\cdot|end_match|$}
@y
@d cs_token_flag=@"1FFFFFFF {amount added to the |eqtb| location in a
  token that stands for a control sequence; is a multiple of~@@"1000000, less~1}
@d max_char_val=@"100 {to separate char and command code}
@d left_brace_token=@"100 {$2^8\cdot|left_brace|$}
@d left_brace_limit=@"200 {$2^8\cdot(|left_brace|+1)$}
@d right_brace_token=@"200 {$2^8\cdot|right_brace|$}
@d right_brace_limit=@"300 {$2^8\cdot(|right_brace|+1)$}
@d math_shift_token=@"300 {$2^8\cdot|math_shift|$}
@d tab_token=@"400 {$2^8\cdot|tab_mark|$}
@d out_param_token=@"500 {$2^8\cdot|out_param|$}
@d space_token=@"A20 {$2^8\cdot|spacer|+|" "|$}
@d letter_token=@"B00 {$2^8\cdot|letter|$}
@d other_token=@"C00 {$2^8\cdot|other_char|$}
@d match_token=@"D00 {$2^8\cdot|match|$}
@d end_match_token=@"E00 {$2^8\cdot|end_match|$}
@z

@x
  if check_kanji(info(p)) then {|wchar_token|}
    begin m:=kcat_code(kcatcodekey(info(p))); c:=info(p);
    end
  else  begin m:=Hi(info(p)); c:=Lo(info(p));
@y
  if check_kanji(info(p)) then {|wchar_token|}
    begin m:=info(p) div max_cjk_val; c:=info(p) mod max_cjk_val; end
  else  begin m:=info(p) div max_char_val; c:=info(p) mod max_char_val;
@z

@x
kanji,kana,other_kchar: print_kanji(KANJI(c));
@y
kanji,kana,other_kchar,hangul: print_kanji(KANJI(c));
@z

@x
kanji,kana,other_kchar: begin print("kanji character ");
@y
kanji,kana,other_kchar,hangul: begin print("kanji character ");
@z

@x
  if (first_count>0)and(trick_buf2[(first_count-1)mod error_line]=1) then
    incr(first_count);
@y
  kcp:=trick_buf2[(first_count-1)mod error_line];
  if (first_count>0)and(kcp>0) then
    first_count:=first_count+nrestmultichr(kcp);
@z

@x
if trick_buf2[p mod error_line]=2 then
  begin p:=p+1; n:=n-1;
  end;
@y
kcp:=trick_buf2[p mod error_line];
if (kcp mod @'10)>1 then begin
  p:=p+nrestmultichr(kcp)+1; n:=n-nrestmultichr(kcp)-1; end;
@z

@x
if trick_buf2[(p-1) mod error_line]=1 then p:=p-1;
@y
kcp:=trick_buf2[(p-1) mod error_line];
if ((kcp mod @'10)>0)and(nrestmultichr(kcp)>0) then p:=p-(kcp mod @'10);
@z

@x
      s:=get_avail; info(s):=Lo(info(loc));
@y
      s:=get_avail; info(s):=(info(loc) mod max_char_val);
@z

@x
  begin cur_chr:=buffer[loc]; incr(loc);
    if multistrlen(ustringcast(buffer), limit+1, loc-1)=2 then
      begin cur_chr:=fromBUFF(ustringcast(buffer), limit+1, loc-1);
      cur_cmd:=kcat_code(kcatcodekey(cur_chr));
      for l:=loc-1 to loc-2+multistrlen(ustringcast(buffer), limit+1, loc-1) do
        buffer2[l]:=1;
      incr(loc);
      end
    else reswitch: cur_cmd:=cat_code(cur_chr);
@y
  begin
    cur_chr:=fromBUFF(ustringcast(buffer), limit+1, loc);
    cur_cmd:=kcat_code(kcatcodekey(cur_chr));
    if (multistrlen(ustringcast(buffer), limit+1, loc)>1) and check_kcat_code(cur_cmd) then begin
      if (cur_cmd=not_cjk) then cur_cmd:=other_kchar;
      for l:=loc to loc-1+multistrlen(ustringcast(buffer), limit+1, loc) do
        buffer2[l]:=1;
      loc:=loc+multistrlen(ustringcast(buffer), limit+1, loc) end
    else begin
      cur_chr:=buffer[loc]; incr(loc);
      reswitch: cur_cmd:=cat_code(cur_chr);
    end;
@z

@x
@ @d add_delims_to(#)==#+math_shift,#+tab_mark,#+mac_param,
  #+sub_mark,#+letter,#+other_char
@d all_jcode(#)==#+kanji,#+kana,#+other_kchar
@y
@ @d add_delims_to(#)==#+math_shift,#+tab_mark,#+mac_param,
  #+sub_mark,#+letter,#+other_char
@d all_jcode(#)==#+kanji,#+kana,#+other_kchar
@d hangul_code(#)==#+hangul
@z

@x
all_jcode(skip_blanks),all_jcode(new_line),all_jcode(mid_line):
  state:=mid_kanji;
@y
all_jcode(skip_blanks),all_jcode(new_line),all_jcode(mid_line):
  state:=mid_kanji;
hangul_code(skip_blanks),hangul_code(new_line),hangul_code(mid_kanji):
  state:=mid_line;
@z

@x
else  begin k:=loc; cur_chr:=buffer[k]; incr(k);
  if multistrlen(ustringcast(buffer), limit+1, k-1)=2 then
    begin cat:=kcat_code(kcatcodekey(fromBUFF(ustringcast(buffer), limit+1, k-1)));
    for l:=k-1 to k-2+multistrlen(ustringcast(buffer), limit+1, k-1) do
      buffer2[l]:=1;
    incr(k);
    end
  else cat:=cat_code(cur_chr);
start_cs:
  if (cat=letter)or(cat=kanji)or(cat=kana) then state:=skip_blanks
@y
else  begin k:=loc;
  cur_chr:=fromBUFF(ustringcast(buffer), limit+1, k);
  cat:=kcat_code(kcatcodekey(cur_chr));
  if (multistrlen(ustringcast(buffer), limit+1, k)>1) and check_kcat_code(cat) then begin
    if (cat=not_cjk) then cat:=other_kchar;
    for l:=k to k-1+multistrlen(ustringcast(buffer), limit+1, k) do
      buffer2[l]:=1;
    k:=k+multistrlen(ustringcast(buffer), limit+1, k) end
  else begin {not multi-byte char}
    cur_chr:=buffer[k];
    cat:=cat_code(cur_chr);
    incr(k);
  end;
start_cs:
  if (cat=letter)or(cat=kanji)or(cat=kana)or(cat=hangul) then state:=skip_blanks
@z

@x
  else if ((cat=letter)or(cat=kanji)or(cat=kana))and(k<=limit) then
@y
  else if ((cat=letter)or(cat=kanji)or(cat=kana)or(cat=hangul))and(k<=limit) then
@z

@x
  if (cat=kanji)or(cat=kana) then
@y
  if (cat=kanji)or(cat=kana)or(cat=hangul) then
@z

@x
begin repeat cur_chr:=buffer[k]; incr(k);
  if multistrlen(ustringcast(buffer), limit+1, k-1)=2 then
    begin cat:=kcat_code(kcatcodekey(fromBUFF(ustringcast(buffer), limit+1, k-1)));
    for l:=k-1 to k-2+multistrlen(ustringcast(buffer), limit+1, k-1) do
      buffer2[l]:=1;
    incr(k);
    end
  else cat:=cat_code(cur_chr);
@y
begin repeat
  cur_chr:=fromBUFF(ustringcast(buffer), limit+1, k);
  cat:=kcat_code(kcatcodekey(cur_chr));
  if (multistrlen(ustringcast(buffer), limit+1, k)>1) and check_kcat_code(cat) then begin
    if (cat=not_cjk) then cat:=other_kchar;
    for l:=k to k-1+multistrlen(ustringcast(buffer), limit+1, k) do
      buffer2[l]:=1;
    k:=k+multistrlen(ustringcast(buffer), limit+1, k) end
  else begin {not multi-byte char}
    cur_chr:=buffer[k];
    cat:=cat_code(cur_chr);
    incr(k);
  end;
@z

@x
until not((cat=letter)or(cat=kanji)or(cat=kana))or(k>limit);
{@@<If an expanded...@@>;}
if not((cat=letter)or(cat=kanji)or(cat=kana)) then decr(k);
if cat=other_kchar then decr(k); {now |k| points to first nonletter}
@y
until not((cat=letter)or(cat=kanji)or(cat=kana)or(cat=hangul))or(k>limit);
{@@<If an expanded...@@>;}
if not((cat=letter)or(cat=kanji)or(cat=kana)or(cat=hangul)) then decr(k);
if cat=other_kchar then k:=k-multilenbuffchar(cur_chr)+1; {now |k| points to first nonletter}
@z

@x
  else if check_kanji(t) then {|wchar_token|}
    begin cur_chr:=t; cur_cmd:=kcat_code(kcatcodekey(t));
    end
  else
    begin cur_cmd:=Hi(t); cur_chr:=Lo(t);
@y
  else if check_kanji(t) then {|wchar_token|}
    begin cur_cmd:=t div max_cjk_val; cur_chr:=t mod max_cjk_val; end
  else
    begin cur_cmd:=t div max_char_val; cur_chr:=t mod max_char_val;
@z

@x
  if (cur_cmd=kanji)or(cur_cmd=kana)or(cur_cmd=other_kchar) then {|wchar_token|}
    cur_tok:=cur_chr
  else cur_tok:=(cur_cmd*@'400)+cur_chr
@y
  if (cur_cmd>=kanji)and(cur_cmd<=hangul) then {|wchar_token|}
    cur_tok:=(cur_cmd*max_cjk_val)+cur_chr
  else cur_tok:=(cur_cmd*max_char_val)+cur_chr
@z

@x
  if check_kanji(info(p)) then {|wchar_token|}
    begin buffer[j]:=Hi(info(p)); buffer2[j]:=1; incr(j); buffer2[j]:=1;
    end
  else buffer2[j]:=0;
  buffer[j]:=Lo(info(p)); incr(j); p:=link(p);
@y
  if check_kanji(info(p)) then {|wchar_token|}
    begin t:=toBUFF(info(p) mod max_cjk_val);
    if BYTE1(t)<>0 then begin buffer[j]:=BYTE1(t); buffer2[j]:=1; incr(j); end;
    if BYTE2(t)<>0 then begin buffer[j]:=BYTE2(t); buffer2[j]:=1; incr(j); end;
    if BYTE3(t)<>0 then begin buffer[j]:=BYTE3(t); buffer2[j]:=1; incr(j); end;
                              buffer[j]:=BYTE4(t); buffer2[j]:=1; incr(j);
    p:=link(p);
    end
  else
    begin buffer[j]:=info(p) mod max_char_val; buffer2[j]:=0; incr(j); p:=link(p);
    end;
@z

@x
  if (cur_cmd=kanji)or(cur_cmd=kana)or(cur_cmd=other_kchar) then
    cur_tok:=cur_chr
  else cur_tok:=(cur_cmd*@'400)+cur_chr
@y
  if (cur_cmd>=kanji)and(cur_cmd<=hangul) then
    cur_tok:=(cur_cmd*max_cjk_val)+cur_chr
  else cur_tok:=(cur_cmd*max_char_val)+cur_chr
@z

@x
  if (cur_cmd=kanji)or(cur_cmd=kana)or(cur_cmd=other_kchar) then
    cur_tok:=cur_chr
  else cur_tok:=(cur_cmd*@'400)+cur_chr
@y
  if (cur_cmd>=kanji)and(cur_cmd<=hangul) then
    cur_tok:=(cur_cmd*max_cjk_val)+cur_chr
  else cur_tok:=(cur_cmd*max_char_val)+cur_chr
@z

@x
char_given,math_given: scanned_result(cur_chr)(int_val);
@y
kchar_given,
char_given,math_given: scanned_result(cur_chr)(int_val);
@z

@x
@d ptex_minor_version_code=ptex_version_code+1 {code for \.{\\ptexminorversion}}
@y
@d ptex_minor_version_code=ptex_version_code+1 {code for \.{\\ptexminorversion}}
@d uptex_version_code=ptex_minor_version_code+1 {code for \.{\\uptexversion}}
@z

@x
primitive("ptexversion",last_item,ptex_version_code);
@!@:ptexversion_}{\.{\\ptexversion} primitive@>
@y
primitive("ptexversion",last_item,ptex_version_code);
@!@:ptexversion_}{\.{\\ptexversion} primitive@>
primitive("uptexversion",last_item,uptex_version_code);
@!@:uptexversion_}{\.{\\uptexversion} primitive@>
@z

@x
  ptex_version_code: print_esc("ptexversion");
@y
  ptex_version_code: print_esc("ptexversion");
  uptex_version_code: print_esc("uptexversion");
@z

@x
    ptex_version_code: cur_val:=pTeX_version;
@y
    ptex_version_code: cur_val:=pTeX_version;
    uptex_version_code: cur_val:=upTeX_version;
@z

@x
  if (cur_cmd=kanji)or(cur_cmd=kana)or(cur_cmd=other_kchar) then {|wchar_token|}
@y
  if (cur_cmd>=kanji)and(cur_cmd<=hangul) then {|wchar_token|}
@z

@x
@!t:halfword; {token being appended}
@!k:pool_pointer; {index into |str_pool|}
begin str_room(1);
p:=temp_head; link(p):=null; k:=b;
while k<pool_ptr do
  begin t:=so(str_pool[k]);
  if t>=@"100 then
    begin t:=fromBUFFshort(str_pool, pool_ptr, k); incr(k);
    end
  else if t=" " then t:=space_token
  else t:=other_token+t;
@y
@!t:halfword; {token being appended}
@!k:pool_pointer; {index into |str_pool|}
@!cc:escape..max_char_code;
begin str_room(1);
p:=temp_head; link(p):=null; k:=b;
while k<pool_ptr do
  begin  t:=so(str_pool[k]);
  if t>=@"180 then { there is no |wchar_token| whose code is 0--127. }
    begin t:=fromBUFFshort(str_pool, pool_ptr, k); cc:=kcat_code(kcatcodekey(t));
    if (cc=not_cjk) then cc:=other_kchar;
    t:=t+cc*max_cjk_val;
    k:=k+multistrlenshort(str_pool, pool_ptr, k)-1;
    end
  else begin t:=so(str_pool[k]);
    if t>=@"100 then t:=t-@"100;
    if t=" " then t:=space_token
    else t:=other_token+t;
  end;
@z

@x

@d ptex_revision_code=12 {command code for \.{\\ptexrevision}}
@d ptex_convert_codes=13 {end of \pTeX's command codes}
@y
@d ptex_revision_code=12 {command code for \.{\\ptexrevision}}
@d uptex_revision_code=13 {command code for \.{\\uptexrevision}}
@d ptex_convert_codes=14 {end of \pTeX's command codes}
@z

@x
primitive("ptexrevision",convert,ptex_revision_code);
@!@:ptexrevision_}{\.{\\ptexrevision} primitive@>
@y
primitive("ptexrevision",convert,ptex_revision_code);
@!@:ptexrevision_}{\.{\\ptexrevision} primitive@>
primitive("uptexrevision",convert,uptex_revision_code);
@!@:uptexrevision_}{\.{\\uptexrevision} primitive@>
@z

@x
  ptex_revision_code:print_esc("ptexrevision");
@y
  ptex_revision_code:print_esc("ptexrevision");
  uptex_revision_code:print_esc("uptexrevision");
@z

@x
ptex_revision_code: do_nothing;
string_code, meaning_code: begin save_scanner_status:=scanner_status;
  scanner_status:=normal; get_token;
  if (cur_cmd=kanji)or(cur_cmd=kana)or(cur_cmd=other_kchar) then {|wchar_token|}
@y
ptex_revision_code, uptex_revision_code: do_nothing;
string_code, meaning_code: begin save_scanner_status:=scanner_status;
  scanner_status:=normal; get_token;
  if (cur_cmd>=kanji)and(cur_cmd<=hangul) then {|wchar_token|}
@z

@x
ucs_code:   begin cur_val:=fromUCS(cur_val);
  if cur_val=0 then print_int(-1) else print_int(cur_val); end;
toucs_code: begin cur_val:=toUCS(cur_val);
  if cur_val=0 then print_int(-1) else print_int(cur_val); end;
@y
ucs_code:   if (isinternalUPTEX) then print_int(fromUCS(cur_val))
  else begin cur_val:=fromUCS(cur_val);
  if cur_val=0 then print_int(-1) else print_int(cur_val); end;
toucs_code: if (isinternalUPTEX) then print_int(toUCS(cur_val))
  else begin cur_val:=toUCS(cur_val);
  if cur_val=0 then print_int(-1) else print_int(cur_val); end;
@z

@x
ptex_revision_code: print(pTeX_revision);
@y
ptex_revision_code: print(pTeX_revision);
uptex_revision_code: print(upTeX_revision);
@z

@x
if (cur_cmd=kanji)or(cur_cmd=kana)or(cur_cmd=other_kchar) then
@y
if (cur_cmd>=kanji)and(cur_cmd<=hangul) then
@z

@x
get_x_token_or_active_char;
if (cur_cmd=kanji)or(cur_cmd=kana)or(cur_cmd=other_kchar) then
@y
get_x_token_or_active_char;
if (cur_cmd>=kanji)and(cur_cmd<=hangul) then
@z

@x
  if (cur_cmd=kanji)or(cur_cmd=kana)or(cur_cmd=other_kchar) then {is kanji}
    begin str_room(2);
    append_char(@"100+Hi(cur_chr)); {kanji upper byte}
    append_char(@"100+Lo(cur_chr)); {kanji lower byte}
@y
  if (cur_cmd>=kanji)and(cur_cmd<=hangul) then {|wchar_token|}
    begin str_room(4); {4 is maximum}
    cur_chr:=toBUFF(cur_chr);
    if BYTE1(cur_chr)<>0 then append_char(@"100+BYTE1(cur_chr));
    if BYTE2(cur_chr)<>0 then append_char(@"100+BYTE2(cur_chr));
    if BYTE3(cur_chr)<>0 then append_char(@"100+BYTE3(cur_chr));
                              append_char(@"100+BYTE4(cur_chr));
@z

@x
@d set2=129 {typeset a character and move right}
@y
@d set2=129 {typeset a character and move right}
@d set3=130 {typeset a character and move right}
@z

@x
    jc:=toDVI(KANJI(info(p)));
    dvi_out(set2); dvi_out(Hi(jc)); dvi_out(Lo(jc));
@y
    jc:=toDVI(KANJI(info(p)) mod max_cjk_val);
    if (jc<@"10000) then begin
      dvi_out(set2);
    end else begin
      dvi_out(set3); dvi_out(BYTE2(jc));
    end;
    dvi_out(BYTE3(jc)); dvi_out(BYTE4(jc));
@z

@x
loop@+  begin get_x_token;
  reswitch: case cur_cmd of
  letter,other_char,char_given:@<Append a new letter or hyphen@>;
  char_num: begin scan_char_num; cur_chr:=cur_val; cur_cmd:=char_given;
    goto reswitch;
    end;
@y
loop@+  begin get_x_token;
  reswitch: case cur_cmd of
  letter,other_char,char_given,kchar_given:@<Append a new letter or hyphen@>;
  char_num,kchar_num: begin scan_char_num; cur_chr:=cur_val; cur_cmd:=char_given;
    goto reswitch;
    end;
@z

@x
hmode+kanji,hmode+kana,hmode+other_kchar: goto main_loop_j;
hmode+char_given:
  if is_char_ascii(cur_chr) then goto main_loop else goto main_loop_j;
hmode+char_num: begin scan_char_num; cur_chr:=cur_val;
  if is_char_ascii(cur_chr) then goto main_loop else goto main_loop_j;
  end;
hmode+no_boundary: begin get_x_token;
  if (cur_cmd=letter)or(cur_cmd=other_char)or
   (cur_cmd=kanji)or(cur_cmd=kana)or(cur_cmd=other_kchar)or
   (cur_cmd=char_given)or(cur_cmd=char_num) then cancel_boundary:=true;
@y
hmode+kanji,hmode+kana,hmode+other_kchar,hmode+hangul,hmode+kchar_given: goto main_loop_j;
hmode+char_given:
  if check_echar_range(cur_chr) then goto main_loop else goto main_loop_j;
hmode+char_num: begin scan_char_num; cur_chr:=cur_val;
  if check_echar_range(cur_chr) then goto main_loop else goto main_loop_j;
  end;
hmode+kchar_num: begin scan_char_num; cur_chr:=cur_val;
  goto main_loop_j;
  end;
hmode+no_boundary: begin get_x_token;
  if (cur_cmd=letter)or(cur_cmd=other_char)or
   ((cur_cmd>=kanji)and(cur_cmd<=hangul))or
   (cur_cmd=char_given)or(cur_cmd=char_num)or
   (cur_cmd=kchar_given)or(cur_cmd=kchar_num) then cancel_boundary:=true;
@z

@x
if cur_cmd=letter then goto main_loop_lookahead+1;
if (cur_cmd=kanji)or(cur_cmd=kana)or(cur_cmd=other_kchar) then
  @<goto |main_lig_loop|@>;
if cur_cmd=other_char then goto main_loop_lookahead+1;
if cur_cmd=char_given then
  begin if is_char_ascii(cur_chr) then goto main_loop_lookahead+1
  else @<goto |main_lig_loop|@>;
  end;
x_token; {now expand and set |cur_cmd|, |cur_chr|, |cur_tok|}
if cur_cmd=letter then goto main_loop_lookahead+1;
if (cur_cmd=kanji)or(cur_cmd=kana)or(cur_cmd=other_kchar) then
  @<goto |main_lig_loop|@>;
if cur_cmd=other_char then goto main_loop_lookahead+1;
if cur_cmd=char_given then
  begin if is_char_ascii(cur_chr) then goto main_loop_lookahead+1
  else @<goto |main_lig_loop|@>;
  end;
if cur_cmd=char_num then
  begin scan_char_num; cur_chr:=cur_val;
  if is_char_ascii(cur_chr) then goto main_loop_lookahead+1
  else @<goto |main_lig_loop|@>;
  end;
@y
if cur_cmd=letter then goto main_loop_lookahead+1;
if (cur_cmd>=kanji)and(cur_cmd<=hangul) then
  @<goto |main_lig_loop|@>;
if cur_cmd=other_char then goto main_loop_lookahead+1;
if cur_cmd=char_given then
  begin if check_echar_range(cur_chr) then goto main_loop_lookahead+1
  else @<goto |main_lig_loop|@>;
  end;
if cur_cmd=kchar_given then
  @<goto |main_lig_loop|@>;
x_token; {now expand and set |cur_cmd|, |cur_chr|, |cur_tok|}
if cur_cmd=letter then goto main_loop_lookahead+1;
if (cur_cmd>=kanji)and(cur_cmd<=hangul) then
  @<goto |main_lig_loop|@>;
if cur_cmd=other_char then goto main_loop_lookahead+1;
if cur_cmd=char_given then
  begin if check_echar_range(cur_chr) then goto main_loop_lookahead+1
  else @<goto |main_lig_loop|@>;
  end;
if cur_cmd=char_num then
  begin scan_char_num; cur_chr:=cur_val;
  if check_echar_range(cur_chr) then goto main_loop_lookahead+1
  else @<goto |main_lig_loop|@>;
  end;
if cur_cmd=kchar_num then
  begin scan_char_num; cur_chr:=cur_val;
  @<goto |main_lig_loop|@>;
  end;
@z

@x
vmode+letter,vmode+other_char,vmode+char_num,vmode+char_given,
   vmode+math_shift,vmode+un_hbox,vmode+vrule,
   vmode+accent,vmode+discretionary,vmode+hskip,vmode+valign,
   vmode+kanji,vmode+kana,vmode+other_kchar,
@y
vmode+letter,vmode+other_char,vmode+char_num,vmode+char_given,
   vmode+kchar_num,vmode+kchar_given,
   vmode+math_shift,vmode+un_hbox,vmode+vrule,
   vmode+accent,vmode+discretionary,vmode+hskip,vmode+valign,
   vmode+kanji,vmode+kana,vmode+other_kchar,vmode+hangul,
@z

@x
if not is_char_ascii(cur_val) then
  begin KANJI(cx):=cur_val;
  if direction=dir_tate then f:=cur_tfont else f:=cur_jfont;
  p:=new_character(f,get_jfm_pos(KANJI(cx),f));
  if p<>null then
    begin
      link(p):=get_avail; info(link(p)):=KANJI(cx);
    end;
@y
if check_echar_range(cur_val)=0 then
  begin KANJI(cx):=cur_val;
  if direction=dir_tate then f:=cur_tfont else f:=cur_jfont;
  p:=new_character(f,get_jfm_pos(KANJI(cx),f));
  if p<>null then
     begin
        link(p):=get_avail;
        info(link(p)):=KANJI(cx) + kcat_code(kcatcodekey(cx))*max_cjk_val;
     end;
@z

@x
if (cur_cmd=letter)or(cur_cmd=other_char) then
  q:=new_character(f,cur_chr)
else if (cur_cmd=kanji)or(cur_cmd=kana)or(cur_cmd=other_kchar) then
  begin  if direction=dir_tate then f:=cur_tfont else f:=cur_jfont;
  cx:=cur_chr;
  end
else if cur_cmd=char_given then
  if is_char_ascii(cur_chr) then q:=new_character(f,cur_chr)
  else begin
    if direction=dir_tate then f:=cur_tfont else f:=cur_jfont;
    KANJI(cx):=cur_chr
    end
  else if cur_cmd=char_num then
    begin scan_char_num;
    if is_char_ascii(cur_val) then q:=new_character(f,cur_val)
    else  begin
      if direction=dir_tate then f:=cur_tfont else f:=cur_jfont;
      KANJI(cx):=cur_val
    end
  end
@y
if (cur_cmd=letter)or(cur_cmd=other_char) then
  q:=new_character(f,cur_chr)
else if (cur_cmd>=kanji)and(cur_cmd<=hangul) then
  begin  if direction=dir_tate then f:=cur_tfont else f:=cur_jfont;
  cx:=cur_chr;
  end
else if cur_cmd=char_given then
  if check_echar_range(cur_chr) then q:=new_character(f,cur_chr)
  else begin
    if direction=dir_tate then f:=cur_tfont else f:=cur_jfont;
    KANJI(cx):=cur_chr
    end
else if cur_cmd=char_num then
  begin scan_char_num;
  if check_echar_range(cur_val) then q:=new_character(f,cur_val)
  else  begin
    if direction=dir_tate then f:=cur_tfont else f:=cur_jfont;
    KANJI(cx):=cur_val
    end
  end
else if cur_cmd=kchar_given then
  begin
    if direction=dir_tate then f:=cur_tfont else f:=cur_jfont;
    KANJI(cx):=cur_chr
  end
else if cur_cmd=kchar_num then
  begin scan_char_num;
    if direction=dir_tate then f:=cur_tfont else f:=cur_jfont;
    KANJI(cx):=cur_val
  end
@z

@x
  begin q:=new_character(f,get_jfm_pos(KANJI(cx),f));
  link(q):=get_avail; info(link(q)):=KANJI(cx); last_jchr:=q;
@y
  begin q:=new_character(f,get_jfm_pos(KANJI(cx),f));
  link(q):=get_avail;
  info(link(q)):=KANJI(cx) + kcat_code(kcatcodekey(cx))*max_cjk_val;
  last_jchr:=q;
@z

@x
letter,other_char,char_given:
  if is_char_ascii(cur_chr) then begin
@y
letter,other_char,char_given:
  if check_echar_range(cur_chr) then begin
@z

@x
    KANJI(cx):=cur_chr;
kanji,kana,other_kchar: cx:=cur_chr;
@y
    KANJI(cx):=cur_chr;
kchar_given:
  KANJI(cx):=cur_chr;
kanji,kana,other_kchar,hangul: cx:=cur_chr;
@z

@x
  math_type(p):=math_jchar; fam(p):=cur_jfam; character(p):=qi(0);
  math_kcode(p-1):=KANJI(cx);
@y
  math_type(p):=math_jchar; fam(p):=cur_jfam; character(p):=qi(0);
  math_kcode(p-1):=KANJI(cx) + kcat_code(kcatcodekey(cx))*max_cjk_val;
@z

@x
mmode+letter,mmode+other_char,mmode+char_given:
  if is_char_ascii(cur_chr) then
    set_math_char(ho(math_code(cur_chr)))
  else set_math_kchar(cur_chr);
mmode+kanji,mmode+kana,mmode+other_kchar: begin
    cx:=cur_chr; set_math_kchar(KANJI(cx));
  end;
mmode+char_num: begin scan_char_num; cur_chr:=cur_val;
  if is_char_ascii(cur_chr) then
    set_math_char(ho(math_code(cur_chr)))
  else set_math_kchar(cur_chr);
  end;
@y
mmode+letter,mmode+other_char,mmode+char_given:
  if check_echar_range(cur_chr) then
    set_math_char(ho(math_code(cur_chr)))
  else set_math_kchar(cur_chr);
mmode+kanji,mmode+kana,mmode+other_kchar,mmode+hangul: begin
    cx:=cur_chr; set_math_kchar(KANJI(cx));
  end;
mmode+char_num: begin scan_char_num; cur_chr:=cur_val;
  if check_echar_range(cur_chr) then
    set_math_char(ho(math_code(cur_chr)))
  else set_math_kchar(cur_chr);
  end;
mmode+kchar_given:
  set_math_kchar(cur_chr);
mmode+kchar_num: begin scan_char_num; cur_chr:=cur_val;
  set_math_kchar(cur_chr);
  end;
@z

@x
any_mode(assign_inhibit_xsp_code),
any_mode(set_auto_spacing),
@y
any_mode(assign_inhibit_xsp_code),
any_mode(set_auto_spacing),
any_mode(set_enable_cjk_token),
@z

@x l.23666 - upTeX
@d char_sub_def_code=7 {|shorthand_def| for \.{\\charsubdef}}

@<Put each...@>=
primitive("chardef",shorthand_def,char_def_code);@/
@!@:char_def_}{\.{\\chardef} primitive@>
@y
@d char_sub_def_code=7 {|shorthand_def| for \.{\\charsubdef}}
@d kchar_def_code=char_sub_def_code+1 {|shorthand_def| for \.{\\kchardef}}

@<Put each...@>=
primitive("chardef",shorthand_def,char_def_code);@/
@!@:char_def_}{\.{\\chardef} primitive@>
primitive("kchardef",shorthand_def,kchar_def_code);@/
@!@:kchar_def_}{\.{\\kchardef} primitive@>
@z

@x
shorthand_def: case chr_code of
  char_def_code: print_esc("chardef");
  math_char_def_code: print_esc("mathchardef");
@y
shorthand_def: case chr_code of
  char_def_code: print_esc("chardef");
  kchar_def_code: print_esc("kchardef");
  math_char_def_code: print_esc("mathchardef");
@z

@x l.23698 - upTeX
char_given: begin print_esc("char"); print_hex(chr_code);
  end;
@y
char_given: begin print_esc("char"); print_hex(chr_code);
  end;
kchar_given: begin print_esc("kchar"); print_hex(chr_code);
  end;
@z

@x l.23729 - upTeX
  char_def_code: begin scan_char_num; define(p,char_given,cur_val);
    end;
@y
  char_def_code: begin scan_char_num; define(p,char_given,cur_val);
    end;
  kchar_def_code: begin scan_char_num; define(p,kchar_given,cur_val);
    end;
@z

@x
@ @<Let |m| be the minimal...@>=
if cur_chr=kcat_code_base then m:=kanji else m:=0
@y
@ @<Let |m| be the minimal...@>=
if cur_chr=kcat_code_base then m:=not_cjk else m:=0
@z

@x
@<Change the case of the token in |p|, if a change is appropriate@>=
t:=info(p);
if (t<cs_token_flag+single_base)and(not check_kanji(t)) then
  begin c:=t mod 256;
@y
@<Change the case of the token in |p|, if a change is appropriate@>=
t:=info(p);
if (t<cs_token_flag+single_base)and(not check_kanji(t)) then
  begin c:=t mod max_char_val;
@z

@x
@d set_auto_xspacing_code=3
@y
@d set_auto_xspacing_code=3
@d reset_enable_cjk_token_code=0
@d set_enable_cjk_token_code=1
@d set_force_cjk_token_code=2
@z

@x
@!@:no_auto_xspacing_}{\.{\\noautoxspacing} primitive@>
@y
@!@:no_auto_xspacing_}{\.{\\noautoxspacing} primitive@>
primitive("enablecjktoken",set_enable_cjk_token,reset_enable_cjk_token_code);
@!@:enable_cjk_token_}{\.{\\enablecjktoken} primitive@>
primitive("disablecjktoken",set_enable_cjk_token,set_enable_cjk_token_code);
@!@:disable_cjk_token_}{\.{\\disablecjktoken} primitive@>
primitive("forcecjktoken",set_enable_cjk_token,set_force_cjk_token_code);
@!@:force_cjk_token_}{\.{\\forcecjktoken} primitive@>
@z

@x
  if (chr_code mod 2)=0 then print_esc("noauto") else print_esc("auto");
  if chr_code<2 then print("spacing") else print("xspacing");
end;
@y
  if (chr_code mod 2)=0 then print_esc("noauto") else print_esc("auto");
  if chr_code<2 then print("spacing") else print("xspacing");
end;
set_enable_cjk_token:begin
  if chr_code=0 then print_esc("enable")
  else if chr_code=1 then print_esc("disable") else print_esc("force");
  print("cjktoken");
end;
@z

@x
  else begin p:=auto_xspacing_code; cur_chr:=(cur_chr mod 2); end;
  define(p,data,cur_chr);
end;
@y
  else begin p:=auto_xspacing_code; cur_chr:=(cur_chr mod 2); end;
  define(p,data,cur_chr);
end;
set_enable_cjk_token: define(enable_cjk_token_code,data,cur_chr);
@z

@x
@ @<Insert a space around the character |p|@>=
if font_dir[font(p)]<>dir_default then
  begin KANJI(cx):=info(link(p));
@y
@ @<Insert a space around the character |p|@>=
if font_dir[font(p)]<>dir_default then
  begin KANJI(cx):=info(link(p)) mod max_cjk_val;
@z

@x
else if font_dir[font(first_char)]<>dir_default then
  begin KANJI(cx):=info(link(first_char));
@y
else if font_dir[font(first_char)]<>dir_default then
  begin KANJI(cx):=info(link(first_char)) mod max_cjk_val;
@z

@x
begin if is_char_node(link(p)) then
  begin q:=p; p:=link(p);
  if font_dir[font(p)]<>dir_default then
    begin KANJI(cx):=info(link(p));
@y
begin if is_char_node(link(p)) then
  begin q:=p; p:=link(p);
  if font_dir[font(p)]<>dir_default then
    begin KANJI(cx):=info(link(p)) mod max_cjk_val;
@z

@x
    begin KANJI(cx):=info(link(p)); i:=kcat_code(kcatcodekey(cx)); k:=0;
    if (i=kanji)or(i=kana) then begin t:=q; s:=p; end;
@y
    begin KANJI(cx):=info(link(p)) mod max_cjk_val;
    i:=info(link(p)) div max_cjk_val; k:=0;
    if (i=kanji)or(i=kana)or(i=hangul) then begin t:=q; s:=p; end;
@z

@x
    fast_get_avail(main_p); info(main_p):=KANJI(cur_chr);
@y
    fast_get_avail(main_p); info(main_p):=KANJI(cur_chr)+cur_cmd*max_cjk_val;
@z

@x
  main_i:=orig_char_info(main_f)(cur_l);
  case cur_cmd of
    kanji,kana,other_kchar: begin
@y
  main_i:=orig_char_info(main_f)(cur_l);
  case cur_cmd of
    kanji,kana,other_kchar,hangul: begin
@z

@x
    kanji,kana,other_kchar: cur_l:=qi(get_jfm_pos(KANJI(cur_chr),main_f));
    letter,other_char: begin ins_kp:=true; cur_l:=qi(0); end;
    char_given: begin
      if is_char_ascii(cur_chr) then
        begin ins_kp:=true; cur_l:=qi(0);
        end
      else cur_l:=qi(get_jfm_pos(KANJI(cur_chr),main_f));
      end;
    char_num: begin scan_char_num; cur_chr:=cur_val;
      if is_char_ascii(cur_chr) then
        begin ins_kp:=true; cur_l:=qi(0);
        end
      else cur_l:=qi(get_jfm_pos(KANJI(cur_chr),main_f));
      end;
@y
    kanji,kana,other_kchar,hangul: cur_l:=qi(get_jfm_pos(KANJI(cur_chr),main_f));
    letter,other_char: begin ins_kp:=true; cur_l:=qi(0); end;
    char_given: begin
      if check_echar_range(cur_chr) then
        begin ins_kp:=true; cur_l:=qi(0);
        end
      else cur_l:=qi(get_jfm_pos(KANJI(cur_chr),main_f));
      end;
    char_num: begin scan_char_num; cur_chr:=cur_val;
      if check_echar_range(cur_chr) then
        begin ins_kp:=true; cur_l:=qi(0);
        end
      else cur_l:=qi(get_jfm_pos(KANJI(cur_chr),main_f));
      end;
    kchar_given: begin
      cur_l:=qi(get_jfm_pos(KANJI(cur_chr),main_f));
      end;
    kchar_num: begin scan_char_num; cur_chr:=cur_val;
      cur_l:=qi(get_jfm_pos(KANJI(cur_chr),main_f));
      end;
@z

@x
procedure print_kanji(@!s:KANJI_code); {prints a single character}
begin
if s>@"FF then
  begin print_char(@"100+Hi(s)); print_char(@"100+Lo(s));
  end else print_char(s);
@y
procedure print_kanji(@!s:KANJI_code); {prints a single character}
begin
s:=toBUFF(s mod max_cjk_val);
if BYTE1(s)<>0 then print_char(@"100+BYTE1(s));
if BYTE2(s)<>0 then print_char(@"100+BYTE2(s));
if BYTE3(s)<>0 then print_char(@"100+BYTE3(s));
                    print_char(@"100+BYTE4(s));
end;

function check_kcat_code(@!ct:integer):integer;
begin
if ((ct>=kanji)and(enable_cjk_token=0))or(enable_cjk_token=2)then
  check_kcat_code:=1
else check_kcat_code:=0;
end;

function check_echar_range(@!c:integer):integer;
begin
if (c>=0)and(c<256)then
  check_echar_range:=1
else check_echar_range:=0;
@z
