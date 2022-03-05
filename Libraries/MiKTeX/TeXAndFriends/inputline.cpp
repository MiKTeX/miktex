/**
 * @file inputline.cpp
 * @author Christian Schenk
 * @brief MiKTeX WebApp input line base implementation
 *
 * @copyright Copyright © 1996-2022 Christian Schenk
 *
 * This file is part of the MiKTeX TeXMF Framework.
 *
 * The MiKTeX TeXMF Framework is licensed under GNU General Public License
 * version 2 or any later version.
 */

#include <unordered_map>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Configuration/ConfigNames>
#include <miktex/Core/CommandLineBuilder>

#if defined(MIKTEX_TEXMF_SHARED)
#   define C4PEXPORT MIKTEXDLLEXPORT
#else
#   define C4PEXPORT
#endif
#define C1F0C63F01D5114A90DDF8FC10FF410B
#include "miktex/C4P/C4P.h"

#if defined(MIKTEX_TEXMF_SHARED)
#   define MIKTEXMFEXPORT MIKTEXDLLEXPORT
#else
#   define MIKTEXMFEXPORT
#endif
#define B8C7815676699B4EA2DE96F0BD727276
#include "miktex/TeXAndFriends/WebAppInputLine.h"

#include "internal.h"

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace MiKTeX::TeXAndFriends;

struct Bom
{
    static constexpr int UTF8 = 0xbfbbef;
    static constexpr int UTF8_length = 3;
    static constexpr int UTF8_mask = 0xffffff;
    static constexpr int UTF16le = 0xfeff;
    static constexpr int UTF16be = 0xfffe;
    static constexpr int UTF16_length = 2;
    static constexpr int UTF16_mask = 0xffff;
};

void Seek(FILE* file, int pos)
{
    if (fseek(file, pos, SEEK_SET) < 0)
    {
        MIKTEX_FATAL_CRT_ERROR("fseek");
    }
}

int CheckBom(FILE* file)
{
    long filePosition = ftell(file);
    if (filePosition < 0)
    {
        MIKTEX_FATAL_CRT_ERROR("ftell");
    }
    if (filePosition > 0)
    {
        return 0;
    }
    int val = 0;
    MIKTEX_ASSERT(Bom::UTF8_length >= Bom::UTF16_length);
    size_t n = fread(&val, 1, Bom::UTF8_length, file);
    if (ferror(file) != 0)
    {
        MIKTEX_FATAL_CRT_ERROR("fread");
    }
    bool maybeUtf16bom = false;
    if (n == Bom::UTF8_length)
    {
        int bom = val & Bom::UTF8_mask;
        if (bom == Bom::UTF8)
        {
        return bom;
        }
        else
        {
        maybeUtf16bom = true;
        }
    }
    if (n == Bom::UTF16_length || maybeUtf16bom)
    {
        int bom = val & Bom::UTF16_mask;
        if (bom == Bom::UTF16be || bom == Bom::UTF16le)
        {
        Seek(file, Bom::UTF16_length);
        return bom;
        }
    }
    Seek(file, 0);
    return 0;
}

struct OpenFileInfo
{
    FileAccess access;
    FileMode mode;
    PathName path;
};

class WebAppInputLine::impl
{
public:
    PathName outputDirectory;
    PathName auxDirectory;
    int optBase;
    PathName foundFile;
    PathName foundFileFq;
    ShellCommandMode shellCommandMode = ShellCommandMode::Forbidden;
    PathName lastInputFileName;
    IInputOutput* inputOutput = nullptr;
    TriState allowInput = TriState::Undetermined;
    TriState allowOutput = TriState::Undetermined;
    unordered_map<const FILE*, OpenFileInfo> openFiles;
};

WebAppInputLine::WebAppInputLine() :
    pimpl(make_unique<impl>())
{
}

WebAppInputLine::~WebAppInputLine() noexcept
{
}

void WebAppInputLine::Init(vector<char*>& args)
{
    WebApp::Init(args);
    pimpl->shellCommandMode = ShellCommandMode::Forbidden;
}

void WebAppInputLine::Finalize()
{
    pimpl->foundFile.Clear();
    pimpl->foundFileFq.Clear();
    pimpl->lastInputFileName.Clear();
    pimpl->outputDirectory.Clear();
    pimpl->auxDirectory.Clear();
    WebApp::Finalize();
}

