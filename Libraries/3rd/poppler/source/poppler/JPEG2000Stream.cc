//========================================================================
//
// JPEG2000Stream.cc
//
// A JPX stream decoder using OpenJPEG
//
// Copyright 2008-2010, 2012, 2017-2023 Albert Astals Cid <aacid@kde.org>
// Copyright 2011 Daniel Glöckner <daniel-gl@gmx.net>
// Copyright 2014, 2016 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright 2013, 2014 Adrian Johnson <ajohnson@redneon.com>
// Copyright 2015 Adam Reichold <adam.reichold@t-online.de>
// Copyright 2015 Jakub Wilk <jwilk@jwilk.net>
// Copyright 2022 Oliver Sander <oliver.sander@tu-dresden.de>
//
// Licensed under GPLv2 or later
//
//========================================================================

#include "config.h"
#include "JPEG2000Stream.h"
#include <openjpeg.h>

struct JPXStreamPrivate
{
    opj_image_t *image = nullptr;
    int counter = 0;
    int ccounter = 0;
    int npixels = 0;
    int ncomps = 0;
    bool inited = false;
    void init2(OPJ_CODEC_FORMAT format, const unsigned char *buf, int length, bool indexed);
};

static inline unsigned char adjustComp(int r, int adjust, int depth, int sgndcorr, bool indexed)
{
    if (!indexed) {
        r += sgndcorr;
        if (adjust) {
            r = (r >> adjust) + ((r >> (adjust - 1)) % 2);
        } else if (depth < 8) {
            r = r << (8 - depth);
        }
    }
    if (unlikely(r > 255)) {
        r = 255;
    }
    return r;
}

static inline int doLookChar(JPXStreamPrivate *priv)
{
    if (unlikely(priv->counter >= priv->npixels)) {
        return EOF;
    }

    return ((unsigned char *)priv->image->comps[priv->ccounter].data)[priv->counter];
}

static inline int doGetChar(JPXStreamPrivate *priv)
{
    const int result = doLookChar(priv);
    if (++priv->ccounter == priv->ncomps) {
        priv->ccounter = 0;
        ++priv->counter;
    }
    return result;
}

JPXStream::JPXStream(Stream *strA) : FilterStream(strA)
{
    priv = new JPXStreamPrivate;
}

JPXStream::~JPXStream()
{
    delete str;
    close();
    delete priv;
}

void JPXStream::reset()
{
    priv->counter = 0;
    priv->ccounter = 0;
}

void JPXStream::close()
{
    if (priv->image != nullptr) {
        opj_image_destroy(priv->image);
        priv->image = nullptr;
        priv->npixels = 0;
    }
}

Goffset JPXStream::getPos()
{
    return priv->counter * priv->ncomps + priv->ccounter;
}

int JPXStream::getChars(int nChars, unsigned char *buffer)
{
    if (unlikely(priv->inited == false)) {
        init();
    }

    for (int i = 0; i < nChars; ++i) {
        const int c = doGetChar(priv);
        if (likely(c != EOF)) {
            buffer[i] = c;
        } else {
            return i;
        }
    }
    return nChars;
}

int JPXStream::getChar()
{
    if (unlikely(priv->inited == false)) {
        init();
    }

    return doGetChar(priv);
}

int JPXStream::lookChar()
{
    if (unlikely(priv->inited == false)) {
        init();
    }

    return doLookChar(priv);
}

GooString *JPXStream::getPSFilter(int psLevel, const char *indent)
{
    return nullptr;
}

bool JPXStream::isBinary(bool last) const
{
    return str->isBinary(true);
}

void JPXStream::getImageParams(int *bitsPerComponent, StreamColorSpaceMode *csMode)
{
    if (unlikely(priv->inited == false)) {
        init();
    }

    *bitsPerComponent = 8;
    int numComps = (priv->image) ? priv->image->numcomps : 1;
    if (priv->image) {
        if (priv->image->color_space == OPJ_CLRSPC_SRGB && numComps == 4) {
            numComps = 3;
        } else if (priv->image->color_space == OPJ_CLRSPC_SYCC && numComps == 4) {
            numComps = 3;
        } else if (numComps == 2) {
            numComps = 1;
        } else if (numComps > 4) {
            numComps = 4;
        }
    }
    if (numComps == 3) {
        *csMode = streamCSDeviceRGB;
    } else if (numComps == 4) {
        *csMode = streamCSDeviceCMYK;
    } else {
        *csMode = streamCSDeviceGray;
    }
}

