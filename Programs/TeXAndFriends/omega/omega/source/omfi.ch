% omfi.ch: Primitives for extra level of infinity.
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
%---------------------------------------
@x [10] m.135 l.2878 - Omega fi order of infinity
specifies the order of infinity to which glue setting applies (|normal|,
|fil|, |fill|, or |filll|). The |subtype| field is not used.
@y
specifies the order of infinity to which glue setting applies (|normal|,
|sfi|, |fil|, |fill|, or |filll|). The |subtype| field is not used.
@z
%---------------------------------------
@x [10] m.150 l.3136 - Omega fi order of infinity
orders of infinity (|normal|, |fil|, |fill|, or |filll|)
@y
orders of infinity (|normal|, |sfi|, |fil|, |fill|, or |filll|)
@z
%---------------------------------------
@x [10] m.150 l.3145 - Omega fi order of infinity
@d fil=1 {first-order infinity}
@d fill=2 {second-order infinity}
@d filll=3 {third-order infinity}
@y
@d sfi=1 {first-order infinity}
@d fil=2 {second-order infinity}
@d fill=3 {third-order infinity}
@d filll=4 {fourth-order infinity}
@z
%---------------------------------------
@x [10] m.150 l.3150 - Omega fi order of infinity
@!glue_ord=normal..filll; {infinity to the 0, 1, 2, or 3 power}
@y
@!glue_ord=normal..filll; {infinity to the 0, 1, 2, 3, or 4 power}
@z
%---------------------------------------
@x [11] m.162 l.3296 - Omega fi order of infinity
@d fil_glue==zero_glue+glue_spec_size {\.{0pt plus 1fil minus 0pt}}
@y
@d sfi_glue==zero_glue+glue_spec_size {\.{0pt plus 1fi minus 0pt}}
@d fil_glue==sfi_glue+glue_spec_size {\.{0pt plus 1fil minus 0pt}}
@z
%---------------------------------------
@x [11] m.164 l.3296 - Omega fi order of infinity
stretch(fil_glue):=unity; stretch_order(fil_glue):=fil;@/
@y
stretch(sfi_glue):=unity; stretch_order(sfi_glue):=sfi;@/
stretch(fil_glue):=unity; stretch_order(fil_glue):=fil;@/
@z
%---------------------------------------
@x [12] m.177 l.3591 - Omega fi order of infinity
  begin print("fil");
  while order>fil do
@y
  begin print("fi");
  while order>sfi do
@z
%---------------------------------------
@x [26] m.454 l.8924 - Omega fi order of infinity
if scan_keyword("fil") then
@.fil@>
  begin cur_order:=fil;
@y
if scan_keyword("fi") then
@.fil@>
  begin cur_order:=sfi;
@z
%---------------------------------------
@x [33] m.650 l.12877 - Omega fi order of infinity
total_stretch[fil]:=0; total_shrink[fil]:=0;
@y
total_stretch[sfi]:=0; total_shrink[sfi]:=0;
total_stretch[fil]:=0; total_shrink[fil]:=0;
@z
%---------------------------------------
@x [33] m.659 l.12996 - Omega fi order of infinity
else if total_stretch[fil]<>0 then o:=fil
@y
else if total_stretch[fil]<>0 then o:=fil
else if total_stretch[sfi]<>0 then o:=sfi
@z
%---------------------------------------
@x [33] m.665 l.13061 - Omega fi order of infinity
else if total_shrink[fil]<>0 then o:=fil
@y
else if total_shrink[fil]<>0 then o:=fil
else if total_shrink[sfi]<>0 then o:=sfi
@z
%---------------------------------------
@x [38] m.822 l.16135 - Omega fi order of infinity
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
%---------------------------------------
@x [38] m.823 l.16144 - Omega fi order of infinity
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
%---------------------------------------
@x [38] m.823 l.16154 - Omega fi order of infinity
k:=1 to 6 do cur_active_width[k]:=cur_active_width[k]+mem[q+k].sc|};$$ and we
want to do this without the overhead of |for| loops. The |do_all_six|
macro makes such six-tuples convenient.

