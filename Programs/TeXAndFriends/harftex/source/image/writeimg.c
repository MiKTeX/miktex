/*

writeimg.c

Copyright 1996-2006 Han The Thanh <thanh@pdftex.org>
Copyright 2006-2012 Taco Hoekwater <taco@luatex.org>

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
#include <kpathsea/c-auto.h>
#include <kpathsea/c-memstr.h>

#include "image/image.h"
#include "image/writejpg.h"
#include "image/writejp2.h"
#include "image/writepng.h"
#include "image/writejbig2.h"

#include "lua.h"
#include "lauxlib.h"

/*tex

The function |readimage| performs some basic initializations. Then it looks at
the file extension to determine the image type and calls specific code/functions.
The main disadvantage is that standard file extensions have to be used, otherwise
pdfTeX is not able to detect the correct image type. The patch now looks at the
file header first regardless of the file extension. This is implemented in
function |check_type_by_header|. If this check fails, the traditional test of
standard file extension is tried, done in function |check_type_by_extension|.

The magic headers are as follows:

\startitemize
    \startitem
        \type {png}: 89 50 4E 47 0D 0A 1A 0A or |"\137PNG\013\010\026\010"|
    \stopitem
    \startitem
        \type {jpg}: FF D8 FF or |"\255\216\255"|.
    \stopitem
    \startitem
        \type {jp2}: 00 00 00 0C 6A 50 20 20 0D 0A or |"\000\000\000\012\106\080\032\032\013\010"|
    \stopitem
    \startitem
        \type {pdf}: |"%PDF"| somewhere in the beginning
    \stopitem
\stopitemize

Functions |check_type_by_header| and |check_type_by_extension|: |img_type(img)|
is set to |IMG_TYPE_NONE| by |new_image_dict()|. Both functions try to detect a
type and set |img_type(img)|. Thus a value other than |IMG_TYPE_NONE| indicates
that a type has been found.

*/

#define HEADER_JPG   "\xFF\xD8"
#define HEADER_PNG   "\x89PNG\r\n\x1A\n"
#define HEADER_JBIG2 "\x97\x4A\x42\x32\x0D\x0A\x1A\x0A"
#define HEADER_JP2   "\x6A\x50\x20\x20"
#define HEADER_PDF   "%PDF-"

#define MAX_HEADER (sizeof(HEADER_PNG)-1)

#define HEADER_PDF_MEMSTREAM "data:application/pdf,"
#define LEN_PDF_MEMSTREAM 21

static void check_type_by_header(image_dict * idict)
{
    int i;
    FILE *file = NULL;
    char header[MAX_HEADER];
    char prefix[LEN_PDF_MEMSTREAM+1];
    if (idict == NULL)
        return;
    if (img_type(idict) != IMG_TYPE_NONE)
        return;
    /*tex Here we read the and also check for a memstream object. */
    if (!img_filepath(idict) || !FOPEN_RBIN_MODE) {
        normal_error("pdf backend","reading image file failed");
    }
    file = fopen(img_filepath(idict), FOPEN_RBIN_MODE);
    if (file == NULL) {
        /*tex We check the prefix of img_filepath(idict). */
        for (i = 0; (unsigned) i < LEN_PDF_MEMSTREAM; i++) {
            prefix[i] = (char) (img_filepath(idict)[i]);
        }
        prefix[LEN_PDF_MEMSTREAM]='\0';
        if (strncmp(prefix, HEADER_PDF_MEMSTREAM, LEN_PDF_MEMSTREAM) == 0) {
            img_type(idict) = IMG_TYPE_PDFMEMSTREAM;
            return;
        } else {
            formatted_error("pdf backend","reading image file '%s' failed",img_filepath(idict));
        }
    }
    /*tex Do we have a valid file but perhaps unsupported? */
    for (i = 0; (unsigned) i < MAX_HEADER; i++) {
        header[i] = (char) xgetc(file);
        if (feof(file)) {
            normal_error("pdf backend","reading image file failed");
        }
    }
    xfclose(file, img_filepath(idict));
    /*tex Further tests: */
    if (strncmp(header, HEADER_JPG, sizeof(HEADER_JPG) - 1) == 0)
        img_type(idict) = IMG_TYPE_JPG;
    else if (strncmp(header + 4, HEADER_JP2, sizeof(HEADER_JP2) - 1) == 0)
        img_type(idict) = IMG_TYPE_JP2;
    else if (strncmp(header, HEADER_PNG, sizeof(HEADER_PNG) - 1) == 0)
        img_type(idict) = IMG_TYPE_PNG;
    else if (strncmp(header, HEADER_JBIG2, sizeof(HEADER_JBIG2) - 1) == 0)
        img_type(idict) = IMG_TYPE_JBIG2;
    else if (strncmp(header, HEADER_PDF, sizeof(HEADER_PDF) - 1) == 0)
        img_type(idict) = IMG_TYPE_PDF;
}

