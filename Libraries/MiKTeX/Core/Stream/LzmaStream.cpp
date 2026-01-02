/**
 * @file Stream/LzmaStream.cpp
 * @author Christian Schenk
 * @brief LZMA stream implementation
 *
 * @copyright Copyright © 1996-2026 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include "config.h"

#include <lzma.h>

#include <miktex/Core/FileStream>
#include <miktex/Core/LzmaStream>

#include <miktex/Util/PathName>

#include "internal.h"

#include "CompressedStreamBase.h"
#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

class LzmaStreamImpl :
    public CompressedStreamBase<LzmaStream>
{

public:

    LzmaStreamImpl(const PathName& path, bool reading)
    {
        StartThread(path, reading);
    }

    virtual ~LzmaStreamImpl()
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

    virtual void DoUncompress(const PathName& path)
    {
        const size_t BUFFER_SIZE = 1024 * 16;
        uint8_t inbuf[BUFFER_SIZE];
        uint8_t outbuf[BUFFER_SIZE];
        unique_ptr<FileStream> fileStream = make_unique<FileStream>(File::Open(path, FileMode::Open, FileAccess::Read, false));
        unique_ptr<lzma_stream_wrapper> lzmaStream = make_unique<lzma_stream_wrapper>();
        lzmaStream->next_in = nullptr;
        lzmaStream->avail_in = 0;
        lzmaStream->next_out = outbuf;
        lzmaStream->avail_out = BUFFER_SIZE;
        bool eof = false;
        while (true)
        {
            if (lzmaStream->avail_in == 0 && !eof)
            {
                lzmaStream->next_in = inbuf;
                lzmaStream->avail_in = fileStream->Read(inbuf, BUFFER_SIZE);
                eof = lzmaStream->avail_in == 0;
            }
            lzma_ret ret = lzma_code(lzmaStream.get(), eof ? LZMA_FINISH : LZMA_RUN);
            if (lzmaStream->avail_out == 0 || ret == LZMA_STREAM_END)
            {
                pipe.Write(outbuf, sizeof(outbuf) - lzmaStream->avail_out);
                lzmaStream->next_out = outbuf;
                lzmaStream->avail_out = BUFFER_SIZE;
            }
            if (ret != LZMA_OK)
            {
                if (ret == LZMA_STREAM_END)
                {
                    lzmaStream.reset();
                    fileStream->Close();
                    return;
                }
                MIKTEX_FATAL_ERROR_2("LZMA decoder did not succeed.", "ret", std::to_string(ret));
            }
        }
    }

private:

    class lzma_stream_wrapper : public lzma_stream
    {

    public:

        lzma_stream_wrapper() :
            lzma_stream(LZMA_STREAM_INIT)
        {
            lzma_ret ret = lzma_auto_decoder(this, UINT64_MAX, 0);
            if (ret != LZMA_OK)
            {
                MIKTEX_FATAL_ERROR_2("LZMA decoder initialization did not succeed.", "ret", std::to_string(ret));
            }
        }

        ~lzma_stream_wrapper()
        {
            lzma_end(this);
        }
    };
};

unique_ptr<LzmaStream> LzmaStream::Create(const PathName& path, bool reading)
{
    return make_unique<LzmaStreamImpl>(path, reading);
}
