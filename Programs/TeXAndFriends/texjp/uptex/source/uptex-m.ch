% $Id$
% This is a change file for upTeX u2.00
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
% (2022-07-23) TTK  upTeX u1.29
% (2022-12-09) HK   Hironori Kitagawa fixed a bug in \char, \kchar.
% (2023-09-16) TTK  upTeX u1.30
% (2024-09-30) TTK  upTeX u1.35
% (2024-10-20) TTK  upTeX u2.00
% (2025-01-02) TTK  Accept extended upTeX internal encoding also in pTeX-compatible EUC/SJIS mode.

@x
\def\pTeX{p\kern-.15em\TeX}
@y
\def\pTeX{p\kern-.15em\TeX}
\def\upTeX{u\pTeX}
@z

@x upTeX: banner
  {printed when \pTeX\ starts}
@y
  {printed when \pTeX\ starts}
@#
@d upTeX_version=2
@d upTeX_revision==".00"
@d upTeX_version_string=='-u2.00' {current \upTeX\ version}
@#
@d upTeX_banner=='This is upTeX, Version 3.141592653',pTeX_version_string,upTeX_version_string
@d upTeX_banner_k==upTeX_banner
  {printed when \upTeX\ starts}
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
@d max_quarterword=@"FFFF {largest allowable value in a |quarterword|}
@d min_halfword==-@"FFFFFFF {smallest allowable value in a |halfword|}
@d max_halfword==@"FFFFFFF {largest allowable value in a |halfword|}
@d max_cjk_val=@"10000
@y
@d min_quarterword=0 {smallest allowable value in a |quarterword|}
@d max_quarterword=@"FFFF {largest allowable value in a |quarterword|}
@d min_halfword=-@"3FFFFFFF {smallest allowable value in a |halfword|}
@d max_halfword=@"3FFFFFFF {largest allowable value in a |halfword|}
@d max_cjk_val=@"1000000 {to separate wchar and kcatcode}
@d max_ivs_val=@"4400000 {to separate wchar with ivs and kcatcode}
@d max_ucs_val=@"110000 {largest Unicode Scalar Value}
@d max_latin_val=@"2E80
@z

@x
@p function new_ligature(@!f:internal_font_number; @!c:quarterword;
                         @!q:pointer):pointer;
var p:pointer; {the new node}
begin p:=get_node(small_node_size); type(p):=ligature_node;
font(lig_char(p)):=f; character(lig_char(p)):=c; lig_ptr(p):=q;
subtype(p):=0; new_ligature:=p;
end;
@y
@p function new_ligature(@!f:internal_font_number; @!c:quarterword;
                         @!q:pointer):pointer;
var p:pointer; {the new node}
begin p:=get_node(small_node_size);
c:=ptenc8bitcodetoucs(font_enc[f],c);
type(p):=ligature_node;
font(lig_char(p)):=f; character(lig_char(p)):=c; lig_ptr(p):=q;
subtype(p):=0; new_ligature:=p;
end;
@z

@x
function new_lig_item(@!c:quarterword):pointer;
var p:pointer; {the new node}
begin p:=get_node(small_node_size); character(p):=c; lig_ptr(p):=null;
@y
function new_lig_item(@!c:quarterword):pointer;
var p:pointer; {the new node}
begin p:=get_node(small_node_size);
c:=ptenc8bitcodetoucs(font_enc[f],c);
character(p):=c; lig_ptr(p):=null;
@z

@x
      if font_dir[font(p)]<>dir_default then
        begin p:=link(p); print_kanji(info(p));
        end
      else print_ASCII(qo(character(p)));
@y
      if font_dir[font(p)]<>dir_default then
        begin p:=link(p); print_kanji(info(p));
        end
      else if check_echar_range(qo(character(p)))=1 then
        print_kanji(qo(character(p)))
      else print_ASCII(qo(character(p)));
@z

@x
  print_char(" ");
  if font_dir[font(p)]<>dir_default then
    begin p:=link(p); print_kanji(info(p));
    end
  else print_ASCII(qo(character(p)));
@y
  print_char(" ");
  if font_dir[font(p)]<>dir_default then
    begin p:=link(p); print_kanji(info(p));
    end
  else if check_echar_range(qo(character(p)))=1 then
    print_kanji(qo(character(p)))
  else print_ASCII(qo(character(p)));
@z

@x
@d kanji=16 {kanji}
@d kana=17 {hiragana, katakana, alphabet}
@d other_kchar=18 {kanji codes}
@d max_char_code=18 {largest catcode for individual characters}
@y
@d latin_ucs=14 {is not cjk characters in ucs code}
@d not_cjk=15 {is not cjk characters}
@d kanji=16 {kanji}
@d kana=17 {hiragana, katakana, alphabet}
@d other_kchar=18 {cjk symbol codes}
@d hangul=19 {hangul codes}
@d modifier=20 {modifier codes}
@d kanji_ivs=23 {kanji with ivs codes}
@d max_char_code=20 {largest catcode for individual characters}
@z

@x
@d char_num=max_char_code+1 {character specified numerically ( \.{\\char} )}
@d math_char_num=char_num+1 {explicit math code ( \.{\\mathchar} )}
@y
@d char_num=max_char_code+1 {character specified numerically ( \.{\\char} )}
@d kchar_num=char_num+1 {cjk character specified numerically ( \.{\\kchar} )}
@d math_char_num=kchar_num+1 {explicit math code ( \.{\\mathchar} )}
@z

@x
@d math_given=char_given+1 {math code defined by \.{\\mathchardef}}
@y
@d kchar_given=char_given+1 {cjk character code defined by \.{\\kchardef}}
@d math_given=kchar_given+1 {math code defined by \.{\\mathchardef}}
@z

@x
@d partoken_name=set_auto_spacing+1 {set |par_token| name}
@y
@d set_enable_cjk_token=set_auto_spacing+1 {set cjk mode
  ( \.{\\enablecjktoken}, \.{\\disablecjktoken}, \.{\\forcecjktoken} )}
@d partoken_name=set_enable_cjk_token+1 {set |par_token| name}
@z

@x
@d single_base=active_base+256 {equivalents of one-character control sequences}
@d null_cs=single_base+256 {equivalent of \.{\\csname\\endcsname}}
@y
@d single_base=active_base+max_latin_val {equivalents of one-character control sequences}
@d null_cs=single_base+max_latin_val {equivalent of \.{\\csname\\endcsname}}
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
  {table of |max_latin_val| command codes (the ``catcodes'')}
