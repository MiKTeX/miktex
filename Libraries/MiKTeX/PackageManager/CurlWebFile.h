/* CurlWebFile.h:                                       -*- C++ -*-

   Copyright (C) 2001-2021 Christian Schenk

   This file is part of MiKTeX Package Manager.

   MiKTeX Package Manager is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Package Manager is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Package Manager; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if !defined(DB88FC1DC9B0497C965F3CFB916FA93A)
#define DB88FC1DC9B0497C965F3CFB916FA93A

#if defined(HAVE_LIBCURL)

#include <string>
#include <memory>
#include <unordered_map>

#include <curl/curl.h>

#include <miktex/Core/Debug>
#include <miktex/Core/Session>
#include <miktex/Trace/TraceStream>

#include "CurlWebSession.h"
#include "WebFile.h"

MPM_INTERNAL_BEGIN_NAMESPACE;

#if defined(_MSC_VER)
#  pragma push_macro("min")
#  undef min
#endif

class CircularBuffer
{
public:
  CircularBuffer()
  {
    capacity = 10 * CURL_MAX_WRITE_SIZE;
    buffer = new unsigned char[capacity];
  }

public:
  ~CircularBuffer()
  {
    delete[] buffer;
  }

public:
  void Clear()
  {
    size = 0;
    head = 0;
    tail = 0;
  }

public:
  void Write(const void* data, std::size_t count)
  {
    if (!CanWrite(count))
    {
      MIKTEX_UNEXPECTED();
    }
    std::size_t num1 = std::min(count, capacity - tail);
    std::size_t num2 = count - num1;
    memcpy(buffer + tail, data, num1);
    memcpy(buffer, reinterpret_cast<const unsigned char*>(data) + num1, num2);
    tail = (tail + count) % capacity;
    size += count;
  }

public:
  void Read(void* data, std::size_t count)
  {
    if (!CanRead(count))
    {
      MIKTEX_UNEXPECTED();
    }
    std::size_t num1 = std::min(count, capacity - head);
    std::size_t num2 = count - num1;
    memcpy(data, buffer + head, num1);
    memcpy(reinterpret_cast<unsigned char*>(data) + num1, buffer, num2);
    head = (head + count) % capacity;
    size -= count;
  }

public:
  bool CanWrite(std::size_t n) const
  {
    return size + n <= capacity;
  }

public:
  std::size_t GetSize() const
  {
    return size;
  }

public:
  void Reserve(std::size_t newCapacity)
  {
    MIKTEX_ASSERT(newCapacity >= capacity);
    unsigned char* newBuffer = new unsigned char[newCapacity];
    std::size_t oldSize = size;
    Read(newBuffer, size);
    delete[] buffer;
    buffer = newBuffer;
    capacity = newCapacity;
    size = oldSize;
    head = 0;
    tail = size;
  }

public:
  std::size_t GetCapacity() const
  {
    return capacity;
  }

private:
  bool CanRead(std::size_t n) const
  {
    return size >= n;
  }

private:
  std::size_t capacity = 0;
  std::size_t size = 0;
  std::size_t head = 0;
  std::size_t tail = 0;
  unsigned char* buffer = nullptr;
};

#if defined(_MSC_VER)
#  pragma pop_macro("min")
#endif

class CurlWebFile :
  public WebFile
{
public:
  CurlWebFile(std::shared_ptr<CurlWebSession> webSession, const std::string& url, const std::unordered_map<std::string, std::string>& formData);

public:
  ~CurlWebFile() override;

public:
  std::size_t Read(void* data, std::size_t n) override;

public:
  void Close() override;

private:
  static std::size_t WriteCallback(char* data, std::size_t elemSize, std::size_t numElements, void* pv);

private:
  void Initialize();

private:
  bool initialized = false;

private:
  std::shared_ptr<CurlWebSession> webSession;

private:
  std::string url;

private:
  std::string urlEncodedpostFields;

private:
  CircularBuffer buffer;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mpm;
};

MPM_INTERNAL_END_NAMESPACE;

#endif

#endif
