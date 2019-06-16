/*

writejpg.w

Copyright 1996-2006 Han The Thanh <thanh@pdftex.org>
Copyright 2006-2011 Taco Hoekwater <taco@luatex.org>

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
#include "image/writejpg.h"

#define JPG_GRAY  1  /* Gray color space, use /DeviceGray    */
#define JPG_RGB   3  /* RGB color space, use /DeviceRGB      */
#define JPG_CMYK  4  /* CMYK color space, use /DeviceCMYK    */

typedef enum {
    M_SOF0  = 0xc0,  /* baseline DCT                         */
    M_SOF1  = 0xc1,  /* extended sequential DCT              */
    M_SOF2  = 0xc2,  /* progressive DCT                      */
    M_SOF3  = 0xc3,  /* lossless (sequential)                */

    M_SOF5  = 0xc5,  /* differential sequential DCT          */
    M_SOF6  = 0xc6,  /* differential progressive DCT         */
    M_SOF7  = 0xc7,  /* differential lossless (sequential)   */

    M_JPG   = 0xc8,  /* reserved for JPEG extensions         */
    M_SOF9  = 0xc9,  /* extended sequential DCT              */
    M_SOF10 = 0xca,  /* progressive DCT                      */
    M_SOF11 = 0xcb,  /* lossless (sequential)                */

    M_SOF13 = 0xcd,  /* differential sequential DCT          */
    M_SOF14 = 0xce,  /* differential progressive DCT         */
    M_SOF15 = 0xcf,  /* differential lossless (sequential)   */

    M_DHT   = 0xc4,  /* define Huffman table(s)              */

    M_DAC   = 0xcc,  /* define arithmetic conditioning table */

    M_RST0  = 0xd0,  /* restart                              */
    M_RST1  = 0xd1,  /* restart                              */
    M_RST2  = 0xd2,  /* restart                              */
    M_RST3  = 0xd3,  /* restart                              */
    M_RST4  = 0xd4,  /* restart                              */
    M_RST5  = 0xd5,  /* restart                              */
    M_RST6  = 0xd6,  /* restart                              */
    M_RST7  = 0xd7,  /* restart                              */

    M_SOI   = 0xd8,  /* start of image                       */
    M_EOI   = 0xd9,  /* end of image                         */
    M_SOS   = 0xda,  /* start of scan                        */
    M_DQT   = 0xdb,  /* define quantization tables           */
    M_DNL   = 0xdc,  /* define number of lines               */
    M_DRI   = 0xdd,  /* define restart interval              */
    M_DHP   = 0xde,  /* define hierarchical progression      */
    M_EXP   = 0xdf,  /* expand reference image(s)            */

    M_APP0  = 0xe0,  /* application marker, used for JFIF    */
    M_APP1  = 0xe1,  /* application marker                   */
    M_APP2  = 0xe2,  /* application marker                   */
    M_APP3  = 0xe3,  /* application marker                   */
    M_APP4  = 0xe4,  /* application marker                   */
    M_APP5  = 0xe5,  /* application marker                   */
    M_APP6  = 0xe6,  /* application marker                   */
    M_APP7  = 0xe7,  /* application marker                   */
    M_APP8  = 0xe8,  /* application marker                   */
    M_APP9  = 0xe9,  /* application marker                   */
    M_APP10 = 0xea,  /* application marker                   */
    M_APP11 = 0xeb,  /* application marker                   */
    M_APP12 = 0xec,  /* application marker                   */
    M_APP13 = 0xed,  /* application marker                   */
    M_APP14 = 0xee,  /* application marker, used by Adobe    */
    M_APP15 = 0xef,  /* application marker                   */

    M_JPG0  = 0xf0,  /* reserved for JPEG extensions         */
    M_JPG13 = 0xfd,  /* reserved for JPEG extensions         */
    M_COM   = 0xfe,  /* comment                              */

    M_TEM   = 0x01,  /* temporary use                        */

    M_ERROR = 0x100  /* dummy marker, internal use only      */
} JPEG_MARKER;