static void check_type_by_extension(image_dict * idict)
{
    char *image_suffix;
    if (idict != NULL)
        return;
    if (img_type(idict) != IMG_TYPE_NONE)
        return;
    if ((image_suffix = strrchr(img_filename(idict), '.')) == 0)
        img_type(idict) = IMG_TYPE_NONE;
    else if (strcasecmp(image_suffix, ".png") == 0)
        img_type(idict) = IMG_TYPE_PNG;
    else if (strcasecmp(image_suffix, ".jpg") == 0 ||
             strcasecmp(image_suffix, ".jpeg") == 0)
        img_type(idict) = IMG_TYPE_JPG;
    else if (strcasecmp(image_suffix, ".jp2") == 0)
        img_type(idict) = IMG_TYPE_JP2;
    else if (strcasecmp(image_suffix, ".jbig2") == 0 ||
             strcasecmp(image_suffix, ".jb2") == 0)
        img_type(idict) = IMG_TYPE_JBIG2;
    else if (strcasecmp(image_suffix, ".pdf") == 0)
        img_type(idict) = IMG_TYPE_PDF;
}

void new_img_pdfstream_struct(image_dict * p)
{
    img_pdfstream_ptr(p) = xtalloc(1, pdf_stream_struct);
    img_pdfstream_stream(p) = NULL;
    img_pdfstream_size(p) = 0;
}

image *new_image(void)
{
    image *p = xtalloc(1, image);
    set_wd_running(p);
    set_ht_running(p);
    set_dp_running(p);
    img_transform(p) = 0;
    img_dict(p) = NULL;
    img_dictref(p) = LUA_NOREF;
    return p;
}

image_dict *new_image_dict(void)
{
    image_dict *p = xtalloc(1, image_dict);
    memset(p, 0, sizeof(image_dict));
    set_wd_running(p);
    set_ht_running(p);
    set_dp_running(p);
    img_transform(p) = 0;
    img_pagenum(p) = 1;
    img_type(p) = IMG_TYPE_NONE;
    img_pagebox(p) = PDF_BOX_SPEC_MEDIA;
    img_unset_bbox(p);
    img_unset_group(p);
    img_state(p) = DICT_NEW;
    /*tex A value of -1 means unused while the used counts from 0 */
    img_index(p) = -1;
    img_luaref(p) = 0;
    img_errorlevel(p) = pdf_inclusion_errorlevel;
    fix_pdf_version(static_pdf);
    img_pdfmajorversion(p) = pdf_major_version;
    img_pdfminorversion(p) = pdf_minor_version;
    return p;
}

static void free_dict_strings(image_dict * p)
{
    xfree(img_filename(p));
    xfree(img_filepath(p));
    xfree(img_attr(p));
    xfree(img_pagename(p));
}

void free_image_dict(image_dict * p)
{
    if (ini_version) {
         /*tex The image may be \.{\\dump}ed to a format. */
        return;
    }
    /*tex Called from limglib.c. */
    switch (img_type(p)) {
        case IMG_TYPE_PDFMEMSTREAM:
        case IMG_TYPE_PDF:
            flush_pdf_info(p);
            break;
        case IMG_TYPE_PNG:
            flush_png_info(p);
            break;
        case IMG_TYPE_JPG:
            flush_jpg_info(p);
            break;
        case IMG_TYPE_JP2:
            flush_jp2_info(p);
            break;
        case IMG_TYPE_JBIG2:
            flush_jbig2_info(p);
            break;
        case IMG_TYPE_PDFSTREAM:
            if (img_pdfstream_ptr(p) != NULL) {
                xfree(img_pdfstream_stream(p));
                xfree(img_pdfstream_ptr(p));
            }
            break;
        case IMG_TYPE_NONE:
            break;
        default:
            normal_error("pdf backend","unknown image type");
    }
    free_dict_strings(p);
    xfree(p);
}

