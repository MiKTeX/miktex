/*

dvigen.w

Copyright 2009-2013 Taco Hoekwater <taco@luatex.org>

This file is part of LuaTeX.

LuaTeX is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.

LuaTeX is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU General Public License along
with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

#include "ptexlib.h"

#undef write_dvi

/*tex This is the current mode: */

#define mode cur_list.mode_field

/*tex

The most important output produced by a run of \TeX\ is the ``device
independent'' (\.{DVI}) file that specifies where characters and rules are to
appear on printed pages. The form of these files was designed by David R. Fuchs
in 1979. Almost any reasonable typesetting device can be driven by a program that
takes \.{DVI} files as input, and dozens of such \.{DVI}-to-whatever programs
have been written. Thus, it is possible to print the output of \TeX\ on many
different kinds of equipment, using \TeX\ as a device-independent ``front end.''

A \.{DVI} file is a stream of 8-bit bytes, which may be regarded as a series of
commands in a machine-like language. The first byte of each command is the
operation code, and this code is followed by zero or more bytes that provide
parameters to the command. The parameters themselves may consist of several
consecutive bytes; for example, the `|set_rule|' command has two parameters, each
of which is four bytes long. Parameters are usually regarded as nonnegative
integers; but four-byte-long parameters, and shorter parameters that denote
distances, can be either positive or negative. Such parameters are given in two's
complement notation. For example, a two-byte-long distance parameter has a value
between $-2^{15}$ and $2^{15}-1$. As in \.{TFM} files, numbers that occupy more
than one byte position appear in BigEndian order.

A \.{DVI} file consists of a ``preamble,'' followed by a sequence of one or more
``pages,'' followed by a ``postamble.'' The preamble is simply a |pre| command,
with its parameters that define the dimensions used in the file; this must come
first. Each ``page'' consists of a |bop| command, followed by any number of other
commands that tell where characters are to be placed on a physical page, followed
by an |eop| command. The pages appear in the order that \TeX\ generated them. If
we ignore |nop| commands and \\{fnt\_def} commands (which are allowed between any
two commands in the file), each |eop| command is immediately followed by a |bop|
command, or by a |post| command; in the latter case, there are no more pages in
the file, and the remaining bytes form the postamble. Further details about the
postamble will be explained later.

Some parameters in \.{DVI} commands are ``pointers.'' These are four-byte
quantities that give the location number of some other byte in the file; the
first byte is number~0, then comes number~1, and so on. For example, one of the
parameters of a |bop| command points to the previous |bop|; this makes it
feasible to read the pages in backwards order, in case the results are being
directed to a device that stacks its output face up. Suppose the preamble of a
\.{DVI} file occupies bytes 0 to 99. Now if the first page occupies bytes 100 to
999, say, and if the second page occupies bytes 1000 to 1999, then the |bop| that
starts in byte 1000 points to 100 and the |bop| that starts in byte 2000 points
to 1000. (The very first |bop|, i.e., the one starting in byte 100, has a pointer
of~$-1$.)

The \.{DVI} format is intended to be both compact and easily interpreted by a
machine. Compactness is achieved by making most of the information implicit
instead of explicit. When a \.{DVI}-reading program reads the commands for a
page, it keeps track of several quantities: (a)~The current font |f| is an
integer; this value is changed only by \\{fnt} and \\{fnt\_num} commands. (b)~The
current position on the page is given by two numbers called the horizontal and
vertical coordinates, |h| and |v|. Both coordinates are zero at the upper left
corner of the page; moving to the right corresponds to increasing the horizontal
coordinate, and moving down corresponds to increasing the vertical coordinate.
Thus, the coordinates are essentially Cartesian, except that vertical directions
are flipped; the Cartesian version of |(h,v)| would be |(h,-v)|. (c)~The current
spacing amounts are given by four numbers |w|, |x|, |y|, and |z|, where |w|
and~|x| are used for horizontal spacing and where |y| and~|z| are used for
vertical spacing. (d)~There is a stack containing |(h,v,w,x,y,z)| values; the
\.{DVI} commands |push| and |pop| are used to change the current level of
operation. Note that the current font~|f| is not pushed and popped; the stack
contains only information about positioning.

The values of |h|, |v|, |w|, |x|, |y|, and |z| are signed integers having up to
32 bits, including the sign. Since they represent physical distances, there is a
small unit of measurement such that increasing |h| by~1 means moving a certain
tiny distance to the right. The actual unit of measurement is variable, as
explained below; \TeX\ sets things up so that its \.{DVI} output is in sp units,
i.e., scaled points, in agreement with all the |scaled| dimensions in \TeX's data
structures.

Here is a list of all the commands that may appear in a \.{DVI} file. Each
command is specified by its symbolic name (e.g., |bop|), its opcode byte (e.g.,
139), and its parameters (if any). The parameters are followed by a bracketed
number telling how many bytes they occupy; for example, `|p[4]|' means that
parameter |p| is four bytes long.

\startitemize

\startitem
    |set_char_0| 0. Typeset character number~0 from font~|f| such that the
    reference point of the character is at |(h,v)|. Then increase |h| by the
    width of that character. Note that a character may have zero or negative
    width, so one cannot be sure that |h| will advance after this command; but
    |h| usually does increase.
\stopitem

\startitem
    \\{set\_char\_1} through \\{set\_char\_127} (opcodes 1 to 127).
    Do the operations of |set_char_0|; but use the character whose number
    matches the opcode, instead of character~0.
\stopitem

\startitem
    |set1| 128 |c[1]|. Same as |set_char_0|, except that character number~|c| is
    typeset. \TeX82 uses this command for characters in the range |128<=c<256|.
\stopitem

\startitem
    |set2| 129 |c[2]|. Same as |set1|, except that |c|~is two bytes long, so it
    is in the range |0<=c<65536|. \TeX82 never uses this command, but it should
    come in handy for extensions of \TeX\ that deal with oriental languages.
\stopitem

\startitem
    |set3| 130 |c[3]|. Same as |set1|, except that |c|~is three bytes long, so it
    can be as large as $2^{24}-1$. Not even the Chinese language has this many
    characters, but this command might prove useful in some yet unforeseen
    extension.
\stopitem

\startitem
    |set4| 131 |c[4]|. Same as |set1|, except that |c|~is four bytes long.
    Imagine that.
\stopitem

\startitem
    |set_rule| 132 |a[4]| |b[4]|. Typeset a solid black rectangle of height~|a|
    and width~|b|, with its bottom left corner at |(h,v)|. Then set |h:=h+b|. If
    either |a<=0| or |b<=0|, nothing should be typeset. Note that if |b<0|, the
    value of |h| will decrease even though nothing else happens. See below for
    details about how to typeset rules so that consistency with \MF\ is
    guaranteed.
\stopitem

\startitem
    |put1| 133 |c[1]|. Typeset character number~|c| from font~|f| such that the
    reference point of the character is at |(h,v)|. (The `put' commands are
    exactly like the `set' commands, except that they simply put out a character
    or a rule without moving the reference point afterwards.)
\stopitem

\startitem
    |put2| 134 |c[2]|. Same as |set2|, except that |h| is not changed.
\stopitem

\startitem
    |put3| 135 |c[3]|. Same as |set3|, except that |h| is not changed.
\stopitem

\startitem
    |put4| 136 |c[4]|. Same as |set4|, except that |h| is not changed.
\stopitem

\startitem
    |put_rule| 137 |a[4]| |b[4]|. Same as |set_rule|, except that |h| is not
    changed.
\stopitem

\startitem
    |nop| 138. No operation, do nothing. Any number of |nop|'s may occur between
    \.{DVI} commands, but a |nop| cannot be inserted between a command and its
    parameters or between two parameters.
\stopitem

\startitem
    |bop| 139 $c_0[4]$ $c_1[4]$ $\ldots$ $c_9[4]$ $p[4]$. Beginning of a page:
    Set |(h,v,w,x,y,z):=(0,0,0,0,0,0)| and set the stack empty. Set the current
    font |f| to an undefined value. The ten $c_i$ parameters hold the values of
    \.{\\count0} $\ldots$ \.{\\count9} in \TeX\ at the time \.{\\shipout} was
    invoked for this page; they can be used to identify pages, if a user wants to
    print only part of a \.{DVI} file. The parameter |p| points to the previous
    |bop| in the file; the first |bop| has $p=-1$.
\stopitem

\startitem
    |eop| 140. End of page: Print what you have read since the previous |bop|. At
    this point the stack should be empty. (The \.{DVI}-reading programs that
    drive most output devices will have kept a buffer of the material that
    appears on the page that has just ended. This material is largely, but not
    entirely, in order by |v| coordinate and (for fixed |v|) by |h|~coordinate;
    so it usually needs to be sorted into some order that is appropriate for the
    device in question.)
\stopitem

\startitem
    |push| 141. Push the current values of |(h,v,w,x,y,z)| onto the top of the
    stack; do not change any of these values. Note that |f| is not pushed.
\stopitem

\startitem
    |pop| 142. Pop the top six values off of the stack and assign them
    respectively to |(h,v,w,x,y,z)|. The number of pops should never exceed the
    number of pushes, since it would be highly embarrassing if the stack were
    empty at the time of a |pop| command.

\startitem
    |right1| 143 |b[1]|. Set |h:=h+b|, i.e., move right |b| units. The parameter
    is a signed number in two's complement notation, |-128<=b<128|; if |b<0|, the
    reference point moves left.
\stopitem

\startitem
    |right2| 144 |b[2]|. Same as |right1|, except that |b| is a two-byte quantity
    in the range |-32768<=b<32768|.
\stopitem

\startitem
    |right3| 145 |b[3]|. Same as |right1|, except that |b| is a three-byte
    quantity in the range |@t$-2^{23}$@><=b<@t$2^{23}$@>|.
\stopitem

\startitem
    |right4| 146 |b[4]|. Same as |right1|, except that |b| is a four-byte
    quantity in the range |@t$-2^{31}$@><=b<@t$2^{31}$@>|.
\stopitem

\startitem
    |w0| 147. Set |h:=h+w|; i.e., move right |w| units. With luck, this
    parameterless command will usually suffice, because the same kind of motion
    will occur several times in succession; the following commands explain how
    |w| gets particular values.
\stopitem

\startitem
    |w1| 148 |b[1]|. Set |w:=b| and |h:=h+b|. The value of |b| is a signed
    quantity in two's complement notation, |-128<=b<128|. This command changes
    the current |w|~spacing and moves right by |b|.
\stopitem

\startitem
    |w2| 149 |b[2]|. Same as |w1|, but |b| is two bytes long, |-32768<=b<32768|.
\stopitem

\startitem
    |w3| 150 |b[3]|. Same as |w1|, but |b| is three bytes long,
    |@t$-2^{23}$@><=b<@t$2^{23}$@>|.
\stopitem

\startitem
    |w4| 151 |b[4]|. Same as |w1|, but |b| is four bytes long,
    |@t$-2^{31}$@><=b<@t$2^{31}$@>|.
\stopitem

\startitem
    |x0| 152. Set |h:=h+x|; i.e., move right |x| units. The `|x|' commands are
    like the `|w|' commands except that they involve |x| instead of |w|.
\stopitem

\startitem
    |x1| 153 |b[1]|. Set |x:=b| and |h:=h+b|. The value of |b| is a signed
    quantity in two's complement notation, |-128<=b<128|. This command changes
    the current |x|~spacing and moves right by |b|.
\stopitem

\startitem
    |x2| 154 |b[2]|. Same as |x1|, but |b| is two bytes long, |-32768<=b<32768|.
\stopitem

\startitem
    |x3| 155 |b[3]|. Same as |x1|, but |b| is three bytes long,
    |@t$-2^{23}$@><=b<@t$2^{23}$@>|.
\stopitem

\startitem
    |x4| 156 |b[4]|. Same as |x1|, but |b| is four bytes long,
    |@t$-2^{31}$@><=b<@t$2^{31}$@>|.
\stopitem

\startitem
    |down1| 157 |a[1]|. Set |v:=v+a|, i.e., move down |a| units. The parameter is
    a signed number in two's complement notation, |-128<=a<128|; if |a<0|, the
    reference point moves up.
\stopitem

\startitem
    |down2| 158 |a[2]|. Same as |down1|, except that |a| is a two-byte quantity
    in the range |-32768<=a<32768|.
\stopitem

\startitem
    |down3| 159 |a[3]|. Same as |down1|, except that |a| is a three-byte quantity
    in the range |@t$-2^{23}$@><=a<@t$2^{23}$@>|.
\stopitem

\startitem
    |down4| 160 |a[4]|. Same as |down1|, except that |a| is a four-byte quantity
    in the range |@t$-2^{31}$@><=a<@t$2^{31}$@>|.
\stopitem

\startitem
    |y0| 161. Set |v:=v+y|; i.e., move down |y| units. With luck, this
    parameterless command will usually suffice, because the same kind of motion
    will occur several times in succession; the following commands explain how
    |y| gets particular values.
\stopitem

\startitem
    |y1| 162 |a[1]|. Set |y:=a| and |v:=v+a|. The value of |a| is a signed
    quantity in two's complement notation, |-128<=a<128|. This command changes
    the current |y|~spacing and moves down by |a|.
\stopitem

\startitem
    |y2| 163 |a[2]|. Same as |y1|, but |a| is two bytes long, |-32768<=a<32768|.
\stopitem

\startitem
    |y3| 164 |a[3]|. Same as |y1|, but |a| is three bytes long,
    |@t$-2^{23}$@><=a<@t$2^{23}$@>|.
\stopitem

\startitem
    |y4| 165 |a[4]|. Same as |y1|, but |a| is four bytes long,
    |@t$-2^{31}$@><=a<@t$2^{31}$@>|.
\stopitem

\startitem
    |z0| 166. Set |v:=v+z|; i.e., move down |z| units. The `|z|' commands are
    like the `|y|' commands except that they involve |z| instead of |y|.
\stopitem

\startitem
    |z1| 167 |a[1]|. Set |z:=a| and |v:=v+a|. The value of |a| is a signed
    quantity in two's complement notation, |-128<=a<128|. This command changes
    the current |z|~spacing and moves down by |a|.
\stopitem

\startitem
    |z2| 168 |a[2]|. Same as |z1|, but |a| is two bytes long, |-32768<=a<32768|.
\stopitem

\startitem
    |z3| 169 |a[3]|. Same as |z1|, but |a| is three bytes long,
    |@t$-2^{23}$@><=a<@t$2^{23}$@>|.
\stopitem

\startitem
    |z4| 170 |a[4]|. Same as |z1|, but |a| is four bytes long,
    |@t$-2^{31}$@><=a<@t$2^{31}$@>|.
\stopitem

\startitem
    |fnt_num_0| 171. Set |f:=0|. Font 0 must previously have been defined by a
    \\{fnt\_def} instruction, as explained below.
\stopitem

\startitem
    \\{fnt\_num\_1} through \\{fnt\_num\_63} (opcodes 172 to 234). Set |f:=1|,
    \dots, \hbox{|f:=63|}, respectively.
\stopitem

\startitem
    |fnt1| 235 |k[1]|. Set |f:=k|. \TeX82 uses this command for font numbers in
    the range |64<=k<256|.
\stopitem

\startitem
    |fnt2| 236 |k[2]|. Same as |fnt1|, except that |k|~is two bytes long, so it
    is in the range |0<=k<65536|. \TeX82 never generates this command, but large
    font numbers may prove useful for specifications of color or texture, or they
    may be used for special fonts that have fixed numbers in some external coding
    scheme.
\stopitem

\startitem
    |fnt3| 237 |k[3]|. Same as |fnt1|, except that |k|~is three bytes long, so it
    can be as large as $2^{24}-1$.
\stopitem

\startitem
    |fnt4| 238 |k[4]|. Same as |fnt1|, except that |k|~is four bytes long; this
    is for the really big font numbers (and for the negative ones).
\stopitem

\startitem
    |xxx1| 239 |k[1]| |x[k]|. This command is undefined in general; it functions
    as a $(k+2)$-byte |nop| unless special \.{DVI}-reading programs are being
    used. \TeX82 generates |xxx1| when a short enough \.{\\special} appears,
    setting |k| to the number of bytes being sent. It is recommended that |x| be
    a string having the form of a keyword followed by possible parameters
    relevant to that keyword.
\stopitem

\startitem
    |xxx2| 240 |k[2]| |x[k]|. Like |xxx1|, but |0<=k<65536|.
\stopitem

\startitem
    |xxx3| 241 |k[3]| |x[k]|. Like |xxx1|, but |0<=k<@t$2^{24}$@>|.
\stopitem

\startitem
    |xxx4| 242 |k[4]| |x[k]|. Like |xxx1|, but |k| can be ridiculously large.
    \TeX82 uses |xxx4| when sending a string of length 256 or more.
\stopitem

\startitem
    |fnt_def1| 243 |k[1]| |c[4]| |s[4]| |d[4]| |a[1]| |l[1]| |n[a+l]|. Define
    font |k|, where |0<=k<256|; font definitions will be explained shortly.
\stopitem

\startitem
    |fnt_def2| 244 |k[2]| |c[4]| |s[4]| |d[4]| |a[1]| |l[1]| |n[a+l]|. Define
    font |k|, where |0<=k<65536|.
\stopitem

\startitem
    |fnt_def3| 245 |k[3]| |c[4]| |s[4]| |d[4]| |a[1]| |l[1]| |n[a+l]|. Define
    font |k|, where |0<=k<@t$2^{24}$@>|.
\stopitem

\startitem
    |fnt_def4| 246 |k[4]| |c[4]| |s[4]| |d[4]| |a[1]| |l[1]| |n[a+l]|. Define
    font |k|, where |@t$-2^{31}$@><=k<@t$2^{31}$@>|.
\stopitem

\startitem
    |pre| 247 |i[1]| |num[4]| |den[4]| |mag[4]| |k[1]| |x[k]|. Beginning of the
    preamble; this must come at the very beginning of the file. Parameters |i|,
    |num|, |den|, |mag|, |k|, and |x| are explained below.
\stopitem

\startitem
    |post| 248. Beginning of the postamble, see below.
\stopitem

\startitem
    |post_post| 249. Ending of the postamble, see below.
\stopitem

\startitem
    Commands 250--255 are undefined at the present time.
\stopitem

*/

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

