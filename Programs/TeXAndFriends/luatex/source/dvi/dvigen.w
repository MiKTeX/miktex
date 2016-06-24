% dvigen.w
%
% Copyright 2009-2013 Taco Hoekwater <taco@@luatex.org>
%
% This file is part of LuaTeX.
%
% LuaTeX is free software; you can redistribute it and/or modify it under
% the terms of the GNU General Public License as published by the Free
% Software Foundation; either version 2 of the License, or (at your
% option) any later version.
%
% LuaTeX is distributed in the hope that it will be useful, but WITHOUT
% ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
% FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
% License for more details.
%
% You should have received a copy of the GNU General Public License along
% with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

\def\MF{MetaFont}
\def\MP{MetaPost}
\def\PASCAL{Pascal}
\def\[#1]{#1}
\pdfoutput=1
\pdfmapline{cmtex10 < cmtex10.pfb}
\pdfmapfile{pdftex.map}

\title{: generation of DVI output}

@ Initial identification of this file, and the needed headers.
@c
#include "ptexlib.h"

@ Here is the start of the actual C file.
@c
#undef write_dvi

/* todo: move macros to api */

#define mode cur_list.mode_field        /* current mode */

#define mag int_par(mag_code)
#define tracing_output int_par(tracing_output_code)
#define tracing_stats int_par(tracing_stats_code)
#define tracing_online int_par(tracing_online_code)
#define page_width dimen_par(page_width_code)
#define page_height dimen_par(page_height_code)
#define page_left_offset dimen_par(page_left_offset_code)
#define page_right_offset dimen_par(page_right_offset_code)
#define page_top_offset dimen_par(page_top_offset_code)
#define page_bottom_offset dimen_par(page_bottom_offset_code)
#define h_offset dimen_par(h_offset_code)
#define v_offset dimen_par(v_offset_code)

#define count(A) eqtb[count_base+(A)].cint

@ The most important output produced by a run of \TeX\ is the ``device
independent'' (\.{DVI}) file that specifies where characters and rules
are to appear on printed pages. The form of these files was designed by
David R. Fuchs in 1979. Almost any reasonable typesetting device can be
@^Fuchs, David Raymond@>
@:DVI_files}{\.{DVI} files@>
driven by a program that takes \.{DVI} files as input, and dozens of such
\.{DVI}-to-whatever programs have been written. Thus, it is possible to
print the output of \TeX\ on many different kinds of equipment, using \TeX\
as a device-independent ``front end.''

A \.{DVI} file is a stream of 8-bit bytes, which may be regarded as a
series of commands in a machine-like language. The first byte of each command
is the operation code, and this code is followed by zero or more bytes
that provide parameters to the command. The parameters themselves may consist
of several consecutive bytes; for example, the `|set_rule|' command has two
parameters, each of which is four bytes long. Parameters are usually
regarded as nonnegative integers; but four-byte-long parameters,
and shorter parameters that denote distances, can be
either positive or negative. Such parameters are given in two's complement
notation. For example, a two-byte-long distance parameter has a value between
$-2^{15}$ and $2^{15}-1$. As in \.{TFM} files, numbers that occupy
more than one byte position appear in BigEndian order.

A \.{DVI} file consists of a ``preamble,'' followed by a sequence of one
or more ``pages,'' followed by a ``postamble.'' The preamble is simply a
|pre| command, with its parameters that define the dimensions used in the
file; this must come first.  Each ``page'' consists of a |bop| command,
followed by any number of other commands that tell where characters are to
be placed on a physical page, followed by an |eop| command. The pages
appear in the order that \TeX\ generated them. If we ignore |nop| commands
and \\{fnt\_def} commands (which are allowed between any two commands in
the file), each |eop| command is immediately followed by a |bop| command,
or by a |post| command; in the latter case, there are no more pages in the
file, and the remaining bytes form the postamble.  Further details about
the postamble will be explained later.

Some parameters in \.{DVI} commands are ``pointers.'' These are four-byte
quantities that give the location number of some other byte in the file;
the first byte is number~0, then comes number~1, and so on. For example,
one of the parameters of a |bop| command points to the previous |bop|;
this makes it feasible to read the pages in backwards order, in case the
results are being directed to a device that stacks its output face up.
Suppose the preamble of a \.{DVI} file occupies bytes 0 to 99. Now if the
first page occupies bytes 100 to 999, say, and if the second
page occupies bytes 1000 to 1999, then the |bop| that starts in byte 1000
points to 100 and the |bop| that starts in byte 2000 points to 1000. (The
very first |bop|, i.e., the one starting in byte 100, has a pointer of~$-1$.)

@ The \.{DVI} format is intended to be both compact and easily interpreted
by a machine. Compactness is achieved by making most of the information
implicit instead of explicit. When a \.{DVI}-reading program reads the
commands for a page, it keeps track of several quantities: (a)~The current
font |f| is an integer; this value is changed only
by \\{fnt} and \\{fnt\_num} commands. (b)~The current position on the page
is given by two numbers called the horizontal and vertical coordinates,
|h| and |v|. Both coordinates are zero at the upper left corner of the page;
moving to the right corresponds to increasing the horizontal coordinate, and
moving down corresponds to increasing the vertical coordinate. Thus, the
coordinates are essentially Cartesian, except that vertical directions are
flipped; the Cartesian version of |(h,v)| would be |(h,-v)|.  (c)~The
current spacing amounts are given by four numbers |w|, |x|, |y|, and |z|,
where |w| and~|x| are used for horizontal spacing and where |y| and~|z|
are used for vertical spacing. (d)~There is a stack containing
|(h,v,w,x,y,z)| values; the \.{DVI} commands |push| and |pop| are used to
change the current level of operation. Note that the current font~|f| is
not pushed and popped; the stack contains only information about
positioning.

The values of |h|, |v|, |w|, |x|, |y|, and |z| are signed integers having up
to 32 bits, including the sign. Since they represent physical distances,
there is a small unit of measurement such that increasing |h| by~1 means
moving a certain tiny distance to the right. The actual unit of
measurement is variable, as explained below; \TeX\ sets things up so that
its \.{DVI} output is in sp units, i.e., scaled points, in agreement with
all the |scaled| dimensions in \TeX's data structures.

@ Here is a list of all the commands that may appear in a \.{DVI} file. Each
command is specified by its symbolic name (e.g., |bop|), its opcode byte
(e.g., 139), and its parameters (if any). The parameters are followed
by a bracketed number telling how many bytes they occupy; for example,
`|p[4]|' means that parameter |p| is four bytes long.

\yskip\hang|set_char_0| 0. Typeset character number~0 from font~|f|
such that the reference point of the character is at |(h,v)|. Then
increase |h| by the width of that character. Note that a character may
have zero or negative width, so one cannot be sure that |h| will advance
after this command; but |h| usually does increase.

\yskip\hang\\{set\_char\_1} through \\{set\_char\_127} (opcodes 1 to 127).
Do the operations of |set_char_0|; but use the character whose number
matches the opcode, instead of character~0.

\yskip\hang|set1| 128 |c[1]|. Same as |set_char_0|, except that character
number~|c| is typeset. \TeX82 uses this command for characters in the
range |128<=c<256|.

\yskip\hang|@!set2| 129 |c[2]|. Same as |set1|, except that |c|~is two
bytes long, so it is in the range |0<=c<65536|. \TeX82 never uses this
command, but it should come in handy for extensions of \TeX\ that deal
with oriental languages.
@^oriental characters@>@^Chinese characters@>@^Japanese characters@>

\yskip\hang|@!set3| 130 |c[3]|. Same as |set1|, except that |c|~is three
bytes long, so it can be as large as $2^{24}-1$. Not even the Chinese
language has this many characters, but this command might prove useful
in some yet unforeseen extension.

\yskip\hang|@!set4| 131 |c[4]|. Same as |set1|, except that |c|~is four
bytes long. Imagine that.

\yskip\hang|set_rule| 132 |a[4]| |b[4]|. Typeset a solid black rectangle
of height~|a| and width~|b|, with its bottom left corner at |(h,v)|. Then
set |h:=h+b|. If either |a<=0| or |b<=0|, nothing should be typeset. Note
that if |b<0|, the value of |h| will decrease even though nothing else happens.
See below for details about how to typeset rules so that consistency with
\MF\ is guaranteed.

\yskip\hang|@!put1| 133 |c[1]|. Typeset character number~|c| from font~|f|
such that the reference point of the character is at |(h,v)|. (The `put'
commands are exactly like the `set' commands, except that they simply put out a
character or a rule without moving the reference point afterwards.)

\yskip\hang|@!put2| 134 |c[2]|. Same as |set2|, except that |h| is not changed.

\yskip\hang|@!put3| 135 |c[3]|. Same as |set3|, except that |h| is not changed.

\yskip\hang|@!put4| 136 |c[4]|. Same as |set4|, except that |h| is not changed.

\yskip\hang|put_rule| 137 |a[4]| |b[4]|. Same as |set_rule|, except that
|h| is not changed.

\yskip\hang|nop| 138. No operation, do nothing. Any number of |nop|'s
may occur between \.{DVI} commands, but a |nop| cannot be inserted between
a command and its parameters or between two parameters.

\yskip\hang|bop| 139 $c_0[4]$ $c_1[4]$ $\ldots$ $c_9[4]$ $p[4]$. Beginning
of a page: Set |(h,v,w,x,y,z):=(0,0,0,0,0,0)| and set the stack empty. Set
the current font |f| to an undefined value.  The ten $c_i$ parameters hold
the values of \.{\\count0} $\ldots$ \.{\\count9} in \TeX\ at the time
\.{\\shipout} was invoked for this page; they can be used to identify
pages, if a user wants to print only part of a \.{DVI} file. The parameter
|p| points to the previous |bop| in the file; the first
|bop| has $p=-1$.

\yskip\hang|eop| 140.  End of page: Print what you have read since the
previous |bop|. At this point the stack should be empty. (The \.{DVI}-reading
programs that drive most output devices will have kept a buffer of the
material that appears on the page that has just ended. This material is
largely, but not entirely, in order by |v| coordinate and (for fixed |v|) by
|h|~coordinate; so it usually needs to be sorted into some order that is
appropriate for the device in question.)

\yskip\hang|push| 141. Push the current values of |(h,v,w,x,y,z)| onto the
top of the stack; do not change any of these values. Note that |f| is
not pushed.

\yskip\hang|pop| 142. Pop the top six values off of the stack and assign
them respectively to |(h,v,w,x,y,z)|. The number of pops should never
exceed the number of pushes, since it would be highly embarrassing if the
stack were empty at the time of a |pop| command.

\yskip\hang|right1| 143 |b[1]|. Set |h:=h+b|, i.e., move right |b| units.
The parameter is a signed number in two's complement notation, |-128<=b<128|;
if |b<0|, the reference point moves left.

\yskip\hang|right2| 144 |b[2]|. Same as |right1|, except that |b| is a
two-byte quantity in the range |-32768<=b<32768|.

\yskip\hang|right3| 145 |b[3]|. Same as |right1|, except that |b| is a
three-byte quantity in the range |@t$-2^{23}$@><=b<@t$2^{23}$@>|.

\yskip\hang|right4| 146 |b[4]|. Same as |right1|, except that |b| is a
four-byte quantity in the range |@t$-2^{31}$@><=b<@t$2^{31}$@>|.

\yskip\hang|w0| 147. Set |h:=h+w|; i.e., move right |w| units. With luck,
this parameterless command will usually suffice, because the same kind of motion
will occur several times in succession; the following commands explain how
|w| gets particular values.

\yskip\hang|w1| 148 |b[1]|. Set |w:=b| and |h:=h+b|. The value of |b| is a
signed quantity in two's complement notation, |-128<=b<128|. This command
changes the current |w|~spacing and moves right by |b|.

\yskip\hang|@!w2| 149 |b[2]|. Same as |w1|, but |b| is two bytes long,
|-32768<=b<32768|.

\yskip\hang|@!w3| 150 |b[3]|. Same as |w1|, but |b| is three bytes long,
|@t$-2^{23}$@><=b<@t$2^{23}$@>|.

\yskip\hang|@!w4| 151 |b[4]|. Same as |w1|, but |b| is four bytes long,
|@t$-2^{31}$@><=b<@t$2^{31}$@>|.

\yskip\hang|x0| 152. Set |h:=h+x|; i.e., move right |x| units. The `|x|'
commands are like the `|w|' commands except that they involve |x| instead
of |w|.

\yskip\hang|x1| 153 |b[1]|. Set |x:=b| and |h:=h+b|. The value of |b| is a
signed quantity in two's complement notation, |-128<=b<128|. This command
changes the current |x|~spacing and moves right by |b|.

\yskip\hang|@!x2| 154 |b[2]|. Same as |x1|, but |b| is two bytes long,
|-32768<=b<32768|.

\yskip\hang|@!x3| 155 |b[3]|. Same as |x1|, but |b| is three bytes long,
|@t$-2^{23}$@><=b<@t$2^{23}$@>|.

\yskip\hang|@!x4| 156 |b[4]|. Same as |x1|, but |b| is four bytes long,
|@t$-2^{31}$@><=b<@t$2^{31}$@>|.

\yskip\hang|down1| 157 |a[1]|. Set |v:=v+a|, i.e., move down |a| units.
The parameter is a signed number in two's complement notation, |-128<=a<128|;
if |a<0|, the reference point moves up.

\yskip\hang|@!down2| 158 |a[2]|. Same as |down1|, except that |a| is a
two-byte quantity in the range |-32768<=a<32768|.

\yskip\hang|@!down3| 159 |a[3]|. Same as |down1|, except that |a| is a
three-byte quantity in the range |@t$-2^{23}$@><=a<@t$2^{23}$@>|.

\yskip\hang|@!down4| 160 |a[4]|. Same as |down1|, except that |a| is a
four-byte quantity in the range |@t$-2^{31}$@><=a<@t$2^{31}$@>|.

\yskip\hang|y0| 161. Set |v:=v+y|; i.e., move down |y| units. With luck,
this parameterless command will usually suffice, because the same kind of motion
will occur several times in succession; the following commands explain how
|y| gets particular values.

\yskip\hang|y1| 162 |a[1]|. Set |y:=a| and |v:=v+a|. The value of |a| is a
signed quantity in two's complement notation, |-128<=a<128|. This command
changes the current |y|~spacing and moves down by |a|.

\yskip\hang|@!y2| 163 |a[2]|. Same as |y1|, but |a| is two bytes long,
|-32768<=a<32768|.

\yskip\hang|@!y3| 164 |a[3]|. Same as |y1|, but |a| is three bytes long,
|@t$-2^{23}$@><=a<@t$2^{23}$@>|.

\yskip\hang|@!y4| 165 |a[4]|. Same as |y1|, but |a| is four bytes long,
|@t$-2^{31}$@><=a<@t$2^{31}$@>|.

\yskip\hang|z0| 166. Set |v:=v+z|; i.e., move down |z| units. The `|z|' commands
are like the `|y|' commands except that they involve |z| instead of |y|.

\yskip\hang|z1| 167 |a[1]|. Set |z:=a| and |v:=v+a|. The value of |a| is a
signed quantity in two's complement notation, |-128<=a<128|. This command
changes the current |z|~spacing and moves down by |a|.

\yskip\hang|@!z2| 168 |a[2]|. Same as |z1|, but |a| is two bytes long,
|-32768<=a<32768|.

\yskip\hang|@!z3| 169 |a[3]|. Same as |z1|, but |a| is three bytes long,
|@t$-2^{23}$@><=a<@t$2^{23}$@>|.

\yskip\hang|@!z4| 170 |a[4]|. Same as |z1|, but |a| is four bytes long,
|@t$-2^{31}$@><=a<@t$2^{31}$@>|.

\yskip\hang|fnt_num_0| 171. Set |f:=0|. Font 0 must previously have been
defined by a \\{fnt\_def} instruction, as explained below.

\yskip\hang\\{fnt\_num\_1} through \\{fnt\_num\_63} (opcodes 172 to 234). Set
|f:=1|, \dots, \hbox{|f:=63|}, respectively.

\yskip\hang|fnt1| 235 |k[1]|. Set |f:=k|. \TeX82 uses this command for font
numbers in the range |64<=k<256|.

\yskip\hang|@!fnt2| 236 |k[2]|. Same as |fnt1|, except that |k|~is two
bytes long, so it is in the range |0<=k<65536|. \TeX82 never generates this
command, but large font numbers may prove useful for specifications of
color or texture, or they may be used for special fonts that have fixed
numbers in some external coding scheme.

\yskip\hang|@!fnt3| 237 |k[3]|. Same as |fnt1|, except that |k|~is three
bytes long, so it can be as large as $2^{24}-1$.

\yskip\hang|@!fnt4| 238 |k[4]|. Same as |fnt1|, except that |k|~is four
bytes long; this is for the really big font numbers (and for the negative ones).

\yskip\hang|xxx1| 239 |k[1]| |x[k]|. This command is undefined in
general; it functions as a $(k+2)$-byte |nop| unless special \.{DVI}-reading
programs are being used. \TeX82 generates |xxx1| when a short enough
\.{\\special} appears, setting |k| to the number of bytes being sent. It
is recommended that |x| be a string having the form of a keyword followed
by possible parameters relevant to that keyword.

\yskip\hang|@!xxx2| 240 |k[2]| |x[k]|. Like |xxx1|, but |0<=k<65536|.

\yskip\hang|@!xxx3| 241 |k[3]| |x[k]|. Like |xxx1|, but |0<=k<@t$2^{24}$@>|.

\yskip\hang|xxx4| 242 |k[4]| |x[k]|. Like |xxx1|, but |k| can be ridiculously
large. \TeX82 uses |xxx4| when sending a string of length 256 or more.

\yskip\hang|fnt_def1| 243 |k[1]| |c[4]| |s[4]| |d[4]| |a[1]| |l[1]| |n[a+l]|.
Define font |k|, where |0<=k<256|; font definitions will be explained shortly.

\yskip\hang|@!fnt_def2| 244 |k[2]| |c[4]| |s[4]| |d[4]| |a[1]| |l[1]| |n[a+l]|.
Define font |k|, where |0<=k<65536|.

\yskip\hang|@!fnt_def3| 245 |k[3]| |c[4]| |s[4]| |d[4]| |a[1]| |l[1]| |n[a+l]|.
Define font |k|, where |0<=k<@t$2^{24}$@>|.

\yskip\hang|@!fnt_def4| 246 |k[4]| |c[4]| |s[4]| |d[4]| |a[1]| |l[1]| |n[a+l]|.
Define font |k|, where |@t$-2^{31}$@><=k<@t$2^{31}$@>|.

\yskip\hang|pre| 247 |i[1]| |num[4]| |den[4]| |mag[4]| |k[1]| |x[k]|.
Beginning of the preamble; this must come at the very beginning of the
file. Parameters |i|, |num|, |den|, |mag|, |k|, and |x| are explained below.

\yskip\hang|post| 248. Beginning of the postamble, see below.

\yskip\hang|post_post| 249. Ending of the postamble, see below.

\yskip\noindent Commands 250--255 are undefined at the present time.

@c
#define set_char_0  0           /* typeset character 0 and move right */
#define set1  128               /* typeset a character and move right */
#define set_rule  132           /* typeset a rule and move right */
#define put1    133             /* typeset a character without moving */
#define put_rule  137           /* typeset a rule */
#define nop  138                /* no operation */
#define bop  139                /* beginning of page */
#define eop  140                /* ending of page */
#define push  141               /* save the current positions */
#define pop  142                /* restore previous positions */
#define right1    143           /* move right */
#define right4    146           /* move right, 4 bytes */
#define w0  147                 /* move right by |w| */
#define w1  148                 /* move right and set |w| */
#define x0  152                 /* move right by |x| */
#define x1  153                 /* move right and set |x| */
#define down1  157              /* move down */
#define down4  160              /* move down, 4 bytes */
#define y0  161                 /* move down by |y| */
#define y1  162                 /* move down and set |y| */
#define z0  166                 /* move down by |z| */
#define z1  167                 /* move down and set |z| */
#define fnt_num_0  171          /* set current font to 0 */
#define fnt1  235               /* set current font */
#define xxx1  239               /* extension to \.{DVI} primitives */
#define xxx4  242               /* potentially long extension to \.{DVI} primitives */
#define fnt_def1  243           /* define the meaning of a font number */
#define pre  247                /* preamble */
#define post  248               /* postamble beginning */
#define post_post  249          /* postamble ending */

@ The preamble contains basic information about the file as a whole. As
stated above, there are six parameters:
$$\hbox{|@!i[1]| |@!num[4]| |@!den[4]| |@!mag[4]| |@!k[1]| |@!x[k]|.}$$
The |i| byte identifies \.{DVI} format; currently this byte is always set
to~2. (The value |i=3| is currently used for an extended format that
allows a mixture of right-to-left and left-to-right typesetting.
Some day we will set |i=4|, when \.{DVI} format makes another
incompatible change---perhaps in the year 2048.)

The next two parameters, |num| and |den|, are positive integers that define
the units of measurement; they are the numerator and denominator of a
fraction by which all dimensions in the \.{DVI} file could be multiplied
in order to get lengths in units of $10^{-7}$ meters. Since $\rm 7227{pt} =
254{cm}$, and since \TeX\ works with scaled points where there are $2^{16}$
sp in a point, \TeX\ sets
$|num|/|den|=(254\cdot10^5)/(7227\cdot2^{16})=25400000/473628672$.
@^sp@>

The |mag| parameter is what \TeX\ calls \.{\\mag}, i.e., 1000 times the
desired magnification. The actual fraction by which dimensions are
multiplied is therefore $|mag|\cdot|num|/1000|den|$. Note that if a \TeX\
source document does not call for any `\.{true}' dimensions, and if you
change it only by specifying a different \.{\\mag} setting, the \.{DVI}
file that \TeX\ creates will be completely unchanged except for the value
of |mag| in the preamble and postamble. (Fancy \.{DVI}-reading programs allow
users to override the |mag|~setting when a \.{DVI} file is being printed.)

Finally, |k| and |x| allow the \.{DVI} writer to include a comment, which is not
interpreted further. The length of comment |x| is |k|, where |0<=k<256|.


@c
#define id_byte 2               /* identifies the kind of \.{DVI} files described here */

@ Font definitions for a given font number |k| contain further parameters
$$\hbox{|c[4]| |s[4]| |d[4]| |a[1]| |l[1]| |n[a+l]|.}$$
The four-byte value |c| is the check sum that \TeX\ found in the \.{TFM}
file for this font; |c| should match the check sum of the font found by
programs that read this \.{DVI} file.
@^check sum@>

Parameter |s| contains a fixed-point scale factor that is applied to
the character widths in font |k|; font dimensions in \.{TFM} files and
other font files are relative to this quantity, which is called the
``at size'' elsewhere in this documentation. The value of |s| is
always positive and less than $2^{27}$. It is given in the same units
as the other \.{DVI} dimensions, i.e., in sp when \TeX82 has made the
file.  Parameter |d| is similar to |s|; it is the ``design size,'' and
(like~|s|) it is given in \.{DVI} units. Thus, font |k| is to be used
at $|mag|\cdot s/1000d$ times its normal size.

The remaining part of a font definition gives the external name of the font,
which is an ASCII string of length |a+l|. The number |a| is the length
of the ``area'' or directory, and |l| is the length of the font name itself;
the standard local system font area is supposed to be used when |a=0|.
The |n| field contains the area in its first |a| bytes.

Font definitions must appear before the first use of a particular font number.
Once font |k| is defined, it must not be defined again; however, we
shall see below that font definitions appear in the postamble as well as
in the pages, so in this sense each font number is defined exactly twice,
if at all. Like |nop| commands, font definitions can
appear before the first |bop|, or between an |eop| and a |bop|.

@ Sometimes it is desirable to make horizontal or vertical rules line up
precisely with certain features in characters of a font. It is possible to
guarantee the correct matching between \.{DVI} output and the characters
generated by \MF\ by adhering to the following principles: (1)~The \MF\
characters should be positioned so that a bottom edge or left edge that is
supposed to line up with the bottom or left edge of a rule appears at the
reference point, i.e., in row~0 and column~0 of the \MF\ raster. This
ensures that the position of the rule will not be rounded differently when
the pixel size is not a perfect multiple of the units of measurement in
the \.{DVI} file. (2)~A typeset rule of height $a>0$ and width $b>0$
should be equivalent to a \MF-generated character having black pixels in
precisely those raster positions whose \MF\ coordinates satisfy
|0<=x<@t$\alpha$@>b| and |0<=y<@t$\alpha$@>a|, where $\alpha$ is the number
of pixels per \.{DVI} unit.
@:METAFONT}{\MF@>
@^alignment of rules with characters@>
@^rules aligning with characters@>

@ The last page in a \.{DVI} file is followed by `|post|'; this command
introduces the postamble, which summarizes important facts that \TeX\ has
accumulated about the file, making it possible to print subsets of the data
with reasonable efficiency. The postamble has the form
$$\vbox{\halign{\hbox{#\hfil}\cr
  |post| |p[4]| |num[4]| |den[4]| |mag[4]| |l[4]| |u[4]| |s[2]| |t[2]|\cr
  $\langle\,$font definitions$\,\rangle$\cr
  |post_post| |q[4]| |i[1]| 223's$[{\G}4]$\cr}}$$
Here |p| is a pointer to the final |bop| in the file. The next three
parameters, |num|, |den|, and |mag|, are duplicates of the quantities that
appeared in the preamble.

Parameters |l| and |u| give respectively the height-plus-depth of the tallest
page and the width of the widest page, in the same units as other dimensions
of the file. These numbers might be used by a \.{DVI}-reading program to
position individual ``pages'' on large sheets of film or paper; however,
the standard convention for output on normal size paper is to position each
page so that the upper left-hand corner is exactly one inch from the left
and the top. Experience has shown that it is unwise to design \.{DVI}-to-printer
software that attempts cleverly to center the output; a fixed position of
the upper left corner is easiest for users to understand and to work with.
Therefore |l| and~|u| are often ignored.

Parameter |s| is the maximum stack depth (i.e., the largest excess of
|push| commands over |pop| commands) needed to process this file. Then
comes |t|, the total number of pages (|bop| commands) present.

The postamble continues with font definitions, which are any number of
\\{fnt\_def} commands as described above, possibly interspersed with |nop|
commands. Each font number that is used in the \.{DVI} file must be defined
exactly twice: Once before it is first selected by a \\{fnt} command, and once
in the postamble.

@ The last part of the postamble, following the |post_post| byte that
signifies the end of the font definitions, contains |q|, a pointer to the
|post| command that started the postamble.  An identification byte, |i|,
comes next; this currently equals~2, as in the preamble.

The |i| byte is followed by four or more bytes that are all equal to
the decimal number 223 (i.e., '337 in octal). \TeX\ puts out four to seven of
these trailing bytes, until the total length of the file is a multiple of
four bytes, since this works out best on machines that pack four bytes per
word; but any number of 223's is allowed, as long as there are at least four
of them. In effect, 223 is a sort of signature that is added at the very end.
@^Fuchs, David Raymond@>

This curious way to finish off a \.{DVI} file makes it feasible for
\.{DVI}-reading programs to find the postamble first, on most computers,
even though \TeX\ wants to write the postamble last. Most operating
systems permit random access to individual words or bytes of a file, so
the \.{DVI} reader can start at the end and skip backwards over the 223's
until finding the identification byte. Then it can back up four bytes, read
|q|, and move to byte |q| of the file. This byte should, of course,
contain the value 248 (|post|); now the postamble can be read, so the
\.{DVI} reader can discover all the information needed for typesetting the
pages. Note that it is also possible to skip through the \.{DVI} file at
reasonably high speed to locate a particular page, if that proves
desirable. This saves a lot of time, since \.{DVI} files used in production
jobs tend to be large.

Unfortunately, however, standard \PASCAL\ does not include the ability to
@^system dependencies@>
access a random position in a file, or even to determine the length of a file.
Almost all systems nowadays provide the necessary capabilities, so \.{DVI}
format has been designed to work most efficiently with modern operating systems.
But if \.{DVI} files have to be processed under the restrictions of standard
\PASCAL, one can simply read them from front to back, since the necessary
header information is present in the preamble and in the font definitions.
(The |l| and |u| and |s| and |t| parameters, which appear only in the
postamble, are ``frills'' that are handy but not absolutely necessary.)


@* \[32] Shipping pages out.
After considering \TeX's eyes and stomach, we come now to the bowels.
@^bowels@>

The |ship_out| procedure is given a pointer to a box; its mission is
to describe that box in \.{DVI} form, outputting a ``page'' to |dvi_file|.
The \.{DVI} coordinates $(h,v)=(0,0)$ should correspond to the upper left
corner of the box being shipped.

Since boxes can be inside of boxes inside of boxes, the main work of
|ship_out| is done by two mutually recursive routines, |hlist_out|
and |vlist_out|, which traverse the hlists and vlists inside of horizontal
and vertical boxes.

As individual pages are being processed, we need to accumulate
information about the entire set of pages, since such statistics must be
reported in the postamble. The global variables |total_pages|, |max_v|,
|max_h|, |max_push|, and |last_bop| are used to record this information.

The variable |doing_leaders| is |true| while leaders are being output.
The variable |dead_cycles| contains the number of times an output routine
has been initiated since the last |ship_out|.

A few additional global variables are also defined here for use in
|vlist_out| and |hlist_out|. They could have been local variables, but
that would waste stack space when boxes are deeply nested, since the
values of these variables are not needed during recursive calls.
@^recursion@>

@c
int total_pages = 0;            /* the number of pages that have been shipped out */
scaled max_v = 0;               /* maximum height-plus-depth of pages shipped so far */
scaled max_h = 0;               /* maximum width of pages shipped so far */
int max_push = 0;               /* deepest nesting of |push| commands encountered so far */
int last_bop = -1;              /* location of previous |bop| in the \.{DVI} output */
int dead_cycles = 0;            /* recent outputs that didn't ship anything out */
boolean doing_leaders = false;  /* are we inside a leader box? */
int oval, ocmd;                 /* used by |out_cmd| for generating |set|, |fnt| and |fnt_def| commands */
pointer g;                      /* current glue specification */
int lq, lr;                     /* quantities used in calculations for leaders */
int cur_s = -1;                 /* current depth of output box nesting, initially $-1$ */

@ The \.{DVI} bytes are output to a buffer instead of being written directly
to the output file. This makes it possible to reduce the overhead of
subroutine calls, thereby measurably speeding up the computation, since
output of \.{DVI} bytes is part of \TeX's inner loop. And it has another
advantage as well, since we can change instructions in the buffer in order to
make the output more compact. For example, a `|down2|' command can be
changed to a `|y2|', thereby making a subsequent `|y0|' command possible,
saving two bytes.

The output buffer is divided into two parts of equal size; the bytes found
in |dvi_buf[0..half_buf-1]| constitute the first half, and those in
|dvi_buf[half_buf..dvi_buf_size-1]| constitute the second. The global
variable |dvi_ptr| points to the position that will receive the next
output byte. When |dvi_ptr| reaches |dvi_limit|, which is always equal
to one of the two values |half_buf| or |dvi_buf_size|, the half buffer that
is about to be invaded next is sent to the output and |dvi_limit| is
changed to its other value. Thus, there is always at least a half buffer's
worth of information present, except at the very beginning of the job.

Bytes of the \.{DVI} file are numbered sequentially starting with 0;
the next byte to be generated will be number |dvi_offset+dvi_ptr|.
A byte is present in the buffer only if its number is |>=dvi_gone|.

Some systems may find it more efficient to make |dvi_buf| a |packed|
array, since output of four bytes at once may be facilitated.
@^system dependencies@>


@ Initially the buffer is all in one piece; we will output half of it only
after it first fills up.

@c
int dvi_buf_size = 800;         /* size of the output buffer; must be a multiple of 8 */
eight_bits *dvi_buf;            /* buffer for \.{DVI} output */
dvi_index half_buf = 0;         /* half of |dvi_buf_size| */
dvi_index dvi_limit = 0;        /* end of the current half buffer */
dvi_index dvi_ptr = 0;          /* the next available buffer address */
int dvi_offset = 0;             /* |dvi_buf_size| times the number of times the output buffer has been fully emptied */
int dvi_gone = 0;               /* the number of bytes already output to |dvi_file| */

@ The actual output of |dvi_buf[a..b]| to |dvi_file| is performed by calling
|write_dvi(a,b)|. For best results, this procedure should be optimized to
run as fast as possible on each particular system, since it is part of
\TeX's inner loop. It is safe to assume that |a| and |b+1| will both be
multiples of 4 when |write_dvi(a,b)| is called; therefore it is possible on
many machines to use efficient methods to pack four bytes per word and to
output an array of words with one system call.
@^system dependencies@>
@^inner loop@>
@^defecation@>

@c
static void write_dvi(dvi_index a, dvi_index b)
{
    dvi_index k;
    for (k = a; k <= b; k++)
        fputc(dvi_buf[k], static_pdf->file);
}

/* outputs half of the buffer */
void dvi_swap(void)
{
    if (dvi_limit == dvi_buf_size) {
        write_dvi(0, half_buf - 1);
        dvi_limit = half_buf;
        dvi_offset = dvi_offset + dvi_buf_size;
        dvi_ptr = 0;
    } else {
        write_dvi(half_buf, dvi_buf_size - 1);
        dvi_limit = dvi_buf_size;
    }
    dvi_gone = dvi_gone + half_buf;
}

@ The |dvi_four| procedure outputs four bytes in two's complement notation,
without risking arithmetic overflow.

@c
void dvi_four(int x)
{
    if (x >= 0) {
        dvi_out(x / 0100000000);
    } else {
        x = x + 010000000000;
        x = x + 010000000000;
        dvi_out((x / 0100000000) + 128);
    }
    x = x % 0100000000;
    dvi_out(x / 0200000);
    x = x % 0200000;
    dvi_out(x / 0400);
    dvi_out(x % 0400);
}

@
A mild optimization of the output is performed by the |dvi_pop|
routine, which issues a |pop| unless it is possible to cancel a
`|push| |pop|' pair. The parameter to |dvi_pop| is the byte address
following the old |push| that matches the new |pop|.


@c
void dvi_push(void)
{
    dvi_out(push);
}

void dvi_pop(int l)
{
    if ((l == dvi_offset + dvi_ptr) && (dvi_ptr > 0))
        decr(dvi_ptr);
    else
        dvi_out(pop);
}

@ Here's a procedure that outputs a font definition. $\Omega$ allows
more than 256 different fonts per job, so the right font definition
command must be selected.

@c
void out_cmd(void)
{
    if ((oval < 0x100) && (oval >= 0)) {
        if ((ocmd != set1) || (oval > 127)) {
            if ((ocmd == fnt1) && (oval < 64))
                oval += fnt_num_0;
            else
                dvi_out(ocmd);
        }
    } else {
        if ((oval < 0x10000) && (oval >= 0)) {
            dvi_out(ocmd + 1);
        } else {
            if ((oval < 0x1000000) && (oval >= 0)) {
                dvi_out(ocmd + 2);
            } else {
                dvi_out(ocmd + 3);
                if (oval >= 0) {
                    dvi_out(oval / 0x1000000);
                } else {
                    oval += 0x40000000;
                    oval += 0x40000000;
                    dvi_out((oval / 0x1000000) + 128);
                    oval = oval % 0x1000000;
                }
                dvi_out(oval / 0x10000);
                oval = oval % 0x10000;
            }
            dvi_out(oval / 0x10000);
            oval = oval % 0x10000;
        }
        dvi_out(oval / 0x100);
        oval = oval % 0x100;
    }
    dvi_out(oval);
}

void dvi_font_def(internal_font_number f)
{
    char *fa;
    oval = f - 1;
    ocmd = fnt_def1;
    out_cmd();
    dvi_out(font_check_0(f));
    dvi_out(font_check_1(f));
    dvi_out(font_check_2(f));
    dvi_out(font_check_3(f));
    dvi_four(font_size(f));
    dvi_four(font_dsize(f));
    dvi_out(0);                 /* |font_area(f)| is unused */
    dvi_out(strlen(font_name(f)));
    /* Output the font name whose internal number is |f| */
    fa = font_name(f);
    while (*fa != '\0') {
        dvi_out(*fa++);
    }
}

@ Versions of \TeX\ intended for small computers might well choose to omit
the ideas in the next few parts of this program, since it is not really
necessary to optimize the \.{DVI} code by making use of the |w0|, |x0|,
|y0|, and |z0| commands. Furthermore, the algorithm that we are about to
describe does not pretend to give an optimum reduction in the length
of the \.{DVI} code; after all, speed is more important than compactness.
But the method is surprisingly effective, and it takes comparatively little
time.

We can best understand the basic idea by first considering a simpler problem
that has the same essential characteristics. Given a sequence of digits,
say $3\,1\,4\,1\,5\,9\,2\,6\,5\,3\,5\,8\,9$, we want to assign subscripts
$d$, $y$, or $z$ to each digit so as to maximize the number of ``$y$-hits''
and ``$z$-hits''; a $y$-hit is an instance of two appearances of the same
digit with the subscript $y$, where no $y$'s intervene between the two
appearances, and a $z$-hit is defined similarly. For example, the sequence
above could be decorated with subscripts as follows:
$$3_z\,1_y\,4_d\,1_y\,5_y\,9_d\,2_d\,6_d\,5_y\,3_z\,5_y\,8_d\,9_d.$$
There are three $y$-hits ($1_y\ldots1_y$ and $5_y\ldots5_y\ldots5_y$) and
one $z$-hit ($3_z\ldots3_z$); there are no $d$-hits, since the two appearances
of $9_d$ have $d$'s between them, but we don't count $d$-hits so it doesn't
matter how many there are. These subscripts are analogous to the \.{DVI}
commands called \\{down}, $y$, and $z$, and the digits are analogous to
different amounts of vertical motion; a $y$-hit or $z$-hit corresponds to
the opportunity to use the one-byte commands |y0| or |z0| in a \.{DVI} file.

\TeX's method of assigning subscripts works like this: Append a new digit,
say $\delta$, to the right of the sequence. Now look back through the
sequence until one of the following things happens: (a)~You see
$\delta_y$ or $\delta_z$, and this was the first time you encountered a
$y$ or $z$ subscript, respectively.  Then assign $y$ or $z$ to the new
$\delta$; you have scored a hit. (b)~You see $\delta_d$, and no $y$
subscripts have been encountered so far during this search.  Then change
the previous $\delta_d$ to $\delta_y$ (this corresponds to changing a
command in the output buffer), and assign $y$ to the new $\delta$; it's
another hit.  (c)~You see $\delta_d$, and a $y$ subscript has been seen
but not a $z$.  Change the previous $\delta_d$ to $\delta_z$ and assign
$z$ to the new $\delta$. (d)~You encounter both $y$ and $z$ subscripts
before encountering a suitable $\delta$, or you scan all the way to the
front of the sequence. Assign $d$ to the new $\delta$; this assignment may
be changed later.

The subscripts $3_z\,1_y\,4_d\ldots\,$ in the example above were, in fact,
produced by this procedure, as the reader can verify. (Go ahead and try it.)

@ In order to implement such an idea, \TeX\ maintains a stack of pointers
to the \\{down}, $y$, and $z$ commands that have been generated for the
current page. And there is a similar stack for \\{right}, |w|, and |x|
commands. These stacks are called the down stack and right stack, and their
top elements are maintained in the variables |down_ptr| and |right_ptr|.

Each entry in these stacks contains four fields: The |width| field is
the amount of motion down or to the right; the |location| field is the
byte number of the \.{DVI} command in question (including the appropriate
|dvi_offset|); the |vlink| field points to the next item below this one
on the stack; and the |vinfo| field encodes the options for possible change
in the \.{DVI} command.

@c
#define location(A) varmem[(A)+1].cint  /* \.{DVI} byte number for a movement command */

halfword down_ptr = null, right_ptr = null;     /* heads of the down and right stacks */

@ Here is a subroutine that produces a \.{DVI} command for some specified
downward or rightward motion. It has two parameters: |w| is the amount
of motion, and |o| is either |down1| or |right1|. We use the fact that
the command codes have convenient arithmetic properties: |y1-down1=w1-right1|
and |z1-down1=x1-right1|.

@c
void movement(scaled w, eight_bits o)
{
    small_number mstate;        /* have we seen a |y| or |z|? */
    halfword p, q;              /* current and top nodes on the stack */
    int k;                      /* index into |dvi_buf|, modulo |dvi_buf_size| */
    if (false) {                /* TODO: HUH? */
        q = new_node(movement_node, 0); /* new node for the top of the stack */
        width(q) = w;
        location(q) = dvi_offset + dvi_ptr;
        if (o == down1) {
            vlink(q) = down_ptr;
            down_ptr = q;
        } else {
            vlink(q) = right_ptr;
            right_ptr = q;
        }
        /* Look at the other stack entries until deciding what sort of \.{DVI} command
           to generate; |goto found| if node |p| is a ``hit'' */
        p = vlink(q);
        mstate = none_seen;
        while (p != null) {
            if (width(p) == w) {
                /* Consider a node with matching width;|goto found| if it's a hit */
                /* We might find a valid hit in a |y| or |z| byte that is already gone
                   from the buffer. But we can't change bytes that are gone forever; ``the
                   moving finger writes, $\ldots\,\,$.'' */

                switch (mstate + vinfo(p)) {
                case none_seen + yz_OK:
                case none_seen + y_OK:
                case z_seen + yz_OK:
                case z_seen + y_OK:
                    if (location(p) < dvi_gone) {
                        goto NOT_FOUND;
                    } else {
                        /* Change buffered instruction to |y| or |w| and |goto found| */
                        k = location(p) - dvi_offset;
                        if (k < 0)
                            k = k + dvi_buf_size;
                        dvi_buf[k] = (eight_bits) (dvi_buf[k] + y1 - down1);
                        vinfo(p) = y_here;
                        goto FOUND;
                    }
                    break;
                case none_seen + z_OK:
                case y_seen + yz_OK:
                case y_seen + z_OK:
                    if (location(p) < dvi_gone) {
                        goto NOT_FOUND;
                    } else {
                        /* Change buffered instruction to |z| or |x| and |goto found| */
                        k = location(p) - dvi_offset;
                        if (k < 0)
                            k = k + dvi_buf_size;
                        dvi_buf[k] = (eight_bits) (dvi_buf[k] + z1 - down1);
                        vinfo(p) = z_here;
                        goto FOUND;
                    }
                    break;
                case none_seen + y_here:
                case none_seen + z_here:
                case y_seen + z_here:
                case z_seen + y_here:
                    goto FOUND;
                    break;
                default:
                    break;
                }
            } else {
                switch (mstate + vinfo(p)) {
                case none_seen + y_here:
                    mstate = y_seen;
                    break;
                case none_seen + z_here:
                    mstate = z_seen;
                    break;
                case y_seen + z_here:
                case z_seen + y_here:
                    goto NOT_FOUND;
                    break;
                default:
                    break;
                }
            }
            p = vlink(p);
        }
    }
  NOT_FOUND:
    /* Generate a |down| or |right| command for |w| and |return| */
    if (abs(w) >= 040000000) {
        dvi_out(o + 3);         /* |down4| or |right4| */
        dvi_four(w);
        return;
    }
    if (abs(w) >= 0100000) {
        dvi_out(o + 2);         /* |down3| or |right3| */
        if (w < 0)
            w = w + 0100000000;
        dvi_out(w / 0200000);
        w = w % 0200000;
        goto TWO;
    }
    if (abs(w) >= 0200) {
        dvi_out(o + 1);         /* |down2| or |right2| */
        if (w < 0)
            w = w + 0200000;
        goto TWO;
    }
    dvi_out(o);                 /* |down1| or |right1| */
    if (w < 0)
        w = w + 0400;
    goto ONE;
  TWO:
    dvi_out(w / 0400);
  ONE:
    dvi_out(w % 0400);
    return;
  FOUND:
    /* Generate a |y0| or |z0| command in order to reuse a previous appearance of~|w| */
    /* The program below removes movement nodes that are introduced after a |push|,
       before it outputs the corresponding |pop|. */
    /*
       When the |movement| procedure gets to the label |found|, the value of
       |vinfo(p)| will be either |y_here| or |z_here|. If it is, say, |y_here|,
       the procedure generates a |y0| command (or a |w0| command), and marks
       all |vinfo| fields between |q| and |p| so that |y| is not OK in that range.
     */
    vinfo(q) = vinfo(p);
    if (vinfo(q) == y_here) {
        dvi_out(o + y0 - down1);        /* |y0| or |w0| */
        while (vlink(q) != p) {
            q = vlink(q);
            switch (vinfo(q)) {
            case yz_OK:
                vinfo(q) = z_OK;
                break;
            case y_OK:
                vinfo(q) = d_fixed;
                break;
            default:
                break;
            }
        }
    } else {
        dvi_out(o + z0 - down1);        /* |z0| or |x0| */
        while (vlink(q) != p) {
            q = vlink(q);
            switch (vinfo(q)) {
            case yz_OK:
                vinfo(q) = y_OK;
                break;
            case z_OK:
                vinfo(q) = d_fixed;
                break;
            default:
                break;
            }
        }
    }
}

@ In case you are wondering when all the movement nodes are removed from
\TeX's memory, the answer is that they are recycled just before
|hlist_out| and |vlist_out| finish outputting a box. This restores the
down and right stacks to the state they were in before the box was output,
except that some |vinfo|'s may have become more restrictive.


@c
/* delete movement nodes with |location>=l| */
void prune_movements(int l)
{
    pointer p;                  /* node being deleted */
    while (down_ptr != null) {
        if (location(down_ptr) < l)
            break;
        p = down_ptr;
        down_ptr = vlink(p);
        flush_node(p);
    }
    while (right_ptr != null) {
        if (location(right_ptr) < l)
            return;
        p = right_ptr;
        right_ptr = vlink(p);
        flush_node(p);
    }
}

scaledpos dvi;                  /* a \.{DVI} position in page coordinates, in sync with DVI file */

@ When |hlist_out| is called, its duty is to output the box represented
by the |hlist_node| pointed to by |temp_ptr|. The reference point of that
box has coordinates |(cur.h,cur.v)|.

Similarly, when |vlist_out| is called, its duty is to output the box represented
by the |vlist_node| pointed to by |temp_ptr|. The reference point of that
box has coordinates |(cur.h,cur.v)|.
@^recursion@>

@ The recursive procedures |hlist_out| and |vlist_out| each have a local variable
|save_dvi| to hold the value of |dvi| just before
entering a new level of recursion.  In effect, the value of |save_dvi|
on \TeX's run-time stack corresponds to the values of |h| and |v|
that a \.{DVI}-reading program will push onto its coordinate stack.

@c
void dvi_place_rule(PDF pdf, halfword q, scaledpos size)
{
    synch_dvi_with_pos(pdf->posstruct->pos);
    if ((subtype(q) >= box_rule) && (subtype(q) <= user_rule)) {
        /* place nothing, only take space */
        if (textdir_is_L(pdf->posstruct->dir))
            dvi.h += size.h;
    } else {
        /* normal_rule or >= 100 being a leader rule */
        if (textdir_is_L(pdf->posstruct->dir)) {
            dvi_out(set_rule);      /* movement optimization for |dir_*L*| */
            dvi.h += size.h;
        } else
            dvi_out(put_rule);
    }
    dvi_four(size.v);
    dvi_four(size.h);
}

void dvi_place_glyph(PDF pdf, internal_font_number f, int c, int ex)
{
    /* TODO: do something on ex, select font (if possible) */
    scaled_whd ci;
    synch_dvi_with_pos(pdf->posstruct->pos);
    if (f != pdf->f_cur) {
        /* Change font |f_cur| to |f| */
        if (!font_used(f)) {
            dvi_font_def(f);
            set_font_used(f, true);
        }
        oval = f - 1;
        ocmd = fnt1;
        out_cmd();
        pdf->f_cur = f;
    }
    if (textdir_is_L(pdf->posstruct->dir)) {
        ci = get_charinfo_whd(f, c);
        dvi_set(c, ci.wd);      /* movement optimization for |dir_*L*| */
    } else
        dvi_put(c);
}

void dvi_special(PDF pdf, halfword p)
{
    int old_setting;            /* holds print |selector| */
    unsigned k;                 /* index into |cur_string| */
    synch_dvi_with_pos(pdf->posstruct->pos);
    old_setting = selector;
    selector = new_string;
    show_token_list(token_link(write_tokens(p)), null, -1);
    selector = old_setting;
    if (cur_length < 256) {
        dvi_out(xxx1);
        dvi_out(cur_length);
    } else {
        dvi_out(xxx4);
        dvi_four((int) cur_length);
    }
    for (k = 0; k < cur_length; k++)
        dvi_out(cur_string[k]);
    cur_length = 0;             /* erase the string */
}

@ Here's an example of how these conventions are used. Whenever it is time to
ship out a box of stuff, we shall use the macro |ensure_dvi_open|.

@c
void ensure_dvi_header_written(PDF pdf)
{
    unsigned l;
    unsigned s;                 /* index into |str_pool| */
    int old_setting;            /* saved |selector| setting */
    assert(output_mode_used == OMODE_DVI);
    assert(pdf->o_state == ST_FILE_OPEN);

    if (half_buf == 0) {
        half_buf = dvi_buf_size / 2;
        dvi_limit = dvi_buf_size;
    }

    dvi_out(pre);
    dvi_out(id_byte);           /* output the preamble */
    dvi_four(25400000);
    dvi_four(473628672);        /* conversion ratio for sp */
    prepare_mag();
    dvi_four(mag);              /* magnification factor is frozen */
    if (output_comment) {
        l = (unsigned) strlen(output_comment);
        dvi_out(l);
        for (s = 0; s < l; s++)
            dvi_out(output_comment[s]);
    } else {                    /* the default code is unchanged */
        old_setting = selector;
        selector = new_string;
        tprint(" LuaTeX output ");
        print_int(int_par(year_code));
        print_char('.');
        print_two(int_par(month_code));
        print_char('.');
        print_two(int_par(day_code));
        print_char(':');
        print_two(int_par(time_code) / 60);
        print_two(int_par(time_code) % 60);
        selector = old_setting;
        dvi_out(cur_length);
        for (s = 0; s < cur_length; s++)
            dvi_out(cur_string[s]);
        cur_length = 0;
    }
}

void dvi_begin_page(PDF pdf)
{
    int k;
    int page_loc;               /* location of the current |bop| */
    ensure_output_state(pdf, ST_HEADER_WRITTEN);
    /* Initialize variables as |ship_out| begins */
    page_loc = dvi_offset + dvi_ptr;
    dvi_out(bop);
    for (k = 0; k <= 9; k++)
        dvi_four(count(k));
    dvi_four(last_bop);
    last_bop = page_loc;
}

void dvi_end_page(PDF pdf)
{
    (void) pdf;
    dvi_out(eop);

#ifdef IPC
    if (ipcon > 0) {
        if (dvi_limit == half_buf) {
            write_dvi(half_buf, dvi_buf_size - 1);
	    fflush(static_pdf->file);
            dvi_gone = dvi_gone + half_buf;
        }
        if (dvi_ptr > 0) {
            write_dvi(0, dvi_ptr - 1);
	    fflush(static_pdf->file);
            dvi_offset = dvi_offset + dvi_ptr;
            dvi_gone = dvi_gone + dvi_ptr;
        }
        dvi_ptr = 0;
        dvi_limit = dvi_buf_size;
        ipcpage(dvi_gone);
    }
#endif                          /* IPC */
}

@ At the end of the program, we must finish things off by writing the
post\-amble. If |total_pages=0|, the \.{DVI} file was never opened.
If |total_pages>=65536|, the \.{DVI} file will lie. And if
|max_push>=65536|, the user deserves whatever chaos might ensue.

@c
void finish_dvi_file(PDF pdf, int version, int revision)
{
    int k;
    int callback_id = callback_defined(stop_run_callback);
    (void) version;
    (void) revision;
    while (cur_s > -1) {
        if (cur_s > 0) {
            dvi_out(pop);
        } else {
            dvi_out(eop);
            incr(total_pages);
        }
        decr(cur_s);
    }
    if (total_pages == 0) {
        if (callback_id == 0) {
            tprint_nl("No pages of output.");
            print_ln();
        } else if (callback_id > 0) {
            run_callback(callback_id, "->");
        }
    } else {
        dvi_out(post);          /* beginning of the postamble */
        dvi_four(last_bop);
        last_bop = dvi_offset + dvi_ptr - 5;    /* |post| location */
        dvi_four(25400000);
        dvi_four(473628672);    /* conversion ratio for sp */
        prepare_mag();
        dvi_four(mag);          /* magnification factor */
        dvi_four(max_v);
        dvi_four(max_h);
        dvi_out(max_push / 256);
        dvi_out(max_push % 256);
        dvi_out((total_pages / 256) % 256);
        dvi_out(total_pages % 256);
        /* Output the font definitions for all fonts that were used */
        k = max_font_id();
        while (k > 0) {
            if (font_used(k)) {
                dvi_font_def(k);
            }
            decr(k);
        }

        dvi_out(post_post);
        dvi_four(last_bop);
        dvi_out(id_byte);
#ifndef IPC
        k = 4 + ((dvi_buf_size - dvi_ptr) % 4); /* the number of 223's */
#else
        k = 7 - ((3 + dvi_offset + dvi_ptr) % 4);       /* the number of 223's */
#endif

        while (k > 0) {
            dvi_out(223);
            decr(k);
        }
        /* Empty the last bytes out of |dvi_buf| */
        /* Here is how we clean out the buffer when \TeX\ is all through; |dvi_ptr|
           will be a multiple of~4. */
        if (dvi_limit == half_buf)
            write_dvi(half_buf, dvi_buf_size - 1);
        if (dvi_ptr > 0)
            write_dvi(0, dvi_ptr - 1);

        if (callback_id == 0) {
            tprint_nl("Output written on ");
            tprint(pdf->file_name);
            tprint(" (");
            print_int(total_pages);
            tprint(" page");
            if (total_pages != 1)
                print_char('s');
            tprint(", ");
            print_int(dvi_offset + dvi_ptr);
            tprint(" bytes).");
        } else if (callback_id > 0) {
            run_callback(callback_id, "->");
        }
        close_file(pdf->file);
    }
}