void read_img(image_dict * idict)
{
    char *filepath = NULL;
    int callback_id;
    if (img_filename(idict) == NULL) {
        normal_error("pdf backend","image file name missing");
    }
    callback_id = callback_defined(find_image_file_callback);
    if (img_filepath(idict) == NULL) {
        if (callback_id > 0) {
            /*tex We always callback, also for a mem stream. */
            if (run_callback(callback_id, "S->S", img_filename(idict),&filepath)) {
                if (filepath && (strlen(filepath) > 0)) {
                    img_filepath(idict) = strdup(filepath);
                }
            }
        }
        if (img_filepath(idict) == NULL && (strstr(img_filename(idict),"data:application/pdf,") != NULL)) {
            /*tex We need to check here for a pdf memstream. */
            img_filepath(idict) = strdup(img_filename(idict));
        } else if (callback_id == 0) {
            /*tex Otherwise we use kpse but only when we don't callback. */
            img_filepath(idict) = kpse_find_file(img_filename(idict), kpse_tex_format, true);
        }
        if (img_filepath(idict) == NULL) {
            /*tex In any case we need a name. */
            formatted_error("pdf backend","cannot find image file '%s'", img_filename(idict));
        }
    }
    recorder_record_input(img_filepath(idict));
    /*tex A few type checks. */
    check_type_by_header(idict);
    check_type_by_extension(idict);
    /*tex Now we're ready to read the image. */
    switch (img_type(idict)) {
        case IMG_TYPE_PDFMEMSTREAM:
        case IMG_TYPE_PDF:
            read_pdf_info(idict);
            break;
        case IMG_TYPE_PNG:
            read_png_info(idict);
            break;
        case IMG_TYPE_JPG:
            read_jpg_info(idict);
            break;
        case IMG_TYPE_JP2:
            read_jp2_info(idict);
            break;
        case IMG_TYPE_JBIG2:
            read_jbig2_info(idict);
            break;
        default:
            img_type(idict) = IMG_TYPE_NONE;
            if (pdf_ignore_unknown_images) {
                normal_warning("pdf backend","internal error: ignoring unknown image type");
            } else {
                normal_error("pdf backend","internal error: unknown image type");
            }
            break;
    }
    cur_file_name = NULL;
    if (img_type(idict) == IMG_TYPE_NONE) {
        img_state(idict) = DICT_NEW;
    } else if (img_state(idict) < DICT_FILESCANNED) {
        img_state(idict) = DICT_FILESCANNED;
    }
}

static image_dict *read_image(char *file_name, int page_num, char *page_name, int colorspace, int page_box, char *user_password, char *owner_password, char *visible_filename)
{
    image *a = new_image();
    image_dict *idict = img_dict(a) = new_image_dict();
    static_pdf->ximage_count++;
    img_objnum(idict) = pdf_create_obj(static_pdf, obj_type_ximage, static_pdf->ximage_count);
    img_index(idict) = static_pdf->ximage_count;
    set_obj_data_ptr(static_pdf, img_objnum(idict), img_index(idict));
    idict_to_array(idict);
    img_colorspace(idict) = colorspace;
    img_pagenum(idict) = page_num;
    img_pagename(idict) = page_name;
    img_userpassword(idict) = user_password;
    img_ownerpassword(idict) = owner_password;
    img_visiblefilename(idict) = visible_filename;
    if (file_name == NULL) {
        normal_error("pdf backend","no image filename given");
    }
    cur_file_name = file_name;
    img_filename(idict) = file_name;
    img_pagebox(idict) = page_box;
    read_img(idict);
    return idict;
}

/*tex

    There can be several page boxes. Normally the cropbox is used.

*/

