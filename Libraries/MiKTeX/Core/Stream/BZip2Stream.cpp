/**
 * @file Stream/BZip2Stream.cpp
 * @author Christian Schenk
 * @brief BZip2 stream implementation
 *
 * @copyright Copyright © 1996-2026 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include "config.h"

#include <bzlib.h>

#include <miktex/Core/BZip2Stream>
#include <miktex/Core/FileStream>

#include <miktex/Util/PathName>

#include "internal.h"

#include "CompressedStreamBase.h"
#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

class BZip2StreamImpl :
    public CompressedStreamBase<BZip2Stream>
{

public:

    BZip2StreamImpl(const PathName& path, bool reading)
    {
        StartThread(path, reading);
    }

    virtual ~BZip2StreamImpl()
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
        const size_t IN_BUFFER_SIZE = 1024 * 16;
        const size_t OUT_BUFFER_SIZE = 1024 * 8;
        char inbuf[IN_BUFFER_SIZE];
        char outbuf[OUT_BUFFER_SIZE];
        unique_ptr<FileStream> fileStream = make_unique<FileStream>(File::Open(path, FileMode::Create, FileAccess::Write, false));
        unique_ptr<bz_stream_wrapper> bzStream = make_unique<bz_stream_wrapper>(false);
        bzStream->next_in = inbuf;
        bzStream->avail_in = 0;
        bzStream->next_out = outbuf;
        bzStream->avail_out = OUT_BUFFER_SIZE;
        auto action = BZ_RUN;
        while (true)
        {
            if (action == BZ_RUN && bzStream->avail_in == 0)
            {
                bzStream->avail_in = static_cast<unsigned int>(pipe.Read(inbuf, IN_BUFFER_SIZE));
                bzStream->next_in = inbuf;
                if (bzStream->avail_in == 0)
                {
                    action = BZ_FINISH;
                }
            }
            int ret = BZ2_bzCompress(bzStream.get(), action);
            if (ret != BZ_RUN_OK && ret != BZ_FINISH_OK && ret != BZ_STREAM_END)
            {
                MIKTEX_FATAL_ERROR_2("BZ2 encoder did not succeed.", "ret", std::to_string(ret));
            }
            if (OUT_BUFFER_SIZE - bzStream->avail_out > 0)
            {
                fileStream->Write(outbuf, OUT_BUFFER_SIZE - bzStream->avail_out);
                bzStream->next_out = outbuf;
                bzStream->avail_out = OUT_BUFFER_SIZE;
            }
            if (ret == BZ_STREAM_END)
            {
                bzStream.reset();
                fileStream->Close();
                return;
            }
        }
    }

    virtual void DoUncompress(const PathName& path) override
    {
        const size_t BUFFER_SIZE = 1024 * 16;
        char inbuf[BUFFER_SIZE];
        char outbuf[BUFFER_SIZE];
        unique_ptr<FileStream> fileStream = make_unique<FileStream>(File::Open(path, FileMode::Open, FileAccess::Read, false));
        unique_ptr<bz_stream_wrapper> bzStream = make_unique<bz_stream_wrapper>(true);
        bzStream->next_in = nullptr;
        bzStream->avail_in = 0;
        bzStream->next_out = outbuf;
        bzStream->avail_out = BUFFER_SIZE;
        bool eof = false;
        while (true)
        {
            if (bzStream->avail_in == 0 && !eof)
            {
                bzStream->next_in = inbuf;
                bzStream->avail_in = static_cast<unsigned int>(fileStream->Read(inbuf, BUFFER_SIZE));
                eof = bzStream->avail_in == 0;
            }
            int ret = BZ2_bzDecompress(bzStream.get());
            if (bzStream->avail_out == 0 || ret == BZ_STREAM_END)
            {
                pipe.Write(outbuf, sizeof(outbuf) - bzStream->avail_out);
                bzStream->next_out = outbuf;
                bzStream->avail_out = BUFFER_SIZE;
            }
            if (ret != BZ_OK)
            {
                if (ret == BZ_STREAM_END)
                {
                    bzStream.reset();
                    fileStream->Close();
                    return;
                }
                MIKTEX_FATAL_ERROR_2("BZ2 decoder did not succeed.", "ret", std::to_string(ret));
            }
        }
    }

private:

    class bz_stream_wrapper : public bz_stream
    {

    public:

        bz_stream_wrapper(bool reading) :
            reading(reading)
        {
            memset(this, 0, sizeof(bz_stream));
            if (reading)
            {
                int ret = BZ2_bzDecompressInit(this, 0, 0);
                if (ret != BZ_OK)
                {
                    MIKTEX_FATAL_ERROR_2("BZ2 decoder initialization did not succeed.", "ret", std::to_string(ret));
                }
            }
            else
            {
                int ret = BZ2_bzCompressInit(this, 1, 0, 0);
                if (ret != BZ_OK)
                {
                    MIKTEX_FATAL_ERROR_2("BZ2 encoder initialization did not succeed.", "ret", std::to_string(ret));
                }
            }
        }

        ~bz_stream_wrapper()
        {
            if (reading)
            {
                BZ2_bzDecompressEnd(this);
            }
            else    
            {
                BZ2_bzCompressEnd(this);
            }
        }

    private:

        bool reading;
    };

};

unique_ptr<BZip2Stream> BZip2Stream::Create(const PathName& path, bool reading)
{
    return make_unique<BZip2StreamImpl>(path, reading);
}
