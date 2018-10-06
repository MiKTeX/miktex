/*

Copyright 2009 Taco Hoekwater <taco@luatex.org>

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

/*tex

    We keep a stack for |pdfextension setmatrix|:

*/

matrix_entry *matrix_stack = NULL;
int matrix_stack_size = 0;
int matrix_stack_used = 0;

boolean matrixused(void)
{
    return matrix_stack_used > 0;
}

static void matrix_stack_room(void)
{
    matrix_entry *new_stack;
    if (matrix_stack_used >= matrix_stack_size) {
        matrix_stack_size += STACK_INCREMENT;
        new_stack = xtalloc((unsigned) matrix_stack_size, matrix_entry);
        memcpy((void *) new_stack, (void *) matrix_stack, (unsigned) matrix_stack_used * sizeof(matrix_entry));
        xfree(matrix_stack);
        matrix_stack = new_stack;
    }
}

/*tex

    The matrix specification has four entries and gets translated to |e| being
    |pos.h and |f| being pos.v. The current active matrix at the top of the
    matrix stack is kept in |M_top|.

    The origin of \.{\\pdfextension setmatrix} is the current point. The
    annotation coordinate system is the original page coordinate system. When
    pdfTeX calculates annotation rectangles it does not take into account this
    transformations, it uses the original coordinate system. To get the corrected
    values, first we go back to the origin, perform the transformation and go
    back:

    \starttyping
      (  1   0  0 )   ( a b 0 )   ( 1 0 0 )
      (  0   1  0 ) x ( c d 0 ) x ( 0 1 0 ) x M\_top
      ( -e  -f  1 )   ( 0 0 1 )   ( e f 1 )

      ( 1  0  0 )   (  a  b 0 )
    = ( 0  1  0 ) x (  c  d 0 ) x M\_top
      ( e  f  1 )   ( -e -f 1 )

      ( a         b         0 )
    = ( c         d         0 ) x M\_top
      ( e(1-a)-fc f(1-d)-eb 1 )
    \stoptyping

*/

static void pdfsetmatrix(const char *in, scaledpos pos)
{
    /*tex
        The argument of |pdfextension setmatrix| starts with |str_pool[in]|
        and ends before |str_pool[pool_ptr]|.
    */
    matrix_entry x, *y, *z;
    if (global_shipping_mode == SHIPPING_PAGE) {
        if (sscanf((const char *) in, " %lf %lf %lf %lf ", &x.a, &x.b, &x.c, &x.d) != 4) {
            formatted_warning("pdf backend","unrecognized format of setmatrix: %s", in);
            return;
        }
        /*tex Calculate this transformation matrix. */
        x.e = (double) pos.h * (1.0 - x.a) - (double) pos.v * x.c;
        x.f = (double) pos.v * (1.0 - x.d) - (double) pos.h * x.b;
        matrix_stack_room();
        z = &matrix_stack[matrix_stack_used];
        if (matrix_stack_used > 0) {
            y = &matrix_stack[matrix_stack_used - 1];
            z->a = x.a * y->a + x.b * y->c;
            z->b = x.a * y->b + x.b * y->d;
            z->c = x.c * y->a + x.d * y->c;
            z->d = x.c * y->b + x.d * y->d;
            z->e = x.e * y->a + x.f * y->c + y->e;
            z->f = x.e * y->b + x.f * y->d + y->f;
        } else {
            z->a = x.a;
            z->b = x.b;
            z->c = x.c;
            z->d = x.d;
            z->e = x.e;
            z->f = x.f;
        }
        matrix_stack_used++;
    }
}

/*tex

    Apply matrix to point (x,y)

    \starttyping
                ( a b 0 )
    ( x y 1 ) x ( c d 0 ) = ( xa+yc+e xb+yd+f 1 )
                ( e f 1 )
    \stoptyping

    If \.{\\pdfextension setmatrix} wasn't used, then return the value unchanged.
    The return values for matrix tranform functions are:

*/

static scaled ret_llx;
static scaled ret_lly;
static scaled ret_urx;
static scaled ret_ury;

scaled getllx(void)
{
    return ret_llx;
}

scaled getlly(void)
{
    return ret_lly;
}

scaled geturx(void)
{
    return ret_urx;
}

scaled getury(void)
{
    return ret_ury;
}

static int last_llx;
static int last_lly;
static int last_urx;
static int last_ury;

#define DO_ROUND(x)  ((x > 0) ? (x + .5) : (x - .5))
#define DO_MIN(a, b) ((a < b) ? a : b)
#define DO_MAX(a, b) ((a > b) ? a : b)

static void do_matrixtransform(scaled x, scaled y, scaled * retx, scaled * rety)
{
    matrix_entry *m = &matrix_stack[matrix_stack_used - 1];
    double x_old = x;
    double y_old = y;
    double x_new = x_old * m->a + y_old * m->c + m->e;
    double y_new = x_old * m->b + y_old * m->d + m->f;
    *retx = (scaled) DO_ROUND(x_new);
    *rety = (scaled) DO_ROUND(y_new);
}

void matrixtransformrect(scaled llx, scaled lly, scaled urx, scaled ury)
{
    scaled x1, x2, x3, x4, y1, y2, y3, y4;
    if (global_shipping_mode == SHIPPING_PAGE && matrix_stack_used > 0) {
        last_llx = llx;
        last_lly = lly;
        last_urx = urx;
        last_ury = ury;
        do_matrixtransform(llx, lly, &x1, &y1);
        do_matrixtransform(llx, ury, &x2, &y2);
        do_matrixtransform(urx, lly, &x3, &y3);
        do_matrixtransform(urx, ury, &x4, &y4);
        ret_llx = DO_MIN(DO_MIN(x1, x2), DO_MIN(x3, x4));
        ret_lly = DO_MIN(DO_MIN(y1, y2), DO_MIN(y3, y4));
        ret_urx = DO_MAX(DO_MAX(x1, x2), DO_MAX(x3, x4));
        ret_ury = DO_MAX(DO_MAX(y1, y2), DO_MAX(y3, y4));
    } else {
        ret_llx = llx;
        ret_lly = lly;
        ret_urx = urx;
        ret_ury = ury;
    }
}

void matrixtransformpoint(scaled x, scaled y)
{
    if (global_shipping_mode == SHIPPING_PAGE && matrix_stack_used > 0) {
        do_matrixtransform(x, y, &ret_llx, &ret_lly);
    } else {
        ret_llx = x;
        ret_lly = y;
    }
}

void matrixrecalculate(scaled urx)
{
    matrixtransformrect(last_llx, last_lly, urx, last_ury);
}

void pdf_out_setmatrix(PDF pdf, halfword p)
{
    scaledpos pos = pdf->posstruct->pos;
    int old_setting;
    str_number s;
    old_setting = selector;
    selector = new_string;
    show_token_list(token_link(pdf_setmatrix_data(p)), null, -1);
    pdfsetmatrix((char *) cur_string, pos);
    tprint(" 0 0 cm");
    selector = old_setting;
    s = make_string();
    pdf_literal(pdf, s, set_origin, false);
    flush_str(s);
}