static pdfboxspec_e scan_pdf_box_spec(void)
{
    if (scan_keyword("mediabox"))
        return PDF_BOX_SPEC_MEDIA;
    else if (scan_keyword("cropbox"))
        return PDF_BOX_SPEC_CROP;
    else if (scan_keyword("bleedbox"))
        return PDF_BOX_SPEC_BLEED;
    else if (scan_keyword("trimbox"))
        return PDF_BOX_SPEC_TRIM;
    else if (scan_keyword("artbox"))
        return PDF_BOX_SPEC_ART;
    else
        return PDF_BOX_SPEC_NONE;
}

void scan_pdfximage(PDF pdf)
{
    scaled_whd alt_rule;
    image_dict *idict;
    int transform = 0, page = 1, pagebox, colorspace = 0;
    char *named = NULL, *attr = NULL, *file_name = NULL, *user = NULL, *owner = NULL, *visible = NULL;
    alt_rule = scan_alt_rule();
    if (scan_keyword("attr")) {
        scan_toks(false, true);
        attr = tokenlist_to_cstring(def_ref, true, NULL);
        delete_token_ref(def_ref);
    }
    if (scan_keyword("named")) {
        scan_toks(false, true);
        if (0) {
            named = tokenlist_to_cstring(def_ref, true, NULL);
            page = 0;
        } else {
            normal_warning("pdf backend","named pages are not supported, using page 1");
            page = 1;
        }
        delete_token_ref(def_ref);
    } else if (scan_keyword("page")) {
        scan_int();
        page = cur_val;
    }
    if (scan_keyword("userpassword")) {
        scan_toks(false, true);
        user = tokenlist_to_cstring(def_ref, true, NULL);
        delete_token_ref(def_ref);
    }
    if (scan_keyword("ownerpassword")) {
        scan_toks(false, true);
        owner = tokenlist_to_cstring(def_ref, true, NULL);
        delete_token_ref(def_ref);
    }
    if (scan_keyword("visiblefilename")) {
        scan_toks(false, true);
        visible = tokenlist_to_cstring(def_ref, true, NULL);
        delete_token_ref(def_ref);
    }
    if (scan_keyword("colorspace")) {
        scan_int();
        colorspace = cur_val;
    }
    pagebox = scan_pdf_box_spec();
    if (pagebox == PDF_BOX_SPEC_NONE) {
        pagebox = pdf_pagebox;
        if (pagebox == PDF_BOX_SPEC_NONE)
            pagebox = PDF_BOX_SPEC_CROP;
    }
    scan_toks(false, true);
    file_name = tokenlist_to_cstring(def_ref, true, NULL);
    if (file_name == NULL) {
        normal_error("pdf backend","no image filename given");
    }
    delete_token_ref(def_ref);
    idict = read_image(file_name, page, named, colorspace, pagebox, user, owner, visible);
    img_attr(idict) = attr;
    img_dimen(idict) = alt_rule;
    img_transform(idict) = transform;
    last_saved_image_index = img_objnum(idict);
    last_saved_image_pages = img_totalpages(idict);
}

void scan_pdfrefximage(PDF pdf)
{
    /*tex One could scan transform as well. */
    int transform = 0;
    /*tex Begin of experiment. */
    int open = 0;
    /*tex End of experiment. */
    image_dict *idict;
    /*tex This scans |<rule spec>| to |alt_rule|. */
    scaled_whd alt_rule, dim;
    alt_rule = scan_alt_rule();
    /*tex Begin of experiment. */
    if (scan_keyword("keepopen")) {
        open = 1;
    }
    /*tex End of experiment. */
    scan_int();
    check_obj_type(pdf, obj_type_ximage, cur_val);
    tail_append(new_rule(image_rule));
    idict = idict_array[obj_data_ptr(pdf, cur_val)];
    /*tex Begin of experiment, */
    if (open) {
        /*tex So we keep the original value when no close is given. */
        idict->keepopen = 1;
    }
    /*tex End of experiment. */
    if (img_state(idict) == DICT_NEW) {
        normal_warning("image","don't rely on the image data to be okay");
        width(tail_par) = 0;
        height(tail_par) = 0;
        depth(tail_par) = 0;
    } else {
        if (alt_rule.wd != null_flag || alt_rule.ht != null_flag || alt_rule.dp != null_flag) {
            dim = scale_img(idict, alt_rule, transform);
        } else {
            dim = scale_img(idict, img_dimen(idict), img_transform(idict));
        }
        width(tail_par) = dim.wd;
        height(tail_par) = dim.ht;
        depth(tail_par) = dim.dp;
        rule_transform(tail_par) = transform;
        rule_index(tail_par) = img_index(idict);
    }
}

