%% Support for some primitives defined in pdfTeX
%%
%% \pdfstrcmp: need for LaTeX3
%%   In comparison, Japanese characters will be always encoded in UTF-8.
%%
%% \pdffilemoddate and co.: for standalone package
%%   (\pdfcreationdate, \pdffilemoddate, \pdffilesize)
%%
%% \pdfsavepos and co.
%%   (\pdfsavepos, \pdfpage{width,height}, \pdflast{x,y}pos)
%%
%% \pdffiledump: for bmpsize package by Heiko Oberdiek
%%
%% \pdfshellescape: by doraTeX's request
%%
%% \pdfmdfivesum: by Akira's request
%%   As \pdfstrcmp, Japanese characters will be always encoded in UTF-8 in
%%   \pdfmdfivesum {...}. (no conversion for \pdfmdfivesum file <filename>)
%%
%% \pdfprimitive and \ifpdfprimitive: for LaTeX3 (2015/07/15)
%%
%% \pdfuniformdeviate and co.:
%%  (\pdfnormaldeviate, \pdfrandomseed, \pdfsetrandomseed)
%%
%% \pdfelapsedtime and \pdfresettimer
%%
%% \expanded
%%
%% \ifincsname
%%
%% \Uchar, \Ucharcat
%%
%% \vadjust pre (2021-07-01)

@x
@* \[8] Packed data.
@y
@* \[7b] Random numbers.

\font\tenlogo=logo10 % font used for the METAFONT logo
\def\MP{{\tenlogo META}\-{\tenlogo POST}}
\def\pdfTeX{pdf\TeX}

This section is (almost) straight from MetaPost. I had to change
the types (use |integer| instead of |fraction|), but that should
not have any influence on the actual calculations (the original
comments refer to quantities like |fraction_four| ($2^{30}$), and
that is the same as the numeric representation of |maxdimen|).

I've copied the low-level variables and routines that are needed, but
only those (e.g. |m_log|), not the accompanying ones like |m_exp|. Most
of the following low-level numeric routines are only needed within the
calculation of |norm_rand|. I've been forced to rename |make_fraction|
to |make_frac| because TeX already has a routine by that name with
a wholly different function (it creates a |fraction_noad| for math
typesetting) -- Taco

And now let's complete our collection of numeric utility routines
by considering random number generation.
\MP\ generates pseudo-random numbers with the additive scheme recommended
in Section 3.6 of {\sl The Art of Computer Programming}; however, the
results are random fractions between 0 and |fraction_one-1|, inclusive.

There's an auxiliary array |randoms| that contains 55 pseudo-random
fractions. Using the recurrence $x_n=(x_{n-55}-x_{n-31})\bmod 2^{28}$,
we generate batches of 55 new $x_n$'s at a time by calling |new_randoms|.
The global variable |j_random| tells which element has most recently
been consumed.

@<Glob...@>=
@!randoms:array[0..54] of integer; {the last 55 random values generated}
@!j_random:0..54; {the number of unused |randoms|}
@!random_seed:scaled; {the default random seed}

@ A small bit of metafont is needed.

