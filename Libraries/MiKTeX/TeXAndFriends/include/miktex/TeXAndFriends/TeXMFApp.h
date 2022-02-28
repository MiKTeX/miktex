/**
 * @file miktex/TeXAndFriends/TeXMFApp.h
 * @author Christian Schenk
 * @brief MiKTeX TeXMF base implementation
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

#include <cstddef>

#include <miktex/Core/BufferSizes>
#include <miktex/Core/Debug>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/FileType>
#include <miktex/Util/PathName>
#include <miktex/Core/Quoter>

#include <miktex/Trace/TraceStream>

#include <miktex/Util/CharBuffer>
#include <miktex/Util/StringUtil>
#include <miktex/Util/inliners.h>

#include "WebAppInputLine.h"

MIKTEX_TEXMF_BEGIN_NAMESPACE;

class IStringHandler
{
public:
    virtual char* strpool() = 0;
    virtual char16_t* strpool16() = 0;
    virtual C4P::C4P_signed32& strptr() = 0;
    virtual C4P::C4P_signed32* strstart() = 0;
    virtual C4P::C4P_signed32& poolsize() = 0;
    virtual C4P::C4P_signed32& poolptr() = 0;
    virtual C4P::C4P_signed32 makestring() = 0;
};

class IErrorHandler
{
public:
    virtual C4P::C4P_integer& interrupt() = 0;
};

class ITeXMFMemoryHandler
{
public:
    virtual void Allocate(const std::unordered_map<std::string, int>& userParams) = 0;
    virtual void Free() = 0;
    virtual void Check() = 0;
    virtual void* ReallocateArray(const std::string& arrayName, void* ptr, std::size_t elemSize, std::size_t numElem, const MiKTeX::Core::SourceLocation& sourceLocation) = 0;
};

class MIKTEXMFTYPEAPI(TeXMFApp) :
    public WebAppInputLine
{

public:
  
    MIKTEXMFEXPORT MIKTEXTHISCALL TeXMFApp();
    TeXMFApp(const TeXMFApp& other) = delete;
    TeXMFApp& operator=(const TeXMFApp& other) = delete;
    TeXMFApp(TeXMFApp&& other) = delete;
    TeXMFApp& operator=(TeXMFApp&& other) = delete;
    virtual MIKTEXMFEXPORT MIKTEXTHISCALL ~TeXMFApp() noexcept;

    typedef std::unordered_map<std::string, int> UserParams;

    MIKTEXMFTHISAPI(IErrorHandler*) GetErrorHandler() const;
    MIKTEXMFTHISAPI(IStringHandler*) GetStringHandler() const;
    MIKTEXMFTHISAPI(ITeXMFMemoryHandler*) GetTeXMFMemoryHandler() const;
    MIKTEXMFTHISAPI(MiKTeX::Util::PathName) GetDefaultMemoryDumpFileName() const;
    MIKTEXMFTHISAPI(UserParams&) GetUserParams() const;
    MIKTEXMFTHISAPI(bool) CStyleErrorMessagesP() const;
    MIKTEXMFTHISAPI(bool) HaltOnErrorP() const;
    MIKTEXMFTHISAPI(bool) IsInitProgram() const;
    MIKTEXMFTHISAPI(bool) IsUnicodeApp() const;
    MIKTEXMFTHISAPI(bool) OpenFontFile(C4P::BufferedFile<unsigned char>* file, const std::string& fontName, MiKTeX::Core::FileType filetype, const char* generator);
    MIKTEXMFTHISAPI(bool) OpenMemoryDumpFile(const MiKTeX::Util::PathName& fileName, FILE** file, void* buf, std::size_t size, bool renew);
    MIKTEXMFTHISAPI(bool) ParseFirstLineP() const;
    MIKTEXMFTHISAPI(int) GetInteraction() const;
    MIKTEXMFTHISAPI(int) GetJobName(int fallbackJobName) const;
    MIKTEXMFTHISAPI(int) GetTeXStringLength(int stringNumber) const;
    MIKTEXMFTHISAPI(int) GetTeXStringStart(int stringNumber) const;
    MIKTEXMFTHISAPI(int) MakeTeXString(const char* lpsz) const;
    MIKTEXMFTHISAPI(std::string) GetTeXString(int stringStart, int stringLength) const;
    MIKTEXMFTHISAPI(void) AddOptions() override;
    MIKTEXMFTHISAPI(void) Finalize() override;
    MIKTEXMFTHISAPI(void) Init(std::vector<char*>& args) override;
    MIKTEXMFTHISAPI(void) InitializeBuffer() const;
    MIKTEXMFTHISAPI(void) InvokeEditor(int editFileName, int editFileNameLength, int editLineNumber, int transcriptFileName, int transcriptFileNameLength) const;
    MIKTEXMFTHISAPI(void) ProcessCommandLineOptions() override;
    MIKTEXMFTHISAPI(void) SetErrorHandler(IErrorHandler* errorHandler);
    MIKTEXMFTHISAPI(void) SetStringHandler(IStringHandler* stringHandler);
    MIKTEXMFTHISAPI(void) SetTeXMFMemoryHandler(ITeXMFMemoryHandler* memoryHandler);
    MIKTEXMFTHISAPI(void) TouchJobOutputFile(FILE* file) const override;
    virtual MIKTEXMFTHISAPI(void) OnTeXMFFinishJob();
    virtual MIKTEXMFTHISAPI(void) OnTeXMFStartJob();

    virtual std::string GetMemoryDumpFileExtension() const
    {
        // must be implemented in sub-classes
        MIKTEX_UNEXPECTED();
    }

    virtual MiKTeX::Core::FileType GetMemoryDumpFileType() const
    {
        // must be implemented in sub-classes
        MIKTEX_UNEXPECTED();
    }

    void SetNameOfFile(const MiKTeX::Util::PathName& fileName) override
    {
        IInputOutput* inputOutput = GetInputOutput();
        ITeXMFMemoryHandler* texmfMemoryHandler = GetTeXMFMemoryHandler();
        inputOutput->nameoffile() = reinterpret_cast<char*>(texmfMemoryHandler->ReallocateArray("nameoffile", inputOutput->nameoffile(), sizeof(inputOutput->nameoffile()[0]), fileName.GetLength() + 1, MIKTEX_SOURCE_LOCATION()));
        MiKTeX::Util::StringUtil::CopyString(inputOutput->nameoffile(), fileName.GetLength() + 1, fileName.GetData());
        inputOutput->namelength() = static_cast<C4P::C4P_signed32>(fileName.GetLength());
    }

    virtual void OnTeXMFInitialize() const
    {
        signal(SIGINT, OnKeybordInterrupt);
    }


#if defined(MIKTEX_DEBUG)
    virtual void CheckMemory()
    {
        GetTeXMFMemoryHandler()->Check();
    }
#endif

    virtual void AllocateMemory()
    {
        GetTeXMFMemoryHandler()->Allocate(GetUserParams());
    }

    virtual void FreeMemory()
    {
        GetTeXMFMemoryHandler()->Free();
    }

    std::string GetTeXString(int stringNumber) const
    {
        return GetTeXString(GetTeXStringStart(stringNumber), GetTeXStringLength(stringNumber));
    }

    void InvokeEditor(const MiKTeX::Util::PathName& editFileName, int editLineNumber, const MiKTeX::Util::PathName& transcriptFileName) const
    {
        Application::InvokeEditor(editFileName, editLineNumber, GetInputFileType(), transcriptFileName);
    }

    int MakeFullNameString()
    {
        return MakeTeXString(GetFoundFile().GetData());
    }

    template<class T> bool OpenMemoryDumpFile(T& f, bool renew = false)
    {
        FILE* file;
        if (!OpenMemoryDumpFile(GetNameOfFile(), &file, nullptr, sizeof(*f), renew))
        {
        return false;
        }
        f.Attach(file, true);
        f.PascalFileIO(false);
        return true;
    }

    template<typename FILE_, typename ELETYPE_> void Dump(FILE_& f, const ELETYPE_& e, std::size_t n)
    {
        if (fwrite(&e, sizeof(e), n, static_cast<FILE*>(f)) != n)
        {
        MIKTEX_FATAL_CRT_ERROR("fwrite");
        }
    }

    template<typename FILE_, typename ELETYPE_> void Dump(FILE_& f, const ELETYPE_& e)
    {
        Dump(f, e, 1);
    }

    template<typename FILE_, typename ELETYPE_> void Undump(FILE_& f, ELETYPE_& e, std::size_t n)
    {
        f.PascalFileIO(false);
        if (fread(&e, sizeof(e), n, static_cast<FILE*>(f)) != n)
        {
        MIKTEX_FATAL_CRT_ERROR("fread");
        }
    }

    template<typename FILE_, typename ELETYPE_> void Undump(FILE_& f, ELETYPE_& e)
    {
        Undump(f, e, 1);
    }

    template<typename FILE_, typename ELETYPE_> void Undump(FILE_& f, ELETYPE_ low, ELETYPE_ high, ELETYPE_& e, std::size_t n)
    {
        Undump(f, e, n);
        for (std::size_t idx = 0; idx < n; ++idx)
        {
        if ((&e)[idx] < low || (&e)[idx] > high)
        {
            MIKTEX_FATAL_ERROR(MIKTEXTEXT("Bad format file."));
        }
        }
    }

    template<typename FILE_, typename ELETYPE_> void Undump(FILE_ &f, ELETYPE_ high, ELETYPE_& e, std::size_t n)
    {
        Undump(f, e, n);
        for (std::size_t idx = 0; idx < n; ++idx)
        {
        if ((&e)[idx] > high)
        {
            MIKTEX_FATAL_ERROR(MIKTEXTEXT("Bad format file."));
        }
        }
    }

    static MIKTEXMFCEEAPI(MiKTeX::Core::Argv) ParseFirstLine(const MiKTeX::Util::PathName& path);
    static MIKTEXMFCEEAPI(void) OnKeybordInterrupt(int);

    static TeXMFApp* GetTeXMFApp()
    {
        MIKTEX_ASSERT(dynamic_cast<TeXMFApp*>(Application::GetApplication()) != nullptr);
        return (TeXMFApp*)Application::GetApplication();
    }

protected:

    MIKTEXMFTHISAPI(bool) ProcessOption(int opt, const std::string& optArg) override;

    std::string GetUsage() const override
    {
        return MIKTEXTEXT("[OPTION...] [COMMAND...]");
    }

    virtual MiKTeX::Util::PathName GetMemoryDumpFileName() const
    {
        // must be implemented in sub-classes
        MIKTEX_UNEXPECTED();
    }

    virtual std::string GetInitProgramName() const
    {
        // must be implemented in sub-classes
        MIKTEX_UNEXPECTED();
    }

    virtual std::string GetVirginProgramName() const
    {
        // must be implemented in sub-classes
        MIKTEX_UNEXPECTED();
    }

    MIKTEXMFTHISAPI(void) CheckPoolPointer(int poolptr, std::size_t len) const;
    MIKTEXMFTHISAPI(bool) IsVirgin() const;

private:

    MIKTEXMFTHISAPI(void) CheckFirstLine(const MiKTeX::Util::PathName& fileName);

private:

    class impl;
    std::unique_ptr<impl> pimpl;
};

inline bool miktexcstyleerrormessagesp()
{
    return TeXMFApp::GetTeXMFApp()->CStyleErrorMessagesP();
}

inline bool miktexparsefirstlinep()
{
    return TeXMFApp::GetTeXMFApp()->ParseFirstLineP();
}

inline void miktexgetdefaultdumpfilename(char* dest)
{
    MiKTeX::Util::StringUtil::CopyString(dest, MiKTeX::Core::BufferSizes::MaxPath, TeXMFApp::GetTeXMFApp()->GetDefaultMemoryDumpFileName().GetData());
}

inline int miktexgetinteraction()
{
    return TeXMFApp::GetTeXMFApp()->GetInteraction();
}

inline int miktexgetjobname(int fallbackJobName)
{
    return TeXMFApp::GetTeXMFApp()->GetJobName(fallbackJobName);
}

inline bool miktexhaltonerrorp()
{
    return TeXMFApp::GetTeXMFApp()->HaltOnErrorP();
}

inline void miktexinitializebuffer()
{
    TeXMFApp::GetTeXMFApp()->InitializeBuffer();
}

inline void miktexinvokeeditor(int editFileName, int editFileNameLength, int editLineNumber, int transcriptFileName, int transcriptFileNameLength)
{
    TeXMFApp::GetTeXMFApp()->InvokeEditor(editFileName, editFileNameLength, editLineNumber, transcriptFileName, transcriptFileNameLength);
}

inline void miktexinvokeeditor(int editFileName, int editFileNameLength, int editLineNumber)
{
    TeXMFApp::GetTeXMFApp()->InvokeEditor(editFileName, editFileNameLength, editLineNumber, 0, 0);
}

inline bool miktexisinitprogram()
{
    return TeXMFApp::GetTeXMFApp()->IsInitProgram();
}

inline int miktexmakefullnamestring()
{
    return TeXMFApp::GetTeXMFApp()->MakeFullNameString();
}

inline void miktexontexmffinishjob()
{
    TeXMFApp::GetTeXMFApp()->OnTeXMFFinishJob();
}

inline void miktexontexmfinitialize()
{
    TeXMFApp::GetTeXMFApp()->OnTeXMFInitialize();
}

inline void miktexontexmfstartjob()
{
    TeXMFApp::GetTeXMFApp()->OnTeXMFStartJob();
}

#define miktexreallocate(p, n) miktexreallocate_(#p, p, n, MIKTEX_SOURCE_LOCATION_DEBUG())

template<typename T> T* miktexreallocate_(const std::string& arrayName, T* p, size_t n, const MiKTeX::Core::SourceLocation& sourceLocation)
{
    return (T*)TeXMFApp::GetTeXMFApp()->GetTeXMFMemoryHandler()->ReallocateArray(arrayName, p, sizeof(*p), n, sourceLocation);
}

template<typename FileType, typename EleType> inline void miktexdump(FileType& f, const EleType& e, std::size_t n)
{
    TeXMFApp::GetTeXMFApp()->Dump(f, e, n);
}

template<typename FileType, typename EleType> inline void miktexdump(FileType& f, const EleType& e)
{
    TeXMFApp::GetTeXMFApp()->Dump(f, e);
}

template<typename FileType> inline void miktexdumpint(FileType& f, int val)
{
    miktexdump(f, val);
}

template<typename FileType, typename EleType> inline void miktexundump(FileType& f, EleType& e, std::size_t n)
{
    TeXMFApp::GetTeXMFApp()->Undump(f, e, n);
}

template<typename FileType, typename EleType> inline void miktexundump(FileType& f, EleType& e)
{
    TeXMFApp::GetTeXMFApp()->Undump(f, e);
}

template<typename FileType, typename LowType, typename HighType, typename EleType> inline void miktexundump(FileType& f, LowType low, HighType high, EleType& e, std::size_t n)
{
    TeXMFApp::GetTeXMFApp()->Undump(f, static_cast<EleType>(low), static_cast<EleType>(high), e, n);
}

template<typename FileType, typename HighType, typename EleType> inline void miktexundump(FileType& f, HighType high, EleType& e, std::size_t n)
{
    TeXMFApp::GetTeXMFApp()->Undump(f, static_cast<EleType>(high), e, n);
}

template<typename FileType> inline void miktexundumpint(FileType& f, int& val)
{
    miktexundump(f, val);
}

inline void miktexcheckmemoryifdebug()
{
#if defined(MIKTEX_DEBUG)
    TeXMFApp::GetTeXMFApp()->CheckMemory();
#endif
}

MIKTEX_TEXMF_END_NAMESPACE;
