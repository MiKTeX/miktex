/*

writepng.c

Copyright 1996-2006 Han The Thanh <thanh@pdftex.org>
Copyright 2006-2013 Taco Hoekwater <taco@luatex.org>

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

#include "ptexlib.h"
#include <assert.h>
#include "image/image.h"
#include "image/writepng.h"

static void close_and_cleanup_png(image_dict * idict)
{
    if (img_file(idict) != NULL) {
        xfclose(img_file(idict), img_filepath(idict));
        img_file(idict) = NULL;
    }
    if (img_png_ptr(idict) != NULL) {
        png_destroy_read_struct(&(img_png_png_ptr(idict)), &(img_png_info_ptr(idict)), NULL);
        xfree(img_png_ptr(idict));
    }
}

void flush_png_info(image_dict * idict)
{
    close_and_cleanup_png(idict);
}

/*tex A dummy function: */

static void warn(png_structp png_ptr, png_const_charp msg)
{
    (void)png_ptr; (void)msg;
}

void read_png_info(image_dict * idict)
{
    png_structp png_p;
    png_infop info_p;
    if (img_type(idict) != IMG_TYPE_PNG) {
        normal_error("readpng","conflicting image dictionary");
    }
    if (img_file(idict) != NULL) {
        normal_error("readpng","image data already read");
    }
    img_totalpages(idict) = 1;
    img_pagenum(idict) = 1;
    img_xres(idict) = img_yres(idict) = 0;
    img_file(idict) = xfopen(img_filepath(idict), FOPEN_RBIN_MODE);
    img_png_ptr(idict) = xtalloc(1, png_img_struct);
    if ((png_p = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, warn)) == NULL) {
        normal_error("readpng","png_create_read_struct() failed");
    }
    img_png_png_ptr(idict) = png_p;
    if ((info_p = png_create_info_struct(png_p)) == NULL) {
        normal_error("readpng","png_create_info_struct() failed");
    }
    img_png_info_ptr(idict) = info_p;
    if (setjmp(png_jmpbuf(png_p))) {
        normal_error("readpng","internal error");
    }
#if PNG_LIBPNG_VER >= 10603
    /*tex ignore possibly incorrect CMF bytes */
    png_set_option(png_p, PNG_MAXIMUM_INFLATE_WINDOW, PNG_OPTION_ON);
#endif
    png_init_io(png_p, img_file(idict));
    png_read_info(png_p, info_p);
    /*tex resolution support */
    img_xsize(idict) = (int) png_get_image_width(png_p, info_p);
    img_ysize(idict) = (int) png_get_image_height(png_p, info_p);
    if (png_get_valid(png_p, info_p, PNG_INFO_pHYs)) {
        img_xres(idict) = round(0.0254 * (double) png_get_x_pixels_per_meter(png_p, info_p));
        img_yres(idict) = round(0.0254 * (double) png_get_y_pixels_per_meter(png_p, info_p));
    }
    switch (png_get_color_type(png_p, info_p)) {
    case PNG_COLOR_TYPE_PALETTE:
        img_procset(idict) |= PROCSET_IMAGE_C | PROCSET_IMAGE_I;
        break;
    case PNG_COLOR_TYPE_GRAY:
    case PNG_COLOR_TYPE_GRAY_ALPHA:
        img_procset(idict) |= PROCSET_IMAGE_B;
        break;
    case PNG_COLOR_TYPE_RGB:
    case PNG_COLOR_TYPE_RGB_ALPHA:
        img_procset(idict) |= PROCSET_IMAGE_C;
        break;
    default:
        formatted_error("readpng","unsupported type of color_type '%i'",(int) png_get_color_type(png_p, info_p));
    }
    img_colordepth(idict) = png_get_bit_depth(png_p, info_p);
    /*tex So we can optionally keep open a file in |img|. */
    if (! img_keepopen(idict)) {
        close_and_cleanup_png(idict);
    }
}

