/* CompressedStreamBase.h:

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

#include "Utils/Pipe.h"

BEGIN_INTERNAL_NAMESPACE;

template<typename Interface> class CompressedStreamBase :
  public Interface
{
public:
  size_t Read(void * pBytes, size_t count) override
  {
    if (IsUnsuccessful())
    {
      throw threadMiKTeXException;
    }
    return pipe.Read(pBytes, count);
  }

public:
  void Write(const void * pBytes, size_t count) override
  {
    UNIMPLEMENTED();
  }

public:
  void Seek(long offset, MiKTeX::Core::SeekOrigin seekOrigin) override
  {
    UNIMPLEMENTED();
  }

public:
  long GetPosition() const override
  {
    UNIMPLEMENTED();
  }

protected:
  void StartThread(const MiKTeX::Core::PathName & path, bool reading)
  {
    thrd = std::thread(&CompressedStreamBase::UncompressThread, this, path, reading);
  }

protected:
  void StopThread()
  {
    pipe.Close();
    thrd.join();
  }

protected:
  void UncompressThread(MiKTeX::Core::PathName path, bool reading)
  {
    try
    {
      if (!reading)
      {
        UNIMPLEMENTED();
      }
      DoUncompress(path);
      pipe.Close();
      Finish(true);
    }
    catch (const MiKTeX::Core::MiKTeXException & e)
    {
      threadMiKTeXException = e;
      Finish(false);
    }
    catch (const std::exception & e)
    {
      threadMiKTeXException = MiKTeX::Core::MiKTeXException(e.what());
      Finish(false);
    }
  }

protected:
  virtual void DoUncompress(const MiKTeX::Core::PathName & path) = 0;

protected:
  std::thread thrd;

protected:
  Pipe pipe;

protected:
  enum State {
    Ready = 1,
    Successful = 2
  };

protected:
  std::atomic_int state {0};

protected:
  bool IsReady()
  {
    return (state.load() & Ready) != 0;
  }

protected:
  bool IsSuccessful()
  {
    return (state.load() & Successful) != 0;
  }

protected:
  bool IsUnsuccessful()
  {
    return state.load() == Ready;
  }

protected:
  void Finish(bool successful)
  {
    state = Ready | (successful ? Successful : 0);
  }

protected:
  MiKTeX::Core::MiKTeXException threadMiKTeXException;
};

END_INTERNAL_NAMESPACE;