void WebAppInputLine::AddOptions()
{
    WebApp::AddOptions();
    pimpl->optBase = (int)GetOptions().size();
}

bool WebAppInputLine::ProcessOption(int opt, const string& optArg)
{
    return WebApp::ProcessOption(opt, optArg);
}

static bool IsOutputFile(const PathName& path)
{
    PathName path_(path);
    if (path_.HasExtension(".gz"))
    {
        path_.SetExtension(nullptr);
    }
    return path_.HasExtension(".dvi")
        || path_.HasExtension(".pdf")
        || path_.HasExtension(".synctex");
}

bool WebAppInputLine::AllowFileName(const PathName& fileName, bool forInput)
{
    shared_ptr<Session> session = GetSession();
    bool allow;
    if (forInput)
    {
        if (pimpl->allowInput == TriState::Undetermined)
        {
            allow = session->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_ALLOWUNSAFEINPUTFILES).GetBool();
            pimpl->allowInput = allow ? TriState::True : TriState::False;
        }
        else
        {
            allow = pimpl->allowInput == TriState::True ? true : false;
        }
    }
    else
    {
        if (pimpl->allowOutput == TriState::Undetermined)
        {
            allow = session->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_ALLOWUNSAFEOUTPUTFILES).GetBool();
            pimpl->allowOutput = allow ? TriState::True : TriState::False;
        }
        else
        {
            allow = pimpl->allowOutput == TriState::True ? true : false;
        }
    }
    if (allow)
    {
        return true;
    }
    return Utils::IsSafeFileName(fileName);
}

bool WebAppInputLine::OpenOutputFile(C4P::FileRoot& f, const PathName& fileNameInternalEncoding, bool isTextFile_deprecated, PathName& outPath)
{
    auto fileName = DecodeFileName(fileNameInternalEncoding);
    shared_ptr<Session> session = GetSession();
    FILE* file = nullptr;
    if (fileName[0] == '|')
    {
        string command = fileName.GetData() + 1;
#if defined(MIKTEX_WINDOWS)
        std::replace(command.begin(), command.end(), '\'', '"');
#endif
        Session::ExamineCommandLineResult examineResult;
        string examinedCommand;
        string safeCommandLine;
        tie(examineResult, examinedCommand, safeCommandLine) = session->ExamineCommandLine(command);
        if (examineResult == Session::ExamineCommandLineResult::SyntaxError)
        {
            LogError(fmt::format("syntax error: {0}", command));
            return false;
        }
        if (examineResult != Session::ExamineCommandLineResult::ProbablySafe && examineResult != Session::ExamineCommandLineResult::MaybeSafe)
        {
            LogError(fmt::format("command is unsafe: {0}", command));
            return false;
        }
        string toBeExecuted;
        switch (pimpl->shellCommandMode)
        {
        case ShellCommandMode::Unrestricted:
            if (session->RunningAsAdministrator() && !session->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_ALLOW_UNRESTRICTED_SUPER_USER).GetBool())
            {
                LogError(fmt::format("not allowed with elevated privileges: {0}", command));
                return false;
            }
            toBeExecuted = command;
            break;
        case ShellCommandMode::Forbidden:
            LogError(fmt::format("command not executed: {0}", command));
            return false;
        case ShellCommandMode::Query:
            // TODO
        case ShellCommandMode::Restricted:
            if (examineResult != Session::ExamineCommandLineResult::ProbablySafe)
            {
                LogError(fmt::format("command not allowed: {0}", command));
                return false;
            }
            toBeExecuted = safeCommandLine;
            break;
        default:
            MIKTEX_UNEXPECTED();
        }
        if (examineResult == Session::ExamineCommandLineResult::ProbablySafe)
        {
            LogInfo(fmt::format("executing restricted output pipe: {0}", toBeExecuted));
        }
        else
        {
            LogWarn(fmt::format("executing unrestricted output pipe: {0}", toBeExecuted));
        }
        file = OpenFileInternal(PathName(toBeExecuted), FileMode::Command, FileAccess::Write);
        pimpl->openFiles[file] = OpenFileInfo{ FileAccess::Write, FileMode::Command, PathName(toBeExecuted) };
    }
    else
    {
        bool isAuxFile = !IsOutputFile(fileName);
        PathName path;
        if (isAuxFile && !pimpl->auxDirectory.Empty())
        {
            path = pimpl->auxDirectory / fileName;
            fileName = path.GetData();
        }
        else if (!pimpl->outputDirectory.Empty())
        {
            path = pimpl->outputDirectory / fileName;
            fileName = path.GetData();
        }
        file = TryOpenFileInternal(fileName, FileMode::Create, FileAccess::Write);
        if (file != nullptr)
        {
            outPath = fileName;
            pimpl->openFiles[file] = OpenFileInfo{ FileAccess::Write,  FileMode::Create, outPath };
        }
    }
    if (file == nullptr)
    {
        return false;
    }
    f.Attach(file, true);
    return true;
}

