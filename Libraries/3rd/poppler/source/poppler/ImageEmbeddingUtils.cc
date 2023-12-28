//========================================================================
//
// ImageEmbeddingUtils.cc
//
// Copyright (C) 2021 Georgiy Sgibnev <georgiy@sgibnev.com>. Work sponsored by lab50.net.
// Copyright (C) 2021, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2021 Marco Genasci <fedeliallalinea@gmail.com>
// Copyright (C) 2023 Jordan Abrahams-Whitehead <ajordanr@google.com>
//
// This file is licensed under the GPLv2 or later
//
//========================================================================

#include <config.h>

#include <memory>
#ifdef ENABLE_LIBJPEG
#    include <cstdio>
extern "C" {
#    include <jpeglib.h>
}
#    include <csetjmp>
#endif
#ifdef ENABLE_LIBPNG
#    include <png.h>
#endif

#include "ImageEmbeddingUtils.h"
#include "goo/gmem.h"
#include "goo/GooCheckedOps.h"
#include "Object.h"
#include "Array.h"
#include "Error.h"
#include "PDFDoc.h"

namespace ImageEmbeddingUtils {

static const uint8_t PNG_MAGIC_NUM[] = { 0x89, 0x50, 0x4e, 0x47 };
static const uint8_t JPEG_MAGIC_NUM[] = { 0xff, 0xd8, 0xff };
static const uint8_t JPEG2000_MAGIC_NUM[] = { 0x00, 0x00, 0x00, 0x0c, 0x6a, 0x50, 0x20, 0x20 };
static const Goffset MAX_MAGIC_NUM_SIZE = sizeof(JPEG2000_MAGIC_NUM);

static bool checkMagicNum(const uint8_t *fileContent, const uint8_t *magicNum, const uint8_t size)
{
    return (memcmp(fileContent, magicNum, size) == 0);
}

// Transforms an image to XObject.
class ImageEmbedder
{
protected:
    static constexpr const char *DEVICE_GRAY = "DeviceGray";
    static constexpr const char *DEVICE_RGB = "DeviceRGB";

    int m_width;
    int m_height;

    ImageEmbedder(const int width, const int height) : m_width(width), m_height(height) { }

    // Creates an object of type XObject. You own the returned ptr.
    static Dict *createImageDict(XRef *xref, const char *colorSpace, const int width, const int height, const int bitsPerComponent)
    {
        Dict *imageDict = new Dict(xref);
        imageDict->add("Type", Object(objName, "XObject"));
        imageDict->add("Subtype", Object(objName, "Image"));
        imageDict->add("ColorSpace", Object(objName, colorSpace));
        imageDict->add("Width", Object(width));
        imageDict->add("Height", Object(height));
        imageDict->add("BitsPerComponent", Object(bitsPerComponent));
        return imageDict;
    }

public:
    ImageEmbedder() = delete;
    ImageEmbedder(const ImageEmbedder &) = delete;
    ImageEmbedder &operator=(const ImageEmbedder &) = delete;
    virtual ~ImageEmbedder();

    // Call it only once.
    // Returns ref to a new object or Ref::INVALID.
    virtual Ref embedImage(XRef *xref) = 0;
};

ImageEmbedder::~ImageEmbedder() { }

#ifdef ENABLE_LIBPNG
// Transforms a PNG image to XObject.
class PngEmbedder : public ImageEmbedder
{
    // LibpngInputStream is a simple replacement for GInputStream.
    // Used with png_set_read_fn().
    class LibpngInputStream
    {
        std::unique_ptr<uint8_t[]> m_fileContent;
        uint8_t *m_iterator;
        png_size_t m_remainingSize;

        void read(png_bytep out, const png_size_t size)
        {
            const png_size_t fixedSize = (m_remainingSize >= size) ? size : m_remainingSize;
            memcpy(out, m_iterator, fixedSize);
            m_iterator += fixedSize;
            m_remainingSize -= fixedSize;
        }

    public:
        LibpngInputStream(std::unique_ptr<uint8_t[]> &&fileContent, const Goffset size) : m_fileContent(std::move(fileContent)), m_iterator(m_fileContent.get()), m_remainingSize(size) { }
        LibpngInputStream() = delete;
        LibpngInputStream(const LibpngInputStream &) = delete;
        LibpngInputStream &operator=(const LibpngInputStream &) = delete;
        ~LibpngInputStream() = default;