/*tex

The preamble contains basic information about the file as a whole. As stated
above, there are six parameters: $$\hbox{|i[1]| |num[4]| |den[4]|
|mag[4]| |k[1]| |x[k]|.}$$ The |i| byte identifies \.{DVI} format;
currently this byte is always set to~2. (The value |i=3| is currently used for an
extended format that allows a mixture of right-to-left and left-to-right
typesetting. Some day we will set |i=4|, when \.{DVI} format makes another
incompatible change---perhaps in the year 2048.)

The next two parameters, |num| and |den|, are positive integers that define the
units of measurement; they are the numerator and denominator of a fraction by
which all dimensions in the \.{DVI} file could be multiplied in order to get
lengths in units of $10^{-7}$ meters. Since $\rm 7227{pt} = 254{cm}$, and since
\TeX\ works with scaled points where there are $2^{16}$ sp in a point, \TeX\ sets
$|num|/|den|=(254\cdot10^5)/(7227\cdot2^{16})=25400000/473628672$.

The |mag| parameter is what \TeX\ calls \.{\\mag}, i.e., 1000 times the desired
magnification. The actual fraction by which dimensions are multiplied is
therefore $|mag|\cdot|num|/1000|den|$. Note that if a \TeX\ source document does
not call for any `\.{true}' dimensions, and if you change it only by specifying a
different \.{\\mag} setting, the \.{DVI} file that \TeX\ creates will be
completely unchanged except for the value of |mag| in the preamble and postamble.
(Fancy \.{DVI}-reading programs allow users to override the |mag|~setting when a
\.{DVI} file is being printed.)

Finally, |k| and |x| allow the \.{DVI} writer to include a comment, which is not
interpreted further. The length of comment |x| is |k|, where |0<=k<256|.

The next macro identifies the kind of \.{DVI} files described here.

*/