static void libopenjpeg_error_callback(const char *msg, void * /*client_data*/)
{
    error(errSyntaxError, -1, "{0:s}", msg);
}

static void libopenjpeg_warning_callback(const char *msg, void * /*client_data*/)
{
    error(errSyntaxWarning, -1, "{0:s}", msg);
}

typedef struct JPXData_s
{
    const unsigned char *data;
    int size;
    OPJ_OFF_T pos;
} JPXData;

#define BUFFER_INITIAL_SIZE 4096

static OPJ_SIZE_T jpxRead_callback(void *p_buffer, OPJ_SIZE_T p_nb_bytes, void *p_user_data)
{
    JPXData *jpxData = (JPXData *)p_user_data;

    if (unlikely(jpxData->size <= jpxData->pos)) {
        return (OPJ_SIZE_T)-1; /* End of file! */
    }
    OPJ_SIZE_T len = jpxData->size - jpxData->pos;
    if (len > p_nb_bytes) {
        len = p_nb_bytes;
    }
    memcpy(p_buffer, jpxData->data + jpxData->pos, len);
    jpxData->pos += len;
    return len;
}

static OPJ_OFF_T jpxSkip_callback(OPJ_OFF_T skip, void *p_user_data)
{
    JPXData *jpxData = (JPXData *)p_user_data;

    jpxData->pos += (skip > jpxData->size - jpxData->pos) ? jpxData->size - jpxData->pos : skip;
    /* Always return input value to avoid "Problem with skipping JPEG2000 box, stream error" */
    return skip;
}

static OPJ_BOOL jpxSeek_callback(OPJ_OFF_T seek_pos, void *p_user_data)
{
    JPXData *jpxData = (JPXData *)p_user_data;

    if (seek_pos > jpxData->size) {
        return OPJ_FALSE;
    }
    jpxData->pos = seek_pos;
    return OPJ_TRUE;
}

void JPXStream::init()
{
    Object oLen, cspace, smaskInDataObj;
    if (getDict()) {
        oLen = getDict()->lookup("Length");
        cspace = getDict()->lookup("ColorSpace");
        smaskInDataObj = getDict()->lookup("SMaskInData");
    }

    int bufSize = BUFFER_INITIAL_SIZE;
    if (oLen.isInt() && oLen.getInt() > 0) {
        bufSize = oLen.getInt();
    }

    bool indexed = false;
    if (cspace.isArray() && cspace.arrayGetLength() > 0) {
        const Object cstype = cspace.arrayGet(0);
        if (cstype.isName("Indexed")) {
            indexed = true;
        }
    }

    const int smaskInData = smaskInDataObj.isInt() ? smaskInDataObj.getInt() : 0;
    const std::vector<unsigned char> buf = str->toUnsignedChars(bufSize);
    priv->init2(OPJ_CODEC_JP2, buf.data(), buf.size(), indexed);

    if (priv->image) {
        int numComps = priv->image->numcomps;
        int alpha = 0;
        if (priv->image->color_space == OPJ_CLRSPC_SRGB && numComps == 4) {
            numComps = 3;
            alpha = 1;
        } else if (priv->image->color_space == OPJ_CLRSPC_SYCC && numComps == 4) {
            numComps = 3;
            alpha = 1;
        } else if (numComps == 2) {
            numComps = 1;
            alpha = 1;
        } else if (numComps > 4) {
            numComps = 4;
            alpha = 1;
        } else {
            alpha = 0;
        }
        priv->npixels = priv->image->comps[0].w * priv->image->comps[0].h;
        priv->ncomps = priv->image->numcomps;
        if (alpha == 1 && smaskInData == 0) {
            priv->ncomps--;
        }
        for (int component = 0; component < priv->ncomps; component++) {
            if (priv->image->comps[component].data == nullptr) {
                close();
                break;
            }
            const int componentPixels = priv->image->comps[component].w * priv->image->comps[component].h;
            if (componentPixels != priv->npixels) {
                error(errSyntaxWarning, -1, "Component {0:d} has different WxH than component 0", component);
                close();
                break;
            }
            unsigned char *cdata = (unsigned char *)priv->image->comps[component].data;
            int adjust = 0;
            int depth = priv->image->comps[component].prec;
            if (priv->image->comps[component].prec > 8) {
                adjust = priv->image->comps[component].prec - 8;
            }
            int sgndcorr = 0;
            if (priv->image->comps[component].sgnd) {
                sgndcorr = 1 << (priv->image->comps[0].prec - 1);
            }
            for (int i = 0; i < priv->npixels; i++) {
                int r = priv->image->comps[component].data[i];
                *(cdata++) = adjustComp(r, adjust, depth, sgndcorr, indexed);
            }
        }
    } else {
        priv->npixels = 0;
    }

    priv->counter = 0;
    priv->ccounter = 0;
    priv->inited = true;
}

