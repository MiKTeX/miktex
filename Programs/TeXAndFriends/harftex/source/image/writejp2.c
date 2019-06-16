/*

writejp2.c

Copyright 2011-2013 Taco Hoekwater <taco@luatex.org>
Copyright 2011-2013 Hartmut Henkel <hartmut@luatex.org>

This file is part of LuaTeX.

LuaTeX is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

LuaTeX is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU General Public License along with
LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

/*tex

    Basic JPEG~2000 image support. Section and Table references below:
    Information technology --- JPEG~2000 image coding system: Core coding system.
    ISO/IEC 15444-1, Second edition, 2004-09-15, file |15444-1annexi.pdf|.

*/

#include "ptexlib.h"
#include <math.h>
#include <assert.h>
#include "image/image.h"
#include "image/writejp2.h"
#include "image/writejbig2.h"

/*tex Table 1.2: Defined boxes */

#define BOX_JP   0x6A502020
#define BOX_FTYP 0x66747970
#define BOX_JP2H 0x6a703268
#define BOX_IHDR 0x69686472
#define BOX_BPCC 0x62706363
#define BOX_COLR 0x636D6170
#define BOX_CDEF 0x63646566
#define BOX_RES  0x72657320
#define BOX_RESC 0x72657363
#define BOX_RESD 0x72657364
#define BOX_JP2C 0x6A703263

/*tex Table 1.4: Box definition */

typedef struct {
    uint64_t lbox;
    unsigned int tbox;
} hdr_struct;

static uint64_t read8bytes(FILE * f)
{
    uint64_t l = read4bytes(f);
    l = (l << 32) + read4bytes(f);
    return l;
}

static hdr_struct read_boxhdr(image_dict * idict)
{
    hdr_struct hdr;
    hdr.lbox = read4bytes(img_file(idict));
    hdr.tbox = read4bytes(img_file(idict));
    if (hdr.lbox == 1)
        hdr.lbox = read8bytes(img_file(idict));
    if (hdr.lbox == 0 && hdr.tbox != BOX_JP2C) {
        normal_error("readjp2","LBox == 0");
    }
    return hdr;
}

/*tex Table 1.5.3.1: Image Header box */

static void scan_ihdr(image_dict * idict)
{
    unsigned int height, width;
    unsigned char bpc;
    height = read4bytes(img_file(idict));
    width = read4bytes(img_file(idict));
    img_ysize(idict) = (int) height;
    img_xsize(idict) = (int) width;
    (void) read2bytes(img_file(idict)); /* nc */
    bpc = (unsigned char) xgetc(img_file(idict));
    img_colordepth(idict) = bpc + 1;
    (void) xgetc(img_file(idict));      /* c */
    (void) xgetc(img_file(idict));      /* unkc */
    (void) xgetc(img_file(idict));      /* ipr */
}

/*tex Table 1.5.3.7.1: Capture Resolution box */

/*tex Table 1.5.3.7.2: Default Display Resolution box */

static void scan_resc_resd(image_dict * idict)
{
    unsigned int vr_n, vr_d, hr_n, hr_d;
    unsigned char vr_e, hr_e;
    double hr_, vr_;
    vr_n = read2bytes(img_file(idict));
    vr_d = read2bytes(img_file(idict));
    hr_n = read2bytes(img_file(idict));
    hr_d = read2bytes(img_file(idict));
    vr_e = (unsigned char) xgetc(img_file(idict));
    hr_e = (unsigned char) xgetc(img_file(idict));
    hr_ = ((double) hr_n / hr_d) * exp(hr_e * log(10.0)) * 0.0254;
    vr_ = ((double) vr_n / vr_d) * exp(vr_e * log(10.0)) * 0.0254;
    img_xres(idict) = (int) (hr_ + 0.5);
    img_yres(idict) = (int) (vr_ + 0.5);
}

/*tex Table 1.5.3.7: Resolution box (superbox) */

static void scan_res(image_dict * idict, uint64_t epos_s)
{
    hdr_struct hdr;
    uint64_t spos, epos;
    epos = xftell(img_file(idict), img_filepath(idict));
    while (1) {
        spos = epos;
        hdr = read_boxhdr(idict);
        epos = spos + hdr.lbox;
        switch (hdr.tbox) {
            case (BOX_RESC):
                /*tex arbitrary: let BOX_RESD have precedence */
                if (img_xres(idict) == 0 && img_yres(idict) == 0) {
                    scan_resc_resd(idict);
                    if (xftell(img_file(idict), img_filepath(idict)) != (long)epos)
                        normal_error("readjp2","resc box size inconsistent");
                }
                break;
            case (BOX_RESD):
                scan_resc_resd(idict);
                if (xftell(img_file(idict), img_filepath(idict)) != (long)epos)
                    normal_error("readjp2","resd box size inconsistent");
                break;
            default:;
        }
        if (epos > epos_s)
            normal_error("readjp2","res box size inconsistent");
        if (epos == epos_s)
            break;
        xfseek(img_file(idict), (long) epos, SEEK_SET, img_filepath(idict));
    }
}

/*tex Table 1.5.3: JP2 Header box (superbox) */

