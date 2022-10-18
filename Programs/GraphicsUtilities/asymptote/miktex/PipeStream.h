/**
 * @file miktex/PipeStream.h
 * @author Christian Schenk
 * @brief Pipe stream
 *
 * @copyright Copyright © 2017-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#include "asy-first.h"
#include "config.h"

#include <cstdio>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <miktex/Util/PathName>
#include <miktex/Core/Process>

#include "InProcPipe.h"

#if !defined(MIKTEX_BEGIN_NS)
#define MIKTEX_BEGIN_NS                         \
namespace MiKTeX {                              \
    namespace Aymptote {
#define MIKTEX_END_NS                           \
    }                                           \
}
#endif

MIKTEX_BEGIN_NS;

class PipeStream
{

public:

    virtual ~PipeStream();

    void Close();
    void CloseIn();
    void Open(const MiKTeX::Util::PathName& fileName, const std::vector<std::string>& arguments);
    size_t Read(void* buf, size_t size);
    int Wait();
    void Write(const void* buf, size_t size);

    bool IsChildRunning()
    {
        return !childStdoutPipe.IsDone();
    }

protected:

    void Finish(bool successful)
    {
        state = Ready | (successful ? Successful : 0);
    }

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

    enum State
    {
        Ready = 1,
        Successful = 2
    };

    MiKTeX::Core::ProcessStartInfo childStartInfo;
    MiKTeX::Core::MiKTeXException childStdoutReaderThreadException;
    std::atomic_size_t childStdoutTotalBytes{ 0 };
    std::atomic_int state{ 0 };

private:

    void StartThreads();
    void StopThreads();
    void ChildStdoutReaderThread();

    std::unique_ptr<MiKTeX::Core::Process> childProcess;
    FILE* childStdinFile = nullptr;
    InProcPipe childStdoutPipe;
    std::thread childStdoutReaderThread;
};

MIKTEX_END_NS;