/*
    The |tex_scale| function follows a sequence of decisions:

    \starttyping
    wd ht dp : res = tex;
    wd ht --
    wd -- dp
    wd -- --
    -- ht dp
    -- ht --
    -- -- dp
    -- -- -- : res = nat;
    \stoptyping

*/

scaled_whd tex_scale(scaled_whd nat, scaled_whd tex)
{
    scaled_whd res;
    if (!is_running(tex.wd) && !is_running(tex.ht) && !is_running(tex.dp)) {
        /*tex width, height, and depth specified */
        res = tex;
    } else if (!is_running(tex.wd)) {
        /*tex max. 2 dimensions are specified */
        res.wd = tex.wd;
        if (!is_running(tex.ht)) {
            res.ht = tex.ht;
            /*tex width and height specified */
            res.dp = ext_xn_over_d(tex.ht, nat.dp, nat.ht);
        } else if (!is_running(tex.dp)) {
            res.dp = tex.dp;
            /*tex width and depth specified */
            res.ht = ext_xn_over_d(tex.wd, nat.ht + nat.dp, nat.wd) - tex.dp;
        } else {
            /*tex only width specified */
            res.ht = ext_xn_over_d(tex.wd, nat.ht, nat.wd);
            res.dp = ext_xn_over_d(tex.wd, nat.dp, nat.wd);
        }
    } else if (!is_running(tex.ht)) {
        res.ht = tex.ht;
        if (!is_running(tex.dp)) {
            res.dp = tex.dp;
            /*tex height and depth specified */
            res.wd = ext_xn_over_d(tex.ht + tex.dp, nat.wd, nat.ht + nat.dp);
        } else {
            /*tex only height specified */
            res.wd = ext_xn_over_d(tex.ht, nat.wd, nat.ht);
            res.dp = ext_xn_over_d(tex.ht, nat.dp, nat.ht);
        }
    } else if (!is_running(tex.dp)) {
        res.dp = tex.dp;
        /*tex only depth specified */
        res.ht = nat.ht - (tex.dp - nat.dp);
        res.wd = nat.wd;
    } else {
        /*tex nothing specified */
        res = nat;
    }
    return res;
}

/*tex

Within |scale_img| only image width and height matter; the offsets and
positioning are not interesting here. But one needs rotation info to swap width
and height. |img_rotation| comes from the optional |/Rotate| key in the PDF file.

*/

scaled_whd scale_img(image_dict * idict, scaled_whd alt_rule, int transform)
{
    /*tex size and resolution of image */
    int x, y, xr, yr, tmp;
    /*tex natural size corresponding to image resolution */
    scaled_whd nat;
    int default_res;
    nat.dp = 0;
    nat.wd = 0;
    nat.ht = 0;
    if (img_nobbox(idict)) {
        if (img_is_bbox(idict)) {
            x = img_xsize(idict) = img_bbox(idict)[2] - img_bbox(idict)[0];
            y = img_ysize(idict) = img_bbox(idict)[3] - img_bbox(idict)[1];
            img_xorig(idict) = img_bbox(idict)[0];
            img_yorig(idict) = img_bbox(idict)[1];
            nat.wd = x;
            nat.ht = y;
        } else {
            normal_error("pdf backend","use boundingbox to pass dimensions");
        }
    } else {
        if ((img_type(idict) == IMG_TYPE_PDF || img_type(idict) == IMG_TYPE_PDFMEMSTREAM
             || img_type(idict) == IMG_TYPE_PDFSTREAM) && img_is_bbox(idict)) {
            /*tex dimensions from image.bbox */
            x = img_xsize(idict) = img_bbox(idict)[2] - img_bbox(idict)[0];
            y = img_ysize(idict) = img_bbox(idict)[3] - img_bbox(idict)[1];
            img_xorig(idict) = img_bbox(idict)[0];
            img_yorig(idict) = img_bbox(idict)[1];
        } else {
            /*tex dimensions, resolutions from image file */
            x = img_xsize(idict);
            y = img_ysize(idict);
        }
        xr = img_xres(idict);
        yr = img_yres(idict);
        if (x <= 0 || y <= 0 || xr < 0 || yr < 0)
            normal_error("pdf backend","invalid image dimensions");
        if (xr > 65535 || yr > 65535) {
            xr = 0;
            yr = 0;
            normal_warning("pdf backend","too large image resolution ignored");
        }
        if (((transform - img_rotation(idict)) & 1) == 1) {
            tmp = x;
            x = y;
            y = tmp;
            tmp = xr;
            xr = yr;
            yr = tmp;
        }
        /*tex always for images */
        if (img_type(idict) == IMG_TYPE_PDF || img_type(idict) == IMG_TYPE_PDFMEMSTREAM
            || img_type(idict) == IMG_TYPE_PDFSTREAM) {
            nat.wd = x;
            nat.ht = y;
        } else {
            default_res = fix_int(pdf_image_resolution, 0, 65535);
            if (default_res > 0 && (xr == 0 || yr == 0)) {
                xr = default_res;
                yr = default_res;
            }
            if (xr > 0 && yr > 0) {
                nat.wd = ext_xn_over_d(one_hundred_inch, x, 100 * xr);
                nat.ht = ext_xn_over_d(one_hundred_inch, y, 100 * yr);
            } else {
                nat.wd = ext_xn_over_d(one_hundred_inch, x, 7200);
                nat.ht = ext_xn_over_d(one_hundred_inch, y, 7200);
            }
        }
    }
    return tex_scale(nat, alt_rule);
}