#define id_byte 2

/*tex

Font definitions for a given font number |k| contain further parameters
$$\hbox{|c[4]| |s[4]| |d[4]| |a[1]| |l[1]| |n[a+l]|.}$$ The four-byte value |c|
is the check sum that \TeX\ found in the \.{TFM} file for this font; |c| should
match the check sum of the font found by programs that read this \.{DVI} file.

Parameter |s| contains a fixed-point scale factor that is applied to the
character widths in font |k|; font dimensions in \.{TFM} files and other font
files are relative to this quantity, which is called the ``at size'' elsewhere in
this documentation. The value of |s| is always positive and less than $2^{27}$.
It is given in the same units as the other \.{DVI} dimensions, i.e., in sp when
\TeX82 has made the file. Parameter |d| is similar to |s|; it is the ``design
size,'' and (like~|s|) it is given in \.{DVI} units. Thus, font |k| is to be used
at $|mag|\cdot s/1000d$ times its normal size.

The remaining part of a font definition gives the external name of the font,
which is an ASCII string of length |a+l|. The number |a| is the length of the
``area'' or directory, and |l| is the length of the font name itself; the
standard local system font area is supposed to be used when |a=0|. The |n| field
contains the area in its first |a| bytes.

Font definitions must appear before the first use of a particular font number.
Once font |k| is defined, it must not be defined again; however, we
shall see below that font definitions appear in the postamble as well as
in the pages, so in this sense each font number is defined exactly twice,
if at all. Like |nop| commands, font definitions can
appear before the first |bop|, or between an |eop| and a |bop|.

Sometimes it is desirable to make horizontal or vertical rules line up precisely
with certain features in characters of a font. It is possible to guarantee the
correct matching between \.{DVI} output and the characters generated by \MF\ by
adhering to the following principles: (1)~The \MF\ characters should be
positioned so that a bottom edge or left edge that is supposed to line up with
the bottom or left edge of a rule appears at the reference point, i.e., in row~0
and column~0 of the \MF\ raster. This ensures that the position of the rule will
not be rounded differently when the pixel size is not a perfect multiple of the
units of measurement in the \.{DVI} file. (2)~A typeset rule of height $a>0$ and
width $b>0$ should be equivalent to a \MF-generated character having black pixels
in precisely those raster positions whose \MF\ coordinates satisfy
|0<=x<@t$\alpha$@>b| and |0<=y<@t$\alpha$@>a|, where $\alpha$ is the number of
pixels per \.{DVI} unit.

The last page in a \.{DVI} file is followed by `|post|'; this command introduces
the postamble, which summarizes important facts that \TeX\ has accumulated about
the file, making it possible to print subsets of the data with reasonable
efficiency. The postamble has the form

$$\vbox{\halign{\hbox{#\hfil}\cr
  |post| |p[4]| |num[4]| |den[4]| |mag[4]| |l[4]| |u[4]| |s[2]| |t[2]|\cr
  $\langle\,$font definitions$\,\rangle$\cr
  |post_post| |q[4]| |i[1]| 223's$[{\G}4]$\cr}}$$

Here |p| is a pointer to the final |bop| in the file. The next three parameters,
|num|, |den|, and |mag|, are duplicates of the quantities that appeared in the
preamble.

Parameters |l| and |u| give respectively the height-plus-depth of the tallest
page and the width of the widest page, in the same units as other dimensions of
the file. These numbers might be used by a \.{DVI}-reading program to position
individual ``pages'' on large sheets of film or paper; however, the standard
convention for output on normal size paper is to position each page so that the
upper left-hand corner is exactly one inch from the left and the top. Experience
has shown that it is unwise to design \.{DVI}-to-printer software that attempts
cleverly to center the output; a fixed position of the upper left corner is
easiest for users to understand and to work with. Therefore |l| and~|u| are often
ignored. Parameter |s| is the maximum stack depth (i.e., the largest excess of

|push| commands over |pop| commands) needed to process this file. Then comes |t|,
the total number of pages (|bop| commands) present.

The postamble continues with font definitions, which are any number of
\\{fnt\_def} commands as described above, possibly interspersed with |nop|
commands. Each font number that is used in the \.{DVI} file must be defined
exactly twice: Once before it is first selected by a \\{fnt} command, and once in
the postamble.

The last part of the postamble, following the |post_post| byte that signifies the
end of the font definitions, contains |q|, a pointer to the |post| command that
started the postamble. An identification byte, |i|, comes next; this currently
equals~2, as in the preamble.

The |i| byte is followed by four or more bytes that are all equal to the decimal
number 223 (i.e., '337 in octal). \TeX\ puts out four to seven of these trailing
bytes, until the total length of the file is a multiple of four bytes, since this
works out best on machines that pack four bytes per word; but any number of 223's
is allowed, as long as there are at least four of them. In effect, 223 is a sort
of signature that is added at the very end.

This curious way to finish off a \.{DVI} file makes it feasible for
\.{DVI}-reading programs to find the postamble first, on most computers, even
though \TeX\ wants to write the postamble last. Most operating systems permit
random access to individual words or bytes of a file, so the \.{DVI} reader can
start at the end and skip backwards over the 223's until finding the
identification byte. Then it can back up four bytes, read |q|, and move to byte
|q| of the file. This byte should, of course, contain the value 248 (|post|); now
the postamble can be read, so the \.{DVI} reader can discover all the information
needed for typesetting the pages. Note that it is also possible to skip through
the \.{DVI} file at reasonably high speed to locate a particular page, if that
proves desirable. This saves a lot of time, since \.{DVI} files used in
production jobs tend to be large.

Unfortunately, however, standard \PASCAL\ does not include the ability to access
a random position in a file, or even to determine the length of a file. Almost
all systems nowadays provide the necessary capabilities, so \.{DVI} format has
been designed to work most efficiently with modern operating systems. But if
\.{DVI} files have to be processed under the restrictions of standard \PASCAL,
one can simply read them from front to back, since the necessary header
information is present in the preamble and in the font definitions. (The |l| and
|u| and |s| and |t| parameters, which appear only in the postamble, are
``frills'' that are handy but not absolutely necessary.)

After considering \TeX's eyes and stomach, we come now to the bowels.

The |ship_out| procedure is given a pointer to a box; its mission is to describe
that box in \.{DVI} form, outputting a ``page'' to |dvi_file|. The \.{DVI}
coordinates $(h,v)=(0,0)$ should correspond to the upper left corner of the box
being shipped.

Since boxes can be inside of boxes inside of boxes, the main work of |ship_out|
is done by two mutually recursive routines, |hlist_out| and |vlist_out|, which
traverse the hlists and vlists inside of horizontal and vertical boxes.

As individual pages are being processed, we need to accumulate information about
the entire set of pages, since such statistics must be reported in the postamble.
The global variables |total_pages|, |max_v|, |max_h|, |max_push|, and |last_bop|
are used to record this information.

The variable |doing_leaders| is |true| while leaders are being output. The
variable |dead_cycles| contains the number of times an output routine has been
initiated since the last |ship_out|.

A few additional global variables are also defined here for use in |vlist_out|
and |hlist_out|. They could have been local variables, but that would waste stack
space when boxes are deeply nested, since the values of these variables are not
needed during recursive calls.

*/