static boolean scan_jp2h(image_dict * idict, uint64_t epos_s)
{
    boolean ihdr_found = false;
    hdr_struct hdr;
    uint64_t spos, epos;
    epos = xftell(img_file(idict), img_filepath(idict));
    while (1) {
        spos = epos;
        hdr = read_boxhdr(idict);
        epos = spos + hdr.lbox;
        switch (hdr.tbox) {
        case (BOX_IHDR):
            scan_ihdr(idict);
            if (xftell(img_file(idict), img_filepath(idict)) != (long)epos)
                normal_error("readjp2","ihdr box size inconsistent");
            ihdr_found = true;
            break;
        case (BOX_RES):
            scan_res(idict, epos);
            break;
        default:;
        }
        if (epos > epos_s)
            normal_error("readjp2","jp2h box size inconsistent");
        if (epos == epos_s)
            break;
        xfseek(img_file(idict), (long) epos, SEEK_SET, img_filepath(idict));
    }
    return ihdr_found;
}

static void close_and_cleanup_jp2(image_dict * idict)
{
    /*tex If one of then is not NULL we already cleaned up. */
    if (img_file(idict) != NULL) {
        xfclose(img_file(idict), img_filepath(idict));
        img_file(idict) = NULL;
    }
    if (img_jp2_ptr(idict) != NULL) {
        xfree(img_jp2_ptr(idict));
    }
}

void flush_jp2_info(image_dict * idict)
{
    close_and_cleanup_jp2(idict);
}

void read_jp2_info(image_dict * idict)
{
    boolean ihdr_found = false;
    hdr_struct hdr;
    uint64_t spos, epos;
    if (img_type(idict) != IMG_TYPE_JP2) {
        normal_error("readjp2","conflicting image dictionary");
    }
    if (img_file(idict) != NULL) {
        normal_error("readjp2","image data already read");
    }
    img_totalpages(idict) = 1;
    img_pagenum(idict) = 1;
    img_xres(idict) = img_yres(idict) = 0;
    img_file(idict) = xfopen(img_filepath(idict), FOPEN_RBIN_MODE);
    img_jp2_ptr(idict) = xtalloc(1, jp2_img_struct);
    xfseek(img_file(idict), 0, SEEK_END, img_filepath(idict));
    img_jp2_ptr(idict)->length = (int) xftell(img_file(idict), img_filepath(idict));
    xfseek(img_file(idict), 0, SEEK_SET, img_filepath(idict));
    if (sizeof(uint64_t) < 8) {
        normal_error("readjp2","size problem");
    }
    spos = epos = 0;
    /*tex Table 1.5.1: JPEG 2000 Signature box */
    hdr = read_boxhdr(idict);
    epos = spos + hdr.lbox;
    xfseek(img_file(idict), (long) epos, SEEK_SET, img_filepath(idict));
    /*tex Table 1.5.2: File Type box */
    spos = epos;
    hdr = read_boxhdr(idict);
    if (hdr.tbox != BOX_FTYP) {
        normal_error("readjp2","missing ftyp box");
    }
    epos = spos + hdr.lbox;
    xfseek(img_file(idict), (long) epos, SEEK_SET, img_filepath(idict));
    while (!ihdr_found) {
        spos = epos;
        hdr = read_boxhdr(idict);
        epos = spos + hdr.lbox;
        switch (hdr.tbox) {
        case BOX_JP2H:
            ihdr_found = scan_jp2h(idict, epos);
            break;
        case BOX_JP2C:
            if (!ihdr_found)
                normal_error("readjp2","no ihdr box found");
            break;
        default:;
        }
        xfseek(img_file(idict), (long) epos, SEEK_SET, img_filepath(idict));
    }
    if (! img_keepopen(idict)) {
        close_and_cleanup_jp2(idict);
    }
}

static void reopen_jp2(image_dict * idict)
{
    int width, height, xres, yres;
    width = img_xsize(idict);
    height = img_ysize(idict);
    xres = img_xres(idict);
    yres = img_yres(idict);
    /*tex We need to make sure that the file kept open. */
    img_keepopen(idict) = 1;
    read_jp2_info(idict);
    if (width != img_xsize(idict) || height != img_ysize(idict)
            || xres != img_xres(idict) || yres != img_yres(idict)) {
        normal_error("writejp2","image dimensions have changed");
    }
}

void write_jp2(PDF pdf, image_dict * idict)
{
    long unsigned l;
    if (img_file(idict) == NULL) {
        reopen_jp2(idict);
    }
    pdf_begin_obj(pdf, img_objnum(idict), OBJSTM_NEVER);
    pdf_begin_dict(pdf);
    pdf_dict_add_name(pdf, "Type", "XObject");
    pdf_dict_add_name(pdf, "Subtype", "Image");
    pdf_dict_add_img_filename(pdf, idict);
    if (img_attr(idict) != NULL && strlen(img_attr(idict)) > 0)
        pdf_printf(pdf, "\n%s\n", img_attr(idict));
    pdf_dict_add_int(pdf, "Width", (int) img_xsize(idict));
    pdf_dict_add_int(pdf, "Height", (int) img_ysize(idict));
    pdf_dict_add_int(pdf, "Length", (int) img_jp2_ptr(idict)->length);
    pdf_dict_add_name(pdf, "Filter", "JPXDecode");
    pdf_end_dict(pdf);
    pdf_begin_stream(pdf);
    l = (long unsigned int) img_jp2_ptr(idict)->length;
    xfseek(img_file(idict), 0, SEEK_SET, img_filepath(idict));
    if (read_file_to_buf(pdf, img_file(idict), l) != l)
        normal_error("writejp2","fread failed");
    pdf_end_stream(pdf);
    pdf_end_obj(pdf);
    /*tex We always:*/
    close_and_cleanup_jp2(idict);
}
