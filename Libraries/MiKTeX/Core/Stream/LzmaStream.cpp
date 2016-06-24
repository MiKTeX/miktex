/* LzmaStream.cpp: LZMA file stream

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/LzmaStream.h"

#include "CompressedStreamBase.h"
#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace std;

class LzmaStreamImpl :
  public CompressedStreamBase<LzmaStream>
{
public:
  LzmaStreamImpl(const PathName & path, bool reading)
  {
    StartThread(path, reading);
  }

public:
  virtual ~LzmaStreamImpl()
  {
    try
    {
      StopThread();
    }
    catch (const exception &)
    {
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
  public:
    ~lzma_stream_wrapper()
    {
      lzma_end(this);
    }
  };

protected:
  virtual void DoUncompress(const PathName & path)
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
};

unique_ptr<LzmaStream> LzmaStream::Create(const PathName & path, bool reading)
{
  return make_unique<LzmaStreamImpl>(path, reading);
}