/* Some global variables are defined in |backend| module. */

static int max_push = 0;  /* deepest nesting of |push| commands encountered so far */
static int last_bop = -1; /* location of previous |bop| in the \.{DVI} output */
static int oval, ocmd;    /* used by |out_cmd| for generating |set|, |fnt| and |fnt_def| commands */
pointer g;                /* current glue specification */

/*tex

The \.{DVI} bytes are output to a buffer instead of being written directly to the
output file. This makes it possible to reduce the overhead of subroutine calls,
thereby measurably speeding up the computation, since output of \.{DVI} bytes is
part of \TeX's inner loop. And it has another advantage as well, since we can
change instructions in the buffer in order to make the output more compact. For
example, a `|down2|' command can be changed to a `|y2|', thereby making a
subsequent `|y0|' command possible, saving two bytes.

The output buffer is divided into two parts of equal size; the bytes found in
|dvi_buf[0..half_buf-1]| constitute the first half, and those in
|dvi_buf[half_buf..dvi_buf_size-1]| constitute the second. The global variable
|dvi_ptr| points to the position that will receive the next output byte. When
|dvi_ptr| reaches |dvi_limit|, which is always equal to one of the two values
|half_buf| or |dvi_buf_size|, the half buffer that is about to be invaded next is
sent to the output and |dvi_limit| is changed to its other value. Thus, there is
always at least a half buffer's worth of information present, except at the very
beginning of the job.

Bytes of the \.{DVI} file are numbered sequentially starting with 0; the next
byte to be generated will be number |dvi_offset+dvi_ptr|. A byte is present in
the buffer only if its number is |>=dvi_gone|.

Some systems may find it more efficient to make |dvi_buf| a |packed| array, since
output of four bytes at once may be facilitated.

Initially the buffer is all in one piece; we will output half of it only after it
first fills up.

*/