void write_img(PDF pdf, image_dict * idict)
{
    if (img_state(idict) < DICT_WRITTEN) {
        report_start_file(filetype_image, img_filepath(idict));
        switch (img_type(idict)) {
        case IMG_TYPE_PNG:
            write_png(pdf, idict);
            break;
        case IMG_TYPE_JPG:
            write_jpg(pdf, idict);
            break;
        case IMG_TYPE_JP2:
            write_jp2(pdf, idict);
            break;
        case IMG_TYPE_JBIG2:
            write_jbig2(pdf, idict);
            break;
        case IMG_TYPE_PDFMEMSTREAM:
        case IMG_TYPE_PDF:
            write_epdf(pdf, idict,(int) pdf_suppress_optional_info);
            break;
        case IMG_TYPE_PDFSTREAM:
            write_pdfstream(pdf, idict);
            break;
        default:
            normal_error("pdf backend","internal error: writing unknown image type");
        }
        report_stop_file(filetype_image);
        if (img_type(idict) == IMG_TYPE_PNG) {
            write_additional_png_objects(pdf);
        }
    }
    if (img_state(idict) < DICT_WRITTEN)
        img_state(idict) = DICT_WRITTEN;
}

int write_img_object(PDF pdf, image_dict * idict, int n)
{
    return write_epdf_object(pdf, idict, n);
}

void pdf_write_image(PDF pdf, int n)
{
    if (pdf->draftmode == 0)
        write_img(pdf, idict_array[obj_data_ptr(pdf, n)]);
}

void check_pdfstream_dict(image_dict * idict)
{
    if (!img_is_bbox(idict) && !img_nobbox(idict)) {
        normal_error("pdf backend","image.stream: no bbox given");
    }
    if (img_state(idict) < DICT_FILESCANNED) {
        img_state(idict) = DICT_FILESCANNED;
    }
}

void write_pdfstream(PDF pdf, image_dict * idict)
{
    pdf_begin_obj(pdf, img_objnum(idict), OBJSTM_NEVER);
    pdf_begin_dict(pdf);
    if (!img_notype(idict)) {
        pdf_dict_add_name(pdf, "Type", "XObject");
        pdf_dict_add_name(pdf, "Subtype", "Form");
        pdf_dict_add_int(pdf, "FormType", 1);
    }
    if (!img_nobbox(idict)) {
        pdf_add_name(pdf, "BBox");
        pdf_begin_array(pdf);
        pdf_add_real(pdf, sp2bp(img_bbox(idict)[0]));
        pdf_add_real(pdf, sp2bp(img_bbox(idict)[1]));
        pdf_add_real(pdf, sp2bp(img_bbox(idict)[2]));
        pdf_add_real(pdf, sp2bp(img_bbox(idict)[3]));
        pdf_end_array(pdf);
    }
    if (img_attr(idict) != NULL && strlen(img_attr(idict)) > 0) {
        pdf_printf(pdf, "\n%s\n", img_attr(idict));
    }
    if (!img_nolength(idict)) {
        pdf_dict_add_streaminfo(pdf);
    }
    pdf_end_dict(pdf);
    pdf_begin_stream(pdf);
    if (img_pdfstream_stream(idict) != NULL) {
        pdf_out_block(pdf, (const char *) img_pdfstream_stream(idict), img_pdfstream_size(idict));
    }
    pdf_end_stream(pdf);
    pdf_end_obj(pdf);
}

