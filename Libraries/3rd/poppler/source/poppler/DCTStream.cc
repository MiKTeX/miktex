//========================================================================
//
// DCTStream.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2005 Jeff Muizelaar <jeff@infidigm.net>
// Copyright 2005-2010, 2012, 2017, 2020-2023 Albert Astals Cid <aacid@kde.org>
// Copyright 2009 Ryszard Trojnacki <rysiek@menel.com>
// Copyright 2010 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright 2011 Daiki Ueno <ueno@unixuser.org>
// Copyright 2011 Tomas Hoger <thoger@redhat.com>
// Copyright 2012, 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright 2020 Lluís Batlle i Rossell <viric@viric.name>
//
//========================================================================

#include "DCTStream.h"

static void str_init_source(j_decompress_ptr cinfo) { }

static boolean str_fill_input_buffer(j_decompress_ptr cinfo)
{
    int c;
    struct str_src_mgr *src = (struct str_src_mgr *)cinfo->src;
    if (src->index == 0) {
        c = 0xFF;
        src->index++;
    } else if (src->index == 1) {
        c = 0xD8;
        src->index++;
    } else {
        c = src->str->getChar();
    }
    src->buffer = c;
    src->pub.next_input_byte = &src->buffer;
    src->pub.bytes_in_buffer = 1;
    if (c != EOF) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static void str_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    struct str_src_mgr *src = (struct str_src_mgr *)cinfo->src;
    if (num_bytes > 0) {
        while (num_bytes > (long)src->pub.bytes_in_buffer) {
            num_bytes -= (long)src->pub.bytes_in_buffer;
            str_fill_input_buffer(cinfo);
        }
        src->pub.next_input_byte += (size_t)num_bytes;
        src->pub.bytes_in_buffer -= (size_t)num_bytes;
    }
}

static void str_term_source(j_decompress_ptr cinfo) { }

DCTStream::DCTStream(Stream *strA, int colorXformA, Dict *dict, int recursion) : FilterStream(strA)
{
    colorXform = colorXformA;
    if (dict != nullptr) {
        Object obj = dict->lookup("Width", recursion);
        err.width = (obj.isInt() && obj.getInt() <= JPEG_MAX_DIMENSION) ? obj.getInt() : 0;
        obj = dict->lookup("Height", recursion);
        err.height = (obj.isInt() && obj.getInt() <= JPEG_MAX_DIMENSION) ? obj.getInt() : 0;
    } else {
        err.height = err.width = 0;
    }
    init();
}

DCTStream::~DCTStream()
{
    jpeg_destroy_decompress(&cinfo);
    delete str;
}

static void exitErrorHandler(jpeg_common_struct *error)
{
    j_decompress_ptr cinfo = (j_decompress_ptr)error;
    str_error_mgr *err = (struct str_error_mgr *)cinfo->err;
    if (cinfo->err->msg_code == JERR_IMAGE_TOO_BIG && err->width != 0 && err->height != 0) {
        cinfo->image_height = err->height;
        cinfo->image_width = err->width;
    } else {
        longjmp(err->setjmp_buffer, 1);
    }
}

void DCTStream::init()
{
    jpeg_std_error(&err.pub);
    err.pub.error_exit = &exitErrorHandler;
    src.pub.init_source = str_init_source;
    src.pub.fill_input_buffer = str_fill_input_buffer;
    src.pub.skip_input_data = str_skip_input_data;
    src.pub.resync_to_restart = jpeg_resync_to_restart;
    src.pub.term_source = str_term_source;
    src.pub.bytes_in_buffer = 0;
    src.pub.next_input_byte = nullptr;
    src.str = str;
    src.index = 0;
    current = nullptr;
    limit = nullptr;

    cinfo.err = &err.pub;
    if (!setjmp(err.setjmp_buffer)) {
        jpeg_create_decompress(&cinfo);
        cinfo.src = (jpeg_source_mgr *)&src;
    }
    row_buffer = nullptr;
}