int dvi_buf_size = 800;     /* size of the output buffer; must be a multiple of 8 */
eight_bits *dvi_buf;        /* buffer for \.{DVI} output */
static int half_buf = 0;    /* half of |dvi_buf_size| */
static int dvi_limit = 0;   /* end of the current half buffer */
static int dvi_ptr = 0;     /* the next available buffer address */
static int dvi_offset = 0;  /* |dvi_buf_size| times the number of times the output buffer has been fully emptied */
static int dvi_gone = 0;    /* the number of bytes already output to |dvi_file| */

/*
To put a byte in the buffer without paying the cost of invoking a procedure
each time, we use the macro |dvi_out|.
*/

#define dvi_out(A) do {                 \
	dvi_buf[dvi_ptr++]=(eight_bits)(A);	\
    if (dvi_ptr==dvi_limit) dvi_swap(); \
} while (0)

#define dvi_set(A,B) do {                       \
    oval=A; ocmd=set1; out_cmd(); dvi.h += (B); \
} while (0)

#define dvi_put(A)  do {          \
    oval=A; ocmd=put1; out_cmd(); \
} while (0)

/*
The |vinfo| fields in the entries of the down stack or the right stack
have six possible settings: |y_here| or |z_here| mean that the \.{DVI}
command refers to |y| or |z|, respectively (or to |w| or |x|, in the
case of horizontal motion); |yz_OK| means that the \.{DVI} command is
\\{down} (or \\{right}) but can be changed to either |y| or |z| (or
to either |w| or |x|); |y_OK| means that it is \\{down} and can be changed
to |y| but not |z|; |z_OK| is similar; and |d_fixed| means it must stay
\\{down}.

The four settings |yz_OK|, |y_OK|, |z_OK|, |d_fixed| would not need to
be distinguished from each other if we were simply solving the
digit-subscripting problem mentioned above. But in \TeX's case there is
a complication because of the nested structure of |push| and |pop|
commands. Suppose we add parentheses to the digit-subscripting problem,
redefining hits so that $\delta_y\ldots \delta_y$ is a hit if all $y$'s between
the $\delta$'s are enclosed in properly nested parentheses, and if the
parenthesis level of the right-hand $\delta_y$ is deeper than or equal to
that of the left-hand one. Thus, `(' and `)' correspond to `|push|'
and `|pop|'. Now if we want to assign a subscript to the final 1 in the
sequence
$$2_y\,7_d\,1_d\,(\,8_z\,2_y\,8_z\,)\,1$$
we cannot change the previous $1_d$ to $1_y$, since that would invalidate
the $2_y\ldots2_y$ hit. But we can change it to $1_z$, scoring a hit
since the intervening $8_z$'s are enclosed in parentheses.
*/

