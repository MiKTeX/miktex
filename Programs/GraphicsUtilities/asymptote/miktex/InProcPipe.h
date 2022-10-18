/**
 * @file miktex/InProcPipe.h
 * @author Christian Schenk
 * @brief In-process pipe
 *
 * @copyright Copyright © 2018-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

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

    ~InProcPipe()
    {
        delete[] buffer;
    }

    void Done() noexcept
    {
        done = true;
        readCondition.notify_one();
        writeCondition.notify_one();
    }

    bool IsDone()
    {
        return done;
    }

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

private:

    bool CanRead() const
    {
        return size > 0;
    }

    bool CanWrite() const
    {
        return size < capacity;
    }

    unsigned char* buffer = nullptr;
    size_t capacity = 0;
    std::atomic_bool done{ false };
    size_t head = 0;
    std::mutex mut;
    std::condition_variable readCondition;
    std::atomic_size_t size{ 0 };
    size_t tail = 0;
    std::condition_variable writeCondition;
};

MIKTEX_END_NS;