#define write_gray_pixel_16(r)       \
    if (j % 4 == 0 || j % 4 == 1)    \
        pdf_quick_out(pdf, *r++);    \
    else                             \
        smask[smask_ptr++] = *r++    \

#define write_gray_pixel_8(r)        \
    if (j % 2 == 0)                  \
        pdf_quick_out(pdf, *r++);    \
    else                             \
        smask[smask_ptr++] = *r++

#define write_rgb_pixel_16(r)        \
    if (!(j % 8 == 6 || j % 8 == 7)) \
        pdf_quick_out(pdf, *r++);    \
    else                             \
        smask[smask_ptr++] = *r++

#define write_rgb_pixel_8(r)         \
    if (j % 4 != 3)                  \
        pdf_quick_out(pdf, *r++);    \
    else                             \
        smask[smask_ptr++] = *r++

#define write_simple_pixel(r)    pdf_quick_out(pdf,*r++)

#define write_noninterlaced(outmac)                                   \
    for (i = 0; i < (int) png_get_image_height(png_p, info_p); i++) { \
        png_read_row(png_p, row, NULL);                               \
        r = row;                                                      \
        k = (size_t) png_get_rowbytes(png_p, info_p);                 \
        while (k > 0) {                                               \
            l = (k > pdf->buf->size) ? pdf->buf->size : k;            \
            pdf_room(pdf, l);                                         \
            for (j = 0; j < l; j++) {                                 \
                outmac;                                               \
            }                                                         \
            k -= l;                                                   \
        }                                                             \
    }

#define write_interlaced(outmac)                                      \
    for (i = 0; i < (int) png_get_image_height(png_p, info_p); i++) { \
        row = rows[i];                                                \
        k = (size_t) png_get_rowbytes(png_p, info_p);                 \
        while (k > 0) {                                               \
            l = (k > pdf->buf->size) ? pdf->buf->size : k;            \
            pdf_room(pdf, l);                                         \
            for (j = 0; j < l; j++) {                                 \
                outmac;                                               \
            }                                                         \
            k -= l;                                                   \
        }                                                             \
        xfree(rows[i]);                                               \
    }

static void write_palette_streamobj(PDF pdf, int palette_objnum, png_colorp palette, int num_palette)
{
    int i;
    if (palette_objnum == 0)
        return;
    assert(palette != NULL);
    pdf_begin_obj(pdf, palette_objnum, OBJSTM_NEVER);
    pdf_begin_dict(pdf);
    pdf_dict_add_streaminfo(pdf);
    pdf_end_dict(pdf);
    pdf_begin_stream(pdf);
    for (i = 0; i < num_palette; i++) {
        pdf_room(pdf, 3);
        pdf_quick_out(pdf, palette[i].red);
        pdf_quick_out(pdf, palette[i].green);
        pdf_quick_out(pdf, palette[i].blue);
    }
    pdf_end_stream(pdf);
    pdf_end_obj(pdf);
}

static void write_smask_streamobj(PDF pdf, image_dict * idict, int smask_objnum, png_bytep smask, int smask_size)
{
    int i;
    png_structp png_p = img_png_png_ptr(idict);
    png_infop info_p = img_png_info_ptr(idict);
    png_byte bitdepth = png_get_bit_depth(png_p, info_p);
    pdf_begin_obj(pdf, smask_objnum, OBJSTM_NEVER);
    pdf_begin_dict(pdf);
    pdf_dict_add_name(pdf, "Type", "XObject");
    pdf_dict_add_name(pdf, "Subtype", "Image");
    pdf_dict_add_img_filename(pdf, idict);
    if (img_attr(idict) != NULL && strlen(img_attr(idict)) > 0)
        pdf_printf(pdf, "\n%s\n", img_attr(idict));
    pdf_dict_add_int(pdf, "Width", (int) png_get_image_width(png_p, info_p));
    pdf_dict_add_int(pdf, "Height", (int) png_get_image_height(png_p, info_p));
    pdf_dict_add_int(pdf, "BitsPerComponent", (bitdepth == 16 ? 8 : bitdepth));
    pdf_dict_add_name(pdf, "ColorSpace", "DeviceGray");
    pdf_dict_add_streaminfo(pdf);
    pdf_end_dict(pdf);
    pdf_begin_stream(pdf);
    for (i = 0; i < smask_size; i++) {
        if (i % 8 == 0)
            pdf_room(pdf, 8);
        pdf_quick_out(pdf, smask[i]);
        if (bitdepth == 16)
            i++;
    }
    pdf_end_stream(pdf);
    pdf_end_obj(pdf);
}