@d do_all_six(#)==#(1);#(2);#(3);#(4);#(5);#(6)

@<Glo...@>=
@!active_width:array[1..6] of scaled;
  {distance from first active node to~|cur_p|}
@!cur_active_width:array[1..6] of scaled; {distance from current active node}
@!background:array[1..6] of scaled; {length of an ``empty'' line}
@!break_width:array[1..6] of scaled; {length being computed after current break}
@y
k:=1 to 7 do cur_active_width[k]:=cur_active_width[k]+mem[q+k].sc|};$$ and we
want to do this without the overhead of |for| loops. The |do_all_six|
macro makes such six-tuples convenient.

@d do_all_six(#)==#(1);#(2);#(3);#(4);#(5);#(6);#(7)

@<Glo...@>=
@!active_width:array[1..7] of scaled;
  {distance from first active node to~|cur_p|}
@!cur_active_width:array[1..7] of scaled; {distance from current active node}
@!background:array[1..7] of scaled; {length of an ``empty'' line}
@!break_width:array[1..7] of scaled; {length being computed after current break}
@z
%---------------------------------------
@x [38] m.827 l.16242 - Omega fi order of infinity
background[2]:=0; background[3]:=0; background[4]:=0; background[5]:=0;@/
@y
background[2]:=0; background[3]:=0; background[4]:=0; background[5]:=0;@/
background[6]:=0;@/
@z
%---------------------------------------
@x [38] m.827 l.16260 - Omega fi order of infinity
background[6]:=shrink(q)+shrink(r);
@y
background[7]:=shrink(q)+shrink(r);
@z
%---------------------------------------
@x [38] m.838 l.16470 - Omega fi order of infinity
break_width[6]:=break_width[6]-shrink(v);
@y
break_width[7]:=break_width[7]-shrink(v);
@z
%---------------------------------------
@x [38] m.852 l.16713 - Omega fi order of infinity
subarray |cur_active_width[2..5]|, in units of points, fil, fill, and filll.
@y
subarray |cur_active_width[2..6]|, in units of points, sfi, fil, fill and filll.
@z
%---------------------------------------
@x [38] m.852 l.16722 - Omega fi order of infinity
if (cur_active_width[3]<>0)or(cur_active_width[4]<>0)or@|
  (cur_active_width[5]<>0) then
@y
if (cur_active_width[3]<>0)or(cur_active_width[4]<>0)or@|
  (cur_active_width[5]<>0)or(cur_active_width[6]<>0) then
@z
%---------------------------------------
@x [38] m.853 l.16738 - Omega fi order of infinity
we can shrink the line from |r| to |cur_p| by at most |cur_active_width[6]|.
 
@<Set the value of |b| to the badness for shrinking...@>=
begin if -shortfall>cur_active_width[6] then b:=inf_bad+1
else b:=badness(-shortfall,cur_active_width[6]);
@y
we can shrink the line from |r| to |cur_p| by at most |cur_active_width[7]|.
 
@<Set the value of |b| to the badness for shrinking...@>=
begin if -shortfall>cur_active_width[7] then b:=inf_bad+1
else b:=badness(-shortfall,cur_active_width[7]);
@z
%---------------------------------------
@x [39] m.868 l.17054 - Omega fi order of infinity
active_width[6]:=active_width[6]+shrink(q)
@y
active_width[7]:=active_width[7]+shrink(q)
@z
%---------------------------------------
@x [44] m.975 l.18932 - Omega fi order of infinity
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
%---------------------------------------
@x [44] m.976 l.18947 - Omega fi order of infinity
  active_height[6]:=active_height[6]+shrink(q);
@y
  active_height[7]:=active_height[7]+shrink(q);
@z
%---------------------------------------
@x [48] m.1201 l.22454 - Omega fi order of infinity
   (total_shrink[fil]<>0)or(total_shrink[fill]<>0)or
   (total_shrink[filll]<>0)) then
@y
   (total_shrink[sfi]<>0)or(total_shrink[fil]<>0)or
   (total_shrink[fill]<>0)or(total_shrink[filll]<>0)) then
@z

