/**
 * @file Stream/GzipStream.cpp
 * @author Christian Schenk
 * @brief Gzip2 stream implementation
 *
 * @copyright Copyright © 1996-2026 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include "config.h"

#include <zlib.h>

#include <miktex/Core/FileStream>
#include <miktex/Core/GzipStream>

#include <miktex/Util/PathName>

#include "internal.h"

#include "CompressedStreamBase.h"
#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

class GzipStreamImpl :
    public CompressedStreamBase<GzipStream>
{

public:

    GzipStreamImpl(const PathName& path, bool reading)
    {
        StartThread(path, reading);
    }

    virtual ~GzipStreamImpl()
    {
        try
        {
            StopThread();
        }
        catch (const exception&)
        {
        }
    }

protected:

    virtual void DoCompress(const PathName& path) override
    {
        UNIMPLEMENTED();
    }

    virtual void DoUncompress(const PathName& path) override
    {
        const size_t BUFFER_SIZE = 1024 * 16;
        unsigned char inbuf[BUFFER_SIZE];
        unsigned char outbuf[BUFFER_SIZE];
        unique_ptr<FileStream> fileStream = make_unique<FileStream>(File::Open(path, FileMode::Open, FileAccess::Read, false));
        unique_ptr<gz_stream_wrapper> gzStream = make_unique<gz_stream_wrapper>();
        gzStream->next_in = nullptr;
        gzStream->avail_in = 0;
        gzStream->next_out = outbuf;
        gzStream->avail_out = BUFFER_SIZE;
        bool eof = false;
        while (true)
        {
            if (gzStream->avail_in == 0 && !eof)
            {
                gzStream->next_in = inbuf;
                gzStream->avail_in = static_cast<uInt>(fileStream->Read(inbuf, BUFFER_SIZE));
                eof = gzStream->avail_in == 0;
            }
            int ret = inflate(gzStream.get(), eof ? Z_FINISH : Z_NO_FLUSH);
            if (gzStream->avail_out == 0 || ret == Z_STREAM_END)
            {
                pipe.Write(outbuf, sizeof(outbuf) - gzStream->avail_out);
                gzStream->next_out = outbuf;
                gzStream->avail_out = BUFFER_SIZE;
            }
            if (ret != Z_OK)
            {
                if (ret == Z_STREAM_END)
                {
                    gzStream.reset();
                    fileStream->Close();
                    return;
                }
                MIKTEX_FATAL_ERROR_2("GZ decoder did not succeed.", "ret", std::to_string(ret));
            }
        }
    }

private:

    class gz_stream_wrapper : public z_stream
    {

    public:
        gz_stream_wrapper()
        {
            memset(this, 0, sizeof(z_stream));
            int ret = inflateInit2(this, 16 + MAX_WBITS);
            if (ret != Z_OK)
            {
                MIKTEX_FATAL_ERROR_2("GZ decoder initialization did not succeed.", "ret", std::to_string(ret));
            }
        }

        ~gz_stream_wrapper()
        {
            inflateEnd(this);
        }
    };

};

unique_ptr<GzipStream> GzipStream::Create(const PathName& path, bool reading)
{
    return make_unique<GzipStreamImpl>(path, reading);
}
