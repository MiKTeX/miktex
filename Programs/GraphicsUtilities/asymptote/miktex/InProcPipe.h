/* miktex/InProcPipe.h:

   Copyright (C) 2017-2018 Christian Schenk

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

#pragma once

#include "asy-first.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <mutex>

#include <miktex/Core/Exceptions>

MIKTEX_BEGIN_NS;

class InProcPipe
{
public:
  InProcPipe()
  {
    capacity = 1024 * 64;
    buffer = new unsigned char[capacity];
  }

public:
  ~InProcPipe()
  {
    delete[] buffer;
  }

public:
  void Done() noexcept
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
    size_t written = 0;
    while (written < count)
    {
      std::unique_lock<std::mutex> lock(mut);
      writeCondition.wait(lock, [this] { return IsDone() || CanWrite(); });
      if (IsDone())
      {
        throw MiKTeX::Core::BrokenPipeException();
      }
      if (CanWrite())
      {
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
  }

public:
  size_t Read(void* data, size_t count)
  {
    if (!CanRead())
    {
      return 0;
    }
    std::unique_lock<std::mutex> lock(mut);
    size_t n = std::min(count, size.load());
    size_t num1 = std::min(n, capacity - head);
    size_t num2 = n - num1;
    memcpy(reinterpret_cast<unsigned char*>(data), buffer + head, num1);
    memcpy(reinterpret_cast<unsigned char*>(data) + num1, buffer, num2);
    head = (head + n) % capacity;
    size -= n;
    writeCondition.notify_one();
    return n;
  }

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
  size_t head = 0;
  size_t tail = 0;
  unsigned char* buffer = nullptr;
  std::mutex mut;
  std::atomic_bool done{ false };
  std::atomic_size_t size{ 0 };
  std::condition_variable writeCondition;
  std::condition_variable readCondition;
};

MIKTEX_END_NS;