static void write_png_gray(PDF pdf, image_dict * idict)
{
    int i;
    size_t j, k, l;
    png_structp png_p = img_png_png_ptr(idict);
    png_infop info_p = img_png_info_ptr(idict);
    png_bytep row, r, *rows;
    pdf_dict_add_streaminfo(pdf);
    pdf_end_dict(pdf);
    pdf_begin_stream(pdf);
    if (png_get_interlace_type(png_p, info_p) == PNG_INTERLACE_NONE) {
        row = xtalloc(png_get_rowbytes(png_p, info_p), png_byte);
        write_noninterlaced(write_simple_pixel(r));
        xfree(row);
    } else {
        if (png_get_image_height(png_p, info_p) * png_get_rowbytes(png_p, info_p) >= 10240000L) {
            formatted_warning("pngwrite","large interlaced bitmap might cause out of memory");
        }
        rows = xtalloc(png_get_image_height(png_p, info_p), png_bytep);
        for (i = 0; i < (int) png_get_image_height(png_p, info_p); i++) {
            rows[i] = xtalloc(png_get_rowbytes(png_p, info_p), png_byte);
        }
        png_read_image(png_p, rows);
        write_interlaced(write_simple_pixel(row));
        xfree(rows);
    }
    pdf_end_stream(pdf);
    pdf_end_obj(pdf);
}

static void write_png_gray_alpha(PDF pdf, image_dict * idict)
{
    int i;
    size_t j, k, l;
    png_structp png_p = img_png_png_ptr(idict);
    png_infop info_p = img_png_info_ptr(idict);
    png_bytep row, r, *rows;
    int smask_objnum = 0;
    png_bytep smask;
    int smask_ptr = 0;
    int smask_size = 0;
    smask_objnum = pdf_create_obj(pdf, obj_type_others, 0);
    pdf_dict_add_ref(pdf, "SMask", (int) smask_objnum);
    smask_size = (int) ((png_get_rowbytes(png_p, info_p) / 2) * png_get_image_height(png_p, info_p));
    smask = xtalloc((unsigned) smask_size, png_byte);
    pdf_dict_add_streaminfo(pdf);
    pdf_end_dict(pdf);
    pdf_begin_stream(pdf);
    if (png_get_interlace_type(png_p, info_p) == PNG_INTERLACE_NONE) {
        row = xtalloc(png_get_rowbytes(png_p, info_p), png_byte);
        if ((png_get_bit_depth(png_p, info_p) == 16) && (pdf->image_hicolor != 0)) {
            write_noninterlaced(write_gray_pixel_16(r));
        } else {
            write_noninterlaced(write_gray_pixel_8(r));
        }
        xfree(row);
    } else {
        if (png_get_image_height(png_p, info_p) * png_get_rowbytes(png_p, info_p) >= 10240000L) {
            formatted_warning("pngwrite","large interlaced bitmap might cause out of memory");
        }
        rows = xtalloc(png_get_image_height(png_p, info_p), png_bytep);
        for (i = 0; i < (int) png_get_image_height(png_p, info_p); i++) {
            rows[i] = xtalloc(png_get_rowbytes(png_p, info_p), png_byte);
        }
        png_read_image(png_p, rows);
        if ((png_get_bit_depth(png_p, info_p) == 16) && (pdf->image_hicolor != 0)) {
            write_interlaced(write_gray_pixel_16(row));
        } else {
            write_interlaced(write_gray_pixel_8(row));
        }
        xfree(rows);
    }
    pdf_end_stream(pdf);
    pdf_end_obj(pdf);
    write_smask_streamobj(pdf, idict, smask_objnum, smask, smask_size);
    xfree(smask);
}