        // Pass this static function to png_set_read_fn().
        static void readCallback(png_structp png, png_bytep out, png_size_t size)
        {
            LibpngInputStream *stream = (LibpngInputStream *)png_get_io_ptr(png);
            if (stream) {
                stream->read(out, size);
            }
        }
    };

    png_structp m_png;
    png_infop m_info;
    LibpngInputStream *m_stream;
    const png_byte m_type;
    const bool m_hasAlpha;
    // Number of color channels.
    const png_byte m_n;
    // Number of color channels excluding alpha channel. Should be 1 or 3.
    const png_byte m_nWithoutAlpha;
    // Shold be 8 or 16.
    const png_byte m_bitDepth;
    // Should be 1 or 2.
    const png_byte m_byteDepth;

    PngEmbedder(png_structp png, png_infop info, LibpngInputStream *stream)
        : ImageEmbedder(png_get_image_width(png, info), png_get_image_height(png, info)),
          m_png(png),
          m_info(info),
          m_stream(stream),
          m_type(png_get_color_type(m_png, m_info)),
          m_hasAlpha(m_type & PNG_COLOR_MASK_ALPHA),
          m_n(png_get_channels(m_png, m_info)),
          m_nWithoutAlpha(m_hasAlpha ? m_n - 1 : m_n),
          m_bitDepth(png_get_bit_depth(m_png, m_info)),
          m_byteDepth(m_bitDepth / 8)
    {
    }

    // Reads pixels into mainBuffer (RGB/gray channels) and maskBuffer (alpha channel).
    void readPixels(png_bytep mainBuffer, png_bytep maskBuffer)
    {
        // Read pixels from m_png.
        const int rowSize = png_get_rowbytes(m_png, m_info);
        png_bytepp pixels = new png_bytep[m_height];
        for (int y = 0; y < m_height; y++) {
            pixels[y] = new png_byte[rowSize];
        }
        png_read_image(m_png, pixels);

        // Copy pixels into mainBuffer and maskBuffer.
        const png_byte pixelSizeWithoutAlpha = m_nWithoutAlpha * m_byteDepth;
        for (int y = 0; y < m_height; y++) {
            png_bytep row = pixels[y];
            for (int x = 0; x < m_width; x++) {
                memcpy(mainBuffer, row, pixelSizeWithoutAlpha);
                mainBuffer += pixelSizeWithoutAlpha;
                row += pixelSizeWithoutAlpha;
                if (m_hasAlpha) {
                    memcpy(maskBuffer, row, m_byteDepth);
                    maskBuffer += m_byteDepth;
                    row += m_byteDepth;
                }
            }
        }

        // Cleanup.
        for (int y = 0; y < m_height; y++) {
            delete[] pixels[y];
        }
        delete[] pixels;
    }

    // Supportive function for create().
    // We don't want to deal with palette images.
    // We don't want to deal with 1/2/4-bit samples.
    static void fixPng(png_structp png, png_infop info)
    {
        const png_byte colorType = png_get_color_type(png, info);
        const png_byte bitDepth = png_get_bit_depth(png, info);

        bool updateRequired = false;
        if (colorType == PNG_COLOR_TYPE_PALETTE) {
            png_set_palette_to_rgb(png);
            updateRequired = true;
        }
        if ((colorType == PNG_COLOR_TYPE_GRAY) && (bitDepth < 8)) {
            png_set_expand_gray_1_2_4_to_8(png);
            updateRequired = true;
        }
        if (png_get_valid(png, info, PNG_INFO_tRNS)) {
            png_set_tRNS_to_alpha(png);
            updateRequired = true;
        }
        if (bitDepth < 8) {
            png_set_packing(png);
            updateRequired = true;
        }
        if (updateRequired) {
            png_read_update_info(png, info);
        }
    }

public:
    PngEmbedder() = delete;
    PngEmbedder(const PngEmbedder &) = delete;
    PngEmbedder &operator=(const PngEmbedder &) = delete;
    ~PngEmbedder() override
    {
        png_destroy_read_struct(&m_png, &m_info, nullptr);
        delete m_stream;
    }

    Ref embedImage(XRef *xref) override;

