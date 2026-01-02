/**
 * @file Stream/CompressedStreamBase.h
 * @author Christian Schenk
 * @brief Compressed stream base implementation
 *
 * @copyright Copyright © 1996-2026 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include <thread>

#include "Utils/Pipe.h"

CORE_INTERNAL_BEGIN_NAMESPACE;

template<typename Interface> class CompressedStreamBase :
    public Interface
{

public:

    size_t Read(void* data, size_t count) override
    {
        if (IsUnsuccessful())
        {
            throw threadMiKTeXException;
        }
        return pipe.Read(data, count);
    }

    void Write(const void* data, size_t count) override
    {
        if (IsUnsuccessful())
        {
            throw threadMiKTeXException;
        }
        pipe.Write(data, count);
    }

    void Seek(long offset, MiKTeX::Core::SeekOrigin seekOrigin) override
    {
        UNIMPLEMENTED();
    }

    long GetPosition() const override
    {
        UNIMPLEMENTED();
    }

protected:

    void StartThread(const MiKTeX::Util::PathName& path, bool reading)
    {
        worker = std::thread(&CompressedStreamBase::Worker, this, path, reading);
    }

    void StopThread()
    {
        pipe.Close();
        worker.join();
    }

    void Worker(MiKTeX::Util::PathName path, bool reading)
    {
        try
        {
            if (reading)
            {
                DoCompress(path);
            }
            else
            {
                DoUncompress(path);
            }
            pipe.Close();
            Finish(true);
        }
        catch (const MiKTeX::Core::MiKTeXException& e)
        {
            threadMiKTeXException = e;
            Finish(false);
        }
        catch (const std::exception& e)
        {
            threadMiKTeXException = MiKTeX::Core::MiKTeXException(e.what());
            Finish(false);
        }
    }

    virtual void DoCompress(const MiKTeX::Util::PathName& path) = 0;
    virtual void DoUncompress(const MiKTeX::Util::PathName& path) = 0;

    std::thread worker;
    Pipe pipe;

    enum State {
        Ready = 1,
        Successful = 2
    };

    std::atomic_int state{ 0 };

    bool IsReady()
    {
        return (state.load() & Ready) != 0;
    }

    bool IsSuccessful()
    {
        return (state.load() & Successful) != 0;
    }

    bool IsUnsuccessful()
    {
        return state.load() == Ready;
    }

    void Finish(bool successful)
    {
        state = Ready | (successful ? Successful : 0);
    }

    MiKTeX::Core::MiKTeXException threadMiKTeXException;
};

CORE_INTERNAL_END_NAMESPACE;