typedef enum {
    y_here = 1,                 /* |vinfo| when the movement entry points to a |y| command */
    z_here = 2,                 /* |vinfo| when the movement entry points to a |z| command */
    yz_OK = 3,                  /* |vinfo| corresponding to an unconstrained \\{down} command */
    y_OK = 4,                   /* |vinfo| corresponding to a \\{down} that can't become a |z| */
    z_OK = 5,                   /* |vinfo| corresponding to a \\{down} that can't become a |y| */
    d_fixed = 6,                /* |vinfo| corresponding to a \\{down} that can't change */
} movement_codes;

/* As we search through the stack, we are in one of three states,
   |y_seen|, |z_seen|, or |none_seen|, depending on whether we have
   encountered |y_here| or |z_here| nodes. These states are encoded as
   multiples of 6, so that they can be added to the |info| fields for quick
   decision-making. */

#  define none_seen 0           /* no |y_here| or |z_here| nodes have been encountered yet */
#  define y_seen 6              /* we have seen |y_here| but not |z_here| */
#  define z_seen 12             /* we have seen |z_here| but not |y_here| */

void movement(scaled w, eight_bits o);

/*
extern void prune_movements(int l);
*/

/*
The actual distances by which we want to move might be computed as the
sum of several separate movements. For example, there might be several
glue nodes in succession, or we might want to move right by the width of
some box plus some amount of glue. More importantly, the baselineskip
distances are computed in terms of glue together with the depth and
height of adjacent boxes, and we want the \.{DVI} file to lump these
three quantities together into a single motion.

Therefore, \TeX\ maintains two pairs of global variables: |dvi.h| and |dvi.v|
are the |h| and |v| coordinates corresponding to the commands actually
output to the \.{DVI} file, while |cur.h| and |cur.v| are the coordinates
corresponding to the current state of the output routines. Coordinate
changes will accumulate in |cur.h| and |cur.v| without being reflected
in the output, until such a change becomes necessary or desirable; we
can call the |movement| procedure whenever we want to make |dvi.h=pos.h|
or |dvi.v=pos.v|.

The current font reflected in the \.{DVI} output is called |dvi_f|;
there is no need for a `\\{cur\_f}' variable.

The depth of nesting of |hlist_out| and |vlist_out| is called |cur_s|;
this is essentially the depth of |push| commands in the \.{DVI} output.
*/

/*tex A \.{DVI} position in page coordinates, in sync with DVI file: */

static scaledpos dvi;

#  define synch_h(p) do {            \
    if (p.h != dvi.h) {              \
      movement(p.h - dvi.h, right1); \
      dvi.h = p.h;                   \
    }                                \
  } while (0)

#  define synch_v(p) do {            \
    if (p.v != dvi.v) {              \
      movement(dvi.v - p.v, down1);  \
      dvi.v = p.v;                   \
    }                                \
  } while (0)

#  define synch_dvi_with_pos(p) do {synch_h(p); synch_v(p); } while (0)

/*tex

The actual output of |dvi_buf[a..b]| to |dvi_file| is performed by calling
|write_dvi(a,b)|. For best results, this procedure should be optimized to run as
fast as possible on each particular system, since it is part of \TeX's inner
loop. It is safe to assume that |a| and |b+1| will both be multiples of 4 when
|write_dvi(a,b)| is called; therefore it is possible on many machines to use
efficient methods to pack four bytes per word and to output an array of words
with one system call.

*/

static void write_dvi(int a, int b)
{
    int k;
    for (k = a; k <= b; k++)
        fputc(dvi_buf[k], static_pdf->file);
}

/*tex This outputs half of the buffer: */

static void dvi_swap(void)
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

/*tex

The |dvi_four| procedure outputs four bytes in two's complement notation, without
risking arithmetic overflow.

*/

static void dvi_four(int x)
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

/*tex

A mild optimization of the output is performed by the |dvi_pop| routine, which
issues a |pop| unless it is possible to cancel a `|push| |pop|' pair. The
parameter to |dvi_pop| is the byte address following the old |push| that matches
the new |pop|.

*/

static void dvi_push(void)
{
    dvi_out(push);
}

static void dvi_pop(int l)
{
    if ((l == dvi_offset + dvi_ptr) && (dvi_ptr > 0))
        decr(dvi_ptr);
    else
        dvi_out(pop);
}

/*tex

Here's a procedure that outputs a font definition. $\Omega$ allows more than 256
different fonts per job, so the right font definition command must be selected.

*/

static void out_cmd(void)
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

static void dvi_font_def(internal_font_number f)
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

