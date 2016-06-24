/* GzipStream.cpp: gzip stream

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

#include "miktex/Core/GzipStream.h"

#include "CompressedStreamBase.h"
#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace std;

class GzipStreamImpl :
  public CompressedStreamBase<GzipStream>
{
public:
  GzipStreamImpl(const PathName & path, bool reading)
  {
    StartThread(path, reading);
  }

public:
  virtual ~GzipStreamImpl()
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
  public:
    ~gz_stream_wrapper()
    {
      inflateEnd(this);
    }
  };

protected:
  void DoUncompress(const PathName & path)
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
        gzStream->avail_in = fileStream->Read(inbuf, BUFFER_SIZE);
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
};

unique_ptr<GzipStream> GzipStream::Create(const PathName & path, bool reading)
{
  return make_unique<GzipStreamImpl>(path, reading);
}