static void write_png_rgb_alpha(PDF pdf, image_dict * idict)
{
    int i;
    size_t j, k, l;
    png_structp png_p = img_png_png_ptr(idict);
    png_infop info_p = img_png_info_ptr(idict);
    png_bytep row, r, *rows;
    int smask_objnum = 0;
    png_bytep smask;
    int smask_ptr = 0;
    int smask_size = 0;
    smask_objnum = pdf_create_obj(pdf, obj_type_others, 0);
    pdf_dict_add_ref(pdf, "SMask", (int) smask_objnum);
    smask_size = (int) ((png_get_rowbytes(png_p, info_p) / 4) * png_get_image_height(png_p, info_p));
    smask = xtalloc((unsigned) smask_size, png_byte);
    pdf_dict_add_streaminfo(pdf);
    pdf_end_dict(pdf);
    pdf_begin_stream(pdf);
    if (png_get_interlace_type(png_p, info_p) == PNG_INTERLACE_NONE) {
        row = xtalloc(png_get_rowbytes(png_p, info_p), png_byte);
        if ((png_get_bit_depth(png_p, info_p) == 16) && (pdf->image_hicolor != 0)) {
            write_noninterlaced(write_rgb_pixel_16(r));
        } else {
            write_noninterlaced(write_rgb_pixel_8(r));
        }
        xfree(row);
    } else {
        if (png_get_image_height(png_p, info_p) * png_get_rowbytes(png_p, info_p) >= 10240000L) {
            formatted_warning("pngwrite","large interlaced bitmap might cause out of memory");
        }
        rows = xtalloc(png_get_image_height(png_p, info_p), png_bytep);
        for (i = 0; i < (int) png_get_image_height(png_p, info_p); i++) {
            rows[i] = xtalloc(png_get_rowbytes(png_p, info_p), png_byte);
        }
        png_read_image(png_p, rows);
        if ((png_get_bit_depth(png_p, info_p) == 16) && (pdf->image_hicolor != 0)) {
            write_interlaced(write_rgb_pixel_16(row));
        } else {
            write_interlaced(write_rgb_pixel_8(row));
        }
        xfree(rows);
    }
    pdf_end_stream(pdf);
    pdf_end_obj(pdf);
    write_smask_streamobj(pdf, idict, smask_objnum, smask, smask_size);
    xfree(smask);
}

/*tex

The |copy_png| code is cheerfully gleaned from Thomas Merz' PDFlib, file
|p_png.c| ``SPNG - Simple PNG''. The goal is to use pdf's native FlateDecode
support, if that is possible. Only a subset of the png files allows this, but for
these it greatly improves inclusion speed.

In the ``PNG Copy'' mode only the IDAT chunks are copied; all other chunks from
the PNG file are discarded. If there are any other chunks in the PNG file, which
might influence the visual appearance of the image, or if image processing like
gamma change is requested, the ``PNG Copy'' function must be skipped; therefore
the lengthy tests.

*/

static int spng_getint(FILE * f)
{
    unsigned char buf[4];
    if (fread(buf, 1, 4, f) != 4) {
        normal_error("writepng", "reading chunk type failed");
    }
    return ((((((int) buf[0] << 8) + buf[1]) << 8) + buf[2]) << 8) + buf[3];
}

#define SPNG_CHUNK_IDAT 0x49444154
#define SPNG_CHUNK_IEND 0x49454E44

