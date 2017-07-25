/* miktex/InProcPipe.h:

   Copyright (C) 2017 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */

#if defined(_MSC_VER)
#  pragma once
#endif

#include <algorithm>
#include <atomic>
#include <chrono>
#include <mutex>

#include <miktex/Core/Exceptions>

#if !defined(MIKTEX_BEGIN_NS)
#define MIKTEX_BEGIN_NS                         \
namespace MiKTeX {                              \
  namespace Aymptote {
#define MIKTEX_END_NS                           \
  }                                             \
}
#endif

MIKTEX_BEGIN_NS;

class InProcPipe
{
public:
  InProcPipe()
  {
    capacity = 1024 * 32;
    buffer = new unsigned char[capacity];
  }

public:
  ~InProcPipe()
  {
    delete[] buffer;
  }

public:
  void Close() noexcept
  {
    done = true;
    readCondition.notify_one();
    writeCondition.notify_one();
  }

public:
  bool IsDone()
  {
    return done;
  }

public:
  void Write(const void* data, size_t count)
  {
    std::unique_lock<std::mutex> lock(mut);
    size_t written = 0;
    while (written < count)
    {
      writeCondition.wait(lock, [this] { return done || CanWrite(); });
      if (done)
      {
        throw MiKTeX::Core::BrokenPipeException();
      }
      size_t n = std::min(count - written, capacity - size);
      size_t num1 = std::min(n, capacity - tail);
      size_t num2 = n - num1;
      memcpy(buffer + tail, reinterpret_cast<const unsigned char*>(data) + written, num1);
      memcpy(buffer, reinterpret_cast<const unsigned char*>(data) + written + num1, num2);
      tail = (tail + n) % capacity;
      size += n;
      readCondition.notify_one();
      written += n;
    }
  }

public:
  size_t Read(void* data, size_t count)
  {
    std::unique_lock<std::mutex> lock(mut);
    size_t read = 0;
    bool noMore = false;
    while (read < count && !noMore)
    {
      if (!readCondition.wait_for(lock, std::chrono::milliseconds(100), [this] { return done || CanRead(); }))
      {
        return read;
      }
      size_t n = std::min(count - read, size);
      size_t num1 = std::min(n, capacity - head);
      size_t num2 = n - num1;
      memcpy(reinterpret_cast<unsigned char*>(data) + read, buffer + head, num1);
      memcpy(reinterpret_cast<unsigned char*>(data) + read + num1, buffer, num2);
      head = (head + n) % capacity;
      size -= n;
      writeCondition.notify_one();
      read += n;
      noMore = done;
    }
    return read;
  }

#if 0
private:
  void Resize(size_t newCapacity)
  {
    if (done || newCapacity <= capacity)
    {
      return;
    }
    unsigned char * newBuffer = new unsigned char[newCapacity];
    size_t size = GetSize();
    size_t num1 = min(size, capacity - head);
    size_t num2 = size - num1;
    memcpy(newBuffer, buffer + head, num1);
    memcpy(newBuffer + num1, buffer, num2);
    delete[] buffer;
    buffer = newBuffer;
    head = 0;
    tail = size;
  }
#endif

private:
  bool CanWrite() const
  {
    return size < capacity;
  }

private:
  bool CanRead() const
  {
    return size > 0;
  }

private:
  size_t capacity = 0;
  size_t size = 0;
  size_t head = 0;
  size_t tail = 0;
  std::atomic_bool done{ false };
  unsigned char* buffer = nullptr;
  std::mutex mut;
  std::condition_variable writeCondition;
  std::condition_variable readCondition;
};

MIKTEX_END_NS;