void DCTStream::reset()
{
    int row_stride;

    str->reset();

    if (row_buffer) {
        jpeg_destroy_decompress(&cinfo);
        init();
    }

    // JPEG data has to start with 0xFF 0xD8
    // but some pdf like the one on
    // https://bugs.freedesktop.org/show_bug.cgi?id=3299
    // does have some garbage before that this seeks for
    // the start marker...
    bool startFound = false;
    int c = 0, c2 = 0;
    while (!startFound) {
        if (!c) {
            c = str->getChar();
            if (c == -1) {
                error(errSyntaxError, -1, "Could not find start of jpeg data");
                return;
            }
            if (c != 0xFF) {
                c = 0;
            }
        } else {
            c2 = str->getChar();
            if (c2 != 0xD8) {
                c = 0;
                c2 = 0;
            } else {
                startFound = true;
            }
        }
    }

    if (!setjmp(err.setjmp_buffer)) {
        if (jpeg_read_header(&cinfo, TRUE) != JPEG_SUSPENDED) {
            // figure out color transform
            if (colorXform == -1 && !cinfo.saw_Adobe_marker) {
                if (cinfo.num_components == 3) {
                    if (cinfo.saw_JFIF_marker) {
                        colorXform = 1;
                    } else if (cinfo.cur_comp_info[0]->component_id == 82 && cinfo.cur_comp_info[1]->component_id == 71 && cinfo.cur_comp_info[2]->component_id == 66) { // ASCII "RGB"
                        colorXform = 0;
                    } else {
                        colorXform = 1;
                    }
                } else {
                    colorXform = 0;
                }
            } else if (cinfo.saw_Adobe_marker) {
                colorXform = cinfo.Adobe_transform;
            }

            switch (cinfo.num_components) {
            case 3:
                cinfo.jpeg_color_space = colorXform ? JCS_YCbCr : JCS_RGB;
                break;
            case 4:
                cinfo.jpeg_color_space = colorXform ? JCS_YCCK : JCS_CMYK;
                break;
            }

            jpeg_start_decompress(&cinfo);

            row_stride = cinfo.output_width * cinfo.output_components;
            row_buffer = cinfo.mem->alloc_sarray((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
        }
    }
}

bool DCTStream::readLine()
{
    if (cinfo.output_scanline < cinfo.output_height) {
        if (!setjmp(err.setjmp_buffer)) {
            if (!jpeg_read_scanlines(&cinfo, row_buffer, 1)) {
                return false;
            } else {
                current = &row_buffer[0][0];
                limit = &row_buffer[0][(cinfo.output_width - 1) * cinfo.output_components] + cinfo.output_components;
                return true;
            }
        } else {
            return false;
        }
    } else {
        return false;
    }
}

int DCTStream::getChar()
{
    if (current == limit) {
        if (!readLine()) {
            return EOF;
        }
    }

    return *current++;
}

int DCTStream::getChars(int nChars, unsigned char *buffer)
{
    for (int i = 0; i < nChars;) {
        if (current == limit) {
            if (!readLine()) {
                return i;
            }
        }
        intptr_t left = limit - current;
        if (left + i > nChars) {
            left = nChars - i;
        }
        memcpy(buffer + i, current, left);
        current += left;
        i += static_cast<int>(left);
    }
    return nChars;
}

int DCTStream::lookChar()
{
    if (unlikely(current == nullptr)) {
        return EOF;
    }
    return *current;
}

GooString *DCTStream::getPSFilter(int psLevel, const char *indent)
{
    GooString *s;

    if (psLevel < 2) {
        return nullptr;
    }
    if (!(s = str->getPSFilter(psLevel, indent))) {
        return nullptr;
    }
    s->append(indent)->append("<< >> /DCTDecode filter\n");
    return s;
}

bool DCTStream::isBinary(bool last) const
{
    return str->isBinary(true);
}