    static std::unique_ptr<ImageEmbedder> create(std::unique_ptr<uint8_t[]> &&fileContent, const Goffset fileSize)
    {
        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (png == nullptr) {
            error(errInternal, -1, "Couldn't load PNG. png_create_read_struct() failed");
            return nullptr;
        }
        png_infop info = png_create_info_struct(png);
        if (info == nullptr) {
            error(errInternal, -1, "Couldn't load PNG. png_create_info_struct() failed");
            png_destroy_read_struct(&png, nullptr, nullptr);
            return nullptr;
        }
        if (setjmp(png_jmpbuf(png))) {
            error(errInternal, -1, "Couldn't load PNG. Failed to set up error handling for reading PNG");
            png_destroy_read_struct(&png, &info, nullptr);
            return nullptr;
        }

        LibpngInputStream *stream = new LibpngInputStream(std::move(fileContent), fileSize);
        png_set_read_fn(png, stream, LibpngInputStream::readCallback);
        png_read_info(png, info);
        fixPng(png, info);
        const png_byte bitDepth = png_get_bit_depth(png, info);
        if ((bitDepth != 8) && (bitDepth != 16)) {
            error(errInternal, -1, "Couldn't load PNG. Fixing bit depth failed");
            png_destroy_read_struct(&png, &info, nullptr);
            delete stream;
            return nullptr;
        }
        return std::unique_ptr<ImageEmbedder>(new PngEmbedder(png, info, stream));
    }
};

Ref PngEmbedder::embedImage(XRef *xref)
{
    // Read pixels.
    Goffset area;
    if (checkedMultiply(static_cast<Goffset>(m_width), static_cast<Goffset>(m_height), &area)) {
        error(errIO, -1, "PngEmbedder::embedImage: width * height overflows Goffset");
        return Ref::INVALID();
    }
    Goffset maskBufferSize;
    static_assert(sizeof(Goffset) >= sizeof(m_byteDepth));
    if (checkedMultiply(area, static_cast<Goffset>(m_byteDepth), &maskBufferSize)) {
        error(errIO, -1, "PngEmbedder::embedImage: width * height * m_byteDepth overflows Goffset");
        return Ref::INVALID();
    }
    Goffset mainBufferSize;
    static_assert(sizeof(Goffset) >= sizeof(m_nWithoutAlpha));
    if (checkedMultiply(maskBufferSize, static_cast<Goffset>(m_nWithoutAlpha), &mainBufferSize)) {
        error(errIO, -1, "PngEmbedder::embedImage: width * height * m_byteDepth * m_nWithoutAlpha overflows Goffset");
        return Ref::INVALID();
    }
    png_bytep mainBuffer = (png_bytep)gmalloc(mainBufferSize);
    png_bytep maskBuffer = (m_hasAlpha) ? (png_bytep)gmalloc(maskBufferSize) : nullptr;
    readPixels(mainBuffer, maskBuffer);

    // Create a mask XObject and a main XObject.
    const char *colorSpace = ((m_type == PNG_COLOR_TYPE_GRAY) || (m_type == PNG_COLOR_TYPE_GRAY_ALPHA)) ? DEVICE_GRAY : DEVICE_RGB;
    Dict *baseImageDict = createImageDict(xref, colorSpace, m_width, m_height, m_bitDepth);
    if (m_hasAlpha) {
        Dict *maskImageDict = createImageDict(xref, DEVICE_GRAY, m_width, m_height, m_bitDepth);
        Ref maskImageRef = xref->addStreamObject(maskImageDict, maskBuffer, maskBufferSize);
        baseImageDict->add("SMask", Object(maskImageRef));
    }
    return xref->addStreamObject(baseImageDict, mainBuffer, mainBufferSize);
}
#endif

#ifdef ENABLE_LIBJPEG

struct JpegErrorManager
{
    jpeg_error_mgr pub;
    jmp_buf setjmpBuffer;
};

// Note: an address of pub is equal to an address of a JpegErrorManager instance.
static void jpegExitErrorHandler(j_common_ptr info)
{
    JpegErrorManager *errorManager = (JpegErrorManager *)info->err;
    (*errorManager->pub.output_message)(info);
    // Jump to the setjmp point.
    longjmp(errorManager->setjmpBuffer, 1);
}

// Transforms a JPEG image to XObject.
class JpegEmbedder : public ImageEmbedder
{
    std::unique_ptr<uint8_t[]> m_fileContent;
    Goffset m_fileSize;

    JpegEmbedder(const int width, const int height, std::unique_ptr<uint8_t[]> &&fileContent, const Goffset fileSize) : ImageEmbedder(width, height), m_fileContent(std::move(fileContent)), m_fileSize(fileSize) { }

public:
    JpegEmbedder() = delete;
    JpegEmbedder(const JpegEmbedder &) = delete;
    JpegEmbedder &operator=(const JpegEmbedder &) = delete;
    ~JpegEmbedder() override = default;

