% arithmetic.w
%
% Copyright 2009-2010 Taco Hoekwater <taco@@luatex.org>
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

\def\MP{MetaPost}

@ @c


#include "ptexlib.h"

@ The principal computations performed by \TeX\ are done entirely in terms of
integers less than $2^{31}$ in magnitude; and divisions are done only when both
dividend and divisor are nonnegative. Thus, the arithmetic specified in this
program can be carried out in exactly the same way on a wide variety of
computers, including some small ones. Why? Because the arithmetic
calculations need to be spelled out precisely in order to guarantee that
\TeX\ will produce identical output on different machines. If some
quantities were rounded differently in different implementations, we would
find that line breaks and even page breaks might occur in different places.
Hence the arithmetic of \TeX\ has been designed with care, and systems that
claim to be implementations of \TeX82 should follow precisely the
@:TeX82}{\TeX82@>
calculations as they appear in the present program.

(Actually there are three places where \TeX\ uses |div| with a possibly negative
numerator. These are harmless; see |div| in the index. Also if the user
sets the \.{\\time} or the \.{\\year} to a negative value, some diagnostic
information will involve negative-numerator division. The same remarks
apply for |mod| as well as for |div|.)

Here is a routine that calculates half of an integer, using an
unambiguous convention with respect to signed odd numbers.

@c
int half(int x)
{
    if (odd(x))
        return ((x + 1) / 2);
    else
        return (x / 2);
}


@ The following function is used to create a scaled integer from a given decimal
fraction $(.d_0d_1\ldots d_{k-1})$, where |0<=k<=17|. The digit $d_i$ is
given in |dig[i]|, and the calculation produces a correctly rounded result.

@c
scaled round_decimals(int k)
{                               /* converts a decimal fraction */
    int a;                      /* the accumulator */
    a = 0;
    while (k-- > 0) {
        a = (a + dig[k] * two) / 10;
    }
    return ((a + 1) / 2);
}


@ Conversely, here is a procedure analogous to |print_int|. If the output
of this procedure is subsequently read by \TeX\ and converted by the
|round_decimals| routine above, it turns out that the original value will
be reproduced exactly; the ``simplest'' such decimal number is output,
but there is always at least one digit following the decimal point.

The invariant relation in the \&{repeat} loop is that a sequence of
decimal digits yet to be printed will yield the original number if and only if
they form a fraction~$f$ in the range $s-\delta\L10\cdot2^{16}f<s$.
We can stop if and only if $f=0$ satisfies this condition; the loop will
terminate before $s$ can possibly become zero.

@c
void print_scaled(scaled s)
{                               /* prints scaled real, rounded to five digits */
    scaled delta;               /* amount of allowable inaccuracy */
    char buffer[20];
    int i = 0;
    if (s < 0) {
        print_char('-');
        negate(s);              /* print the sign, if negative */
    }
    print_int(s / unity);       /* print the integer part */
    buffer[i++] = '.';
    s = 10 * (s % unity) + 5;
    delta = 10;
    do {
        if (delta > unity)
            s = s + 0100000 - 50000;    /* round the last digit */
        buffer[i++] = '0' + (s / unity);
        s = 10 * (s % unity);
        delta = delta * 10;
    } while (s > delta);
    buffer[i++] = '\0';
    tprint(buffer);
}

@ Physical sizes that a \TeX\ user specifies for portions of documents are
represented internally as scaled points. Thus, if we define an `sp' (scaled
@^sp@>
point) as a unit equal to $2^{-16}$ printer's points, every dimension
inside of \TeX\ is an integer number of sp. There are exactly
4,736,286.72 sp per inch.  Users are not allowed to specify dimensions
larger than $2^{30}-1$ sp, which is a distance of about 18.892 feet (5.7583
meters); two such quantities can be added without overflow on a 32-bit
computer.

The present implementation of \TeX\ does not check for overflow when
@^overflow in arithmetic@>
dimensions are added or subtracted. This could be done by inserting a
few dozen tests of the form `\ignorespaces|if x>=010000000000 then
@t\\{report\_overflow}@>|', but the chance of overflow is so remote that
such tests do not seem worthwhile.

\TeX\ needs to do only a few arithmetic operations on scaled quantities,
other than addition and subtraction, and the following subroutines do most of
the work. A single computation might use several subroutine calls, and it is
desirable to avoid producing multiple error messages in case of arithmetic
overflow; so the routines set the global variable |arith_error| to |true|
instead of reporting errors directly to the user. Another global variable,
|tex_remainder|, holds the remainder after a division.

@c
boolean arith_error;            /* has arithmetic overflow occurred recently? */
scaled tex_remainder;           /* amount subtracted to get an exact division */


@ The first arithmetical subroutine we need computes $nx+y$, where |x|
and~|y| are |scaled| and |n| is an integer. We will also use it to
multiply integers.

@c
scaled mult_and_add(int n, scaled x, scaled y, scaled max_answer)
{
    if (n == 0)
        return y;
    if (n < 0) {
        negate(x);
        negate(n);
    }
    if (((x <= (max_answer - y) / n) && (-x <= (max_answer + y) / n))) {
        return (n * x + y);
    } else {
        arith_error = true;
        return 0;
    }
}

@ We also need to divide scaled dimensions by integers. 
@c
scaled x_over_n(scaled x, int n)
{
    boolean negative;           /* should |tex_remainder| be negated? */
    negative = false;
    if (n == 0) {
        arith_error = true;
        tex_remainder = x;
        return 0;
    } else {
        if (n < 0) {
            negate(x);
            negate(n);
            negative = true;
        }
        if (x >= 0) {
            tex_remainder = x % n;
            if (negative)
                negate(tex_remainder);
            return (x / n);
        } else {
            tex_remainder = -((-x) % n);
            if (negative)
                negate(tex_remainder);
            return (-((-x) / n));
        }
    }
}


@ Then comes the multiplication of a scaled number by a fraction |n/d|,
where |n| and |d| are nonnegative integers |<=@t$2^{16}$@>| and |d| is
positive. It would be too dangerous to multiply by~|n| and then divide
by~|d|, in separate operations, since overflow might well occur; and it
would be too inaccurate to divide by |d| and then multiply by |n|. Hence
this subroutine simulates 1.5-precision arithmetic.

@c
scaled xn_over_d(scaled x, int n, int d)
{
    nonnegative_integer t, u, v, xx, dd;        /* intermediate quantities */
    boolean positive = true;    /* was |x>=0|? */
    if (x < 0) {
        negate(x);
        positive = false;
    }
    xx = (nonnegative_integer) x;
    dd = (nonnegative_integer) d;
    t = ((xx % 0100000) * (nonnegative_integer) n);
    u = ((xx / 0100000) * (nonnegative_integer) n + (t / 0100000));
    v = (u % dd) * 0100000 + (t % 0100000);
    if (u / dd >= 0100000)
        arith_error = true;
    else
        u = 0100000 * (u / dd) + (v / dd);
    if (positive) {
        tex_remainder = (int) (v % dd);
        return (scaled) u;
    } else {
        /* casts are for ms cl */
        tex_remainder = -(int) (v % dd);
        return -(scaled) (u);
    }
}


@ The next subroutine is used to compute the ``badness'' of glue, when a
total~|t| is supposed to be made from amounts that sum to~|s|.  According
to {\sl The \TeX book}, the badness of this situation is $100(t/s)^3$;
however, badness is simply a heuristic, so we need not squeeze out the
last drop of accuracy when computing it. All we really want is an
approximation that has similar properties.
@:TeXbook}{\sl The \TeX book@>

The actual method used to compute the badness is easier to read from the
program than to describe in words. It produces an integer value that is a
reasonably close approximation to $100(t/s)^3$, and all implementations
of \TeX\ should use precisely this method. Any badness of $2^{13}$ or more is
treated as infinitely bad, and represented by 10000.

It is not difficult to prove that $$\hbox{|badness(t+1,s)>=badness(t,s)
>=badness(t,s+1)|}.$$ The badness function defined here is capable of
computing at most 1095 distinct values, but that is plenty.

@c
halfword badness(scaled t, scaled s)
{                               /* compute badness, given |t>=0| */
    int r;                      /* approximation to $\alpha t/s$, where $\alpha^3\approx
                                   100\cdot2^{18}$ */
    if (t == 0) {
        return 0;
    } else if (s <= 0) {
        return inf_bad;
    } else {
        if (t <= 7230584)
            r = (t * 297) / s;  /* $297^3=99.94\times2^{18}$ */
        else if (s >= 1663497)
            r = t / (s / 297);
        else
            r = t;
        if (r > 1290)
            return inf_bad;     /* $1290^3<2^{31}<1291^3$ */
        else
            return ((r * r * r + 0400000) / 01000000);
        /* that was $r^3/2^{18}$, rounded to the nearest integer */
    }
}


@ When \TeX\ ``packages'' a list into a box, it needs to calculate the
proportionality ratio by which the glue inside the box should stretch
or shrink. This calculation does not affect \TeX's decision making,
so the precise details of rounding, etc., in the glue calculation are not
of critical importance for the consistency of results on different computers.

We shall use the type |glue_ratio| for such proportionality ratios.
A glue ratio should take the same amount of memory as an
|integer| (usually 32 bits) if it is to blend smoothly with \TeX's
other data structures. Thus |glue_ratio| should be equivalent to
|short_real| in some implementations of PASCAL. Alternatively,
it is possible to deal with glue ratios using nothing but fixed-point
arithmetic; see {\sl TUGboat \bf3},1 (March 1982), 10--27. (But the
routines cited there must be modified to allow negative glue ratios.)
@^system dependencies@>


@ This section is (almost) straight from MetaPost. I had to change
the types (use |integer| instead of |fraction|), but that should
not have any influence on the actual calculations (the original
comments refer to quantities like |fraction_four| ($2^{30}$), and
that is the same as the numeric representation of |max_dimen|).

I've copied the low-level variables and routines that are needed, but
only those (e.g. |m_log|), not the accompanying ones like |m_exp|. Most
of the following low-level numeric routines are only needed within the
calculation of |norm_rand|. I've been forced to rename |make_fraction|
to |make_frac| because TeX already has a routine by that name with
a wholly different function (it creates a |fraction_noad| for math
typesetting) -- Taco

And now let's complete our collection of numeric utility routines
by considering random number generation.
\MP{} generates pseudo-random numbers with the additive scheme recommended
in Section 3.6 of {\sl The Art of Computer Programming}; however, the
results are random fractions between 0 and |fraction_one-1|, inclusive.

There's an auxiliary array |randoms| that contains 55 pseudo-random
fractions. Using the recurrence $x_n=(x_{n-55}-x_{n-31})\bmod 2^{28}$,
we generate batches of 55 new $x_n$'s at a time by calling |new_randoms|.
The global variable |j_random| tells which element has most recently
been consumed.

@c
static int randoms[55];         /* the last 55 random values generated */
static int j_random;            /* the number of unused |randoms| */
scaled random_seed;             /* the default random seed */

@ A small bit of metafont is needed. 

@c
#define fraction_half 01000000000       /* $2^{27}$, represents 0.50000000 */
#define fraction_one 02000000000        /* $2^{28}$, represents 1.00000000 */
#define fraction_four 010000000000      /* $2^{30}$, represents 4.00000000 */
#define el_gordo 017777777777   /* $2^{31}-1$, the largest value that \MP\ likes */

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
instruction. But when we are restricted to PASCAL arithmetic it
is necessary either to resort to multiple-precision maneuvering
or to use a simple but slow iteration. The multiple-precision technique
would be about three times faster than the code adopted here, but it
would be comparatively long and tricky, involving about sixteen
additional multiplications and divisions.

This operation is part of \MP's ``inner loop''; indeed, it will
consume nearly 10\%! of the running time (exclusive of input and output)
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

@c
static int make_frac(int p, int q)
{
    int f;                      /* the fraction bits, with a leading 1 bit */
    int n;                      /* the integer part of $\vert p/q\vert$ */
    register int be_careful;    /* disables certain compiler optimizations */
    boolean negative = false;   /* should the result be negated? */
    if (p < 0) {
        negate(p);
        negative = true;
    }
    if (q <= 0) {
#ifdef DEBUG
        if (q == 0)
            confusion("/");
#endif
        negate(q);
        negative = !negative;
    }
    n = p / q;
    p = p % q;
    if (n >= 8) {
        arith_error = true;
        if (negative)
            return (-el_gordo);
        else
            return el_gordo;
    } else {
        n = (n - 1) * fraction_one;
        /* Compute $f=\lfloor 2^{28}(1+p/q)+{1\over2}\rfloor$ */
        /* The |repeat| loop here preserves the following invariant relations
           between |f|, |p|, and~|q|:
           (i)~|0<=p<q|; (ii)~$fq+p=2^k(q+p_0)$, where $k$ is an integer and
           $p_0$ is the original value of~$p$.

           Notice that the computation specifies
           |(p-q)+p| instead of |(p+p)-q|, because the latter could overflow.
           Let us hope that optimizing compilers do not miss this point; a
           special variable |be_careful| is used to emphasize the necessary
           order of computation. Optimizing compilers should keep |be_careful|
           in a register, not store it in memory.
         */
        f = 1;
        do {
            be_careful = p - q;
            p = be_careful + p;
            if (p >= 0)
                f = f + f + 1;
            else {
                f += f;
                p = p + q;
            }
        } while (f < fraction_one);
        be_careful = p - q;
        if (be_careful + p >= 0)
            incr(f);

        if (negative)
            return (-(f + n));
        else
            return (f + n);
    }
}

@ @c
static int take_frac(int q, int f)
{
    int p;                      /* the fraction so far */
    int n;                      /* additional multiple of $q$ */
    register int be_careful;    /* disables certain compiler optimizations */
    boolean negative = false;   /* should the result be negated? */
    /* Reduce to the case that |f>=0| and |q>0| */
    if (f < 0) {
        negate(f);
        negative = true;
    }
    if (q < 0) {
        negate(q);
        negative = !negative;
    }

    if (f < fraction_one) {
        n = 0;
    } else {
        n = f / fraction_one;
        f = f % fraction_one;
        if (q <= el_gordo / n) {
            n = n * q;
        } else {
            arith_error = true;
            n = el_gordo;
        }
    }
    f = f + fraction_one;
    /* Compute $p=\lfloor qf/2^{28}+{1\over2}\rfloor-q$ */
    /* The invariant relations in this case are (i)~$\lfloor(qf+p)/2^k\rfloor
       =\lfloor qf_0/2^{28}+{1\over2}\rfloor$, where $k$ is an integer and
       $f_0$ is the original value of~$f$; (ii)~$2^k\L f<2^{k+1}$. 
     */
    p = fraction_half;          /* that's $2^{27}$; the invariants hold now with $k=28$ */
    if (q < fraction_four) {
        do {
            if (odd(f))
                p = halfp(p + q);
            else
                p = halfp(p);
            f = halfp(f);
        } while (f != 1);
    } else {
        do {
            if (odd(f))
                p = p + halfp(q - p);
            else
                p = halfp(p);
            f = halfp(f);
        } while (f != 1);
    }

    be_careful = n - el_gordo;
    if (be_careful + p > 0) {
        arith_error = true;
        n = el_gordo - p;
    }
    if (negative)
        return (-(n + p));
    else
        return (n + p);
}



@ The subroutines for logarithm and exponential involve two tables.
The first is simple: |two_to_the[k]| equals $2^k$. The second involves
a bit more calculation, which the author claims to have done correctly:
|spec_log[k]| is $2^{27}$ times $\ln\bigl(1/(1-2^{-k})\bigr)=
2^{-k}+{1\over2}2^{-2k}+{1\over3}2^{-3k}+\cdots\,$, rounded to the
nearest integer.

@c
static int two_to_the[31];      /* powers of two */
static int spec_log[29];        /* special logarithms */

@ @c
void initialize_arithmetic(void)
{
    int k;
    two_to_the[0] = 1;
    for (k = 1; k <= 30; k++)
        two_to_the[k] = 2 * two_to_the[k - 1];
    spec_log[1] = 93032640;
    spec_log[2] = 38612034;
    spec_log[3] = 17922280;
    spec_log[4] = 8662214;
    spec_log[5] = 4261238;
    spec_log[6] = 2113709;
    spec_log[7] = 1052693;
    spec_log[8] = 525315;
    spec_log[9] = 262400;
    spec_log[10] = 131136;
    spec_log[11] = 65552;
    spec_log[12] = 32772;
    spec_log[13] = 16385;
    for (k = 14; k <= 27; k++)
        spec_log[k] = two_to_the[27 - k];
    spec_log[28] = 1;
}

@ @c
static int m_log(int x)
{
    int y, z;                   /* auxiliary registers */
    int k;                      /* iteration counter */
    if (x <= 0) {
        /* Handle non-positive logarithm */
        print_err("Logarithm of ");
        print_scaled(x);
        tprint(" has been replaced by 0");
        help2("Since I don't take logs of non-positive numbers,",
              "I'm zeroing this one. Proceed, with fingers crossed.");
        error();
        return 0;
    } else {
        y = 1302456956 + 4 - 100;       /* $14\times2^{27}\ln2\approx1302456956.421063$ */
        z = 27595 + 6553600;    /* and $2^{16}\times .421063\approx 27595$ */
        while (x < fraction_four) {
            x += x;
            y = y - 93032639;
            z = z - 48782;
        }                       /* $2^{27}\ln2\approx 93032639.74436163$
                                   and $2^{16}\times.74436163\approx 48782$ */
        y = y + (z / unity);
        k = 2;
        while (x > fraction_four + 4) {
            /* Increase |k| until |x| can be multiplied by a
               factor of $2^{-k}$, and adjust $y$ accordingly */
            z = ((x - 1) / two_to_the[k]) + 1;  /* $z=\lceil x/2^k\rceil$ */
            while (x < fraction_four + z) {
                z = halfp(z + 1);
                k = k + 1;
            }
            y = y + spec_log[k];
            x = x - z;
        }
        return (y / 8);
    }
}



@ The following somewhat different subroutine tests rigorously if $ab$ is
greater than, equal to, or less than~$cd$,
given integers $(a,b,c,d)$. In most cases a quick decision is reached.
The result is $+1$, 0, or~$-1$ in the three respective cases.

@c
static int ab_vs_cd(int a, int b, int c, int d)
{
    int q, r;                   /* temporary registers */
    /* Reduce to the case that |a,c>=0|, |b,d>0| */
    if (a < 0) {
        negate(a);
        negate(b);
    }
    if (c < 0) {
        negate(c);
        negate(d);
    }
    if (d <= 0) {
        if (b >= 0)
            return (((a == 0 || b == 0) && (c == 0 || d == 0)) ? 0 : 1);
        if (d == 0)
            return (a == 0 ? 0 : -1);
        q = a;
        a = c;
        c = q;
        q = -b;
        b = -d;
        d = q;
    } else if (b <= 0) {
        if (b < 0 && a > 0)
            return -1;
        return (c == 0 ? 0 : -1);
    }

    while (1) {
        q = a / d;
        r = c / b;
        if (q != r)
            return (q > r ? 1 : -1);
        q = a % d;
        r = c % b;
        if (r == 0)
            return (q == 0 ? 0 : 1);
        if (q == 0)
            return -1;
        a = b;
        b = q;
        c = d;
        d = r;                  /* now |a>d>0| and |c>b>0| */
    }
}



@ To consume a random integer, the program below will say `|next_random|'
and then it will fetch |randoms[j_random]|.

@c
#define next_random() do {					\
	if (j_random==0) new_randoms(); else decr(j_random);	\
    } while (0)

static void new_randoms(void)
{
    int k;                      /* index into |randoms| */
    int x;                      /* accumulator */
    for (k = 0; k <= 23; k++) {
        x = randoms[k] - randoms[k + 31];
        if (x < 0)
            x = x + fraction_one;
        randoms[k] = x;
    }
    for (k = 24; k <= 54; k++) {
        x = randoms[k] - randoms[k - 24];
        if (x < 0)
            x = x + fraction_one;
        randoms[k] = x;
    }
    j_random = 54;
}


@ To initialize the |randoms| table, we call the following routine. 

@c
void init_randoms(int seed)
{
    int j, jj, k;               /* more or less random integers */
    int i;                      /* index into |randoms| */
    j = abs(seed);
    while (j >= fraction_one)
        j = halfp(j);
    k = 1;
    for (i = 0; i <= 54; i++) {
        jj = k;
        k = j - k;
        j = jj;
        if (k < 0)
            k = k + fraction_one;
        randoms[(i * 21) % 55] = j;
    }
    new_randoms();
    new_randoms();
    new_randoms();              /* ``warm up'' the array */
}


@ To produce a uniform random number in the range |0<=u<x| or |0>=u>x|
or |0=u=x|, given a |scaled| value~|x|, we proceed as shown here.

Note that the call of |take_frac| will produce the values 0 and~|x|
with about half the probability that it will produce any other particular
values between 0 and~|x|, because it rounds its answers.

@c
int unif_rand(int x)
{
    int y;                      /* trial value */
    next_random();
    y = take_frac(abs(x), randoms[j_random]);
    if (y == abs(x))
        return 0;
    else if (x > 0)
        return y;
    else
        return -y;
}


@ Finally, a normal deviate with mean zero and unit standard deviation
can readily be obtained with the ratio method (Algorithm 3.4.1R in
{\sl The Art of Computer Programming\/}).

@c
int norm_rand(void)
{
    int x, u, l;                /* what the book would call $2^{16}X$, $2^{28}U$, and $-2^{24}\ln U$ */
    do {
        do {
            next_random();
            x = take_frac(112429, randoms[j_random] - fraction_half);
            /* $2^{16}\sqrt{8/e}\approx 112428.82793$ */
            next_random();
            u = randoms[j_random];
        } while (abs(x) >= u);
        x = make_frac(x, u);
        l = 139548960 - m_log(u);       /* $2^{24}\cdot12\ln2\approx139548959.6165$ */
    } while (ab_vs_cd(1024, l, x, x) < 0);
    return x;
}

@ This function could also be expressed as a macro, but it is a useful
   breakpoint for debugging.

@c
int fix_int(int val, int min, int max)
{
    return (val < min ? min : (val > max ? max : val));
}
