% fam256.ch
% (C) 2009--2017 by Hironori Kitagawa.
%
% This patch is derived from om16bit.ch and omfi.ch (both in Omega).
% (Omega is copyright by John Plaice and Yannis Haralambous.)
% 
%-----------------------------------------------
@x
@d hyph_prime=607 {another prime for hashing \.{\\hyphenation} exceptions;
                if you change this, you should also change |iinf_hyphen_size|.}
@y
@d hyph_prime=607 {another prime for hashing \.{\\hyphenation} exceptions;
                if you change this, you should also change |iinf_hyphen_size|.}
@d text_size=0 {size code for the largest size in a family}
@d script_size=256 {size code for the medium size in a family}
@d script_script_size=512 {size code for the smallest size in a family}
@z
%-----------------------------------------------
@x 
@d not_found4=49 {like |not_found|, when there's more than four}
@y
@d not_found4=49 {like |not_found|, when there's more than four}
@d not_found5=50 {like |not_found|, when there's more than five}
@z
%-----------------------------------------------
@x
specifies the order of infinity to which glue setting applies (|normal|,
|fil|, |fill|, or |filll|). The |subtype| field is not used in \TeX.
@y
specifies the order of infinity to which glue setting applies (|normal|,
|sfi|, |fil|, |fill|, or |filll|). The |subtype| field is not used in \TeX.
@z
%-----------------------------------------------
@x
orders of infinity (|normal|, |fil|, |fill|, or |filll|)
@y
orders of infinity (|normal|, |sfi|, |fil|, |fill|, or |filll|)
@z
%-----------------------------------------------
@x
@d fil=1 {first-order infinity}
@d fill=2 {second-order infinity}
@d filll=3 {third-order infinity}
@y
@d sfi=1 {first-order infinity}
@d fil=2 {second-order infinity}
@d fill=3 {third-order infinity}
@d filll=4 {fourth-order infinity}
@z
%-----------------------------------------------
@x
@!glue_ord=normal..filll; {infinity to the 0, 1, 2, or 3 power}
@y
@!glue_ord=normal..filll; {infinity to the 0, 1, 2, 3, or 4 power}
@z
%-----------------------------------------------
@x
@d fil_glue==zero_glue+glue_spec_size {\.{0pt plus 1fil minus 0pt}}
@y
@d sfi_glue==zero_glue+glue_spec_size {\.{0pt plus 1fi minus 0pt}}
@d fil_glue==sfi_glue+glue_spec_size {\.{0pt plus 1fil minus 0pt}}
@z
%-----------------------------------------------
@x
stretch(fil_glue):=unity; stretch_order(fil_glue):=fil;@/
stretch(fill_glue):=unity; stretch_order(fill_glue):=fill;@/
@y
stretch(sfi_glue):=unity; stretch_order(sfi_glue):=sfi;@/
stretch(fil_glue):=unity; stretch_order(fil_glue):=fil;@/
stretch(fill_glue):=unity; stretch_order(fill_glue):=fill;@/
@z
%-----------------------------------------------
@x
  begin print("fil");
  while order>fil do
@y
  begin print("fi");
  while order>sfi do
@z
%-----------------------------------------------
@x
@d last_item=math_given+1 {most recent item ( \.{\\lastpenalty},
@y
@d omath_given=math_given+1 {math code defined by \.{\\omathchardef}}
@d last_item=omath_given+1 {most recent item ( \.{\\lastpenalty},
@z
%-----------------------------------------------
@x
@d math_font_base=cur_font_loc+1 {table of 48 math font numbers}
@d cur_jfont_loc=math_font_base+48
@y
@d math_font_base=cur_font_loc+1 {table of 768 math font numbers}
@d cur_jfont_loc=math_font_base+768
@z
%-----------------------------------------------
@x
@d var_code==@'70000 {math code meaning ``use the current family''}
@y
@d var_code==@"70000 {math code meaning ``use the current family''}
@z
%-----------------------------------------------
@x
for k:=math_font_base to math_font_base+47 do eqtb[k]:=eqtb[cur_font_loc];
@y
for k:=math_font_base to math_font_base+767 do eqtb[k]:=eqtb[cur_font_loc];
@z
%-----------------------------------------------
@x
begin if n=cur_font_loc then print("current font")
else if n<math_font_base+16 then
  begin print_esc("textfont"); print_int(n-math_font_base);
  end
else if n<math_font_base+32 then
  begin print_esc("scriptfont"); print_int(n-math_font_base-16);
  end
else  begin print_esc("scriptscriptfont"); print_int(n-math_font_base-32);
@y
begin if n=cur_font_loc then print("current font")
else if n<math_font_base+script_size then
  begin print_esc("textfont"); print_int(n-math_font_base);
  end
else if n<math_font_base+script_script_size then
  begin print_esc("scriptfont"); print_int(n-math_font_base-script_size);
  end
else  begin print_esc("scriptscriptfont"); 
  print_int(n-math_font_base-script_script_size);
@z
%-----------------------------------------------
@x
@d del_code(#)==eqtb[del_code_base+#].int
@y
@d del_code(#)==eqtb[del_code_base+#].int
@d del_code1(#)==getintone(eqtb[del_code_base+#])
@z
%-----------------------------------------------
@x
for k:=0 to 255 do del_code(k):=-1;
del_code("."):=0; {this null delimiter is used in error recovery}
@y
for k:=0 to 255 do
  begin del_code(k):=-1; setintone(eqtb[del_code_base+k],-1);
  end;
del_code("."):=0; setintone(eqtb[del_code_base+"."],0);
      {this null delimiter is used in error recovery}
@z
%-----------------------------------------------
@x
primitive("delimiter",delim_num,0);@/
@!@:delimiter_}{\.{\\delimiter} primitive@>
@y
primitive("delimiter",delim_num,0);@/
@!@:delimiter_}{\.{\\delimiter} primitive@>
primitive("odelimiter",delim_num,1);@/
@!@:delimiter_}{\.{\\odelimiter} primitive@>
@z
%-----------------------------------------------
@x
primitive("mathaccent",math_accent,0);@/
@!@:math_accent_}{\.{\\mathaccent} primitive@>
primitive("mathchar",math_char_num,0);@/
@!@:math_char_}{\.{\\mathchar} primitive@>
@y
primitive("mathaccent",math_accent,0);@/
@!@:math_accent_}{\.{\\mathaccent} primitive@>
primitive("mathchar",math_char_num,0);@/
@!@:math_char_}{\.{\\mathchar} primitive@>
primitive("omathaccent",math_accent,1);@/
@!@:math_accent_}{\.{\\omathaccent} primitive@>
primitive("omathchar",math_char_num,1);@/
@!@:math_char_}{\.{\\omathchar} primitive@>
@z
%-----------------------------------------------
@x
primitive("radical",radical,0);@/
@!@:radical_}{\.{\\radical} primitive@>
@y
primitive("radical",radical,0);@/
@!@:radical_}{\.{\\radical} primitive@>
primitive("oradical",radical,1);@/
@!@:radical_}{\.{\\oradical} primitive@>
@z
%-----------------------------------------------
@x
delim_num: print_esc("delimiter");
@y
delim_num: if chr_code=0 then print_esc("delimiter")
  else print_esc("odelimiter");
@z
@x
math_accent: print_esc("mathaccent");
math_char_num: print_esc("mathchar");
@y
math_accent: if chr_code=0 then print_esc("mathaccent")
  else print_esc("omathaccent");
math_char_num: if chr_code=0 then print_esc("mathchar")
  else print_esc("omathchar");
@z
@x
radical: print_esc("radical");
@y
radical: if chr_code=0 then print_esc("radical")
  else print_esc("oradical");
@z
%-----------------------------------------------
@x
@p procedure eq_word_define(@!p:pointer;@!w:integer);
label exit;
begin if eTeX_ex and(eqtb[p].int=w) then
  begin assign_trace(p,"reassigning")@;@/
  return;
  end;
assign_trace(p,"changing")@;@/
if xeq_level[p]<>cur_level then
  begin eq_save(p,xeq_level[p]); xeq_level[p]:=cur_level;
  end;
eqtb[p].int:=w;
assign_trace(p,"into")@;@/
exit:end;
@y
@p procedure eq_word_define(@!p:pointer;@!w:integer);
label exit;
begin if eTeX_ex and(eqtb[p].int=w) then
  begin assign_trace(p,"reassigning")@;@/
  return;
  end;
assign_trace(p,"changing")@;@/
if xeq_level[p]<>cur_level then
  begin eq_save(p,xeq_level[p]); xeq_level[p]:=cur_level;
  end;
eqtb[p].int:=w;
assign_trace(p,"into")@;@/
exit:end;
@#
procedure del_eq_word_define(@!p:pointer;@!w,wone:integer);
label exit;
begin if eTeX_ex and(eqtb[p].int=w)and(getintone(eqtb[p])=wone) then
  begin assign_trace(p,"reassigning")@;@/
  return;
  end;
assign_trace(p,"changing")@;@/
if xeq_level[p]<>cur_level then
  begin eq_save(p,xeq_level[p]); xeq_level[p]:=cur_level;
  end;
eqtb[p].int:=w; setintone(eqtb[p],wone);
assign_trace(p,"into")@;@/
exit:end;
@z
%-----------------------------------------------
@x
procedure geq_word_define(@!p:pointer;@!w:integer); {global |eq_word_define|}
begin assign_trace(p,"globally changing")@;@/
begin eqtb[p].int:=w; xeq_level[p]:=level_one;
end;
assign_trace(p,"into")@;@/
end;
@y
procedure geq_word_define(@!p:pointer;@!w:integer); {global |eq_word_define|}
begin assign_trace(p,"globally changing")@;@/
begin eqtb[p].int:=w; xeq_level[p]:=level_one;
end;
assign_trace(p,"into")@;@/
end;
@#
procedure del_geq_word_define(@!p:pointer;@!w,wone:integer); 
  {global |del_eq_word_define|}
begin assign_trace(p,"globally changing")@;@/
begin eqtb[p].int:=w; setintone(eqtb[p],wone); xeq_level[p]:=level_one;
end;
assign_trace(p,"into")@;@/
end;
@z
%-----------------------------------------------
@x
@d tok_val=5 {token lists}

@<Glob...@>=
@!cur_val:integer; {value returned by numeric scanners}
@y
@d tok_val=5 {token lists}

@<Glob...@>=
@!cur_val:integer; {value returned by numeric scanners}
@!cur_val1:integer; 
@z
%-----------------------------------------------
@x
char_given,math_given: scanned_result(cur_chr)(int_val);
@y
omath_given,
char_given,math_given: scanned_result(cur_chr)(int_val);
@z
%-----------------------------------------------
@x
@ @<Fetch a character code from some table@>=
begin
if m=math_code_base then
  begin scan_ascii_num;
  scanned_result(ho(math_code(cur_val)))(int_val); end
@y
@ @<Fetch a character code from some table@>=
begin
if m=math_code_base then begin
  scan_ascii_num; cur_val1:=ho(math_code(cur_val));
  if ((cur_val1 div @"10000)>8) or
     (((cur_val1 mod @"10000) div @"100)>15) then
    begin print_err("Extended mathchar used as mathchar");
@.Bad mathchar@>
    help2("A mathchar number must be between 0 and ""7FFF.")@/
      ("I changed this one to zero."); int_error(cur_val1);
    scanned_result(0)(int_val)
    end;
  cur_val1:=(cur_val1 div @"10000)*@"1000+cur_val1 mod @"1000;
  scanned_result(cur_val1)(int_val);
  end
else if m=(math_code_base+128) then begin
  scan_ascii_num; cur_val1:=ho(math_code(cur_val));
  cur_val:=(cur_val1 div @"10000) * @"1000000 
           +((cur_val1 div @"100) mod @"100) * @"10000
           +(cur_val1 mod @"100);
  scanned_result(cur_val)(int_val);
  end
else if m=del_code_base then begin
  scan_ascii_num; cur_val1:=del_code(cur_val); cur_val:=del_code1(cur_val);
  if ((cur_val1 div @"100) mod @"100 >= 16) or (cur_val>=@"1000) then
  begin print_err("Extended delimiter code used as delcode");
@.Bad delimiter code@>
    help2("A numeric delimiter code must be between 0 and 2^{27}-1.")@/
      ("I changed this one to zero."); error;
    scanned_result(0)(int_val);
    end
  else if cur_val1<0 then
    scanned_result(cur_val)(int_val)
  else
    scanned_result(cur_val1*@"1000+cur_val)(int_val);
  end
else if m=(del_code_base+128) then begin
  { Aleph seems \.{\\odelcode} always returns $-1$.}
  scan_ascii_num; scanned_result(-1)(int_val);
  end
@z
%-----------------------------------------------
@x
procedure scan_four_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>15) then
  begin print_err("Bad number");
@.Bad number@>
  help2("Since I expected to read a number between 0 and 15,")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;
@y
procedure scan_four_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>15) then
  begin print_err("Bad number");
@.Bad number@>
  help2("Since I expected to read a number between 0 and 15,")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;
@#
procedure scan_big_four_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>255) then
  begin print_err("Bad number");
@.Bad register code@>
  help2("Since I expected to read a number between 0 and 255,")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;
@z
%-----------------------------------------------
@x
procedure scan_fifteen_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>@'77777) then
  begin print_err("Bad mathchar");
@.Bad mathchar@>
  help2("A mathchar number must be between 0 and 32767.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;
@y
procedure scan_fifteen_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>@'77777) then
  begin print_err("Bad mathchar");
@.Bad mathchar@>
  help2("A mathchar number must be between 0 and 32767.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
cur_val:=((cur_val div @"1000) * @"10000)+(cur_val mod @"1000);
end; 
@#
procedure scan_real_fifteen_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>@'77777) then
  begin print_err("Bad mathchar");
@.Bad mathchar@>
  help2("A mathchar number must be between 0 and 32767.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;
@#
procedure scan_big_fifteen_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>@"7FFFFFF) then
  begin print_err("Bad extended mathchar");
@.Bad mathchar@>
  help2("An extended mathchar number must be between 0 and ""7FFFFFF.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
{ e-pTeX doesn't support 65536 characters for math font. }
cur_val:=((cur_val div @"10000) * @"100)+(cur_val mod @"100);
end;
@#
procedure scan_omega_fifteen_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>@"7FFFFFF) then
  begin print_err("Bad extended mathchar");
@.Bad mathchar@>
  help2("An extended mathchar number must be between 0 and ""7FFFFFF.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;
@z
%-----------------------------------------------
@x
procedure scan_twenty_seven_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>@'777777777) then
  begin print_err("Bad delimiter code");
@.Bad delimiter code@>
  help2("A numeric delimiter code must be between 0 and 2^{27}-1.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;
@y
procedure scan_twenty_seven_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>@'777777777) then
  begin print_err("Bad delimiter code");
@.Bad delimiter code@>
  help2("A numeric delimiter code must be between 0 and 2^{27}-1.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
cur_val1 := cur_val mod @"1000; cur_val := cur_val div @"1000;
cur_val := ((cur_val div @"1000) * @"10000) + (cur_val mod @"1000);
end;
@#
procedure scan_fifty_one_bit_int;
var iiii:integer;
begin scan_int;
if (cur_val<0)or(cur_val>@'777777777) then
  begin print_err("Bad delimiter code");
@.Bad delimiter code@>
  help2("A numeric delimiter (first part) must be between 0 and 2^{27}-1.")
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
iiii:=((cur_val div @"10000) * @"100) + (cur_val mod @"100);
scan_int;
if (cur_val<0)or(cur_val>@"FFFFFF) then
  begin print_err("Bad delimiter code");
@.Bad delimiter code@>
help2("A numeric delimiter (second part) must be between 0 and 2^{24}-1.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
{ e-pTeX doesn't support 65536 characters for math font. }
cur_val1:=((cur_val div @"10000) * @"100) + (cur_val mod @"100);
cur_val:=iiii;
end;
@z
%-----------------------------------------------
@x
@<Scan for \(f)\.{fil} units...@>=
if scan_keyword("fil") then
@.fil@>
  begin cur_order:=fil;
@y
@<Scan for \(f)\.{fil} units...@>=
if scan_keyword("fi") then
@.fil@>
  begin cur_order:=sfi;
@z
%-----------------------------------------------
@x
else if cur_cmd=def_family then
  begin m:=cur_chr; scan_four_bit_int; f:=equiv(m+cur_val);
@y
else if cur_cmd=def_family then
  begin m:=cur_chr; scan_big_four_bit_int; f:=equiv(m+cur_val);
@z
%-----------------------------------------------
@x
total_stretch[normal]:=0; total_shrink[normal]:=0;
@y
total_stretch[normal]:=0; total_shrink[normal]:=0;
total_stretch[sfi]:=0; total_shrink[sfi]:=0;
@z
%-----------------------------------------------
@x
else if total_stretch[fil]<>0 then o:=fil
@y
else if total_stretch[fil]<>0 then o:=fil
else if total_stretch[sfi]<>0 then o:=sfi
@z
%-----------------------------------------------
@x
else if total_shrink[fil]<>0 then o:=fil
@y
else if total_shrink[fil]<>0 then o:=fil
else if total_shrink[sfi]<>0 then o:=sfi
@z
%-----------------------------------------------
@x
\yskip\hang|math_type(q)=math_char| means that |fam(q)| refers to one of
the sixteen font families, and |character(q)| is the number of a character
@y
\yskip\hang|math_type(q)=math_char| means that |fam(q)| refers to one of
the 256 font families, and |character(q)| is the number of a character
@z
%-----------------------------------------------
@x
procedure print_fam_and_char(@!p:pointer;@!t:small_number);
@y
procedure print_fam_and_char(@!p:pointer;@!t:integer);
@z
%-----------------------------------------------
@x
other font information. A size code, which is a multiple of 16, is added to a
family number to get an index into the table of internal font numbers
for each combination of family and size.  (Be alert: Size codes get
larger as the type gets smaller.)

@d text_size=0 {size code for the largest size in a family}
@d script_size=16 {size code for the medium size in a family}
@d script_script_size=32 {size code for the smallest size in a family}
@y
other font information. A size code, which is a multiple of 256, is added to a
family number to get an index into the table of internal font numbers
for each combination of family and size.  (Be alert: Size codes get
larger as the type gets smaller.)
@z
%-----------------------------------------------
@x
else cur_size:=16*((cur_style-text_style) div 2);
@y
else cur_size:=script_size*((cur_style-text_style) div 2);
@z
%-----------------------------------------------
@x
function var_delimiter(@!d:pointer;@!s:small_number;@!v:scaled):pointer;
@y
function var_delimiter(@!d:pointer;@!s:integer;@!v:scaled):pointer;
@z
%-----------------------------------------------
@x
@!hd: sixteen_bits; {height-depth byte}
@!r: four_quarters; {extensible pieces}
@!z: small_number; {runs through font family members}
@y
@!hd: sixteen_bits; {height-depth byte}
@!r: four_quarters; {extensible pieces}
@!z: integer; {runs through font family members}
@z
%-----------------------------------------------
@x
  begin z:=z+s+16;
  repeat z:=z-16; g:=fam_fnt(z);
@y
  begin z:=z+s+script_size;
  repeat z:=z-script_size; g:=fam_fnt(z);
@z
%-----------------------------------------------
@x
  until z<16;
@y
  until z<script_size;
@z
%-----------------------------------------------
@x
@!cur_mlist:pointer; {beginning of mlist to be translated}
@!cur_style:small_number; {style code at current place in the list}
@!cur_size:small_number; {size code corresponding to |cur_style|}
@y
@!cur_mlist:pointer; {beginning of mlist to be translated}
@!cur_style:small_number; {style code at current place in the list}
@!cur_size:integer; {size code corresponding to |cur_style|}
@z
%-----------------------------------------------
@x
@!p,@!x,@!y,@!z: pointer; {temporary registers for list construction}
@!pen:integer; {a penalty to be inserted}
@!s:small_number; {the size of a noad to be deleted}
@y
@!p,@!x,@!y,@!z: pointer; {temporary registers for list construction}
@!pen:integer; {a penalty to be inserted}
@!s:integer; {the size of a noad to be deleted}
@z
%-----------------------------------------------
@x
procedure make_scripts(@!q:pointer;@!delta:scaled);
var p,@!x,@!y,@!z:pointer; {temporary registers for box construction}
@!shift_up,@!shift_down,@!clr:scaled; {dimensions in the calculation}
@!t:small_number; {subsidiary size code}
@y
procedure make_scripts(@!q:pointer;@!delta:scaled);
var p,@!x,@!y,@!z:pointer; {temporary registers for box construction}
@!shift_up,@!shift_down,@!clr:scaled; {dimensions in the calculation}
@!t:integer; {subsidiary size code}
@z
%-----------------------------------------------
@x
contains six scaled numbers, since it must record the net change in glue
stretchability with respect to all orders of infinity. The natural width
difference appears in |mem[q+1].sc|; the stretch differences in units of
pt, fil, fill, and filll appear in |mem[q+2..q+5].sc|; and the shrink difference
appears in |mem[q+6].sc|. The |subtype| field of a delta node is not used.

@d delta_node_size=7 {number of words in a delta node}
@y
contains seven scaled numbers, since it must record the net change in glue
stretchability with respect to all orders of infinity. The natural width
difference appears in |mem[q+1].sc|; the stretch differences in units of
pt, sfi, fil, fill, and filll appear in |mem[q+2..q+6].sc|; and the shrink
difference appears in |mem[q+7].sc|. The |subtype| field of a delta node
is not used.

@d delta_node_size=8 {number of words in a delta node}
@z
%-----------------------------------------------
@x
@ As the algorithm runs, it maintains a set of six delta-like registers
for the length of the line following the first active breakpoint to the
current position in the given hlist. When it makes a pass through the
active list, it also maintains a similar set of six registers for the
@y
@ As the algorithm runs, it maintains a set of seven delta-like registers
for the length of the line following the first active breakpoint to the
current position in the given hlist. When it makes a pass through the
active list, it also maintains a similar set of seven registers for the
@z
%-----------------------------------------------
@x
k:=1 to 6 do cur_active_width[k]:=cur_active_width[k]+mem[q+k].sc|};$$ and we
want to do this without the overhead of |for| loops. The |do_all_six|
macro makes such six-tuples convenient.

@d do_all_six(#)==#(1);#(2);#(3);#(4);#(5);#(6)
@y
k:=1 to 7 do cur_active_width[k]:=cur_active_width[k]+mem[q+k].sc|};$$ and we
want to do this without the overhead of |for| loops. The |do_all_six|
macro makes such seven-tuples convenient.

@d do_all_six(#)==#(1);#(2);#(3);#(4);#(5);#(6);#(7)
@z
%-----------------------------------------------
@x
@!active_width:array[1..6] of scaled;
  {distance from first active node to~|cur_p|}
@!cur_active_width:array[1..6] of scaled; {distance from current active node}
@!background:array[1..6] of scaled; {length of an ``empty'' line}
@!break_width:array[1..6] of scaled; {length being computed after current break}
@y
@!active_width:array[1..7] of scaled;
  {distance from first active node to~|cur_p|}
@!cur_active_width:array[1..7] of scaled; {distance from current active node}
@!background:array[1..7] of scaled; {length of an ``empty'' line}
@!break_width:array[1..7] of scaled; {length being computed after current break}
@z
%-----------------------------------------------
@x
background[2]:=0; background[3]:=0; background[4]:=0; background[5]:=0;@/
background[2+stretch_order(q)]:=stretch(q);@/
background[2+stretch_order(r)]:=@|background[2+stretch_order(r)]+stretch(r);@/
background[6]:=shrink(q)+shrink(r);
@y
background[2]:=0; background[3]:=0; background[4]:=0; background[5]:=0;@/
background[6]:=0;@/
background[2+stretch_order(q)]:=stretch(q);@/
background[2+stretch_order(r)]:=@|background[2+stretch_order(r)]+stretch(r);@/
background[7]:=shrink(q)+shrink(r);
@z
%-----------------------------------------------
@x
      break_width[6]:=break_width[6]-shrink(cur_kanji_skip);
@y
      break_width[7]:=break_width[7]-shrink(cur_kanji_skip);
@z
%-----------------------------------------------
@x
break_width[6]:=break_width[6]-shrink(v);
@y
break_width[7]:=break_width[7]-shrink(v);
@z
%-----------------------------------------------
@x
subarray |cur_active_width[2..5]|, in units of points, fil, fill, and filll.
@y
subarray |cur_active_width[2..6]|, in units of points, sfi, fil, fill and filll.
@z
%-----------------------------------------------
@x
if (cur_active_width[3]<>0)or(cur_active_width[4]<>0)or@|
  (cur_active_width[5]<>0) then
@y
if (cur_active_width[3]<>0)or(cur_active_width[4]<>0)or@|
  (cur_active_width[5]<>0)or(cur_active_width[6]<>0) then
@z
%-----------------------------------------------
@x
@ Shrinkability is never infinite in a paragraph;
we can shrink the line from |r| to |cur_p| by at most |cur_active_width[6]|.

@<Set the value of |b| to the badness for shrinking...@>=
begin if -shortfall>cur_active_width[6] then b:=inf_bad+1
else b:=badness(-shortfall,cur_active_width[6]);
@y
@ Shrinkability is never infinite in a paragraph;
we can shrink the line from |r| to |cur_p| by at most |cur_active_width[7]|.

@<Set the value of |b| to the badness for shrinking...@>=
begin if -shortfall>cur_active_width[7] then b:=inf_bad+1
else b:=badness(-shortfall,cur_active_width[7]);
@z
%-----------------------------------------------
@x
      active_width[6]:=active_width[6]+shrink(cur_kanji_skip);
@y
      active_width[7]:=active_width[7]+shrink(cur_kanji_skip);
@z
%-----------------------------------------------
@x
active_width[6]:=active_width[6]+shrink(q)
@y
active_width[7]:=active_width[7]+shrink(q)
@z
%-----------------------------------------------
@x
  if (active_height[3]<>0) or (active_height[4]<>0) or
    (active_height[5]<>0) then b:=0
  else b:=badness(h-cur_height,active_height[2])
else if cur_height-h>active_height[6] then b:=awful_bad
else b:=badness(cur_height-h,active_height[6])
@y
  if (active_height[3]<>0) or (active_height[4]<>0) or
    (active_height[5]<>0) or (active_height[6]<>0) then b:=0
  else b:=badness(h-cur_height,active_height[2])
else if cur_height-h>active_height[7] then b:=awful_bad
else b:=badness(cur_height-h,active_height[7])
@z
%-----------------------------------------------
@x
  active_height[6]:=active_height[6]+shrink(q);
@y
  active_height[7]:=active_height[7]+shrink(q);
@z
%-----------------------------------------------
@x
on the current page. This array contains six |scaled| numbers, like the
@y
on the current page. This array contains seven |scaled| numbers, like the
@z
%-----------------------------------------------
@x
@d page_shrink==page_so_far[6] {shrinkability of the current page}
@d page_depth==page_so_far[7] {depth of the current page}
@y
@d page_shrink==page_so_far[7] {shrinkability of the current page}
@d page_depth==page_so_far[8] {depth of the current page}
@z
%-----------------------------------------------
@x
@<Glob...@>=
@!page_so_far:array [0..7] of scaled; {height and glue of the current page}
@y
@<Glob...@>=
@!page_so_far:array [0..8] of scaled; {height and glue of the current page}
@z
%-----------------------------------------------
@x
primitive("pagefilstretch",set_page_dimen,3);
@!@:page_fil_stretch_}{\.{\\pagefilstretch} primitive@>
primitive("pagefillstretch",set_page_dimen,4);
@!@:page_fill_stretch_}{\.{\\pagefillstretch} primitive@>
primitive("pagefilllstretch",set_page_dimen,5);
@!@:page_filll_stretch_}{\.{\\pagefilllstretch} primitive@>
primitive("pageshrink",set_page_dimen,6);
@!@:page_shrink_}{\.{\\pageshrink} primitive@>
primitive("pagedepth",set_page_dimen,7);
@!@:page_depth_}{\.{\\pagedepth} primitive@>
@y
primitive("pagefistretch",set_page_dimen,3);
@!@:page_fi_stretch_}{\.{\\pagefistretch} primitive@>
primitive("pagefilstretch",set_page_dimen,4);
@!@:page_fil_stretch_}{\.{\\pagefilstretch} primitive@>
primitive("pagefillstretch",set_page_dimen,5);
@!@:page_fill_stretch_}{\.{\\pagefillstretch} primitive@>
primitive("pagefilllstretch",set_page_dimen,6);
@!@:page_filll_stretch_}{\.{\\pagefilllstretch} primitive@>
primitive("pageshrink",set_page_dimen,7);
@!@:page_shrink_}{\.{\\pageshrink} primitive@>
primitive("pagedepth",set_page_dimen,8);
@!@:page_depth_}{\.{\\pagedepth} primitive@>
@z
%-----------------------------------------------
@x
3: print_esc("pagefilstretch");
4: print_esc("pagefillstretch");
5: print_esc("pagefilllstretch");
6: print_esc("pageshrink");
@y
3: print_esc("pagefistretch");
4: print_esc("pagefilstretch");
5: print_esc("pagefillstretch");
6: print_esc("pagefilllstretch");
7: print_esc("pageshrink");
@z
%-----------------------------------------------
@x
print_plus(3)("fil");
print_plus(4)("fill");
print_plus(5)("filll");
@y
print_plus(3)("fi");
print_plus(4)("fil");
print_plus(5)("fill");
print_plus(6)("filll");
@z
%-----------------------------------------------
@x
  if (page_so_far[3]<>0) or (page_so_far[4]<>0) or@|
    (page_so_far[5]<>0) then b:=0
@y
  if (page_so_far[3]<>0) or (page_so_far[4]<>0) or@|
    (page_so_far[5]<>0) or (page_so_far[6]<>0) then b:=0
@z
%-----------------------------------------------
@x
non_math(math_given), non_math(math_comp), non_math(delim_num),
@y
non_math(math_given), non_math(omath_given),
non_math(math_comp), non_math(delim_num),
@z
%-----------------------------------------------
@x
@d fil_code=0 {identifies \.{\\hfil} and \.{\\vfil}}
@d fill_code=1 {identifies \.{\\hfill} and \.{\\vfill}}
@d ss_code=2 {identifies \.{\\hss} and \.{\\vss}}
@d fil_neg_code=3 {identifies \.{\\hfilneg} and \.{\\vfilneg}}
@d skip_code=4 {identifies \.{\\hskip} and \.{\\vskip}}
@d mskip_code=5 {identifies \.{\\mskip}}
@y
@d sfi_code=0 {identifies \.{\\hfi} and \.{\\vfi}}
@d fil_code=1 {identifies \.{\\hfil} and \.{\\vfil}}
@d fill_code=2 {identifies \.{\\hfill} and \.{\\vfill}}
@d ss_code=3 {identifies \.{\\hss} and \.{\\vss}}
@d fil_neg_code=4 {identifies \.{\\hfilneg} and \.{\\vfilneg}}
@d skip_code=5 {identifies \.{\\hskip} and \.{\\vskip}}
@d mskip_code=6 {identifies \.{\\mskip}}
@z
%-----------------------------------------------
@x
primitive("hfil",hskip,fil_code);
@!@:hfil_}{\.{\\hfil} primitive@>
@y
primitive("hfi",hskip,sfi_code);
@!@:hfi_}{\.{\\hfi} primitive@>
primitive("hfil",hskip,fil_code);
@!@:hfil_}{\.{\\hfil} primitive@>
@z
%-----------------------------------------------
@x
primitive("vfil",vskip,fil_code);
@!@:vfil_}{\.{\\vfil} primitive@>
@y
primitive("vfi",vskip,sfi_code);
@!@:vfi_}{\.{\\vfi} primitive@>
primitive("vfil",vskip,fil_code);
@!@:vfil_}{\.{\\vfil} primitive@>
@z
%-----------------------------------------------
@x
hskip: case chr_code of
  skip_code:print_esc("hskip");
@y
hskip: case chr_code of
  skip_code:print_esc("hskip");
  sfi_code:print_esc("hfi");
@z
%-----------------------------------------------
@x
vskip: case chr_code of
  skip_code:print_esc("vskip");
@y
vskip: case chr_code of
  skip_code:print_esc("vskip");
  sfi_code:print_esc("vfi");
@z
%-----------------------------------------------
@x
begin s:=cur_chr;
case s of
fil_code: cur_val:=fil_glue;
@y
begin s:=cur_chr;
case s of
sfi_code: cur_val:=sfi_glue;
fil_code: cur_val:=fil_glue;
@z
%-----------------------------------------------
@x
|global_box_flag-1| represent `\.{\\setbox0}' through `\.{\\setbox32767}';
codes |global_box_flag| through |ship_out_flag-1| represent
`\.{\\global\\setbox0}' through `\.{\\global\\setbox32767}';
@y
|global_box_flag-1| represent `\.{\\setbox0}' through `\.{\\setbox65535}';
codes |global_box_flag| through |ship_out_flag-1| represent
`\.{\\global\\setbox0}' through `\.{\\global\\setbox65535}';
@z
%-----------------------------------------------
@x
@d box_flag==@'10000000000 {context code for `\.{\\setbox0}'}
@d global_box_flag==@'10000100000 {context code for `\.{\\global\\setbox0}'}
@d ship_out_flag==@'10000200000  {context code for `\.{\\shipout}'}
@d leader_flag==@'10000200001  {context code for `\.{\\leaders}'}
@y
@d box_flag==@"40000000 {context code for `\.{\\setbox0}'}
@d global_box_flag==@"40010000 {context code for `\.{\\global\\setbox0}'}
@d ship_out_flag==@"40020000  {context code for `\.{\\shipout}'}
@d leader_flag==@"40020001  {context code for `\.{\\leaders}'}
@z
%-----------------------------------------------
@x
@d fam_in_range==((cur_fam>=0)and(cur_fam<16))
@y
@d fam_in_range==((cur_fam>=0)and(cur_fam<script_size))
@z
%-----------------------------------------------
@x
    c:=ho(math_code(cur_chr));
    if c=@'100000 then
@y
    c:=ho(math_code(cur_chr));
    if c=@"80000 then
@z
%-----------------------------------------------
@x
math_char_num: begin scan_fifteen_bit_int; c:=cur_val;
  end;
math_given: c:=cur_chr;
delim_num: begin scan_twenty_seven_bit_int; c:=cur_val div @'10000;
@y
math_char_num: begin
  if cur_chr=0 then scan_fifteen_bit_int
  else scan_big_fifteen_bit_int;
  c:=cur_val;
  end;
math_given: c:=((cur_chr div @"1000) * @"10000) + (cur_chr mod @"1000);
omath_given: c:=((cur_chr div @"10000) * @"100) + (cur_chr mod @"100);
delim_num: begin
  if cur_chr=0 then scan_twenty_seven_bit_int
  else scan_fifty_one_bit_int;
  c:=cur_val;
@z
%-----------------------------------------------
@x
  if (c>=var_code)and(fam_in_range) then fam(p):=cur_fam
  else fam(p):=(c div 256) mod 16;
@y
  if (c>=var_code)and(fam_in_range) then fam(p):=cur_fam
  else fam(p):=(c div 256) mod 256;
@z
%-----------------------------------------------
@x
mmode+math_char_num: begin scan_fifteen_bit_int; set_math_char(cur_val);
  end;
mmode+math_given: set_math_char(cur_chr);
mmode+delim_num: begin scan_twenty_seven_bit_int;
  set_math_char(cur_val div @'10000);
@y
mmode+math_char_num: begin
  if cur_chr=0 then scan_fifteen_bit_int
  else scan_big_fifteen_bit_int;
  set_math_char(cur_val);
  end;
mmode+math_given: begin
  set_math_char(((cur_chr div @"1000) * @"10000)+(cur_chr mod @"1000));
  end;
mmode+omath_given: begin
  set_math_char(((cur_chr div @"10000) * @"100)+(cur_chr mod @"100));
  end;
mmode+delim_num: begin
  if cur_chr=0 then scan_twenty_seven_bit_int
  else scan_fifty_one_bit_int;
  set_math_char(cur_val); {character code of left delimiter}
@z
%-----------------------------------------------
@x
procedure set_math_char(@!c:integer);
var p:pointer; {the new noad}
begin if c>=@'100000 then
  @<Treat |cur_chr|...@>
else  begin p:=new_noad; math_type(nucleus(p)):=math_char;
  character(nucleus(p)):=qi(c mod 256);
  fam(nucleus(p)):=(c div 256) mod 16;
  if c>=var_code then
    begin if fam_in_range then fam(nucleus(p)):=cur_fam;
    type(p):=ord_noad;
    end
  else  type(p):=ord_noad+(c div @'10000);
  link(tail):=p; tail:=p;
  if font_dir[fam_fnt(fam(nucleus(p))+cur_size)]<>dir_default then begin
@y
procedure set_math_char(@!c:integer);
var p:pointer; {the new noad}
begin if c>=@"80000 then
  @<Treat |cur_chr|...@>
else  begin p:=new_noad; math_type(nucleus(p)):=math_char;
  character(nucleus(p)):=qi(c mod 256);
  fam(nucleus(p)):=(c div 256) mod 256;
  if c>=var_code then
    begin if fam_in_range then fam(nucleus(p)):=cur_fam;
    type(p):=ord_noad;
   end
  else  type(p):=ord_noad+(c div @"10000);
  link(tail):=p; tail:=p;
  if font_dir[fam_fnt(fam(nucleus(p))+cur_size)]<>dir_default then begin
@z
%-----------------------------------------------
@x
@<Declare act...@>=
procedure scan_delimiter(@!p:pointer;@!r:boolean);
begin if r then scan_twenty_seven_bit_int
else  begin @<Get the next non-blank non-relax...@>;
  case cur_cmd of
  letter,other_char: cur_val:=del_code(cur_chr);
  delim_num: scan_twenty_seven_bit_int;
  othercases cur_val:=-1
  endcases;
  end;
if cur_val<0 then @<Report that an invalid delimiter code is being changed
   to null; set~|cur_val:=0|@>;
small_fam(p):=(cur_val div @'4000000) mod 16;
small_char(p):=qi((cur_val div @'10000) mod 256);
large_fam(p):=(cur_val div 256) mod 16;
large_char(p):=qi(cur_val mod 256);
end;
@y
@<Declare act...@>=
procedure scan_delimiter(@!p:pointer;@!r:boolean);
begin if r=1 then scan_twenty_seven_bit_int
else if r=2 then scan_fifty_one_bit_int
else  begin @<Get the next non-blank non-relax...@>;
  case cur_cmd of
  letter,other_char: begin
    cur_val:=del_code(cur_chr); cur_val1:=del_code1(cur_chr);
    end;
  delim_num: if cur_chr=0 then scan_twenty_seven_bit_int
             else scan_fifty_one_bit_int;
  othercases begin cur_val:=-1; cur_val1:=-1; end;
  endcases;
  end;
if cur_val<0 then begin @<Report that an invalid delimiter code is being changed
   to null; set~|cur_val:=0|@>;
 cur_val1:=0;
 end;
small_fam(p):=(cur_val div @"100) mod @"100;
small_char(p):=qi(cur_val mod @"100);
large_fam(p):=(cur_val1 div @"100) mod @"100;
large_char(p):=qi(cur_val1 mod @"100);
end;
@z
%-----------------------------------------------
@x
scan_delimiter(left_delimiter(tail),true);
@y
scan_delimiter(left_delimiter(tail),cur_chr+1);
@z
%-----------------------------------------------
@x
scan_fifteen_bit_int;
character(accent_chr(tail)):=qi(cur_val mod 256);
if (cur_val>=var_code)and fam_in_range then fam(accent_chr(tail)):=cur_fam
else fam(accent_chr(tail)):=(cur_val div 256) mod 16;
@y
if cur_chr=0 then scan_fifteen_bit_int
else scan_big_fifteen_bit_int;
character(accent_chr(tail)):=qi(cur_val mod 256);
if (cur_val>=var_code)and fam_in_range then fam(accent_chr(tail)):=cur_fam
else fam(accent_chr(tail)):=(cur_val div 256) mod 256;
@z
%-----------------------------------------------
@x
begin if (e<>0)and((w-total_shrink[normal]+q<=z)or@|
   (total_shrink[fil]<>0)or(total_shrink[fill]<>0)or
   (total_shrink[filll]<>0)) then
@y
begin if (e<>0)and((w-total_shrink[normal]+q<=z)or@|
   (total_shrink[sfi]<>0)or(total_shrink[fil]<>0)or
   (total_shrink[fill]<>0)or(total_shrink[filll]<>0)) then
@z
%-----------------------------------------------
@x
@d word_define(#)==if global then geq_word_define(#)@+else eq_word_define(#)
@y
@d word_define(#)==if global then geq_word_define(#)@+else eq_word_define(#)
@d del_word_define(#)==if global 
                       then del_geq_word_define(#)@+else del_eq_word_define(#)
@z
%-----------------------------------------------
@x
@d count_def_code=2 {|shorthand_def| for \.{\\countdef}}
@d dimen_def_code=3 {|shorthand_def| for \.{\\dimendef}}
@d skip_def_code=4 {|shorthand_def| for \.{\\skipdef}}
@d mu_skip_def_code=5 {|shorthand_def| for \.{\\muskipdef}}
@d toks_def_code=6 {|shorthand_def| for \.{\\toksdef}}
@d char_sub_def_code=7 {|shorthand_def| for \.{\\charsubdef}}
@y
@d omath_char_def_code=2 {|shorthand_def| for \.{\\omathchardef}}
@d count_def_code=3 {|shorthand_def| for \.{\\countdef}}
@d dimen_def_code=4 {|shorthand_def| for \.{\\dimendef}}
@d skip_def_code=5 {|shorthand_def| for \.{\\skipdef}}
@d mu_skip_def_code=6 {|shorthand_def| for \.{\\muskipdef}}
@d toks_def_code=7 {|shorthand_def| for \.{\\toksdef}}
@d char_sub_def_code=8 {|shorthand_def| for \.{\\charsubdef}}
@z
%-----------------------------------------------
@x
primitive("mathchardef",shorthand_def,math_char_def_code);@/
@!@:math_char_def_}{\.{\\mathchardef} primitive@>
@y
primitive("mathchardef",shorthand_def,math_char_def_code);@/
@!@:math_char_def_}{\.{\\mathchardef} primitive@>
primitive("omathchardef",shorthand_def,omath_char_def_code);@/
@!@:math_char_def_}{\.{\\omathchardef} primitive@>
@z
%-----------------------------------------------
@x
  math_char_def_code: print_esc("mathchardef");
@y
  math_char_def_code: print_esc("mathchardef");
  omath_char_def_code: print_esc("omathchardef");
@z
%-----------------------------------------------
@x
math_given: begin print_esc("mathchar"); print_hex(chr_code);
  end;
@y
math_given: begin print_esc("mathchar"); print_hex(chr_code);
  end;
omath_given: begin print_esc("omathchar"); print_hex(chr_code);
  end;
@z
%-----------------------------------------------
@x
  math_char_def_code: begin scan_fifteen_bit_int; define(p,math_given,cur_val);
@y
  math_char_def_code: begin scan_real_fifteen_bit_int;
    define(p,math_given,cur_val);
    end;
  omath_char_def_code: begin scan_omega_fifteen_bit_int;
    define(p,omath_given,cur_val);
@z
%-----------------------------------------------
@x
primitive("mathcode",def_code,math_code_base);
@!@:math_code_}{\.{\\mathcode} primitive@>
@y
primitive("mathcode",def_code,math_code_base);
@!@:math_code_}{\.{\\mathcode} primitive@>
primitive("omathcode",def_code,math_code_base+128);
@!@:math_code_}{\.{\\omathcode} primitive@>
@z
%-----------------------------------------------
@x
primitive("delcode",def_code,del_code_base);
@!@:del_code_}{\.{\\delcode} primitive@>
@y
primitive("delcode",def_code,del_code_base);
@!@:del_code_}{\.{\\delcode} primitive@>
primitive("odelcode",def_code,del_code_base+128);
@!@:del_code_}{\.{\\odelcode} primitive@>
@z
%-----------------------------------------------
@x
  else if chr_code=lc_code_base then print_esc("lccode")
  else if chr_code=uc_code_base then print_esc("uccode")
  else if chr_code=sf_code_base then print_esc("sfcode")
  else print_esc("delcode");
@y
  else if chr_code=math_code_base+128 then print_esc("omathcode")
  else if chr_code=lc_code_base then print_esc("lccode")
  else if chr_code=uc_code_base then print_esc("uccode")
  else if chr_code=sf_code_base then print_esc("sfcode")
  else if chr_code=del_code_base then print_esc("delcode")
  else print_esc("odelcode");
@z
%-----------------------------------------------
@x
@<Assignments@>=
def_code: begin
  @<Let |m| be the minimal legal code value, based on |cur_chr|@>;
  @<Let |n| be the largest legal code value, based on |cur_chr|@>;
  p:=cur_chr;
  if p=kcat_code_base then
    begin scan_char_num; p:=p+kcatcodekey(cur_val);
      if cur_val>=max_latin_val then m:=not_cjk; end
  else if p<math_code_base then
    begin scan_latin_num; p:=p+cur_val; end
  else begin scan_ascii_num; p:=p+cur_val; end;
  scan_optional_equals; scan_int;
  if ((cur_val<m)and(p<del_code_base))or(cur_val>n) then
  begin print_err("Invalid code ("); print_int(cur_val);
@.Invalid code@>
    if p<del_code_base then
      begin print("), should be in the range "); print_int(m); print("..");
      end
    else print("), should be at most ");
    print_int(n);
    if m=0 then
      begin help1("I'm going to use 0 instead of that illegal code value.");@/
      error; cur_val:=0;
      end
    else
      begin help1("I'm going to use 16 instead of that illegal code value.");@/
      error; cur_val:=16;
      end;
  end;
  if p<math_code_base then define(p,data,cur_val)
  else if p<del_code_base then define(p,data,hi(cur_val))
  else word_define(p,cur_val);
  end;
@y
@<Assignments@>=
def_code: begin
  if cur_chr=(del_code_base+128) then begin
    p:=cur_chr-128; scan_ascii_num; p:=p+cur_val; scan_optional_equals;
    scan_int; cur_val1:=cur_val; scan_int; {backwards}
    if (cur_val1>@"FFFFFF) or (cur_val>@"FFFFFF) then
      begin print_err("Invalid code ("); print_int(cur_val1); print(" ");
      print_int(cur_val);
      print("), should be at most ""FFFFFF ""FFFFFF");
      help1("I'm going to use 0 instead of that illegal code value.");@/
      error; cur_val1:=0; cur_val:=0;
      end;
    cur_val1:=(cur_val1 div @"10000)*@"100+(cur_val1 mod @"100);
    cur_val:=(cur_val div @"10000)*@"100+(cur_val mod @"100);
    del_word_define(p,cur_val1,cur_val);
    end
  else begin
    @<Let |m| be the minimal legal code value, based on |cur_chr|@>;
    @<Let |n| be the largest legal code value, based on |cur_chr|@>;
    p:=cur_chr; cur_val1:=p;
    if p=kcat_code_base then begin scan_char_num; p:=p+kcatcodekey(cur_val);
      if cur_val>=max_latin_val then m:=not_cjk; end
    else if p<math_code_base then begin scan_latin_num; p:=p+cur_val; end
    else begin scan_ascii_num; p:=p+cur_val; end;
    scan_optional_equals; scan_int;
    if ((cur_val<m)and(p<del_code_base))or(cur_val>n) then
    begin print_err("Invalid code ("); print_int(cur_val);
@.Invalid code@>
      if p<del_code_base then
        begin print("), should be in the range "); print_int(m); print("..");
        end
      else print("), should be at most ");
      print_int(n);
      if m=0 then
        begin help1("I'm going to use 0 instead of that illegal code value.");@/
        error; cur_val:=0;
        end
      else
        begin help1("I'm going to use 16 instead of that illegal code value.");@/
        error; cur_val:=16;
        end;
    end;
    if p<math_code_base then define(p,data,cur_val)
    else if cur_val1=math_code_base then begin
      if cur_val=@"8000 then cur_val:=@"80000
      else cur_val:=((cur_val div @"1000)*@"10000)+(cur_val mod @"1000);
      define(p,data,hi(cur_val));
      end
    else if cur_val1=math_code_base+128 then begin
      cur_val:=((cur_val div @"10000) * @"100) + (cur_val mod @"100);
      define(p-128,data,hi(cur_val));
      end
    else if cur_val1=del_code_base then begin
      if cur_val>=0 then begin
        cur_val1:=cur_val div @"1000;
        cur_val1:=(cur_val1 div @"1000)*@"10000 + cur_val1 mod @"1000;
        cur_val:=cur_val mod @"1000;
        del_word_define(p,cur_val1,cur_val); end
      else
        del_word_define(p, -1, cur_val);
      end
    else define(p,data,cur_val);
    end;
  end;
@z
%-----------------------------------------------
@x
else if cur_chr=kcat_code_base then n:=max_char_code
else if cur_chr<math_code_base then n:=max_latin_val
else if cur_chr=math_code_base then n:=@'100000
@y
else if cur_chr=kcat_code_base then n:=max_char_code
else if cur_chr<math_code_base then n:=max_latin_val
else if cur_chr=math_code_base then n:=@"8000
else if cur_chr=(math_code_base+128) then n:=@"8000000
@z
%-----------------------------------------------
@x
def_family: begin p:=cur_chr; scan_four_bit_int; p:=p+cur_val;
@y
def_family: begin p:=cur_chr; scan_big_four_bit_int; p:=p+cur_val;
@z
%-----------------------------------------------
@x
@ @<Dump regions 5 and 6 of |eqtb|@>=
repeat j:=k;
while j<eqtb_size do
  begin if eqtb[j].int=eqtb[j+1].int then goto found2;
  incr(j);
  end;
l:=eqtb_size+1; goto done2; {|j=eqtb_size|}
found2: incr(j); l:=j;
while j<eqtb_size do
  begin if eqtb[j].int<>eqtb[j+1].int then goto done2;
@y
@ @<Dump regions 5 and 6 of |eqtb|@>=
repeat j:=k;
while j<eqtb_size do
  begin if (eqtb[j].int=eqtb[j+1].int) and@|
    (getintone(eqtb[j])=getintone(eqtb[j+1])) then goto found2;
  incr(j);
  end;
l:=eqtb_size+1; goto done2; {|j=eqtb_size|}
found2: incr(j); l:=j;
while j<eqtb_size do
  begin if (eqtb[j].int<>eqtb[j+1].int)or@|
          (getintone(eqtb[j])<>getintone(eqtb[j+1])) then goto done2;
@z
%-----------------------------------------------
@x
  else cur_val:=shrink_order(q);
@y
  else cur_val:=shrink_order(q);
  if cur_val>normal then cur_val:=cur_val-1; {compatible to \eTeX}
@z
%-----------------------------------------------
@x FAM256 65536 Registers
@ \eTeX\ (in extended mode) supports 32768 (i.e., $2^{15}$) count,
@y
@ \epTeX\ (in extended mode) supports 65536 (i.e., $2^{16}$) count,
@z
%-----------------------------------------------
@x
Similarly there are 32768 mark classes; the command \.{\\marks}|n|
creates a mark node for a given mark class |0<=n<=32767| (where
@y
Similarly there are 65536 mark classes; the command \.{\\marks}|n|
creates a mark node for a given mark class |0<=n<=65535| (where
@z
%-----------------------------------------------
@x
not exceed 255 in compatibility mode resp.\ 32767 in extended mode.
@y
not exceed 255 in compatibility mode resp.\ 65535 in extended mode.
@z
%-----------------------------------------------
@x
max_reg_num:=32767;
max_reg_help_line:="A register number must be between 0 and 32767.";
@y
max_reg_num:=65535;
max_reg_help_line:="A register number must be between 0 and 65535.";
@z
%-----------------------------------------------
@x
sparse array of the up to 32512 additional registers of each kind and
one for the sparse array of the up to 32767 additional mark classes.
The root of each such tree, if it exists, is an index node containing 16
pointers to subtrees for 4096 consecutive array elements.  Similar index
nodes are the starting points for all nonempty subtrees for 4096, 256,
and 16 consecutive array elements.  These four levels of index nodes are
followed by a fifth level with nodes for the individual array elements.
@y
sparse array of the up to 65280 additional registers of each kind and
one for the sparse array of the up to 65535 additional mark classes.
The root of each such tree, if it exists, is an index node containing 16
pointers to subtrees for 65536 consecutive array elements.  Similar index
nodes are the starting points for all nonempty subtrees for 65536, 4096, 
256, and 16 consecutive array elements.  These five levels of index nodes
are followed by a sixth level with nodes for the individual array elements.
@z
%-----------------------------------------------
@x
@d hex_dig1(#)==# div 4096 {the fourth lowest hexadecimal digit}
@d hex_dig2(#)==(# div 256) mod 16 {the third lowest hexadecimal digit}
@d hex_dig3(#)==(# div 16) mod 16 {the second lowest hexadecimal digit}
@d hex_dig4(#)==# mod 16 {the lowest hexadecimal digit}
@y
@d hex_dig1(#)==# div 65536 {the fifth lowest hexadecimal digit}
@d hex_dig2(#)==(# div 4096) mod 16 {the fourth lowest hexadecimal digit}
@d hex_dig3(#)==(# div 256) mod 16 {the third lowest hexadecimal digit}
@d hex_dig4(#)==(# div 16) mod 16 {the second lowest hexadecimal digit}
@d hex_dig5(#)==# mod 16 {the lowest hexadecimal digit}
@z
%-----------------------------------------------
@x
procedure find_sa_element(@!t:small_number;@!n:halfword;@!w:boolean);
  {sets |cur_val| to sparse array element location or |null|}
label not_found,not_found1,not_found2,not_found3,not_found4,exit;
var q:pointer; {for list manipulations}
@!i:small_number; {a four bit index}
begin cur_ptr:=sa_root[t];
if_cur_ptr_is_null_then_return_or_goto(not_found);@/
q:=cur_ptr; i:=hex_dig1(n); get_sa_ptr;
if_cur_ptr_is_null_then_return_or_goto(not_found1);@/
q:=cur_ptr; i:=hex_dig2(n); get_sa_ptr;
if_cur_ptr_is_null_then_return_or_goto(not_found2);@/
q:=cur_ptr; i:=hex_dig3(n); get_sa_ptr;
if_cur_ptr_is_null_then_return_or_goto(not_found3);@/
q:=cur_ptr; i:=hex_dig4(n); get_sa_ptr;
if (cur_ptr=null)and w then goto not_found4;
return;
not_found: new_index(t,null); {create first level index node}
sa_root[t]:=cur_ptr; q:=cur_ptr; i:=hex_dig1(n);
not_found1: new_index(i,q); {create second level index node}
add_sa_ptr; q:=cur_ptr; i:=hex_dig2(n);
not_found2: new_index(i,q); {create third level index node}
add_sa_ptr; q:=cur_ptr; i:=hex_dig3(n);
not_found3: new_index(i,q); {create fourth level index node}
add_sa_ptr; q:=cur_ptr; i:=hex_dig4(n);
not_found4: @<Create a new array element of type |t| with index |i|@>;
link(cur_ptr):=q; add_sa_ptr;
exit:end;
@y
procedure find_sa_element(@!t:small_number;@!n:halfword;@!w:boolean);
  {sets |cur_val| to sparse array element location or |null|}
label not_found,not_found1,not_found2,not_found3,not_found4,not_found5,exit;
var q:pointer; {for list manipulations}
@!i:small_number; {a four bit index}
begin cur_ptr:=sa_root[t];
if_cur_ptr_is_null_then_return_or_goto(not_found);@/
q:=cur_ptr; i:=hex_dig1(n); get_sa_ptr;
if_cur_ptr_is_null_then_return_or_goto(not_found1);@/
q:=cur_ptr; i:=hex_dig2(n); get_sa_ptr;
if_cur_ptr_is_null_then_return_or_goto(not_found2);@/
q:=cur_ptr; i:=hex_dig3(n); get_sa_ptr;
if_cur_ptr_is_null_then_return_or_goto(not_found3);@/
q:=cur_ptr; i:=hex_dig4(n); get_sa_ptr;
if_cur_ptr_is_null_then_return_or_goto(not_found4);@/
q:=cur_ptr; i:=hex_dig5(n); get_sa_ptr;
if (cur_ptr=null)and w then goto not_found5;
return;
not_found: new_index(t,null); {create first level index node}
sa_root[t]:=cur_ptr; q:=cur_ptr; i:=hex_dig1(n);
not_found1: new_index(i,q); {create second level index node}
add_sa_ptr; q:=cur_ptr; i:=hex_dig2(n);
not_found2: new_index(i,q); {create third level index node}
add_sa_ptr; q:=cur_ptr; i:=hex_dig3(n);
not_found3: new_index(i,q); {create fourth level index node}
add_sa_ptr; q:=cur_ptr; i:=hex_dig4(n);
not_found4: new_index(i,q); {create fifth level index node}
add_sa_ptr; q:=cur_ptr; i:=hex_dig5(n);
not_found5: @<Create a new array element of type |t| with index |i|@>;
link(cur_ptr):=q; add_sa_ptr;
exit:end;
@z
%-----------------------------------------------
@x
repeat i:=hex_dig4(sa_index(q)); p:=q; q:=link(p); free_node(p,s);
@y
repeat i:=hex_dig5(sa_index(q)); p:=q; q:=link(p); free_node(p,s);
@z
%-----------------------------------------------
@x
else  begin n:=hex_dig4(sa_index(q)); q:=link(q); n:=n+16*sa_index(q);
  q:=link(q); n:=n+256*(sa_index(q)+16*sa_index(link(q)));
@y
else  begin n:=hex_dig5(sa_index(q)); q:=link(q); n:=n+16*sa_index(q);
  q:=link(q); n:=n+256*(sa_index(q)+16*sa_index(link(q)));
  q:=link(link(q)); n:=n+65536*sa_index(q);
@z
%-----------------------------------------------
@x
begin if l<4 then {|q| is an index node}
@y
begin if l<5 then {|q| is an index node}
@z
%-----------------------------------------------
@x
@!fill_width:array[0..2] of scaled; {infinite stretch components of
@y
@!fill_width:array[0..3] of scaled; {infinite stretch components of
@z
%-----------------------------------------------
@x
    if (background[3]=0)and(background[4]=0)and(background[5]=0) then
    begin do_last_line_fit:=true;
    active_node_size:=active_node_size_extended;
    fill_width[0]:=0; fill_width[1]:=0; fill_width[2]:=0;
@y
    if (background[3]=0)and(background[4]=0)and@|
       (background[5]=0)and(background[6]=0) then
    begin do_last_line_fit:=true;
    active_node_size:=active_node_size_extended;
    fill_width[0]:=0; fill_width[1]:=0; fill_width[2]:=0; fill_width[3]:=0;
@z
%-----------------------------------------------
@x
if (cur_active_width[3]<>fill_width[0])or@|
  (cur_active_width[4]<>fill_width[1])or@|
  (cur_active_width[5]<>fill_width[2]) then goto not_found;
  {infinite stretch of this line not entirely due to |par_fill_skip|}
if active_short(r)>0 then g:=cur_active_width[2]
else g:=cur_active_width[6];
@y
if (cur_active_width[3]<>fill_width[0])or@|
  (cur_active_width[4]<>fill_width[1])or@|
  (cur_active_width[5]<>fill_width[2])or@|
  (cur_active_width[6]<>fill_width[3]) then goto not_found;
  {infinite stretch of this line not entirely due to |par_fill_skip|}
if active_short(r)>0 then g:=cur_active_width[2]
else g:=cur_active_width[7];
@z
%---------------------------------------
@x
begin if -g>cur_active_width[6] then g:=-cur_active_width[6];
b:=badness(-g,cur_active_width[6]);
@y
begin if -g>cur_active_width[7] then g:=-cur_active_width[7];
b:=badness(-g,cur_active_width[7]);
@z
%-----------------------------------------------
@x
if shortfall>0 then g:=cur_active_width[2]
else if shortfall<0 then g:=cur_active_width[6]
@y
if shortfall>0 then g:=cur_active_width[2]
else if shortfall<0 then g:=cur_active_width[7]
@z
