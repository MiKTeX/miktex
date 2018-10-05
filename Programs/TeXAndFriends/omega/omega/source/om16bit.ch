% om16bit.ch: Basic changes to TeX data structures allowing all
%             8-bit entities to become 32-bit entities, in particular
%             characters and glyphs.
%
% This file is part of the Omega project, which
% is based on the web2c distribution of TeX.
% 
% Copyright (c) 1994--2000 John Plaice and Yannis Haralambous
% 
% This library is free software; you can redistribute it and/or
% modify it under the terms of the GNU Library General Public
% License as published by the Free Software Foundation; either
% version 2 of the License, or (at your option) any later version.
% 
% This library is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
% Library General Public License for more details.
% 
% You should have received a copy of the GNU Library General Public
% License along with this library; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
%
@x limbo l.1 - Omega
% This program is copyright (C) 1982 by D. E. Knuth; all rights are reserved.
@y
% This program is copyright
% (C) 1994--2000 by John Plaice and Yannis Haralambous.
% It is designed to be a change file for D. E. Knuth's TeX version 3.14159.
%
% This program is copyright (C) 1982 by D. E. Knuth; all rights are reserved.
@z
%---------------------------------------
@x limbo l.80
\def\title{\TeX82}
@y
\def\title{$\Omega$ (OMEGA)}
@z
%---------------------------------------
@x [1] m.1 l.90 - Omega
This is \TeX, a document compiler intended to produce typesetting of high
quality.
The \PASCAL\ program that follows is the definition of \TeX82, a standard
@:PASCAL}{\PASCAL@>
@!@:TeX82}{\TeX82@>
@y
This is $\Omega$, a document compiler intended to simplify high-quality
typesetting for many of the world's languages.  It is an extension
of D. E. Knuth's \TeX, which was designed essentially for the
typesetting of languages using the Latin alphabet.

The $\Omega$ system loosens many of the restrictions imposed by~\TeX:
register numbers are no longer limited to 8~bits;  fonts may have more 
than 256~characters;  more than 256~fonts may be used;  etc.  In addition,
$\Omega$ allows much more complex ligature mechanisms between characters,
thereby simplifying the typesetting of alphabets that have preserved 
their calligraphic traditions.

The \PASCAL\ program that follows is a modification of the definition of \TeX82,
a standard @:PASCAL}{\PASCAL@> @!@:TeX82}{\TeX82@>
@z
%---------------------------------------
@x [1] m.1 l.97 - Omega
will be obtainable on a great variety of computers.

@y
will be obtainable on a great variety of computers.

As little as possible is changed in this document.  This means that
unless the contrary is explicitly stated, references to \TeX\ in the 
documentation are equally applicable to~$\Omega$.  References to `the
author' in the documentation are to D. E. Knuth.  

@z
%---------------------------------------
@x [1] m.2 l.187 - Omega
@d banner=='This is TeX, Version 3.14159' {printed when \TeX\ starts}
@y
@d banner=='This is Omega, Version 3.14159--1.15' {printed when \TeX\ starts}
@z
%---------------------------------------
@x [1] m.4 l.243
program TEX; {all file names are defined dynamically}
@y
program OMEGA; {all file names are defined dynamically}
@z
%---------------------------------------
@x [1] m.11 l.392 - Omega
@!font_max=75; {maximum internal font number; must not exceed |max_quarterword|
  and must be at most |font_base+256|}
@!font_mem_size=20000; {number of words of |font_info| for all fonts}
@y
@!font_max=65535; {maximum internal font number; must be at most |font_biggest|}
@z
%---------------------------------------
@x [1] m.11 l.412 - Omega
@!pool_name='TeXformats:TEX.POOL                     ';
@y
@!pool_name='TeXformats:OMEGA.POOL                   ';
@z
%---------------------------------------
@x [1] m.12 l.437 - Omega
@d hash_size=2100 {maximum number of control sequences; it should be at most
  about |(mem_max-mem_min)/10|}
@d hash_prime=1777 {a prime number equal to about 85\pct! of |hash_size|}
@d hyph_size=307 {another prime; the number of \.{\\hyphenation} exceptions}
@y
@d hash_size=65536 {maximum number of control sequences; it should be at most
  about |(mem_max-mem_min)/10|}
@d hash_prime=55711 {a prime number equal to about 85\pct! of |hash_size|}
@d hyph_size=307 {another prime; the number of \.{\\hyphenation} exceptions}
@d biggest_char=65535 {the largest allowed character number;
   must be |<=max_quarterword|}
@d too_big_char=65536 {|biggest_char+1|}
@d special_char=65537 {|biggest_char+2|}
@d number_chars=65536 {|biggest_char+1|}
@d biggest_reg=65535 {the largest allowed register number;
   must be |<=max_quarterword|}
@d number_regs=65536 {|biggest_reg+1|}
@d font_biggest=65535 {the real biggest font}
@d number_fonts=font_biggest-font_base+2
@d number_math_fonts=768
@d math_font_biggest=767
@d text_size=0 {size code for the largest size in a family}
@d script_size=256 {size code for the medium size in a family}
@d script_script_size=512 {size code for the smallest size in a family}
@d biggest_lang=255
@d too_big_lang=256
@z
%---------------------------------------
@x [8] m.110 l.2355 - Omega
In order to make efficient use of storage space, \TeX\ bases its major data
structures on a |memory_word|, which contains either a (signed) integer,
possibly scaled, or a (signed) |glue_ratio|, or a small number of
fields that are one half or one quarter of the size used for storing
integers.

@y
In order to make efficient use of storage space, \TeX\ bases its major data
structures on a |memory_word|, which contains either a (signed) integer,
possibly scaled, or a (signed) |glue_ratio|, or a small number of
fields that are one half or one quarter of the size used for storing
integers.

That is the description for \TeX.  For $\Omega$, we are going to keep
the same terminology, except that |quarterword| is going to refer to
16~bits, and |halfword| is going to refer to 32~bits.  So, in fact,
a |memory_word| will take 64 bits, and, on a 64-bit machine, will run
smaller than \TeX\ will!

@z
%---------------------------------------
@x [8] m.110 l.2378 - Omega
Since we are assuming 32-bit integers, a halfword must contain at least
16 bits, and a quarterword must contain at least 8 bits.
@^system dependencies@>
But it doesn't hurt to have more bits; for example, with enough 36-bit
words you might be able to have |mem_max| as large as 262142, which is
eight times as much memory as anybody had during the first four years of
\TeX's existence.

N.B.: Valuable memory space will be dreadfully wasted unless \TeX\ is compiled
by a \PASCAL\ that packs all of the |memory_word| variants into
the space of a single integer. This means, for example, that |glue_ratio|
words should be |short_real| instead of |real| on some computers. Some
\PASCAL\ compilers will pack an integer whose subrange is `|0..255|' into
an eight-bit field, but others insist on allocating space for an additional
sign bit; on such systems you can get 256 values into a quarterword only
if the subrange is `|-128..127|'.

@y
@z
%---------------------------------------
@x [8] m.110 l.2406 - Omega
@d max_quarterword=255 {largest allowable value in a |quarterword|}
@d min_halfword==0 {smallest allowable value in a |halfword|}
@d max_halfword==65535 {largest allowable value in a |halfword|}
@y
@d max_quarterword=@"FFFF {largest allowable value in a |quarterword|}
@d min_halfword=0 {smallest allowable value in a |halfword|}
@d max_halfword=@"3FFFFFFF {largest allowable value in a |halfword|}
@z
%---------------------------------------
@x [8] m.111 l.2416 - Omega
if (min_quarterword>0)or(max_quarterword<127) then bad:=11;
if (min_halfword>0)or(max_halfword<32767) then bad:=12;
@y
if (min_quarterword>0)or(max_quarterword<@"7FFF) then bad:=11;
if (min_halfword>0)or(max_halfword<@"3FFFFFFF) then bad:=12;
@z
%---------------------------------------
@x [8] m.111 l.2423 - Omega
if font_max>font_base+256 then bad:=16;
@y
if font_max>font_base+@"10000 then bad:=16;
@z
%---------------------------------------
@x [8] m.111 l.2426 - Omega
if max_quarterword-min_quarterword<255 then bad:=19;
@y
if max_quarterword-min_quarterword<@"FFFF then bad:=19;
@z
%---------------------------------------
@x [10] m.134 l.2832 - Omega
Note that the format of a |char_node| allows for up to 256 different
fonts and up to 256 characters per font; but most implementations will
probably limit the total number of fonts to fewer than 75 per job,
and most fonts will stick to characters whose codes are
less than 128 (since higher codes
are more difficult to access on most keyboards).

Extensions of \TeX\ intended for oriental languages will need even more
than $256\times256$ possible characters, when we consider different sizes
@^oriental characters@>@^Chinese characters@>@^Japanese characters@>
and styles of type.  It is suggested that Chinese and Japanese fonts be
handled by representing such characters in two consecutive |char_node|
entries: The first of these has |font=font_base|, and its |link| points
to the second;
the second identifies the font and the character dimensions.
The saving feature about oriental characters is that most of them have
the same box dimensions. The |character| field of the first |char_node|
is a ``\\{charext}'' that distinguishes between graphic symbols whose
dimensions are identical for typesetting purposes. (See the \MF\ manual.)
Such an extension of \TeX\ would not be difficult; further details are
left to the reader.
@y
Note that the format of a |char_node| allows for up to 65536 different
fonts and up to 65536 characters per font. 
@z
%---------------------------------------
@x [15] m.208 l.4127 - Omega
@d min_internal=68 {the smallest code that can follow \.{\\the}}
@d char_given=68 {character code defined by \.{\\chardef}}
@d math_given=69 {math code defined by \.{\\mathchardef}}
@d last_item=70 {most recent item ( \.{\\lastpenalty},
  \.{\\lastkern}, \.{\\lastskip} )}
@d max_non_prefixed_command=70 {largest command code that can't be \.{\\global}}
@y
@d min_internal=end_cs_name+1 {the smallest code that can follow \.{\\the}}
@d char_given=min_internal {character code defined by \.{\\chardef}}
@d math_given=char_given+1 {math code defined by \.{\\mathchardef}}
@d omath_given=math_given+1 {math code defined by \.{\\omathchardef}}
@d last_item=omath_given+1 {most recent item ( \.{\\lastpenalty},
  \.{\\lastkern}, \.{\\lastskip} )}
@d max_non_prefixed_command=last_item
   {largest command code that can't be \.{\\global}}
@z
%---------------------------------------
@x [15] m.209 l.4138 - Omega
@d toks_register=71 {token list register ( \.{\\toks} )}
@d assign_toks=72 {special token list ( \.{\\output}, \.{\\everypar}, etc.~)}
@d assign_int=73 {user-defined integer ( \.{\\tolerance}, \.{\\day}, etc.~)}
@d assign_dimen=74 {user-defined length ( \.{\\hsize}, etc.~)}
@d assign_glue=75 {user-defined glue ( \.{\\baselineskip}, etc.~)}
@d assign_mu_glue=76 {user-defined muglue ( \.{\\thinmuskip}, etc.~)}
@d assign_font_dimen=77 {user-defined font dimension ( \.{\\fontdimen} )}
@d assign_font_int=78 {user-defined font integer ( \.{\\hyphenchar},
  \.{\\skewchar} )}
@d set_aux=79 {specify state info ( \.{\\spacefactor}, \.{\\prevdepth} )}
@d set_prev_graf=80 {specify state info ( \.{\\prevgraf} )}
@d set_page_dimen=81 {specify state info ( \.{\\pagegoal}, etc.~)}
@d set_page_int=82 {specify state info ( \.{\\deadcycles},
  \.{\\insertpenalties} )}
@d set_box_dimen=83 {change dimension of box ( \.{\\wd}, \.{\\ht}, \.{\\dp} )}
@d set_shape=84 {specify fancy paragraph shape ( \.{\\parshape} )}
@d def_code=85 {define a character code ( \.{\\catcode}, etc.~)}
@d def_family=86 {declare math fonts ( \.{\\textfont}, etc.~)}
@d set_font=87 {set current font ( font identifiers )}
@d def_font=88 {define a font file ( \.{\\font} )}
@d register=89 {internal register ( \.{\\count}, \.{\\dimen}, etc.~)}
@d max_internal=89 {the largest code that can follow \.{\\the}}
@d advance=90 {advance a register or parameter ( \.{\\advance} )}
@d multiply=91 {multiply a register or parameter ( \.{\\multiply} )}
@d divide=92 {divide a register or parameter ( \.{\\divide} )}
@d prefix=93 {qualify a definition ( \.{\\global}, \.{\\long}, \.{\\outer} )}
@d let=94 {assign a command code ( \.{\\let}, \.{\\futurelet} )}
@d shorthand_def=95 {code definition ( \.{\\chardef}, \.{\\countdef}, etc.~)}
@d read_to_cs=96 {read into a control sequence ( \.{\\read} )}
@d def=97 {macro definition ( \.{\\def}, \.{\\gdef}, \.{\\xdef}, \.{\\edef} )}
@d set_box=98 {set a box ( \.{\\setbox} )}
@d hyph_data=99 {hyphenation data ( \.{\\hyphenation}, \.{\\patterns} )}
@d set_interaction=100 {define level of interaction ( \.{\\batchmode}, etc.~)}
@d max_command=100 {the largest command code seen at |big_switch|}
@y
@d toks_register=max_non_prefixed_command+1
   {token list register ( \.{\\toks} )}
@d assign_toks=toks_register+1
   {special token list ( \.{\\output}, \.{\\everypar}, etc.~)}
@d assign_int=assign_toks+1
   {user-defined integer ( \.{\\tolerance}, \.{\\day}, etc.~)}
@d assign_dimen=assign_int+1
   {user-defined length ( \.{\\hsize}, etc.~)}
@d assign_glue=assign_dimen+1
   {user-defined glue ( \.{\\baselineskip}, etc.~)}
@d assign_mu_glue=assign_glue+1
   {user-defined muglue ( \.{\\thinmuskip}, etc.~)}
@d assign_font_dimen=assign_mu_glue+1
   {user-defined font dimension ( \.{\\fontdimen} )}
@d assign_font_int=assign_font_dimen+1
   {user-defined font integer ( \.{\\hyphenchar}, \.{\\skewchar} )}
@d set_aux=assign_font_int+1
   {specify state info ( \.{\\spacefactor}, \.{\\prevdepth} )}
@d set_prev_graf=set_aux+1
   {specify state info ( \.{\\prevgraf} )}
@d set_page_dimen=set_prev_graf+1
   {specify state info ( \.{\\pagegoal}, etc.~)}
@d set_page_int=set_page_dimen+1
   {specify state info ( \.{\\deadcycles},
  \.{\\insertpenalties} )}
@d set_box_dimen=set_page_int+1
   {change dimension of box ( \.{\\wd}, \.{\\ht}, \.{\\dp} )}
@d set_shape=set_box_dimen+1
   {specify fancy paragraph shape ( \.{\\parshape} )}
@d def_code=set_shape+1
   {define a character code ( \.{\\catcode}, etc.~)}
@d def_family=def_code+1
   {declare math fonts ( \.{\\textfont}, etc.~)}
@d set_font=def_family+1
   {set current font ( font identifiers )}
@d def_font=set_font+1
   {define a font file ( \.{\\font} )}
@d register=def_font+1
   {internal register ( \.{\\count}, \.{\\dimen}, etc.~)}
@d max_internal=register
   {the largest code that can follow \.{\\the}}
@d advance=max_internal+1
   {advance a register or parameter ( \.{\\advance} )}
@d multiply=advance+1
   {multiply a register or parameter ( \.{\\multiply} )}
@d divide=multiply+1
   {divide a register or parameter ( \.{\\divide} )}
@d prefix=divide+1
   {qualify a definition ( \.{\\global}, \.{\\long}, \.{\\outer} )}
@d let=prefix+1
   {assign a command code ( \.{\\let}, \.{\\futurelet} )}
@d shorthand_def=let+1
   {code definition ( \.{\\chardef}, \.{\\countdef}, etc.~)}
@d read_to_cs=shorthand_def+1
   {read into a control sequence ( \.{\\read} )}
@d def=read_to_cs+1
   {macro definition ( \.{\\def}, \.{\\gdef}, \.{\\xdef}, \.{\\edef} )}
@d set_box=def+1
   {set a box ( \.{\\setbox} )}
@d hyph_data=set_box+1
   {hyphenation data ( \.{\\hyphenation}, \.{\\patterns} )}
@d set_interaction=hyph_data+1
   {define level of interaction ( \.{\\batchmode}, etc.~)}
@d max_command=set_interaction
   {the largest command code seen at |big_switch|}
@z
%---------------------------------------
@x [17] m.220 l.4486 - Omega
@d eq_level(#)==eq_level_field(eqtb[#]) {level of definition}
@d eq_type(#)==eq_type_field(eqtb[#]) {command code for equivalent}
@d equiv(#)==equiv_field(eqtb[#]) {equivalent value}
@y
@d eq_level(#)==new_eq_level(#) {level of definition}
@d eq_type(#)==new_eq_type(#) {command code for equivalent}
@d equiv(#)==new_equiv(#) {equivalent value}
@d equiv1(#)==new_equiv1(#) {equivalent value}
@z
%---------------------------------------
@x [17] m.222 l.4496 - Omega
In the first region we have 256 equivalents for ``active characters'' that
act as control sequences, followed by 256 equivalents for single-character
control sequences.
@y
In the first region we have |number_chars| equivalents for ``active characters''
that act as control sequences, followed by |number_chars| equivalents for 
single-character control sequences.
@z
%---------------------------------------
@x [17] m.222 l.4507 - Omega
@d single_base=active_base+256 {equivalents of one-character control sequences}
@d null_cs=single_base+256 {equivalent of \.{\\csname\\endcsname}}
@y
@d single_base=active_base+number_chars 
   {equivalents of one-character control sequences}
@d null_cs=single_base+number_chars {equivalent of \.{\\csname\\endcsname}}
@z
%---------------------------------------
@x [17] m.222 l.4530 - Omega
eq_type(undefined_control_sequence):=undefined_cs;
equiv(undefined_control_sequence):=null;
eq_level(undefined_control_sequence):=level_zero;
for k:=active_base to undefined_control_sequence-1 do
  eqtb[k]:=eqtb[undefined_control_sequence];
@y
set_eq_type(undefined_control_sequence,undefined_cs);
set_equiv(undefined_control_sequence,null);
set_eq_level(undefined_control_sequence,level_zero);
@z
%---------------------------------------
@x [17] m.224 l.4547 - Omega
@ Region 3 of |eqtb| contains the 256 \.{\\skip} registers, as well as the
glue parameters defined here. It is important that the ``muskip''
parameters have larger numbers than the others.
@y
@ Region 3 of |eqtb| contains the |number_regs| \.{\\skip} registers, 
as well as the glue parameters defined here. It is important that the 
``muskip'' parameters have larger numbers than the others.
@z
%---------------------------------------
@x [17] m.224 l.4572 - Omega
@d skip_base=glue_base+glue_pars {table of 256 ``skip'' registers}
@d mu_skip_base=skip_base+256 {table of 256 ``muskip'' registers}
@d local_base=mu_skip_base+256 {beginning of region 4}
@y
@d skip_base=glue_base+glue_pars {table of |number_regs| ``skip'' registers}
@d mu_skip_base=skip_base+number_regs 
   {table of |number_regs| ``muskip'' registers}
@d local_base=mu_skip_base+number_regs {beginning of region 4}
@z
%---------------------------------------
@x [17] m.228 l.4685 - Omega
equiv(glue_base):=zero_glue; eq_level(glue_base):=level_one;
eq_type(glue_base):=glue_ref;
for k:=glue_base+1 to local_base-1 do eqtb[k]:=eqtb[glue_base];
@y
set_equiv(glue_base,zero_glue); set_eq_level(glue_base,level_one);
set_eq_type(glue_base,glue_ref);
@z
%---------------------------------------
@x [17] m.230 l.4721 - Omega
@d toks_base=local_base+10 {table of 256 token list registers}
@d box_base=toks_base+256 {table of 256 box registers}
@d cur_font_loc=box_base+256 {internal font number outside math mode}
@d math_font_base=cur_font_loc+1 {table of 48 math font numbers}
@d cat_code_base=math_font_base+48
  {table of 256 command codes (the ``catcodes'')}
@d lc_code_base=cat_code_base+256 {table of 256 lowercase mappings}
@d uc_code_base=lc_code_base+256 {table of 256 uppercase mappings}
@d sf_code_base=uc_code_base+256 {table of 256 spacefactor mappings}
@d math_code_base=sf_code_base+256 {table of 256 math mode mappings}
@d int_base=math_code_base+256 {beginning of region 5}
@y
@d toks_base=local_base+10 {table of |number_regs| token list registers}
@d box_base=toks_base+number_regs {table of |number_regs| box registers}
@d cur_font_loc=box_base+number_regs {internal font number outside math mode}
@d math_font_base=cur_font_loc+1
   {table of |number_math_fonts| math font numbers}
@d cat_code_base=math_font_base+number_math_fonts
  {table of |number_chars| command codes (the ``catcodes'')}
@d lc_code_base=cat_code_base+number_chars 
  {table of |number_chars| lowercase mappings}
@d uc_code_base=lc_code_base+number_chars 
  {table of |number_chars| uppercase mappings}
@d sf_code_base=uc_code_base+number_chars 
  {table of |number_chars| spacefactor mappings}
@d math_code_base=sf_code_base+number_chars 
  {table of |number_chars| math mode mappings}
@d int_base=math_code_base+number_chars {beginning of region 5}
@z
%---------------------------------------
@x [17] m.232 l.4802 - Omega
@d var_code==@'70000 {math code meaning ``use the current family''}

@<Initialize table entries...@>=
par_shape_ptr:=null; eq_type(par_shape_loc):=shape_ref;
eq_level(par_shape_loc):=level_one;@/
for k:=output_routine_loc to toks_base+255 do
  eqtb[k]:=eqtb[undefined_control_sequence];
box(0):=null; eq_type(box_base):=box_ref; eq_level(box_base):=level_one;
for k:=box_base+1 to box_base+255 do eqtb[k]:=eqtb[box_base];
cur_font:=null_font; eq_type(cur_font_loc):=data;
eq_level(cur_font_loc):=level_one;@/
for k:=math_font_base to math_font_base+47 do eqtb[k]:=eqtb[cur_font_loc];
equiv(cat_code_base):=0; eq_type(cat_code_base):=data;
eq_level(cat_code_base):=level_one;@/
for k:=cat_code_base+1 to int_base-1 do eqtb[k]:=eqtb[cat_code_base];
for k:=0 to 255 do
  begin cat_code(k):=other_char; math_code(k):=hi(k); sf_code(k):=1000;
  end;
cat_code(carriage_return):=car_ret; cat_code(" "):=spacer;
cat_code("\"):=escape; cat_code("%"):=comment;
cat_code(invalid_code):=invalid_char; cat_code(null_code):=ignore;
for k:="0" to "9" do math_code(k):=hi(k+var_code);
for k:="A" to "Z" do
  begin cat_code(k):=letter; cat_code(k+"a"-"A"):=letter;@/
  math_code(k):=hi(k+var_code+@"100);
  math_code(k+"a"-"A"):=hi(k+"a"-"A"+var_code+@"100);@/
  lc_code(k):=k+"a"-"A"; lc_code(k+"a"-"A"):=k+"a"-"A";@/
  uc_code(k):=k; uc_code(k+"a"-"A"):=k;@/
  sf_code(k):=999;
  end;
@y
@d var_code==@"7000000 {math code meaning ``use the current family''}

@<Initialize table entries...@>=
set_equiv(par_shape_loc,null); set_eq_type(par_shape_loc,shape_ref);
set_eq_level(par_shape_loc,level_one);@/
set_equiv(cat_code_base+carriage_return,car_ret);
set_equiv(cat_code_base+" ",spacer);
set_equiv(cat_code_base+"\",escape);
set_equiv(cat_code_base+"%",comment);
set_equiv(cat_code_base+invalid_code,invalid_char);
set_equiv(cat_code_base+null_code,ignore);
for k:="0" to "9" do set_equiv(math_code_base+k,hi(k+var_code));
for k:="A" to "Z" do
  begin set_equiv(cat_code_base+k,letter);
  set_equiv(cat_code_base+k+"a"-"A",letter);@/
  set_equiv(math_code_base+k,hi(k+var_code+@"10000));
  set_equiv(math_code_base+k+"a"-"A",hi(k+"a"-"A"+var_code+@"10000));@/
  set_equiv(lc_code_base+k,k+"a"-"A");
  set_equiv(lc_code_base+k+"a"-"A",k+"a"-"A");@/
  set_equiv(uc_code_base+k,k);
  set_equiv(uc_code_base+k+"a"-"A",k);@/
  set_equiv(sf_code_base+k,999);
  end;
@z
%---------------------------------------
@x [17] m.234 l.4857 - Omega
@ @<Show the font identifier in |eqtb[n]|@>=
begin if n=cur_font_loc then print("current font")
else if n<math_font_base+16 then
  begin print_esc("textfont"); print_int(n-math_font_base);
  end
else if n<math_font_base+32 then
  begin print_esc("scriptfont"); print_int(n-math_font_base-16);
  end
else  begin print_esc("scriptscriptfont"); print_int(n-math_font_base-32);
  end;
print_char("=");@/
print_esc(hash[font_id_base+equiv(n)].rh);
  {that's |font_id_text(equiv(n))|}
@y
@ @<Show the font identifier in |eqtb[n]|@>=
begin if n=cur_font_loc then print("current font")
else if n<math_font_base+script_size then
  begin print_esc("textfont"); print_int(n-math_font_base);
  end
else if n<math_font_base+script_script_size then
  begin print_esc("scriptfont"); print_int(n-math_font_base-script_size);
  end
else  begin print_esc("scriptscriptfont");
  print_int(n-math_font_base-script_script_size);
  end;
print_char("=");@/
print_esc(newtext(font_id_base+equiv(n)));
  {that's |font_id_text(equiv(n))|}
@z
%---------------------------------------
@x [17] m.236 l.4955 - Omega
@d count_base=int_base+int_pars {256 user \.{\\count} registers}
@d del_code_base=count_base+256 {256 delimiter code mappings}
@d dimen_base=del_code_base+256 {beginning of region 6}
@#
@d del_code(#)==eqtb[del_code_base+#].int
@d count(#)==eqtb[count_base+#].int
@d int_par(#)==eqtb[int_base+#].int {an integer parameter}
@y
@d count_base=int_base+int_pars {|number_regs| user \.{\\count} registers}
@d del_code_base=count_base+number_regs {|number_chars| delimiter code mappings}
@d dimen_base=del_code_base+number_chars {beginning of region 6}
@#
@d del_code0(#)==new_equiv(del_code_base+#)
@d del_code1(#)==new_equiv1(del_code_base+#)
@d count(#)==new_eqtb_int(count_base+#)
@d int_par(#)==new_eqtb_int(int_base+#) {an integer parameter}
@z
%---------------------------------------
@x [17] m.240 l.5213 - Omega
for k:=int_base to del_code_base-1 do eqtb[k].int:=0;
mag:=1000; tolerance:=10000; hang_after:=1; max_dead_cycles:=25;
escape_char:="\"; end_line_char:=carriage_return;
for k:=0 to 255 do del_code(k):=-1;
del_code("."):=0; {this null delimiter is used in error recovery}
@y
set_new_eqtb_int(int_base+mag_code,1000);
set_new_eqtb_int(int_base+tolerance_code,10000);
set_new_eqtb_int(int_base+hang_after_code,1);
set_new_eqtb_int(int_base+max_dead_cycles_code,25);
set_new_eqtb_int(int_base+escape_char_code,"\");
set_new_eqtb_int(int_base+end_line_char_code,carriage_return);
set_equiv(del_code_base+".",0);
set_equiv1(del_code_base+".",0);
    {this null delimiter is used in error recovery}

@ @<Set newline character to -1@>=
set_new_eqtb_int(int_base+new_line_char_code,-1)

@ @<Set newline character to nl@>=
set_new_eqtb_int(int_base+new_line_char_code,nl)

@z
%---------------------------------------
@x [17] m.242 l.5240 - Omega
print_char("="); print_int(eqtb[n].int);
@y
print_char("="); print_int(new_eqtb_int(n));
@z
%---------------------------------------
@x [17] m.247 l.5273 - Omega
here, and the 256 \.{\\dimen} registers.
@y
here, and the |number_regs| \.{\\dimen} registers.
@z
%---------------------------------------
@x [17] m.247 l.5298 - Omega
  {table of 256 user-defined \.{\\dimen} registers}
@d eqtb_size=scaled_base+255 {largest subscript of |eqtb|}
@#
@d dimen(#)==eqtb[scaled_base+#].sc
@d dimen_par(#)==eqtb[dimen_base+#].sc {a scaled quantity}
@y
  {table of |number_regs| user-defined \.{\\dimen} registers}
@d eqtb_size=scaled_base+biggest_reg {largest subscript of |eqtb|}
@#
@d dimen(#)==new_eqtb_sc(scaled_base+(#))
@d dimen_par(#)==new_eqtb_sc(dimen_base+(#)) {a scaled quantity}
@z
%---------------------------------------
@x [17] m.250 l.5405 - Omega
for k:=dimen_base to eqtb_size do eqtb[k].sc:=0;
@y

@ @p function init_eqtb_entry (p:pointer):memory_word;
var mw:memory_word;
begin
   { Regions 1 and 2 }
   if (p>=active_base) and
      (p<=undefined_control_sequence) then begin
     equiv_field(mw):=null;
     eq_type_field(mw):=undefined_cs;
     eq_level_field(mw):=level_zero;
     end
   { Region 3 }
   else if (p>=glue_base) and
           (p<=local_base+1) then begin
     equiv_field(mw):=zero_glue;
     eq_type_field(mw):=glue_ref;
     eq_level_field(mw):=level_one;
     end
   { Region 4 }
   else if (p>=par_shape_loc) and
           (p<=toks_base+biggest_reg) then begin
     equiv_field(mw):=null;
     eq_type_field(mw):=undefined_cs;
     eq_level_field(mw):=level_zero;
     end
   else if (p>=box_base) and
           (p<=box_base+biggest_reg) then begin
     equiv_field(mw):=null;
     eq_type_field(mw):=box_ref;
     eq_level_field(mw):=level_one;
     end
   else if (p>=cur_font_loc) and
           (p<=math_font_base+math_font_biggest) then begin
     equiv_field(mw):=null_font;
     eq_type_field(mw):=data;
     eq_level_field(mw):=level_one;
     end
   else if (p>=cat_code_base) and
           (p<=cat_code_base+biggest_char) then begin
     equiv_field(mw):=other_char;
     eq_type_field(mw):=data;
     eq_level_field(mw):=level_one;
     end
   else if (p>=lc_code_base) and
           (p<=uc_code_base+biggest_char) then begin
     equiv_field(mw):=0;
     eq_type_field(mw):=data;
     eq_level_field(mw):=level_one;
     end
   else if (p>=sf_code_base) and
           (p<=sf_code_base+biggest_char) then begin
     equiv_field(mw):=1000;
     eq_type_field(mw):=data;
     eq_level_field(mw):=level_one;
     end
   else if (p>=math_code_base) and
           (p<=math_code_base+biggest_char) then begin
     equiv_field(mw):=hi(p-math_code_base);
     eq_type_field(mw):=data;
     eq_level_field(mw):=level_one;
     end
   { Region 5 }
   else if (p>=int_base) and (p<=del_code_base-1) then begin
     setintzero(mw,0);
     setintone(mw,0);
     end
   else if (p>=del_code_base) and (p<=del_code_base+biggest_char) then begin
     setintzero(mw,-1);
     setintone(mw,-1);
     end
   { Region 6 }
   else if (p>=dimen_base) and (p<=eqtb_size) then begin
     setintzero(mw,0);
     setintone(mw,0);
     end
   else begin
     equiv_field(mw):=null;
     eq_type_field(mw):=undefined_cs;
     eq_level_field(mw):=level_zero;
     end;
   init_eqtb_entry:=mw;
end;
 
@z
%---------------------------------------
@x [17] m.251 l.5411 - Omega
print_char("="); print_scaled(eqtb[n].sc); print("pt");
@y
print_char("="); print_scaled(new_eqtb_sc(n)); print("pt");
@z
%---------------------------------------
@x [17] m.254 l.5435 - Omega
@ The last two regions of |eqtb| have fullword values instead of the
three fields |eq_level|, |eq_type|, and |equiv|. An |eq_type| is unnecessary,
but \TeX\ needs to store the |eq_level| information in another array
called |xeq_level|.

@<Glob...@>=
@!eqtb:array[active_base..eqtb_size] of memory_word;
@!xeq_level:array[int_base..eqtb_size] of quarterword;
@y
@ The last two regions of |eqtb| have fullword values instead of the
three fields |eq_level|, |eq_type|, and |equiv|. An |eq_type| is unnecessary,
but \TeX\ needs to store the |eq_level| information in another array
called |xeq_level|.

@d xeq_level(#) == new_xeq_level(eqtb_size+eqtb_size+#)
@d set_xeq_level(#) == set_new_eqtb_int(eqtb_size+eqtb_size+#)
@z
%---------------------------------------
@x [17] m.255 l.5439 - Omega
for k:=int_base to eqtb_size do xeq_level[k]:=level_one;
@y
@z
%---------------------------------------
@x [17] m.255 l.5446 - Omega
for q:=active_base to box_base+255 do
@y
for q:=active_base to box_base+biggest_reg do
@z
%---------------------------------------
@x [18] m.256 l.5477 - Omega
@d next(#) == hash[#].lh {link for coalesced lists}
@d text(#) == hash[#].rh {string number for control sequence name}
@d hash_is_full == (hash_used=hash_base) {test if all positions are occupied}
@d font_id_text(#) == text(font_id_base+#) {a frozen font identifier's name}

@<Glob...@>=
@!hash: array[hash_base..undefined_control_sequence-1] of two_halves;
  {the hash table}
@y
@d hash_is_full == (hash_used=hash_base) {test if all positions are occupied}
@d font_id_text(#) == newtext(font_id_base+#) {a frozen font identifier's name}
@d newtext(#) == new_hash_text(eqtb_size+#)
@d newnext(#) == new_hash_next(eqtb_size+#)
@d settext(#) == set_hash_text(eqtb_size+#)
@d setnext(#) == set_hash_next(eqtb_size+#)

@<Glob...@>=
@z
%---------------------------------------
@x [18] m.257 l.5491 - Omega
next(hash_base):=0; text(hash_base):=0;
for k:=hash_base+1 to undefined_control_sequence-1 do hash[k]:=hash[hash_base];
@y
@z
%---------------------------------------
@x [18] m.258 l.5497 - Omega
hash_used:=frozen_control_sequence; {nothing is used}
cs_count:=0;
eq_type(frozen_dont_expand):=dont_expand;
text(frozen_dont_expand):="notexpanded:";
@y
hash_used:=frozen_control_sequence; {nothing is used}
cs_count:=0;
set_eq_type(frozen_dont_expand,dont_expand);
settext(frozen_dont_expand,"notexpanded:");
@z
%---------------------------------------
@x [18] m.259 l.5514 - Omega
@!k:pointer; {index in |buffer| array}
begin @<Compute the hash code |h|@>;
p:=h+hash_base; {we start searching here; note that |0<=h<hash_prime|}
loop@+begin if text(p)>0 then if length(text(p))=l then
    if str_eq_buf(text(p),j) then goto found;
  if next(p)=0 then
    begin if no_new_control_sequence then
      p:=undefined_control_sequence
    else @<Insert a new control sequence after |p|, then make
      |p| point to it@>;
    goto found;
    end;
  p:=next(p);
  end;
found: id_lookup:=p;
@y
@!k:pointer; {index in |buffer| array}
@!newstring:integer;
begin @<Compute the hash code |h|@>;
p:=h+hash_base; {we start searching here; note that |0<=h<hash_prime|}
loop@+begin if newtext(p)>0 then if length(newtext(p))=l then
    if str_eq_buf(newtext(p),j) then goto found;
  if newnext(p)=0 then
    begin if no_new_control_sequence then
      p:=undefined_control_sequence
    else @<Insert a new control sequence after |p|, then make
      |p| point to it@>;
    goto found;
    end;
  p:=newnext(p);
  end;
found: id_lookup:=p;
@z
%---------------------------------------
@x [18] m.260 l.5532 - Omega
begin if text(p)>0 then
  begin repeat if hash_is_full then overflow("hash size",hash_size);
@:TeX capacity exceeded hash size}{\quad hash size@>
  decr(hash_used);
  until text(hash_used)=0; {search for an empty location in |hash|}
  next(p):=hash_used; p:=hash_used;
  end;
str_room(l); d:=cur_length;
while pool_ptr>str_start[str_ptr] do
  begin decr(pool_ptr); str_pool[pool_ptr+l]:=str_pool[pool_ptr];
  end; {move current string up to make room for another}
for k:=j to j+l-1 do append_char(buffer[k]);
text(p):=make_string; pool_ptr:=pool_ptr+d;
@y
begin if newtext(p)>0 then
  begin repeat if hash_is_full then overflow("hash size",hash_size);
@:TeX capacity exceeded hash size}{\quad hash size@>
  decr(hash_used);
  until newtext(hash_used)=0; {search for an empty location in |hash|}
  setnext(p,hash_used); p:=hash_used;
  end;
str_room(l); d:=cur_length;
while pool_ptr>str_start[str_ptr] do
  begin decr(pool_ptr); str_pool[pool_ptr+l]:=str_pool[pool_ptr];
  end; {move current string up to make room for another}
for k:=j to j+l-1 do append_char(buffer[k]);
newstring:=make_string;
settext(p,newstring); pool_ptr:=pool_ptr+d;
@z
%---------------------------------------
@x [18] m.262 l.5584 - Omega
else if (text(p)<0)or(text(p)>=str_ptr) then print_esc("NONEXISTENT.")
@.NONEXISTENT@>
else  begin print_esc(text(p));
@y
else if (newtext(p)<0)or(newtext(p)>=str_ptr) then print_esc("NONEXISTENT.")
@.NONEXISTENT@>
else  begin print_esc(newtext(p));
@z
%---------------------------------------
@x [18] m.263 l.5601 - Omega
else print_esc(text(p));
@y
else print_esc(newtext(p));
@z
%---------------------------------------
@x [18] m.264 l.5610 - Omega
@p @!init procedure primitive(@!s:str_number;@!c:quarterword;@!o:halfword);
var k:pool_pointer; {index into |str_pool|}
@!j:small_number; {index into |buffer|}
@!l:small_number; {length of the string}
begin if s<256 then cur_val:=s+single_base
else  begin k:=str_start[s]; l:=str_start[s+1]-k;
    {we will move |s| into the (empty) |buffer|}
  for j:=0 to l-1 do buffer[j]:=so(str_pool[k+j]);
  cur_val:=id_lookup(0,l); {|no_new_control_sequence| is |false|}
  flush_string; text(cur_val):=s; {we don't want to have the string twice}
  end;
eq_level(cur_val):=level_one; eq_type(cur_val):=c; equiv(cur_val):=o;
@y
@p @!init procedure primitive(@!s:str_number;@!c:quarterword;@!o:halfword);
var k:pool_pointer; {index into |str_pool|}
@!j:small_number; {index into |buffer|}
@!l:small_number; {length of the string}
begin if s<=biggest_char then cur_val:=s+single_base
else  begin k:=str_start[s]; l:=str_start[s+1]-k;
    {we will move |s| into the (empty) |buffer|}
  for j:=0 to l-1 do buffer[j]:=so(str_pool[k+j]);
  cur_val:=id_lookup(0,l); {|no_new_control_sequence| is |false|}
  flush_string; settext(cur_val,s); {we don't want to have the string twice}
  end;
set_eq_level(cur_val,level_one); set_eq_type(cur_val,c);
set_equiv(cur_val,o);
@z
%---------------------------------------
@x [18] m.265 l.5648 - Omega
primitive("delimiter",delim_num,0);@/
@!@:delimiter_}{\.{\\delimiter} primitive@>
@y
primitive("delimiter",delim_num,0);@/
@!@:delimiter_}{\.{\\delimiter} primitive@>
primitive("odelimiter",delim_num,1);@/
@!@:delimiter_}{\.{\\odelimiter} primitive@>
@z
%---------------------------------------
@x [18] m.265 l.5656 - Omega
text(frozen_end_group):="endgroup"; eqtb[frozen_end_group]:=eqtb[cur_val];@/
@y
settext(frozen_end_group,"endgroup");
set_new_eqtb(frozen_end_group,new_eqtb(cur_val));@/
@z
%---------------------------------------
@x [18] m.265 l.5673 - Omega
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
%---------------------------------------
@x [18] m.265 l.5697 - Omega
primitive("radical",radical,0);@/
@!@:radical_}{\.{\\radical} primitive@>
@y
primitive("radical",radical,0);@/
@!@:radical_}{\.{\\radical} primitive@>
primitive("oradical",radical,1);@/
@!@:radical_}{\.{\\oradical} primitive@>
@z
%---------------------------------------
@x [18] m.265 l.5701 - Omega
primitive("relax",relax,256); {cf.\ |scan_file_name|}
@!@:relax_}{\.{\\relax} primitive@>
text(frozen_relax):="relax"; eqtb[frozen_relax]:=eqtb[cur_val];@/
@y
primitive("relax",relax,too_big_char); {cf.\ |scan_file_name|}
@!@:relax_}{\.{\\relax} primitive@>
settext(frozen_relax,"relax");
set_new_eqtb(frozen_relax,new_eqtb(cur_val));@/
@z
%---------------------------------------
@x [19] m.276 l.5951 - Omega
else  begin save_stack[save_ptr]:=eqtb[p]; incr(save_ptr);
@y
else  begin save_stack[save_ptr]:=new_eqtb(p); incr(save_ptr);
@z
%---------------------------------------
@x [19] m.277 l.5967 - Omega
begin if eq_level(p)=cur_level then eq_destroy(eqtb[p])
else if cur_level>level_one then eq_save(p,eq_level(p));
eq_level(p):=cur_level; eq_type(p):=t; equiv(p):=e;
@y
begin if eq_level(p)=cur_level then eq_destroy(new_eqtb(p))
else if cur_level>level_one then eq_save(p,eq_level(p));
set_eq_level(p,cur_level); set_eq_type(p,t); set_equiv(p,e);
@z
%---------------------------------------
@x [19] m.278 l.5980 - Omega
@p procedure eq_word_define(@!p:pointer;@!w:integer);
begin if xeq_level[p]<>cur_level then
  begin eq_save(p,xeq_level[p]); xeq_level[p]:=cur_level;
  end;
eqtb[p].int:=w;
end;
@y
@p procedure eq_word_define(@!p:pointer;@!w:integer);
begin if xeq_level(p)<>cur_level then
  begin eq_save(p,xeq_level(p)); set_xeq_level(p,cur_level);
  end;
set_new_eqtb_int(p,w);
end;

procedure del_eq_word_define(@!p:pointer;@!w,wone:integer);
begin if xeq_level(p)<>cur_level then
  begin eq_save(p,xeq_level(p)); set_xeq_level(p,cur_level);
  end;
set_equiv(p,w); set_equiv1(p,wone);
end;

@z
%---------------------------------------
@x [19] m.279 l.5990 - Omega
begin eq_destroy(eqtb[p]);
eq_level(p):=level_one; eq_type(p):=t; equiv(p):=e;
end;
@#
procedure geq_word_define(@!p:pointer;@!w:integer); {global |eq_word_define|}
begin eqtb[p].int:=w; xeq_level[p]:=level_one;
end;
@y
begin eq_destroy(new_eqtb(p));
set_eq_level(p,level_one); set_eq_type(p,t); set_equiv(p,e);
end;
@#
procedure geq_word_define(@!p:pointer;@!w:integer); {global |eq_word_define|}
begin set_new_eqtb_int(p,w); set_xeq_level(p,level_one);
end;

procedure del_geq_word_define(@!p:pointer;@!w,wone:integer);
  {global |del_eq_word_define|}
begin set_equiv(p,w); set_equiv1(p,wone); set_xeq_level(p,level_one);
end;
@z
%---------------------------------------
@x [19] m.282 l.6036 - Omega
    else save_stack[save_ptr]:=eqtb[undefined_control_sequence];
@y
    else save_stack[save_ptr]:=new_eqtb(undefined_control_sequence);
@z
%---------------------------------------
@x [19] m.283 l.6056 - Omega
  else  begin eq_destroy(eqtb[p]); {destroy the current value}
    eqtb[p]:=save_stack[save_ptr]; {restore the saved value}
    @!stat if tracing_restores>0 then restore_trace(p,"restoring");@+tats@;@/
    end
else if xeq_level[p]<>level_one then
  begin eqtb[p]:=save_stack[save_ptr]; xeq_level[p]:=l;
@y
  else  begin eq_destroy(new_eqtb(p)); {destroy the current value}
    set_new_eqtb(p,save_stack[save_ptr]); {restore the saved value}
    @!stat if tracing_restores>0 then restore_trace(p,"restoring");@+tats@;@/
    end
else if xeq_level(p)<>level_one then
  begin set_new_eqtb(p,save_stack[save_ptr]); set_xeq_level(p,l);
@z
%---------------------------------------
@x [20] m.289 l.6129 - Omega
number $2^8m+c$; the command code is in the range |1<=m<=14|. (2)~A control
sequence whose |eqtb| address is |p| is represented as the number
|cs_token_flag+p|. Here |cs_token_flag=@t$2^{12}-1$@>| is larger than
@y
number $2^16m+c$; the command code is in the range |1<=m<=14|. (2)~A control
sequence whose |eqtb| address is |p| is represented as the number
|cs_token_flag+p|. Here |cs_token_flag=@t$2^{20}-1$@>| is larger than
@z
%---------------------------------------
@x [20] m.289 l.6142 - Omega
@d cs_token_flag==@'7777 {amount added to the |eqtb| location in a
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
@d cs_token_flag=@"FFFFF {amount added to the |eqtb| location in a
  token that stands for a control sequence; is a multiple of~65536, less~1}
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
%---------------------------------------
@x [20] m.293 l.6256 - Omega
else  begin m:=info(p) div @'400; c:=info(p) mod @'400;
@y
else  begin m:=info(p) div max_char_val; c:=info(p) mod max_char_val;
@z
%---------------------------------------
@x [21] m.298 l.6375 - Omega
procedure print_cmd_chr(@!cmd:quarterword;@!chr_code:halfword);
@y
procedure print_cmd_chr(@!cmd:quarterword;@!chr_code:halfword);
@z
%---------------------------------------
@x [24] m.334 l.7110 - Omega
primitive("par",par_end,256); {cf. |scan_file_name|}
@y
primitive("par",par_end,too_big_char); {cf. |scan_file_name|}
@z
%---------------------------------------
@x [24] m.357 l.7462 - Omega
  else  begin cur_cmd:=t div @'400; cur_chr:=t mod @'400;
@y
  else  begin cur_cmd:=t div max_char_val; cur_chr:=t mod max_char_val;
@z
%---------------------------------------
@x [24] m.358 l.7479 - Omega
@d no_expand_flag=257 {this characterizes a special variant of |relax|}
@y
@d no_expand_flag=special_char {this characterizes a special variant of |relax|}
@z
%---------------------------------------
@x [24] m.365 l.7606 - Omega
if cur_cs=0 then cur_tok:=(cur_cmd*@'400)+cur_chr
@y
if cur_cs=0 then cur_tok:=(cur_cmd*max_char_val)+cur_chr
@z
%---------------------------------------
@x [25] m.374 l.7728 - Omega
  begin eq_define(cur_cs,relax,256); {N.B.: The |save_stack| might change}
@y
  begin eq_define(cur_cs,relax,too_big_char); 
        {N.B.: The |save_stack| might change}
@z
%---------------------------------------
@x [25] m.374 l.7750 - Omega
  buffer[j]:=info(p) mod @'400; incr(j); p:=link(p);
@y
  buffer[j]:=info(p) mod max_char_val; incr(j); p:=link(p);
@z
%---------------------------------------
@x [25] m.380 l.7812 - Omega
done: if cur_cs=0 then cur_tok:=(cur_cmd*@'400)+cur_chr
@y
done: if cur_cs=0 then cur_tok:=(cur_cmd*max_char_val)+cur_chr
@z
%---------------------------------------
@x [25] m.381 l.7824 - Omega
if cur_cs=0 then cur_tok:=(cur_cmd*@'400)+cur_chr
@y
if cur_cs=0 then cur_tok:=(cur_cmd*max_char_val)+cur_chr
@z
%---------------------------------------
@x [25] m.391 l.7985 - Omega
if (info(r)>match_token+255)or(info(r)<match_token) then s:=null
@y
if (info(r)>=end_match_token)or(info(r)<match_token) then s:=null
@z
%---------------------------------------
@x [26] m.407 l.8161 - Omega
@ The |scan_left_brace| routine is called when a left brace is supposed to be
the next non-blank token. (The term ``left brace'' means, more precisely,
a character whose catcode is |left_brace|.) \TeX\ allows \.{\\relax} to
appear before the |left_brace|.

@p procedure scan_left_brace; {reads a mandatory |left_brace|}
begin @<Get the next non-blank non-relax non-call token@>;
if cur_cmd<>left_brace then
  begin print_err("Missing { inserted");
@.Missing \{ inserted@>
  help4("A left brace was mandatory here, so I've put one in.")@/
    ("You might want to delete and/or insert some corrections")@/
    ("so that I will find a matching right brace soon.")@/
    ("(If you're confused by all this, try typing `I}' now.)");
  back_error; cur_tok:=left_brace_token+"{"; cur_cmd:=left_brace;
  cur_chr:="{"; incr(align_state);
  end;
