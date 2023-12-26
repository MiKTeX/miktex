/**
 * @file util.cpp
 * @author Christian Schenk
 * @briefUtility functions (Unix)
 *
 * @copyright Copyright © 1996-2023 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include "config.h"

#include <unistd.h>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

#if defined(HAVE_SYS_UTSNAME_H)
#include <sys/utsname.h>
#endif

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Directory>
#include <miktex/Core/File>
#include <miktex/Util/PathName>
#include <miktex/Util/StringUtil>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

string Utils::GetOSVersionString()
{
    string version;
#if defined(HAVE_UNAME_SYSCALL)
    struct utsname buf;
    if (uname(&buf) < 0)
    {
        MIKTEX_FATAL_CRT_ERROR("uname");
    }
    version = buf.sysname;
    version += ' ';
    version += buf.release;
    version += ' ';
    version += buf.version;
    version += ' ';
    version += buf.machine;
#else
#warning Unimplemented : Utils::GetOSVersionString
    version = "UnkOS 0.1";
#endif
    return version;
}

void Utils::SetEnvironmentString(const string& valueName, const string& value)
{
    string oldValue;
    if (::GetEnvironmentString(valueName, oldValue) && oldValue == value)
    {
        return;
    }
    auto trace_config = TraceStream::Open(MIKTEX_TRACE_CONFIG);
    trace_config->WriteLine("core", fmt::format(T_("setting env {0}={1}"), valueName, value));
    if (setenv(valueName.c_str(), value.c_str(), 1) != 0)
    {
        MIKTEX_FATAL_CRT_ERROR_2("setenv", "name", valueName);
    }
}

void Utils::RemoveEnvironmentString(const string& valueName)
{
    auto trace_config = TraceStream::Open(MIKTEX_TRACE_CONFIG);
    trace_config->WriteLine("core", fmt::format(T_("unsetting env {0}"), valueName));
    if (unsetenv(valueName.c_str()) != 0)
    {
        MIKTEX_FATAL_CRT_ERROR_2("unsetenv", "name", valueName);
    }
}

void Utils::CheckHeap()
{
}

void Utils::ShowWebPage(const string& url)
{
    UNIMPLEMENTED();
}

bool Utils::SupportsHardLinks(const PathName &path)
{
    return true;
}

bool Utils::CheckPath(bool repair)
{
#if 1
    // TODO
    if (repair)
    {
        UNIMPLEMENTED();
    }
#endif

    shared_ptr<Session> session = MIKTEX_SESSION();

    string envPath;
    if (!Utils::GetEnvironmentString("PATH", envPath))
    {
        return false;
    }

    PathName linkTargetDirectory = session->GetSpecialPath(SpecialPath::LinkTargetDirectory);

    string repairedPath;
    bool pathCompetition;

    bool pathOkay = !Directory::Exists(linkTargetDirectory) || !FixProgramSearchPath(envPath, linkTargetDirectory, true, repairedPath, pathCompetition);

    bool repaired = false;

    if (!pathOkay && !repair)
    {
        SESSION_IMPL()->trace_error->WriteLine("core", T_("Something is wrong with the PATH:"));
        SESSION_IMPL()->trace_error->WriteLine("core", envPath.c_str());
    }
    else if (!pathOkay && repair)
    {
        SESSION_IMPL()->trace_error->WriteLine("core", T_("Setting new PATH:"));
        SESSION_IMPL()->trace_error->WriteLine("core", repairedPath.c_str());
        envPath = repairedPath;
        if (session->IsAdminMode())
        {
            // TODO: edit system configuration
        }
        else
        {
            // TODO: edit user configuration
        }
        pathOkay = true;
        repaired = true;
    }
    return repaired || pathOkay;
}

PathName Utils::GetExe()
{
#if defined(__APPLE__)
    CharBuffer<char> buf;
    uint32_t bufSize = buf.GetCapacity();
    if (_NSGetExecutablePath(buf.GetData(), &bufSize) < 0)
    {
        buf.Reserve(bufSize);
        if (_NSGetExecutablePath(buf.GetData(), &bufSize) != 0)
        {
            MIKTEX_UNEXPECTED();
        }
    }
    return PathName(buf.GetData());
#else
    return File::ReadSymbolicLink(PathName("/proc/self/exe"));
#endif
}

string Utils::GetExeName()
{
#if defined(__APPLE__)
    return GetExe().GetFileNameWithoutExtension().ToString();
#else
    ifstream cmdline = File::CreateInputStream(PathName("/proc/self/comm"));
    string argv0;
    char ch;
    while (cmdline.get(ch) && ch != '\n' && ch != 0)
    {
        argv0 += ch;
    }
    cmdline.close();
    return PathName(argv0).GetFileNameWithoutExtension().ToString();
#endif
}