    Ref embedImage(XRef *xref) override;

    static std::unique_ptr<ImageEmbedder> create(std::unique_ptr<uint8_t[]> &&fileContent, const Goffset fileSize)
    {
        jpeg_decompress_struct info;
        JpegErrorManager errorManager;
        info.err = jpeg_std_error(&errorManager.pub);
        errorManager.pub.error_exit = jpegExitErrorHandler;
        if (setjmp(errorManager.setjmpBuffer)) {
            // The setjmp point.
            jpeg_destroy_decompress(&info);
            error(errInternal, -1, "libjpeg failed to process the file");
            return nullptr;
        }

        jpeg_create_decompress(&info);
        // fileSize is guaranteed to be in the range 0..int max by the checks in embed()
        // jpeg_mem_src takes an unsigned long in the 3rd parameter
        jpeg_mem_src(&info, fileContent.get(), static_cast<unsigned long>(fileSize));
        jpeg_read_header(&info, TRUE);
        jpeg_start_decompress(&info);
        auto result = std::unique_ptr<ImageEmbedder>(new JpegEmbedder(info.output_width, info.output_height, std::move(fileContent), fileSize));
        jpeg_abort_decompress(&info);
        jpeg_destroy_decompress(&info);
        return result;
    }
};

Ref JpegEmbedder::embedImage(XRef *xref)
{
    if (m_fileContent == nullptr) {
        return Ref::INVALID();
    }
    Dict *baseImageDict = createImageDict(xref, DEVICE_RGB, m_width, m_height, 8);
    baseImageDict->add("Filter", Object(objName, "DCTDecode"));
    Ref baseImageRef = xref->addStreamObject(baseImageDict, m_fileContent.release(), m_fileSize);
    return baseImageRef;
}
#endif

Ref embed(XRef *xref, const GooFile &imageFile)
{
    // Load the image file.
    const Goffset fileSize = imageFile.size();
    if (fileSize < 0) {
        error(errIO, -1, "Image file size could not be calculated");
        return Ref::INVALID();
    }
    // GooFile::read only takes an integer so for now we don't support huge images
    if (fileSize > std::numeric_limits<int>::max()) {
        error(errIO, -1, "file size too big");
        return Ref::INVALID();
    }
    std::unique_ptr<uint8_t[]> fileContent = std::make_unique<uint8_t[]>(fileSize);
    const int bytesRead = imageFile.read((char *)fileContent.get(), static_cast<int>(fileSize), 0);
    if ((bytesRead != fileSize) || (fileSize < MAX_MAGIC_NUM_SIZE)) {
        error(errIO, -1, "Couldn't load the image file");
        return Ref::INVALID();
    }

    std::unique_ptr<ImageEmbedder> embedder;
    if (checkMagicNum(fileContent.get(), PNG_MAGIC_NUM, sizeof(PNG_MAGIC_NUM))) {
#ifdef ENABLE_LIBPNG
        embedder = PngEmbedder::create(std::move(fileContent), fileSize);
#else
        error(errUnimplemented, -1, "PNG format is not supported");
#endif
    } else if (checkMagicNum(fileContent.get(), JPEG_MAGIC_NUM, sizeof(JPEG_MAGIC_NUM))) {
#ifdef ENABLE_LIBJPEG
        embedder = JpegEmbedder::create(std::move(fileContent), fileSize);
#else
        error(errUnimplemented, -1, "JPEG format is not supported");
#endif
    } else if (checkMagicNum(fileContent.get(), JPEG2000_MAGIC_NUM, sizeof(JPEG2000_MAGIC_NUM))) {
        // TODO: implement JPEG2000 support using libopenjpeg2.
        error(errUnimplemented, -1, "JPEG2000 format is not supported");
        return Ref::INVALID();
    } else {
        error(errUnimplemented, -1, "Image format is not supported");
        return Ref::INVALID();
    }

    if (!embedder) {
        return Ref::INVALID();
    }
    return embedder->embedImage(xref);
}

Ref embed(XRef *xref, const std::string &imagePath)
{
    std::unique_ptr<GooFile> imageFile(GooFile::open(imagePath));
    if (!imageFile) {
        error(errIO, -1, "Couldn't open {0:s}", imagePath.c_str());
        return Ref::INVALID();
    }
    return embed(xref, *imageFile);
}

}
