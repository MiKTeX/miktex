/**
 * @file unxProcess.h
 * @author Christian Schenk
 * @brief Process handling (Unix-alike)
 *
 * @copyright Copyright © 1996-2023 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#pragma once

#if !defined(B6278A08DFEE4038A08449DD17C4E3D3)
#define B6278A08DFEE4038A08449DD17C4E3D3

#include <memory>

#include <miktex/Core/Process>
#include <miktex/Core/TemporaryFile>

CORE_INTERNAL_BEGIN_NAMESPACE;

class unxProcess :
    public MiKTeX::Core::Process
{

public:

    FILE* get_StandardInput() override;

    FILE* get_StandardOutput() override;

    FILE* get_StandardError() override;

    void WaitForExit() override;

    bool WaitForExit(int milliseconds) override;

    MiKTeX::Core::ProcessExitStatus get_ExitStatus() const override;

    int get_ExitCode() const override;

    bool MIKTEXTHISCALL get_Exception(MiKTeX::Core::MiKTeXException& ex) const override;

    void Close() override;

    int GetSystemId() override;

    std::unique_ptr<MiKTeX::Core::Process> get_Parent() override;

    std::string get_ProcessName() override;

    MiKTeX::Core::ProcessInfo GetProcessInfo() override;

    unxProcess()
    {
    }

    unxProcess(const MiKTeX::Core::ProcessStartInfo& startinfo);

    ~unxProcess() override;

private:

    void Create();

    int fdStandardError = -1;
    int fdStandardInput = -1;
    int fdStandardOutput = -1;
    FILE* pFileStandardError = nullptr;
    FILE* pFileStandardInput = nullptr;
    FILE* pFileStandardOutput = nullptr;
    pid_t pid = -1;
    MiKTeX::Core::ProcessStartInfo startinfo;
    int status;
    std::unique_ptr<MiKTeX::Core::TemporaryFile> tmpFile;

    friend class MiKTeX::Core::Process;
};

CORE_INTERNAL_END_NAMESPACE;

#endif