idict_entry *idict_ptr, *idict_array = NULL;
size_t idict_limit;

void idict_to_array(image_dict * idict)
{
    if (idict_ptr - idict_array == 0) {
        /*tex align to count from 1 */
        alloc_array(idict, 1, SMALL_BUF_SIZE);
        idict_ptr++;
    }
    alloc_array(idict, 1, SMALL_BUF_SIZE);
    *idict_ptr = idict;
    idict_ptr++;
}

void pdf_dict_add_img_filename(PDF pdf, image_dict * idict)
{
    char *p;
    if ((pdf_image_addfilename > 0) && ((pdf_suppress_optional_info & 2) == 0)) {
        /*tex
            For now |PTEX.FileName| is only used for \PDF, but we're prepared
            for \JPG, \PNG, ...
        */
        if (! ( (img_type(idict) == IMG_TYPE_PDF) || (img_type(idict) == IMG_TYPE_PDFMEMSTREAM) ))
            return;
        if (img_visiblefilename(idict) != NULL) {
            if (strlen(img_visiblefilename(idict)) == 0) {
                /*tex empty string blocks PTEX.FileName output */
                return;
            } else {
                p = img_visiblefilename(idict);
            }
        } else {
            /*tex unset so let's use the default */
            p = img_filepath(idict);
        }
        /*tex write additional information */
        pdf_add_name(pdf, "PTEX.FileName");
        pdf_printf(pdf, " (%s)", convertStringToPDFString(p, strlen(p)));
    }
}

/*tex

To allow the use of box resources inside saved boxes in -ini mode, the
information in the array has to be (un)dumped with the format. The next two
routines take care of that.

Most of the work involved in setting up the images is simply executed again. This
solves the many possible errors resulting from the split in two separate runs.

There was only one problem remaining: The |pdfversion| and
|pdfinclusionerrorlevel| can have changed inbetween the call to |readimage| and
dump time.

Some of the dumped values are really type int, not integer,but since the macro
falls back to |generic_dump| anyway, that does not matter.

We might drop this feature as it makes no sense to store images in the format.

*/

#define dumpinteger generic_dump
#define undumpinteger generic_undump

/*tex

(Un)dumping a string means dumping the allocation size, followed by the bytes.
The trailing \.{\\0} is dumped as well, because that makes the code simpler. The
rule specification ends up in |alt_rule|.

*/

scaled_whd scan_alt_rule(void)
{
    boolean loop;
    scaled_whd alt_rule;
    alt_rule.wd = null_flag;
    alt_rule.ht = null_flag;
    alt_rule.dp = null_flag;
    do {
        loop = false;
        if (scan_keyword("width")) {
            scan_normal_dimen();
            alt_rule.wd = cur_val;
            loop = true;
        } else if (scan_keyword("height")) {
            scan_normal_dimen();
            alt_rule.ht = cur_val;
            loop = true;
        } else if (scan_keyword("depth")) {
            scan_normal_dimen();
            alt_rule.dp = cur_val;
            loop = true;
        }
    } while (loop);
    return alt_rule;
}

/*tex

    This copy a file of arbitrary size to the buffer and flushed as needed.

*/

size_t read_file_to_buf(PDF pdf, FILE * f, size_t len)
{
    size_t i, j, k = 0;
    while (len > 0) {
        i = (size_t) (len > pdf->buf->size) ? (size_t) pdf->buf->size : len;
        pdf_room(pdf, (int) i);
        j = fread(pdf->buf->p, 1, i, f);
        pdf->buf->p += j;
        k += j;
        len -= j;
        if (i != j)
            break;
    }
    return k;
}