@d fraction_half==@'1000000000 {$2^{27}$, represents 0.50000000}
@d fraction_one==@'2000000000 {$2^{28}$, represents 1.00000000}
@d fraction_four==@'10000000000 {$2^{30}$, represents 4.00000000}
@d el_gordo == @'17777777777 {$2^{31}-1$, the largest value that \MP\ likes}
@d halfp(#)==(#) div 2
@d double(#) == #:=#+# {multiply a variable by two}

@ The |make_frac| routine produces the |fraction| equivalent of
|p/q|, given integers |p| and~|q|; it computes the integer
$f=\lfloor2^{28}p/q+{1\over2}\rfloor$, when $p$ and $q$ are
positive. If |p| and |q| are both of the same scaled type |t|,
the ``type relation'' |make_frac(t,t)=fraction| is valid;
and it's also possible to use the subroutine ``backwards,'' using
the relation |make_frac(t,fraction)=t| between scaled types.

If the result would have magnitude $2^{31}$ or more, |make_frac|
sets |arith_error:=true|. Most of \MP's internal computations have
been designed to avoid this sort of error.

If this subroutine were programmed in assembly language on a typical
machine, we could simply compute |(@t$2^{28}$@>*p)div q|, since a
double-precision product can often be input to a fixed-point division
instruction. But when we are restricted to \PASCAL\ arithmetic it
is necessary either to resort to multiple-precision maneuvering
or to use a simple but slow iteration. The multiple-precision technique
would be about three times faster than the code adopted here, but it
would be comparatively long and tricky, involving about sixteen
additional multiplications and divisions.

This operation is part of \MP's ``inner loop''; indeed, it will
consume nearly 10\pct! of the running time (exclusive of input and output)
if the code below is left unchanged. A machine-dependent recoding
will therefore make \MP\ run faster. The present implementation
is highly portable, but slow; it avoids multiplication and division
except in the initial stage. System wizards should be careful to
replace it with a routine that is guaranteed to produce identical
results in all cases.
@^system dependencies@>

As noted below, a few more routines should also be replaced by machine-dependent
code, for efficiency. But when a procedure is not part of the ``inner loop,''
such changes aren't advisable; simplicity and robustness are
preferable to trickery, unless the cost is too high.
@^inner loop@>

@p function make_frac(@!p,@!q:integer):integer;
var @!f:integer; {the fraction bits, with a leading 1 bit}
@!n:integer; {the integer part of $\vert p/q\vert$}
@!negative:boolean; {should the result be negated?}
@!be_careful:integer; {disables certain compiler optimizations}
begin if p>=0 then negative:=false
else  begin negate(p); negative:=true;
  end;
if q<=0 then
  begin debug if q=0 then confusion("/");@;@+gubed@;@/
@:this can't happen /}{\quad \./@>
  negate(q); negative:=not negative;
  end;
n:=p div q; p:=p mod q;
if n>=8 then
  begin arith_error:=true;
  if negative then make_frac:=-el_gordo@+else make_frac:=el_gordo;
  end
else  begin n:=(n-1)*fraction_one;
  @<Compute $f=\lfloor 2^{28}(1+p/q)+{1\over2}\rfloor$@>;
  if negative then make_frac:=-(f+n)@+else make_frac:=f+n;
  end;
end;

@ The |repeat| loop here preserves the following invariant relations
between |f|, |p|, and~|q|:
(i)~|0<=p<q|; (ii)~$fq+p=2^k(q+p_0)$, where $k$ is an integer and
$p_0$ is the original value of~$p$.

Notice that the computation specifies
|(p-q)+p| instead of |(p+p)-q|, because the latter could overflow.
Let us hope that optimizing compilers do not miss this point; a
special variable |be_careful| is used to emphasize the necessary
order of computation. Optimizing compilers should keep |be_careful|
in a register, not store it in memory.
@^inner loop@>

@<Compute $f=\lfloor 2^{28}(1+p/q)+{1\over2}\rfloor$@>=
f:=1;
repeat be_careful:=p-q; p:=be_careful+p;
if p>=0 then f:=f+f+1
else  begin double(f); p:=p+q;
  end;
until f>=fraction_one;
be_careful:=p-q;
if be_careful+p>=0 then incr(f)

@

@p function take_frac(@!q:integer;@!f:integer):integer;
var @!p:integer; {the fraction so far}
@!negative:boolean; {should the result be negated?}
@!n:integer; {additional multiple of $q$}
@!be_careful:integer; {disables certain compiler optimizations}
begin @<Reduce to the case that |f>=0| and |q>0|@>;
if f<fraction_one then n:=0
else  begin n:=f div fraction_one; f:=f mod fraction_one;
  if q<=el_gordo div n then n:=n*q
  else  begin arith_error:=true; n:=el_gordo;
    end;
  end;
f:=f+fraction_one;
@<Compute $p=\lfloor qf/2^{28}+{1\over2}\rfloor-q$@>;
be_careful:=n-el_gordo;
if be_careful+p>0 then
  begin arith_error:=true; n:=el_gordo-p;
  end;
if negative then take_frac:=-(n+p)
else take_frac:=n+p;
end;

@ @<Reduce to the case that |f>=0| and |q>0|@>=
if f>=0 then negative:=false
else  begin negate(f); negative:=true;
  end;
if q<0 then
  begin negate(q); negative:=not negative;
  end;

@ The invariant relations in this case are (i)~$\lfloor(qf+p)/2^k\rfloor
=\lfloor qf_0/2^{28}+{1\over2}\rfloor$, where $k$ is an integer and
$f_0$ is the original value of~$f$; (ii)~$2^k\L f<2^{k+1}$.
@^inner loop@>

@<Compute $p=\lfloor qf/2^{28}+{1\over2}\rfloor-q$@>=
p:=fraction_half; {that's $2^{27}$; the invariants hold now with $k=28$}
if q<fraction_four then
  repeat if odd(f) then p:=halfp(p+q)@+else p:=halfp(p);
  f:=halfp(f);
  until f=1
else  repeat if odd(f) then p:=p+halfp(q-p)@+else p:=halfp(p);
  f:=halfp(f);
  until f=1

@ The subroutines for logarithm and exponential involve two tables.
The first is simple: |two_to_the[k]| equals $2^k$. The second involves
a bit more calculation, which the author claims to have done correctly:
|spec_log[k]| is $2^{27}$ times $\ln\bigl(1/(1-2^{-k})\bigr)=
2^{-k}+{1\over2}2^{-2k}+{1\over3}2^{-3k}+\cdots\,$, rounded to the
nearest integer.

@<Glob...@>=
@!two_to_the:array[0..30] of integer; {powers of two}
@!spec_log:array[1..28] of integer; {special logarithms}


@ @<Set init...@>=
two_to_the[0]:=1;
for k:=1 to 30 do two_to_the[k]:=2*two_to_the[k-1];
spec_log[1]:=93032640;
spec_log[2]:=38612034;
spec_log[3]:=17922280;
spec_log[4]:=8662214;
spec_log[5]:=4261238;
spec_log[6]:=2113709;
spec_log[7]:=1052693;
spec_log[8]:=525315;
spec_log[9]:=262400;
spec_log[10]:=131136;
spec_log[11]:=65552;
spec_log[12]:=32772;
spec_log[13]:=16385;
for k:=14 to 27 do spec_log[k]:=two_to_the[27-k];
spec_log[28]:=1;

@

@p function m_log(@!x:integer):integer;
var @!y,@!z:integer; {auxiliary registers}
@!k:integer; {iteration counter}
begin if x<=0 then @<Handle non-positive logarithm@>
else  begin y:=1302456956+4-100; {$14\times2^{27}\ln2\approx1302456956.421063$}
  z:=27595+6553600; {and $2^{16}\times .421063\approx 27595$}
  while x<fraction_four do
    begin double(x); y:=y-93032639; z:=z-48782;
    end; {$2^{27}\ln2\approx 93032639.74436163$
      and $2^{16}\times.74436163\approx 48782$}
  y:=y+(z div unity); k:=2;
  while x>fraction_four+4 do
    @<Increase |k| until |x| can be multiplied by a
      factor of $2^{-k}$, and adjust $y$ accordingly@>;
  m_log:=y div 8;
  end;
end;

@ @<Increase |k| until |x| can...@>=
begin z:=((x-1) div two_to_the[k])+1; {$z=\lceil x/2^k\rceil$}
while x<fraction_four+z do
  begin z:=halfp(z+1); k:=k+1;
  end;
y:=y+spec_log[k]; x:=x-z;
end

@ @<Handle non-positive logarithm@>=
begin print_err("Logarithm of ");
@.Logarithm...replaced by 0@>
print_scaled(x); print(" has been replaced by 0");
help2("Since I don't take logs of non-positive numbers,")@/
  ("I'm zeroing this one. Proceed, with fingers crossed.");
error; m_log:=0;
end

@ The following somewhat different subroutine tests rigorously if $ab$ is
greater than, equal to, or less than~$cd$,
given integers $(a,b,c,d)$. In most cases a quick decision is reached.
The result is $+1$, 0, or~$-1$ in the three respective cases.

@d return_sign(#)==begin ab_vs_cd:=#; return;
  end

@p function ab_vs_cd(@!a,b,c,d:integer):integer;
label exit;
var @!q,@!r:integer; {temporary registers}
begin @<Reduce to the case that |a,c>=0|, |b,d>0|@>;
loop@+  begin q := a div d; r := c div b;
  if q<>r then
    if q>r then return_sign(1)@+else return_sign(-1);
  q := a mod d; r := c mod b;
  if r=0 then
    if q=0 then return_sign(0)@+else return_sign(1);
  if q=0 then return_sign(-1);
  a:=b; b:=q; c:=d; d:=r;
  end; {now |a>d>0| and |c>b>0|}
exit:end;

@ @<Reduce to the case that |a...@>=
if a<0 then
  begin negate(a); negate(b);
  end;
if c<0 then
  begin negate(c); negate(d);
  end;
if d<=0 then
  begin if b>=0 then
    if ((a=0)or(b=0))and((c=0)or(d=0)) then return_sign(0)
    else return_sign(1);
  if d=0 then
    if a=0 then return_sign(0)@+else return_sign(-1);
  q:=a; a:=c; c:=q; q:=-b; b:=-d; d:=q;
  end
else if b<=0 then
  begin if b<0 then if a>0 then return_sign(-1);
  if c=0 then return_sign(0) else return_sign(-1);
  end

@ To consume a random integer, the program below will say `|next_random|'
and then it will fetch |randoms[j_random]|.

@d next_random==if j_random=0 then new_randoms
  else decr(j_random)

@p procedure new_randoms;
var @!k:0..54; {index into |randoms|}
@!x:integer; {accumulator}
begin for k:=0 to 23 do
  begin x:=randoms[k]-randoms[k+31];
  if x<0 then x:=x+fraction_one;
  randoms[k]:=x;
  end;
for k:=24 to 54 do
  begin x:=randoms[k]-randoms[k-24];
  if x<0 then x:=x+fraction_one;
  randoms[k]:=x;
  end;
j_random:=54;
end;

@ To initialize the |randoms| table, we call the following routine.

@p procedure init_randoms(@!seed:integer);
var @!j,@!jj,@!k:integer; {more or less random integers}
@!i:0..54; {index into |randoms|}
begin j:=abs(seed);
while j>=fraction_one do j:=halfp(j);
k:=1;
for i:=0 to 54 do
  begin jj:=k; k:=j-k; j:=jj;
  if k<0 then k:=k+fraction_one;
  randoms[(i*21)mod 55]:=j;
  end;
new_randoms; new_randoms; new_randoms; {``warm up'' the array}
end;

@ To produce a uniform random number in the range |0<=u<x| or |0>=u>x|
or |0=u=x|, given a |scaled| value~|x|, we proceed as shown here.

Note that the call of |take_frac| will produce the values 0 and~|x|
with about half the probability that it will produce any other particular
values between 0 and~|x|, because it rounds its answers.

@p function unif_rand(@!x:integer):integer;
var @!y:integer; {trial value}
begin next_random; y:=take_frac(abs(x),randoms[j_random]);
if y=abs(x) then unif_rand:=0
else if x>0 then unif_rand:=y
else unif_rand:=-y;
end;

@ Finally, a normal deviate with mean zero and unit standard deviation
can readily be obtained with the ratio method (Algorithm 3.4.1R in
{\sl The Art of Computer Programming\/}).

@p function norm_rand:integer;
var @!x,@!u,@!l:integer; {what the book would call $2^{16}X$, $2^{28}U$,
  and $-2^{24}\ln U$}
begin repeat
  repeat next_random;
  x:=take_frac(112429,randoms[j_random]-fraction_half);
    {$2^{16}\sqrt{8/e}\approx 112428.82793$}
  next_random; u:=randoms[j_random];
  until abs(x)<u;
x:=make_frac(x,u);
l:=139548960-m_log(u); {$2^{24}\cdot12\ln2\approx139548959.6165$}
until ab_vs_cd(1024,l,x,x)>=0;
norm_rand:=x;
end;
@* \[8] Packed data.
@z

% [10] \vadjust pre
@x
@d adjust_ptr(#)==mem[#+1].int
  {vertical list to be moved out of horizontal list}
@y
@d adjust_pre == subtype  {<>0 => pre-adjustment}
@#{|append_list| is used to append a list to |tail|}
@d append_list(#) == begin link(tail) := link(#); append_list_end
@d append_list_end(#) == tail := #; end

@d adjust_ptr(#)==mem[#+1].int
  {vertical list to be moved out of horizontal list}
@z

% [11] \vadjust pre
@x
@d hi_mem_stat_min==mem_top-13 {smallest statically allocated word in
  the one-word |mem|}
@d hi_mem_stat_usage=14 {the number of one-word nodes always present}
@y
@d pre_adjust_head==mem_top-14  {head of pre-adjustment list returned by |hpack|}
@d hi_mem_stat_min==mem_top-14 {smallest statically allocated word in
  the one-word |mem|}
@d hi_mem_stat_usage=15 {the number of one-word nodes always present}
@z

@x
@* \[12] Displaying boxes.
@y
@<Declare procedures that need to be declared forward for \pdfTeX@>@;

@* \[12] Displaying boxes.
@z

% [12] \vadjust pre
@x
@ @<Display adjustment |p|@>=
begin print_esc("vadjust"); node_list_display(adjust_ptr(p)); {recursive call}
end
@y
@ @<Display adjustment |p|@>=
begin print_esc("vadjust"); if adjust_pre(p) <> 0 then print(" pre ");
node_list_display(adjust_ptr(p)); {recursive call}
end
@z

@x \[if]pdfprimitive
@d frozen_special=frozen_control_sequence+10
  {permanent `\.{\\special}'}
@d frozen_null_font=frozen_control_sequence+11
  {permanent `\.{\\nullfont}'}
@y
@d frozen_special=frozen_control_sequence+10
  {permanent `\.{\\special}'}
@d frozen_primitive=frozen_control_sequence+11
  {permanent `\.{\\pdfprimitive}'}
@d prim_eqtb_base=frozen_primitive+1
@d prim_size=2100 {maximum number of primitives }
@d frozen_null_font=prim_eqtb_base+prim_size+1
  {permanent `\.{\\nullfont}'}
@z

@x
@d dimen_pars=23 {total number of dimension parameters}
@y
@d pdf_page_width_code=23  {page width}
@d pdf_page_height_code=24 {page height}
@d dimen_pars=25 {total number of dimension parameters}
@z

@x \pdfpage{width,height}
@d emergency_stretch==dimen_par(emergency_stretch_code)
@y
@d emergency_stretch==dimen_par(emergency_stretch_code)
@d pdf_page_width==dimen_par(pdf_page_width_code)
@d pdf_page_height==dimen_par(pdf_page_height_code)
@z

@x \pdfpage{width,height}
emergency_stretch_code:print_esc("emergencystretch");
@y
emergency_stretch_code:print_esc("emergencystretch");
pdf_page_width_code:    print_esc("pdfpagewidth");
pdf_page_height_code:   print_esc("pdfpageheight");
@z

@x \[if]pdfprimitive
@!cs_count:integer; {total number of known identifiers}
@y
@!cs_count:integer; {total number of known identifiers}

@ Primitive support needs a few extra variables and definitions

@d prim_prime=1777 {about 85\pct! of |primitive_size|}
@d prim_base=1
@d prim_next(#) == prim[#].lh {link for coalesced lists}
@d prim_text(#) == prim[#].rh {string number for control sequence name, plus one}
@d prim_is_full == (prim_used=prim_base) {test if all positions are occupied}
@d prim_eq_level_field(#)==#.hh.b1
@d prim_eq_type_field(#)==#.hh.b0
@d prim_equiv_field(#)==#.hh.rh
@d prim_eq_level(#)==prim_eq_level_field(eqtb[prim_eqtb_base+#]) {level of definition}
@d prim_eq_type(#)==prim_eq_type_field(eqtb[prim_eqtb_base+#]) {command code for equivalent}
@d prim_equiv(#)==prim_equiv_field(eqtb[prim_eqtb_base+#]) {equivalent value}
@d undefined_primitive=0
@d biggest_char=255 { 65535 in XeTeX }

@<Glob...@>=
@!prim: array [0..prim_size] of two_halves;  {the primitives table}
@!prim_used:pointer; {allocation pointer for |prim|}
@z

@x \[if]pdfprimitive
@ @<Set init...@>=
no_new_control_sequence:=true; {new identifiers are usually forbidden}
@y
@ @<Set init...@>=
no_new_control_sequence:=true; {new identifiers are usually forbidden}
prim_next(0):=0; prim_text(0):=0;
for k:=1 to prim_size do prim[k]:=prim[0];
@z

@x \[if]pdfprimitive
text(frozen_dont_expand):="notexpanded:";
@.notexpanded:@>
@y
prim_used:=prim_size; {nothing is used}
text(frozen_dont_expand):="notexpanded:";
@.notexpanded:@>
eq_type(frozen_primitive):=ignore_spaces;
equiv(frozen_primitive):=1;
eq_level(frozen_primitive):=level_one;
text(frozen_primitive):="pdfprimitive";
@z

@x \[if]pdfprimitive
@ Single-character control sequences do not need to be looked up in a hash
table, since we can use the character code itself as a direct address.
@y
@ Here is the subroutine that searches the primitive table for an identifier

@p function prim_lookup(@!s:str_number):pointer; {search the primitives table}
label found; {go here if you found it}
var h:integer; {hash code}
@!p:pointer; {index in |hash| array}
@!k:pointer; {index in string pool}
@!j,@!l:integer;
begin
if s<=biggest_char then begin
  if s<0 then begin p:=undefined_primitive; goto found; end
  else p:=(s mod prim_prime)+prim_base; {we start searching here}
  l:=1
  end
else begin
  j:=str_start[s];
  if s = str_ptr then l := cur_length else l := length(s);
  @<Compute the primitive code |h|@>;
  p:=h+prim_base; {we start searching here; note that |0<=h<prim_prime|}
  end;
loop@+begin
  if prim_text(p)>1+biggest_char then { |p| points a multi-letter primitive }
    begin if length(prim_text(p)-1)=l then
      if str_eq_str(prim_text(p)-1,s) then goto found;
    end
  else if prim_text(p)=1+s then goto found; { |p| points a single-letter primitive }
  if prim_next(p)=0 then
    begin if no_new_control_sequence then
      p:=undefined_primitive
    else @<Insert a new primitive after |p|, then make
      |p| point to it@>;
    goto found;
    end;
  p:=prim_next(p);
  end;
found: prim_lookup:=p;
end;

@ @<Insert a new primitive...@>=
begin if prim_text(p)>0 then
  begin repeat if prim_is_full then overflow("primitive size",prim_size);
@:TeX capacity exceeded primitive size}{\quad primitive size@>
  decr(prim_used);
  until prim_text(prim_used)=0; {search for an empty location in |prim|}
  prim_next(p):=prim_used; p:=prim_used;
  end;
prim_text(p):=s+1;
end

@ The value of |prim_prime| should be roughly 85\pct! of
|prim_size|, and it should be a prime number.

@<Compute the primitive code |h|@>=
h:=str_pool[j];
for k:=j+1 to j+l-1 do
  begin h:=h+h+str_pool[k];
  while h>=prim_prime do h:=h-prim_prime;
  end

@ Single-character control sequences do not need to be looked up in a hash
table, since we can use the character code itself as a direct address.
@z

@x print_cs: \pdfprimitive
else  begin l:=text(p);
@y
else  begin
  if (p>=prim_eqtb_base)and(p<frozen_null_font) then
    l:=prim_text(p-prim_eqtb_base)-1 else l:=text(p);
@z

@x
else print_esc(text(p));
@y
else if (p>=prim_eqtb_base)and(p<frozen_null_font) then
    print_esc(prim_text(p-prim_eqtb_base)-1)
else print_esc(text(p));
@z


@x \[if]pdfprimitive
@p @!init procedure primitive(@!s:str_number;@!c:quarterword;@!o:halfword);
var k:pool_pointer; {index into |str_pool|}
@y
@p @!init procedure primitive(@!s:str_number;@!c:quarterword;@!o:halfword);
var k:pool_pointer; {index into |str_pool|}
@!prim_val:integer; {needed to fill |prim_eqtb|}
@z

@x \[if]pdfprimitive
begin if s<256 then cur_val:=s+single_base
@y
begin if s<256 then begin
  cur_val:=s+single_base;
  prim_val:=prim_lookup(s);
end
@z

@x \[if]pdfprimitive
  flush_string; text(cur_val):=s; {we don't want to have the string twice}
  end;
eq_level(cur_val):=level_one; eq_type(cur_val):=c; equiv(cur_val):=o;
end;
tini
@y
  flush_string; text(cur_val):=s; {we don't want to have the string twice}
  prim_val:=prim_lookup(s);
  end;
eq_level(cur_val):=level_one; eq_type(cur_val):=c; equiv(cur_val):=o;
prim_eq_level(prim_val):=level_one;
prim_eq_type(prim_val):=c;
prim_equiv(prim_val):=o;
end;
tini
@z

@x \[if]pdfprimitive
ignore_spaces: print_esc("ignorespaces");
@y
ignore_spaces: if chr_code=0 then print_esc("ignorespaces") else print_esc("pdfprimitive");
@z

@x \[if]pdfprimitive
no_expand: print_esc("noexpand");
@y
no_expand: if chr_code=0 then print_esc("noexpand")
   else print_esc("pdfprimitive");
@z

@x \ifincsname
var t:halfword; {token that is being ``expanded after''}
@!p,@!q,@!r:pointer; {for list manipulation}
@y
var t:halfword; {token that is being ``expanded after''}
@!b:boolean; {keep track of nested csnames}
@!p,@!q,@!r:pointer; {for list manipulation}
@z

@x
@ @<Expand a nonmacro@>=
@y
@ @<Glob...@>=
@!is_in_csname: boolean;

@ @<Set init...@>=
is_in_csname := false;

@ @<Expand a nonmacro@>=
@z

@x
no_expand:@<Suppress expansion of the next token@>;
@y
no_expand: if cur_chr=0 then @<Suppress expansion of the next token@>
  else @<Implement \.{\\pdfprimitive}@>;
@z

@x
@<Suppress expansion...@>=
begin save_scanner_status:=scanner_status; scanner_status:=normal;
get_token; scanner_status:=save_scanner_status; t:=cur_tok;
back_input; {now |start| and |loc| point to the backed-up token |t|}
if (t>=cs_token_flag)and(t<>end_write_token) then
  begin p:=get_avail; info(p):=cs_token_flag+frozen_dont_expand;
  link(p):=loc; start:=p; loc:=p;
  end;
end
@y
@<Suppress expansion...@>=
begin save_scanner_status:=scanner_status; scanner_status:=normal;
get_token; scanner_status:=save_scanner_status; t:=cur_tok;
back_input; {now |start| and |loc| point to the backed-up token |t|}
if (t>=cs_token_flag)and(t<>end_write_token) then
  begin p:=get_avail; info(p):=cs_token_flag+frozen_dont_expand;
  link(p):=loc; start:=p; loc:=p;
  end;
end

@ The \.{\\pdfprimitive} handling. If the primitive meaning of the next
token is an expandable command, it suffices to replace the current
token with the primitive one and restart |expand|/

Otherwise, the token we just read has to be pushed back, as well
as a token matching the internal form of \.{\\pdfprimitive}, that is
sneaked in as an alternate form of |ignore_spaces|.
@!@:pdfprimitive_}{\.{\\pdfprimitive} primitive (internalized)@>

Simply pushing back a token that matches the correct internal command
does not work, because approach would not survive roundtripping to a
temporary file.

@<Implement \.{\\pdfprimitive}@>=
begin save_scanner_status := scanner_status; scanner_status:=normal;
get_token; scanner_status:=save_scanner_status;
if cur_cs < hash_base then
  cur_cs := prim_lookup(cur_cs-single_base)
else
  cur_cs := prim_lookup(text(cur_cs));
if cur_cs<>undefined_primitive then begin
  t := prim_eq_type(cur_cs);
  if t>max_command then begin
    cur_cmd := t;
    cur_chr := prim_equiv(cur_cs);
    cur_tok := (cur_cmd*@'400)+cur_chr;
    cur_cs  := 0;
    goto reswitch;
    end
  else begin
    back_input; { now |loc| and |start| point to a one-item list }
    p:=get_avail; info(p):=cs_token_flag+frozen_primitive;
    link(p):=loc; loc:=p; start:=p;
    end;
  end;
end

@ This block deals with unexpandable \.{\\primitive} appearing at a spot where
an integer or an internal values should have been found. It fetches the
next token then resets |cur_cmd|, |cur_cs|, and |cur_tok|, based on the
primitive value of that token. No expansion takes place, because the
next token may be all sorts of things. This could trigger further
expansion creating new errors.

@<Reset |cur_tok| for unexpandable primitives, goto restart @>=
begin
get_token;
if cur_cs < hash_base then
  cur_cs := prim_lookup(cur_cs-single_base)
else
  cur_cs  := prim_lookup(text(cur_cs));
if cur_cs<>undefined_primitive then begin
  cur_cmd := prim_eq_type(cur_cs);
  cur_chr := prim_equiv(cur_cs);
  cur_cs  := prim_eqtb_base+cur_cs;
  cur_tok := cs_token_flag+cur_cs;
  end
else begin
  cur_cmd := relax;
  cur_chr := 0;
  cur_tok := cs_token_flag+frozen_relax;
  cur_cs  := frozen_relax;
  end;
goto restart;
end
@z

@x
begin r:=get_avail; p:=r; {head of the list of characters}
repeat get_x_token;
@y
begin r:=get_avail; p:=r; {head of the list of characters}
b := is_in_csname; is_in_csname := true;
repeat get_x_token;
@z

@x
@<Look up the characters of list |r| in the hash table, and set |cur_cs|@>;
@y
is_in_csname := b;
@<Look up the characters of list |r| in the hash table, and set |cur_cs|@>;
@z

@x scan_keyword
@!k:pool_pointer; {index into |str_pool|}
begin p:=backup_head; link(p):=null; k:=str_start[s];
@y
@!k:pool_pointer; {index into |str_pool|}
@!save_cur_cs:pointer; {to save |cur_cs|}
begin p:=backup_head; link(p):=null; k:=str_start[s];
save_cur_cs:=cur_cs;
@z

@x scan_keyword
    scan_keyword:=false; return;
@y
    cur_cs:=save_cur_cs;
    scan_keyword:=false; return;
@z

@x \[if]pdfprimitive : scan_something_internal
procedure scan_something_internal(@!level:small_number;@!negative:boolean);
  {fetch an internal parameter}
label exit;
@y
procedure scan_something_internal(@!level:small_number;@!negative:boolean);
  {fetch an internal parameter}
label exit, restart;
@z

@x \[if]pdfprimitive : scan_something_internal
begin m:=cur_chr;
@y
begin restart: m:=cur_chr;
@z

@x \[if]pdfprimitive : scan_something_internal
last_item: @<Fetch an item in the current node, if appropriate@>;
@y
last_item: @<Fetch an item in the current node, if appropriate@>;
ignore_spaces: {trap unexpandable primitives}
  if cur_chr=1 then @<Reset |cur_tok| for unexpandable primitives, goto restart@>;
@z

@x
@d eptex_version_code=ptex_minor_version_code+1 {code for \.{\\epTeXversion}}
@y
@d eptex_version_code=ptex_minor_version_code+1 {code for \.{\\epTeXversion}}
@d pdf_last_x_pos_code=eptex_version_code+1 {code for \.{\\pdflastxpos}}
@d pdf_last_y_pos_code=pdf_last_x_pos_code+1 {code for \.{\\pdflastypos}}
@d pdf_shell_escape_code=pdf_last_y_pos_code+1 {code for \.{\\pdflastypos}}
@d elapsed_time_code =pdf_shell_escape_code+1 {code for \.{\\pdfelapsedtime}}
@d random_seed_code=elapsed_time_code+1 {code for \.{\\pdfrandomseed}}
@z

@x
@d eTeX_int=ptex_minor_version_code+1 {first of \eTeX\ codes for integers}
@y
@d eTeX_int=random_seed_code+1 {first of \eTeX\ codes for integers}
@z

@x \[if]pdfprimitive: scan_int
@p procedure scan_int; {sets |cur_val| to an integer}
label done;
@y
@p procedure scan_int; {sets |cur_val| to an integer}
label done, restart;
@z

@x \[if]pdfprimitive: scan_int
if cur_tok=alpha_token then @<Scan an alphabetic character code into |cur_val|@>
@y
restart:
if cur_tok=alpha_token then @<Scan an alphabetic character code into |cur_val|@>
else if cur_tok=cs_token_flag+frozen_primitive then
  @<Reset |cur_tok| for unexpandable primitives, goto restart@>
@z

@x \Ucharcat: str_toks_cat
function str_toks(@!b:pool_pointer):pointer;
@y
function str_toks_cat(@!b:pool_pointer;@!cat:small_number):pointer;
@z

@x \Ucharcat: str_toks_cat
  else if t=" " then t:=space_token
  else t:=other_token+t;
@y
  else if (t=" ")and(cat=0) then t:=space_token
  else if (cat=0)or(cat>=kanji) then t:=other_token+t
  else if cat=active_char then t:= cs_token_flag + active_base + t
  else t:=left_brace_token*cat+t;
@z

@x \Ucharcat: str_toks_cat
pool_ptr:=b; str_toks:=p;
end;
@y
pool_ptr:=b; str_toks_cat:=p;
end;

function str_toks(@!b:pool_pointer):pointer;
begin str_toks:=str_toks_cat(b,0); end;
@z

@x
@d etex_convert_codes=etex_convert_base+1 {end of \eTeX's command codes}
@d job_name_code=etex_convert_codes {command code for \.{\\jobname}}
@y
@d etex_convert_codes=etex_convert_base+1 {end of \eTeX's command codes}
@d expanded_code            = etex_convert_codes {command code for \.{\\expanded}}
@d pdf_first_expand_code    = expanded_code + 1 {base for \pdfTeX-like command codes}
@d pdf_strcmp_code          = pdf_first_expand_code+0 {command code for \.{\\pdfstrcmp}}
@d pdf_creation_date_code   = pdf_first_expand_code+1 {command code for \.{\\pdfcreationdate}}
@d pdf_file_mod_date_code   = pdf_first_expand_code+2 {command code for \.{\\pdffilemoddate}}
@d pdf_file_size_code       = pdf_first_expand_code+3 {command code for \.{\\pdffilesize}}
@d pdf_mdfive_sum_code      = pdf_first_expand_code+4 {command code for \.{\\pdfmdfivesum}}
@d pdf_file_dump_code       = pdf_first_expand_code+5 {command code for \.{\\pdffiledump}}
@d uniform_deviate_code     = pdf_first_expand_code+6 {command code for \.{\\pdfuniformdeviate}}
@d normal_deviate_code      = pdf_first_expand_code+7 {command code for \.{\\pdfnormaldeviate}}
@d pdf_convert_codes        = pdf_first_expand_code+8 {end of \pdfTeX-like command codes}
@d Uchar_convert_code       = pdf_convert_codes   {command code for \.{\\Uchar}}
@d Ucharcat_convert_code    = pdf_convert_codes+1 {command code for \.{\\Ucharcat}}
@d eptex_convert_codes      = pdf_convert_codes+2 {end of \epTeX's command codes}
@d job_name_code=eptex_convert_codes {command code for \.{\\jobname}}
@z

@x
primitive("jobname",convert,job_name_code);@/
@y
@#
primitive("expanded",convert,expanded_code);@/
@!@:expanded_}{\.{\\expanded} primitive@>
@#
primitive("jobname",convert,job_name_code);@/
@z

@x
  eTeX_revision_code: print_esc("eTeXrevision");
@y
  eTeX_revision_code: print_esc("eTeXrevision");
  expanded_code:      print_esc("expanded");
  pdf_strcmp_code:        print_esc("pdfstrcmp");
  pdf_creation_date_code: print_esc("pdfcreationdate");
  pdf_file_mod_date_code: print_esc("pdffilemoddate");
  pdf_file_size_code:     print_esc("pdffilesize");
  pdf_mdfive_sum_code:    print_esc("pdfmdfivesum");
  pdf_file_dump_code:     print_esc("pdffiledump");
  uniform_deviate_code:   print_esc("pdfuniformdeviate");
  normal_deviate_code:    print_esc("pdfnormaldeviate");
  Uchar_convert_code:     print_esc("Uchar");
  Ucharcat_convert_code:  print_esc("Ucharcat");
@z

@x
@p procedure conv_toks;
@y

The extra temp string |u| is needed because |pdf_scan_ext_toks| incorporates
any pending string in its output. In order to save such a pending string,
we have to create a temporary string that is destroyed immediately after.

@d save_cur_string==if str_start[str_ptr]<pool_ptr then u:=make_string else u:=0
@d restore_cur_string==if u<>0 then decr(str_ptr)

@ Not all catcode values are allowed by \.{\\Ucharcat}:
@d illegal_Ucharcat_ascii_catcode(#)==(#<left_brace)or(#>active_char)or(#=out_param)or(#=ignore)
@d illegal_Ucharcat_wchar_catcode(#)==(#<kanji)or(#>other_kchar)

@p procedure conv_toks;
@z

@x
@!save_scanner_status:small_number; {|scanner_status| upon entry}
@y
@!save_scanner_status:small_number; {|scanner_status| upon entry}
@!save_def_ref: pointer; {|def_ref| upon entry, important if inside `\.{\\message}'}
@!save_warning_index: pointer;
@!bool: boolean; {temp boolean}
@!u: str_number; {saved current string string}
@!s: str_number; {first temp string}
@!i: integer;
@!j: integer;
@!cat:small_number; {desired catcode, or 0 for automatic |spacer|/|other_char| selection}
@z

@x
begin c:=cur_chr; @<Scan the argument for command |c|@>;
@y
begin cat:=0; c:=cur_chr; @<Scan the argument for command |c|@>;
u:=0; { will become non-nil if a string is already being built}
@z

@x
selector:=old_setting; link(garbage):=str_toks(b); ins_list(link(temp_head));
@y
selector:=old_setting; link(garbage):=str_toks_cat(b,cat); ins_list(link(temp_head));
@z

@x
eTeX_revision_code: do_nothing;
@y
eTeX_revision_code: do_nothing;
expanded_code:
  begin
    save_scanner_status := scanner_status;
    save_warning_index := warning_index;
    save_def_ref := def_ref;
    save_cur_string;
    scan_pdf_ext_toks;
    warning_index := save_warning_index;
    scanner_status := save_scanner_status;
    ins_list(link(def_ref));
    free_avail(def_ref);
    def_ref := save_def_ref;
    restore_cur_string;
    return;
  end;
pdf_strcmp_code:
  begin
    save_scanner_status := scanner_status;
    save_warning_index := warning_index;
    save_def_ref := def_ref;
    save_cur_string;
    compare_strings;
    def_ref := save_def_ref;
    warning_index := save_warning_index;
    scanner_status := save_scanner_status;
    restore_cur_string;
  end;
pdf_creation_date_code:
  begin
    b := pool_ptr;
    getcreationdate;
    link(garbage) := str_toks(b);
    ins_list(link(temp_head));
    return;
  end;
pdf_file_mod_date_code:
  begin
    save_scanner_status := scanner_status;
    save_warning_index := warning_index;
    save_def_ref := def_ref;
    save_cur_string;
    scan_pdf_ext_toks;
    s := tokens_to_string(def_ref);
    delete_token_ref(def_ref);
    def_ref := save_def_ref;
    warning_index := save_warning_index;
    scanner_status := save_scanner_status;
    b := pool_ptr;
    getfilemoddate(s);
    link(garbage) := str_toks(b);
    flush_str(s);
    ins_list(link(temp_head));
    restore_cur_string;
    return;
  end;
pdf_file_size_code:
  begin
    save_scanner_status := scanner_status;
    save_warning_index := warning_index;
    save_def_ref := def_ref;
    save_cur_string;
    scan_pdf_ext_toks;
    s := tokens_to_string(def_ref);
    delete_token_ref(def_ref);
    def_ref := save_def_ref;
    warning_index := save_warning_index;
    scanner_status := save_scanner_status;
    b := pool_ptr;
    getfilesize(s);
    link(garbage) := str_toks(b);
    flush_str(s);
    ins_list(link(temp_head));
    restore_cur_string;
    return;
  end;
pdf_mdfive_sum_code:
  begin
    save_scanner_status := scanner_status;
    save_warning_index := warning_index;
    save_def_ref := def_ref;
    save_cur_string;
    bool := scan_keyword("file");
    scan_pdf_ext_toks;
    if bool then s := tokens_to_string(def_ref)
    else begin
      isprint_utf8:=true; s := tokens_to_string(def_ref); isprint_utf8:=false;
    end;
    delete_token_ref(def_ref);
    def_ref := save_def_ref;
    warning_index := save_warning_index;
    scanner_status := save_scanner_status;
    b := pool_ptr;
    getmd5sum(s, bool);
    link(garbage) := str_toks(b);
    flush_str(s);
    ins_list(link(temp_head));
    restore_cur_string;
    return;
  end;
pdf_file_dump_code:
  begin
    save_scanner_status := scanner_status;
    save_warning_index := warning_index;
    save_def_ref := def_ref;
    save_cur_string;
    {scan offset}
    cur_val := 0;
    if (scan_keyword("offset")) then begin
      scan_int;
      if (cur_val < 0) then begin
        print_err("Bad file offset");
@.Bad file offset@>
        help2("A file offset must be between 0 and 2^{31}-1,")@/
          ("I changed this one to zero.");
        int_error(cur_val);
        cur_val := 0;
      end;
    end;
    i := cur_val;
    {scan length}
    cur_val := 0;
    if (scan_keyword("length")) then begin
      scan_int;
      if (cur_val < 0) then begin
        print_err("Bad dump length");
@.Bad dump length@>
        help2("A dump length must be between 0 and 2^{31}-1,")@/
          ("I changed this one to zero.");
        int_error(cur_val);
        cur_val := 0;
      end;
    end;
    j := cur_val;
    {scan file name}
    scan_pdf_ext_toks;
    s := tokens_to_string(def_ref);
    delete_token_ref(def_ref);
    def_ref := save_def_ref;
    warning_index := save_warning_index;
    scanner_status := save_scanner_status;
    b := pool_ptr;
    getfiledump(s, i, j);
    link(garbage) := str_toks(b);
    flush_str(s);
    ins_list(link(temp_head));
    restore_cur_string;
    return;
  end;
uniform_deviate_code:     scan_int;
normal_deviate_code:      do_nothing;
Uchar_convert_code:       scan_char_num;
Ucharcat_convert_code:
  begin
    scan_ascii_num;
    i:=cur_val;
    scan_int;
    if illegal_Ucharcat_ascii_catcode(cur_val) then
      begin print_err("Invalid code ("); print_int(cur_val);
@.Invalid code@>
      print("), should be in the ranges 1..4, 6..8, 10..13");
      help1("I'm going to use 12 instead of that illegal code value.");@/
      error; cat:=12;
    end else cat:=cur_val;
    cur_val:=i;
    end;
@z

@x
eTeX_revision_code: print(eTeX_revision);
@y
eTeX_revision_code: print(eTeX_revision);
pdf_strcmp_code: print_int(cur_val);
uniform_deviate_code:     print_int(unif_rand(cur_val));
normal_deviate_code:      print_int(norm_rand);
Uchar_convert_code:
if is_char_ascii(cur_val) then print_char(cur_val) else print_kanji(cur_val);
Ucharcat_convert_code:
if cat<kanji then print_char(cur_val) else print_kanji(cur_val);
@z

@x e-pTeX: if primitives - leave room for \ifincsname
@d if_tdir_code=if_case_code+4 { `\.{\\iftdir}' }
@y
@d if_in_csname_code=20 { `\.{\\ifincsname}';  |if_font_char_code| + 1 }
@d if_pdfprimitive_code=21 { `\.{\\ifpdfprimitive}' }
@#
@d if_tdir_code=if_pdfprimitive_code+1 { `\.{\\iftdir}' }
@z

@x \[if]pdfprimitive
  if_mbox_code:print_esc("ifmbox");
@y
  if_mbox_code:print_esc("ifmbox");
  if_pdfprimitive_code:print_esc("ifpdfprimitive");
@z

@x \ifincsname
var b:boolean; {is the condition true?}
@!r:"<"..">"; {relation to be evaluated}
@y
var b:boolean; {is the condition true?}
@!e:boolean; {keep track of nested csnames}
@!r:"<"..">"; {relation to be evaluated}
@z

@x \[if]pdfprimitive
if_void_code, if_hbox_code, if_vbox_code, if_tbox_code, if_ybox_code, if_dbox_code, if_mbox_code:
  @<Test box register status@>;
@y
if_void_code, if_hbox_code, if_vbox_code, if_tbox_code, if_ybox_code, if_dbox_code, if_mbox_code:
  @<Test box register status@>;
if_pdfprimitive_code: begin
  save_scanner_status:=scanner_status;
  scanner_status:=normal;
  get_next;
  scanner_status:=save_scanner_status;
  if cur_cs < hash_base then
    m := prim_lookup(cur_cs-single_base)
  else
    m := prim_lookup(text(cur_cs));
  b :=((cur_cmd<>undefined_cs) and
       (m<>undefined_primitive) and
       (cur_cmd=prim_eq_type(m)) and
       (cur_chr=prim_equiv(m)));
  end;
@z

@x
@ @<Initialize variables as |ship_out| begins@>=
@y
@ @<Initialize variables as |ship_out| begins@>=
@<Calculate DVI page dimensions and margins@>;
@z

% [33] \vadjust pre
@x
if adjust_tail<>null then link(adjust_tail):=null;
@y
if adjust_tail<>null then link(adjust_tail):=null;
if pre_adjust_tail<>null then link(pre_adjust_tail):=null;
@z

% [33] \vadjust pre
@x
  ins_node,mark_node,adjust_node: if adjust_tail<>null then
@y
  ins_node,mark_node,adjust_node: if (adjust_tail<>null) or (pre_adjust_tail<> null) then
@z

% [33] \vadjust pre
@x
to make a deletion.
@^inner loop@>
@y
to make a deletion.
@^inner loop@>

@<Glob...@>=
@!pre_adjust_tail: pointer;

@ @<Set init...@>=
pre_adjust_tail := null;

@ Materials in \.{\\vadjust} used with \.{pre} keyword will be appended to
|pre_adjust_tail| instead of |adjust_tail|.

@d update_adjust_list(#) == begin
    if # = null then
        confusion("pre vadjust");
    link(#) := adjust_ptr(p);
    while link(#) <> null do
        # := link(#);
end
@z

% [33] \vadjust pre
@x
@<Transfer node |p| to the adjustment list@>=
begin while link(q)<>p do q:=link(q);
if type(p)=adjust_node then
  begin link(adjust_tail):=adjust_ptr(p);
  while link(adjust_tail)<>null do adjust_tail:=link(adjust_tail);
  p:=link(p); free_node(link(q),small_node_size);
  end
else  begin link(adjust_tail):=p; adjust_tail:=p; p:=link(p);
  end;
link(q):=p; p:=q;
@y
@<Transfer node |p| to the adjustment list@>=
begin while link(q)<>p do q:=link(q);
    if type(p) = adjust_node then begin
        if adjust_pre(p) <> 0 then
            update_adjust_list(pre_adjust_tail)
        else
            update_adjust_list(adjust_tail);
        p := link(p); free_node(link(q), small_node_size);
    end
else  begin link(adjust_tail):=p; adjust_tail:=p; p:=link(p);
  end;
link(q):=p; p:=q;
@z

% [37] \vadjust pre
@x
@d align_stack_node_size=5 {number of |mem| words to save alignment states}
@y
@d align_stack_node_size=6 {number of |mem| words to save alignment states}
@z

% [37] \vadjust pre
@x
@!cur_head,@!cur_tail:pointer; {adjustment list pointers}
@y
@!cur_head,@!cur_tail:pointer; {adjustment list pointers}
@!cur_pre_head,@!cur_pre_tail:pointer; {pre-adjustment list pointers}
@z

% [37] \vadjust pre
@x
cur_head:=null; cur_tail:=null;
@y
cur_head:=null; cur_tail:=null;
cur_pre_head:=null; cur_pre_tail:=null;
@z

% [37] procedure |push_alignment|: \vadjust pre
@x
info(p+4):=cur_head; link(p+4):=cur_tail;
align_ptr:=p;
cur_head:=get_avail;
@y
info(p+4):=cur_head; link(p+4):=cur_tail;
info(p+5):=cur_pre_head; link(p+5):=cur_pre_tail;
align_ptr:=p;
cur_head:=get_avail;
cur_pre_head:=get_avail;
@z

% [37] procedure |pop_alignment|: \vadjust pre
@x
begin free_avail(cur_head);
p:=align_ptr;
cur_tail:=link(p+4); cur_head:=info(p+4);
@y
begin free_avail(cur_head);
free_avail(cur_pre_head);
p:=align_ptr;
cur_tail:=link(p+4); cur_head:=info(p+4);
cur_pre_tail:=link(p+5); cur_pre_head:=info(p+5);
@z

% [37] \vadjust pre
@x
cur_align:=link(preamble); cur_tail:=cur_head; init_span(cur_align);
@y
cur_align:=link(preamble); cur_tail:=cur_head; cur_pre_tail:=cur_pre_head;
init_span(cur_align);
@z

% [37] \vadjust pre + pTeX
@x
  begin adjust_tail:=cur_tail; adjust_hlist(head,false);
@y
  begin adjust_tail:=cur_tail; pre_adjust_tail:=cur_pre_tail;
  adjust_hlist(head,false);
@z

% [37] \vadjust pre
@x
  cur_tail:=adjust_tail; adjust_tail:=null;
@y
  cur_tail:=adjust_tail; adjust_tail:=null;
  cur_pre_tail:=pre_adjust_tail; pre_adjust_tail:=null;
@z

% [37] \vadjust pre
@x
  pop_nest; append_to_vlist(p);
  if cur_head<>cur_tail then
    begin link(tail):=link(cur_head); tail:=cur_tail;
    end;
@y
  pop_nest;
  if cur_pre_head <> cur_pre_tail then
      append_list(cur_pre_head)(cur_pre_tail);
  append_to_vlist(p);
  if cur_head <> cur_tail then
      append_list(cur_head)(cur_tail);
@z

% [39] \vadjust pre
@x
@ @<Append the new box to the current vertical list...@>=
append_to_vlist(just_box);
if adjust_head<>adjust_tail then
  begin link(tail):=link(adjust_head); tail:=adjust_tail;
   end;
adjust_tail:=null
@y
@ @<Append the new box to the current vertical list...@>=
if pre_adjust_head <> pre_adjust_tail then
    append_list(pre_adjust_head)(pre_adjust_tail);
pre_adjust_tail := null;
append_to_vlist(just_box);
if adjust_head <> adjust_tail then
    append_list(adjust_head)(adjust_tail);
adjust_tail := null
@z

% [39] \vadjust pre
@x
adjust_tail:=adjust_head; just_box:=hpack(q,cur_width,exactly);
@y
adjust_tail:=adjust_head;
pre_adjust_tail := pre_adjust_head;
just_box:=hpack(q,cur_width,exactly);
@z

@x \[if]pdfprimitive: main_loop
any_mode(ignore_spaces): begin @<Get the next non-blank non-call...@>;
  goto reswitch;
  end;
@y
any_mode(ignore_spaces): begin
  if cur_chr = 0 then begin
    @<Get the next non-blank non-call...@>;
    goto reswitch;
  end
  else begin
    t:=scanner_status;
    scanner_status:=normal;
    get_next;
    scanner_status:=t;
    if cur_cs < hash_base then
      cur_cs := prim_lookup(cur_cs-single_base)
    else
      cur_cs  := prim_lookup(text(cur_cs));
    if cur_cs<>undefined_primitive then begin
      cur_cmd := prim_eq_type(cur_cs);
      cur_chr := prim_equiv(cur_cs);
      cur_tok := cs_token_flag+prim_eqtb_base+cur_cs;
      goto reswitch;
      end;
    end;
  end;
@z

% [47] \vadjust pre
@x
  if abs(mode)=vmode then
    begin append_to_vlist(cur_box);
    if adjust_tail<>null then
      begin if adjust_head<>adjust_tail then
        begin link(tail):=link(adjust_head); tail:=adjust_tail;
        end;
      adjust_tail:=null;
      end;
    if mode>0 then build_page;
    end
@y
  if abs(mode)=vmode then
    begin
        if pre_adjust_tail <> null then begin
            if pre_adjust_head <> pre_adjust_tail then
                append_list(pre_adjust_head)(pre_adjust_tail);
            pre_adjust_tail := null;
        end;
        append_to_vlist(cur_box);
        if adjust_tail <> null then begin
            if adjust_head <> adjust_tail then
                append_list(adjust_head)(adjust_tail);
            adjust_tail := null;
        end;
    if mode>0 then build_page;
    end
@z

% [47] \vadjust pre + pTeX
@x
adjusted_hbox_group: begin adjust_hlist(head,false);
  adjust_tail:=adjust_head; package(0);
@y
adjusted_hbox_group: begin adjust_hlist(head,false);
  adjust_tail:=adjust_head;
  pre_adjust_tail:=pre_adjust_head; package(0);
@z

% [47] \vadjust pre
@x
saved(0):=cur_val; incr(save_ptr);
@y
saved(0) := cur_val;
if (cur_cmd = vadjust) and scan_keyword("pre") then
    saved(1) := 1
else
    saved(1) := 0;
save_ptr := save_ptr + 2;
@z

% [47] \vadjust pre
@x
  d:=split_max_depth; f:=floating_penalty; unsave; decr(save_ptr);
@y
  d:=split_max_depth; f:=floating_penalty; unsave; save_ptr := save_ptr - 2;
@z

% [47] \vadjust pre + pTeX
@x
      r:=get_node(small_node_size); type(r):=adjust_node;@/
      subtype(r):=0; {the |subtype| is not used}
      adjust_ptr(r):=list_ptr(p); delete_glue_ref(q);
@y
      r:=get_node(small_node_size); type(r):=adjust_node;@/
      adjust_pre(r) := saved(1); {the |subtype| is used for |adjust_pre|}
      adjust_ptr(r):=list_ptr(p); delete_glue_ref(q);
@z

% [48] \vadjust pre
@x
@!t:pointer; {tail of adjustment list}
@y
@!t:pointer; {tail of adjustment list}
@!pre_t:pointer; {tail of pre-adjustment list}
@z

% [48] \vadjust pre
@x
adjust_tail:=adjust_head; b:=hpack(p,natural); p:=list_ptr(b);
t:=adjust_tail; adjust_tail:=null;@/
@y
adjust_tail:=adjust_head; pre_adjust_tail:=pre_adjust_head;
b:=hpack(p,natural); p:=list_ptr(b);
t:=adjust_tail; adjust_tail:=null;@/
pre_t:=pre_adjust_tail; pre_adjust_tail:=null;@/
@z

% [48] \vadjust pre
@x
if t<>adjust_head then {migrating material comes after equation number}
  begin link(tail):=link(adjust_head); tail:=t;
  end;
@y
if t<>adjust_head then {migrating material comes after equation number}
  begin link(tail):=link(adjust_head); tail:=t;
  end;
if pre_t<>pre_adjust_head then
  begin link(tail):=link(pre_adjust_head); tail:=pre_t;
  end;
@z

@x \[if]pdfprimitive: dump prim table
@<Dump the hash table@>=
@y
@<Dump the hash table@>=
for p:=0 to prim_size do dump_hh(prim[p]);
@z

@x \[if]pdfprimitive: undump prim table
@ @<Undump the hash table@>=
@y
@ @<Undump the hash table@>=
for p:=0 to prim_size do undump_hh(prim[p]);
@z

@x
fix_date_and_time;@/
@y
fix_date_and_time;@/
isprint_utf8:=false;
random_seed:=(microseconds*1000)+(epochseconds mod 1000000);@/
init_randoms(random_seed);@/
@z

@x
@d epTeX_input_encoding_code=6 {command modifier for \.{\\epTeXinputencoding}}
@y
@d epTeX_input_encoding_code=6 {command modifier for \.{\\epTeXinputencoding}}
@d pdf_save_pos_node=epTeX_input_encoding_code+1
@d set_random_seed_code=pdf_save_pos_node+1
@d reset_timer_code=set_random_seed_code+1
@z

@x
  set_language_code:print_esc("setlanguage");
@y
  set_language_code:print_esc("setlanguage");
  pdf_save_pos_node: print_esc("pdfsavepos");
  set_random_seed_code: print_esc("pdfsetrandomseed");
  reset_timer_code: print_esc("pdfresettimer");
@z

@x
set_language_code:@<Implement \.{\\setlanguage}@>;
@y
set_language_code:@<Implement \.{\\setlanguage}@>;
pdf_save_pos_node: @<Implement \.{\\pdfsavepos}@>;
set_random_seed_code: @<Implement \.{\\pdfsetrandomseed}@>;
reset_timer_code: @<Implement \.{\\pdfresettimer}@>;
@z

@x \pdfsavepos
  print_int(what_lhm(p)); print_char(",");
  print_int(what_rhm(p)); print_char(")");
  end;
@y
  print_int(what_lhm(p)); print_char(",");
  print_int(what_rhm(p)); print_char(")");
  end;
pdf_save_pos_node: print_esc("pdfsavepos");
set_random_seed_code: print_esc("pdfsetrandomseed");
reset_timer_code: print_esc("pdfresettimer");
@z

@x \pdfsavepos
close_node,language_node: begin r:=get_node(small_node_size);
  words:=small_node_size;
  end;
@y
close_node,language_node: begin r:=get_node(small_node_size);
  words:=small_node_size;
  end;
pdf_save_pos_node:
   r := get_node(small_node_size);
@z

@x \pdfsavepos
close_node,language_node: free_node(p,small_node_size);
@y
close_node,language_node: free_node(p,small_node_size);
pdf_save_pos_node: free_node(p, small_node_size);
@z

@x
procedure special_out(@!p:pointer);
var old_setting:0..max_selector; {holds print |selector|}
@!k:pool_pointer; {index into |str_pool|}
@y
procedure special_out(@!p:pointer);
label done;
var old_setting:0..max_selector; {holds print |selector|}
@!k:pool_pointer; {index into |str_pool|}
@!s,@!t,@!cw, @!num, @!denom: scaled;
@!bl: boolean;
@!i: small_number;
@z

@x
pool_ptr:=str_start[str_ptr]; {erase the string}
@y
if read_papersize_special>0 then
  @<Determine whether this \.{\\special} is a papersize special@>;
done: pool_ptr:=str_start[str_ptr]; {erase the string}
@z

@x
language_node:do_nothing;
@y
language_node:do_nothing;
pdf_save_pos_node:
  @<Save current position in DVI mode@>;
@z

@x
primitive("eTeXrevision",convert,eTeX_revision_code);@/
@!@:eTeX_revision_}{\.{\\eTeXrevision} primitive@>
@y
primitive("eTeXrevision",convert,eTeX_revision_code);@/
@!@:eTeX_revision_}{\.{\\eTeXrevision} primitive@>
primitive("pdfprimitive",no_expand,1);@/
@!@:pdfprimitive_}{\.{\\pdfprimitive} primitive@>
primitive("pdfstrcmp",convert,pdf_strcmp_code);@/
@!@:pdf_strcmp_}{\.{\\pdfstrcmp} primitive@>
primitive("pdfcreationdate",convert,pdf_creation_date_code);@/
@!@:pdf_creation_date_}{\.{\\pdfcreationdate} primitive@>
primitive("pdffilemoddate",convert,pdf_file_mod_date_code);@/
@!@:pdf_file_mod_date_}{\.{\\pdffilemoddate} primitive@>
primitive("pdffilesize",convert,pdf_file_size_code);@/
@!@:pdf_file_size_}{\.{\\pdffilesize} primitive@>
primitive("pdfmdfivesum",convert,pdf_mdfive_sum_code);@/
@!@:pdf_mdfive_sum_}{\.{\\pdfmdfivesum} primitive@>
primitive("pdffiledump",convert,pdf_file_dump_code);@/
@!@:pdf_file_dump_}{\.{\\pdffiledump} primitive@>
primitive("pdfsavepos",extension,pdf_save_pos_node);@/
@!@:pdf_save_pos_}{\.{\\pdfsavepos} primitive@>
primitive("pdfpagewidth",assign_dimen,dimen_base+pdf_page_width_code);@/
@!@:pdf_page_width_}{\.{\\pdfpagewidth} primitive@>
primitive("pdfpageheight",assign_dimen,dimen_base+pdf_page_height_code);@/
@!@:pdf_page_height_}{\.{\\pdfpageheight} primitive@>
primitive("pdflastxpos",last_item,pdf_last_x_pos_code);@/
@!@:pdf_last_x_pos_}{\.{\\pdflastxpos} primitive@>
primitive("pdflastypos",last_item,pdf_last_y_pos_code);@/
@!@:pdf_last_y_pos_}{\.{\\pdflastypos} primitive@>
primitive("pdfshellescape",last_item,pdf_shell_escape_code);
@!@:pdf_shell_escape_}{\.{\\pdfshellescape} primitive@>
primitive("ifpdfprimitive",if_test,if_pdfprimitive_code);
@!@:if_pdfprimitive_}{\.{\\ifpdfprimitive} primitive@>
primitive("pdfuniformdeviate",convert,uniform_deviate_code);@/
@!@:uniform_deviate_}{\.{\\pdfuniformdeviate} primitive@>
primitive("pdfnormaldeviate",convert,normal_deviate_code);@/
@!@:normal_deviate_}{\.{\\pdfnormaldeviate} primitive@>
primitive("pdfrandomseed",last_item,random_seed_code);
@!@:random_seed_}{\.{\\pdfrandomseed} primitive@>
primitive("pdfsetrandomseed",extension,set_random_seed_code);@/
@!@:set_random_seed_code}{\.{\\pdfsetrandomseed} primitive@>
primitive("pdfelapsedtime",last_item,elapsed_time_code);
@!@:elapsed_time_}{\.{\\pdfelapsedtime} primitive@>
primitive("pdfresettimer",extension,reset_timer_code);@/
@!@:reset_timer_}{\.{\\pdfresettimer} primitive@>
primitive("Uchar",convert,Uchar_convert_code);@/
@!@:Uchar_}{\.{\\Uchar} primitive@>
primitive("Ucharcat",convert,Ucharcat_convert_code);@/
@!@:Ucharcat_}{\.{\\Ucharcat} primitive@>
@z

@x
eTeX_version_code: print_esc("eTeXversion");
@y
eTeX_version_code: print_esc("eTeXversion");
pdf_last_x_pos_code:  print_esc("pdflastxpos");
pdf_last_y_pos_code:  print_esc("pdflastypos");
elapsed_time_code: print_esc("pdfelapsedtime");
pdf_shell_escape_code: print_esc("pdfshellescape");
random_seed_code:     print_esc("pdfrandomseed");
@z

@x
eTeX_version_code: cur_val:=eTeX_version;
@y
eTeX_version_code: cur_val:=eTeX_version;
pdf_last_x_pos_code: cur_val := pdf_last_x_pos;
pdf_last_y_pos_code: cur_val := pdf_last_y_pos;
pdf_shell_escape_code:
  begin
  if shellenabledp then begin
    if restrictedshell then cur_val :=2
    else cur_val := 1;
  end
  else cur_val := 0;
  end;
elapsed_time_code: cur_val := get_microinterval;
random_seed_code:  cur_val := random_seed;
@z

@x
primitive("iffontchar",if_test,if_font_char_code);
@!@:if_font_char_}{\.{\\iffontchar} primitive@>
@y
primitive("iffontchar",if_test,if_font_char_code);
@!@:if_font_char_}{\.{\\iffontchar} primitive@>
primitive("ifincsname",if_test,if_in_csname_code);
@!@:if_in_csname_}{\.{\\ifincsname} primitive@>
@z

@x
if_font_char_code:print_esc("iffontchar");
@y
if_font_char_code:print_esc("iffontchar");
if_in_csname_code:print_esc("ifincsname");
@z

@x
if_cs_code:begin n:=get_avail; p:=n; {head of the list of characters}
  repeat get_x_token;
@y
if_cs_code:begin n:=get_avail; p:=n; {head of the list of characters}
 e := is_in_csname; is_in_csname := true;
  repeat get_x_token;
@z

@x
  b:=(eq_type(cur_cs)<>undefined_cs);
@y
  b:=(eq_type(cur_cs)<>undefined_cs);
  is_in_csname := e;
@z

@x
if_font_char_code:begin scan_font_ident; n:=cur_val;
@y
if_in_csname_code: b := is_in_csname;
if_font_char_code:begin scan_font_ident; n:=cur_val;
@z

@x
procedure print_kanji(@!s:KANJI_code); {prints a single character}
begin
if s>@"FF then
  begin print_char(@"100+Hi(s)); print_char(@"100+Lo(s));
  end else print_char(s);
end;
@y
procedure print_kanji(@!s:integer); {prints a single character}
begin
if s>@"FF then begin
  if isprint_utf8 then begin
    s:=UCStoUTF8(toUCS(s));
    if BYTE1(s)<>0 then print_char(@"100+BYTE1(s));
    if BYTE2(s)<>0 then print_char(@"100+BYTE2(s));
    if BYTE3(s)<>0 then print_char(@"100+BYTE3(s));
                        print_char(@"100+BYTE4(s));
  end
  else begin print_char(@"100+Hi(s)); print_char(@"100+Lo(s)); end;
end
else print_char(s);
end;


@z

@x
@* \[54] System-dependent changes.
@y
@* \[54/pdf\TeX] System-dependent changes for {\tt\char"5Cpdfstrcmp}.
@d call_func(#) == begin if # <> 0 then do_nothing end
@d flushable(#) == (# = str_ptr - 1)

@<Glob...@>=
@!isprint_utf8: boolean;
@!epochseconds: integer;
@!microseconds: integer;

@
@d max_integer == @"7FFFFFFF {$2^{31}-1$}

@<Declare procedures that need to be declared forward for \pdfTeX@>=
procedure pdf_error(t, p: str_number);
begin
    normalize_selector;
    print_err("pdfTeX error");
    if t <> 0 then begin
        print(" (");
        print(t);
        print(")");
    end;
    print(": "); print(p);
    succumb;
end;

function get_microinterval:integer;
var s,@!m:integer; {seconds and microseconds}
begin
   seconds_and_micros(s,m);
   if (s-epochseconds)>32767 then
     get_microinterval := max_integer
   else if (microseconds>m)  then
     get_microinterval := ((s-1-epochseconds)*65536)+ (((m+1000000-microseconds)/100)*65536)/10000
   else
     get_microinterval := ((s-epochseconds)*65536)  + (((m-microseconds)/100)*65536)/10000;
end;

@ @<Declare procedures needed in |do_ext...@>=

procedure compare_strings; {to implement \.{\\pdfstrcmp}}
label done;
var s1, s2: str_number;
    i1, i2, j1, j2: pool_pointer;
    c1, c2: integer;
    save_cur_cs: pointer;
begin
    save_cur_cs:=cur_cs; call_func(scan_toks(false, true));
    isprint_utf8:=true; s1 := tokens_to_string(def_ref); isprint_utf8:=false;
    delete_token_ref(def_ref);
    cur_cs:=save_cur_cs; call_func(scan_toks(false, true));
    isprint_utf8:=true; s2 := tokens_to_string(def_ref); isprint_utf8:=false;
    delete_token_ref(def_ref);
    i1 := str_start[s1];
    j1 := str_start[s1 + 1];
    i2 := str_start[s2];
    j2 := str_start[s2 + 1];
    while (i1 < j1) and (i2 < j2) do begin
        if str_pool[i1]>=@"100 then c1:=str_pool[i1]-@"100 else c1:=str_pool[i1];
        if str_pool[i2]>=@"100 then c2:=str_pool[i2]-@"100 else c2:=str_pool[i2];
        if c1<c2 then begin cur_val := -1; goto done; end
        else if c1>c2 then begin cur_val := 1; goto done; end;
        incr(i1);
        incr(i2);
    end;
    if (i1 = j1) and (i2 = j2) then
        cur_val := 0
    else if i1 < j1 then
        cur_val := 1
    else
        cur_val := -1;
done:
    flush_str(s2);
    flush_str(s1);
    cur_val_level := int_val;
end;

@ Next, we implement \.{\\pdfsavepos} and related primitives.

@<Glob...@>=
@!cur_page_width: scaled; {"physical" width of page being shipped}
@!cur_page_height: scaled; {"physical" height of page being shipped}
@!pdf_last_x_pos: integer;
@!pdf_last_y_pos: integer;

@ @<Implement \.{\\pdfsavepos}@>=
begin
    new_whatsit(pdf_save_pos_node, small_node_size);
    inhibit_glue_flag:=false;
end

@ @<Save current position in DVI mode@>=
begin
  case dvi_dir of
  dir_yoko: begin pdf_last_x_pos := cur_h;  pdf_last_y_pos := cur_v;  end;
  dir_tate: begin pdf_last_x_pos := -cur_v; pdf_last_y_pos := cur_h;  end;
  dir_dtou: begin pdf_last_x_pos := cur_v;  pdf_last_y_pos := -cur_h; end;
  endcases;
  pdf_last_x_pos := pdf_last_x_pos + 4736286;
  pdf_last_y_pos := cur_page_height - pdf_last_y_pos - 4736286;
  {4736286 = 1in, the funny DVI origin offset}
end

@ @<Calculate DVI page dimensions and margins@>=
  if pdf_page_height <> 0 then
    cur_page_height := pdf_page_height
  else if (box_dir(p)=dir_tate)or(box_dir(p)=dir_dtou) then
    cur_page_height := width(p) + 2*v_offset + 2*4736286
  else
    cur_page_height := height(p) + depth(p) + 2*v_offset + 2*4736286;
    {4736286 = 1in, the funny DVI origin offset}
  if pdf_page_width <> 0 then
    cur_page_width := pdf_page_width
  else if (box_dir(p)=dir_tate)or(box_dir(p)=dir_dtou) then
    cur_page_width := height(p) + depth(p) + 2*h_offset + 2*4736286
  else
    cur_page_width := width(p) + 2*h_offset + 2*4736286
    {4736286 = 1in, the funny DVI origin offset}


@ Of course \epTeX\ can produce a \.{DVI} file only, not a PDF file.
A \.{DVI} file does not have the information of the page height,
which is needed to implement \.{\\pdflastypos} correctly.
To keep the information of the page height, I (H.~Kitagawa)
adopted \.{\\pdfpageheight} primitive from pdf\TeX.

In \pTeX (and \hbox{\epTeX}), the papersize special
\.{\\special\{papersize=\<width>,\<height>\}} is commonly used
for specifying page width/height.
If \.{\\readpapersizespecial} is greater than~0, the papersize special also
changes the value of \.{\\pdfpagewidth} and \.{\\pdfpageheight}.
This process is done in the following routine.

{\def\<#1>{\langle\hbox{#1\/}\rangle}
In present implementation, the papersize special $\<special>$,
which can be interpreted by this routine, is defined as follows.
$$\eqalign{%
  \<special> &\longrightarrow \.{papersize=}\<length>\.{,}\<length>\cr
  \<length>  &\longrightarrow \<decimal>
    \<optional~\.{true}>\<physical unit>\cr
  \<decimal> &\longrightarrow \.{.} \mid \<digit>\<decimal> \mid
    \<decimal>\<digit>\cr
}$$}
Note that any space, ``\.{,}'' as a decimal separator, minus~symbol
are neither permitted.

@d ifps(#)==@+if k+(#)>pool_ptr then goto done @+ else @+ if
@d sop(#)==so(str_pool[#])
@f ifps==if

@<Determine whether this \.{\\special} is a papersize special@>=
begin k:=str_start[str_ptr];@/
ifps(10) @,
   (sop(k+0)<>'p')or(sop(k+1)<>'a')or(sop(k+2)<>'p')or
   (sop(k+3)<>'e')@|or(sop(k+4)<>'r')or(sop(k+5)<>'s')or
   (sop(k+6)<>'i')or(sop(k+7)<>'z')@|or(sop(k+8)<>'e')or
   (sop(k+9)<>'=')  then goto done;
k:=k+10;
@<Read dimensions in the argument in the papersize special@>;
ifps(1) @, sop(k)=',' then begin
  incr(k); cw:=s;
  @<Read dimensions in the argument in the papersize special@>;
  if pool_ptr>k then goto done;
  geq_word_define(dimen_base+pdf_page_width_code,cw);
  geq_word_define(dimen_base+pdf_page_height_code,s);@|
  cur_page_height := s; cur_page_width := cw;
end;
end;

@

@d if_ps_unit(#)==if bl then @+ begin @+ ifps(2) sop(k)=(#) @, if_ps_unit_two
@d if_ps_unit_two(#)==and (sop(k+1)=(#)) then begin bl:=false; k:=k+2; if_ps_unit_end
@d if_ps_unit_end(#)==# @+ end @+ end;

@d do_ps_conversion(#)==num:=#; do_ps_conversion_end
@d do_ps_conversion_end(#)==
  s:=xn_over_d(s,num,#); s:=s*unity+((num*t+@'200000*remainder) div #)

@<Read dimensions in the argument in the papersize special@>=
s:=0; t:=0; bl:=true;
while (k<pool_ptr)and bl do
  if (sop(k)>='0')and (sop(k)<='9') then begin s:=10*s+sop(k)-'0'; incr(k); @+end
  else bl:=false;
ifps(1) sop(k)='.' then
  begin incr(k); bl:=true; i:=0; dig[0]:=0;
  while (k<pool_ptr)and bl do begin
    if (sop(k)>='0')and (sop(k)<='9') then
      begin if i<17 then begin dig[i]:=sop(k)-'0'; incr(i); @+end;
      incr(k); end
    else bl:=false;
  end;
  t:=round_decimals(i);
  end;
if k+4>pool_ptr then
  if (sop(k)='t')and(sop(k+1)='r')and(sop(k+2)='u')and(sop(k+3)='e') then
    k:=k+4;
if mag<>1000 then
  begin s:=xn_over_d(s,1000,mag);
  t:=(1000*t+@'200000*remainder) div mag;
  s:=s+(t div @'200000); t:=t mod @'200000;
end;
bl:=true;@/
if_ps_unit('p')('t')(s:=s*unity+t)@/
if_ps_unit('i')('n')(do_ps_conversion(7227)(100))@/
if_ps_unit('p')('c')(do_ps_conversion(12)(1))@/
if_ps_unit('c')('m')(do_ps_conversion(7227)(254))@/
if_ps_unit('m')('m')(do_ps_conversion(7227)(2540))@/
if_ps_unit('b')('p')(do_ps_conversion(7227)(7200))@/
if_ps_unit('d')('d')(do_ps_conversion(1238)(1157))@/
if_ps_unit('c')('c')(do_ps_conversion(14856)(1157))@/
if_ps_unit('s')('p')(do_nothing)

@ Finally, we declare some routine needed for \.{\\pdffilemoddate}.

@<Glob...@>=
@!isprint_utf8: boolean;
@!last_tokens_string: str_number; {the number of the most recently string
created by |tokens_to_string|}

@ @<Declare procedures needed in |do_ext...@>=
procedure scan_pdf_ext_toks;
begin
    call_func(scan_toks(false, true)); {like \.{\\special}}
end;

@ @<Declare procedures that need to be declared forward for \pdfTeX@>=
function tokens_to_string(p: pointer): str_number; {return a string from tokens
list}
begin
    if selector = new_string then
        pdf_error("tokens", "tokens_to_string() called while selector = new_string");
    old_setting:=selector; selector:=new_string;
    show_token_list(link(p),null,pool_size-pool_ptr);
    selector:=old_setting;
    last_tokens_string := make_string;
    tokens_to_string := last_tokens_string;
end;
procedure flush_str(s: str_number); {flush a string if possible}
begin
    if flushable(s) then
        flush_string;
end;

@ @<Set initial values of key variables@>=
  seconds_and_micros(epochseconds,microseconds);
  init_start_time;

@ Negative random seed values are silently converted to positive ones

@<Implement \.{\\pdfsetrandomseed}@>=
begin
  scan_int;
  if cur_val<0 then negate(cur_val);
  random_seed := cur_val;
  init_randoms(random_seed);
end

@ @<Implement \.{\\pdfresettimer}@>=
begin
  seconds_and_micros(epochseconds,microseconds);
end

@* \[54] System-dependent changes.
@z