static unsigned int read_exif_bytes(unsigned char **p, int n, int b)
{
    unsigned int rval = 0;
    unsigned char *pp = *p;
    if (b) {
        switch (n) {
            case 4:
                rval += *pp++; rval <<= 8;
                rval += *pp++; rval <<= 8;
            case 2:
                rval += *pp++; rval <<= 8;
                rval += *pp;
                break;
        }
    } else {
        pp += n;
        switch (n) {
            case 4:
                rval += *--pp; rval <<= 8;
                rval += *--pp; rval <<= 8;
            case 2:
                rval += *--pp; rval <<= 8;
                rval += *--pp;
                break;
        }
    }
    *p += n;
    return rval;
}

/*tex

    The Exif block can contain the data on the resolution in two forms:
    XResolution, YResolution and ResolutionUnit (tag 282, 283 and 296) as well as
    PixelPerUnitX, PixelPerUnitY and PixelUnit (tag 0x5111, 0x5112 and 0x5110).
    Tags 282, 293 and 296 have the priority, with ResolutionUnit set to inch by
    default, then tag 0x5110, 0x5111 and 0x5112, where the only valid value for
    PixelUnit is 0.0254, and finally the given value xx and yy, choosen if the
    Exif x and y resolution are not strictly positive.

    The next one doesn't save the data, just reads the tags we need based on info
    from \typ {http://www.exif.org/Exif2-2.PDF}.

*/

static void read_APP1_Exif (FILE *fp, unsigned short length, int *xx, int *yy, int *or)
{
    unsigned char *buffer = (unsigned char *)xmalloc(length);
    unsigned char *p, *rp;
    unsigned char *tiff_header;
    char bigendian;
    int i;
    int num_fields, tag, type;
    /*tex silence uninitialized warnings */
    int value = 0;
    unsigned int num = 0;
    unsigned int den = 0;
    boolean found_x = false;
    boolean found_y = false;
    int tempx = 0;
    int tempy = 0;
    double xres = 0;
    double yres = 0;
    double res_unit = 1.0;
    unsigned int xres_ms = 0;
    unsigned int yres_ms = 0;
    double res_unit_ms = 0;
    boolean found_x_ms = false;
    boolean found_y_ms = false;
    boolean found_res= false;
    int orientation = 1;
    size_t ret_len;
    ret_len = fread(buffer, length, 1, fp);
    if (ret_len != 1)
        goto err ;
    p = buffer;
    while ((p < buffer + length) && (*p == 0))
        ++p;
    tiff_header = p;
    if ((*p == 'M') && (*(p+1) == 'M'))
        bigendian = 1;
    else if ((*p == 'I') && (*(p+1) == 'I'))
        bigendian = 0;
    else
        goto err;
    p += 2;
    i = read_exif_bytes(&p, 2, bigendian);
    if (i != 42)
        goto err;
    i = read_exif_bytes(&p, 4, bigendian);
    p = tiff_header + i;
    num_fields = read_exif_bytes(&p, 2, bigendian);
    while (num_fields-- > 0) {
        tag = read_exif_bytes(&p, 2, bigendian);
        type = read_exif_bytes(&p, 2, bigendian);
        read_exif_bytes(&p, 4, bigendian);
        switch (type) {
            case 1:
                /*tex byte */
                value = *p++;
                p += 3;
                break;
            case 3:
                /*tex unsigned short */
            case 8:
                /*tex signed short */
                value = read_exif_bytes(&p, 2, bigendian);
                p += 2;
                break;
            case 4:
                /*tex unsigned long */
            case 9:
                /*tex signed long */
                value = read_exif_bytes(&p, 4, bigendian);
                break;
            case 5:
                /*tex rational */
            case 10:
                /*tex srational */
                value = read_exif_bytes(&p, 4, bigendian);
                rp = tiff_header + value;
                num = read_exif_bytes(&rp, 4, bigendian);
                den = read_exif_bytes(&rp, 4, bigendian);
                break;
            case 7:
                /*tex undefined */
                value = *p++;
                p += 3;
                break;
            case 2:
                /*tex ascii */
            default:
                p += 4;
                break;
        }
        switch (tag) {
            case 274:
                /*tex orientation */
                orientation = value;
                break;
            case 282:
                /*tex x res */
                if (den != 0) {
                    xres = num / den;
                    found_x = true;
                }
                break;
            case 283:
                /*tex y res */
                if (den != 0) {
                    yres = num / den;
                    found_y = true ;
                }
                break;
            case 296:
                /*tex res unit */
                switch (value) {
                    case 2:
                        res_unit = 1.0;
                        break;
                    case 3:
                        res_unit = 2.54;
                        break;
                    default:
                        res_unit = 0;
                        break;
                }
                break;
            case 0x5110:
                /*tex PixelUnit */
                switch (value) {
                    case 1:
                        res_unit_ms = 0.0254; /* Unit is meter */
                        break;
                    default:
                        res_unit_ms = 0;
                }
                break;
           case 0x5111:
               /*tex PixelPerUnitX */
                found_x_ms = true ;
                xres_ms = value;
                break;
           case 0x5112:
               /*tex PixelPerUnitY */
                found_y_ms = true ;
                yres_ms = value ;
                break;
        }
    }
    if (found_x && found_y && res_unit>0) {
        found_res = true;
        tempx = (int)(xres * res_unit+0.5);
        tempy = (int)(yres * res_unit+0.5);
    } else if (found_x_ms && found_y_ms && res_unit_ms==0.0254) {
        found_res = true;
        tempx = (int)(xres_ms * res_unit_ms+0.5);
        tempy = (int)(yres_ms * res_unit_ms+0.5);
    }
    if (found_res) {
        if (tempx>0 && tempy>0) {
            if ((tempx!=(*xx) || tempy!=(*yy)) && (*xx!=0 && (*yy!=0) )  ) {
                formatted_warning("readjpg","Exif resolution %ddpi x %ddpi differs from the input resolution %ddpi x %ddpi",tempx,tempy,*xx,*yy);
            }
            if (tempx==1 || tempy==1) {
                formatted_warning("readjpg","Exif resolution %ddpi x %ddpi looks weird", tempx, tempy);
            }
            *xx = tempx;
            *yy = tempy;
        } else {
            formatted_warning("readjpg","Bad Exif resolution %ddpi x %ddpi (zero or negative value of a signed integer)",tempx,tempy);
        }
    }
    *or = orientation;
err:
    free(buffer);
    return;
}