MiKTeX::Util::PathName WebAppInputLine::DecodeFileName(const PathName& fileNameInternalEncoding)
{
#if defined(MIKTEX_WINDOWS)
    if (!Utils::IsUTF8(fileNameInternalEncoding.GetData()))
    {
        LogWarn("converting ANSI file name");
        auto fileName = StringUtil::AnsiToUTF8(fileNameInternalEncoding.GetData());
        LogWarn("conversion succeeded: " + fileName);
        return PathName(fileName);
    }
#endif
    return fileNameInternalEncoding;
}

bool WebAppInputLine::OpenInputFile(FILE** ppFile, const PathName& fileNameInternalEncoding)
{
    auto fileName = DecodeFileName(fileNameInternalEncoding);

    shared_ptr<Session> session = GetSession();

    if (fileName[0] == '|')
    {
        string command = fileName.GetData() + 1;
#if defined(MIKTEX_WINDOWS)
        std::replace(command.begin(), command.end(), '\'', '"');
#endif
        Session::ExamineCommandLineResult examineResult;
        string examinedCommand;
        string toBeExecuted;
        tie(examineResult, examinedCommand, toBeExecuted) = session->ExamineCommandLine(command);
        if (examineResult == Session::ExamineCommandLineResult::SyntaxError)
        {
            LogError("command line syntax error: " + command);
            return false;
        }
        if (examineResult != Session::ExamineCommandLineResult::ProbablySafe && examineResult != Session::ExamineCommandLineResult::MaybeSafe)
        {
            LogError("command is unsafe: " + command);
            return false;
        }
        switch (pimpl->shellCommandMode)
        {
        case ShellCommandMode::Unrestricted:
            toBeExecuted = command;
            break;
        case ShellCommandMode::Forbidden:
            LogError("command not executed: " + command);
            return false;
        case ShellCommandMode::Query:
            // TODO
        case ShellCommandMode::Restricted:
            if (examineResult != Session::ExamineCommandLineResult::ProbablySafe)
            {
                LogError("command not allowed: " + command);
                return false;
            }
            break;
        default:
            MIKTEX_UNEXPECTED();
        }
        LogInfo("executing input pipe: " + toBeExecuted);
        *ppFile = OpenFileInternal(PathName(toBeExecuted), FileMode::Command, FileAccess::Read);
        pimpl->openFiles[*ppFile] = OpenFileInfo{ FileAccess::Read,  FileMode::Command, PathName(toBeExecuted) };
        pimpl->foundFile.Clear();
        pimpl->foundFileFq.Clear();
    }
    else
    {
        if (!session->FindFile(fileName.GetData(), GetInputFileType(), pimpl->foundFile))
        {
            return false;
        }

        pimpl->foundFileFq = pimpl->foundFile;
        pimpl->foundFileFq.MakeFullyQualified();

#if 1 // 2015-01-15
        if (pimpl->foundFile[0] == '.' && PathNameUtil::IsDirectoryDelimiter(pimpl->foundFile[1]))
        {
            PathName temp(pimpl->foundFile.GetData() + 2);
            pimpl->foundFile = temp;
        }
#endif

        try
        {
            if (pimpl->foundFile.HasExtension(".gz"))
            {
                CommandLineBuilder cmd("zcat");
                cmd.AppendArgument(pimpl->foundFile);
                *ppFile = OpenFileInternal(PathName(cmd.ToString()), FileMode::Command, FileAccess::Read);
                pimpl->openFiles[*ppFile] = OpenFileInfo{ FileAccess::Read, FileMode::Command, PathName(cmd.ToString()) };
            }
            else if (pimpl->foundFile.HasExtension(".bz2"))
            {
                CommandLineBuilder cmd("bzcat");
                cmd.AppendArgument(pimpl->foundFile);
                *ppFile = OpenFileInternal(PathName(cmd.ToString()), FileMode::Command, FileAccess::Read);
                pimpl->openFiles[*ppFile] = OpenFileInfo{ FileAccess::Read, FileMode::Command, PathName(cmd.ToString()) };
            }
            else if (pimpl->foundFile.HasExtension(".xz") || pimpl->foundFile.HasExtension(".lzma"))
            {
                CommandLineBuilder cmd("xzcat");
                cmd.AppendArgument(pimpl->foundFile);
                *ppFile = OpenFileInternal(PathName(cmd.ToString()), FileMode::Command, FileAccess::Read);
                pimpl->openFiles[*ppFile] = OpenFileInfo{ FileAccess::Read, FileMode::Command, PathName(cmd.ToString()) };
            }
            else
            {
                *ppFile = OpenFileInternal(pimpl->foundFile, FileMode::Open, FileAccess::Read);
                pimpl->openFiles[*ppFile] = OpenFileInfo{ FileAccess::Read, FileMode::Open, pimpl->foundFile };
            }
        }
#if defined(MIKTEX_WINDOWS)
        catch (const SharingViolationException&)
        {
        }
#endif
        catch (const UnauthorizedAccessException&)
        {
        }
        catch (const FileNotFoundException&)
        {
        }
    }

    if (*ppFile == nullptr)
    {
        return false;
    }

    if (!AmI("xetex"))
    {
        auto openFileInfo = session->TryGetOpenFileInfo(*ppFile);
        if (openFileInfo.first && openFileInfo.second.mode != FileMode::Command)
        {
            int bom = CheckBom(*ppFile);
            switch (bom)
            {
            case Bom::UTF8:
                LogInfo("UTF8 BOM detected: " + openFileInfo.second.fileName);
                break;
            case Bom::UTF16be:
                LogInfo("UTF16be BOM detected: " + openFileInfo.second.fileName);
                break;
            case Bom::UTF16le:
                LogInfo("UTF16le BOM detected: " + openFileInfo.second.fileName);
                break;
            }
        }
    }

    pimpl->lastInputFileName = fileName;

    return true;
}