/*tex

Versions of \TeX\ intended for small computers might well choose to omit the
ideas in the next few parts of this program, since it is not really necessary to
optimize the \.{DVI} code by making use of the |w0|, |x0|, |y0|, and |z0|
commands. Furthermore, the algorithm that we are about to describe does not
pretend to give an optimum reduction in the length of the \.{DVI} code; after
all, speed is more important than compactness. But the method is surprisingly
effective, and it takes comparatively little time.

We can best understand the basic idea by first considering a simpler problem that
has the same essential characteristics. Given a sequence of digits, say
$3\,1\,4\,1\,5\,9\,2\,6\,5\,3\,5\,8\,9$, we want to assign subscripts $d$, $y$,
or $z$ to each digit so as to maximize the number of ``$y$-hits'' and
``$z$-hits''; a $y$-hit is an instance of two appearances of the same digit with
the subscript $y$, where no $y$'s intervene between the two appearances, and a
$z$-hit is defined similarly. For example, the sequence above could be decorated
with subscripts as follows:
$$3_z\,1_y\,4_d\,1_y\,5_y\,9_d\,2_d\,6_d\,5_y\,3_z\,5_y\,8_d\,9_d.$$ There are
three $y$-hits ($1_y\ldots1_y$ and $5_y\ldots5_y\ldots5_y$) and one $z$-hit
($3_z\ldots3_z$); there are no $d$-hits, since the two appearances of $9_d$ have
$d$'s between them, but we don't count $d$-hits so it doesn't matter how many
there are. These subscripts are analogous to the \.{DVI} commands called
\\{down}, $y$, and $z$, and the digits are analogous to different amounts of
vertical motion; a $y$-hit or $z$-hit corresponds to the opportunity to use the
one-byte commands |y0| or |z0| in a \.{DVI} file.

\TeX's method of assigning subscripts works like this: Append a new digit, say
$\delta$, to the right of the sequence. Now look back through the sequence until
one of the following things happens: (a)~You see $\delta_y$ or $\delta_z$, and
this was the first time you encountered a $y$ or $z$ subscript, respectively.
Then assign $y$ or $z$ to the new $\delta$; you have scored a hit. (b)~You see
$\delta_d$, and no $y$ subscripts have been encountered so far during this
search. Then change the previous $\delta_d$ to $\delta_y$ (this corresponds to
changing a command in the output buffer), and assign $y$ to the new $\delta$;
it's another hit. (c)~You see $\delta_d$, and a $y$ subscript has been seen but
not a $z$. Change the previous $\delta_d$ to $\delta_z$ and assign $z$ to the new
$\delta$. (d)~You encounter both $y$ and $z$ subscripts before encountering a
suitable $\delta$, or you scan all the way to the front of the sequence. Assign
$d$ to the new $\delta$; this assignment may be changed later.

The subscripts $3_z\,1_y\,4_d\ldots\,$ in the example above were, in fact,
produced by this procedure, as the reader can verify. (Go ahead and try it.)

In order to implement such an idea, \TeX\ maintains a stack of pointers to the
\\{down}, $y$, and $z$ commands that have been generated for the current page.
And there is a similar stack for \\{right}, |w|, and |x| commands. These stacks
are called the down stack and right stack, and their top elements are maintained
in the variables |down_ptr| and |right_ptr|.

Each entry in these stacks contains four fields: The |width| field is the amount
of motion down or to the right; the |location| field is the byte number of the
\.{DVI} command in question (including the appropriate |dvi_offset|); the |vlink|
field points to the next item below this one on the stack; and the |vinfo| field
encodes the options for possible change in the \.{DVI} command.

*/

/*tex The \.{DVI} byte number for a movement command: */

#define location(A) varmem[(A)+1].cint

/*tex The heads of the down and right stacks: */

static halfword down_ptr = null;
static halfword right_ptr = null;

/*tex

Here is a subroutine that produces a \.{DVI} command for some specified downward
or rightward motion. It has two parameters: |w| is the amount of motion, and |o|
is either |down1| or |right1|. We use the fact that the command codes have
convenient arithmetic properties: |y1-down1=w1-right1| and |z1-down1=x1-right1|.

*/