/*tex

    Contrary to \PDF\ where several parallel usage can happen (\PDF, |TEX, \LUA)
    with bitmaps we care less about keeping files open. So, we can keep files
    open in the img lib but then they are closed after inclusion anyway.

*/

static void close_and_cleanup_jpg(image_dict * idict)
{
    /*tex if one of then is not NULL we already cleaned up */
    if (img_file(idict) != NULL) {
        xfclose(img_file(idict), img_filepath(idict));
        img_file(idict) = NULL;
    }
    if (img_jpg_ptr(idict) != NULL) {
        xfree(img_jpg_ptr(idict));
    }
}

void flush_jpg_info(image_dict * idict)
{
    close_and_cleanup_jpg(idict);
}

/*tex

    The jpeg images are scanned for resolution, colorspace, depth, dimensions and
    orientation. We need to look at the exif blob for that. The original version
    did a quick test for jfif and exif but there can be more blobs later on. The
    current approach is to run over the linked list of blobs which is somewhat
    less efficient but not noticeable.

*/

void read_jpg_info(image_dict * idict)
{
    int i, position, units = 0;
    unsigned short length;
    int okay = 0 ;
    FILE *fp = img_file(idict);
    if (img_type(idict) != IMG_TYPE_JPG) {
        normal_error("readjpg","conflicting image dictionary");
    }
    if (fp != NULL) {
        normal_error("readjpg","image data already read");
    }
    fp = xfopen(img_filepath(idict), FOPEN_RBIN_MODE);
    img_totalpages(idict) = 1;
    img_pagenum(idict) = 1;
    img_xres(idict) = img_yres(idict) = 0;
    img_file(idict) = fp;
    if (fp == NULL) {
        normal_error("readjpg","unable to read image file");
    }
    img_jpg_ptr(idict) = xtalloc(1, jpg_img_struct);
    xfseek(fp, 0, SEEK_END, img_filepath(idict));
    img_jpg_ptr(idict)->length = xftell(fp, img_filepath(idict));
    xfseek(fp, 0, SEEK_SET, img_filepath(idict));
    if ((unsigned int) read2bytes(fp) != 0xFFD8) {
        normal_error("readjpg","no header found");
    }
    xfseek(fp, 0, SEEK_SET, img_filepath(idict));
    while (1) {
        if (feof(fp)) {
            if (okay) {
                break ;
            } else {
                normal_error("readjpg","premature file end");
            }
        } else if (fgetc(fp) != 0xFF) {
            if (okay) {
                break ;
            } else {
                normal_error("readjpg","no marker found");
            }
        }
        i = xgetc(fp);
        position = ftell(fp);
        length = 0 ;
        switch (i) {
            case M_SOF3:
                /*tex lossless */
            case M_SOF5:
            case M_SOF6:
            case M_SOF7:
                /*tex lossless */
            case M_SOF9:
            case M_SOF10:
            case M_SOF11:
                /*tex lossless */
            case M_SOF13:
            case M_SOF14:
            case M_SOF15:
                /*tex lossless */
                formatted_error("readjpg","unsupported compression SOF_%d", i - M_SOF0);
                break;
            case M_SOF2:
                if (img_pdfmajorversion(idict) < 2 && img_pdfminorversion(idict) <= 2) {
                    normal_error("readjpg","progressive DCT with PDF-1.2 is not permitted");
                }
            case M_SOF0:
            case M_SOF1:
                /*tex read segment length  */
                length = (int) read2bytes(fp);
                img_colordepth(idict) = xgetc(fp);
                img_ysize(idict) = (int) read2bytes(fp);
                img_xsize(idict) = (int) read2bytes(fp);
                img_jpg_color(idict) = xgetc(fp);
                switch (img_jpg_color(idict)) {
                    case JPG_GRAY:
                        img_procset(idict) |= PROCSET_IMAGE_B;
                        break;
                    case JPG_RGB:
                        img_procset(idict) |= PROCSET_IMAGE_C;
                        break;
                    case JPG_CMYK:
                        img_procset(idict) |= PROCSET_IMAGE_C;
                        break;
                    default:
                        formatted_error("readjpg","unsupported color space %i", (int) img_jpg_color(idict));
                }
                okay = 1 ;
                break ;
            case M_APP0:
                {
                    char app_sig[32];
                    length = (int) read2bytes(fp);
                    if (length > 6) {
                        if (fread(app_sig, sizeof(char), 5, fp) != 5)
                            return;
                        if (!memcmp(app_sig, "JFIF\000", 5)) {
                            /*tex skip two bytes, compiler is also happy*/
                            units = (int) read2bytes(fp);
                            units = xgetc(fp);
                            img_xres(idict) = (int) read2bytes(fp);
                            img_yres(idict) = (int) read2bytes(fp);
                            switch (units) {
                                case 1:
                                    /*tex pixels per inch */
                                    if ((img_xres(idict) == 1) || (img_yres(idict) == 1)) {
                                        formatted_warning("readjpg","unusual resolution of %ddpi by %ddpi", img_xres(idict), img_yres(idict));
                                    }
                                    break;
                                case 2:
                                    /*tex pixels per cm */
                                    img_xres(idict) = (int) ((double) img_xres(idict) * 2.54);
                                    img_yres(idict) = (int) ((double) img_yres(idict) * 2.54);
                                    break;
                                default:
                                    img_xres(idict) = img_yres(idict) = 0;
                                    break;
                                }
                            }
                        /*tex
                            If either xres or yres is 0 but the other isn't, set
                            it to the value of the other.
                        */
                    }
                }
                break;
            case M_APP1:
                {
                    char app_sig[32];
                    length = (int) read2bytes(fp);
                    if (length > 7) {
                        if (fread(app_sig, sizeof(char), 5, fp) != 5)
                            return;
                        if (!memcmp(app_sig, "Exif\000", 5)) {
                            int xxres = img_xres(idict);
                            int yyres = img_yres(idict);
                            int orientation = img_orientation(idict);
                            read_APP1_Exif(fp, length - 7, &xxres, &yyres, &orientation);
                            img_xres(idict) = xxres;
                            img_yres(idict) = yyres;
                            img_orientation(idict) = orientation;
                        }
                    }
                }
                break;
            /*tex ignore markers without parameters */
            case M_SOI:
            case M_EOI:
            case M_TEM:
            case M_RST0:
            case M_RST1:
            case M_RST2:
            case M_RST3:
            case M_RST4:
            case M_RST5:
            case M_RST6:
            case M_RST7:
                break;
            default:
                /*tex skip variable length markers */
                length = (int) read2bytes(fp);
                break;
        }
        if (length > 0) {
            xfseek(fp, position + length, SEEK_SET, img_filepath(idict));
        }
    }
    xfseek(fp, 0, SEEK_SET, img_filepath(idict));
    if (! img_keepopen(idict)) {
        close_and_cleanup_jpg(idict);
    }
    if (okay){
        if ((img_xres(idict) == 0) && (img_yres(idict) != 0)) {
            img_xres(idict) = img_yres(idict);
        }
        if ((img_yres(idict) == 0) && (img_xres(idict) != 0)) {
            img_yres(idict) = img_xres(idict);
        }
    } else {
        normal_error("readjpg","unknown fatal error");
    }
}