static void copy_png(PDF pdf, image_dict * idict)
{
    int type, streamlength = 0, idat = 0;
    size_t len;
    boolean endflag = false;
    FILE *f;
    png_structp png_p;
    png_infop info_p;
    assert(idict != NULL);
    png_p = img_png_png_ptr(idict);
    info_p = img_png_info_ptr(idict);
    f = (FILE *) png_get_io_ptr(png_p);
    /*tex 1st pass to find overall stream /Length */
    if (fseek(f, 8, SEEK_SET) != 0)
        normal_error("writepng", "fseek in file failed");
    do {
        len = spng_getint(f);
        type = spng_getint(f);
        switch (type) {
        case SPNG_CHUNK_IEND:
            endflag = true;
            break;
        case SPNG_CHUNK_IDAT:
            streamlength += len;
        default:
            if (fseek(f, len + 4, SEEK_CUR) != 0) {
                normal_error("writepng", "fseek in file failed");
            }
        }
    } while (endflag == false);
    pdf_dict_add_int(pdf, "Length", streamlength);
    pdf_dict_add_name(pdf, "Filter", "FlateDecode");
    pdf_add_name(pdf, "DecodeParms");
    pdf_begin_dict(pdf);
    pdf_dict_add_int(pdf, "Colors", png_get_color_type(png_p,info_p) == PNG_COLOR_TYPE_RGB ? 3 : 1);
    pdf_dict_add_int(pdf, "Columns", png_get_image_width(png_p, info_p));
    pdf_dict_add_int(pdf, "BitsPerComponent", png_get_bit_depth(png_p, info_p));
    pdf_dict_add_int(pdf, "Predictor", 10);
    pdf_end_dict(pdf);
    pdf_end_dict(pdf);
    pdf_begin_stream(pdf);
    /*tex 2nd pass to copy data */
    endflag = false;
    if (fseek(f, 8, SEEK_SET) != 0)
        normal_error("writepng", "fseek in file failed");
    do {
        len = spng_getint(f);
        type = spng_getint(f);
        switch (type) {
        case SPNG_CHUNK_IDAT:
            /*tex do copy */
            if (idat == 2) {
                normal_error("writepng", "IDAT chunk sequence broken");
            }
            idat = 1;
            if (read_file_to_buf(pdf, f, len) != len) {
                normal_error("writepng", "fread failed");
            } else if (fseek(f, 4, SEEK_CUR) != 0) {
                normal_error("writepng", "fseek in file failed");
            }
            break;
        case SPNG_CHUNK_IEND:
            /*tex done */
            endflag = true;
            break;
        default:
            if (idat == 1)
                idat = 2;
            if (fseek(f, len + 4, SEEK_CUR) != 0)
                normal_error("writepng", "fseek in file failed");
        }
    } while (endflag == false);
    pdf_end_stream(pdf);
    pdf_end_obj(pdf);
}

static void reopen_png(image_dict * idict)
{
    int width, height, xres, yres;
    /*tex A consistency check: */
    width = img_xsize(idict);
    height = img_ysize(idict);
    xres = img_xres(idict);
    yres = img_yres(idict);
    /*tex We need to ake sure that the file kept open. */
    img_keepopen(idict) = 1;
    read_png_info(idict);
    if (width != img_xsize(idict) || height != img_ysize(idict) || xres != img_xres(idict) || yres != img_yres(idict)) {
        normal_error("writepng", "image dimensions have changed");
    }
}