@d kcat_code_base=cat_code_base+max_latin_val
  {table of 512 command codes for the wchar's catcodes }
@d auto_xsp_code_base=kcat_code_base+512 {table of 256 auto spacer flag}
@z

@x
@d lc_code_base=kansuji_base+10 {table of 256 lowercase mappings}
@d uc_code_base=lc_code_base+256 {table of 256 uppercase mappings}
@d sf_code_base=uc_code_base+256 {table of 256 spacefactor mappings}
@d math_code_base=sf_code_base+256 {table of 256 math mode mappings}
@d char_sub_code_base=math_code_base+256 {table of character substitutions}
@d int_base=char_sub_code_base+256 {beginning of region 5}
@y
@d lc_code_base=kansuji_base+10 {table of |max_latin_val| lowercase mappings}
@d uc_code_base=lc_code_base+max_latin_val {table of |max_latin_val| uppercase mappings}
@d sf_code_base=uc_code_base+max_latin_val {table of |max_latin_val| spacefactor mappings}
@d math_code_base=sf_code_base+max_latin_val {table of 256 math mode mappings}
@d char_sub_code_base=math_code_base+256 {table of character substitutions}
@d int_base=char_sub_code_base+256 {beginning of region 5}
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
  auto_xsp_code(k):=0;
  inhibit_xsp_type(k):=0; kinsoku_type(k):=0;
  end;
@y
eqtb[auto_xspacing_code]:=eqtb[cat_code_base];
eqtb[enable_cjk_token_code]:=eqtb[cat_code_base];
for k:=0 to 255 do
  begin cat_code(k):=other_char;
  math_code(k):=hi(k);
  auto_xsp_code(k):=0;
  inhibit_xsp_type(k):=0; kinsoku_type(k):=0;
  end;
for k:=0 to 511 do
  begin kcat_code(k):=other_kchar;
  end;
for k:=0 to max_latin_val-1 do
  begin sf_code(k):=1000;
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
  @+@t\1@>for k:=@"68 to @"69 do kcat_code(k):=kanji; { CJK Radicals Supplement .. Kangxi Radicals }
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
  @t\hskip10pt@>kcat_code(@"9C):=modifier; { Variation Selectors }
  { \hskip10pt|kcat_code(@"A2):=other_kchar;| Halfwidth and Fullwidth Forms }
  @+@t\1@>for k:=@"115 to @"118 do kcat_code(k):=kana; { Kana Extended-B .. Small Kana Extension }
  @+@t\1@>for k:=@"145 to @"14F do kcat_code(k):=kanji; { CJK Unified Ideographs Extension B .. J }
  @t\hskip10pt@>kcat_code(@"15B):=modifier; { Variation Selectors Supplement }
  @+@t\1@>for k:=@"1F9 to @"1FC do kcat_code(k):=modifier;
    { Combining Katakana-Hiragana (Semi-)Voiced Sound Mark .. Emoji Modifier Fitzpatrick }
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
@+@t\1@>for k:=@"15F to @"162 do kcat_code(k):=kanji; { for japanese-otf, japanese-otf-uptex }
@+@t\1@>for k:=@"170 to @"171 do kcat_code(k):=kana; { Kana with (Semi-)Voiced Sound Mark }
@t\hskip10pt@>kcat_code(@"175):=kanji; { Standardized Variation Sequence }
@+@t\1@>for k:=@"177 to @"178 do kcat_code(k):=kanji; { Ideographic Variation Sequence }
@z

@x
@d enc_jis=1
@d enc_ucs=2
@y
@d enc_jis=1
@d enc_ucs=2
@d enc_t1=@"80
@d enc_ts1=@"81
@d enc_ly1=@"82
@d enc_t5=@"83
@d enc_l7x=@"84
@d enc_t2a=@"90
@d enc_t2b=@"91
@d enc_t2c=@"92
@d enc_lgr=@"A0
@z

@x
begin if s<256 then cur_val:=s+single_base
@y
begin if s<256 then cur_val:=s+single_base
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
primitive("relax",relax,256); {cf.\ |scan_file_name|}
@y
primitive("relax",relax,max_cjk_val); {cf.\ |scan_file_name|}
@z

@x
char_num: print_esc("char");
@y
char_num: print_esc("char");
kchar_num: print_esc("kchar");
@z

@x
  if font_enc[f]=enc_ucs then print("+Unicode")
  else if font_enc[f]=enc_jis then print("+JIS");
@y
  if font_enc[f]=enc_ucs then print("+Unicode")
  else if font_enc[f]=enc_jis then print("+JIS")
  else if font_enc[f]=enc_t1  then print("+T1")
  else if font_enc[f]=enc_ts1 then print("+TS1")
  else if font_enc[f]=enc_ly1 then print("+LY1")
  else if font_enc[f]=enc_t5  then print("+T5")
  else if font_enc[f]=enc_l7x then print("+L7x")
  else if font_enc[f]=enc_t2a then print("+T2A")
  else if font_enc[f]=enc_t2b then print("+T2B")
  else if font_enc[f]=enc_t2c then print("+T2C")
  else if font_enc[f]=enc_lgr then print("+LGR");
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
@d max_char_val=@"10000 {to separate char and command code}
@d left_brace_token=@"10000 {$2^16\cdot|left_brace|$}
@d left_brace_limit=@"20000 {$2^16\cdot(|left_brace|+1)$}
@d right_brace_token=@"20000 {$2^16\cdot|right_brace|$}
@d right_brace_limit=@"30000 {$2^16\cdot(|right_brace|+1)$}
@d math_shift_token=@"30000 {$2^16\cdot|math_shift|$}
@d tab_token=@"40000 {$2^16\cdot|tab_mark|$}
@d out_param_token=@"50000 {$2^16\cdot|out_param|$}
@d space_token=@"A0020 {$2^16\cdot|spacer|+|" "|$}
@d letter_token=@"B0000 {$2^16\cdot|letter|$}
@d other_token=@"C0000 {$2^16\cdot|other_char|$}
@d match_token=@"D0000 {$2^16\cdot|match|$}
@d end_match_token=@"E0000 {$2^16\cdot|end_match|$}
@z

@x
procedure show_token_list(@!p,@!q:integer;@!l:integer);
label exit;
var m,@!c:integer; {pieces of a token}
@!match_chr:ASCII_code; {character used in a `|match|'}
@y
procedure show_token_list(@!p,@!q:integer;@!l:integer);
label exit;
var m,@!c:integer; {pieces of a token}
@!match_chr:0..max_latin_val; {character used in a `|match|'}
@z

@x
  if check_kanji(info(p)) then {|wchar_token|}
    begin m:=kcat_code(kcatcodekey(info(p))); c:=info(p);
    end
  else  begin m:=Hi(info(p)); c:=Lo(info(p));
    end;
  if (m<kanji)and(c>256) then print_esc("BAD.")
@.BAD@>
@y
  if check_kanji(info(p)) then {|wchar_token|}
    begin
      m:=ktokentocmd(info(p));
      c:=ktokentochr(info(p));
    end
  else  begin m:=info(p) div max_char_val; c:=info(p) mod max_char_val;
    end;
  if (m<kanji)and(c>=max_latin_val) then print_esc("BAD.")
@.BAD@>
@z

@x
@<Display the token ...@>=
case m of
kanji,kana,other_kchar: print_kanji(KANJI(c));
left_brace,right_brace,math_shift,tab_mark,sup_mark,sub_mark,spacer,
  letter,other_char: print(c);
mac_param: begin print(c); print(c);
  end;
out_param: begin print(match_chr);
  if c<=9 then print_char(c+"0")
  else  begin print_char("!"); return;
    end;
  end;
match: begin match_chr:=c; print(c); incr(n); print_char(n);
  if n>"9" then return;
  end;
@y
@<Display the token ...@>=
case m of
kanji,kana,other_kchar,hangul,modifier: print_kanji(KANJI(c));
left_brace,right_brace,math_shift,tab_mark,sup_mark,sub_mark,spacer,
  letter,other_char: if (check_echar_range(c)=1)or(check_mchar_range(c))
    then print_kanji(KANJI(c)) else print(c);
mac_param: begin if (check_echar_range(c)=1)or(check_mchar_range(c))
    then begin print_kanji(KANJI(c)); print_kanji(KANJI(c)); end
    else begin print(c); print(c); end
  end;
out_param: begin
  if (check_echar_range(match_chr)=1)or(check_mchar_range(match_chr))
    then print_kanji(KANJI(match_chr)) else print(match_chr);
  if c<=9 then print_char(c+"0")
  else  begin print_char("!"); return;
    end;
  end;
match: begin match_chr:=c;
  if (check_echar_range(c)=1)or(check_mchar_range(c))
    then print_kanji(KANJI(c)) else print(c);
  incr(n); print_char(n);
  if n>"9" then return;
  end;
@z

@x
@d chr_cmd(#)==begin print(#); print_ASCII(chr_code);
  end
@y
@d chr_cmd(#)==begin print(#);
   if chr_code < @"100 then print_ASCII(chr_code)
   else print_kanji(chr_code);
  end
@z

@x
kanji,kana,other_kchar: begin print("kanji character ");
@y
kanji,kana,other_kchar,hangul,modifier: begin print("kanji character ");
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
      s:=get_avail; info(s):=ktokentochr(info(loc));
@z

@x
primitive("par",par_end,256); {cf.\ |scan_file_name|}
@y
primitive("par",par_end,max_cjk_val); {cf.\ |scan_file_name|}
@z

@x
@!c,@!cc:ASCII_code; {constituents of a possible expanded code}
@!d:2..3; {number of excess characters in an expanded code}
@y
@!c,@!cc,@!cd,@!ce:ASCII_code; {constituents of a possible expanded code}
@!d:2..7; {number of excess characters in an expanded code}
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
    if (multistrlen(ustringcast(buffer), limit+1, loc)>1) and check_kcat_code(cur_cmd,cur_chr) then begin
      if (cur_cmd=latin_ucs) then cur_cmd:=cat_code(cur_chr);
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
@d all_jcode(#)==#+kanji,#+kana,#+other_kchar,#+modifier
@d hangul_code(#)==#+hangul
@z

@x
all_jcode(skip_blanks),all_jcode(skip_blanks_kanji),all_jcode(new_line),
all_jcode(mid_line):
  state:=mid_kanji;
@y
all_jcode(skip_blanks),all_jcode(skip_blanks_kanji),all_jcode(new_line),
all_jcode(mid_line):
  state:=mid_kanji;
hangul_code(skip_blanks),hangul_code(skip_blanks_kanji),hangul_code(new_line),
hangul_code(mid_kanji):
  state:=mid_line;
@z

@x
@d hex_to_cur_chr==
  if c<="9" then cur_chr:=c-"0" @+else cur_chr:=c-"a"+10;
  if cc<="9" then cur_chr:=16*cur_chr+cc-"0"
  else cur_chr:=16*cur_chr+cc-"a"+10
@y
@d hex_to_cur_chr==
  if c<="9" then cur_chr:=c-"0" @+else cur_chr:=c-"a"+10;
  if cc<="9" then cur_chr:=16*cur_chr+cc-"0"
  else cur_chr:=16*cur_chr+cc-"a"+10
@d long_hex_to_cur_chr==
  if c<="9" then cur_chr:=c-"0" @+else cur_chr:=c-"a"+10;
  if cc<="9" then cur_chr:=16*cur_chr+cc-"0"
  else cur_chr:=16*cur_chr+cc-"a"+10;
  if cd<="9" then cur_chr:=16*cur_chr+cd-"0"
  else cur_chr:=16*cur_chr+cd-"a"+10;
  if ce<="9" then cur_chr:=16*cur_chr+ce-"0"
  else cur_chr:=16*cur_chr+ce-"a"+10
@z

@x
  begin c:=buffer[loc+1]; @+if c<@'200 then {yes we have an expanded char}
@y
  begin if (cur_chr=buffer[loc+1]) and (cur_chr=buffer[loc+2]) and
           ((loc+6)<=limit) then
     begin c:=buffer[loc+3]; cc:=buffer[loc+4];
       cd:=buffer[loc+5]; ce:=buffer[loc+6];
       if is_hex(c) and is_hex(cc) and is_hex(cd) and is_hex(ce) then
       begin loc:=loc+7; long_hex_to_cur_chr; goto reswitch;
       end
     end;
  c:=buffer[loc+1]; @+if c<@'200 then {yes we have an expanded char}
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
  if cat=letter then state:=skip_blanks
@y
else  begin k:=loc;
  cur_chr:=fromBUFF(ustringcast(buffer), limit+1, k);
  cat:=kcat_code(kcatcodekey(cur_chr));
  if (multistrlen(ustringcast(buffer), limit+1, k)>1) and check_kcat_code(cat,cur_chr) then begin
    if (cat=latin_ucs)or(cat=not_cjk) then cat:=other_kchar;
    for l:=k to k-1+multistrlen(ustringcast(buffer), limit+1, k) do
      buffer2[l]:=1;
    k:=k+multistrlen(ustringcast(buffer), limit+1, k) end
  else begin {not multi-byte char}
    cur_chr:=buffer[k];
    cat:=cat_code(cur_chr);
    incr(k);
  end;
start_cs:
  if (cat=letter)or(cat=hangul) then state:=skip_blanks
@z

@x
  else if ((cat=letter)or(cat=kanji)or(cat=kana))and(k<=limit) then
@y
  else if ((cat=letter)or(cat=kanji)or(cat=kana)or(cat=hangul)or(cat=modifier))and(k<=limit) then
@z

@x
  if (cat=kanji)or(cat=kana) then
@y
  if (cat=kanji)or(cat=kana)or(cat=hangul)or(cat=modifier) then
@z

@x
begin repeat cur_chr:=buffer[k]; incr(k);
  if multistrlen(ustringcast(buffer), limit+1, k-1)=2 then
    begin cat:=kcat_code(kcatcodekey(fromBUFF(ustringcast(buffer), limit+1, k-1)));
    for l:=k-1 to k-2+multistrlen(ustringcast(buffer), limit+1, k-1) do
      buffer2[l]:=1;
    incr(k);
    if (cat=kanji)or(cat=kana) then
      begin if (ptex_lineend mod 2)=0 then state:=skip_blanks_kanji
      else state:=skip_blanks end;
    end
  else cat:=cat_code(cur_chr);
@y
begin repeat
  cur_chr:=fromBUFF(ustringcast(buffer), limit+1, k);
  cat:=kcat_code(kcatcodekey(cur_chr));
  if (multistrlen(ustringcast(buffer), limit+1, k)>1) and check_kcat_code(cat,cur_chr) then begin
    if (cat=latin_ucs) then cat:=cat_code(cur_chr);
    if (cat=not_cjk) then cat:=other_kchar;
    for l:=k to k-1+multistrlen(ustringcast(buffer), limit+1, k) do
      buffer2[l]:=1;
    k:=k+multistrlen(ustringcast(buffer), limit+1, k);
    if (cat=kanji)or(cat=kana) then
      begin if (ptex_lineend mod 2)=0 then state:=skip_blanks_kanji
      else state:=skip_blanks end
    else if cat=hangul then state:=skip_blanks;
    end
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
until not((cat=letter)or(cat=kanji)or(cat=kana)or(cat=hangul)or(cat=modifier))or(k>limit);
{@@<If an expanded...@@>;}
if not((cat=letter)or(cat=kanji)or(cat=kana)or(cat=hangul)or(cat=modifier)) then begin
  if (cat=mac_param)and(buffer2[k-1])and(multilenbuffchar(cur_chr)>1) then k:=k-multilenbuffchar(cur_chr)
  else decr(k);
  end;
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
    begin
      cur_cmd:=ktokentocmd(t);
      cur_chr:=ktokentochr(t);
    end
  else
    begin cur_cmd:=t div max_char_val; cur_chr:=t mod max_char_val;
@z

@x
@d no_expand_flag=257 {this characterizes a special variant of |relax|}
@y
@d no_expand_flag=max_cjk_val+1 {this characterizes a special variant of |relax|}
@z

@x get_token
  if (cur_cmd=kanji)or(cur_cmd=kana)or(cur_cmd=other_kchar) then {|wchar_token|}
    cur_tok:=cur_chr
  else cur_tok:=(cur_cmd*@'400)+cur_chr
@y
  if (cur_cmd>=kanji)and(cur_cmd<=modifier) then {|wchar_token|}
    if (cur_cmd=kanji)and(cur_chr>=max_cjk_val) then
      cur_tok:=(kanji_ivs*max_cjk_val)+cur_chr
    else
      cur_tok:=(cur_cmd*max_cjk_val)+cur_chr
  else if (cur_cmd=latin_ucs) then
      cur_tok:=(cat_code(cur_chr)*max_cjk_val)+cur_chr
  else if (check_echar_range(cur_chr)=1) then
      cur_tok:=(cur_cmd*max_cjk_val)+cur_chr
  else cur_tok:=(cur_cmd*max_char_val)+cur_chr
@z

@x
  begin eq_define(cur_cs,relax,256); {N.B.: The |save_stack| might change}
@y
  begin eq_define(cur_cs,relax,max_cjk_val); {N.B.: The |save_stack| might change}
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

@x get_x_token
  if (cur_cmd=kanji)or(cur_cmd=kana)or(cur_cmd=other_kchar) then
    cur_tok:=cur_chr
  else cur_tok:=(cur_cmd*@'400)+cur_chr
@y
  if (cur_cmd>=kanji)and(cur_cmd<=modifier) then
    if (cur_cmd=kanji)and(cur_chr>=max_cjk_val) then
      cur_tok:=(kanji_ivs*max_cjk_val)+cur_chr
    else
      cur_tok:=(cur_cmd*max_cjk_val)+cur_chr
  else if (cur_cmd=latin_ucs) then
      cur_tok:=(cat_code(cur_chr)*max_cjk_val)+cur_chr
  else if (check_echar_range(cur_chr)=1) then
      cur_tok:=(cur_cmd*max_cjk_val)+cur_chr
  else cur_tok:=(cur_cmd*max_char_val)+cur_chr
@z

@x x_token
  if (cur_cmd=kanji)or(cur_cmd=kana)or(cur_cmd=other_kchar) then
    cur_tok:=cur_chr
  else cur_tok:=(cur_cmd*@'400)+cur_chr
@y
  if (cur_cmd>=kanji)and(cur_cmd<=modifier) then
    if (cur_cmd=kanji)and(cur_chr>=max_cjk_val) then
      cur_tok:=(kanji_ivs*max_cjk_val)+cur_chr
    else
      cur_tok:=(cur_cmd*max_cjk_val)+cur_chr
  else if (cur_cmd=latin_ucs) then
      cur_tok:=(cat_code(cur_chr)*max_cjk_val)+cur_chr
  else if (check_echar_range(cur_chr)=1) then
      cur_tok:=(cur_cmd*max_cjk_val)+cur_chr
  else cur_tok:=(cur_cmd*max_char_val)+cur_chr
@z

@x
if (info(r)>match_token+255)or(info(r)<match_token) then s:=null
@y
if (info(r)>=match_token+max_latin_val)or(info(r)<match_token) then s:=null
@z

@x
char_given,math_given: scanned_result(cur_chr)(int_val);
@y
kchar_given,
char_given,math_given: scanned_result(cur_chr)(int_val);
@z

@x
else if m<math_code_base then { \.{\\lccode}, \.{\\uccode}, \.{\\sfcode}, \.{\\catcode} }
  begin scan_ascii_num;
  scanned_result(equiv(m+cur_val))(int_val) end
@y
else if m<math_code_base then { \.{\\lccode}, \.{\\uccode}, \.{\\sfcode}, \.{\\catcode} }
  begin scan_latin_num;
  scanned_result(equiv(m+cur_val))(int_val) end
@z

@x
@d ptex_minor_version_code=ptex_version_code+1 {code for \.{\\ptexminorversion}}
@y
@d uptex_version_code=ptex_version_code+1 {code for \.{\\uptexversion}}
@d ptex_minor_version_code=uptex_version_code+1 {code for \.{\\ptexminorversion}}
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
procedure scan_ascii_num;
begin scan_int;
if (cur_val<0)or(cur_val>255) then
  begin print_err("Bad character code");
@.Bad character code@>
  help2("A character number must be between 0 and 255.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;
@y
procedure scan_ascii_num;
begin scan_int;
if (cur_val<0)or(cur_val>255) then
  begin print_err("Bad character code");
@.Bad character code@>
  help2("A character number must be between 0 and 255.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;
procedure scan_latin_num;
begin scan_int;
if not is_char_ascii(cur_val) then
  begin print_err("Bad character code");
@.Bad character code@>
  help2("A character number must be between 0 and ""2E7F.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;
@z

@x
  if (cur_cmd=kanji)or(cur_cmd=kana)or(cur_cmd=other_kchar) then {|wchar_token|}
@y
  if (cur_cmd>=kanji)and(cur_cmd<=modifier) then {|wchar_token|}
@z

@x
    if str_start[m+1]-str_start[m]
       = multistrlenshort(str_pool, str_start[m+1], str_start[m])
       then
      cur_val:=fromBUFFshort(str_pool, str_start[m+1], str_start[m])
    else begin cur_cmd:=invalid_char; cur_val:=256; end;
  end;
if (cur_val>255)and(cur_cmd<kanji) then
  begin print_err("Improper alphabetic or KANJI constant");
@y
    if str_start[m+1]-str_start[m]
       = multistrlenshort(str_pool, str_start[m+1], str_start[m])
       then
      cur_val:=fromBUFFshort(str_pool, str_start[m+1], str_start[m])
    else begin cur_cmd:=invalid_char; cur_val:=max_cjk_val; end;
  end;
if (cur_val>=max_latin_val)and(cur_cmd<kanji) then
  begin print_err("Improper alphabetic or KANJI constant");
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
    if (cc=latin_ucs) then cc:=other_char;
    if (cc=not_cjk) then cc:=other_kchar;
    if (cc=kanji)and(t>=max_cjk_val) then cc:=kanji_ivs;
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
@d ptex_revision_code=14 {command code for \.{\\ptexrevision}}
@d ptex_convert_codes=15 {end of \pTeX's command codes}
@y
@d ptex_revision_code=14 {command code for \.{\\ptexrevision}}
@d uptex_revision_code=15 {command code for \.{\\uptexrevision}}
@d ptex_convert_codes=16 {end of \pTeX's command codes}
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
  if (cur_cmd>=kanji)and(cur_cmd<=modifier) then {|wchar_token|}
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
  begin m:=cur_cmd; n:=cur_chr;
  end
else if (cur_cmd>active_char)or(cur_chr>255) then
@y
if (cur_cmd>=kanji)and(cur_cmd<=modifier) then
  begin m:=cur_cmd; n:=cur_chr;
  end
else if (cur_cmd>active_char)or(cur_chr>=max_latin_val) then
@z

@x
get_x_token_or_active_char;
if (cur_cmd=kanji)or(cur_cmd=kana)or(cur_cmd=other_kchar) then
  begin cur_cmd:=cur_cmd;
  end {dummy}
else if (cur_cmd>active_char)or(cur_chr>255) then
@y
get_x_token_or_active_char;
if (cur_cmd>=kanji)and(cur_cmd<=modifier) then
  begin cur_cmd:=cur_cmd;
  end {dummy}
else if (cur_cmd>active_char)or(cur_chr>=max_latin_val) then
@z

@x
@p procedure scan_file_name;
label done;
var
  @!save_warning_index: pointer;
begin
@y
@p procedure scan_file_name;
label done;
var
  @!save_warning_index: pointer;
  @!v,@!nn,@!jj: integer;
begin
@z

@x
  if (cur_cmd=kanji)or(cur_cmd=kana)or(cur_cmd=other_kchar) then {is kanji}
    begin str_room(2);
    append_char(@"100+Hi(cur_chr)); {kanji upper byte}
    append_char(@"100+Lo(cur_chr)); {kanji lower byte}
    end
  else if (cur_cmd>other_char)or(cur_chr>255) then {not an alphabet}
@y
  if (cur_cmd>=kanji)and(cur_cmd<=modifier) then {|wchar_token|}
    begin
    if (isinternalUPTEX) then begin
      cur_chr:=toUCS(cur_chr);
      nn:=UVSgetcodepointlength(cur_chr);
      jj:=1;
      while jj<=nn do begin
        v:=UVSgetcodepointinsequence(cur_chr,jj);
        if (v>0) then begin
          str_room(4);
          v:=UCStoUTF8(v);
          if BYTE1(v)<>0 then append_char(@"100+BYTE1(v));
          if BYTE2(v)<>0 then append_char(@"100+BYTE2(v));
          if BYTE3(v)<>0 then append_char(@"100+BYTE3(v));
                              append_char(@"100+BYTE4(v));
          end;
        incr(jj);
        end
      end
    else begin
      str_room(4); {4 is maximum}
      cur_chr:=toBUFF(cur_chr);
      if BYTE1(cur_chr)<>0 then append_char(@"100+BYTE1(cur_chr));
      if BYTE2(cur_chr)<>0 then append_char(@"100+BYTE2(cur_chr));
      if BYTE3(cur_chr)<>0 then append_char(@"100+BYTE3(cur_chr));
                                append_char(@"100+BYTE4(cur_chr));
      end;
    end
  else if (cur_cmd>other_char)or(cur_chr>=max_latin_val) then {not an alphabet}
@z

@x
@d non_char==qi(256) {a |halfword| code that can't match a real character}
@y
@d non_char==qi(65535) {a code that can't match a real character}
@z

@x
@!font_bc: ^eight_bits;
  {beginning (smallest) character code}
@!font_ec: ^eight_bits;
  {ending (largest) character code}
@y
@!font_bc: ^sixteen_bits;
  {beginning (smallest) character code}
@!font_ec: ^sixteen_bits;
  {ending (largest) character code}
@z

@x
@d char_width_end(#)==#.b0].sc
@d char_width(#)==font_info[width_base[#]+char_width_end
@d char_exists(#)==(#.b0>min_quarterword)
@d char_italic_end(#)==(qo(#.b2)) div 4].sc
@d char_italic(#)==font_info[italic_base[#]+char_italic_end
@d height_depth(#)==qo(#.b1)
@d char_height_end(#)==(#) div 16].sc
@d char_height(#)==font_info[height_base[#]+char_height_end
@d char_depth_end(#)==(#) mod 16].sc
@d char_depth(#)==font_info[depth_base[#]+char_depth_end
@d char_tag(#)==((qo(#.b2)) mod 4)
@y
@d char_width_end(#)==#.b0].sc
@d char_width(#)==font_info[width_base[#]+char_width_end
@d char_exists(#)==(#.b0>min_quarterword)
@d char_italic_end(#)==(qo(#.b2)) div 256].sc
@d char_italic(#)==font_info[italic_base[#]+char_italic_end
@d height_depth(#)==qo(#.b1)
@d char_height_end(#)==(#) div 256].sc
@d char_height(#)==font_info[height_base[#]+char_height_end
@d char_depth_end(#)==(#) mod 256].sc
@d char_depth(#)==font_info[depth_base[#]+char_depth_end
@d char_tag(#)==((qo(#.b2)) mod 4)
@z

@x
@!cx:KANJI_code; {kanji code}
@y
@!cx:KANJI_code; {kanji code}
@!ofm_flag:integer;
@!font_level,@!header_length:integer;
@!fn_dir:integer;
@!ncw,@!nlw,@!neew:integer;
@z

@x
@!a,@!b,@!c,@!d:eight_bits; {byte variables}
@y
@!a,@!b,@!c,@!d:integer; {byte variables}
@z

@x
if file_opened then print(" not loadable: Bad metric (TFM) file")
else if name_too_long then print(" not loadable: Metric (TFM) file name too long")
else print(" not loadable: Metric (TFM) file not found");
@y
if file_opened then print(" not loadable: Bad metric (TFM/OFM) file")
else if name_too_long then print(" not loadable: Metric (TFM/OFM) file name too long")
else print(" not loadable: Metric (TFM/OFM) file not found");
@z

@x
pack_file_name(nom,aire,"");
if not b_open_in(tfm_file) then abort;
@y
pack_file_name(nom,aire,"");
if not ofm_open_in(tfm_file) then
  if not b_open_in(tfm_file) then abort;
@z

@x
@d read_sixteen(#)==begin #:=fbyte;
  if #>127 then abort;
  fget; #:=#*@'400+fbyte;
  end
@d read_twentyfourx(#)==begin #:=fbyte;
  fget; #:=#*@"100+fbyte;
  fget; #:=#+fbyte*@"10000;
  end
@d store_four_quarters(#)==begin fget; a:=fbyte; qw.b0:=qi(a);
  fget; b:=fbyte; qw.b1:=qi(b);
  fget; c:=fbyte; qw.b2:=qi(c);
  fget; d:=fbyte; qw.b3:=qi(d);
  #:=qw;
  end
@y
@d read_sixteen(#)==begin #:=fbyte;
  if #>127 then abort;
  fget; #:=#*@'400+fbyte;
  end
@d read_sixteen_unsigned(#)==begin #:=fbyte;
  fget; #:=#*@'400+fbyte;
  end
@d read_twentyfourx(#)==begin #:=fbyte;
  fget; #:=#*@"100+fbyte;
  fget; #:=#+fbyte*@"10000;
  end
@d read_thirtytwo(#)==begin #:=fbyte;
  if #>127 then abort;
  fget; #:=#*@'400+fbyte;
  fget; #:=#*@'400+fbyte;
  fget; #:=#*@'400+fbyte;
  end
@d store_four_quarters(#)==begin
  if (ofm_flag<>0) then begin
    fget; read_sixteen_unsigned(a); qw.b0:=a;
    fget; read_sixteen_unsigned(b); qw.b1:=b;
    fget; read_sixteen_unsigned(c); qw.b2:=c;
    fget; read_sixteen_unsigned(d); qw.b3:=d;
    #:=qw;
    end
  else begin
    fget; a:=fbyte; qw.b0:=qi(a);
    fget; b:=fbyte; qw.b1:=qi(b);
    fget; c:=fbyte; qw.b2:=qi(c);
    fget; d:=fbyte; qw.b3:=qi(d);
    #:=qw;
    end
  end
@d store_character_info(#)==begin
  if (ofm_flag<>0) then begin
    fget; read_sixteen_unsigned(a); qw.b0:=a;
    fget; read_sixteen_unsigned(b); qw.b1:=b;
    fget; read_sixteen_unsigned(c); qw.b2:=c;
    fget; read_sixteen_unsigned(d); qw.b3:=d;
    #:=qw;
    end
  else begin
    fget; a:=fbyte; qw.b0:=qi(a);
    fget; b:=fbyte;
    b:=(b div 16)*256 + (b mod 16); qw.b1:=b;
    fget; c:=fbyte;
    c:=(c div 4)*256 + (c mod 4); qw.b2:=c;
    fget; d:=fbyte; qw.b3:=qi(d);
    #:=qw;
    end
  end
@z

@x
@ @<Read the {\.{TFM}} size fields@>=
begin read_sixteen(lf);
fget; read_sixteen(lh);
@y
@ @<Read the {\.{TFM}} size fields@>=
begin read_sixteen(lf);
fget; read_sixteen(lh);
ofm_flag:=0;
font_level:=-1;
ncw:=0; nlw:=0; neew:=0;
@z

@x
else if lf=tate_jfm_id then
  begin jfm_flag:=dir_tate; nt:=lh;
  fget; read_sixteen(lf);
  fget; read_sixteen(lh);
  end
@y
else if lf=tate_jfm_id then
  begin jfm_flag:=dir_tate; nt:=lh;
  fget; read_sixteen(lf);
  fget; read_sixteen(lh);
  end
else if lf=0 then
  begin ofm_flag:=1;
  font_level:=lh;
  jfm_flag:=dir_default; nt:=0;
  if (font_level<>0) then abort;
  fget; read_thirtytwo(lf);
  fget; read_thirtytwo(lh);
  end
@z

@x
else begin jfm_flag:=dir_default; nt:=0;
  end;
fget; read_sixteen(bc);
fget; read_sixteen(ec);
if (bc>ec+1)or(ec>255) then abort;
if bc>255 then {|bc=256| and |ec=255|}
  begin bc:=1; ec:=0;
  end;
fget; read_sixteen(nw);
fget; read_sixteen(nh);
fget; read_sixteen(nd);
fget; read_sixteen(ni);
fget; read_sixteen(nl);
fget; read_sixteen(nk);
fget; read_sixteen(ne);
fget; read_sixteen(np);
@y
else begin jfm_flag:=dir_default; nt:=0;
  end;
if ofm_flag<>1 then begin
  fget; read_sixteen(bc);
  fget; read_sixteen(ec);
  if (bc>ec+1)or(ec>255) then abort;
  if bc>255 then {|bc=256| and |ec=255|}
    begin bc:=1; ec:=0;
    end;
  fget; read_sixteen(nw);
  fget; read_sixteen(nh);
  fget; read_sixteen(nd);
  fget; read_sixteen(ni);
  fget; read_sixteen(nl);
  fget; read_sixteen(nk);
  fget; read_sixteen(ne);
  fget; read_sixteen(np);
  end
else begin
  fget; read_thirtytwo(bc);
  fget; read_thirtytwo(ec);
  if (bc>ec+1)or(ec>65535) then abort;
  if bc>65535 then {|bc=65536| and |ec=65535|}
    begin bc:=1; ec:=0;
    end;
  fget; read_thirtytwo(nw);
  fget; read_thirtytwo(nh);
  fget; read_thirtytwo(nd);
  fget; read_thirtytwo(ni);
  fget; read_thirtytwo(nl);
  fget; read_thirtytwo(nk);
  fget; read_thirtytwo(ne);
  fget; read_thirtytwo(np);
  fget; read_thirtytwo(fn_dir);
  nlw:=2*nl;
  neew:=2*ne;
  if font_level=0 then begin
    header_length:=14;
    ncw:=2*(ec-bc+1);
    end
  else begin
    abort;
    end;
end;
if ofm_flag<>0 then
  begin if lf<>header_length+lh+ncw+nw+nh+nd+ni+nlw+nk+neew+np
      then abort;
  end
else
@z

@x
@<Use size fields to allocate font information@>=
if jfm_flag<>dir_default then
@y
@<Use size fields to allocate font information@>=
if ofm_flag<>0 then begin
  if font_level=0 then
    lf:=lf-14-lh-(ec-bc+1)-nl-ne
  else
    abort;
  end
else
if jfm_flag<>dir_default then
@z

@x
font_enc[f]:=jfm_enc; if jfm_flag=dir_default then font_enc[f]:=0;
font_num_ext[f]:=nt;
ctype_base[f]:=fmem_ptr;
char_base[f]:=ctype_base[f]+nt-bc;
width_base[f]:=char_base[f]+ec+1;
@y
font_enc[f]:=0;
if (jfm_flag=dir_default)and(jfm_enc>=enc_t1) then font_enc[f]:=jfm_enc;
if (jfm_flag<>dir_default)and(jfm_enc<=enc_ucs) then font_enc[f]:=jfm_enc;
font_num_ext[f]:=nt;
ctype_base[f]:=fmem_ptr;
char_base[f]:=ctype_base[f]+nt-bc;
width_base[f]:=char_base[f]+ec+1;
@z

@x
store_four_quarters(font_check[f]);
@y
begin
  fget; a:=fbyte; qw.b0:=qi(a);
  fget; b:=fbyte; qw.b1:=qi(b);
  fget; c:=fbyte; qw.b2:=qi(c);
  fget; d:=fbyte; qw.b3:=qi(d);
  font_check[f]:=qw;
  end;
@z

@x
@ @<Read character data@>=
if jfm_flag<>dir_default then
  for k:=ctype_base[f] to ctype_base[f]+nt-1 do
    begin
    fget; read_twentyfourx(cx);
    if jfm_enc=enc_ucs then {Unicode TFM}
      font_info[k].hh.rh:=toDVI(fromUCS(cx))
    else if jfm_enc=enc_jis then {JIS-encoded TFM}
      font_info[k].hh.rh:=toDVI(fromJIS(cx))
    else
      font_info[k].hh.rh:=tokanji(cx); {|kchar_code|}
    fget; cx:=fbyte;
    font_info[k].hh.lhfield:=tonum(cx); {|kchar_type|}
    end;
for k:=char_base[f]+bc to width_base[f]-1 do
  begin store_four_quarters(font_info[k].qqqq);
  if (a>=nw)or(b div @'20>=nh)or(b mod @'20>=nd)or
    (c div 4>=ni) then abort;
  case c mod 4 of
  lig_tag: if d>=nl then abort;
  ext_tag: if d>=ne then abort;
  list_tag: @<Check for charlist cycle@>;
  othercases do_nothing {|no_tag|}
  endcases;
  end
@y
@ @<Read character data@>=
if ofm_flag<>0 then begin
if font_level=1 then begin
  abort;
  end;
  end
else
if jfm_flag<>dir_default then
  for k:=ctype_base[f] to ctype_base[f]+nt-1 do
    begin
    fget; read_twentyfourx(cx);
    if jfm_enc=enc_ucs then {Unicode TFM}
      font_info[k].hh.rh:=toDVI(fromUCS(cx))
    else if jfm_enc=enc_jis then {JIS-encoded TFM}
      font_info[k].hh.rh:=toDVI(fromJIS(cx))
    else
      font_info[k].hh.rh:=tokanji(cx); {|kchar_code|}
    fget; cx:=fbyte;
    font_info[k].hh.lhfield:=tonum(cx); {|kchar_type|}
    end;
k:=char_base[f]+bc;
while k<=width_base[f]-1 do
  begin store_character_info(font_info[k].qqqq);
  if (a>=nw)or((b div 256)>=nh)or((b mod 256)>=nd)or
    ((c div 256)>=ni) then abort;
  case c mod 4 of
  lig_tag: if d>=nl then abort;
  ext_tag: if d>=ne then abort;
  list_tag: @<Check for charlist cycle@>;
  othercases do_nothing {|no_tag|}
  endcases;
  incr(k);
  if font_level=1 then begin
    abort;
    end;
  end
@z

@x
@<Read ligature/kern program@>=
bch_label:=@'77777; bchar:=256;
@y
@<Read ligature/kern program@>=
bch_label:=@'77777; bchar:=max_latin_val;
@z

@x
@p procedure char_warning(@!f:internal_font_number;@!c:eight_bits);
@y
@p procedure char_warning(@!f:internal_font_number;@!c:sixteen_bits);
@z

@x
  if (c<" ")or(c>"~") then
    begin print_char("^"); print_char("^");
    if c<64 then print_char(c+64)
    else if c<128 then print_char(c-64)
    else begin print_lc_hex(c div 16);  print_lc_hex(c mod 16); end
    end
@y
  if (c<" ")or(c>"~") then begin
    print_char("^"); print_char("^");
    if c<64 then print_char(c+64)
    else if c<128 then print_char(c-64)
    else if c<256 then begin
        print_lc_hex(c div 16);    print_lc_hex(c mod 16); end
    else begin print_char("^"); print_char("^");
        print_lc_hex(c div 4096);  print_lc_hex((c mod 4096) div 256);
        print_lc_hex((c mod 256) div 16);  print_lc_hex(c mod 16); end
             end
@z

@x
@p function new_character(@!f:internal_font_number;@!c:eight_bits):pointer;
label exit;
var p:pointer; {newly allocated node}
@!ec:quarterword;  {effective character of |c|}
begin ec:=effective_char(false,f,qi(c));
if font_bc[f]<=qo(ec) then if font_ec[f]>=qo(ec) then
  if char_exists(orig_char_info(f)(ec)) then  {N.B.: not |char_info|}
    begin p:=get_avail; font(p):=f; character(p):=qi(c);
    new_character:=p; return;
    end;
char_warning(f,c);
new_character:=null;
exit:end;
@y
@p function new_character(@!f:internal_font_number;@!c:sixteen_bits):pointer;
label exit;
var p:pointer; {newly allocated node}
@!ec,cc:quarterword;  {effective character of |c|}
begin cc:=c;
c:=ptencucsto8bitcode(font_enc[f],c);
ec:=effective_char(false,f,qi(c));
if font_bc[f]<=qo(ec) then if font_ec[f]>=qo(ec) then
  if char_exists(orig_char_info(f)(ec)) then  {N.B.: not |char_info|}
    begin p:=get_avail; font(p):=f; character(p):=qi(c);
    new_character:=p; return;
    end;
char_warning(f,cc);
new_character:=null;
exit:end;
@z

@x
@d set2=129 {typeset a character and move right}
@y
@d set2=129 {typeset a character and move right}
@d set3=130 {typeset a character and move right}
@d set4=131 {typeset a character and move right}
@z

@x
  if font_dir[f]=dir_default then
    begin chain:=false;
    if font_ec[f]>=qo(c) then if font_bc[f]<=qo(c) then
      if char_exists(orig_char_info(f)(c)) then  {N.B.: not |char_info|}
        begin if c>=qi(128) then dvi_out(set1);
        dvi_out(qo(c));@/
@y
  if font_dir[f]=dir_default then
    begin chain:=false;
    c:=ptencucsto8bitcode(font_enc[f],c);
    if font_ec[f]>=qo(c) then if font_bc[f]<=qo(c) then
      if char_exists(orig_char_info(f)(c)) then  {N.B.: not |char_info|}
        begin if c>=qi(@"100) then begin
          dvi_out(set2); dvi_out(Hi(c)); dvi_out(Lo(c));
          end
        else begin if c>=qi(128) then dvi_out(set1);
          dvi_out(qo(c));@/
        end;
@z

@x
    jc:=KANJI(info(p));
@y
    jc:=ktokentochr(info(p));
@z

@x
    dvi_out(set2); dvi_out(Hi(jc)); dvi_out(Lo(jc));
@y
    if (jc<@"10000) then begin
      dvi_out(set2);
    end else if (jc<@"1000000) then begin
      dvi_out(set3); dvi_out(BYTE2(jc));
    end else begin
      dvi_out(set4); dvi_out(BYTE1(jc)); dvi_out(BYTE2(jc));
    end;
    dvi_out(BYTE3(jc)); dvi_out(BYTE4(jc));
@z

@x
@!hd:eight_bits; {height and depth indices for a character}
@y
@!hd:sixteen_bits; {height and depth indices for a character}
@!cx:integer;
@z

@x
@<Incorporate character dimensions into the dimensions of the hbox...@>=
begin f:=font(p); i:=char_info(f)(character(p)); hd:=height_depth(i);
@y
@<Incorporate character dimensions into the dimensions of the hbox...@>=
begin f:=font(p); cx:=ptencucsto8bitcode(font_enc[f],character(p));
i:=char_info(f)(cx); hd:=height_depth(i);
@z

@x
@!hd: eight_bits; {height-depth byte}
@y
@!hd: sixteen_bits; {height-depth byte}
@z

@x
@!hd:eight_bits; {|height_depth| byte}
@y
@!hd:sixteen_bits; {|height_depth| byte}
@z

@x
@!hd:eight_bits; {|height_depth| byte}
@y
@!hd:sixteen_bits; {|height_depth| byte}
@z

@x
@d span_code=256 {distinct from any character}
@d cr_code=257 {distinct from |span_code| and from any character}
@y
@d span_code=max_cjk_val {distinct from any character}
@d cr_code=max_cjk_val+1 {distinct from |span_code| and from any character}
@z

@x
@!cc:ASCII_code;
@y
@!cc:sixteen_bits;
@z

@x
  repeat f:=post_f; cc:=character(cur_p);
  act_width:=act_width+char_width(f)(orig_char_info(f)(cc));
@y
  repeat f:=post_f;
  cc:=ptencucsto8bitcode(font_enc[f],character(cur_p));
  act_width:=act_width+char_width(f)(orig_char_info(f)(cc));
@z

@x hyphen
@!hc:array[0..65] of 0..256; {word to be hyphenated}
@!hn:0..64; {the number of positions occupied in |hc|;
                                  not always a |small_number|}
@!ha,@!hb:pointer; {nodes |ha..hb| should be replaced by the hyphenated result}
@!hf:internal_font_number; {font number of the letters in |hc|}
@!hu:array[0..63] of 0..256; {like |hc|, before conversion to lowercase}
@!hyf_char:integer; {hyphen character of the relevant font}
@!cur_lang,@!init_cur_lang:ASCII_code; {current hyphenation table of interest}
@!l_hyf,@!r_hyf,@!init_l_hyf,@!init_r_hyf:integer; {limits on fragment sizes}
@!hyf_bchar:halfword; {boundary character after $c_n$}
@y
@!hc:array[0..65] of 0..max_latin_val; {word to be hyphenated}
@!hn:0..64; {the number of positions occupied in |hc|;
                                  not always a |small_number|}
@!ha,@!hb:pointer; {nodes |ha..hb| should be replaced by the hyphenated result}
@!hf:internal_font_number; {font number of the letters in |hc|}
@!hu:array[0..63] of 0..max_latin_val; {like |hc|, before conversion to lowercase}
@!hyf_char:integer; {hyphen character of the relevant font}
@!cur_lang,@!init_cur_lang:ASCII_code; {current hyphenation table of interest}
@!l_hyf,@!r_hyf,@!init_l_hyf,@!init_r_hyf:integer; {limits on fragment sizes}
@!hyf_bchar:halfword; {boundary character after $c_n$}
@!max_hyph_char:integer;

@ @<Set initial values of key variables@>=
max_hyph_char:=max_latin_val-1;
@z

@x
@<Local variables for line...@>=
@!j:small_number; {an index into |hc| or |hu|}
@!c:0..255; {character being considered for hyphenation}
@y
@<Local variables for line...@>=
@!j:small_number; {an index into |hc| or |hu|}
@!c:sixteen_bits; {character being considered for hyphenation}
@z

@x
if hyf_char>255 then goto done1;
@y
if hyf_char>=max_latin_val then goto done1;
@z

@x
hn:=0;
loop@+  begin if is_char_node(s) then
    begin if font(s)<>hf then goto done3;
    hyf_bchar:=character(s); c:=qo(hyf_bchar);
    if lc_code(c)=0 then goto done3;
    if hn=63 then goto done3;
    hb:=s; incr(hn); hu[hn]:=c; hc[hn]:=lc_code(c); hyf_bchar:=non_char;
@y
hn:=0;
loop@+  begin if is_char_node(s) then
    begin if font(s)<>hf then goto done3;
    hyf_bchar:=character(s); c:=qo(hyf_bchar);
    if lc_code(c)=0 then goto done3;
    if lc_code(c)>max_hyph_char then goto done3;
    if hn=63 then goto done3;
    hb:=s; incr(hn); hu[hn]:=c; hc[hn]:=lc_code(c); hyf_bchar:=non_char;
@z

@x
while q>null do
  begin c:=qo(character(q));
  if lc_code(c)=0 then goto done3;
  if j=63 then goto done3;
  incr(j); hu[j]:=c; hc[j]:=lc_code(c);@/
  q:=link(q);
  end;
@y
while q>null do
  begin c:=qo(character(q));
  if lc_code(c)=0 then goto done3;
  if lc_code(c)>max_hyph_char then goto done3;
  if j=63 then goto done3;
  incr(j); hu[j]:=c; hc[j]:=lc_code(c);@/
  q:=link(q);
  end;
@z

@x
      begin hu[0]:=256; init_lig:=false;
@y
      begin hu[0]:=max_hyph_char; init_lig:=false;
@z

@x
found2: s:=ha; j:=0; hu[0]:=256; init_lig:=false; init_list:=null;
@y
found2: s:=ha; j:=0; hu[0]:=max_hyph_char; init_lig:=false; init_list:=null;
@z

@x
getting the input $x_j\ldots x_n$ from the |hu| array. If $x_j=256$,
we consider $x_j$ to be an implicit left boundary character; in this
case |j| must be strictly less than~|n|. There is a
parameter |bchar|, which is either 256 or an implicit right boundary character
@y
getting the input $x_j\ldots x_n$ from the |hu| array. If $x_j=|max_hyph_char|$,
we consider $x_j$ to be an implicit left boundary character; in this
case |j| must be strictly less than~|n|. There is a
parameter |bchar|, which is either |max_hyph_char|
or an implicit right boundary character
@z

@x
@<Local variables for hyph...@>=
@!major_tail,@!minor_tail:pointer; {the end of lists in the main and
  discretionary branches being reconstructed}
@!c:ASCII_code; {character temporarily replaced by a hyphen}
@y
@<Local variables for hyph...@>=
@!major_tail,@!minor_tail:pointer; {the end of lists in the main and
  discretionary branches being reconstructed}
@!c:sixteen_bits; {character temporarily replaced by a hyphen}
@z

@x
  begin decr(l); c:=hu[l]; c_loc:=l; hu[l]:=256;
@y
  begin decr(l); c:=hu[l]; c_loc:=l; hu[l]:=max_hyph_char;
@z

@x
hyphenation algorithm is quite short. In the following code we set |hc[hn+2]|
to the impossible value 256, in order to guarantee that |hc[hn+3]| will
@y
hyphenation algorithm is quite short. In the following code we set |hc[hn+2]| to
the impossible value |max_hyph_char|, in order to guarantee that |hc[hn+3]| will
@z

@x
hc[0]:=0; hc[hn+1]:=0; hc[hn+2]:=256; {insert delimiters}
@y
hc[0]:=0; hc[hn+1]:=0; hc[hn+2]:=max_hyph_char; {insert delimiters}
@z

@x first_fit
@!ll:1..256; {upper limit of |trie_min| updating}
@y
@!ll:1..max_latin_val; {upper limit of |trie_min| updating}
@z

@x
  @<Ensure that |trie_max>=h+256|@>;
@y
  @<Ensure that |trie_max>=h+max_hyph_char|@>;
@z

@x
@ By making sure that |trie_max| is at least |h+256|, we can be sure that
|trie_max>z|, since |h=z-c|. It follows that location |trie_max| will
never be occupied in |trie|, and we will have |trie_max>=trie_link(z)|.

@<Ensure that |trie_max>=h+256|@>=
if trie_max<h+256 then
  begin if trie_size<=h+256 then overflow("pattern memory",trie_size);
@y
@ By making sure that |trie_max| is at least |h+max_hyph_char|,
we can be sure that
|trie_max>z|, since |h=z-c|. It follows that location |trie_max| will
never be occupied in |trie|, and we will have |trie_max>=trie_link(z)|.

@<Ensure that |trie_max>=h+max_hyph_char|@>=
if trie_max<h+max_hyph_char then
  begin if trie_size<=h+max_hyph_char then overflow("pattern memory",trie_size);
@z

@x
  until trie_max=h+256;
@y
  until trie_max=h+max_hyph_char;
@z

@x
  begin for r:=0 to 256 do clear_trie;
  trie_max:=256;
  end
@y
  begin for r:=0 to max_hyph_char do clear_trie;
  trie_max:=max_hyph_char;
  end
@z

@x
    if cur_chr=0 then
      begin print_err("Nonletter");
@.Nonletter@>
      help1("(See Appendix H.)"); error;
      end;
    end;
@y
    if cur_chr=0 then
      begin print_err("Nonletter");
@.Nonletter@>
      help1("(See Appendix H.)"); error;
      end;
    end;
    if cur_chr>max_hyph_char then max_hyph_char:=cur_chr;
@z

@x
begin @<Get ready to compress the trie@>;
@y
begin
incr(max_hyph_char);
@<Get ready to compress the trie@>;
@z

@x
var@!t:integer; {general-purpose temporary variable}
@!cx:KANJI_code; {kanji character}
@y
var@!t:integer; {general-purpose temporary variable}
@!cx:KANJI_code; {kanji character}
@!cy:sixteen_bits;
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
hmode+kanji,hmode+kana,hmode+other_kchar,hmode+hangul,hmode+modifier: goto main_loop_j;
hmode+char_given:
  if check_echar_range(cur_chr) then goto main_loop
  else begin cur_cmd:=kcat_code(kcatcodekey(cur_chr));
    if (cur_cmd<=not_cjk) then cur_cmd:=other_kchar;
    goto main_loop_j; end;
hmode+kchar_given:
  begin cur_cmd:=kcat_code(kcatcodekey(cur_chr)); goto main_loop_j; end;
hmode+char_num: begin scan_char_num; cur_chr:=cur_val;
  if check_echar_range(cur_chr) then goto main_loop
  else begin cur_cmd:=kcat_code(kcatcodekey(cur_chr));
    if (cur_cmd<=not_cjk) then cur_cmd:=other_kchar;
    goto main_loop_j; end;
  end;
hmode+kchar_num: begin scan_char_num; cur_chr:=cur_val;
  cur_cmd:=kcat_code(kcatcodekey(cur_chr));
  goto main_loop_j;
  end;
hmode+no_boundary: begin get_x_token;
  if (cur_cmd=letter)or(cur_cmd=other_char)or
   ((cur_cmd>=kanji)and(cur_cmd<=modifier))or
   (cur_cmd=char_given)or(cur_cmd=char_num)or
   (cur_cmd=kchar_given)or(cur_cmd=kchar_num) then cancel_boundary:=true;
@z

@x
main_loop_move+2:
if(qo(effective_char(false,main_f,qi(cur_chr)))>font_ec[main_f])or
  (qo(effective_char(false,main_f,qi(cur_chr)))<font_bc[main_f]) then
  begin char_warning(main_f,cur_chr); free_avail(lig_stack); goto big_switch;
  end;
main_i:=effective_char_info(main_f,cur_l);
@y
main_loop_move+2:
cur_chr:=ptencucsto8bitcode(font_enc[main_f],cur_chr);
if(qo(effective_char(false,main_f,qi(cur_chr)))>font_ec[main_f])or
  (qo(effective_char(false,main_f,qi(cur_chr)))<font_bc[main_f]) then
  begin char_warning(main_f,cur_chr); free_avail(lig_stack); goto big_switch;
  end;
if not ligature_present then cur_l:=ptencucsto8bitcode(font_enc[main_f],cur_l);
main_i:=effective_char_info(main_f,cur_l);
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
if (cur_cmd>=kanji)and(cur_cmd<=modifier) then
  @<goto |main_lig_loop|@>;
if cur_cmd=other_char then goto main_loop_lookahead+1;
if cur_cmd=char_given then
  begin if check_echar_range(cur_chr) then goto main_loop_lookahead+1
  else begin cur_cmd:=kcat_code(kcatcodekey(cur_chr)); @<goto |main_lig_loop|@>; end;
  end;
if cur_cmd=kchar_given then
  begin cur_cmd:=kcat_code(kcatcodekey(cur_chr)); @<goto |main_lig_loop|@>; end;
x_token; {now expand and set |cur_cmd|, |cur_chr|, |cur_tok|}
if cur_cmd=letter then goto main_loop_lookahead+1;
if (cur_cmd>=kanji)and(cur_cmd<=modifier) then
  @<goto |main_lig_loop|@>;
if cur_cmd=other_char then goto main_loop_lookahead+1;
if cur_cmd=char_given then
  begin if check_echar_range(cur_chr) then goto main_loop_lookahead+1
  else begin cur_cmd:=kcat_code(kcatcodekey(cur_chr)); @<goto |main_lig_loop|@>; end;
  end;
if cur_cmd=kchar_given then
  begin cur_cmd:=kcat_code(kcatcodekey(cur_chr)); @<goto |main_lig_loop|@>; end;
if cur_cmd=char_num then
  begin scan_char_num; cur_chr:=cur_val;
  if check_echar_range(cur_chr) then goto main_loop_lookahead+1
  else begin cur_cmd:=kcat_code(kcatcodekey(cur_chr)); @<goto |main_lig_loop|@>; end;
  end;
if cur_cmd=kchar_num then
  begin scan_char_num; cur_chr:=cur_val;
  cur_cmd:=kcat_code(kcatcodekey(cur_chr));
  @<goto |main_lig_loop|@>;
  end;
@z

@x
main_lig_loop+1:main_j:=font_info[main_k].qqqq;
main_lig_loop+2:if next_char(main_j)=cur_r then
 if skip_byte(main_j)<=stop_flag then
  @<Do ligature or kern command, returning to |main_lig_loop|
  or |main_loop_wrapup| or |main_loop_move|@>;
@y
main_lig_loop+1:main_j:=font_info[main_k].qqqq;
main_lig_loop+2:
 cy:=ptencucsto8bitcode(font_enc[main_f],cur_r);
 if next_char(main_j)=cy then
 if skip_byte(main_j)<=stop_flag then
  @<Do ligature or kern command, returning to |main_lig_loop|
  or |main_loop_wrapup| or |main_loop_move|@>;
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
   vmode+kanji,vmode+kana,vmode+other_kchar,vmode+hangul,vmode+modifier,
@z


@x
  begin c:=hyphen_char[cur_font];
  if c>=0 then if c<256 then pre_break(tail):=new_character(cur_font,c);
  end
@y
  begin c:=hyphen_char[cur_font];
  if c>=0 then if c<max_latin_val then pre_break(tail):=new_character(cur_font,c);
  end
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
        if (kcat_code(kcatcodekey(cx))=kanji)and(cx>=max_cjk_val) then
          info(link(p)):=KANJI(cx) + kanji_ivs*max_cjk_val
        else
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
else if (cur_cmd>=kanji)and(cur_cmd<=modifier) then
  begin  if direction=dir_tate then f:=cur_tfont else f:=cur_jfont;
  cx:=cur_chr;
  end
else if cur_cmd=char_given then
  if check_echar_range(cur_chr) then q:=new_character(f,cur_chr)
  else begin
    if direction=dir_tate then f:=cur_tfont else f:=cur_jfont;
    KANJI(cx):=cur_chr; cur_cmd:=kcat_code(kcatcodekey(cx));
    end
else if cur_cmd=char_num then
  begin scan_char_num;
  if check_echar_range(cur_val) then q:=new_character(f,cur_val)
  else  begin
    if direction=dir_tate then f:=cur_tfont else f:=cur_jfont;
    KANJI(cx):=cur_val; cur_cmd:=kcat_code(kcatcodekey(cx));
    end
  end
else if cur_cmd=kchar_given then
  begin
    if direction=dir_tate then f:=cur_tfont else f:=cur_jfont;
    KANJI(cx):=cur_chr; cur_cmd:=kcat_code(kcatcodekey(cx));
  end
else if cur_cmd=kchar_num then
  begin scan_char_num;
    if direction=dir_tate then f:=cur_tfont else f:=cur_jfont;
    KANJI(cx):=cur_val; cur_cmd:=kcat_code(kcatcodekey(cx));
  end
@z

@x
  begin q:=new_character(f,get_jfm_pos(KANJI(cx),f));
  link(q):=get_avail; info(link(q)):=KANJI(cx); last_jchr:=q;
@y
  begin q:=new_character(f,get_jfm_pos(KANJI(cx),f));
  link(q):=get_avail;
  if (kcat_code(kcatcodekey(cx))=kanji)and(cx>=max_cjk_val) then
    info(link(q)):=KANJI(cx) + kanji_ivs*max_cjk_val
  else
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
char_num: begin scan_char_num; cur_chr:=cur_val; cur_cmd:=char_given;
  goto reswitch;
  end;
@y
    KANJI(cx):=cur_chr;
kanji,kana,other_kchar,hangul,modifier: cx:=cur_chr;
kchar_given:
  KANJI(cx):=cur_chr;
char_num: begin scan_char_num; cur_chr:=cur_val; cur_cmd:=char_given;
  goto reswitch;
  end;
kchar_num: begin scan_char_num; cur_chr:=cur_val; cur_cmd:=kchar_given;
  goto reswitch;
  end;
@z

@x
  math_type(p):=math_jchar; fam(p):=cur_jfam; character(p):=qi(0);
  math_kcode(p-1):=KANJI(cx);
@y
  math_type(p):=math_jchar; fam(p):=cur_jfam; character(p):=qi(0);
  if (kcat_code(kcatcodekey(cx))=kanji)and(cx>=max_cjk_val) then
    math_kcode(p-1):=KANJI(cx) + kanji_ivs*max_cjk_val
  else
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
mmode+kanji,mmode+kana,mmode+other_kchar,mmode+hangul,mmode+modifier: begin
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
  if p=kcat_code_base then
    begin scan_char_num; p:=p+kcatcodekey(cur_val) end
  else begin scan_ascii_num; p:=p+cur_val; end;
@y
  if p=kcat_code_base then
    begin scan_char_num; p:=p+kcatcodekey(cur_val);
      if cur_val>=max_latin_val then m:=not_cjk; end
  else if p<math_code_base then
    begin scan_latin_num; p:=p+cur_val; end
  else begin scan_ascii_num; p:=p+cur_val; end;
@z

@x
@ @<Let |m| be the minimal...@>=
if cur_chr=kcat_code_base then m:=kanji else m:=0

@ @<Let |n| be the largest...@>=
if cur_chr=cat_code_base then n:=invalid_char {1byte |max_char_code|}
else if cur_chr=kcat_code_base then n:=max_char_code
@y
@ @<Let |m| be the minimal...@>=
if cur_chr=kcat_code_base then m:=latin_ucs else m:=0

@ @<Let |n| be the largest...@>=
if cur_chr=cat_code_base then n:=invalid_char {1byte |max_char_code|}
else if cur_chr=kcat_code_base then n:=max_char_code
else if cur_chr<math_code_base then n:=max_latin_val
@z

@x
procedure shift_case;
var b:pointer; {|lc_code_base| or |uc_code_base|}
@!p:pointer; {runs through the token list}
@!t:halfword; {token}
@!c:eight_bits; {character code}
@y
procedure shift_case;
var b:pointer; {|lc_code_base| or |uc_code_base|}
@!p:pointer; {runs through the token list}
@!t:halfword; {token}
@!c:sixteen_bits; {character code}
@z

@x
@<Change the case of the token in |p|, if a change is appropriate@>=
t:=info(p);
if (t<cs_token_flag+single_base)and(not check_kanji(t)) then
  begin c:=t mod 256;
@y
@<Change the case of the token in |p|, if a change is appropriate@>=
t:=info(p);
if (t<cs_token_flag+single_base) then
  begin c:=t mod max_char_val;
@z

@x
font_bc:=xmalloc_array(eight_bits, font_max);
font_ec:=xmalloc_array(eight_bits, font_max);
@y
font_bc:=xmalloc_array(sixteen_bits, font_max);
font_ec:=xmalloc_array(sixteen_bits, font_max);
@z

@x
  font_bc:=xmalloc_array(eight_bits, font_max);
  font_ec:=xmalloc_array(eight_bits, font_max);
@y
  font_bc:=xmalloc_array(sixteen_bits, font_max);
  font_ec:=xmalloc_array(sixteen_bits, font_max);
@z

@x
@ @<Scan the font encoding specification@>=
begin jfm_enc:=0;
if scan_keyword_noexpand("in") then
  if scan_keyword_noexpand("jis") then jfm_enc:=enc_jis
  else if scan_keyword_noexpand("ucs") then jfm_enc:=enc_ucs
  else begin
    print_err("Unknown TFM encoding");
@.Unknown TFM encoding@>
@y
@ @<Scan the font encoding specification@>=
begin jfm_enc:=0;
if scan_keyword_noexpand("in") then
  if scan_keyword_noexpand("jis") then jfm_enc:=enc_jis
  else if scan_keyword_noexpand("ucs") then jfm_enc:=enc_ucs
  else if scan_keyword_noexpand("t1") then jfm_enc:=enc_t1
  else if scan_keyword_noexpand("ts1") then jfm_enc:=enc_ts1
  else if scan_keyword_noexpand("ly1") then jfm_enc:=enc_ly1
  else if scan_keyword_noexpand("t5") then jfm_enc:=enc_t5
  else if scan_keyword_noexpand("l7x") then jfm_enc:=enc_l7x
  else if scan_keyword_noexpand("t2a") then jfm_enc:=enc_t2a
  else if scan_keyword_noexpand("t2b") then jfm_enc:=enc_t2b
  else if scan_keyword_noexpand("t2c") then jfm_enc:=enc_t2c
  else if scan_keyword_noexpand("lgr") then jfm_enc:=enc_lgr
  else begin
    print_err("Unknown TFM encoding");
@.Unknown TFM encoding@>
@z

@x
@!t: eight_bits;
@y
@!t: sixteen_bits;
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
  cx:KANJI_code; {temporary register for KANJI character}
  ax:ASCII_code; {temporary register for ASCII character}
@y
  cx:KANJI_code; {temporary register for KANJI character}
  ax:sixteen_bits; {temporary register for ASCII character}
@z

@x
@ @<Insert a space around the character |p|@>=
if font_dir[font(p)]<>dir_default then
  begin KANJI(cx):=info(link(p));
@y
@ @<Insert a space around the character |p|@>=
if font_dir[font(p)]<>dir_default then
  begin KANJI(cx):=ktokentochr(info(link(p)));
@z

@x
else if font_dir[font(first_char)]<>dir_default then
  begin KANJI(cx):=info(link(first_char));
@y
else if font_dir[font(first_char)]<>dir_default then
  begin KANJI(cx):=ktokentochr(info(link(first_char)));
@z

@x
else if font_dir[font(last_char)]<>dir_default then
  begin insert_skip:=after_wchar; KANJI(cx):=info(link(last_char));
@y
else if font_dir[font(last_char)]<>dir_default then
  begin insert_skip:=after_wchar;
  KANJI(cx):=ktokentochr(info(link(last_char)));
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
    begin KANJI(cx):=ktokentochr(info(link(p)));
@z

@x
    begin KANJI(cx):=info(link(p)); i:=kcat_code(kcatcodekey(cx)); k:=0;
    if (i=kanji)or(i=kana) then begin t:=q; s:=p; end;
@y
    begin KANJI(cx):=ktokentochr(info(link(p)));
    i:=ktokentocmd(info(link(p))); k:=0;
    if (i=kanji)or(i=kana)or(i=hangul)or(i=modifier) then begin t:=q; s:=p; end;
@z

@x
    begin if not disp_called then
      begin prev_node:=tail; tail_append(get_node(small_node_size));
      type(tail):=disp_node; disp_dimen(tail):=0; disp_called:=true
      end;
    fast_get_avail(main_p); font(main_p):=main_f; character(main_p):=cur_l;
    link(tail):=main_p; tail:=main_p; last_jchr:=tail;
    fast_get_avail(main_p); info(main_p):=KANJI(cur_chr);
@y
    begin if (cur_cmd=modifier) then begin
      KANJI(cx):=info(main_p) mod max_cjk_val;
      if (UVScombinecode(cx,cur_chr)>0) then begin
        cx:=UVScombinecode(cx,cur_chr);
        if (kcat_code(kcatcodekey(KANJI(cx)))=kanji)and(cx>=max_cjk_val) then
          info(main_p):=KANJI(cx)+kanji_ivs*max_cjk_val
        else
          info(main_p):=KANJI(cx)+kcat_code(kcatcodekey(KANJI(cx)))*max_cjk_val;
        ins_kp:=false;
        goto again_2
        end
      end;
    if not disp_called then
      begin prev_node:=tail; tail_append(get_node(small_node_size));
      type(tail):=disp_node; disp_dimen(tail):=0; disp_called:=true
      end;
    fast_get_avail(main_p); font(main_p):=main_f; character(main_p):=cur_l;
    link(tail):=main_p; tail:=main_p; last_jchr:=tail;
    fast_get_avail(main_p);
    if (cur_cmd=kanji)and(cur_chr>=max_cjk_val) then
      info(main_p):=KANJI(cur_chr)+kanji_ivs*max_cjk_val
    else if (cur_cmd>=kanji)and(cur_cmd<=modifier) then
      info(main_p):=KANJI(cur_chr)+cur_cmd*max_cjk_val
    else if cur_cmd=not_cjk then
      info(main_p):=KANJI(cur_chr)+other_kchar*max_cjk_val
    else if cur_cmd=latin_ucs then
      info(main_p):=KANJI(cur_chr)+cat_code(cur_chr)*max_cjk_val
    else { Does this case occur? }
      info(main_p):=KANJI(cur_chr)+kcat_code(kcatcodekey(KANJI(cur_chr)))*max_cjk_val;
@z

@x
  main_i:=orig_char_info(main_f)(cur_l);
  case cur_cmd of
    kanji,kana,other_kchar: begin
@y
  main_i:=orig_char_info(main_f)(cur_l);
  case cur_cmd of
    kanji,kana,other_kchar,hangul,modifier: begin
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
    kanji,kana,other_kchar,hangul,modifier: cur_l:=qi(get_jfm_pos(KANJI(cur_chr),main_f));
    letter,other_char: begin ins_kp:=true; cur_l:=qi(0); end;
    char_given: begin
      if check_echar_range(cur_chr) then
        begin ins_kp:=true; cur_l:=qi(0);
        end
      else cur_l:=qi(get_jfm_pos(KANJI(cur_chr),main_f));
      cur_cmd:=kcat_code(kcatcodekey(cur_chr));
      end;
    char_num: begin scan_char_num; cur_chr:=cur_val;
      if check_echar_range(cur_chr) then
        begin ins_kp:=true; cur_l:=qi(0);
        end
      else cur_l:=qi(get_jfm_pos(KANJI(cur_chr),main_f));
      cur_cmd:=kcat_code(kcatcodekey(cur_chr));
      end;
    kchar_given: begin
      cur_l:=qi(get_jfm_pos(KANJI(cur_chr),main_f));
      cur_cmd:=kcat_code(kcatcodekey(cur_chr));
      end;
    kchar_num: begin scan_char_num; cur_chr:=cur_val;
      cur_l:=qi(get_jfm_pos(KANJI(cur_chr),main_f));
      cur_cmd:=kcat_code(kcatcodekey(cur_chr));
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
var @!v,@!nn,@!jj: integer;
begin
if (isinternalUPTEX) then begin
  s:=ktokentochr(s);
  s:=toUCS(s);
  nn:=UVSgetcodepointlength(s);
  jj:=1;
  while jj<=nn do begin
    v:=UVSgetcodepointinsequence(s,jj);
    if (v>0) then begin
      v:=UCStoUTF8(v);
      if BYTE1(v)<>0 then print_char(@"100+BYTE1(v));
      if BYTE2(v)<>0 then print_char(@"100+BYTE2(v));
      if BYTE3(v)<>0 then print_char(@"100+BYTE3(v));
                          print_char(@"100+BYTE4(v));
      end;
    incr(jj);
    end
  end
else begin
  s:=toBUFF(s mod max_cjk_val);
  if BYTE1(s)<>0 then print_char(@"100+BYTE1(s));
  if BYTE2(s)<>0 then print_char(@"100+BYTE2(s));
  if BYTE3(s)<>0 then print_char(@"100+BYTE3(s));
                      print_char(@"100+BYTE4(s));
  end;
end;

function check_kcat_code(@!ct:integer;@!cx:integer):integer;
begin
if (((ct>=kanji)or((ct=latin_ucs)and(cx<max_latin_val)))and(enable_cjk_token=0))or(enable_cjk_token=2)then
  check_kcat_code:=1
else check_kcat_code:=0;
end;

function check_echar_range(@!c:integer):integer;
begin
if (c>127)and(c<max_latin_val)and(kcat_code(kcatcodekey(c))=latin_ucs)then
  check_echar_range:=1
else if (c>=0)and(c<256)then
  check_echar_range:=2
else check_echar_range:=0;
end;

function check_mchar_range(@!c:integer):integer;
begin
if (c>255)and(c<max_latin_val)then
  check_mchar_range:=1
else check_mchar_range:=0;
@z