bool WebAppInputLine::OpenInputFile(C4P::FileRoot& f, const PathName& fileNameInternalEncoding)
{
    FILE* file = nullptr;

    if (!OpenInputFile(&file, fileNameInternalEncoding))
    {
        return false;
    }

    f.Attach(file, true);

#if defined(PASCAL_TEXT_IO)
    MIKTEX_UNIMPLEMENTED();
#endif

    return true;
}

void WebAppInputLine::CloseFile(C4P::FileRoot& f)
{
    f.AssertValid();
    unordered_map<const FILE*, OpenFileInfo>::iterator it = pimpl->openFiles.find(f);
    bool isCommand = false;
    bool isOutput = false;
    if (it != pimpl->openFiles.end())
    {
        isCommand = (it->second.mode == FileMode::Command);
        isOutput = (it->second.access == FileAccess::Write);
        pimpl->openFiles.erase(it);
    }
    if (isOutput)
    {
        TouchJobOutputFile(f);
    }
    CloseFileInternal(f);
}

void WebAppInputLine::TouchJobOutputFile(FILE*) const
{
}

void WebAppInputLine::SetOutputDirectory(const PathName& path)
{
    if (pimpl->outputDirectory == path)
    {
        return;
    }
    LogInfo("setting output directory: " + path.ToString());
    pimpl->outputDirectory = path;
}

PathName WebAppInputLine::GetOutputDirectory() const
{
    return pimpl->outputDirectory;
}

void WebAppInputLine::SetAuxDirectory(const PathName& path)
{
    if (pimpl->auxDirectory == path)
    {
        return;
    }
    LogInfo("setting aux directory: " + path.ToString());
    pimpl->auxDirectory = path;
}

PathName WebAppInputLine::GetAuxDirectory() const
{
    return pimpl->auxDirectory;
}

PathName WebAppInputLine::GetFoundFile() const
{
    return pimpl->foundFile;
}

PathName WebAppInputLine::GetFoundFileFq() const
{
    return pimpl->foundFileFq;
}

