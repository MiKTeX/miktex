/**
 * @file miktex/TeXAndFriends/WebAppInputLine.h
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

#pragma once

#include <miktex/TeXAndFriends/config.h>

#include <memory>
#include <string>
#include <iostream>

#include <miktex/Core/BufferSizes>
#include <miktex/Core/File>
#include <miktex/Core/FileType>
#include <miktex/Util/PathName>

#include <miktex/Util/StringUtil>

#include "WebApp.h"

#define MIKTEX_TEXMF_BEGIN_NAMESPACE        \
namespace MiKTeX {                          \
    namespace TeXAndFriends {

#define MIKTEX_TEXMF_END_NAMESPACE          \
    }                                       \
}

MIKTEX_TEXMF_BEGIN_NAMESPACE;

class ICharacterConverter
{
public:
    virtual char* xchr() = 0;
    virtual char16_t* xchr16() = 0;
    virtual char* xord() = 0;
    virtual char* xprn() = 0;
};

class IInputOutput
{
public:
    virtual C4P::C4P_signed32& loc() = 0;
    virtual C4P::C4P_signed32& limit() = 0;
    virtual C4P::C4P_signed32 first() = 0;
    virtual C4P::C4P_signed32& last() = 0;
    virtual C4P::C4P_signed32 bufsize() = 0;
    virtual char*& nameoffile() = 0;
    virtual C4P::C4P_signed32& namelength() = 0;
    virtual char* buffer() = 0;
    virtual char* buffer2() = 0;
    virtual char32_t* buffer32() = 0;
    virtual C4P::C4P_signed32& maxbufstack() = 0;
    virtual void overflow(C4P::C4P_signed32 s, C4P::C4P_integer n) = 0;
};

class MIKTEXMFTYPEAPI(WebAppInputLine) :
    public WebApp
{

public:

    MIKTEXMFEXPORT MIKTEXTHISCALL WebAppInputLine();
    WebAppInputLine(const WebAppInputLine& other) = delete;
    WebAppInputLine& operator=(const WebAppInputLine& other) = delete;
    WebAppInputLine(WebAppInputLine&& other) = delete;
    WebAppInputLine& operator=(WebAppInputLine&& other) = delete;
    virtual MIKTEXMFEXPORT MIKTEXTHISCALL ~WebAppInputLine() noexcept;

    MIKTEXMFTHISAPI(ICharacterConverter*) GetCharacterConverter() const;
    MIKTEXMFTHISAPI(IInputOutput*) GetInputOutput() const;
    MIKTEXMFTHISAPI(MiKTeX::Util::PathName) GetAuxDirectory() const;
    MIKTEXMFTHISAPI(MiKTeX::Util::PathName) GetFoundFile() const;
    MIKTEXMFTHISAPI(MiKTeX::Util::PathName) GetFoundFileFq() const;
    MIKTEXMFTHISAPI(MiKTeX::Util::PathName) GetOutputDirectory() const;
    MIKTEXMFTHISAPI(bool) AllowFileName(const MiKTeX::Util::PathName & fileName, bool forInput);
    MIKTEXMFTHISAPI(bool) InputLine(C4P::C4P_text & f, C4P::C4P_boolean bypassEndOfLine) const;
    MIKTEXMFTHISAPI(bool) OpenInputFile(C4P::FileRoot & f, const MiKTeX::Util::PathName & fileNameInternalEncoding);
    MIKTEXMFTHISAPI(bool) OpenInputFile(FILE * *ppFile, const MiKTeX::Util::PathName & fileNameInternalEncoding);
    MIKTEXMFTHISAPI(bool) OpenOutputFile(C4P::FileRoot & f, const MiKTeX::Util::PathName & fileNameInternalEncoding, bool isTextFile_deprecated, MiKTeX::Util::PathName & outPath);
    MIKTEXMFTHISAPI(void) CloseFile(C4P::FileRoot & f);
    MIKTEXMFTHISAPI(void) Finalize() override;
    MIKTEXMFTHISAPI(void) Init(std::vector<char*>&args) override;
    MIKTEXMFTHISAPI(void) SetAuxDirectory(const MiKTeX::Util::PathName & path);
    MIKTEXMFTHISAPI(void) SetCharacterConverter(ICharacterConverter* characterConverter);
    MIKTEXMFTHISAPI(void) SetInputOutput(IInputOutput * inputOutput);
    MIKTEXMFTHISAPI(void) SetOutputDirectory(const MiKTeX::Util::PathName & path);
    virtual MIKTEXMFTHISAPI(FILE*) OpenFileInternal(const MiKTeX::Util::PathName & path, MiKTeX::Core::FileMode mode, MiKTeX::Core::FileAccess access);
    virtual MIKTEXMFTHISAPI(FILE*) TryOpenFileInternal(const MiKTeX::Util::PathName & path, MiKTeX::Core::FileMode mode, MiKTeX::Core::FileAccess access);
    virtual MIKTEXMFTHISAPI(MiKTeX::Util::PathName) DecodeFileName(const MiKTeX::Util::PathName & fileNameInternalEncoding);
    virtual MIKTEXMFTHISAPI(size_t) InputLineInternal(FILE * f, char* buffer, char* buffer2, size_t bufferSize, size_t bufferPosition, int& lastChar) const;
    virtual MIKTEXMFTHISAPI(void) CloseFileInternal(FILE * f);

    virtual int GetFormatIdent() const
    {
        MIKTEX_UNEXPECTED();
    }

    MiKTeX::Util::PathName GetNameOfFile() const
    {
        IInputOutput* inputOutput = GetInputOutput();
        return MiKTeX::Util::PathName(inputOutput->nameoffile());
    }

    virtual void SetNameOfFile(const MiKTeX::Util::PathName & fileName)
    {
        IInputOutput* inputOutput = GetInputOutput();
        MiKTeX::Util::StringUtil::CopyString(inputOutput->nameoffile(), MiKTeX::Core::BufferSizes::MaxPath + 1, fileName.GetData());
        inputOutput->namelength() = static_cast<C4P::C4P_signed16>(fileName.GetLength());
    }

    static WebAppInputLine* GetWebAppInputLine()
    {
        return dynamic_cast<WebAppInputLine*>(Application::GetApplication());
    }

protected:

    MIKTEXMFTHISAPI(MiKTeX::Core::ShellCommandMode) GetShellCommandMode() const;
    MIKTEXMFTHISAPI(MiKTeX::Util::PathName) GetLastInputFileName() const;
    MIKTEXMFTHISAPI(bool) ProcessOption(int opt, const std::string& optArg) override;
    MIKTEXMFTHISAPI(void) AddOptions() override;
    MIKTEXMFTHISAPI(void) EnableShellCommands(MiKTeX::Core::ShellCommandMode mode);
    virtual MIKTEXMFTHISAPI(void) TouchJobOutputFile(FILE*) const;

private:

    virtual MIKTEXMFTHISAPI(void) BufferSizeExceeded() const;

    class impl;
    std::unique_ptr<impl> pimpl;
};

template<class FileType> inline bool inputln(FileType& f, C4P::C4P_boolean bypassEndOfLine = true)
{
    return WebAppInputLine::GetWebAppInputLine()->InputLine(f, bypassEndOfLine);
}

template<class FileType> inline void miktexclosefile(FileType& f)
{
    WebAppInputLine::GetWebAppInputLine()->CloseFile(f);
}

template<class FileType> inline bool miktexopeninputfile(FileType& f)
{
    bool done = WebAppInputLine::GetWebAppInputLine()->OpenInputFile(*static_cast<C4P::FileRoot*>(&f), WebAppInputLine::GetWebAppInputLine()->GetNameOfFile());
    if (done)
    {
        WebAppInputLine::GetWebAppInputLine()->SetNameOfFile(WebAppInputLine::GetWebAppInputLine()->GetFoundFileFq());
    }
    return done;
}

inline bool miktexallownameoffile(C4P::C4P_boolean forInput)
{
    return WebAppInputLine::GetWebAppInputLine()->AllowFileName(WebAppInputLine::GetWebAppInputLine()->GetNameOfFile(), forInput);
}

template<class FileType> inline bool miktexopenoutputfile(FileType& f, C4P::C4P_boolean isTextFile_deprecated)
{
    // must open with read/write sharing flags
    // cf. bug 2006511
    MiKTeX::Util::PathName outPath;
    bool done = WebAppInputLine::GetWebAppInputLine()->OpenOutputFile(*static_cast<C4P::FileRoot*>(&f), WebAppInputLine::GetWebAppInputLine()->GetNameOfFile(), isTextFile_deprecated, outPath);
    if (done)
    {
        WebAppInputLine::GetWebAppInputLine()->SetNameOfFile(outPath);
    }
    return done;
}

MIKTEX_TEXMF_END_NAMESPACE;
