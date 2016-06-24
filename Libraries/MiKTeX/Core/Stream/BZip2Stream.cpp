/* BZip2Stream.cpp: bzip2 stream

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

#include "miktex/Core/BZip2Stream.h"

#include "CompressedStreamBase.h"
#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace std;

class BZip2StreamImpl :
  public CompressedStreamBase<BZip2Stream>
{
public:
  BZip2StreamImpl(const PathName & path, bool reading)
  {
    StartThread(path, reading);
  }

public:
  virtual ~BZip2StreamImpl()
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
  class bz_stream_wrapper : public bz_stream
  {
  public:
    bz_stream_wrapper()
    {
      memset(this, 0, sizeof(bz_stream));
      int ret = BZ2_bzDecompressInit(this, 0, 0);
      if (ret != BZ_OK)
      {
        MIKTEX_FATAL_ERROR_2("BZ2 decoder initialization did not succeed.", "ret", std::to_string(ret));
      }
    }
  public:
    ~bz_stream_wrapper()
    {
      BZ2_bzDecompressEnd(this);
    }
  };

protected:
  virtual void DoUncompress(const PathName & path)
  {
    const size_t BUFFER_SIZE = 1024 * 16;
    char inbuf[BUFFER_SIZE];
    char outbuf[BUFFER_SIZE];
    unique_ptr<FileStream> fileStream = make_unique<FileStream>(File::Open(path, FileMode::Open, FileAccess::Read, false));
    unique_ptr<bz_stream_wrapper> bzStream = make_unique<bz_stream_wrapper>();
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
        bzStream->avail_in = fileStream->Read(inbuf, BUFFER_SIZE);
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
};

unique_ptr<BZip2Stream> BZip2Stream::Create(const PathName & path, bool reading)
{
  return make_unique<BZip2StreamImpl>(path, reading);
}