void JPXStreamPrivate::init2(OPJ_CODEC_FORMAT format, const unsigned char *buf, int length, bool indexed)
{
    JPXData jpxData;

    jpxData.data = buf;
    jpxData.pos = 0;
    jpxData.size = length;

    opj_stream_t *stream;

    stream = opj_stream_default_create(OPJ_TRUE);

    opj_stream_set_user_data(stream, &jpxData, nullptr);

    opj_stream_set_read_function(stream, jpxRead_callback);
    opj_stream_set_skip_function(stream, jpxSkip_callback);
    opj_stream_set_seek_function(stream, jpxSeek_callback);
    /* Set the length to avoid an assert */
    opj_stream_set_user_data_length(stream, length);

    opj_codec_t *decoder;

    /* Use default decompression parameters */
    opj_dparameters_t parameters;
    opj_set_default_decoder_parameters(&parameters);
    if (indexed) {
        parameters.flags |= OPJ_DPARAMETERS_IGNORE_PCLR_CMAP_CDEF_FLAG;
    }

    /* Get the decoder handle of the format */
    decoder = opj_create_decompress(format);
    if (decoder == nullptr) {
        error(errSyntaxWarning, -1, "Unable to create decoder");
        goto error;
    }

    /* Catch events using our callbacks */
    opj_set_warning_handler(decoder, libopenjpeg_warning_callback, nullptr);
    opj_set_error_handler(decoder, libopenjpeg_error_callback, nullptr);

    /* Setup the decoder decoding parameters */
    if (!opj_setup_decoder(decoder, &parameters)) {
        error(errSyntaxWarning, -1, "Unable to set decoder parameters");
        goto error;
    }

    /* Decode the stream and fill the image structure */
    image = nullptr;
    if (!opj_read_header(stream, decoder, &image)) {
        error(errSyntaxWarning, -1, "Unable to read header");
        goto error;
    }

    /* Optional if you want decode the entire image */
    if (!opj_set_decode_area(decoder, image, parameters.DA_x0, parameters.DA_y0, parameters.DA_x1, parameters.DA_y1)) {
        error(errSyntaxWarning, -1, "X2");
        goto error;
    }

    /* Get the decoded image */
    if (!(opj_decode(decoder, stream, image) && opj_end_decompress(decoder, stream))) {
        error(errSyntaxWarning, -1, "Unable to decode image");
        goto error;
    }

    opj_destroy_codec(decoder);
    opj_stream_destroy(stream);

    if (image != nullptr) {
        return;
    }

error:
    if (image != nullptr) {
        opj_image_destroy(image);
        image = nullptr;
    }
    opj_stream_destroy(stream);
    opj_destroy_codec(decoder);
    if (format == OPJ_CODEC_JP2) {
        error(errSyntaxWarning, -1, "Did no succeed opening JPX Stream as JP2, trying as J2K.");
        init2(OPJ_CODEC_J2K, buf, length, indexed);
    } else if (format == OPJ_CODEC_J2K) {
        error(errSyntaxWarning, -1, "Did no succeed opening JPX Stream as J2K, trying as JPT.");
        init2(OPJ_CODEC_JPT, buf, length, indexed);
    } else {
        error(errSyntaxError, -1, "Did no succeed opening JPX Stream.");
    }
}
