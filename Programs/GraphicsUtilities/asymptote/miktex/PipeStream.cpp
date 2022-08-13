/**
 * @file miktex/PipeStream.cpp
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

#if defined(MIKTEX_WINDOWS)
#include <io.h>
#endif

#include <miktex/App/Application>

#include <miktex/Core/FileStream>
#include <miktex/Core/Session>
#include <miktex/Util/StringUtil>

#include "PipeStream.h"

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

using namespace MiKTeX::Aymptote;

PipeStream::~PipeStream()
{
    Close();
}

void PipeStream::Open(const PathName& fileName, const vector<string>& arguments)
{
    childStartInfo.FileName = fileName.ToString();
    childStartInfo.Arguments = arguments;
    childStartInfo.RedirectStandardInput = true;
    childStartInfo.RedirectStandardError = true;
    childStartInfo.RedirectStandardOutput = true;
    childProcess = Process::Start(childStartInfo);
    Application::GetApplication()->LogInfo("started PipeStream child process " + std::to_string(childProcess->GetSystemId()) + ": " + StringUtil::Flatten(arguments, ' '));
    childStdinFile = childProcess->get_StandardInput();
    setvbuf(childStdinFile, nullptr, _IONBF, 0);
    StartThreads();
}

void PipeStream::Close()
{
    CloseIn();
    StopThreads();
    if (childProcess != nullptr)
    {
        if (!childProcess->WaitForExit(1000))
        {
            Application::GetApplication()->LogWarn("PipeStream child process " + std::to_string(childProcess->GetSystemId()) + " is still running: " + StringUtil::Flatten(childStartInfo.Arguments, ' '));
        }
    }
}

void PipeStream::CloseIn()
{
    if (childStdinFile != nullptr)
    {
        if (fclose(childStdinFile) != 0)
        {
            MIKTEX_FATAL_CRT_ERROR("fclose");
        }
        childStdinFile = nullptr;
    }
}

void PipeStream::Write(const void* buf, size_t size)
{
    if (IsUnsuccessful())
    {
        throw childStdoutReaderThreadException;
    }
    if (!IsChildRunning())
    {
        MIKTEX_FATAL_ERROR_2("Broken pipe", "argv0", childStartInfo.FileName);
    }
    if (fwrite(buf, 1, size, childStdinFile) != size)
    {
        MIKTEX_FATAL_CRT_ERROR("fwrite");
    }
    if (fflush(childStdinFile) != 0)
    {
        MIKTEX_FATAL_CRT_ERROR("fflush");
    }
}

size_t PipeStream::Read(void* buf, size_t size)
{
    if (IsUnsuccessful())
    {
        throw childStdoutReaderThreadException;
    }
    return childStdoutPipe.Read(buf, size);
}

int PipeStream::Wait()
{
    if (!childProcess->WaitForExit(10000))
    {
        MIKTEX_FATAL_ERROR("PipeStream child process did not complete");
    }
    return childProcess->get_ExitCode();
}

void PipeStream::StartThreads()
{
    childStdoutReaderThread = std::thread(&PipeStream::ChildStdoutReaderThread, this);
}

void PipeStream::StopThreads()
{
    if (childStdoutReaderThread.joinable())
    {
        childStdoutPipe.Done();
        childStdoutReaderThread.join();
    }
}

void PipeStream::ChildStdoutReaderThread()
{
    try
    {
        FileStream childStdoutFile(childProcess->get_StandardOutput());
        setvbuf(childStdoutFile.GetFile(), nullptr, _IONBF, 0);
        HANDLE childStdoutFileHandle = (HANDLE)_get_osfhandle(fileno(childStdoutFile.GetFile()));
        if (childStdoutFileHandle == INVALID_HANDLE_VALUE)
        {
            MIKTEX_UNEXPECTED();
        }
        const size_t BUFFER_SIZE = 1024 * 32;
        unsigned char inbuf[BUFFER_SIZE];
        do
        {
            DWORD avail;
            if (!PeekNamedPipe(childStdoutFileHandle, nullptr, 0, nullptr, &avail, nullptr))
            {
                if (GetLastError() == ERROR_BROKEN_PIPE)
                {
                    Application::GetApplication()->LogWarn("broken PipeStream (" + std::to_string(childProcess->GetSystemId()) + ") after " + std::to_string(childStdoutTotalBytes) + " bytes");
                    break;
                }
                MIKTEX_FATAL_WINDOWS_ERROR("PeekNamedPipe");
            }
            if (avail == 0)
            {
                continue;
            }
            size_t n = childStdoutFile.Read(inbuf, BUFFER_SIZE > avail ? avail : BUFFER_SIZE);
            childStdoutPipe.Write(inbuf, n);
            childStdoutTotalBytes += n;
        } while (true);
        Finish(true);
        childStdoutFile.Close();
        childStdoutPipe.Done();
    }
    catch (const MiKTeX::Core::MiKTeXException& e)
    {
        childStdoutReaderThreadException = e;
        Finish(false);
        childStdoutPipe.Done();
        Application::GetApplication()->LogError("MiKTeX exception caught: "s + e.GetErrorMessage());
    }
    catch (const std::exception& e)
    {
        childStdoutReaderThreadException = MiKTeX::Core::MiKTeXException(e.what());
        Finish(false);
        childStdoutPipe.Done();
        Application::GetApplication()->LogError("std exception caught: "s + e.what());
    }
}