static void reopen_jpg(image_dict * idict)
{
    int width = img_xsize(idict);
    int height = img_ysize(idict);
    int xres = img_xres(idict);
    int yres = img_yres(idict);
    /*tex
        we need to make sure that the file kept open
    */
    img_keepopen(idict) = 1;
    read_jpg_info(idict);
    if (width != img_xsize(idict) || height != img_ysize(idict) || xres != img_xres(idict) || yres != img_yres(idict)) {
        normal_error("writejpg","image dimensions have changed");
    }
}

void write_jpg(PDF pdf, image_dict * idict)
{
    size_t l;
    if (img_file(idict) == NULL) {
        reopen_jpg(idict);
    }
    pdf_begin_obj(pdf, img_objnum(idict), OBJSTM_NEVER);
    pdf_begin_dict(pdf);
    pdf_dict_add_name(pdf, "Type", "XObject");
    pdf_dict_add_name(pdf, "Subtype", "Image");
    pdf_dict_add_img_filename(pdf, idict);
    if (img_attr(idict) != NULL && strlen(img_attr(idict)) > 0) {
        pdf_printf(pdf, "\n%s\n", img_attr(idict));
    }
    pdf_dict_add_int(pdf, "Width", (int) img_xsize(idict));
    pdf_dict_add_int(pdf, "Height", (int) img_ysize(idict));
    pdf_dict_add_int(pdf, "BitsPerComponent", (int) img_colordepth(idict));
    pdf_dict_add_int(pdf, "Length", (int) img_jpg_ptr(idict)->length);
    if (img_colorspace(idict) != 0) {
        pdf_dict_add_ref(pdf, "ColorSpace", (int) img_colorspace(idict));
    } else {
        switch (img_jpg_color(idict)) {
            case JPG_GRAY:
                pdf_dict_add_name(pdf, "ColorSpace", "DeviceGray");
                break;
            case JPG_RGB:
                pdf_dict_add_name(pdf, "ColorSpace", "DeviceRGB");
                break;
            case JPG_CMYK:
                pdf_dict_add_name(pdf, "ColorSpace", "DeviceCMYK");
                pdf_add_name(pdf, "Decode");
                pdf_begin_array(pdf);
                pdf_add_int(pdf, 1);
                pdf_add_int(pdf, 0);
                pdf_add_int(pdf, 1);
                pdf_add_int(pdf, 0);
                pdf_add_int(pdf, 1);
                pdf_add_int(pdf, 0);
                pdf_add_int(pdf, 1);
                pdf_add_int(pdf, 0);
                pdf_end_array(pdf);
                break;
            default:
                formatted_error("writejpg","unsupported JPEG color space %i", (int) img_jpg_color(idict));
        }
    }
    pdf_dict_add_name(pdf, "Filter", "DCTDecode");
    pdf_end_dict(pdf);
    pdf_begin_stream(pdf);
    l = (size_t) img_jpg_ptr(idict)->length;
    xfseek(img_file(idict), 0, SEEK_SET, img_filepath(idict));
    if (read_file_to_buf(pdf, img_file(idict), l) != l) {
        normal_error("writejpg","fread failed");
    }
    pdf_end_stream(pdf);
    pdf_end_obj(pdf);
    close_and_cleanup_jpg(idict);
}