void WebAppInputLine::EnableShellCommands(ShellCommandMode mode)
{
    if (mode == pimpl->shellCommandMode)
    {
        return;
    }
    auto session = GetSession();
    switch (mode)
    {
    case ShellCommandMode::Forbidden:
        LogInfo("disabling shell commands");
        break;
    case ShellCommandMode::Restricted:
        LogInfo("allowing known shell commands");
        break;
    case ShellCommandMode::Unrestricted:
        if (session->RunningAsAdministrator() && !session->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_ALLOW_UNRESTRICTED_SUPER_USER).GetBool())
        {
            LogError("unrestricted shell commands not allowed when running with elevated privileges");
            return;
        }
        LogInfo("allowing all shell commands");
        break;
    default:
        break;
    }
    pimpl->shellCommandMode = mode;
}

ShellCommandMode WebAppInputLine::GetShellCommandMode() const
{
    return pimpl->shellCommandMode;
}

PathName WebAppInputLine::GetLastInputFileName() const
{
    return pimpl->lastInputFileName;
}

void WebAppInputLine::SetInputOutput(IInputOutput* inputOutput)
{
    pimpl->inputOutput = inputOutput;
}

IInputOutput* WebAppInputLine::GetInputOutput() const
{
    return pimpl->inputOutput;
}

void WebAppInputLine::BufferSizeExceeded() const
{
    if (GetFormatIdent() == 0)
    {
        LogError("buffer size exceeded");
        throw new C4P::Exception9999;
    }
    else
    {
        IInputOutput* inputOutput = GetInputOutput();
        inputOutput->loc() = inputOutput->first();
        inputOutput->limit() = inputOutput->last() - 1;
        inputOutput->overflow(256, inputOutput->bufsize());
    }
}

size_t WebAppInputLine::InputLineInternal(FILE* f, char* buffer, char* buffer2, size_t bufferSize, size_t bufferPosition, int& lastChar) const
{
    MIKTEX_ASSERT(buffer2 == nullptr);
    do
    {
        errno = 0;
        while (bufferPosition < bufferSize && (lastChar = GetC(f)) != EOF && lastChar != '\n' && lastChar != '\r')
        {
            buffer[bufferPosition++] = lastChar;
        }
    } while (lastChar == EOF && errno == EINTR);
    return bufferPosition;
}

/**
 * @brief Read a line of input.
 *
 * This is a refactored implementation derived from TL's input_line function (see texmfmp.c).
 *
 * @param f
 * @param bypassEndOfLine
 * @return true
 * @return false
 */
bool WebAppInputLine::InputLine(C4P::C4P_text& f, C4P::C4P_boolean bypassEndOfLine) const
{
    f.AssertValid();

    if (f.IsPascalFileIO())
    {
        // this seems to be console input
    }

    IInputOutput* inputOutput = GetInputOutput();

    const auto first = inputOutput->first();
    auto& last = inputOutput->last();
    auto bufsize = inputOutput->bufsize();
    const char* xord = GetCharacterConverter()->xord();
    char* buffer = inputOutput->buffer();
    char* buffer2 = inputOutput->buffer2();
    int lastChar = EOF;

    last = static_cast<C4P::C4P_signed32>(InputLineInternal(f, buffer, buffer2, bufsize, first, lastChar));

    if (lastChar == EOF && last == first)
    {
        return false;
    }

    if (lastChar != EOF && lastChar != '\n' && lastChar != '\r')
    {
        MIKTEX_FATAL_ERROR("Unable to read an entire line.");
    }

    buffer[last] = ' ';

    if (last >= inputOutput->maxbufstack())
    {
        inputOutput->maxbufstack() = last;
    }

    if (lastChar == '\r')
    {
        while ((lastChar = GetC(f)) == EOF && errno == EINTR)
        {
        }
        if (lastChar != '\n')
        {
            ungetc(lastChar, f);
        }
    }

    while (last > first && buffer[last - 1] == ' ')
    {
        last--;
    }

    for (int i = first; i <= last; i++)
    {
        buffer[i] = xord[buffer[i] & 0xff];
    }

    if (AmI(TeXjpEngine))
    {
        for (int i = last + 1; (i < last + 5 && i < bufsize); i++)
        {
            buffer[i] = 0;
        }
    }

    return true;
}

FILE* WebAppInputLine::OpenFileInternal(const PathName& path, FileMode mode, FileAccess access)
{
    return GetSession()->OpenFile(path, mode, access, false);
}

FILE* WebAppInputLine::TryOpenFileInternal(const PathName& path, FileMode mode, FileAccess access)
{
    return GetSession()->TryOpenFile(path, mode, access, false);
}

void WebAppInputLine::CloseFileInternal(FILE* f)
{
    GetSession()->CloseFile(f);
}
