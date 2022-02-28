/**
 * @file miktex/TeXAndFriends/TeXApp.h
 * @author Christian Schenk
 * @brief MiKTeX TeX base implementation
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

#include <cstddef>

#include <memory>
#include <string>

#include <miktex/Core/BufferSizes>
#include <miktex/Core/FileType>
#include <miktex/Util/PathName>

#include <miktex/Util/CharBuffer>
#include <miktex/Util/inliners.h>

#include "TeXMFApp.h"

MIKTEX_TEXMF_BEGIN_NAMESPACE;

class IFormatHandler
{
public:
    virtual C4P::C4P_signed32& formatident() = 0;
};

enum class SourceSpecial
{
    Auto,
    CarriageReturn,
    Display,
    HorizontalBox,
    Math,
    Paragraph,
    ParagraphEnd,
    VerticalBox,
};

class MIKTEXMFTYPEAPI(TeXApp) :
    public TeXMFApp
{

public:

    MIKTEXMFEXPORT MIKTEXTHISCALL TeXApp();
    TeXApp(const TeXApp& other) = delete;
    TeXApp& operator=(const TeXApp& other) = delete;
    TeXApp(TeXApp&& other) = delete;
    TeXApp& operator=(TeXApp&& other) = delete;
    virtual MIKTEXMFEXPORT MIKTEXTHISCALL ~TeXApp() noexcept;

    enum class Write18Result
    {
        QuotationError = -1,
        Disallowed = 0,
        Executed = 1,
        ExecutedAllowed = 2
    };

    MIKTEXMFTHISAPI(IFormatHandler*) GetFormatHandler() const;
    MIKTEXMFTHISAPI(MiKTeX::Core::ShellCommandMode) GetWrite18Mode() const;
    MIKTEXMFTHISAPI(Write18Result) Write18(const std::string& command, int& exitCode) const;
    MIKTEXMFTHISAPI(bool) EncTeXP() const;
    MIKTEXMFTHISAPI(bool) IsNewSource(int sourceFileName, int line) const;
    MIKTEXMFTHISAPI(bool) IsSourceSpecialOn(SourceSpecial s) const;
    MIKTEXMFTHISAPI(bool) MLTeXP() const;
    MIKTEXMFTHISAPI(bool) SourceSpecialsP() const;
    MIKTEXMFTHISAPI(bool) Write18P() const;
    MIKTEXMFTHISAPI(int) GetSynchronizationOptions() const;
    MIKTEXMFTHISAPI(int) MakeSrcSpecial(int sourceFileName, int line) const;
    MIKTEXMFTHISAPI(void) Finalize() override;
    MIKTEXMFTHISAPI(void) OnTeXMFStartJob() override;
    MIKTEXMFTHISAPI(void) RememberSourceInfo(int sourceFileName, int line) const;
    MIKTEXMFTHISAPI(void) SetFormatHandler(IFormatHandler* formatHandler);

    MiKTeX::Core::FileType GetInputFileType() const override
    {
        return MiKTeX::Core::FileType::TEX;
    }

    int GetFormatIdent() const override
    {
        return GetFormatHandler()->formatident();
    }

    std::string GetMemoryDumpFileExtension() const override
    {
        return ".fmt";
    }

    MiKTeX::Core::FileType GetMemoryDumpFileType() const override
    {
        return MiKTeX::Core::FileType::FMT;
    }

    static TeXApp* GetTeXApp()
    {
        MIKTEX_ASSERT(dynamic_cast<TeXApp*>(Application::GetApplication()) != nullptr);
        return (TeXApp*)Application::GetApplication();
    }

protected:

    MIKTEXMFTHISAPI(bool) ProcessOption(int c, const std::string& optArg) override;
    MIKTEXMFTHISAPI(void) AddOptions() override;
    MIKTEXMFTHISAPI(void) Init(std::vector<char*>& args) override;

private:

    class impl;
    std::unique_ptr<impl> pimpl;
};

inline void miktexallocatememory()
{
    TeXApp::GetTeXApp()->AllocateMemory();
}

template<class FileType> void miktexclosedvifile(FileType& f)
{
    TeXApp::GetTeXApp()->CloseFile(f);
}

template<class FileType> void miktexclosepdffile(FileType& f)
{
    TeXApp::GetTeXApp()->CloseFile(f);
}

inline void miktexfreememory()
{
    TeXApp::GetTeXApp()->FreeMemory();
}

inline bool miktexinsertsrcspecialauto()
{
    return TeXApp::GetTeXApp()->IsSourceSpecialOn(SourceSpecial::Auto);
}

inline bool miktexinsertsrcspecialeverycr()
{
    return TeXApp::GetTeXApp()->IsSourceSpecialOn(SourceSpecial::CarriageReturn);
}

inline bool miktexinsertsrcspecialeverydisplay()
{
    return TeXApp::GetTeXApp()->IsSourceSpecialOn(SourceSpecial::Display);
}

inline bool miktexinsertsrcspecialeveryhbox()
{
    return TeXApp::GetTeXApp()->IsSourceSpecialOn(SourceSpecial::HorizontalBox);
}

inline bool miktexinsertsrcspecialeverymath()
{
    return TeXApp::GetTeXApp()->IsSourceSpecialOn(SourceSpecial::Math);
}

inline bool miktexinsertsrcspecialeverypar()
{
    return TeXApp::GetTeXApp()->IsSourceSpecialOn(SourceSpecial::Paragraph);
}

inline bool miktexinsertsrcspecialeveryparend()
{
    return TeXApp::GetTeXApp()->IsSourceSpecialOn(SourceSpecial::ParagraphEnd);
}

inline bool miktexinsertsrcspecialeveryvbox()
{
    return TeXApp::GetTeXApp()->IsSourceSpecialOn(SourceSpecial::VerticalBox);
}

inline bool miktexisnewsource(int fileName, int lineNo)
{
    return TeXApp::GetTeXApp()->IsNewSource(fileName, lineNo);
}

inline int miktexmakesrcspecial(int fileName, int lineNo)
{
    return TeXApp::GetTeXApp()->MakeSrcSpecial(fileName, lineNo);
}

template<class FileType> inline bool miktexopendvifile(FileType& f)
{
    MiKTeX::Util::PathName outPath;
    bool done = TeXApp::GetTeXApp()->OpenOutputFile(*reinterpret_cast<C4P::FileRoot*>(&f), TeXApp::GetTeXApp()->GetNameOfFile(), false, outPath);
    if (done)
    {
        TeXApp::GetTeXApp()->SetNameOfFile(outPath);
    }
    return done;
}

template<class FileType> inline bool miktexopenpdffile(FileType& f)
{
    MiKTeX::Util::PathName outPath;
    bool done = TeXApp::GetTeXApp()->OpenOutputFile(*reinterpret_cast<C4P::FileRoot*>(&f), TeXApp::GetTeXApp()->GetNameOfFile(), false, outPath);
    if (done)
    {
        TeXApp::GetTeXApp()->SetNameOfFile(outPath);
    }
    return done;
}

template<class FileType> inline bool miktexopenformatfile(FileType& f, bool renew = false)
{
    return TeXApp::GetTeXApp()->OpenMemoryDumpFile(f, renew);
}

inline void miktexremembersourceinfo(int fileName, int lineNo)
{
    TeXApp::GetTeXApp()->RememberSourceInfo(fileName, lineNo);
}

inline bool miktexwrite18p()
{
    return TeXApp::GetTeXApp()->Write18P();
}

inline bool miktexenctexp()
{
    return TeXApp::GetTeXApp()->EncTeXP();
}

inline bool miktexmltexp()
{
    return TeXApp::GetTeXApp()->MLTeXP();
}

inline int miktexgetsynchronizationoptions()
{
    return TeXApp::GetTeXApp()->GetSynchronizationOptions();
}

inline bool miktexsourcespecialsp()
{
    return TeXApp::GetTeXApp()->SourceSpecialsP();
}

inline bool miktexiscompatible()
{
    return !TeXApp::GetTeXApp()->SourceSpecialsP() && !TeXApp::GetTeXApp()->CStyleErrorMessagesP() && !TeXApp::GetTeXApp()->ParseFirstLineP();
}

// web2c shim
inline bool restrictedshell()
{
    return TeXApp::GetTeXApp()->GetWrite18Mode() == MiKTeX::Core::ShellCommandMode::Restricted || TeXApp::GetTeXApp()->GetWrite18Mode() == MiKTeX::Core::ShellCommandMode::Query;
}

// web2c shim
inline bool shellenabledp()
{
    return miktexwrite18p();
}

// web2c shim
inline bool srcspecialsp()
{
    return TeXApp::GetTeXApp()->SourceSpecialsP();
}

MIKTEX_TEXMF_END_NAMESPACE;
