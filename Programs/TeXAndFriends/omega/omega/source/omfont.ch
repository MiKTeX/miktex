% omfont.ch: Reading and storing font metric information
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
%%---------------------------------------
@x [15] m.208 l.
@d min_internal=end_cs_name+1 {the smallest code that can follow \.{\\the}}
@y
@d char_ghost=end_cs_name+1
   {\.{\\ghostleft}, \.{\\ghostright} character for kerning}
@d min_internal=char_ghost+1 {the smallest code that can follow \.{\\the}}
@z
%---------------------------------------
@x [17] m.222 l.4530 - Omega
  {begins table of 257 permanent font identifiers}
@d undefined_control_sequence=frozen_null_font+257 {dummy location}
@y
  {begins table of |number_fonts| permanent font identifiers}
@d undefined_control_sequence=frozen_null_font+number_fonts
  {dummy location}
@z
%---------------------------------------
@x [18] m.265 l.5648 - Omega
primitive("insert",insert,0);@/
@!@:insert_}{\.{\\insert} primitive@>
@y
primitive("insert",insert,0);@/
@!@:insert_}{\.{\\insert} primitive@>
primitive("leftghost",char_ghost,0);@/
@!@:leftghost_}{\.{\\leftghost} primitive@>
@z
%---------------------------------------
@x [18] m.265 l.5648 - Omega
primitive("setbox",set_box,0);@/
@!@:set_box_}{\.{\\setbox} primitive@>
@y
primitive("rightghost",char_ghost,1);@/
@!@:rightghost_}{\.{\\rightghost} primitive@>
primitive("setbox",set_box,0);@/
@!@:set_box_}{\.{\\setbox} primitive@>
@z
%---------------------------------------
@x [18] m.266 l.5648 - Omega
char_num: print_esc("char");
@y
char_num: print_esc("char");
char_ghost: if cur_chr=0 then print_esc("leftghost")
  else print_esc("rightghost");
@z
%---------------------------------------
@x [26] m.425 l.8524 - font parameters
begin find_font_dimen(false); font_info[fmem_ptr].sc:=0;
scanned_result(font_info[cur_val].sc)(dimen_val);
@y
begin find_font_dimen(false);
font_info(dimen_font)(font_file_size(dimen_font)).sc:=0;
scanned_result(font_info(dimen_font)(cur_val).sc)(dimen_val);
@z
%---------------------------------------
@x [26] m.426 l.8530 - font parameters
if m=0 then scanned_result(hyphen_char[cur_val])(int_val)
else scanned_result(skew_char[cur_val])(int_val);
@y
if m=0 then scanned_result(hyphen_char(cur_val))(int_val)
else scanned_result(skew_char(cur_val))(int_val);
@z
%---------------------------------------
@x [27] m.472 l.9258 - font parameters
font_name_code: begin print(font_name[cur_val]);
  if font_size[cur_val]<>font_dsize[cur_val] then
    begin print(" at "); print_scaled(font_size[cur_val]);
@y
font_name_code: begin print(font_name(cur_val));
  if font_size(cur_val)<>font_dsize(cur_val) then
    begin print(" at "); print_scaled(font_size(cur_val));
@z
%---------------------------------------
@x [30] m.539 l.10392 - Omega
@^Ramshaw, Lyle Harold@>
of information in a compact but useful form.

@y
@^Ramshaw, Lyle Harold@>
of information in a compact but useful form.

$\Omega$ is capable of reading not only \.{TFM} files, but also
\.{OFM} files, which can describe fonts with up to 65536 characters
and with huge lig/kern tables.  These fonts will often be virtual
fonts built up from real fonts with 256 characters, but $\Omega$
is not aware of this.  

The documentation below describes \.{TFM} files, with slight additions
to show where \.{OFM} files differ.

@z
%---------------------------------------
@x [30] m.540 l.10417 - Omega
Note that a font may contain as many as 256 characters (if |bc=0| and |ec=255|),
and as few as 0 characters (if |bc=ec+1|).

Incidentally, when two or more 8-bit bytes are combined to form an integer of
16 or more bits, the most significant bytes appear first in the file.
This is called BigEndian order.
@!@^BigEndian order@>

@y
Note that a \.{TFM} font may contain as many as 256 characters 
(if |bc=0| and |ec=255|), and as few as 0 characters (if |bc=ec+1|).

Incidentally, when two or more 8-bit bytes are combined to form an integer of
16 or more bits, the most significant bytes appear first in the file.
This is called BigEndian order.
@!@^BigEndian order@>

The first 52 bytes (13 words) of an \.{OFM} file contains thirteen
32-bit integers that give the lengths of the various subsequent
portions of the file.  The first word is 0 (future versions of
\.{OFM} files could have different values;  what is important is that
the first two bytes be 0 to differentiate \.{TFM} and \.{OFM} files).
The next twelve integers are as above, all nonegative and less
than~$2^{31}$.  We must have |bc-1<=ec<=65535|, and
$$\hbox{|lf=13+lh+2*(ec-bc+1)+nw+nh+nd+ni+nl+nk+ne+np|.}$$
Note that an \.{OFM} font may contain as many as 65536 characters 
(if |bc=0| and |ec=65535|), and as few as 0 characters (if |bc=ec+1|).

@z
%---------------------------------------
@x [30] m.543 l.10492 - Omega
per character. Each word in this part of the file contains six fields
@y
per character. Each word in this part of a \.{TFM} file contains six fields
@z
%---------------------------------------
@x [30] m.543 l.10506 - Omega
imposes a limit of 16 different heights, 16 different depths, and
64 different italic corrections.

@y
imposes a limit of 16 different heights, 16 different depths, and
64 different italic corrections.

For \.{OFM} files, two words (eight bytes) are used. 
The arrangement is as follows.

\yskip\hang first and second bytes: |@!width_index| (16 bits)\par
\hang third byte: |@!height_index| (8 bits)\par
\hang fourth byte: |@!depth_index| (8~bits)\par
\hang fifth and sixth bytes: 
|@!italic_index| (14 bits) times 4, plus |@!tag| (2~bits)\par
\hang seventh and eighth bytes: |@!remainder| (16 bits)\par
\yskip\noindent
Therefore the \.{OFM} format imposes a limit of 256 different heights,
256 different depths, and 16384 different italic corrections.

@z
%---------------------------------------
@x [30] m.545 l.10547 - Omega
that explains what to do for special letter pairs. Each word in this array is a
|@!lig_kern_command| of four bytes.
@y
that explains what to do for special letter pairs. Each word in this array,
in a \.{TFM} file, is a |@!lig_kern_command| of four bytes.
@z
%---------------------------------------
@x [30] m.545 l.10557 - Omega
\hang fourth byte: |remainder|.\par
\yskip\noindent
@y
\hang fourth byte: |remainder|.\par
\yskip\noindent
In an \.{OFM} file, eight bytes are used, two bytes for each field.

@z
%---------------------------------------
@x [30] m.545 l.10587 - Omega
appear in a location |<=255|.
@y
appear in a location |<=255| in a \.{TFM} file, |<=65535| in an \.{OFM} file.
@z
%---------------------------------------
@x [30] m.545 l.10599 - Omega
@d skip_byte(#)==#.b0
@d next_char(#)==#.b1
@d op_byte(#)==#.b2
@d rem_byte(#)==#.b3
@y
@d top_skip_byte(#)==(#.b0 div 256)
@d skip_byte(#)==(#.b0 mod 256)
@d next_char_end(#)==#.b1
@d next_char(#)==font_offset(#)+next_char_end
@d op_byte(#)==#.b2
@d rem_byte(#)==#.b3
@d rem_char_byte_end(#)==#.b3
@d rem_char_byte(#)==font_offset(#)+rem_char_byte_end
@d rem_top_byte(#)==(#.b3 div 256)
@d rem_bot_byte(#)==(#.b3 mod 256)
@z
%---------------------------------------
@x [30] m.546 l.10604 - Omega
consists of four bytes called |@!top|, |@!mid|, |@!bot|, and |@!rep| (in this
order). These bytes are the character codes of individual pieces used to
@y
consists of four bytes in a \.{TFM} file, 
called |@!top|, |@!mid|, |@!bot|, and |@!rep| (in this order). 
In an \.{OFM} file, each field takes two bytes, for eight in total.
These bytes are the character codes of individual pieces used to
@z
%---------------------------------------
@x [30] m.546 l.10618 - Omega
@d ext_top(#)==#.b0 {|top| piece in a recipe}
@d ext_mid(#)==#.b1 {|mid| piece in a recipe}
@d ext_bot(#)==#.b2 {|bot| piece in a recipe}
@d ext_rep(#)==#.b3 {|rep| piece in a recipe}
@y
@d ext_top_end(#)==#.b0 {|top| piece in a recipe}
@d ext_top(#)==font_offset(#)+ext_top_end
@d ext_mid_end(#)==#.b1 {|mid| piece in a recipe}
@d ext_mid(#)==font_offset(#)+ext_mid_end
@d ext_bot_end(#)==#.b2 {|bot| piece in a recipe}
@d ext_bot(#)==font_offset(#)+ext_bot_end
@d ext_rep_end(#)==#.b3 {|rep| piece in a recipe}
@d ext_rep(#)==font_offset(#)+ext_rep_end
@z
%---------------------------------------
@x [30] m.548 l.10674 - Omega
@!font_index=0..font_mem_size; {index into |font_info|}
@y
@!font_index=integer; {index into |font_info|}
@z
%---------------------------------------
@x [30] m.549 l.10678 - Omega
@d non_char==qi(256) {a |halfword| code that can't match a real character}
@y
@d non_char==qi(too_big_char) {a code that can't match a real character}
@z
%---------------------------------------
@x [30] m.549 l.10686 - Omega
@!font_info:array[font_index] of memory_word;
  {the big collection of font data}
@!fmem_ptr:font_index; {first unused word of |font_info|}
@!font_ptr:internal_font_number; {largest internal font number in use}
@!font_check:array[internal_font_number] of four_quarters; {check sum}
@!font_size:array[internal_font_number] of scaled; {``at'' size}
@!font_dsize:array[internal_font_number] of scaled; {``design'' size}
@!font_params:array[internal_font_number] of font_index; {how many font
  parameters are present}
@!font_name:array[internal_font_number] of str_number; {name of the font}
@!font_area:array[internal_font_number] of str_number; {area of the font}
@!font_bc:array[internal_font_number] of eight_bits;
  {beginning (smallest) character code}
@!font_ec:array[internal_font_number] of eight_bits;
  {ending (largest) character code}
@!font_glue:array[internal_font_number] of pointer;
  {glue specification for interword space, |null| if not allocated}
@!font_used:array[internal_font_number] of boolean;
  {has a character from this font actually appeared in the output?}
@!hyphen_char:array[internal_font_number] of integer;
  {current \.{\\hyphenchar} values}
@!skew_char:array[internal_font_number] of integer;
  {current \.{\\skewchar} values}
@!bchar_label:array[internal_font_number] of font_index;
  {start of |lig_kern| program for left boundary character,
  |non_address| if there is none}
@!font_bchar:array[internal_font_number] of min_quarterword..non_char;
  {right boundary character, |non_char| if there is none}
@!font_false_bchar:array[internal_font_number] of min_quarterword..non_char;
  {|font_bchar| if it doesn't exist in the font, otherwise |non_char|}
@y
@!font_ptr:internal_font_number; {largest internal font number in use}
@z
%---------------------------------------
@x [30] m.550 l.10723 - Omega
|font_info[char_base[f]+c].qqqq|; and if |w| is the |width_index|
part of this word (the |b0| field), the width of the character is
|font_info[width_base[f]+w].sc|. (These formulas assume that
|min_quarterword| has already been added to |c| and to |w|, since \TeX\
stores its quarterwords that way.)

@<Glob...@>=
@!char_base:array[internal_font_number] of integer;
  {base addresses for |char_info|}
@!width_base:array[internal_font_number] of integer;
  {base addresses for widths}
@!height_base:array[internal_font_number] of integer;
  {base addresses for heights}
@!depth_base:array[internal_font_number] of integer;
  {base addresses for depths}
@!italic_base:array[internal_font_number] of integer;
  {base addresses for italic corrections}
@!lig_kern_base:array[internal_font_number] of integer;
  {base addresses for ligature/kerning programs}
@!kern_base:array[internal_font_number] of integer;
  {base addresses for kerns}
@!exten_base:array[internal_font_number] of integer;
  {base addresses for extensible recipes}
@!param_base:array[internal_font_number] of integer;
  {base addresses for font parameters}
@y
|font_info(f)(char_base(f)+c).qqqq|; and if |w| is the |width_index|
part of this word (the |b0| field), the width of the character is
|font_info(f)(width_base(f)+w).sc|. (These formulas assume that
|min_quarterword| has already been added to |c| and to |w|, since \TeX\
stores its quarterwords that way.)

@d font_info_end(#)==#]
@d font_info(#)==font_tables[#,font_info_end
@d offset_file_size=0
@d offset_check=1
@d offset_offset=offset_check+4
@d offset_size=offset_offset+1
@d offset_dsize=offset_size+1
@d offset_params=offset_dsize+1
@d offset_name_sort=offset_params+1
@d offset_name=offset_name_sort+1
@d offset_area=offset_name+1
@d offset_bc=offset_area+1
@d offset_ec=offset_bc+1
@d offset_glue=offset_ec+1
@d offset_used=offset_glue+1
@d offset_hyphen=offset_used+1
@d offset_skew=offset_hyphen+1
@d offset_bchar_label=offset_skew+1
@d offset_bchar=offset_bchar_label+1
@d offset_false_bchar=offset_bchar+1
@d offset_ivalues_start=offset_false_bchar+1
@d offset_fvalues_start=offset_ivalues_start+1
@d offset_mvalues_start=offset_fvalues_start+1
@d offset_rules_start=offset_mvalues_start+1
@d offset_glues_start=offset_rules_start+1
@d offset_penalties_start=offset_glues_start+1
@d offset_ivalues_base=offset_penalties_start+1
@d offset_fvalues_base=offset_ivalues_base+1
@d offset_mvalues_base=offset_fvalues_base+1
@d offset_rules_base=offset_mvalues_base+1
@d offset_glues_base=offset_rules_base+1
@d offset_penalties_base=offset_glues_base+1
@d offset_char_base=offset_penalties_base+1
@d offset_char_attr_base=offset_char_base+1
@d offset_width_base=offset_char_attr_base+1
@d offset_height_base=offset_width_base+1
@d offset_depth_base=offset_height_base+1
@d offset_italic_base=offset_depth_base+1
@d offset_lig_kern_base=offset_italic_base+1
@d offset_kern_base=offset_lig_kern_base+1
@d offset_exten_base=offset_kern_base+1
@d offset_param_base=offset_exten_base+1
@d offset_charinfo_base=offset_param_base+1
@d font_file_size(#)==font_info(#)(offset_file_size).int
@d font_check_0(#)==font_info(#)(offset_check).int
@d font_check_1(#)==font_info(#)(offset_check+1).int
@d font_check_2(#)==font_info(#)(offset_check+2).int
@d font_check_3(#)==font_info(#)(offset_check+3).int
@d font_offset(#)==font_info(#)(offset_offset).int
@d font_size(#)==font_info(#)(offset_size).int
@d font_dsize(#)==font_info(#)(offset_dsize).int
@d font_params(#)==font_info(#)(offset_params).int
@d font_name_sort(#)==font_info(#)(offset_name_sort).int
@d font_name(#)==font_info(#)(offset_name).int
@d font_area(#)==font_info(#)(offset_area).int
@d font_bc(#)==font_info(#)(offset_bc).int
@d font_ec(#)==font_info(#)(offset_ec).int
@d font_glue(#)==font_info(#)(offset_glue).int
@d font_used(#)==font_info(#)(offset_used).int
@d hyphen_char(#)==font_info(#)(offset_hyphen).int
@d skew_char(#)==font_info(#)(offset_skew).int
@d bchar_label(#)==font_info(#)(offset_bchar_label).int
@d font_bchar(#)==font_info(#)(offset_bchar).int
@d font_false_bchar(#)==font_info(#)(offset_false_bchar).int
@d ivalues_start(#)==font_info(#)(offset_ivalues_start).int
@d fvalues_start(#)==font_info(#)(offset_fvalues_start).int
@d mvalues_start(#)==font_info(#)(offset_mvalues_start).int
@d rules_start(#)==font_info(#)(offset_rules_start).int
@d glues_start(#)==font_info(#)(offset_glues_start).int
@d penalties_start(#)==font_info(#)(offset_penalties_start).int
@d ivalues_base(#)==font_info(#)(offset_ivalues_base).int
@d fvalues_base(#)==font_info(#)(offset_fvalues_base).int
@d mvalues_base(#)==font_info(#)(offset_mvalues_base).int
@d rules_base(#)==font_info(#)(offset_rules_base).int
@d glues_base(#)==font_info(#)(offset_glues_base).int
@d penalties_base(#)==font_info(#)(offset_penalties_base).int
@d char_attr_base(#)==font_info(#)(offset_char_attr_base).int
@d char_base(#)==font_info(#)(offset_char_base).int
@d width_base(#)==font_info(#)(offset_width_base).int
@d height_base(#)==font_info(#)(offset_height_base).int
@d depth_base(#)==font_info(#)(offset_depth_base).int
@d italic_base(#)==font_info(#)(offset_italic_base).int
@d lig_kern_base(#)==font_info(#)(offset_lig_kern_base).int
@d kern_base(#)==font_info(#)(offset_kern_base).int
@d exten_base(#)==font_info(#)(offset_exten_base).int
@d param_base(#)==font_info(#)(offset_param_base).int

@<Glob...@>=
@!dimen_font:integer;
@z
%---------------------------------------
@x [30] m.551 l.10743 - Omega
for k:=font_base to font_max do font_used[k]:=false;
@y
@z
%---------------------------------------
@x [30] m.552 l.10749 - Omega
font_ptr:=null_font; fmem_ptr:=7;
font_name[null_font]:="nullfont"; font_area[null_font]:="";
hyphen_char[null_font]:="-"; skew_char[null_font]:=-1;
bchar_label[null_font]:=non_address;
font_bchar[null_font]:=non_char; font_false_bchar[null_font]:=non_char;
font_bc[null_font]:=1; font_ec[null_font]:=0;
font_size[null_font]:=0; font_dsize[null_font]:=0;
char_base[null_font]:=0; width_base[null_font]:=0;
height_base[null_font]:=0; depth_base[null_font]:=0;
italic_base[null_font]:=0; lig_kern_base[null_font]:=0;
kern_base[null_font]:=0; exten_base[null_font]:=0;
font_glue[null_font]:=null; font_params[null_font]:=7;
param_base[null_font]:=-1;
for k:=0 to 6 do font_info[k].sc:=0;
@y
font_ptr:=null_font;
allocate_font_table(null_font,offset_charinfo_base+108);
font_file_size(null_font):=offset_charinfo_base+108;
font_used(null_font):=false;
font_name(null_font):="nullfont"; font_area(null_font):="";
hyphen_char(null_font):="-"; skew_char(null_font):=-1;
bchar_label(null_font):=non_address;
font_bchar(null_font):=non_char; font_false_bchar(null_font):=non_char;
font_bc(null_font):=1; font_ec(null_font):=0;
font_size(null_font):=0; font_dsize(null_font):=0;
char_base(null_font):=offset_charinfo_base;
char_attr_base(null_font):=offset_charinfo_base;
ivalues_start(null_font):=offset_charinfo_base;
fvalues_start(null_font):=offset_charinfo_base;
mvalues_start(null_font):=offset_charinfo_base;
rules_start(null_font):=offset_charinfo_base;
glues_start(null_font):=offset_charinfo_base;
penalties_start(null_font):=offset_charinfo_base;
ivalues_base(null_font):=offset_charinfo_base;
fvalues_base(null_font):=offset_charinfo_base;
mvalues_base(null_font):=offset_charinfo_base;
rules_base(null_font):=offset_charinfo_base;
glues_base(null_font):=offset_charinfo_base;
penalties_base(null_font):=offset_charinfo_base;
width_base(null_font):=offset_charinfo_base;
height_base(null_font):=offset_charinfo_base;
depth_base(null_font):=offset_charinfo_base;
italic_base(null_font):=offset_charinfo_base;
lig_kern_base(null_font):=offset_charinfo_base;
kern_base(null_font):=offset_charinfo_base;
exten_base(null_font):=offset_charinfo_base;
font_glue(null_font):=null;
font_params(null_font):=7;
param_base(null_font):=offset_charinfo_base-1;
font_offset(null_font):=0;
for k:=1 to 7 do font_info(null_font)(param_base(null_font)+k).sc:=0;
@z
%---------------------------------------
@x [30] m.553 l.10767 - Omega
text(frozen_null_font):="nullfont"; eqtb[frozen_null_font]:=eqtb[cur_val];
@y
settext(frozen_null_font,"nullfont");
set_new_eqtb(frozen_null_font,new_eqtb(cur_val));
@z
%---------------------------------------
@x [30] m.554 l.10771 - Omega
$$\hbox{|font_info[width_base[f]+font_info[char_base[f]+c].qqqq.b0].sc|}$$
@y
$$\hbox{|font_info(f)(width_base(f)+font_info(f)(char_base(f)+c).qqqq.b0).sc|}$$
@z
%---------------------------------------
@x [30] m.554 l.10785 - Omega
|height_depth(q)| will be the 8-bit quantity
$$b=|height_index|\times16+|depth_index|,$$ and if |b| is such a byte we
@y
|height_depth(q)| will be the 16-bit quantity
$$b=|height_index|\times256+|depth_index|,$$ and if |b| is such a byte we
@z
%---------------------------------------
@x [30] m.554 l.10798 - Omega
@d char_info_end(#)==#].qqqq
@d char_info(#)==font_info[char_base[#]+char_info_end
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
@d char_info_end(#)==#].qqqq
@d char_info(#)==font_tables[#,char_base(#)-font_offset(#)+char_info_end
@d char_width_end(#)==#.b0].sc
@d char_width(#)==font_tables[#,width_base(#)+char_width_end
@d char_exists(#)==(#.b0>min_quarterword)
@d char_italic_end(#)==(qo(#.b2)) div 256].sc
@d char_italic(#)==font_tables[#,italic_base(#)+char_italic_end
@d height_depth(#)==qo(#.b1)
@d char_height_end(#)==(#) div 256].sc
@d char_height(#)==font_tables[#,height_base(#)+char_height_end
@d char_depth_end(#)==(#) mod 256].sc
@d char_depth(#)==font_tables[#,depth_base(#)+char_depth_end
@d char_tag(#)==((qo(#.b2)) mod 4)
@z
%---------------------------------------
@x [30] m.557 l.10834 - Omega
|i=font_info[lig_kern_start(f)(j)]| or |font_info[lig_kern_restart(f)(i)]|,
depending on whether or not |skip_byte(i)<=stop_flag|.

The constant |kern_base_offset| should be simplified, for \PASCAL\ compilers
that do not do local optimization.
@^system dependencies@>

@d char_kern_end(#)==256*op_byte(#)+rem_byte(#)].sc
@d char_kern(#)==font_info[kern_base[#]+char_kern_end
@d kern_base_offset==256*(128+min_quarterword)
@d lig_kern_start(#)==lig_kern_base[#]+rem_byte {beginning of lig/kern program}
@d lig_kern_restart_end(#)==256*op_byte(#)+rem_byte(#)+32768-kern_base_offset
@d lig_kern_restart(#)==lig_kern_base[#]+lig_kern_restart_end
@y
|i=font_info(f)(lig_kern_start(f)(j))| or
|font_info(f)(lig_kern_restart(f)(i))|,
depending on whether or not |skip_byte(i)<=stop_flag|.

The constant |kern_base_offset| should be simplified, for \PASCAL\ compilers
that do not do local optimization.
@^system dependencies@>

@d char_kern_end(#)==256*op_byte(#)+rem_byte(#)].sc
@d char_kern(#)==font_tables[#,kern_base(#)+char_kern_end
@d kern_base_offset==256*(128+min_quarterword)
@d lig_kern_start(#)==lig_kern_base(#)+rem_byte {beginning of lig/kern program}
@d lig_kern_restart_end(#)==256*op_byte(#)+rem_byte(#)+32768-kern_base_offset
@d lig_kern_restart(#)==lig_kern_base(#)+lig_kern_restart_end

@d attr_zero_char_ivalue_end(#)==#].int].int
@d attr_zero_char_ivalue(#)==
   font_tables[#,font_tables[#,char_attr_base(#)+attr_zero_char_ivalue_end
@d attr_zero_ivalue_end(#)==#].int
@d attr_zero_ivalue(#)==font_tables[#,ivalues_base(#)+attr_zero_ivalue_end
@d attr_zero_penalty_end(#)==#].int
@d attr_zero_penalty(#)==font_tables[#,penalties_base(#)+attr_zero_penalty_end
@d attr_zero_kern_end(#)==#].int
@d attr_zero_kern(#)==font_tables[#,mvalues_base(#)+attr_zero_kern_end

@z
%---------------------------------------
@x [30] m.558 l.10843 - Omega
@d param_end(#)==param_base[#]].sc
@d param(#)==font_info[#+param_end
@d slant==param(slant_code) {slant to the right, per unit distance upward}
@d space==param(space_code) {normal space between words}
@d space_stretch==param(space_stretch_code) {stretch between words}
@d space_shrink==param(space_shrink_code) {shrink between words}
@d x_height==param(x_height_code) {one ex}
@d quad==param(quad_code) {one em}
@d extra_space==param(extra_space_code) {additional space at end of sentence}
@y
@d slant(#)==font_info(#)(param_base(#)+slant_code).sc
   {slant to the right, per unit distance upward}
@d space(#)==font_info(#)(param_base(#)+space_code).sc
   {normal space between words}
@d space_stretch(#)==font_info(#)(param_base(#)+space_stretch_code).sc
   {stretch between words}
@d space_shrink(#)==font_info(#)(param_base(#)+space_shrink_code).sc
   {shrink between words}
@d x_height(#)==font_info(#)(param_base(#)+x_height_code).sc
   {one ex}
@d quad(#)==font_info(#)(param_base(#)+quad_code).sc
   {one em}
@d extra_space(#)==font_info(#)(param_base(#)+extra_space_code).sc
   {additional space at end of sentence}
@z
%---------------------------------------
@x [30] m.560 l.10876 - Omega
@p function read_font_info(@!u:pointer;@!nom,@!aire:str_number;
  @!s:scaled):internal_font_number; {input a \.{TFM} file}
@y
@p function read_font_info(@!u:pointer;@!nom,@!aire:str_number;
   @!s:scaled;offset:quarterword):internal_font_number; {input a \.{TFM} file}
@z
%---------------------------------------
@x [30] m.560 l.10882 - Omega
  {sizes of subfiles}
@!f:internal_font_number; {the new font's number}
@!g:internal_font_number; {the number to return}
@!a,@!b,@!c,@!d:eight_bits; {byte variables}
@y
@!font_dir:halfword;
  {sizes of subfiles}
@!f:internal_font_number; {the new font's number}
@!g:internal_font_number; {the number to return}
@!a,@!b,@!c,@!d:integer; {byte variables}
@!i,@!k_param,@!param,@!font_counter:integer; {counters}
@!font_level,@!header_length:integer;
@!nco,@!ncw,@!npc,@!nlw,@!neew:integer;
@!nki,@!nwi,@!nkf,@!nwf,@!nkm,@!nwm:integer;
@!nkr,@!nwr,@!nkg,@!nwg,@!nkp,@!nwp:integer;
@!table_size:array [0..31] of integer;
@!bytes_per_entry,@!extra_char_bytes:integer;
@!repeat_no,@!table_counter:integer;
@z
%---------------------------------------
@x [30] m.560 l.10888 - Omega
@!bchar:0..256; {right boundary character, or 256}
@y
@!bchar:0..too_big_char; {right boundary character, or |too_big_char|}
@!first_two:integer;
@z
%---------------------------------------
@x [30] m.563 l.10943 - Omega
if file_opened then print(" not loadable: Bad metric (TFM) file")
else print(" not loadable: Metric (TFM) file not found");
@y
if file_opened then print(" not loadable: Bad metric (TFM/OFM) file")
else print(" not loadable: Metric (TFM/OFM) file not found");
@z
%---------------------------------------
@x [30] m.563 l.10943 - Omega
if aire="" then pack_file_name(nom,TEX_font_area,".tfm")
else pack_file_name(nom,aire,".tfm");
if not b_open_in(tfm_file) then abort;
@y
if aire="" then pack_file_name(nom,TEX_font_area,".ofm")
else pack_file_name(nom,aire,".ofm");
if not b_open_in(tfm_file) then abort;
@z
%---------------------------------------
@x [30] m.564 l.10962 - Omega
@d store_four_quarters(#)==begin fget; a:=fbyte; qw.b0:=qi(a);
  fget; b:=fbyte; qw.b1:=qi(b);
  fget; c:=fbyte; qw.b2:=qi(c);
  fget; d:=fbyte; qw.b3:=qi(d);
  #:=qw;
  end
@y
@d read_sixteen_unsigned(#)==begin #:=fbyte;
  fget; #:=#*@'400+fbyte;
  end
@d read_thirtytwo(#)==begin #:=fbyte;
  if #>127 then abort;
  fget; #:=#*@'400+fbyte;
  fget; #:=#*@'400+fbyte;
  fget; #:=#*@'400+fbyte;
  end
@d store_four_bytes(#)==begin fget; a:=fbyte; qw.b0:=a;
  fget; b:=fbyte; qw.b1:=b;
  fget; c:=fbyte; qw.b2:=c;
  fget; d:=fbyte; qw.b3:=d;
  #:=qw;
  end
@d store_char_info(#)==begin if (font_level<>-1) then
  begin fget; read_sixteen_unsigned(a); qw.b0:=a;
        fget; read_sixteen_unsigned(b); qw.b1:=b;
        fget; read_sixteen_unsigned(c); qw.b2:=c;
        fget; read_sixteen_unsigned(d); qw.b3:=d;
        #:=qw
  end
else 
  begin fget; a:=fbyte; qw.b0:=a;
        fget; b:=fbyte; b:=((b div 16)*256)+(b mod 16); qw.b1:=b;
        fget; c:=fbyte; c:=((c div 4)*256)+(c mod 4); qw.b2:=c;
        fget; d:=fbyte; qw.b3:=d;
        #:=qw
  end
end
@d store_four_quarters(#)==begin if (font_level<>-1) then
  begin fget; read_sixteen_unsigned(a); qw.b0:=a;
        fget; read_sixteen_unsigned(b); qw.b1:=b;
        fget; read_sixteen_unsigned(c); qw.b2:=c;
        fget; read_sixteen_unsigned(d); qw.b3:=d;
        #:=qw
  end
else 
  begin fget; a:=fbyte; qw.b0:=a;
        fget; b:=fbyte; qw.b1:=b;
        fget; c:=fbyte; qw.b2:=c;
        fget; d:=fbyte; qw.b3:=d;
        #:=qw
  end
end
@z
%---------------------------------------
@x [30] m.565 l.10970 - Omega
begin read_sixteen(lf);
fget; read_sixteen(lh);
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
if lf<>6+lh+(ec-bc+1)+nw+nh+nd+ni+nl+nk+ne+np then abort;
end
@y
nco:=0; ncw:=0; npc:=0;
nki:=0; nwi:=0; nkf:=0; nwf:=0; nkm:=0; nwm:=0;
nkr:=0; nwr:=0; nkg:=0; nwg:=0; nkp:=0; nwp:=0;
read_sixteen(first_two);
if first_two<>0 then begin
  font_level:=-1;
  lf:=first_two;
  fget; read_sixteen(lh);
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
  header_length:=6;
  ncw:=(ec-bc+1);
  nlw:=nl;
  neew:=ne;
  end
else begin
  fget; read_sixteen(font_level);
  if (font_level<>0) and (font_level<>1) then abort;
  fget; read_thirtytwo(lf);
  fget; read_thirtytwo(lh);
  fget; read_thirtytwo(bc);
  fget; read_thirtytwo(ec);
  if (bc>ec+1)or(ec>biggest_char) then abort;
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
  fget; read_thirtytwo(font_dir);
  nlw:=2*nl;
  neew:=2*ne;
  if font_level=0 then begin
    header_length:=14;
    ncw:=2*(ec-bc+1);
    end
  else begin
    header_length:=29;
    fget; read_thirtytwo(nco);
    fget; read_thirtytwo(ncw);
    fget; read_thirtytwo(npc);
    fget; read_thirtytwo(nki); {Kinds of font ivalues}
    fget; read_thirtytwo(nwi); {Words of font ivalues}
    fget; read_thirtytwo(nkf); {Kinds of font fvalues}
    fget; read_thirtytwo(nwf); {Words of font fvalues}
    fget; read_thirtytwo(nkm); {Kinds of font mvalues}
    fget; read_thirtytwo(nwm); {Words of font mvalues}
    fget; read_thirtytwo(nkr); {Kinds of font rules}
    fget; read_thirtytwo(nwr); {Words of font rules}
    fget; read_thirtytwo(nkg); {Kinds of font glues}
    fget; read_thirtytwo(nwg); {Words of font glues}
    fget; read_thirtytwo(nkp); {Kinds of font penalties}
    fget; read_thirtytwo(nwp); {Words of font penalties}
   end
  end;
if lf<>(header_length+lh+ncw+nw+nh+nd+ni+nlw+nk+neew+np+
        nki+nwi+nkf+nwf+nkm+nwm+nkr+nwr+nkg+nwg+nkp+nwp) then abort;
@z
%---------------------------------------
@x [30] m.566 l.10996 - Omega
lf:=lf-6-lh; {|lf| words should be loaded into |font_info|}
if np<7 then lf:=lf+7-np; {at least seven parameters will appear}
if (font_ptr=font_max)or(fmem_ptr+lf>font_mem_size) then
  @<Apologize for not loading the font, |goto done|@>;
f:=font_ptr+1;
char_base[f]:=fmem_ptr-bc;
width_base[f]:=char_base[f]+ec+1;
height_base[f]:=width_base[f]+nw;
depth_base[f]:=height_base[f]+nh;
italic_base[f]:=depth_base[f]+nd;
lig_kern_base[f]:=italic_base[f]+ni;
kern_base[f]:=lig_kern_base[f]+nl-kern_base_offset;
exten_base[f]:=kern_base[f]+kern_base_offset+nk;
param_base[f]:=exten_base[f]+ne
@y
case font_level of
-1:  lf:=lf-6-lh; {|lf| words should be loaded into |font_info|}
 0:  lf:=lf-14-lh-(ec-bc+1)-nl-ne;
   {some of the sections contain pairs of
    words that become single |memory_word|s}
 1:  lf:=lf-29-lh-ncw+(1+npc)*(ec-bc+1)-nl-ne;
   {We explode the characters}
end;
if np<7 then lf:=lf+7-np; {at least seven parameters will appear}
if (font_ptr=font_max) then
  @<Apologize for not loading the font, |goto done|@>;
f:=font_ptr+1;
allocate_font_table(f,offset_charinfo_base+lf+100);
    {leave room for 100 extra parameters -- potential bug}
font_file_size(f):=offset_charinfo_base+lf+100;
font_used(f):=false;
font_offset(f):=offset;
char_base(f):=offset_charinfo_base-bc;
char_attr_base(f):=char_base(f)+ec+1;
ivalues_start(f):=char_attr_base(f)+npc*(ec-bc+1);
fvalues_start(f):=ivalues_start(f)+nki;
mvalues_start(f):=fvalues_start(f)+nkf;
rules_start(f):=mvalues_start(f)+nkm;
glues_start(f):=rules_start(f)+nkr;
penalties_start(f):=glues_start(f)+nkg;
ivalues_base(f):=penalties_start(f)+nkp;
fvalues_base(f):=ivalues_base(f)+nwi;
mvalues_base(f):=fvalues_base(f)+nwf;
rules_base(f):=mvalues_base(f)+nwm;
glues_base(f):=rules_base(f)+nwr;
penalties_base(f):=glues_base(f)+nwg;
width_base(f):=penalties_base(f)+nwp;
height_base(f):=width_base(f)+nw;
depth_base(f):=height_base(f)+nh;
italic_base(f):=depth_base(f)+nd;
lig_kern_base(f):=italic_base(f)+ni;
kern_base(f):=lig_kern_base(f)+nl-kern_base_offset;
exten_base(f):=kern_base(f)+kern_base_offset+nk;
param_base(f):=exten_base(f)+ne;
@z
%---------------------------------------
@x [30] m.568 l.11026 - Omega
store_four_quarters(font_check[f]);
@y
fget; font_check_0(f):=fbyte;
fget; font_check_1(f):=fbyte;
fget; font_check_2(f):=fbyte;
fget; font_check_3(f):=fbyte;
@z
%---------------------------------------
@x [30] m.568 l.11033 - Omega
font_dsize[f]:=z;
if s<>-1000 then
  if s>=0 then z:=s
  else z:=xn_over_d(z,-s,1000);
font_size[f]:=z;
@y
font_dsize(f):=z;
if s<>-1000 then
  if s>=0 then z:=s
  else z:=xn_over_d(z,-s,1000);
font_size(f):=z;
@z
%---------------------------------------
@x [30] m.569 l.11041 - Omega
for k:=fmem_ptr to width_base[f]-1 do
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
if font_level=1 then begin
  i:=0;
  k:=ivalues_start(f);
  font_counter:=ivalues_base(f);
  while k<fvalues_start(f) do       {IVALUE starts}
    begin
    read_thirtytwo(param);
    font_info(f)(k).int := font_counter;
    font_counter:=font_counter+param;
    table_size[i]:=1;
    incr(i); incr(k);
    end;
  while k<mvalues_start(f) do       {FVALUE starts}
    begin
    read_thirtytwo(param);
    font_info(f)(k).int := font_counter;
    font_counter:=font_counter+param;
    table_size[i]:=1;
    incr(i); incr(k);
    end;
  while k<rules_start(f) do         {MVALUE starts}
    begin
    read_thirtytwo(param);
    font_info(f)(k).int := font_counter;
    font_counter:=font_counter+param;
    table_size[i]:=1;
    incr(i); incr(k);
    end;
  while k<glues_start(f) do         {RULE starts}
    begin
    read_thirtytwo(param);
    font_info(f)(k).int := font_counter;
    font_counter:=font_counter+param*3;
    table_size[i]:=3;
    incr(i); incr(k);
    end;
  while k<penalties_start(f) do     {GLUE starts}
    begin
    read_thirtytwo(param);
    font_info(f)(k).int := font_counter;
    font_counter:=font_counter+param*4;
    table_size[i]:=4;
    incr(i); incr(k);
    end;
  while k<ivalues_base(f) do        {PENALTY starts}
    begin
    read_thirtytwo(param);
    font_info(f)(k).int := font_counter;
    font_counter:=font_counter+param*1;
    table_size[i]:=1;
    incr(i); incr(k);
    end;
  while k<fvalues_base(f) do        {IVALUE entries}
    begin
    read_thirtytwo(font_info(f)(k).int);
    incr(k);
    end;
  while k<mvalues_base(f) do        {FVALUE entries}
    begin
    read_thirtytwo(font_info(f)(k).sc);
    incr(k);
    end;
  while k<rules_base(f) do          {MVALUE entries}
    begin
    read_thirtytwo(font_info(f)(k).int);
    incr(k);
    end;
  while k<glues_base(f) do          {RULE entries}
    begin
    store_scaled(font_info(f)(k).sc);
    store_scaled(font_info(f)(k+1).sc);
    store_scaled(font_info(f)(k+2).sc);
    k:=k+3;
    end;
  while k<penalties_base(f) do      {GLUE entries}
    begin
    fget; read_sixteen(font_info(f)(k).hh.lhfield);
    fget; read_sixteen(font_info(f)(k).hh.rh);
    store_scaled(font_info(f)(k+1).sc);
    store_scaled(font_info(f)(k+2).sc);
    store_scaled(font_info(f)(k+3).sc);
    k:=k+4;
    end;
  while k<offset_charinfo_base do      {PENALTY entries}
    begin
    read_thirtytwo(font_info(f)(k).int);
    incr(k);
    end;
  end;
k:=char_base(f)+bc;
k_param:=char_attr_base(f);
bytes_per_entry:=(12 + 2*npc) div 4 * 4;
extra_char_bytes:=bytes_per_entry - (10 + 2*npc);
while k<char_attr_base(f) do begin
  store_char_info(font_info(f)(k).qqqq);
  if (a>=nw)or((b div @"100)>=nh)or((b mod @"100)>=nd)or
     ((c div @"100)>=ni) then abort;
  case c mod 4 of
    lig_tag: if d>=nl then abort;
    ext_tag: if d>=ne then abort;
    list_tag: @<Check for charlist cycle@>;
    othercases do_nothing {|no_tag|}
    endcases;
  incr(k);
  if font_level=1 then begin
    fget; read_sixteen_unsigned(repeat_no);
    for i:=0 to npc-1 do begin
      fget; read_sixteen(param);
      font_info(f)(k_param).int :=
         font_info(f)(ivalues_start(f)+i).int + param*table_size[i];
      incr(k_param);
      end;
    for i:=1 to extra_char_bytes do fget;
    for i:=1 to repeat_no do begin
      font_info(f)(k):=font_info(f)(k-1);
      incr(k);
      for table_counter:=0 to npc-1 do begin
        font_info(f)(k_param).int := font_info(f)(k_param-npc).int;
        incr(k_param);
        end;
      end;
    end;
  end
@z
%---------------------------------------
@x [30] m.570 l.11059 - Omega
@d current_character_being_worked_on==k+bc-fmem_ptr
@y
@d current_character_being_worked_on==k-char_base(f)
@z
%---------------------------------------
@x [30] m.570 l.11065 - Omega
  begin qw:=char_info(f)(d);
  {N.B.: not |qi(d)|, since |char_base[f]| hasn't been adjusted yet}
@y
  begin qw:=char_info(f)(d);
  {N.B.: not |qi(d)|, since |char_base(f)| hasn't been adjusted yet}
@z
%---------------------------------------
@x [30] m.571 l.11098 - Omega
for k:=width_base[f] to lig_kern_base[f]-1 do
  store_scaled(font_info[k].sc);
if font_info[width_base[f]].sc<>0 then abort; {\\{width}[0] must be zero}
if font_info[height_base[f]].sc<>0 then abort; {\\{height}[0] must be zero}
if font_info[depth_base[f]].sc<>0 then abort; {\\{depth}[0] must be zero}
if font_info[italic_base[f]].sc<>0 then abort; {\\{italic}[0] must be zero}
@y
for k:=width_base(f) to lig_kern_base(f)-1 do
  store_scaled(font_info(f)(k).sc);
if font_info(f)(width_base(f)).sc<>0 then abort; {\\{width}[0] must be zero}
if font_info(f)(height_base(f)).sc<>0 then abort; {\\{height}[0] must be zero}
if font_info(f)(depth_base(f)).sc<>0 then abort; {\\{depth}[0] must be zero}
if font_info(f)(italic_base(f)).sc<>0 then abort; {\\{italic}[0] must be zero}
@z
%---------------------------------------
@x [30] m.573 l.11114 - Omega
@ @d check_existence(#)==@t@>@;@/
  begin check_byte_range(#);
  qw:=char_info(f)(#); {N.B.: not |qi(#)|}
  if not char_exists(qw) then abort;
  end

@<Read ligature/kern program@>=
bch_label:=@'77777; bchar:=256;
if nl>0 then
  begin for k:=lig_kern_base[f] to kern_base[f]+kern_base_offset-1 do
    begin store_four_quarters(font_info[k].qqqq);
    if a>128 then
      begin if 256*c+d>=nl then abort;
      if a=255 then if k=lig_kern_base[f] then bchar:=b;
      end
    else begin if b<>bchar then check_existence(b);
      if c<128 then check_existence(d) {check ligature}
      else if 256*(c-128)+d>=nk then abort; {check kern}
      if a<128 then if k-lig_kern_base[f]+a+1>=nl then abort;
      end;
    end;
  if a=255 then bch_label:=256*c+d;
  end;
for k:=kern_base[f]+kern_base_offset to exten_base[f]-1 do
  store_scaled(font_info[k].sc);
@y
@ @d check_existence(#)==@t@>@;@/
  begin check_byte_range(#);
  qw:=char_info(f)(#+font_offset(f)); {N.B.: not |qi(#)|}
  if not char_exists(qw) then abort;
  end

@<Read ligature/kern program@>=
bch_label:=nl {infinity}; bchar:=too_big_char;
if nl>0 then
  begin for k:=lig_kern_base(f) to kern_base(f)+kern_base_offset-1 do
    begin store_four_quarters(font_info(f)(k).qqqq);
    if a>128 then
      begin if 256*c+d>=nl then abort;
      if a=255 then if k=lig_kern_base(f) then bchar:=b;
      end
    else begin if b<>bchar then check_existence(b);
      if c<128 then check_existence(d) {check ligature}
      else if 256*(c-128)+d>=nk then abort; {check kern}
      if a<128 then if k-lig_kern_base(f)+a+1>=nl then abort;
      end;
    end;
  if a=255 then bch_label:=256*c+d;
  end;
for k:=kern_base(f)+kern_base_offset to exten_base(f)-1 do
  store_scaled(font_info(f)(k).sc);
@z
%---------------------------------------
@x [30] m.574 l.11141 - Omega
for k:=exten_base[f] to param_base[f]-1 do
  begin store_four_quarters(font_info[k].qqqq);
@y
for k:=exten_base(f) to param_base(f)-1 do
  begin store_four_quarters(font_info(f)(k).qqqq);
@z
%---------------------------------------
@x [30] m.575 l.11157 - Omega
    fget; font_info[param_base[f]].sc:=
      (sw*@'20)+(fbyte div@'20);
    end
  else store_scaled(font_info[param_base[f]+k-1].sc);
if eof(tfm_file) then abort;
for k:=np+1 to 7 do font_info[param_base[f]+k-1].sc:=0;
@y
    fget; font_info(f)(param_base(f)).sc:=
      (sw*@'20)+(fbyte div@'20);
    end
  else store_scaled(font_info(f)(param_base(f)+k-1).sc);
if eof(tfm_file) then abort;
for k:=np+1 to 7 do font_info(f)(param_base(f)+k-1).sc:=0;
@z
%---------------------------------------
@x [30] m.576 l.11169 - Omega
@d adjust(#)==#[f]:=qo(#[f])
  {correct for the excess |min_quarterword| that was added}

@<Make final adjustments...@>=
if np>=7 then font_params[f]:=np@+else font_params[f]:=7;
hyphen_char[f]:=default_hyphen_char; skew_char[f]:=default_skew_char;
if bch_label<nl then bchar_label[f]:=bch_label+lig_kern_base[f]
else bchar_label[f]:=non_address;
font_bchar[f]:=qi(bchar);
font_false_bchar[f]:=qi(bchar);
if bchar<=ec then if bchar>=bc then
  begin qw:=char_info(f)(bchar); {N.B.: not |qi(bchar)|}
  if char_exists(qw) then font_false_bchar[f]:=non_char;
  end;
font_name[f]:=nom;
font_area[f]:=aire;
font_bc[f]:=bc; font_ec[f]:=ec; font_glue[f]:=null;
adjust(char_base); adjust(width_base); adjust(lig_kern_base);
adjust(kern_base); adjust(exten_base);
decr(param_base[f]);
fmem_ptr:=fmem_ptr+lf; font_ptr:=f; g:=f; goto done
@y
@d adjust(#)==#(f):=qo(#(f))
  {correct for the excess |min_quarterword| that was added}

@<Make final adjustments...@>=
if np>=7 then font_params(f):=np@+else font_params(f):=7;
font_offset(f):=offset;
hyphen_char(f):=default_hyphen_char+offset;
skew_char(f):=default_skew_char+offset;
bchar:=bchar+offset;
if bch_label<nl then bchar_label(f):=bch_label+lig_kern_base(f)
else bchar_label(f):=non_address;
font_bchar(f):=qi(bchar);
font_false_bchar(f):=qi(bchar);
if bchar<=ec then if bchar>=bc then
  begin qw:=char_info(f)(bchar); {N.B.: not |qi(bchar)|}
  if char_exists(qw) then font_false_bchar(f):=non_char;
  end;
font_name(f):=nom;
font_area(f):=aire;
font_bc(f):=bc; font_ec(f):=ec; font_glue(f):=null;
font_bc(f):=bc+offset; font_ec(f):=ec+offset; font_glue(f):=null;
adjust(char_base); adjust(width_base); adjust(lig_kern_base);
adjust(kern_base); adjust(exten_base);
decr(param_base(f));
font_ptr:=f; g:=f; goto done
@z
%---------------------------------------
@x [30] m.577 l.11202 - Omega
  begin m:=cur_chr; scan_four_bit_int; f:=equiv(m+cur_val);
@y
  begin m:=cur_chr; scan_big_four_bit_int; f:=equiv(m+cur_val);
@z
%---------------------------------------
@x [30] m.578 l.11223 - Omega
if n<=0 then cur_val:=fmem_ptr
else  begin if writing and(n<=space_shrink_code)and@|
    (n>=space_code)and(font_glue[f]<>null) then
    begin delete_glue_ref(font_glue[f]);
    font_glue[f]:=null;
    end;
  if n>font_params[f] then
    if f<font_ptr then cur_val:=fmem_ptr
    else @<Increase the number of parameters in the last font@>
  else cur_val:=n+param_base[f];
  end;
@<Issue an error message if |cur_val=fmem_ptr|@>;
end;
@y
dimen_font:=f;
if n<=0 then cur_val:=font_file_size(f)
else  begin if writing and(n<=space_shrink_code)and@|
    (n>=space_code)and(font_glue(f)<>null) then
    begin delete_glue_ref(font_glue(f));
    font_glue(f):=null;
    end;
  if n>font_params(f) then
    if f<font_ptr then cur_val:=font_file_size(f)
    else @<Increase the number of parameters in the last font@>
  else cur_val:=n+param_base(f);
  end;
@<Issue an error message if |cur_val=font_file_size(f)|@>;
end;
@z
%---------------------------------------
@x [30] m.579 l.11240 - Omega
@ @<Issue an error message if |cur_val=fmem_ptr|@>=
if cur_val=fmem_ptr then
  begin print_err("Font "); print_esc(font_id_text(f));
  print(" has only "); print_int(font_params[f]);
  print(" fontdimen parameters");
@.Font x has only...@>
  help2("To increase the number of font parameters, you must")@/
    ("use \fontdimen immediately after the \font is loaded.");
  error;
  end
@y
@ @<Issue an error message if |cur_val=font_file_size(f)|@>=
if cur_val=font_file_size(f) then
  begin print_err("Font "); print_esc(font_id_text(f));
  print(" has only "); print_int(font_params(f));
  print(" fontdimen parameters");
@.Font x has only...@>
  help2("To increase the number of font parameters, you must")@/
    ("use \fontdimen immediately after the \font is loaded.");
  error;
  end
@z
%---------------------------------------
@x [30] m.580 l.11252 - font parameters
@ @<Increase the number of parameters...@>=
begin repeat if fmem_ptr=font_mem_size then
  overflow("font memory",font_mem_size);
@:TeX capacity exceeded font memory}{\quad font memory@>
font_info[fmem_ptr].sc:=0; incr(fmem_ptr); incr(font_params[f]);
until n=font_params[f];
cur_val:=fmem_ptr-1; {this equals |param_base[f]+font_params[f]|}
@y
@ @<Increase the number of parameters...@>=
begin if n+font_params(f)>font_file_size(f) then
  begin print_nl("Out of font parameter space"); succumb; end;
repeat
font_info(f)(param_base(f)+font_params(f)).sc:=0; incr(font_params(f));
until n=font_params(f);
cur_val:=param_base(f)+font_params(f);
@z
%---------------------------------------
@x [30] m.581 l.11268 - font parameters
  slow_print(font_name[f]); print_char("!"); end_diagnostic(false);
@y
  slow_print(font_name(f)); print_char("!"); end_diagnostic(false);
@z
%---------------------------------------
@x [30] m.582 l.11276 - font parameters
begin if font_bc[f]<=c then if font_ec[f]>=c then
@y
begin if font_bc(f)<=c then if font_ec(f)>=c then
@z
%---------------------------------------
@x [32] m.592 l.11820 - Omega
@!c,@!f:quarterword; {character and font in current |char_node|}
@y
@!c,@!f:quarterword; {character and font in current |char_node|}
@!oval,@!ocmd:integer; {used by |out_cmd| for generating
  |set|, |fnt| and |fnt_def| commands}
@z
%---------------------------------------
@x [32] m.602 l.11939 - Omega
@ Here's a procedure that outputs a font definition. Since \TeX82 uses at
most 256 different fonts per job, |fnt_def1| is always used as the command code.
@y
@ Here's a procedure that outputs a font definition. $\Omega$ allows
more than 256 different fonts per job, so the right font definition 
command must be selected.

@d Incr_Decr(#) == #
@d Incr(#) == #:=#+Incr_Decr {increase a variable}
@d Decr(#) == #:=#-Incr_Decr {decrease a variable}

@d dvi_debug_out(#)==begin
dvi_out(#);
end

@d out_cmd ==
begin
if (oval<@"100)and(oval>=0) then begin
  if (ocmd<>set1)or(oval>127) then
  if (ocmd=fnt1)and(oval<64) then Incr(oval)(fnt_num_0) @+ else dvi_debug_out(ocmd);
  end
else
  begin if (oval<@"10000)and(oval>=0) then dvi_debug_out(ocmd+1) @+ else @;
    begin if (oval<@"1000000)and(oval>=0) then dvi_debug_out(ocmd+2) @+ else @;
      begin dvi_debug_out(ocmd+3);
      if oval>=0 then dvi_debug_out(oval div @"1000000)
      else  begin Incr(oval)(@"40000000); Incr(oval)(@"40000000);
        dvi_debug_out((oval div @"1000000) + 128); oval:=oval mod @"1000000;
        end;
      dvi_debug_out(oval div @"10000); oval:=oval mod @"10000;
      end;
    dvi_debug_out(oval div @"10000); oval:=oval mod @"10000;
    end;
  dvi_debug_out(oval div @"100); oval:=oval mod @"100;
  end;
dvi_debug_out(oval)
end

@z
%---------------------------------------
@x [32] m.602 l.11944 - Omega
begin dvi_out(fnt_def1);
dvi_out(f-font_base-1);@/
dvi_out(qo(font_check[f].b0));
dvi_out(qo(font_check[f].b1));
dvi_out(qo(font_check[f].b2));
dvi_out(qo(font_check[f].b3));@/
dvi_four(font_size[f]);
dvi_four(font_dsize[f]);@/
dvi_out(length(font_area[f]));
dvi_out(length(font_name[f]));
@y
begin
oval:=f-font_base-1; ocmd:= fnt_def1; out_cmd;@/
dvi_out(qo(font_check_0(f)));
dvi_out(qo(font_check_1(f)));
dvi_out(qo(font_check_2(f)));
dvi_out(qo(font_check_3(f)));@/
dvi_four(font_size(f));
dvi_four(font_dsize(f));@/
dvi_out(length(font_area(f)));
dvi_out(length(font_name(f)));
@z
%---------------------------------------
@x [32] m.603 l.11958 - Omega
for k:=str_start[font_area[f]] to str_start[font_area[f]+1]-1 do
  dvi_out(so(str_pool[k]));
for k:=str_start[font_name[f]] to str_start[font_name[f]+1]-1 do
  dvi_out(so(str_pool[k]))
@y
for k:=str_start(font_area(f)) to str_start(font_area(f)+1)-1 do
  dvi_out(so(str_pool[k]));
for k:=str_start(font_name(f)) to str_start(font_name(f)+1)-1 do
  dvi_out(so(str_pool[k]))
@z
%---------------------------------------
@x [32] m.620 l.12334 - Omega
  if c>=qi(128) then dvi_out(set1);
  dvi_out(qo(c));@/
@y
  oval:=c-font_offset(f); ocmd:=set1; out_cmd;@/
@z
%---------------------------------------
@x [32] m.621 l.12345 - Omega
begin if not font_used[f] then
  begin dvi_font_def(f); font_used[f]:=true;
  end;
if f<=64+font_base then dvi_out(f-font_base-1+fnt_num_0)
else  begin dvi_out(fnt1); dvi_out(f-font_base-1);
  end;
@y
begin if not font_used(f) then
  begin dvi_font_def(f); font_used(f):=true;
  end;
oval:=f-font_base-1; ocmd:=fnt1; out_cmd;@/
@z
%---------------------------------------
@x [32] m.643 l.12757 - Omega
  begin if font_used[font_ptr] then dvi_font_def(font_ptr);
@y
  begin if font_used(font_ptr) then dvi_font_def(font_ptr);
@z
%---------------------------------------
@x [35] m.699 l.13742 - Omega
@d text_size=0 {size code for the largest size in a family}
@d script_size=16 {size code for the medium size in a family}
@d script_script_size=32 {size code for the smallest size in a family}
@y
@z
%---------------------------------------
@x [35] m.700 l.13762 - Omega
@d mathsy_end(#)==fam_fnt(2+#)]].sc
@d mathsy(#)==font_info[#+param_base[mathsy_end
@y
@d mathsy_end(#)==sc
@d mathsy(#)==font_info(fam_fnt(2+cur_size))(#+param_base(fam_fnt(2+cur_size))).mathsy_end
@z
%---------------------------------------
@x [35] m.701 l.13789 - Omega
@d mathex(#)==font_info[#+param_base[fam_fnt(3+cur_size)]].sc
@y
@d mathex(#)==font_info(fam_fnt(3+cur_size))(#+param_base(fam_fnt(3+cur_size))).sc
@z
%---------------------------------------
@x [35] m.703 l.13813 - Omega
@<Set up the values of |cur_size| and |cur_mu|, based on |cur_style|@>=
begin if cur_style<script_style then cur_size:=text_size
else cur_size:=16*((cur_style-text_style) div 2);
cur_mu:=x_over_n(math_quad(cur_size),18);
end
@y
@<Set up the values of |cur_size| and |cur_mu|, based on |cur_style|@>=
begin if cur_style<script_style then cur_size:=text_size
else cur_size:=script_size*((cur_style-text_style) div 2);
cur_mu:=x_over_n(math_quad(cur_size),18);
end
@z
%---------------------------------------
@x [35] m.706 l.13855 - Omega
function var_delimiter(@!d:pointer;@!s:small_number;@!v:scaled):pointer;
label found,continue;
var b:pointer; {the box that will be constructed}
@!f,@!g: internal_font_number; {best-so-far and tentative font codes}
@!c,@!x,@!y: quarterword; {best-so-far and tentative character codes}
@!m,@!n: integer; {the number of extensible pieces}
@!u: scaled; {height-plus-depth of a tentative character}
@!w: scaled; {largest height-plus-depth so far}
@!q: four_quarters; {character info}
@!hd: eight_bits; {height-depth byte}
@!r: four_quarters; {extensible pieces}
@!z: small_number; {runs through font family members}
@y
function var_delimiter(@!d:pointer;@!s:integer;@!v:scaled):pointer;
label found,continue;
var b:pointer; {the box that will be constructed}
@!f,@!g: internal_font_number; {best-so-far and tentative font codes}
@!c,@!x,@!y: quarterword; {best-so-far and tentative character codes}
@!m,@!n: integer; {the number of extensible pieces}
@!u: scaled; {height-plus-depth of a tentative character}
@!w: scaled; {largest height-plus-depth so far}
@!q: four_quarters; {character info}
@!hd: eight_bits; {height-depth byte}
@!r: four_quarters; {extensible pieces}
@!z: integer; {runs through font family members}
@z
%---------------------------------------
@x [35] m.706 l.13881 - Omega
shift_amount(b):=half(height(b)-depth(b)) - axis_height(s);
@y
z:=cur_size; cur_size:=s;
shift_amount(b):=half(height(b)-depth(b)) - axis_height(cur_size);
cur_size:=z;
@z
%---------------------------------------
@x [35] m.707 l.13889 - Omega
@<Look at the variants of |(z,x)|; set |f| and |c|...@>=
if (z<>0)or(x<>min_quarterword) then
  begin z:=z+s+16;
  repeat z:=z-16; g:=fam_fnt(z);
  if g<>null_font then
    @<Look at the list of characters starting with |x| in
      font |g|; set |f| and |c| whenever
      a better character is found; |goto found| as soon as a
      large enough variant is encountered@>;
  until z<16;
  end
@y
@<Look at the variants of |(z,x)|; set |f| and |c|...@>=
if (z<>0)or(x<>min_quarterword) then
  begin z:=z+s+script_size;
  repeat z:=z-script_size; g:=fam_fnt(z);
  if g<>null_font then
    @<Look at the list of characters starting with |x| in
      font |g|; set |f| and |c| whenever
      a better character is found; |goto found| as soon as a
      large enough variant is encountered@>;
  until z<script_size;
  end
@z
%---------------------------------------
@x [35] m.708 l.13903 - Omega
if (qo(y)>=font_bc[g])and(qo(y)<=font_ec[g]) then
@y
if (qo(y)>=font_bc(g))and(qo(y)<=font_ec(g)) then
@z
%---------------------------------------
@x [35] m.713 l.13974 - Omega
r:=font_info[exten_base[f]+rem_byte(q)].qqqq;@/
@<Compute the minimum suitable height, |w|, and the corresponding
  number of extension steps, |n|; also set |width(b)|@>;
c:=ext_bot(r);
if c<>min_quarterword then stack_into_box(b,f,c);
c:=ext_rep(r);
for m:=1 to n do stack_into_box(b,f,c);
c:=ext_mid(r);
if c<>min_quarterword then
  begin stack_into_box(b,f,c); c:=ext_rep(r);
  for m:=1 to n do stack_into_box(b,f,c);
  end;
c:=ext_top(r);
@y
r:=font_info(f)(exten_base(f)+rem_byte(q)).qqqq;@/
@<Compute the minimum suitable height, |w|, and the corresponding
  number of extension steps, |n|; also set |width(b)|@>;
c:=ext_bot(f)(r);
if c<>min_quarterword then stack_into_box(b,f,c);
c:=ext_rep(f)(r);
for m:=1 to n do stack_into_box(b,f,c);
c:=ext_mid(f)(r);
if c<>min_quarterword then
  begin stack_into_box(b,f,c); c:=ext_rep(f)(r);
  for m:=1 to n do stack_into_box(b,f,c);
  end;
c:=ext_top(f)(r);
@z
%---------------------------------------
@x [35] m.714 l.13997 - Omega
c:=ext_rep(r); u:=height_plus_depth(f,c);
w:=0; q:=char_info(f)(c); width(b):=char_width(f)(q)+char_italic(f)(q);@/
c:=ext_bot(r);@+if c<>min_quarterword then w:=w+height_plus_depth(f,c);
c:=ext_mid(r);@+if c<>min_quarterword then w:=w+height_plus_depth(f,c);
c:=ext_top(r);@+if c<>min_quarterword then w:=w+height_plus_depth(f,c);
n:=0;
if u>0 then while w<v do
  begin w:=w+u; incr(n);
  if ext_mid(r)<>min_quarterword then w:=w+u;
  end
@y
c:=ext_rep(f)(r); u:=height_plus_depth(f,c);
w:=0; q:=char_info(f)(c); width(b):=char_width(f)(q)+char_italic(f)(q);@/
c:=ext_bot(f)(r);@+if c<>min_quarterword then w:=w+height_plus_depth(f,c);
c:=ext_mid(f)(r);@+if c<>min_quarterword then w:=w+height_plus_depth(f,c);
c:=ext_top(f)(r);@+if c<>min_quarterword then w:=w+height_plus_depth(f,c);
n:=0;
if u>0 then while w<v do
  begin w:=w+u; incr(n);
  if (ext_mid(f)(r))<>min_quarterword then w:=w+u;
  end
@z
%---------------------------------------
@x [36] m.719 l.14107 - Omega
@!cur_style:small_number; {style code at current place in the list}
@!cur_size:small_number; {size code corresponding to |cur_style|}
@y
@!cur_style:integer; {style code at current place in the list}
@!cur_size:integer; {size code corresponding to |cur_style|}
@z
%---------------------------------------
@x [36] m.720 l.14122 - Omega
function clean_box(@!p:pointer;@!s:small_number):pointer;
label found;
var q:pointer; {beginning of a list to be boxed}
@!save_style:small_number; {|cur_style| to be restored}
@y
function clean_box(@!p:pointer;@!s:integer):pointer;
label found;
var q:pointer; {beginning of a list to be boxed}
@!save_style:integer; {|cur_style| to be restored}
@z
%---------------------------------------
@x [36] m.722 l.14172 - Omega
else  begin if (qo(cur_c)>=font_bc[cur_f])and(qo(cur_c)<=font_ec[cur_f]) then
@y
else  begin if (qo(cur_c)>=font_bc(cur_f))and(qo(cur_c)<=font_ec(cur_f)) then
@z
%---------------------------------------
@x [36] m.726 l.14228 - Omega
procedure mlist_to_hlist;
label reswitch, check_dimensions, done_with_noad, done_with_node, delete_q,
  done;
var mlist:pointer; {beginning of the given list}
@!penalties:boolean; {should penalty nodes be inserted?}
@!style:small_number; {the given style}
@!save_style:small_number; {holds |cur_style| during recursion}
@!q:pointer; {runs through the mlist}
@!r:pointer; {the most recent noad preceding |q|}
@!r_type:small_number; {the |type| of noad |r|, or |op_noad| if |r=null|}
@!t:small_number; {the effective |type| of noad |q| during the second pass}
@!p,@!x,@!y,@!z: pointer; {temporary registers for list construction}
@!pen:integer; {a penalty to be inserted}
@!s:small_number; {the size of a noad to be deleted}
@y
procedure mlist_to_hlist;
label reswitch, check_dimensions, done_with_noad, done_with_node, delete_q,
  done;
var mlist:pointer; {beginning of the given list}
@!penalties:boolean; {should penalty nodes be inserted?}
@!style:integer; {the given style}
@!save_style:integer; {holds |cur_style| during recursion}
@!q:pointer; {runs through the mlist}
@!r:pointer; {the most recent noad preceding |q|}
@!r_type:integer; {the |type| of noad |r|, or |op_noad| if |r=null|}
@!t:integer; {the effective |type| of noad |q| during the second pass}
@!p,@!x,@!y,@!z: pointer; {temporary registers for list construction}
@!pen:integer; {a penalty to be inserted}
@!s:integer; {the size of a noad to be deleted}
@z
%---------------------------------------
@x [36] m.741 l.14495 - Omega
if math_type(nucleus(q))=math_char then
  begin fetch(nucleus(q));
  if char_tag(cur_i)=lig_tag then
    begin a:=lig_kern_start(cur_f)(cur_i);
    cur_i:=font_info[a].qqqq;
    if skip_byte(cur_i)>stop_flag then
      begin a:=lig_kern_restart(cur_f)(cur_i);
      cur_i:=font_info[a].qqqq;
      end;
    loop@+ begin if qo(next_char(cur_i))=skew_char[cur_f] then
        begin if op_byte(cur_i)>=kern_flag then
          if skip_byte(cur_i)<=stop_flag then s:=char_kern(cur_f)(cur_i);
        goto done1;
        end;
      if skip_byte(cur_i)>=stop_flag then goto done1;
      a:=a+qo(skip_byte(cur_i))+1;
      cur_i:=font_info[a].qqqq;
      end;
    end;
  end;
done1:
@y
if math_type(nucleus(q))=math_char then
  begin fetch(nucleus(q));
  if char_tag(cur_i)=lig_tag then
    begin a:=lig_kern_start(cur_f)(cur_i);
    cur_i:=font_info(cur_f)(a).qqqq;
    if skip_byte(cur_i)>stop_flag then
      begin a:=lig_kern_restart(cur_f)(cur_i);
      cur_i:=font_info(cur_f)(a).qqqq;
      end;
    loop@+ begin if qo((next_char(cur_f)(cur_i)))=skew_char(cur_f) then
        begin if op_byte(cur_i)>=kern_flag then
          if skip_byte(cur_i)<=stop_flag then s:=char_kern(cur_f)(cur_i);
        goto done1;
        end;
      if skip_byte(cur_i)>=stop_flag then goto done1;
      a:=a+qo(skip_byte(cur_i))+1;
      cur_i:=font_info(cur_f)(a).qqqq;
      end;
    end;
  end;
done1:
@z
%---------------------------------------
@x [36] m.752 l.14722 - Omega
        cur_i:=font_info[a].qqqq;
        if skip_byte(cur_i)>stop_flag then
          begin a:=lig_kern_restart(cur_f)(cur_i);
          cur_i:=font_info[a].qqqq;
          end;
        loop@+ begin @<If instruction |cur_i| is a kern with |cur_c|, attach
            the kern after~|q|; or if it is a ligature with |cur_c|, combine
            noads |q| and~|p| appropriately; then |return| if the cursor has
            moved past a noad, or |goto restart|@>;
          if skip_byte(cur_i)>=stop_flag then return;
          a:=a+qo(skip_byte(cur_i))+1;
          cur_i:=font_info[a].qqqq;
@y
        cur_i:=font_info(cur_f)(a).qqqq;
        if skip_byte(cur_i)>stop_flag then
          begin a:=lig_kern_restart(cur_f)(cur_i);
          cur_i:=font_info(cur_f)(a).qqqq;
          end;
        loop@+ begin @<If instruction |cur_i| is a kern with |cur_c|, attach
            the kern after~|q|; or if it is a ligature with |cur_c|, combine
            noads |q| and~|p| appropriately; then |return| if the cursor has
            moved past a noad, or |goto restart|@>;
          if skip_byte(cur_i)>=stop_flag then return;
          a:=a+qo(skip_byte(cur_i))+1;
          cur_i:=font_info(cur_f)(a).qqqq;
@z
%---------------------------------------
@x [36] m.753 l.14751 - Omega
if next_char(cur_i)=cur_c then if skip_byte(cur_i)<=stop_flag then
@y
if (next_char(cur_f)(cur_i))=cur_c then if skip_byte(cur_i)<=stop_flag then
@z
%---------------------------------------
@x [36] m.756 l.14833 - Omega
procedure make_scripts(@!q:pointer;@!delta:scaled);
var p,@!x,@!y,@!z:pointer; {temporary registers for box construction}
@!shift_up,@!shift_down,@!clr:scaled; {dimensions in the calculation}
@!t:small_number; {subsidiary size code}
begin p:=new_hlist(q);
if is_char_node(p) then
  begin shift_up:=0; shift_down:=0;
  end
else  begin z:=hpack(p,natural);
  if cur_style<script_style then t:=script_size@+else t:=script_script_size;
  shift_up:=height(z)-sup_drop(t);
  shift_down:=depth(z)+sub_drop(t);
@y
procedure make_scripts(@!q:pointer;@!delta:scaled);
var p,@!x,@!y,@!z:pointer; {temporary registers for box construction}
@!shift_up,@!shift_down,@!clr:scaled; {dimensions in the calculation}
@!t:integer; {subsidiary size code}
begin p:=new_hlist(q);
if is_char_node(p) then
  begin shift_up:=0; shift_down:=0;
  end
else  begin z:=hpack(p,natural);
  t:=cur_size;
  if cur_style<script_style
  then cur_size:=script_size@+else cur_size:=script_script_size;
  shift_up:=height(z)-sup_drop(cur_size);
  shift_down:=depth(z)+sub_drop(cur_size);
  cur_size:=t;
@z
%---------------------------------------
@x [36] m.762 l.14967 - Omega
function make_left_right(@!q:pointer;@!style:small_number;
  @!max_d,@!max_h:scaled):small_number;
var delta,@!delta1,@!delta2:scaled; {dimensions used in the calculation}
begin if style<script_style then cur_size:=text_size
else cur_size:=16*((style-text_style) div 2);
@y
function make_left_right(@!q:pointer;@!style:integer;
  @!max_d,@!max_h:scaled):integer;
var delta,@!delta1,@!delta2:scaled; {dimensions used in the calculation}
begin if style<script_style then cur_size:=text_size
else cur_size:=script_size*((style-text_style) div 2);
@z
%---------------------------------------
@x [40] m.896 l.17534 - Omega
done2: hyf_char:=hyphen_char[hf];
@y
done2: hyf_char:=hyphen_char(hf);
@z
%---------------------------------------
@x [40] m.897 l.17555 - Omega
    hyf_bchar:=font_bchar[hf];
@y
    hyf_bchar:=font_bchar(hf);
@z
%---------------------------------------
@x [40] m.898 l.17578 - Omega
if odd(subtype(s)) then hyf_bchar:=font_bchar[hf]@+else hyf_bchar:=non_char;
@y
if odd(subtype(s)) then hyf_bchar:=font_bchar(hf)@+else hyf_bchar:=non_char;
@z
%---------------------------------------
@x [41] m.909 l.17800 - Omega
  begin k:=bchar_label[hf];
  if k=non_address then goto done@+else q:=font_info[k].qqqq;
  end
else begin q:=char_info(hf)(cur_l);
  if char_tag(q)<>lig_tag then goto done;
  k:=lig_kern_start(hf)(q); q:=font_info[k].qqqq;
  if skip_byte(q)>stop_flag then
    begin k:=lig_kern_restart(hf)(q); q:=font_info[k].qqqq;
@y
  begin k:=bchar_label(hf);
  if k=non_address then goto done@+else q:=font_info(hf)(k).qqqq;
  end
else begin q:=char_info(hf)(cur_l);
  if char_tag(q)<>lig_tag then goto done;
  k:=lig_kern_start(hf)(q); q:=font_info(hf)(k).qqqq;
  if skip_byte(q)>stop_flag then
    begin k:=lig_kern_restart(hf)(q); q:=font_info(hf)(k).qqqq;
@z
%---------------------------------------
@x [41] m.909 l.17811 - Omega
loop@+begin if next_char(q)=test_char then if skip_byte(q)<=stop_flag then
@y
loop@+begin if (next_char(hf)(q))=test_char then if skip_byte(q)<=stop_flag then
@z
%---------------------------------------
@x [41] m.909 l.17829 - Omega
  k:=k+qo(skip_byte(q))+1; q:=font_info[k].qqqq;
@y
  k:=k+qo(skip_byte(q))+1; q:=font_info(hf)(k).qqqq;
@z
%---------------------------------------
@x [41] m.915 l.17963 - Omega
  begin l:=reconstitute(l,i,font_bchar[hf],non_char)+1;
@y
  begin l:=reconstitute(l,i,font_bchar(hf),non_char)+1;
@z
%---------------------------------------
@x [41] m.916 l.17980 - Omega
if bchar_label[hf]<>non_address then {put left boundary at beginning of new line}
@y
if bchar_label(hf)<>non_address then {put left boundary at beginning of new line}
@z
%---------------------------------------
@x [46] m.1030 l.19987 - Omega
hmode+no_boundary: begin get_x_token;
  if (cur_cmd=letter)or(cur_cmd=other_char)or(cur_cmd=char_given)or
   (cur_cmd=char_num) then cancel_boundary:=true;
  goto reswitch;
  end;
@y
hmode+no_boundary: begin get_x_token;
  if (cur_cmd=letter)or(cur_cmd=other_char)or(cur_cmd=char_given)or
   (cur_cmd=char_num) then cancel_boundary:=true;
  goto reswitch;
  end;
hmode+char_ghost: begin t:=cur_chr; get_x_token;
  if (cur_cmd=letter)or(cur_cmd=other_char)or(cur_cmd=char_given)or
   (cur_cmd=char_num) then begin
    if t=0 then new_left_ghost:=true
    else new_right_ghost:=true;
    end;
  goto reswitch;
  end;
@z
%---------------------------------------
@x [46] m.1032 l.20030 - Omega
  |font_bchar[cur_font]|,&otherwise;\cr}$$
except when |character(lig_stack)=font_false_bchar[cur_font]|.
@y
  |font_bchar(cur_font)|,&otherwise;\cr}$$
except when |character(lig_stack)=font_false_bchar(cur_font)|.
@z
%---------------------------------------
@x [46] m.1032 
@!cancel_boundary:boolean; {should the left boundary be ignored?}
@!ins_disc:boolean; {should we insert a discretionary node?}
@y
@!cancel_boundary:boolean; {should the left boundary be ignored?}
@!left_ghost:boolean; {character left of cursor is a left ghost}
@!right_ghost:boolean; {character left of cursor is a right ghost}
@!new_left_ghost:boolean; {character right of cursor is a left ghost}
@!new_right_ghost:boolean; {character right of cursor is a right ghost}
@!ins_disc:boolean; {should we insert a discretionary node?}
@!k_glue:integer;
@!font_glue_spec:pointer;
@z
%---------------------------------------
@x [46] m.1033 
ligature_present:=false; cancel_boundary:=false; lft_hit:=false; rt_hit:=false;
@y
ligature_present:=false; cancel_boundary:=false; lft_hit:=false; rt_hit:=false;
left_ghost:=false; right_ghost:=false;
new_left_ghost:=false; new_right_ghost:=false;
@z
%---------------------------------------
@x [46] m.1034 l.20075 - Omega
adjust_space_factor;@/
main_f:=cur_font;
bchar:=font_bchar[main_f]; false_bchar:=font_false_bchar[main_f];
if mode>0 then if language<>clang then fix_language;
fast_get_avail(lig_stack); font(lig_stack):=main_f; cur_l:=qi(cur_chr);
character(lig_stack):=cur_l;@/
cur_q:=tail;
if cancel_boundary then
  begin cancel_boundary:=false; main_k:=non_address;
  end
else main_k:=bchar_label[main_f];
if main_k=non_address then goto main_loop_move+2; {no left boundary processing}
cur_r:=cur_l; cur_l:=non_char;
goto main_lig_loop+1; {begin with cursor after left boundary}
@y
adjust_space_factor;@/
main_f:=cur_font;
bchar:=font_bchar(main_f); false_bchar:=font_false_bchar(main_f);
if mode>0 then if language<>clang then fix_language;
fast_get_avail(lig_stack); font(lig_stack):=main_f; cur_l:=qi(cur_chr);
character(lig_stack):=cur_l;@/
cur_q:=tail;
if cancel_boundary then
  begin cancel_boundary:=false; main_k:=non_address;
  left_ghost:=new_left_ghost; right_ghost:=new_right_ghost;
  new_left_ghost:=false; new_right_ghost:=false;
  end
else main_k:=bchar_label(main_f);
if main_k=non_address then goto main_loop_move+2; {no left boundary processing}
cur_r:=cur_l; cur_l:=non_char;
if not new_right_ghost then
  goto main_lig_loop+1; {begin with cursor after left boundary}
@z
%---------------------------------------
@x [46] m.1035 l.20122 - Omega
  begin if character(tail)=qi(hyphen_char[main_f]) then if link(cur_q)>null then
@y
  begin if character(tail)=qi(hyphen_char(main_f)) then if link(cur_q)>null then
@z
%---------------------------------------
@x [46] m.1036 l.20135 - Omega
if lig_stack=null then goto reswitch;
cur_q:=tail; cur_l:=character(lig_stack);
main_loop_move+1:if not is_char_node(lig_stack) then goto main_loop_move_lig;
main_loop_move+2:if(cur_chr<font_bc[main_f])or(cur_chr>font_ec[main_f]) then
  begin char_warning(main_f,cur_chr); free_avail(lig_stack); goto big_switch;
  end;
main_i:=char_info(main_f)(cur_l);
if not char_exists(main_i) then
  begin char_warning(main_f,cur_chr); free_avail(lig_stack); goto big_switch;
  end;
tail_append(lig_stack) {|main_loop_lookahead| is next}
@y
if lig_stack=null then goto reswitch;
cur_q:=tail; cur_l:=character(lig_stack);
main_loop_move+1:if not is_char_node(lig_stack) then goto main_loop_move_lig;
main_loop_move+2:
left_ghost:=new_left_ghost; right_ghost:=new_right_ghost;
new_left_ghost:=false; new_right_ghost:=false;
if(cur_chr<font_bc(main_f))or(cur_chr>font_ec(main_f)) then
  begin char_warning(main_f,cur_chr); free_avail(lig_stack); goto big_switch;
  end;
main_i:=char_info(main_f)(cur_l);
if not char_exists(main_i) then
  begin char_warning(main_f,cur_chr); free_avail(lig_stack); goto big_switch;
  end;
if not (left_ghost or right_ghost) then
  tail_append(lig_stack) {|main_loop_lookahead| is next}
@z
%---------------------------------------
@x [46] m.1038 l.20177 - Omega
if cur_cmd=no_boundary then bchar:=non_char;
@y
if cur_cmd=char_ghost then begin t:=cur_chr; get_x_token;
  if (cur_cmd=letter)or(cur_cmd=other_char)or(cur_cmd=char_given)or
   (cur_cmd=char_num) then begin
    if t=0 then new_left_ghost:=true
    else new_right_ghost:=true;
    back_input;
    goto main_loop_lookahead;
    end;
  end;
if cur_cmd=no_boundary then bchar:=non_char;
@z
%---------------------------------------
@x [46] m.1039 l.20193 - Omega
@<If there's a ligature/kern command...@>=
if char_tag(main_i)<>lig_tag then goto main_loop_wrapup;
main_k:=lig_kern_start(main_f)(main_i); main_j:=font_info[main_k].qqqq;
if skip_byte(main_j)<=stop_flag then goto main_lig_loop+2;
main_k:=lig_kern_restart(main_f)(main_j);
main_lig_loop+1:main_j:=font_info[main_k].qqqq;
main_lig_loop+2:if next_char(main_j)=cur_r then
 if skip_byte(main_j)<=stop_flag then
  @<Do ligature or kern command, returning to |main_lig_loop|
  or |main_loop_wrapup| or |main_loop_move|@>;
if skip_byte(main_j)=qi(0) then incr(main_k)
else begin if skip_byte(main_j)>=stop_flag then goto main_loop_wrapup;
  main_k:=main_k+qo(skip_byte(main_j))+1;
  end;
goto main_lig_loop+1
@y
@<If there's a ligature/kern command...@>=
if new_right_ghost or left_ghost then goto main_loop_wrapup;
if char_tag(main_i)<>lig_tag then goto main_loop_wrapup;
main_k:=lig_kern_start(main_f)(main_i);
main_j:=font_info(main_f)(main_k).qqqq;
if skip_byte(main_j)<=stop_flag then goto main_lig_loop+2;
main_k:=lig_kern_restart(main_f)(main_j);
main_lig_loop+1: main_j:=font_info(main_f)(main_k).qqqq;
main_lig_loop+2:if top_skip_byte(main_j)=0 then begin
    if next_char(main_f)(main_j)=cur_r then
      if skip_byte(main_j)<=stop_flag then
        @<Do ligature or kern command, returning to |main_lig_loop|
          or |main_loop_wrapup| or |main_loop_move|@>;
    end
  else begin
    if (font_bc(main_f)<=cur_r) then
      if (font_ec(main_f)>=cur_r) then
        if (char_exists(char_info(main_f)(cur_r))) then
          if cur_r<>bchar then
            if next_char(main_f)(main_j)=
               attr_zero_char_ivalue(main_f)(cur_r) then
              @<Do glue or penalty command@>;
    end;
if skip_byte(main_j)=qi(0) then incr(main_k)
else begin if skip_byte(main_j)>=stop_flag then goto main_loop_wrapup;
  main_k:=main_k+qo(skip_byte(main_j))+1;
  end;
goto main_lig_loop+1

@ There are currently three commands.  Command 17 will add a penalty
node between the two characters.  Command 18 will add a glue
node between the two characters.  Command 19 will add a penalty node,
then a glue node between the two characters.  Command 20
will add a kern nore between the two characters.

@<Do glue or penalty command@>=
begin
if new_left_ghost or right_ghost then goto main_loop_wrapup;
case op_byte(main_j) of
  qi(20): begin wrapup(rt_hit);
    tail_append(new_kern(attr_zero_kern(main_f)(rem_byte(main_j))));
    end;
  qi(18): begin wrapup(rt_hit);
    k_glue:=glues_base(main_f) + (rem_byte(main_j)*4);
    font_glue_spec:=new_spec(zero_glue);
    width(font_glue_spec):= font_info(main_f)(k_glue+1).sc;
    stretch(font_glue_spec):=font_info(main_f)(k_glue+2).sc;
    shrink(font_glue_spec):=font_info(main_f)(k_glue+3).sc;
    tail_append(new_glue(font_glue_spec));
    end;
  qi(19): begin wrapup(rt_hit);
    tail_append(new_penalty(attr_zero_penalty(main_f)(rem_top_byte(main_j))));
    k_glue:=glues_base(main_f) + (rem_bot_byte(main_j)*4);
    font_glue_spec:=new_spec(zero_glue);
    width(font_glue_spec):= font_info(main_f)(k_glue+1).sc;
    stretch(font_glue_spec):=font_info(main_f)(k_glue+2).sc;
    shrink(font_glue_spec):=font_info(main_f)(k_glue+3).sc;
    tail_append(new_glue(font_glue_spec));
    end;
  qi(17): begin wrapup(rt_hit);
    tail_append(new_penalty(attr_zero_penalty(main_f)(rem_byte(main_j))));
    end;
  end;
goto main_loop_move;
end
@z
%---------------------------------------
@x [46] m.1040 l.20219 - Omega
begin if op_byte(main_j)>=kern_flag then
  begin wrapup(rt_hit);
  tail_append(new_kern(char_kern(main_f)(main_j))); goto main_loop_move;
  end;
if cur_l=non_char then lft_hit:=true
else if lig_stack=null then rt_hit:=true;
check_interrupt; {allow a way out in case there's an infinite ligature loop}
case op_byte(main_j) of
qi(1),qi(5):begin cur_l:=rem_byte(main_j); {\.{=:\?}, \.{=:\?>}}
  main_i:=char_info(main_f)(cur_l); ligature_present:=true;
  end;
qi(2),qi(6):begin cur_r:=rem_byte(main_j); {\.{\?=:}, \.{\?=:>}}
  if lig_stack=null then {right boundary character is being consumed}
    begin lig_stack:=new_lig_item(cur_r); bchar:=non_char;
    end
  else if is_char_node(lig_stack) then {|link(lig_stack)=null|}
    begin main_p:=lig_stack; lig_stack:=new_lig_item(cur_r);
    lig_ptr(lig_stack):=main_p;
    end
  else character(lig_stack):=cur_r;
  end;
qi(3):begin cur_r:=rem_byte(main_j); {\.{\?=:\?}}
  main_p:=lig_stack; lig_stack:=new_lig_item(cur_r);
  link(lig_stack):=main_p;
  end;
qi(7),qi(11):begin wrapup(false); {\.{\?=:\?>}, \.{\?=:\?>>}}
  cur_q:=tail; cur_l:=rem_byte(main_j);
  main_i:=char_info(main_f)(cur_l); ligature_present:=true;
  end;
othercases begin cur_l:=rem_byte(main_j); ligature_present:=true; {\.{=:}}
  if lig_stack=null then goto main_loop_wrapup
  else goto main_loop_move+1;
  end
endcases;
if op_byte(main_j)>qi(4) then
  if op_byte(main_j)<>qi(7) then goto main_loop_wrapup;
if cur_l<non_char then goto main_lig_loop;
main_k:=bchar_label[main_f]; goto main_lig_loop+1;
end
@y
begin if op_byte(main_j)>=kern_flag then
  begin wrapup(rt_hit);
  tail_append(new_kern(char_kern(main_f)(main_j)));
  if new_left_ghost or right_ghost then begin
    subtype(tail):=explicit;
    end;
  goto main_loop_move;
  end;
if new_left_ghost or right_ghost then goto main_loop_wrapup;
if cur_l=non_char then lft_hit:=true
else if lig_stack=null then rt_hit:=true;
check_interrupt; {allow a way out in case there's an infinite ligature loop}
case op_byte(main_j) of
qi(1),qi(5):begin cur_l:=rem_char_byte(main_f)(main_j); {\.{=:\?}, \.{=:\?>}}
  main_i:=char_info(main_f)(cur_l); ligature_present:=true;
  end;
qi(2),qi(6):begin cur_r:=rem_char_byte(main_f)(main_j); {\.{\?=:}, \.{\?=:>}}
  if lig_stack=null then {right boundary character is being consumed}
    begin lig_stack:=new_lig_item(cur_r); bchar:=non_char;
    end
  else if is_char_node(lig_stack) then {|link(lig_stack)=null|}
    begin main_p:=lig_stack; lig_stack:=new_lig_item(cur_r);
    lig_ptr(lig_stack):=main_p;
    end
  else character(lig_stack):=cur_r;
  end;
qi(3):begin cur_r:=rem_char_byte(main_f)(main_j); {\.{\?=:\?}}
  main_p:=lig_stack; lig_stack:=new_lig_item(cur_r);
  link(lig_stack):=main_p;
  end;
qi(7),qi(11):begin wrapup(false); {\.{\?=:\?>}, \.{\?=:\?>>}}
  cur_q:=tail; cur_l:=rem_char_byte(main_f)(main_j);
  main_i:=char_info(main_f)(cur_l); ligature_present:=true;
  end;
othercases begin cur_l:=rem_char_byte(main_f)(main_j); 
                 ligature_present:=true; {\.{=:}}
  if lig_stack=null then goto main_loop_wrapup
  else goto main_loop_move+1;
  end
endcases;
if op_byte(main_j)>qi(4) then
  if op_byte(main_j)<>qi(7) then goto main_loop_wrapup;
if cur_l<non_char then goto main_lig_loop;
main_k:=bchar_label(main_f); goto main_lig_loop+1;
end
@z
%---------------------------------------
@x [46] m.1042 l.20285 - Omega
begin main_p:=font_glue[cur_font];
if main_p=null then
  begin main_p:=new_spec(zero_glue); main_k:=param_base[cur_font]+space_code;
  width(main_p):=font_info[main_k].sc; {that's |space(cur_font)|}
  stretch(main_p):=font_info[main_k+1].sc; {and |space_stretch(cur_font)|}
  shrink(main_p):=font_info[main_k+2].sc; {and |space_shrink(cur_font)|}
  font_glue[cur_font]:=main_p;
@y
begin main_p:=font_glue(cur_font);
if main_p=null then
  begin main_p:=new_spec(zero_glue); main_k:=param_base(cur_font)+space_code;
  width(main_p):=font_info(cur_font)(main_k).sc; {that's |space(cur_font)|}
  stretch(main_p):=font_info(cur_font)(main_k+1).sc;
     {and |space_stretch(cur_font)|}
  shrink(main_p):=font_info(cur_font)(main_k+2).sc;
     {and |space_shrink(cur_font)|}
  font_glue(cur_font):=main_p;
@z
%---------------------------------------
@x [47] m.1090
vmode+letter,vmode+other_char,vmode+char_num,vmode+char_given,
@y
vmode+letter,vmode+other_char,vmode+char_num,vmode+char_given,
vmode+char_ghost,
@z
%---------------------------------------
@x [48] m.1117 l.21343 - Omega
  begin c:=hyphen_char[cur_font];
@y
  begin c:=hyphen_char(cur_font);
@z
%---------------------------------------
@x [49] m.1195 l.22361 - Omega
if (font_params[fam_fnt(2+text_size)]<total_mathsy_params)or@|
   (font_params[fam_fnt(2+script_size)]<total_mathsy_params)or@|
   (font_params[fam_fnt(2+script_script_size)]<total_mathsy_params) then
  begin print_err("Math formula deleted: Insufficient symbol fonts");@/
@.Math formula deleted...@>
  help3("Sorry, but I can't typeset math unless \textfont 2")@/
    ("and \scriptfont 2 and \scriptscriptfont 2 have all")@/
    ("the \fontdimen values needed in math symbol fonts.");
  error; flush_math; danger:=true;
  end
else if (font_params[fam_fnt(3+text_size)]<total_mathex_params)or@|
   (font_params[fam_fnt(3+script_size)]<total_mathex_params)or@|
   (font_params[fam_fnt(3+script_script_size)]<total_mathex_params) then
@y
if (font_params(fam_fnt(2+text_size))<total_mathsy_params)or@|
   (font_params(fam_fnt(2+script_size))<total_mathsy_params)or@|
   (font_params(fam_fnt(2+script_script_size))<total_mathsy_params) then
  begin print_err("Math formula deleted: Insufficient symbol fonts");@/
@.Math formula deleted...@>
  help3("Sorry, but I can't typeset math unless \textfont 2")@/
    ("and \scriptfont 2 and \scriptscriptfont 2 have all")@/
    ("the \fontdimen values needed in math symbol fonts.");
  error; flush_math; danger:=true;
  end
else if (font_params(fam_fnt(3+text_size))<total_mathex_params)or@|
   (font_params(fam_fnt(3+script_size))<total_mathex_params)or@|
   (font_params(fam_fnt(3+script_script_size))<total_mathex_params) then
@z
%---------------------------------------
@x [49] m.1253 l.23250 - Omega
assign_font_dimen: begin find_font_dimen(true); k:=cur_val;
  scan_optional_equals; scan_normal_dimen; font_info[k].sc:=cur_val;
  end;
assign_font_int: begin n:=cur_chr; scan_font_ident; f:=cur_val;
  scan_optional_equals; scan_int;
  if n=0 then hyphen_char[f]:=cur_val@+else skew_char[f]:=cur_val;
@y
assign_font_dimen: begin find_font_dimen(true); k:=cur_val;
  scan_optional_equals; scan_normal_dimen;
  font_info(dimen_font)(k).sc:=cur_val;
  end;
assign_font_int: begin n:=cur_chr; scan_font_ident; f:=cur_val;
  scan_optional_equals; scan_int;
  if n=0 then hyphen_char(f):=cur_val@+else skew_char(f):=cur_val;
@z
%---------------------------------------
@x [49] m.1257 l.23269 - Omega
@!flushable_string:str_number; {string not yet referenced}
begin if job_name=0 then open_log_file;
  {avoid confusing \.{texput} with the font name}
@.texput@>
get_r_token; u:=cur_cs;
if u>=hash_base then t:=text(u)
@y
@!offset:integer;
@!flushable_string:str_number; {string not yet referenced}
begin if job_name=0 then open_log_file;
  {avoid confusing \.{texput} with the font name}
@.texput@>
get_r_token; u:=cur_cs;
if u>=hash_base then t:=newtext(u)
@z
%---------------------------------------
@x [49] m.1257 l.23290 - Omega
@<Scan the font size specification@>;
@<If this font has already been loaded, set |f| to the internal
  font number and |goto common_ending|@>;
f:=read_font_info(u,cur_name,cur_area,s);
common_ending: equiv(u):=f; eqtb[font_id_base+f]:=eqtb[u]; font_id_text(f):=t;
@y
@<Scan the font size specification@>;
name_in_progress:=true;
if scan_keyword("offset") then begin
  scan_int;
  offset:=cur_val;
  if (cur_val<0) then begin
    print_err("Illegal offset has been changed to 0");
    help1("The offset must be bigger than 0."); int_error(cur_val);
    offset:=0;
    end
  end
else offset:=0;
name_in_progress:=false;
@<If this font has already been loaded, set |f| to the internal
  font number and |goto common_ending|@>;
f:=read_font_info(u,cur_name,cur_area,s,offset);
common_ending: set_equiv(u,f);
set_new_eqtb(font_id_base+f,new_eqtb(u)); settext(font_id_base+f,t);
@z
%---------------------------------------
@x [49] m.1260 l.23333 - Omega
for f:=font_base+1 to font_ptr do
  if str_eq_str(font_name[f],cur_name)and str_eq_str(font_area[f],cur_area) then
    begin if cur_name=flushable_string then
      begin flush_string; cur_name:=font_name[f];
      end;
    if s>0 then
      begin if s=font_size[f] then goto common_ending;
      end
    else if font_size[f]=xn_over_d(font_dsize[f],-s,1000) then
      goto common_ending;
    end
@y
for f:=font_base+1 to font_ptr do begin
  if str_eq_str(font_name(f),cur_name) and
     str_eq_str(font_area(f),cur_area) then
    begin if cur_name=flushable_string then
      begin flush_string; cur_name:=font_name(f);
      end;
    if s>0 then
      begin if s=font_size(f) then goto common_ending;
      end
    else if font_size(f)=xn_over_d(font_dsize(f),-s,1000) then
      goto common_ending;
    end
  end
@z
%---------------------------------------
@x [49] m.1261 l.23345 - Omega
set_font:begin print("select font "); slow_print(font_name[chr_code]);
  if font_size[chr_code]<>font_dsize[chr_code] then
    begin print(" at "); print_scaled(font_size[chr_code]);
@y
set_font:begin print("select font "); slow_print(font_name(chr_code));
  if font_size(chr_code)<>font_dsize(chr_code) then
    begin print(" at "); print_scaled(font_size(chr_code));
@z
%---------------------------------------
@x [50] m.1320 l.23983 - Omega
@ @<Dump the font information@>=
dump_int(fmem_ptr);
for k:=0 to fmem_ptr-1 do dump_wd(font_info[k]);
dump_int(font_ptr);
for k:=null_font to font_ptr do
  @<Dump the array info for internal font number |k|@>;
print_ln; print_int(fmem_ptr-7); print(" words of font info for ");
print_int(font_ptr-font_base); print(" preloaded font");
if font_ptr<>font_base+1 then print_char("s")
@y
@ @<Dump the font information@>=
dump_int(font_ptr);
for k:=null_font to font_ptr do
  @<Dump the array info for internal font number |k|@>;
print_ln; print_int(font_ptr-font_base); print(" preloaded font");
if font_ptr<>font_base+1 then print_char("s")
@z
%---------------------------------------
@x [50] m.1321 l.23993 - Omega
@ @<Undump the font information@>=
undump_size(7)(font_mem_size)('font mem size')(fmem_ptr);
for k:=0 to fmem_ptr-1 do undump_wd(font_info[k]);
undump_size(font_base)(font_max)('font max')(font_ptr);
for k:=null_font to font_ptr do
  @<Undump the array info for internal font number |k|@>
@y
@ @<Undump the font information@>=
undump_size(font_base)(font_max)('font max')(font_ptr);
for k:=null_font to font_ptr do
  @<Undump the array info for internal font number |k|@>
@z
%---------------------------------------
@x [50] m.1322 l.24000 - Omega
@ @<Dump the array info for internal font number |k|@>=
begin dump_qqqq(font_check[k]);
dump_int(font_size[k]);   
dump_int(font_dsize[k]);
dump_int(font_params[k]);@/
dump_int(hyphen_char[k]); 
dump_int(skew_char[k]);@/
dump_int(font_name[k]); 
dump_int(font_area[k]);@/
dump_int(font_bc[k]);   
dump_int(font_ec[k]);@/
dump_int(char_base[k]);
dump_int(width_base[k]);
dump_int(height_base[k]);@/
dump_int(depth_base[k]);  
dump_int(italic_base[k]);
dump_int(lig_kern_base[k]);@/
dump_int(kern_base[k]);
dump_int(exten_base[k]);
dump_int(param_base[k]);@/
dump_int(font_glue[k]);@/
dump_int(bchar_label[k]);
dump_int(font_bchar[k]);
dump_int(font_false_bchar[k]);@/
print_nl("\font"); print_esc(font_id_text(k)); print_char("=");
print_file_name(font_name[k],font_area[k],"");
if font_size[k]<>font_dsize[k] then
  begin print(" at "); print_scaled(font_size[k]); print("pt");
  end;
end
@y
@ @<Dump the array info for internal font number |k|@>=
begin dump_font_table(k,param_base(k)+font_params(k)+1);
print_nl("\font"); print_esc(font_id_text(k)); print_char("=");
print_file_name(font_name(k),font_area(k),"");
if font_size(k)<>font_dsize(k) then
  begin print(" at "); print_scaled(font_size(k)); print("pt");
  end;
end
@z
%---------------------------------------
@x [50] m.1323 l.24031 - Omega
@ @<Undump the array info for internal font number |k|@>=
begin undump_qqqq(font_check[k]);@/
undump_int(font_size[k]);
undump_int(font_dsize[k]);
undump(min_halfword)(max_halfword)(font_params[k]);@/
undump_int(hyphen_char[k]);
undump_int(skew_char[k]);@/
undump(0)(str_ptr)(font_name[k]);
undump(0)(str_ptr)(font_area[k]);@/
undump(0)(255)(font_bc[k]);
undump(0)(255)(font_ec[k]);@/
undump_int(char_base[k]);
undump_int(width_base[k]);
undump_int(height_base[k]);@/
undump_int(depth_base[k]);
undump_int(italic_base[k]);
undump_int(lig_kern_base[k]);@/
undump_int(kern_base[k]);
undump_int(exten_base[k]);
undump_int(param_base[k]);@/
undump(min_halfword)(lo_mem_max)(font_glue[k]);@/
undump(0)(fmem_ptr-1)(bchar_label[k]);
undump(min_quarterword)(non_char)(font_bchar[k]);
undump(min_quarterword)(non_char)(font_false_bchar[k]);
end
@y
@ @<Undump the array info for internal font number |k|@>=
begin undump_font_table(k);@/
end
@z
%---------------------------------------
@x [51] m.1334 l.24277 - Omega
  wlog(' ',fmem_ptr:1,' words of font info for ',
    font_ptr-font_base:1,' font');
  if font_ptr<>font_base+1 then wlog('s');
@y
  wlog(font_ptr-font_base:1,' font');
  if font_ptr<>font_base+1 then wlog('s');
@z
%---------------------------------------
@x [52] m.1339 l.24429 - Omega
4: print_word(eqtb[n]);
5: print_word(font_info[n]);
@y
4: print_word(new_eqtb(n));
5:  ;
@z
%---------------------------------------
