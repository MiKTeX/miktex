/*

Copyright 2009-2010 Taco Hoekwater <taco@luatex.org>

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

void place_img(PDF pdf, image_dict * idict, scaled_whd dim, int transform)
{
    /*tex A transformation matrix: */
    double a[6];
    double xoff, yoff, tmp;
    pdfstructure *p = pdf->pstruct;
    scaledpos pos = pdf->posstruct->pos;
    /*tex The number of digits after the decimal point: */
    int r;
    int k;
    scaledpos tmppos;
    pdffloat cm[6];
    int groupref;
    a[0] = a[3] = 1.0e6;
    a[1] = a[2] = 0;
    if (img_type(idict) == IMG_TYPE_PDF || img_type(idict) == IMG_TYPE_PDFMEMSTREAM
        || img_type(idict) == IMG_TYPE_PDFSTREAM) {
        a[0] /= (double) img_xsize(idict);
        a[3] /= (double) img_ysize(idict);
        xoff = (double) img_xorig(idict) / (double) img_xsize(idict);
        yoff = (double) img_yorig(idict) / (double) img_ysize(idict);
        r = 6;
    } else {
        if (img_type(idict) == IMG_TYPE_PNG) {
            groupref = img_group_ref(idict);
            if ((groupref > 0) && (pdf->img_page_group_val == 0))
                pdf->img_page_group_val = groupref;
        }
        a[0] /= (double) one_hundred_bp;
        a[3] = a[0];
        xoff = yoff = 0;
        r = 4;
    }
    if ((transform & 7) > 3) {
        /*tex Mirror cases: */
        a[0] *= -1;
        xoff *= -1;
    }
    switch ((transform + img_rotation(idict)) & 3) {
        case 0:
            /*tex No transform. */
            break;
        case 1:
            /*tex rotation over 90 degrees (counterclockwise) */
            a[1] = a[0];
            a[2] = -a[3];
            a[3] = a[0] = 0;
            tmp = yoff;
            yoff = xoff;
            xoff = -tmp;
            break;
        case 2:
            /*tex rotation over 180 degrees (counterclockwise) */
            a[0] *= -1;
            a[3] *= -1;
            xoff *= -1;
            yoff *= -1;
            break;
        case 3:
            /*tex rotation over 270 degrees (counterclockwise) */
            a[1] = -a[0];
            a[2] = a[3];
            a[3] = a[0] = 0;
            tmp = yoff;
            yoff = -xoff;
            xoff = tmp;
            break;
        default:;
    }
    xoff *= (double) dim.wd;
    yoff *= (double) (dim.ht + dim.dp);
    a[0] *= (double) dim.wd;
    a[1] *= (double) (dim.ht + dim.dp);
    a[2] *= (double) dim.wd;
    a[3] *= (double) (dim.ht + dim.dp);
    a[4] = (double) pos.h - xoff;
    a[5] = (double) pos.v - yoff;
    k = transform + img_rotation(idict);
    if ((transform & 7) > 3)
        k++;
    switch (k & 3) {
        case 0:
            /*tex No transform */
            break;
        case 1:
            /*tex rotation over 90 degrees (counterclockwise) */
            a[4] += (double) dim.wd;
            break;
        case 2:
            /*tex rotation over 180 degrees (counterclockwise) */
            a[4] += (double) dim.wd;
            a[5] += (double) (dim.ht + dim.dp);
            break;
        case 3:
            /*tex rotation over 270 degrees (counterclockwise) */
            a[5] += (double) (dim.ht + dim.dp);
            break;
        default:;
    }
    setpdffloat(cm[0], i64round(a[0]), r);
    setpdffloat(cm[1], i64round(a[1]), r);
    setpdffloat(cm[2], i64round(a[2]), r);
    setpdffloat(cm[3], i64round(a[3]), r);
    tmppos.h = round(a[4]);
    tmppos.v = round(a[5]);
    pdf_goto_pagemode(pdf);
    (void) calc_pdfpos(p, tmppos);
    cm[4] = p->cm[4];
    cm[5] = p->cm[5];
    if (pdf->img_page_group_val == 0)
        pdf->img_page_group_val = img_group_ref(idict);
    pdf_puts(pdf, "q\n");
    pdf_print_cm(pdf, cm);
    pdf_puts(pdf, "/Im");
    pdf_print_int(pdf, img_index(idict));
    pdf_print_resname_prefix(pdf);
    pdf_puts(pdf, " Do\nQ\n");
    addto_page_resources(pdf, obj_type_ximage, img_objnum(idict));
    if (img_state(idict) < DICT_OUTIMG)
        img_state(idict) = DICT_OUTIMG;
}

/*tex For normal output see |pdflistout.c|: */

void pdf_place_image(PDF pdf, halfword p)
{
    scaled_whd dim;
    image_dict *idict = idict_array[rule_index(p)];
    dim.wd = width(p);
    dim.ht = height(p);
    dim.dp = depth(p);
    place_img(pdf, idict, dim, rule_transform(p));
}