void movement(scaled w, eight_bits o)
{
    small_number mstate;        /* have we seen a |y| or |z|? */
    halfword p, q;              /* current and top nodes on the stack */
    int k;                      /* index into |dvi_buf|, modulo |dvi_buf_size| */
    /*tex something todo? */
    if (false) {
        /*tex new node for the top of the stack */
        q = new_node(movement_node, 0);
        width(q) = w;
        location(q) = dvi_offset + dvi_ptr;
        if (o == down1) {
            vlink(q) = down_ptr;
            down_ptr = q;
        } else {
            vlink(q) = right_ptr;
            right_ptr = q;
        }
        /*tex

            Look at the other stack entries until deciding what sort of \.{DVI}
            command to generate; |goto found| if node |p| is a ``hit''.
        */
        p = vlink(q);
        mstate = none_seen;
        while (p != null) {
            if (width(p) == w) {
                /*
                    Consider a node with matching width;|goto found| if it's a
                    hit. We might find a valid hit in a |y| or |z| byte that is
                    already gone from the buffer. But we can't change bytes that
                    are gone forever; ``the moving finger writes, $\ldots\,\,$.''
                */
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
    /*tex
        Generate a |down| or |right| command for |w| and |return|:
    */
    if (abs(w) >= 040000000) {
        /*tex |down4| or |right4| */
        dvi_out(o + 3);
        dvi_four(w);
        return;
    }
    if (abs(w) >= 0100000) {
        /*tex |down3| or |right3| */
        dvi_out(o + 2);
        if (w < 0)
            w = w + 0100000000;
        dvi_out(w / 0200000);
        w = w % 0200000;
        goto TWO;
    }
    if (abs(w) >= 0200) {
        /*tex |down2| or |right2| */
        dvi_out(o + 1);
        if (w < 0)
            w = w + 0200000;
        goto TWO;
    }
    /*tex |down1| or |right1| */
    dvi_out(o);
    if (w < 0)
        w = w + 0400;
    goto ONE;
  TWO:
    dvi_out(w / 0400);
  ONE:
    dvi_out(w % 0400);
    return;
  FOUND:
    /*tex

        Generate a |y0| or |z0| command in order to reuse a previous appearance
        of~|w|.

        The program below removes movement nodes that are introduced after a
        |push|, before it outputs the corresponding |pop|.

        When the |movement| procedure gets to the label |found|, the value of
        |vinfo(p)| will be either |y_here| or |z_here|. If it is, say, |y_here|,
        the procedure generates a |y0| command (or a |w0| command), and marks all
        |vinfo| fields between |q| and |p| so that |y| is not OK in that range.

     */
    vinfo(q) = vinfo(p);
    if (vinfo(q) == y_here) {
        /*tex |y0| or |w0| */
        dvi_out(o + y0 - down1);
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
        /*tex |z0| or |x0| */
        dvi_out(o + z0 - down1);
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

/*tex

In case you are wondering when all the movement nodes are removed from \TeX's
memory, the answer is that they are recycled just before |hlist_out| and
|vlist_out| finish outputting a box. This restores the down and right stacks to
the state they were in before the box was output, except that some |vinfo|'s may
have become more restrictive.

Here we delete movement nodes with |location>=l|:

*/

static void prune_movements(int l)
{
    pointer p;
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

/*tex

When |hlist_out| is called, its duty is to output the box represented by the
|hlist_node| pointed to by |temp_ptr|. The reference point of that box has
coordinates |(cur.h,cur.v)|.

Similarly, when |vlist_out| is called, its duty is to output the box represented
by the |vlist_node| pointed to by |temp_ptr|. The reference point of that box has
coordinates |(cur.h,cur.v)|.

The recursive procedures |hlist_out| and |vlist_out| each have a local variable
|save_dvi| to hold the value of |dvi| just before entering a new level of
recursion. In effect, the value of |save_dvi| on \TeX's run-time stack
corresponds to the values of |h| and |v| that a \.{DVI}-reading program will push
onto its coordinate stack.

*/

void dvi_place_rule(PDF pdf, halfword q, scaledpos size)
{
    synch_dvi_with_pos(pdf->posstruct->pos);
    if ((subtype(q) >= box_rule) && (subtype(q) <= user_rule)) {
        /*tex place nothing, only take space */
        if (textdir_is_L(pdf->posstruct->dir))
            dvi.h += size.h;
    } else {
        /*tex normal_rule or >= 100 being a leader rule */
        if (textdir_is_L(pdf->posstruct->dir)) {
            /*tex movement optimization for |dir_*L*| */
            dvi_out(set_rule);
            dvi.h += size.h;
        } else
            dvi_out(put_rule);
    }
    dvi_four(size.v);
    dvi_four(size.h);
}

void dvi_place_glyph(PDF pdf, internal_font_number f, int c, int ex)
{
    scaled_whd ci;
    synch_dvi_with_pos(pdf->posstruct->pos);
    if (f != pdf->f_cur) {
        /*tex Change font |f_cur| to |f| */
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
        /*tex movement optimization for |dir_*L*| */
        dvi_set(c, ci.wd);
    } else {
        dvi_put(c);
    }
}

void dvi_special(PDF pdf, halfword p)
{
    /*tex holds print |selector| */
    int old_setting;
    /*tex index into |cur_string| */
    unsigned k;
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
    for (k = 0; k < cur_length; k++) {
        dvi_out(cur_string[k]);
    }
    /*tex erase the string */
    cur_length = 0;
}

/*tex

Here's an example of how these conventions are used. Whenever it is time to ship
out a box of stuff, we shall use the macro |ensure_dvi_open|.

*/

void dvi_write_header(PDF pdf)
{
    unsigned l;
    /*tex index into |str_pool| */
    unsigned s;
    /*tex saved |selector| setting */
    int old_setting;
    if (half_buf == 0) {
        half_buf = dvi_buf_size / 2;
        dvi_limit = dvi_buf_size;
    }
    dvi_out(pre);
    /*tex output the preamble */
    dvi_out(id_byte);
    dvi_four(25400000);
    /*tex conversion ratio for sp */
    dvi_four(473628672);
    prepare_mag();
    /*tex magnification factor is frozen */
    dvi_four(mag_par);
    if (output_comment) {
        l = (unsigned) strlen(output_comment);
        dvi_out(l);
        for (s = 0; s < l; s++) {
            dvi_out(output_comment[s]);
        }
    } else {
        /*tex the default code is unchanged */
        old_setting = selector;
        selector = new_string;
        tprint(" LuaTeX output ");
        print_int(year_par);
        print_char('.');
        print_two(month_par);
        print_char('.');
        print_two(day_par);
        print_char(':');
        print_two(time_par / 60);
        print_two(time_par % 60);
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
    /*tex location of the current |bop| */
    int page_loc;
    ensure_output_state(pdf, ST_HEADER_WRITTEN);
    /*tex Initialize variables as |ship_out| begins */
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
}

/*tex

At the end of the program, we must finish things off by writing the post\-amble.
If |total_pages=0|, the \.{DVI} file was never opened. If |total_pages>=65536|,
the \.{DVI} file will lie. And if |max_push>=65536|, the user deserves whatever
chaos might ensue.

*/

void dvi_open_file(PDF pdf) {
    ensure_output_file_open(pdf, ".dvi");
}

void dvi_finish_file(PDF pdf, int fatal_error)
{
    int k;
    int callback_id = callback_defined(stop_run_callback);
    if (fatal_error) {
        print_err(" ==> Fatal error occurred, bad output DVI file produced!");
    }
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
        /*tex beginning of the postamble */
        dvi_out(post);
        dvi_four(last_bop);
        last_bop = dvi_offset + dvi_ptr - 5;
        /*tex |post| location */
        dvi_four(25400000);
        /*tex conversion ratio for sp */
        dvi_four(473628672);
        prepare_mag();
        /*tex magnification factor */
        dvi_four(mag_par);
        dvi_four(max_v);
        dvi_four(max_h);
        dvi_out(max_push / 256);
        dvi_out(max_push % 256);
        dvi_out((total_pages / 256) % 256);
        dvi_out(total_pages % 256);
        /*tex Output the font definitions for all fonts that were used */
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
        /*tex the number of 223's */
#ifndef IPC
        k = 4 + ((dvi_buf_size - dvi_ptr) % 4);
#else
        k = 7 - ((3 + dvi_offset + dvi_ptr) % 4);
#endif
        while (k > 0) {
            dvi_out(223);
            decr(k);
        }
        /*tex
            Here is how we clean out the buffer when \TeX\ is all through;
            |dvi_ptr| will be a multiple of~4.
        */
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

void dvi_push_list(PDF pdf, scaledpos *saved_pos, int *saved_loc)
{
    if (cur_s > max_push) {
        max_push = cur_s;
    }
    if (cur_s > 0) {
        dvi_push();
        *saved_pos = dvi;
    }
    *saved_loc = dvi_offset + dvi_ptr;
}

void dvi_pop_list(PDF pdf, scaledpos *saved_pos, int *saved_loc)
{
    prune_movements(*saved_loc);
    if (cur_s > 0) {
        dvi_pop(*saved_loc);
        dvi = *saved_pos;
    }
}

void dvi_set_reference_point(PDF pdf, posstructure *refpoint)
{
    refpoint->pos.h = one_true_inch;
    refpoint->pos.v = pdf->page_size.v - one_true_inch;
    dvi = refpoint->pos;
}

int dvi_get_status_ptr(PDF pdf)
{
    return dvi_ptr;
}

int dvi_get_status_gone(PDF pdf)
{
    return dvi_gone;
}