end;
@y
@ The |scan_left_brace| routine is called when a left brace is supposed to be
the next non-blank token. (The term ``left brace'' means, more precisely,
a character whose catcode is |left_brace|.) \TeX\ allows \.{\\relax} to
appear before the |left_brace|.

@p procedure scan_left_brace; {reads a mandatory |left_brace|}
begin @<Get the next non-blank non-relax non-call token@>;
if cur_cmd<>left_brace then
  begin print_err("Missing { inserted");
@.Missing \{ inserted@>
  help4("A left brace was mandatory here, so I've put one in.")@/
    ("You might want to delete and/or insert some corrections")@/
    ("so that I will find a matching right brace soon.")@/
    ("(If you're confused by all this, try typing `I}' now.)");
  back_error; cur_tok:=left_brace_token+"{"; cur_cmd:=left_brace;
  cur_chr:="{"; incr(align_state);
  end;
end;

@ The |scan_right_brace| routine is called when a right brace is supposed to be
the next non-blank token. (The term ``right brace'' means, more precisely,
a character whose catcode is |right_brace|.) \TeX\ allows \.{\\relax} to
appear before the |right_brace|.

@p procedure scan_right_brace; {reads a mandatory |right_brace|}
begin @<Get the next non-blank non-relax non-call token@>;
if cur_cmd<>right_brace then
  begin print_err("Missing { inserted");
@.Missing \{ inserted@>
  help4("A right brace was mandatory here, so I've put one in.")@/
    ("You might want to delete and/or insert some corrections")@/
    ("so that I will find a matching right brace soon.")@/
    ("(If you're confused by all this, try typing `I}' now.)");
  back_error; cur_tok:=right_brace_token+"}"; cur_cmd:=right_brace;
  cur_chr:="}"; incr(align_state);
  end;
end;
@z
%---------------------------------------
@x [26] m.410 l.8293 - Omega
@!cur_val:integer; {value returned by numeric scanners}
@y
@!cur_val:integer; {value returned by numeric scanners}
@!cur_val1:integer; {delcodes are now 51 digits}
@z
%---------------------------------------
@x [26] m.413 l.8335 - Omega
assign_int: scanned_result(eqtb[m].int)(int_val);
assign_dimen: scanned_result(eqtb[m].sc)(dimen_val);
@y
assign_int: scanned_result(new_eqtb_int(m))(int_val);
assign_dimen: scanned_result(new_eqtb_sc(m))(dimen_val);
@z
%---------------------------------------
@x [26] m.413 l.8345 - Omega
char_given,math_given: scanned_result(cur_chr)(int_val);
@y
char_given,math_given,omath_given: scanned_result(cur_chr)(int_val);
@z
%---------------------------------------
@x [26] m.414 l.8356 - Omega
@ @<Fetch a character code from some table@>=
begin scan_char_num;
if m=math_code_base then scanned_result(ho(math_code(cur_val)))(int_val)
else if m<math_code_base then scanned_result(equiv(m+cur_val))(int_val)
else scanned_result(eqtb[m+cur_val].int)(int_val);
@y
@ @<Fetch a character code from some table@>=
begin scan_char_num;
if m=math_code_base then begin
  cur_val1:=ho(math_code(cur_val));
  if ((cur_val1 div @"1000000)>8) or
     (((cur_val1 mod @"1000000) div @"10000)>15) or
     ((cur_val1 mod @"10000)>255) then
    begin print_err("Extended mathchar used as mathchar");
@.Bad mathchar@>
    help2("A mathchar number must be between 0 and ""7FFF.")@/
      ("I changed this one to zero."); int_error(cur_val1); cur_val1:=0;
    end;
  cur_val1:=((cur_val1 div @"1000000)*@"1000) +
            (((cur_val1 mod @"1000000) div @"10000)*@"100) +
            (cur_val1 mod @"10000);
  scanned_result(cur_val1)(int_val)
  end
else if m=(math_code_base+256) then
  scanned_result(ho(math_code(cur_val)))(int_val)
else if m<math_code_base then scanned_result(equiv(m+cur_val))(int_val)
else scanned_result(new_eqtb_int(m+cur_val))(int_val);
@z
%---------------------------------------
@x [26] m.433 l.8593 - Omega
procedure scan_eight_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>255) then
  begin print_err("Bad register code");
@.Bad register code@>
  help2("A register number must be between 0 and 255.")@/
@y
procedure scan_eight_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>biggest_reg) then
  begin print_err("Bad register code");
@.Bad register code@>
  help2("A register number must be between 0 and 65535.")@/
@z
%---------------------------------------
@x [26] m.434 l.8604 - Omega
procedure scan_char_num;
begin scan_int;
if (cur_val<0)or(cur_val>255) then
  begin print_err("Bad character code");
@.Bad character code@>
  help2("A character number must be between 0 and 255.")@/
@y
procedure scan_char_num;
begin scan_int;
if (cur_val<0)or(cur_val>biggest_char) then
  begin print_err("Bad character code");
@.Bad character code@>
  help2("A character number must be between 0 and 65535.")@/
@z
%---------------------------------------
@x [26] m.435 l.8618 - Omega
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

procedure scan_big_four_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>255) then
  begin print_err("Bad number");
@.Bad number@>
  help2("Since I expected to read a number between 0 and 255,")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;
@z
%---------------------------------------
@x [26] m.436 l.8629 - Omega
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
procedure scan_real_fifteen_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>@'77777) then
  begin print_err("Bad mathchar");
@.Bad mathchar@>
  help2("A mathchar number must be between 0 and 32767.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;

procedure scan_fifteen_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>@'77777) then
  begin print_err("Bad mathchar");
@.Bad mathchar@>
  help2("A mathchar number must be between 0 and 32767.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
cur_val := ((cur_val div @"1000) * @"1000000) +
           (((cur_val mod @"1000) div @"100) * @"10000) +
           (cur_val mod @"100);
end;

procedure scan_big_fifteen_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>@"7FFFFFF) then
  begin print_err("Bad extended mathchar");
@.Bad mathchar@>
  help2("An extended mathchar number must be between 0 and ""7FFFFFF.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;
@z
%---------------------------------------
@x [26] m.437 l.8640 - Omega
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
cur_val1 := (((cur_val mod @"1000) div @"100) * @"10000) +
            (cur_val mod @"100);
cur_val := cur_val div @"1000;
cur_val := ((cur_val div @"1000) * @"1000000) +
           (((cur_val mod @"1000) div @"100) * @"10000) +
           (cur_val mod @"100);
end;

procedure scan_fifty_one_bit_int;
var iiii:integer;
begin scan_int;
if (cur_val<0)or(cur_val>@'777777777) then
  begin print_err("Bad delimiter code");
@.Bad delimiter code@>
  help2("A numeric delimiter (first part) must be between 0 and 2^{27}-1.")
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
iiii:=cur_val;
scan_int;
if (cur_val<0)or(cur_val>@"FFFFFF) then
  begin print_err("Bad delimiter code");
@.Bad delimiter code@>
help2("A numeric delimiter (second part) must be between 0 and 2^{24}-1.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
cur_val1:=cur_val;
cur_val:=iiii;
end;

procedure scan_string_argument;
var s:integer;
begin
scan_left_brace;
get_x_token;
while (cur_cmd<>right_brace) do
  begin
  if (cur_cmd=letter) or (cur_cmd=other_char) then begin
    str_room(1); append_char(cur_chr);
    end
  else if (cur_cmd=spacer) then begin
    str_room(1); append_char(" ");
    end
  else begin
    print("Bad token appearing in string argument");
    end;
  get_x_token;
  end;
s:=make_string;
if str_eq_str("mi",s) then s:="mi";
if str_eq_str("mo",s) then s:="mo";
if str_eq_str("mn",s) then s:="mn";
cur_val:=s;
end;
@z
%---------------------------------------
@x [26] m.442 l.8720 - Omega
if cur_val>255 then
  begin print_err("Improper alphabetic constant");
@y
if cur_val>biggest_char then
  begin print_err("Improper alphabetic constant");
@z
%---------------------------------------
@x [27] m.468 l.9200 - Omega
@d job_name_code=5 {command code for \.{\\jobname}}
@y
@d omega_code=5 {command code for \.{\\OmegaVersion}}
@d job_name_code=6 {command code for \.{\\jobname}}
@z
%---------------------------------------
@x [27] m.468 l.9200 - Omega
primitive("jobname",convert,job_name_code);@/
@!@:job_name_}{\.{\\jobname} primitive@>
@y
primitive("OmegaVersion",convert,omega_code);@/
@!@:omega_version_}{\.{\\OmegaVersion} primitive@>
primitive("jobname",convert,job_name_code);@/
@!@:job_name_}{\.{\\jobname} primitive@>
@z
%---------------------------------------
@x [27] m.469 l.9223 - Omega
  othercases print_esc("jobname")
@y
  omega_code: print_esc("OmegaVersion");
  othercases print_esc("jobname")
@z
%---------------------------------------
@x [27] m.471 l.9248 - Omega
job_name_code: if job_name=0 then open_log_file;
@y
omega_code:;
job_name_code: if job_name=0 then open_log_file;
@z
%---------------------------------------
@x [27] m.472 l.9258 - Omega
job_name_code: print(job_name);
@y
omega_code: print("1.15");
job_name_code: print(job_name);
@z
%---------------------------------------
@x [28] m.491 l.9610 - Omega
text(frozen_fi):="fi"; eqtb[frozen_fi]:=eqtb[cur_val];
@y
settext(frozen_fi,"fi"); set_new_eqtb(frozen_fi,new_eqtb(cur_val));
@z
%---------------------------------------
@x [28] m.506 l.9802 - Omega
if (cur_cmd>active_char)or(cur_chr>255) then {not a character}
  begin m:=relax; n:=256;
@y
if (cur_cmd>active_char)or(cur_chr>biggest_char) then {not a character}
  begin m:=relax; n:=too_big_char;
@z
%---------------------------------------
@x [28] m.506 l.9808 - Omega
if (cur_cmd>active_char)or(cur_chr>255) then
  begin cur_cmd:=relax; cur_chr:=256;
@y
if (cur_cmd>active_char)or(cur_chr>biggest_char) then
  begin cur_cmd:=relax; cur_chr:=too_big_char;
@z
%---------------------------------------
@x [29] m.526 l.10178 - Omega
loop@+begin if (cur_cmd>other_char)or(cur_chr>255) then {not a character}
@y
loop@+begin if (cur_cmd>other_char)or(cur_chr>biggest_char) then 
    {not a character}
@z
%---------------------------------------
@x [37] m.780 l.15354 - Omega
@d span_code=256 {distinct from any character}
@d cr_code=257 {distinct from |span_code| and from any character}
@y
@d span_code=special_char {distinct from any character}
@d cr_code=span_code+1 {distinct from |span_code| and from any character}
@z
%---------------------------------------
@x [37] m.780 l.15364 - Omega
text(frozen_cr):="cr"; eqtb[frozen_cr]:=eqtb[cur_val];@/
primitive("crcr",car_ret,cr_cr_code);
@!@:cr_cr_}{\.{\\crcr} primitive@>
text(frozen_end_template):="endtemplate"; text(frozen_endv):="endtemplate";
eq_type(frozen_endv):=endv; equiv(frozen_endv):=null_list;
eq_level(frozen_endv):=level_one;@/
eqtb[frozen_end_template]:=eqtb[frozen_endv];
eq_type(frozen_end_template):=end_template;
@y
settext(frozen_cr,"cr");
set_new_eqtb(frozen_cr,new_eqtb(cur_val));@/
primitive("crcr",car_ret,cr_cr_code);
@!@:cr_cr_}{\.{\\crcr} primitive@>
settext(frozen_end_template,"endtemplate"); settext(frozen_endv,"endtemplate");
set_eq_type(frozen_endv,endv); set_equiv(frozen_endv,null_list);
set_eq_level(frozen_endv,level_one);@/
set_new_eqtb(frozen_end_template,new_eqtb(frozen_endv));
set_eq_type(frozen_end_template,end_template);
@z
%---------------------------------------
@x [37] m.798 l.15651 - Omega
if n>max_quarterword then confusion("256 spans"); {this can happen, but won't}
@^system dependencies@>
@:this can't happen 256 spans}{\quad 256 spans@>
@y
if n>max_quarterword then confusion("too many spans"); 
   {this can happen, but won't}
@^system dependencies@>
@:this can't happen too many spans}{\quad too many spans@>
@z
%---------------------------------------
@x [37] m.804 l.15794 - Omega
  overfull_rule:=0; {prevent rule from being packaged}
  p:=hpack(preamble,saved(1),saved(0)); overfull_rule:=rule_save;
@y
  set_new_eqtb_sc(dimen_base+overfull_rule_code,0);
      {prevent rule from being packaged}
  p:=hpack(preamble,saved(1),saved(0));
  set_new_eqtb_sc(dimen_base+overfull_rule_code,rule_save);
@z
%---------------------------------------
@x [37] m.827 l.16255 - Omega
check_shrinkage(left_skip); check_shrinkage(right_skip);@/
@y
if (shrink_order(left_skip)<>normal)and(shrink(left_skip)<>0) then
  begin set_equiv(glue_base+left_skip_code,finite_shrink(left_skip));
  end;
if (shrink_order(right_skip)<>normal)and(shrink(right_skip)<>0) then
  begin set_equiv(glue_base+right_skip_code,finite_shrink(right_skip));
  end;
@z
%---------------------------------------
@x [40] m.891 l.17434 - Omega
$c_1$ will be treated as nonletters. The |hyphen_char| for that font
must be between 0 and 255, otherwise hyphenation will not be attempted.
@y
$c_1$ will be treated as nonletters. The |hyphen_char| for that font must 
be between 0 and |biggest_char|, otherwise hyphenation will not be attempted.
@z
%---------------------------------------
@x [40] m.892 l.17469 - Omega
@!hc:array[0..65] of 0..256; {word to be hyphenated}
@!hn:small_number; {the number of positions occupied in |hc|}
@!ha,@!hb:pointer; {nodes |ha..hb| should be replaced by the hyphenated result}
@!hf:internal_font_number; {font number of the letters in |hc|}
@!hu:array[0..63] of 0..256; {like |hc|, before conversion to lowercase}
@!hyf_char:integer; {hyphen character of the relevant font}
@!cur_lang,@!init_cur_lang:ASCII_code; {current hyphenation table of interest}
@!l_hyf,@!r_hyf,@!init_l_hyf,@!init_r_hyf:integer; {limits on fragment sizes}
@!hyf_bchar:halfword; {boundary character after $c_n$}
@y
@!hc:array[0..65] of 0..too_big_char; {word to be hyphenated}
@!hn:small_number; {the number of positions occupied in |hc|}
@!ha,@!hb:pointer; {nodes |ha..hb| should be replaced by the hyphenated result}
@!hf:internal_font_number; {font number of the letters in |hc|}
@!hu:array[0..63] of 0..too_big_char; 
     {like |hc|, before conversion to lowercase}
@!hyf_char:integer; {hyphen character of the relevant font}
@!cur_lang,@!init_cur_lang:0..biggest_lang; 
     {current hyphenation table of interest}
@!l_hyf,@!r_hyf,@!init_l_hyf,@!init_r_hyf:integer; {limits on fragment sizes}
@!hyf_bchar:halfword; {boundary character after $c_n$}
@!max_hyph_char:integer;

@ @<Set initial values of key variables@>=
max_hyph_char:=too_big_lang;

@z
%---------------------------------------
@x [40] m.893 l.17483 - Omega
@!c:0..255; {character being considered for hyphenation}
@y
@!c:ASCII_code; {character being considered for hyphenation}
@z
%---------------------------------------
@x [40] m.896 l.17534 - Omega
if hyf_char>255 then goto done1;
@y
if hyf_char>biggest_char then goto done1;
@z
%---------------------------------------
@x [40] m.897 l.17546 - Omega
    if lc_code(c)=0 then goto done3;
@y
    if lc_code(c)=0 then goto done3;
    if lc_code(c)>max_hyph_char then goto done3;
@z
%---------------------------------------
@x [40] m.898 l.17572 - Omega
  if lc_code(c)=0 then goto done3;
@y
  if lc_code(c)=0 then goto done3;
  if lc_code(c)>max_hyph_char then goto done3;
@z
%---------------------------------------
@x [41] m.903 l.17648 - Omega
      begin hu[0]:=256; init_lig:=false;
@y
      begin hu[0]:=max_hyph_char; init_lig:=false;
@z
%---------------------------------------
@x [41] m.903 l.17660 - Omega
found2: s:=ha; j:=0; hu[0]:=256; init_lig:=false; init_list:=null;
@y
found2: s:=ha; j:=0; hu[0]:=max_hyph_char; init_lig:=false; init_list:=null;
@z
%---------------------------------------
@x [41] m.905 l.17707 - Omega
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
%---------------------------------------
@x [41] m.916 l.17980 - Omega
  begin decr(l); c:=hu[l]; c_loc:=l; hu[l]:=256;
@y
  begin decr(l); c:=hu[l]; c_loc:=l; hu[l]:=max_hyph_char;
@z
%---------------------------------------
@x [42] m.921 l.18079 - Omega
@!op_start:array[ASCII_code] of 0..trie_op_size; {offset for current language}
@y
@!op_start:array[0..biggest_lang] of 0..trie_op_size; 
           {offset for current language}
@z
%---------------------------------------
@x [42] m.923 l.18086 - Omega
hyphenation algorithm is quite short. In the following code we set |hc[hn+2]|
to the impossible value 256, in order to guarantee that |hc[hn+3]| will
@y
hyphenation algorithm is quite short. In the following code we set |hc[hn+2]| to
the impossible value |max_hyph_char|, in order to guarantee that |hc[hn+3]| will
@z
%---------------------------------------
@x [42] m.923 l.18095 - Omega
hc[0]:=0; hc[hn+1]:=0; hc[hn+2]:=256; {insert delimiters}
@y
hc[0]:=0; hc[hn+1]:=0; hc[hn+2]:=max_hyph_char; {insert delimiters}
@z
%---------------------------------------
@x [42] m.934 l.18206 - Omega
@d set_cur_lang==if language<=0 then cur_lang:=0
  else if language>255 then cur_lang:=0
@y
@d set_cur_lang==if language<=0 then cur_lang:=0
  else if language>biggest_lang then cur_lang:=0
@z
%---------------------------------------
@x [43] m.943 l.18348 - Omega
@!trie_used:array[ASCII_code] of quarterword;
  {largest opcode used so far for this language}
@!trie_op_lang:array[1..trie_op_size] of ASCII_code;
@y
@!trie_used:array[0..biggest_lang] of quarterword;
  {largest opcode used so far for this language}
@!trie_op_lang:array[1..trie_op_size] of 0..biggest_lang;
@z
%---------------------------------------
@x [43] m.945 l.18400 - Omega
for j:=1 to 255 do op_start[j]:=op_start[j-1]+qo(trie_used[j-1]);
@y
for j:=1 to biggest_lang do op_start[j]:=op_start[j-1]+qo(trie_used[j-1]);
@z
%---------------------------------------
@x [43] m.946 l.18416 - Omega
for k:=0 to 255 do trie_used[k]:=min_quarterword;
@y
for k:=0 to biggest_lang do trie_used[k]:=min_quarterword;
@z
%---------------------------------------
@x [43] m.947 l.18438 - Omega
@!init @!trie_c:packed array[trie_pointer] of packed_ASCII_code;
@y
@!init @!trie_c:packed array[trie_pointer] of ASCII_code;
@z
%---------------------------------------
@x [43] m.952 l.18551 - Omega
for p:=0 to 255 do trie_min[p]:=p+1;
@y
for p:=0 to biggest_char do trie_min[p]:=p+1;
@z
%---------------------------------------
@x [43] m.953 l.18569 - Omega
@!ll:1..256; {upper limit of |trie_min| updating}
@y
@!ll:1..too_big_char; {upper limit of |trie_min| updating}
@z
%---------------------------------------
@x [43] m.953 l.18573 - Omega
  @<Ensure that |trie_max>=h+256|@>;
@y
  @<Ensure that |trie_max>=h+max_hyph_char|@>;
@z
%---------------------------------------
@x [43] m.954 l.18582 - Omega
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
%---------------------------------------
@x [43] m.954 l.18592 - Omega
  until trie_max=h+256;
@y
  until trie_max=h+max_hyph_char;
@z
%---------------------------------------
@x [43] m.956 l.18607 - Omega
if l<256 then
  begin if z<256 then ll:=z @+else ll:=256;
@y
if l<max_hyph_char then
  begin if z<max_hyph_char then ll:=z @+else ll:=max_hyph_char;
@z
%---------------------------------------
@x [43] m.958 l.18637 - Omega
if trie_root=0 then {no patterns were given}
  begin for r:=0 to 256 do trie[r]:=h;
  trie_max:=256;
@y
if trie_root=0 then {no patterns were given}
  begin for r:=0 to max_hyph_char do trie[r]:=h;
  trie_max:=max_hyph_char;
@z
%---------------------------------------
@x [43] m.962 l.18714 - Omega
@ @<Append a new letter or a hyphen level@>=
if digit_sensed or(cur_chr<"0")or(cur_chr>"9") then
  begin if cur_chr="." then cur_chr:=0 {edge-of-word delimiter}
  else  begin cur_chr:=lc_code(cur_chr);
    if cur_chr=0 then
      begin print_err("Nonletter");
@.Nonletter@>
      help1("(See Appendix H.)"); error;
      end;
    end;
@y
@ @<Append a new letter or a hyphen level@>=
if digit_sensed or(cur_chr<"0")or(cur_chr>"9") then
  begin if cur_chr="." then cur_chr:=0 {edge-of-word delimiter}
  else  begin cur_chr:=lc_code(cur_chr);
    if cur_chr=0 then
      begin print_err("Nonletter");
@.Nonletter@>
      help1("(See Appendix H.)"); error;
      end;
    end;
    if cur_chr>max_hyph_char then max_hyph_char:=cur_chr;
@z
%---------------------------------------
@x [43] m.966 l. - Omega
procedure init_trie;
var @!p:trie_pointer; {pointer for initialization}
@!j,@!k,@!t:integer; {all-purpose registers for initialization}
@!r,@!s:trie_pointer; {used to clean up the packed |trie|}
@!h:two_halves; {template used to zero out |trie|'s holes}
begin @<Get ready to compress the trie@>;
@y
procedure init_trie;
var @!p:trie_pointer; {pointer for initialization}
@!j,@!k,@!t:integer; {all-purpose registers for initialization}
@!r,@!s:trie_pointer; {used to clean up the packed |trie|}
@!h:two_halves; {template used to zero out |trie|'s holes}
begin
incr(max_hyph_char);
@<Get ready to compress the trie@>;
@z
%---------------------------------------
@x [44] m.977 l.19008 - Omega
if q=null then box(n):=null {the |eq_level| of the box stays the same}
else box(n):=vpack(q,natural);
@y
if q=null then set_equiv(box_base+n,null)
    {the |eq_level| of the box stays the same}
else set_equiv(box_base+n,vpack(q,natural));
@z
%---------------------------------------
@x [45] m.992 l.24928 - Omega
flush_node_list(box(n)); box(n):=null;
@y
flush_node_list(box(n)); set_equiv(box_base+n,null);
@z
%---------------------------------------
@x [45] m.994 l.19364 - Omega
@!n:min_quarterword..255; {insertion box number}
@y
@!n:min_quarterword..biggest_reg; {insertion box number}
@z
%---------------------------------------
@x [45] m.1012 l.19655 - Omega
@!n:min_quarterword..255; {insertion box number}
@y
@!n:min_quarterword..biggest_reg; {insertion box number}
@z
%---------------------------------------
@x [45] m.1014 l.19710 - Omega
split_top_skip:=save_split_top_skip;
@y
set_equiv(glue_base+split_top_skip_code,save_split_top_skip);
@z
%---------------------------------------
@x [45] m.1017 l.19753 - Omega
save_vbadness:=vbadness; vbadness:=inf_bad;
save_vfuzz:=vfuzz; vfuzz:=max_dimen; {inhibit error messages}
box(255):=vpackage(link(page_head),best_size,exactly,page_max_depth);
vbadness:=save_vbadness; vfuzz:=save_vfuzz;
@y
save_vbadness:=vbadness;
set_new_eqtb_int(int_base+vbadness_code,inf_bad);
save_vfuzz:=vfuzz;
set_new_eqtb_sc(dimen_base+vfuzz_code,max_dimen);
    {inhibit error messages}
set_equiv(box_base+255,
          vpackage(link(page_head),best_size,exactly,page_max_depth));
set_new_eqtb_int(int_base+vbadness_code,save_vbadness);
set_new_eqtb_sc(dimen_base+vfuzz_code,save_vfuzz);
@z
%---------------------------------------
@x [45] m.1018 l.19774 - Omega
    if box(n)=null then box(n):=new_null_box;
@y
    if box(n)=null then set_equiv(box_base+n,new_null_box);
@z
%---------------------------------------
@x [45] m.1021 l.19817 - Omega
    split_top_skip:=split_top_ptr(p);
    ins_ptr(p):=prune_page_top(broken_ptr(r));
    if ins_ptr(p)<>null then
      begin temp_ptr:=vpack(ins_ptr(p),natural);
      height(p):=height(temp_ptr)+depth(temp_ptr);
      free_node(temp_ptr,box_node_size); wait:=true;
      end;
    end; 
best_ins_ptr(r):=null;
n:=qo(subtype(r));
temp_ptr:=list_ptr(box(n));
free_node(box(n),box_node_size);
box(n):=vpack(temp_ptr,natural);
@y
    set_equiv(glue_base+split_top_skip_code,split_top_ptr(p));
    ins_ptr(p):=prune_page_top(broken_ptr(r));
    if ins_ptr(p)<>null then
      begin temp_ptr:=vpack(ins_ptr(p),natural);
      height(p):=height(temp_ptr)+depth(temp_ptr);
      free_node(temp_ptr,box_node_size); wait:=true;
      end;
    end; 
best_ins_ptr(r):=null;
n:=qo(subtype(r));
temp_ptr:=list_ptr(box(n));
free_node(box(n),box_node_size);
set_equiv(box_base+n,vpack(temp_ptr,natural));
@z
%---------------------------------------
@x [45] m.1023 l.19854 - Omega
ship_out(box(255)); box(255):=null;
@y
ship_out(box(255)); set_equiv(box_base+255,null);
@z
%---------------------------------------
@x [46] m.1046 l.20339 - Omega
non_math(math_given), non_math(math_comp), non_math(delim_num),
@y
non_math(math_given), non_math(omath_given),
non_math(math_comp), non_math(delim_num),
@z
%---------------------------------------
@x [47] m.1071 l.20718 - Omega
|box_flag+255| represent `\.{\\setbox0}' through `\.{\\setbox255}';
codes |box_flag+256| through |box_flag+511| represent `\.{\\global\\setbox0}'
through `\.{\\global\\setbox255}';
code |box_flag+512| represents `\.{\\shipout}'; and codes |box_flag+513|
through |box_flag+515| represent `\.{\\leaders}', `\.{\\cleaders}',
@y
|box_flag+biggest_reg| represent `\.{\\setbox0}' through 
`\.{\\setbox}|biggest_reg|'; codes |box_flag+biggest_reg+1| through 
|box_flag+2*biggest_reg| represent `\.{\\global\\setbox0}'
through `\.{\\global\\setbox}|biggest_reg|'; code |box_flag+2*number_regs| 
represents `\.{\\shipout}'; and codes |box_flag+2*number_regs+1|
through |box_flag+2*number_regs+3| represent `\.{\\leaders}', `\.{\\cleaders}',
@z
%---------------------------------------
@x [47] m.1071 l.20732 - Omega
@d ship_out_flag==box_flag+512 {context code for `\.{\\shipout}'}
@d leader_flag==box_flag+513 {context code for `\.{\\leaders}'}
@y
@d ship_out_flag==box_flag+(number_regs+number_regs) 
   {context code for `\.{\\shipout}'}
@d leader_flag==ship_out_flag+1 {context code for `\.{\\leaders}'}
@z
%---------------------------------------
@x [47] m.1077 l.20853 - Omega
if box_context<box_flag+256 then
  eq_define(box_base-box_flag+box_context,box_ref,cur_box)
else geq_define(box_base-box_flag-256+box_context,box_ref,cur_box)
@y
if box_context<box_flag+number_regs then
  eq_define(box_base-box_flag+box_context,box_ref,cur_box)
else geq_define(box_base-box_flag-number_regs+box_context,box_ref,cur_box)
@z
%---------------------------------------
@x [47] m.1079 l.20888 - Omega
  box(cur_val):=null; {the box becomes void, at the same level}
@y
  set_equiv(box_base+cur_val,null);
      {the box becomes void, at the same level}
@z
%---------------------------------------
@x [47] m.1100 l.21156 - Omega
  if saved(0)<255 then
@y
  if saved(0)<>255 then
@z
%---------------------------------------
@x [47] m.1109 l.21265 - Omega
@ The |un_hbox| and |un_vbox| commands unwrap one of the 256 current boxes.
@y
@ The |un_hbox| and |un_vbox| commands unwrap one of the current boxes.
@z
%---------------------------------------
@x [47] m.1110 l.21287 - Omega
else  begin link(tail):=list_ptr(p); box(cur_val):=null;
@y
else  begin link(tail):=list_ptr(p);
set_equiv(box_base+cur_val,null);
@z
%---------------------------------------
@x [47] m.1117 l.21342 - Omega
  if c>=0 then if c<256 then pre_break(tail):=new_character(cur_font,c);
@y
  if c>=0 then if c<=biggest_char then 
     pre_break(tail):=new_character(cur_font,c);
@z
%---------------------------------------
@x [48] m.1151 l.21810 - Omega
@ Recall that the |nucleus|, |subscr|, and |supscr| fields in a noad are
broken down into subfields called |math_type| and either |info| or
|(fam,character)|. The job of |scan_math| is to figure out what to place
in one of these principal fields; it looks at the subformula that
comes next in the input, and places an encoding of that subformula
into a given word of |mem|.

@d fam_in_range==((cur_fam>=0)and(cur_fam<16))

@<Declare act...@>=
procedure scan_math(@!p:pointer);
label restart,reswitch,exit;
var c:integer; {math character code}
begin restart:@<Get the next non-blank non-relax...@>;
reswitch:case cur_cmd of
letter,other_char,char_given: begin c:=ho(math_code(cur_chr));
    if c=@'100000 then
      begin @<Treat |cur_chr| as an active character@>;
      goto restart;
      end;
    end;
char_num: begin scan_char_num; cur_chr:=cur_val; cur_cmd:=char_given;
  goto reswitch;
  end;
math_char_num: begin scan_fifteen_bit_int; c:=cur_val;
  end;
math_given: c:=cur_chr;
delim_num: begin scan_twenty_seven_bit_int; c:=cur_val div @'10000;
  end;
othercases @<Scan a subformula enclosed in braces and |return|@>
endcases;@/
math_type(p):=math_char; character(p):=qi(c mod 256);
if (c>=var_code)and fam_in_range then fam(p):=cur_fam
else fam(p):=(c div 256) mod 16;
exit:end;
@y
@ Recall that the |nucleus|, |subscr|, and |supscr| fields in a noad are
broken down into subfields called |math_type| and either |info| or
|(fam,character)|. The job of |scan_math| is to figure out what to place
in one of these principal fields; it looks at the subformula that
comes next in the input, and places an encoding of that subformula
into a given word of |mem|.

@d fam_in_range==((cur_fam>=0)and(cur_fam<16))

@<Declare act...@>=
@t\4@>@<Declare the function called |fin_mlist|@>@t@>@;@/

procedure scan_math(@!p:pointer);
label restart,reswitch,exit;
var c:integer; {math character code}
begin restart:@<Get the next non-blank non-relax...@>;
reswitch:
case cur_cmd of
letter,other_char,char_given: begin
  c:=ho(math_code(cur_chr));
  if c=@"8000000 then
    begin @<Treat |cur_chr| as an active character@>;
    goto restart;
    end;
  end;
char_num: begin scan_char_num; cur_chr:=cur_val; cur_cmd:=char_given;
  goto reswitch;
  end;
math_char_num: begin
  if cur_chr=0 then scan_fifteen_bit_int
  else scan_big_fifteen_bit_int;
  c:=cur_val;
  end;
math_given: begin
  c := ((cur_chr div @"1000) * @"1000000) +
         (((cur_chr mod @"1000) div @"100) * @"10000) +
         (cur_chr mod @"100);
  end;
omath_given: begin c:=cur_chr;
  end;
delim_num: begin
  if cur_chr=0 then scan_twenty_seven_bit_int
  else scan_fifty_one_bit_int;
  c:=cur_val;
  end;
othercases @<Scan a subformula enclosed in braces and |return|@>
endcases;@/
math_type(p):=math_char; character(p):=qi(c mod @"10000);
if (c>=var_code)and fam_in_range then fam(p):=cur_fam
else fam(p):=(c div @"10000) mod @"100;
exit:end;
@z
%---------------------------------------
@x [48] m.1154 l.21860 - Omega
mmode+letter,mmode+other_char,mmode+char_given:
  set_math_char(ho(math_code(cur_chr)));
mmode+char_num: begin scan_char_num; cur_chr:=cur_val;
  set_math_char(ho(math_code(cur_chr)));
  end;
mmode+math_char_num: begin scan_fifteen_bit_int; set_math_char(cur_val);
  end;
mmode+math_given: set_math_char(cur_chr);
mmode+delim_num: begin scan_twenty_seven_bit_int;
  set_math_char(cur_val div @'10000);
@y
mmode+letter,mmode+other_char,mmode+char_given:
  set_math_char(ho(math_code(cur_chr)));
mmode+char_num: begin scan_char_num; cur_chr:=cur_val;
  set_math_char(ho(math_code(cur_chr)));
  end;
mmode+math_char_num: begin
  if cur_chr=0 then scan_fifteen_bit_int
  else scan_big_fifteen_bit_int;
  set_math_char(cur_val);
  end;
mmode+math_given: begin
  set_math_char(((cur_chr div @"1000) * @"1000000) +
                (((cur_chr mod @"1000) div @"100) * @"10000) +
                (cur_chr mod @"100));
  end;
mmode+omath_given: begin
  set_math_char(cur_chr);
  end;
mmode+delim_num: begin
  if cur_chr=0 then scan_twenty_seven_bit_int
  else scan_fifty_one_bit_int;
  set_math_char(cur_val);
@z
%---------------------------------------
@x [48] m.1155 l.21878 - Omega
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
  end;
end;
@y
procedure set_math_char(@!c:integer);
var p,q,r:pointer; {the new noad}
begin if c>=@"8000000 then
  @<Treat |cur_chr|...@>
else  begin p:=new_noad; math_type(nucleus(p)):=math_char;
  character(nucleus(p)):=qi(c mod @"10000);
  fam(nucleus(p)):=(c div @"10000) mod @"100;
  if c>=var_code then
    begin if fam_in_range then fam(nucleus(p)):=cur_fam;
    type(p):=ord_noad;
    end
  else  type(p):=ord_noad+(c div @"1000000);
  link(tail):=p; tail:=p;
  end;
end;
@z
%---------------------------------------
@x [48] m.1160 l.21966 - Omega
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
@y
procedure scan_delimiter(@!p:pointer;@!r:integer);
begin if r=1 then scan_twenty_seven_bit_int
else if r=2 then scan_fifty_one_bit_int
else  begin @<Get the next non-blank non-relax...@>;
  case cur_cmd of
  letter,other_char: begin
    cur_val:=del_code0(cur_chr); cur_val1:=del_code1(cur_chr);
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
small_fam(p):=(cur_val div @"10000) mod @"100;
small_char(p):=qi(cur_val mod @"10000);
large_fam(p):=(cur_val1 div @"10000) mod @"100;
large_char(p):=qi(cur_val1 mod @"10000);
@z
%---------------------------------------
@x [48] m.1163 l.22005 - Omega
scan_delimiter(left_delimiter(tail),true); scan_math(nucleus(tail));
@y
scan_delimiter(left_delimiter(tail),cur_chr+1); scan_math(nucleus(tail));
@z
%---------------------------------------
@x [48] m.1165 l.22012 - Omega
procedure math_ac;
begin if cur_cmd=accent then
  @<Complain that the user should have said \.{\\mathaccent}@>;
tail_append(get_node(accent_noad_size));
type(tail):=accent_noad; subtype(tail):=normal;
mem[nucleus(tail)].hh:=empty_field;
mem[subscr(tail)].hh:=empty_field;
mem[supscr(tail)].hh:=empty_field;
math_type(accent_chr(tail)):=math_char;
scan_fifteen_bit_int;
character(accent_chr(tail)):=qi(cur_val mod 256);
if (cur_val>=var_code)and fam_in_range then fam(accent_chr(tail)):=cur_fam
else fam(accent_chr(tail)):=(cur_val div 256) mod 16;
scan_math(nucleus(tail));
end;
@y
procedure math_ac;
begin if cur_cmd=accent then
  @<Complain that the user should have said \.{\\mathaccent}@>;
tail_append(get_node(accent_noad_size));
type(tail):=accent_noad; subtype(tail):=normal;
mem[nucleus(tail)].hh:=empty_field;
mem[subscr(tail)].hh:=empty_field;
mem[supscr(tail)].hh:=empty_field;
math_type(accent_chr(tail)):=math_char;
if cur_chr=0 then scan_fifteen_bit_int
else scan_big_fifteen_bit_int;
character(accent_chr(tail)):=qi(cur_val mod @"10000);
if (cur_val>=var_code)and fam_in_range then fam(accent_chr(tail)):=cur_fam
else fam(accent_chr(tail)):=(cur_val div @"10000) mod @"100;
scan_math(nucleus(tail));
end;
@z
%---------------------------------------
@x [48] m.1174 l.22085 - Omega
@t\4@>@<Declare the function called |fin_mlist|@>@t@>@;@/
@y
@z
%---------------------------------------
@x [48] m.1188 l.22284 - Omega
text(frozen_right):="right"; eqtb[frozen_right]:=eqtb[cur_val];
@y
settext(frozen_right,"right");
set_new_eqtb(frozen_right,new_eqtb(cur_val));
@z
%---------------------------------------
@x [49] m.1214 l.22700 - Omega
@d word_define(#)==if global then geq_word_define(#)@+else eq_word_define(#)
@y
@d word_define(#)==if global then geq_word_define(#)@+else eq_word_define(#)
@d del_word_define(#)==if global
                       then del_geq_word_define(#)@+else del_eq_word_define(#)
@z
%---------------------------------------
@x [49] m.1216 l.22734 - Omega
text(frozen_protection):="inaccessible";
@y
settext(frozen_protection,"inaccessible");
@z
%---------------------------------------
@x [49] m.1222 l.22789 - Omega
@d char_def_code=0 {|shorthand_def| for \.{\\chardef}}
@d math_char_def_code=1 {|shorthand_def| for \.{\\mathchardef}}
@d count_def_code=2 {|shorthand_def| for \.{\\countdef}}
@d dimen_def_code=3 {|shorthand_def| for \.{\\dimendef}}
@d skip_def_code=4 {|shorthand_def| for \.{\\skipdef}}
@d mu_skip_def_code=5 {|shorthand_def| for \.{\\muskipdef}}
@d toks_def_code=6 {|shorthand_def| for \.{\\toksdef}}

@<Put each...@>=
primitive("chardef",shorthand_def,char_def_code);@/
@!@:char_def_}{\.{\\chardef} primitive@>
primitive("mathchardef",shorthand_def,math_char_def_code);@/
@!@:math_char_def_}{\.{\\mathchardef} primitive@>
@y
@d char_def_code=0 {|shorthand_def| for \.{\\chardef}}
@d math_char_def_code=1 {|shorthand_def| for \.{\\mathchardef}}
@d omath_char_def_code=2 {|shorthand_def| for \.{\\omathchardef}}
@d count_def_code=3 {|shorthand_def| for \.{\\countdef}}
@d dimen_def_code=4 {|shorthand_def| for \.{\\dimendef}}
@d skip_def_code=5 {|shorthand_def| for \.{\\skipdef}}
@d mu_skip_def_code=6 {|shorthand_def| for \.{\\muskipdef}}
@d toks_def_code=7 {|shorthand_def| for \.{\\toksdef}}

@<Put each...@>=
primitive("chardef",shorthand_def,char_def_code);@/
@!@:char_def_}{\.{\\chardef} primitive@>
primitive("mathchardef",shorthand_def,math_char_def_code);@/
@!@:math_char_def_}{\.{\\mathchardef} primitive@>
primitive("omathchardef",shorthand_def,omath_char_def_code);@/
@!@:math_char_def_}{\.{\\omathchardef} primitive@>
@z
%---------------------------------------
@x [49] m.1224 l.22833 - Omega
shorthand_def: begin n:=cur_chr; get_r_token; p:=cur_cs; define(p,relax,256);
  scan_optional_equals;
  case n of
  char_def_code: begin scan_char_num; define(p,char_given,cur_val);
    end;
  math_char_def_code: begin scan_fifteen_bit_int; define(p,math_given,cur_val);
    end;
@y
shorthand_def: begin n:=cur_chr; get_r_token; p:=cur_cs; 
  define(p,relax,too_big_char); scan_optional_equals;
  case n of
  char_def_code: begin scan_char_num; define(p,char_given,cur_val);
    end;
  math_char_def_code: begin scan_real_fifteen_bit_int;
    define(p,math_given,cur_val);
    end;
  omath_char_def_code: begin scan_big_fifteen_bit_int;
    define(p,omath_given,cur_val);
    end;
@z
%---------------------------------------
@x [49] m.1230 l. - Omega
primitive("mathcode",def_code,math_code_base);
@!@:math_code_}{\.{\\mathcode} primitive@>
primitive("lccode",def_code,lc_code_base);
@!@:lc_code_}{\.{\\lccode} primitive@>
primitive("uccode",def_code,uc_code_base);
@!@:uc_code_}{\.{\\uccode} primitive@>
primitive("sfcode",def_code,sf_code_base);
@!@:sf_code_}{\.{\\sfcode} primitive@>
primitive("delcode",def_code,del_code_base);
@!@:del_code_}{\.{\\delcode} primitive@>
@y
primitive("mathcode",def_code,math_code_base);
@!@:math_code_}{\.{\\mathcode} primitive@>
primitive("omathcode",def_code,math_code_base+256);
@!@:math_code_}{\.{\\omathcode} primitive@>
primitive("lccode",def_code,lc_code_base);
@!@:lc_code_}{\.{\\lccode} primitive@>
primitive("uccode",def_code,uc_code_base);
@!@:uc_code_}{\.{\\uccode} primitive@>
primitive("sfcode",def_code,sf_code_base);
@!@:sf_code_}{\.{\\sfcode} primitive@>
primitive("delcode",def_code,del_code_base);
@!@:del_code_}{\.{\\delcode} primitive@>
primitive("odelcode",def_code,del_code_base+256);
@!@:del_code_}{\.{\\odelcode} primitive@>
@z
%---------------------------------------
@x [49] m.1232 l.22990 - Omega
def_code: begin @<Let |n| be the largest legal code value, based on |cur_chr|@>;
  p:=cur_chr; scan_char_num; p:=p+cur_val; scan_optional_equals;
  scan_int;
  if ((cur_val<0)and(p<del_code_base))or(cur_val>n) then
    begin print_err("Invalid code ("); print_int(cur_val);
@.Invalid code@>
    if p<del_code_base then print("), should be in the range 0..")
    else print("), should be at most ");
    print_int(n);
    help1("I'm going to use 0 instead of that illegal code value.");@/
    error; cur_val:=0;
    end;
  if p<math_code_base then define(p,data,cur_val)
  else if p<del_code_base then define(p,data,hi(cur_val))
  else word_define(p,cur_val);
@y
def_code: begin
 if cur_chr=(del_code_base+256) then begin
   p:=cur_chr-256; scan_char_num; p:=p+cur_val; scan_optional_equals;
   scan_int; cur_val1:=cur_val; scan_int; {backwards}
   if (cur_val1>@"FFFFFF) or (cur_val>@"FFFFFF) then
     begin print_err("Invalid code ("); print_int(cur_val1); print(" ");
     print_int(cur_val);
     print("), should be at most ""FFFFFF ""FFFFFF");
     help1("I'm going to use 0 instead of that illegal code value.");@/
     error; cur_val1:=0; cur_val:=0;
     end;
   del_word_define(p,cur_val1,cur_val);
   end
 else begin
  @<Let |n| be the largest legal code value, based on |cur_chr|@>;
  p:=cur_chr; scan_char_num; p:=p+cur_val; scan_optional_equals;
  scan_int;
  if ((cur_val<0)and(p<del_code_base))or(cur_val>n) then
    begin print_err("Invalid code ("); print_int(cur_val);
@.Invalid code@>
    if p<del_code_base then print("), should be in the range 0..")
    else print("), should be at most ");
    print_int(n);
    help1("I'm going to use 0 instead of that illegal code value.");@/
    error; cur_val:=0;
    end;
  if p<math_code_base then define(p,data,cur_val)
  else if p<(math_code_base+256) then begin
    if cur_val=@"8000 then cur_val:=@"8000000
    else cur_val:=((cur_val div @"1000) * @"1000000) +
                  (((cur_val mod @"1000) div @"100) * @"10000) +
                  (cur_val mod @"100);
    define(p,data,hi(cur_val));
    end
  else if p<del_code_base then define(p-256,data,hi(cur_val))
  else begin
   cur_val1:=cur_val div @"1000;
   cur_val1:=(cur_val1 div @"100)*@"10000 + (cur_val1 mod @"100);
   cur_val:=cur_val mod @"1000;
   cur_val:=(cur_val div @"100)*@"10000 + (cur_val mod @"100);
   del_word_define(p,cur_val1,cur_val);
   end;
  end;
@z
%---------------------------------------
@x [49] m.1233 l.22990 - Omega
if cur_chr=cat_code_base then n:=max_char_code
else if cur_chr=math_code_base then n:=@'100000
else if cur_chr=sf_code_base then n:=@'77777
else if cur_chr=del_code_base then n:=@'77777777
else n:=255
@y
if cur_chr=cat_code_base then n:=max_char_code
else if cur_chr=math_code_base then n:=@'100000
else if cur_chr=(math_code_base+256) then n:=@"8000000
else if cur_chr=sf_code_base then n:=@'77777
else if cur_chr=del_code_base then n:=@'77777777
else n:=biggest_char
@z
%---------------------------------------
@x [49] m.1234 l.22990 - Omega
def_family: begin p:=cur_chr; scan_four_bit_int; p:=p+cur_val;
@y
def_family: begin p:=cur_chr; scan_big_four_bit_int; p:=p+cur_val;
@z
%---------------------------------------
@x [49] m.1238 l.23059 - Omega
  if q=advance then cur_val:=cur_val+eqtb[l].int;
@y
  if q=advance then cur_val:=cur_val+new_eqtb_int(l);
@z
%---------------------------------------
@x [49] m.1240 l.23086 - Omega
    if p=int_val then cur_val:=mult_integers(eqtb[l].int,cur_val)
    else cur_val:=nx_plus_y(eqtb[l].int,cur_val,0)
  else cur_val:=x_over_n(eqtb[l].int,cur_val)
@y
    if p=int_val then cur_val:=mult_integers(new_eqtb_int(l),cur_val)
    else cur_val:=nx_plus_y(new_eqtb_int(l),cur_val,0)
  else cur_val:=x_over_n(new_eqtb_int(l),cur_val)
@z
%---------------------------------------
@x [49] m.1241 l.23109 - Omega
  if global then n:=256+cur_val@+else n:=cur_val;
@y
  if global then n:=number_regs+cur_val@+else n:=cur_val;
@z
%---------------------------------------
@x [49] m.1289 l.23546 - Omega
|cs_token_flag+active_base| is a multiple of~256.
@y
|cs_token_flag+active_base| is a multiple of~|max_char_val|.
@z
%---------------------------------------
@x [49] m.1289 l.23552 - Omega
  begin c:=t mod 256;
@y
  begin c:=t mod max_char_val;
@z
%---------------------------------------
@x [50] m.1313 l.23892 - Omega
@<Dump regions 1 to 4 of |eqtb|@>;
@<Dump regions 5 and 6 of |eqtb|@>;
@y
dump_eqtb_table;
@z
%---------------------------------------
@x [50] m.1314 l.23904 - Omega
@<Undump regions 1 to 6 of |eqtb|@>;
@y
undump_eqtb_table;
@z
%---------------------------------------
@x [50] m.1315 l.23904 - Omega
@ The table of equivalents usually contains repeated information, so we dump it
in compressed form: The sequence of $n+2$ values $(n,x_1,\ldots,x_n,m)$ in the
format file represents $n+m$ consecutive entries of |eqtb|, with |m| extra
copies of $x_n$, namely $(x_1,\ldots,x_n,x_n,\ldots,x_n)$.
 
@<Dump regions 1 to 4 of |eqtb|@>=
k:=active_base;
repeat j:=k;
while j<int_base-1 do
  begin if (equiv(j)=equiv(j+1))and(eq_type(j)=eq_type(j+1))and@|
    (eq_level(j)=eq_level(j+1)) then goto found1;
  incr(j);
  end;
l:=int_base; goto done1; {|j=int_base-1|}
found1: incr(j); l:=j;
while j<int_base-1 do
  begin if (equiv(j)<>equiv(j+1))or(eq_type(j)<>eq_type(j+1))or@|
    (eq_level(j)<>eq_level(j+1)) then goto done1;
  incr(j);
  end;
done1:dump_int(l-k);
while k<l do
  begin dump_wd(eqtb[k]); incr(k);
  end;
k:=j+1; dump_int(k-l);
until k=int_base

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
  incr(j);
  end;
done2:dump_int(l-k);
while k<l do
  begin dump_wd(eqtb[k]); incr(k);
  end;
k:=j+1; dump_int(k-l);
until k>eqtb_size
 
@ @<Undump regions 1 to 6 of |eqtb|@>=
k:=active_base;
repeat undump_int(x);
if (x<1)or(k+x>eqtb_size+1) then goto bad_fmt;
for j:=k to k+x-1 do undump_wd(eqtb[j]);
k:=k+x;
undump_int(x);
if (x<0)or(k+x>eqtb_size+1) then goto bad_fmt;
for j:=k to k+x-1 do eqtb[j]:=eqtb[k-1];
k:=k+x;
until k>eqtb_size
@y

@z
%---------------------------------------
@x [50] m.1318 l.23967 - Omega
@<Dump the hash table@>=
dump_int(hash_used); cs_count:=frozen_control_sequence-1-hash_used;
for p:=hash_base to hash_used do if text(p)<>0 then
  begin dump_int(p); dump_hh(hash[p]); incr(cs_count);
  end;
for p:=hash_used+1 to undefined_control_sequence-1 do dump_hh(hash[p]);
dump_int(cs_count);@/
print_ln; print_int(cs_count); print(" multiletter control sequences")
@y
@<Dump the hash table@>=
dump_int(hash_used);
@z
%---------------------------------------
@x [50] m.1319 l.23976 - Omega
undump(hash_base)(frozen_control_sequence)(hash_used); p:=hash_base-1;
repeat undump(p+1)(hash_used)(p); undump_hh(hash[p]);
until p=hash_used;
for p:=hash_used+1 to undefined_control_sequence-1 do undump_hh(hash[p]);
undump_int(cs_count)
@y
undump(hash_base)(frozen_control_sequence)(hash_used);
@z
%---------------------------------------
@x [50] m.1324 l.24066 - Omega
for k:=0 to trie_max do dump_hh(trie[k]);
@y
for k:=0 to trie_max do dump_hh(trie[k]);
dump_int(max_hyph_char);
@z
%---------------------------------------
@x [50] m.1324 l.24078 - Omega
for k:=255 downto 0 do if trie_used[k]>min_quarterword then
@y
for k:=biggest_lang downto 0 do if trie_used[k]>min_quarterword then
@z
%---------------------------------------
@x [50] m.1325 l.24094 - Omega
for k:=0 to j do undump_hh(trie[k]);
@y
for k:=0 to j do undump_hh(trie[k]);
undump_int(max_hyph_char);
@z
%---------------------------------------
@x [50] m.1325 l.24101 - Omega
init for k:=0 to 255 do trie_used[k]:=min_quarterword;@+tini@;@/
k:=256;
@y
init for k:=0 to biggest_lang do trie_used[k]:=min_quarterword;@+tini@;@/
k:=biggest_lang+1;
@z
%---------------------------------------
@x [50] m.1326 l.24100 - Omega
dump_int(interaction); dump_int(format_ident); dump_int(69069);
tracing_stats:=0
@y
dump_int(interaction); dump_int(format_ident); dump_int(69069);
set_new_eqtb_int(int_base+tracing_stats_code,0)
@z
%---------------------------------------
@x [53] m.1341 l.24506 - Omega
@d what_lang(#)==link(#+1) {language number, in the range |0..255|}
@y
@d what_lang(#)==link(#+1) {language number, in the range |0..biggest_lang|}
@z
%---------------------------------------
@x [53] m.1370
@!write_file:array[0..15] of alpha_file;
@y
@!outside_string_array: array[0..1000] of 0..255;
@!write_file:array[0..15] of alpha_file;
@z
%---------------------------------------
@x [53] m. l.24761 - Omega
text(end_write):="endwrite"; eq_level(end_write):=level_one;
eq_type(end_write):=outer_call; equiv(end_write):=null;
@y
settext(end_write,"endwrite"); set_eq_level(end_write,level_one);
set_eq_type(end_write,outer_call); set_equiv(end_write,null);
@z
%---------------------------------------
@x [53] m.1376 l.24878 - Omega
else if language>255 then l:=0
@y
else if language>biggest_lang then l:=0
@z
%---------------------------------------
@x [53] m.1377 l.24893 - Omega
  else if cur_val>255 then clang:=0
@y
  else if cur_val>biggest_lang then clang:=0
@z
%---------------------------------------
@x [54] 
This section should be replaced, if necessary, by any special
modifications of the program
that are necessary to make \TeX\ work at a particular installation.
It is usually best to design your change file so that all changes to
previous sections preserve the section numbering; then everybody's version
will be consistent with the published program. More extensive changes,
which introduce new sections, can be inserted here; then only the index
itself will get a new section number.
@^system dependencies@>

@y
@* \[54] $\Omega$ changes.

@z
%---------------------------------------