void write_png(PDF pdf, image_dict * idict)
{
#ifndef PNG_FP_1
    /*tex for libpng < 1.5.0 */
#  define PNG_FP_1    100000
#endif
    int num_palette, palette_objnum = 0;
    boolean png_copy = true;
    double gamma = 0.0;
    png_fixed_point int_file_gamma = 0;
    png_structp png_p;
    png_infop info_p;
    png_colorp palette;
    assert(idict != NULL);
    if (img_file(idict) == NULL)
        reopen_png(idict);
    assert(img_png_ptr(idict) != NULL);
    png_p = img_png_png_ptr(idict);
    info_p = img_png_info_ptr(idict);
    /*tex simple transparency support */
    if (png_get_valid(png_p, info_p, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_p);
        png_copy = false;
    }
    /*tex alpha channel support */
    if (pdf->minor_version < 4
        && png_get_color_type(png_p, info_p) | PNG_COLOR_MASK_ALPHA) {
        png_set_strip_alpha(png_p);
        png_copy = false;
    }
    /*tex 16 bit depth support */
    if (pdf->minor_version < 5)
        pdf->image_hicolor = 0;
    if ((png_get_bit_depth(png_p, info_p) == 16) && (pdf->image_hicolor == 0)) {
        png_set_strip_16(png_p);
        png_copy = false;
    }
    /*tex gamma support */
    if (png_get_valid(png_p, info_p, PNG_INFO_gAMA)) {
        png_get_gAMA(png_p, info_p, &gamma);
        png_get_gAMA_fixed(png_p, info_p, &int_file_gamma);
    }
    if (pdf->image_apply_gamma) {
        if (png_get_valid(png_p, info_p, PNG_INFO_gAMA))
            png_set_gamma(png_p, (pdf->gamma / 1000.0), gamma);
        else
            png_set_gamma(png_p, (pdf->gamma / 1000.0), (1000.0 / pdf->image_gamma));
        png_copy = false;
    }
    /*tex reset structure */
    (void) png_set_interlace_handling(png_p);
    png_read_update_info(png_p, info_p);
    pdf_begin_obj(pdf, img_objnum(idict), OBJSTM_NEVER);
    pdf_begin_dict(pdf);
    pdf_dict_add_name(pdf, "Type", "XObject");
    pdf_dict_add_name(pdf, "Subtype", "Image");
    pdf_dict_add_img_filename(pdf, idict);
    if (img_attr(idict) != NULL && strlen(img_attr(idict)) > 0) {
        pdf_printf(pdf, "\n%s\n", img_attr(idict));
    }
    pdf_dict_add_int(pdf, "Width", (int) png_get_image_width(png_p, info_p));
    pdf_dict_add_int(pdf, "Height", (int) png_get_image_height(png_p, info_p));
    pdf_dict_add_int(pdf, "BitsPerComponent", (int) png_get_bit_depth(png_p, info_p));
    if (img_colorspace(idict) != 0) {
        pdf_dict_add_ref(pdf, "ColorSpace", (int) img_colorspace(idict));
    } else {
        switch (png_get_color_type(png_p, info_p)) {
            case PNG_COLOR_TYPE_PALETTE:
                png_get_PLTE(png_p, info_p, &palette, &num_palette);
                palette_objnum = pdf_create_obj(pdf, obj_type_others, 0);
                pdf_add_name(pdf, "ColorSpace");
                pdf_begin_array(pdf);
                pdf_add_name(pdf, "Indexed");
                pdf_add_name(pdf, "DeviceRGB");
                /*tex hival */
                pdf_add_int(pdf, (int) (num_palette - 1));
                /*tex lookup */
                pdf_add_ref(pdf, (int) palette_objnum);
                pdf_end_array(pdf);
                break;
            case PNG_COLOR_TYPE_GRAY:
            case PNG_COLOR_TYPE_GRAY_ALPHA:
                pdf_dict_add_name(pdf, "ColorSpace", "DeviceGray");
                break;
            case PNG_COLOR_TYPE_RGB:
            case PNG_COLOR_TYPE_RGB_ALPHA:
                pdf_dict_add_name(pdf, "ColorSpace", "DeviceRGB");
                break;
            default:
                formatted_error("writepng", "unsupported color_type '%i'", png_get_color_type(png_p, info_p));
        }
    }
    if (png_copy
        && pdf->minor_version > 1
        && png_get_interlace_type(png_p, info_p) == PNG_INTERLACE_NONE
        && (png_get_color_type(png_p, info_p) == PNG_COLOR_TYPE_GRAY
         || png_get_color_type(png_p, info_p) == PNG_COLOR_TYPE_RGB)
        && !pdf->image_apply_gamma
        && (!png_get_valid(png_p, info_p, PNG_INFO_gAMA) || int_file_gamma == PNG_FP_1)
        && !png_get_valid(png_p, info_p, PNG_INFO_cHRM)
        && !png_get_valid(png_p, info_p, PNG_INFO_iCCP)
        && !png_get_valid(png_p, info_p, PNG_INFO_sBIT)
        && !png_get_valid(png_p, info_p, PNG_INFO_sRGB)
        && !png_get_valid(png_p, info_p, PNG_INFO_bKGD)
        && !png_get_valid(png_p, info_p, PNG_INFO_hIST)
        && !png_get_valid(png_p, info_p, PNG_INFO_tRNS)
        && !png_get_valid(png_p, info_p, PNG_INFO_sPLT)) {
        copy_png(pdf, idict);
    } else {
        if (img_errorlevel(idict) > 1) {
            if (!png_copy)
                normal_warning("pngcopy","failed");
            if (!(pdf->minor_version > 1))
                formatted_warning("pngcopy","skipped because minorversion is '%d'", pdf->minor_version);
            if (!(png_get_interlace_type(png_p, info_p) == PNG_INTERLACE_NONE))
                normal_warning("pngcopy","skipped because of interlacing");
            if (!((png_get_color_type(png_p, info_p) == PNG_COLOR_TYPE_GRAY)
               || (png_get_color_type(png_p, info_p) == PNG_COLOR_TYPE_RGB)))
                normal_warning("pngcopy","skipped because of colortype");
            if (pdf->image_apply_gamma)
                normal_warning("pngcopy","skipped because of gamma (1)");
            if (!(!png_get_valid(png_p, info_p, PNG_INFO_gAMA) || int_file_gamma == PNG_FP_1))
                normal_warning("pngcopy","skipped because of gamma (2)");
            if (png_get_valid(png_p, info_p, PNG_INFO_cHRM))
                normal_warning("pngcopy","skipped because of cHRM");
            if (png_get_valid(png_p, info_p, PNG_INFO_iCCP))
                normal_warning("pngcopy","skipped because of iCCP");
            if (png_get_valid(png_p, info_p, PNG_INFO_sBIT))
                normal_warning("pngcopy","skipped because of sBIT");
            if (png_get_valid(png_p, info_p, PNG_INFO_sRGB))
                normal_warning("pngcopy","skipped because of sRGB");
            if (png_get_valid(png_p, info_p, PNG_INFO_bKGD))
                normal_warning("pngcopy","skipped because of bKGD");
            if (png_get_valid(png_p, info_p, PNG_INFO_hIST))
                normal_warning("pngcopy","skipped because of hIST");
            if (png_get_valid(png_p, info_p, PNG_INFO_tRNS))
                normal_warning("pngcopy","skipped because of tRNS");
            if (png_get_valid(png_p, info_p, PNG_INFO_sPLT))
                normal_warning("pngcopy","skipped because of sPLT");
        }
        switch (png_get_color_type(png_p, info_p)) {
            case PNG_COLOR_TYPE_PALETTE:
            case PNG_COLOR_TYPE_GRAY:
            case PNG_COLOR_TYPE_RGB:
                write_png_gray(pdf, idict);
                break;
            case PNG_COLOR_TYPE_GRAY_ALPHA:
                if (pdf->minor_version >= 4) {
                    write_png_gray_alpha(pdf, idict);
                } else
                    write_png_gray(pdf, idict);
                break;
            case PNG_COLOR_TYPE_RGB_ALPHA:
                if (pdf->minor_version >= 4) {
                    write_png_rgb_alpha(pdf, idict);
                } else
                    write_png_gray(pdf, idict);
                break;
            default:
                assert(0);
        }
    }
    write_palette_streamobj(pdf, palette_objnum, palette, num_palette);
    /*tex always */
    close_and_cleanup_png(idict);
}


/*tex

    Called after the xobject generated by |write_png| has been finished; used to
    write out additional objects.

*/

void write_additional_png_objects(PDF pdf)
{
    (void) pdf;
    return;
}
