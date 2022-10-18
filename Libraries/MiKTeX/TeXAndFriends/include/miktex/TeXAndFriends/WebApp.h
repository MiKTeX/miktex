/**
 * @file miktex/TeXAndFriends/WebApp.h
 * @author Christian Schenk
 * @brief MiKTeX WebApp base implementation
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

#include "Prototypes.h"

#include <exception>
#include <memory>
#include <string>
#include <vector>

#include <miktex/C4P/C4P>

#include <miktex/Core/Exceptions>
#include <miktex/Core/FileType>
#include <miktex/Util/OptionSet>
#include <miktex/Util/PathName>
#include <miktex/Core/Utils>

#include <miktex/Util/StringUtil>

#include <miktex/Wrappers/PoptWrapper>

/// @cond
namespace C4P
{
    class Exception31 {};        // aux_done
    class Exception32 {};        // bst_done
    class Exception9932 {};      // no_bst_file
    class Exception9998 {};      // end_of_TEX
    class Exception9999 {};      // final_end
}
/// @endcond

MIKTEX_TEXMF_BEGIN_NAMESPACE;

constexpr const char* TeXEngine = "TeXEngine";
constexpr const char* METAFONTEngine = "METAFONTEngine";
constexpr const char* TeXjpEngine = "TeXjpEngine";

class IInitFinalize
{
public:
    virtual C4P::C4P_signed8& history() = 0;
};

template<class FileType> inline bool miktexopentfmfile(FileType& f, const char* fileName)
{
    return OpenTFMFile(&f, MiKTeX::Util::PathName(fileName));
}

template<class FileType> inline bool miktexopenvffile(FileType& f, const char* fileName)
{
    return OpenVFFile(&f, MiKTeX::Util::PathName(fileName));
}

template<class FileType> inline int miktexopenxfmfile(FileType& f, const char* fileName)
{
    return OpenXFMFile(&f, MiKTeX::Util::PathName(fileName));
}

template<class FileType> inline bool miktexopenxvffile(FileType& f, const char* fileName)
{
    return OpenXVFFile(&f, MiKTeX::Util::PathName(fileName));
}

template<class FileType> inline void miktexprintmiktexbanner(FileType& f)
{
    fprintf(f, " (%s)", MiKTeX::Core::Utils::GetMiKTeXBannerString().c_str());
}

class MIKTEXMFTYPEAPI(WebApp) :
    public MiKTeX::App::Application
{

public:

    MIKTEXMFEXPORT MIKTEXTHISCALL WebApp();
    WebApp(const WebApp & other) = delete;
    WebApp& operator=(const WebApp & other) = delete;
    WebApp(WebApp && other) = delete;
    WebApp& operator=(WebApp && other) = delete;
    virtual MIKTEXMFEXPORT MIKTEXTHISCALL ~WebApp() noexcept;

    MIKTEXMFTHISAPI(C4P::ProgramBase*) GetProgram() const;
    MIKTEXMFTHISAPI(IInitFinalize*) GetInitFinalize() const;
    MIKTEXMFTHISAPI(bool) AmI(const std::string& name) const;
    MIKTEXMFTHISAPI(bool) GetVerboseFlag() const;
    MIKTEXMFTHISAPI(void) IAm(const std::string& name);
    MIKTEXMFTHISAPI(std::string) GetProgramName() const;
    MIKTEXMFTHISAPI(void) Finalize() override;
    MIKTEXMFTHISAPI(void) Init(std::vector<char*>&args) override;
    MIKTEXMFTHISAPI(void) SetInitFinalize(IInitFinalize* initFinalize);
    MIKTEXMFTHISAPI(void) SetProgram(C4P::ProgramBase* program, const std::string& programName, const std::string& version, const std::string& copyright, const std::string& trademarks);
    virtual MIKTEXMFTHISAPI(std::string) TheNameOfTheGame() const;
    virtual MIKTEXMFTHISAPI(void) ProcessCommandLineOptions();

    virtual unsigned long GetHelpId() const
    {
        return 0;
    }

    virtual MiKTeX::Core::FileType GetInputFileType() const
    {
        // must be implemented in sub-classes
        MIKTEX_UNEXPECTED();
    }

    static WebApp* GetWebApp()
    {
        MIKTEX_ASSERT(dynamic_cast<WebApp*>(Application::GetApplication()) != nullptr);
        return reinterpret_cast<WebApp*>(Application::GetApplication());
    }

protected:

    enum
    {
        OPT_UNSUPPORTED = INT_MAX - 100,
        OPT_NOOP,
    };

    MIKTEXMFTHISAPI(std::string) Translate(const char* msgId);
    MIKTEXMFTHISAPI(std::vector<poptOption>) GetOptions() const;
    MIKTEXMFTHISAPI(void) AddOption(const std::string& aliasName, const std::string& name);
    MIKTEXMFTHISAPI(void) AddOption(const std::string& name, const std::string& help, int opt, int argInfo = POPT_ARG_NONE, const std::string& argDescription = "", void* arg = nullptr, char shortName = 0);
    MIKTEXMFTHISAPI(void) AddOptionShortcut(const std::string& longName, const std::vector<std::string>& args);
    MIKTEXMFTHISAPI(void) BadUsage();
    MIKTEXMFTHISAPI(void) ShowHelp(bool usageOnly = false) const;
    MIKTEXMFTHISAPI(void) ShowProgramVersion() const;
    virtual MIKTEXMFTHISAPI(bool) ProcessOption(int opt, const std::string& optArg);
    virtual MIKTEXMFTHISAPI(void) AddOptions();

    void AddOption(const std::string& name, int opt, int argInfo = POPT_ARG_NONE, const std::string& argDescription = "", void* arg = nullptr, char shortName = 0)
    {
        AddOption(name, "", opt, argInfo, argDescription, arg, shortName);
    }

    virtual std::string GetUsage() const
    {
        // must be implemented in sub-classes
        MIKTEX_UNEXPECTED();
    }

private:

    class impl;
    std::unique_ptr<impl> pimpl;
};

inline bool miktexgetverboseflag()
{
    return WebApp::GetWebApp()->GetVerboseFlag();
}

inline bool miktexgetquietflag()
{
    return WebApp::GetWebApp()->GetQuietFlag();
}

inline void miktexprocesscommandlineoptions()
{
    WebApp::GetWebApp()->ProcessCommandLineOptions();
}

template<class PROGRAM_CLASS, class WEBAPP_CLASS> class ProgramRunner
{

public:

    int Run(PROGRAM_CLASS& prog, WEBAPP_CLASS& app, const std::string& progName, int argc, char* argv[])
    {
        std::string componentVersion;
#if defined(MIKTEX_COMPONENT_VERSION_STR)
        componentVersion = MIKTEX_COMPONENT_VERSION_STR;
#endif
        std::string componentCopyright;
#if defined(MIKTEX_COMP_COPYRIGHT_STR)
        componentCopyright = MIKTEX_COMP_COPYRIGHT_STR;
#endif
        std::string componentTrademark;
#if defined(MIKTEX_COMP_TM_STR)
        componentTrademark = MIKTEX_COMP_TM_STR;
#endif
        app.SetProgram(&prog, progName, componentVersion, componentCopyright, componentTrademark);
        prog.SetParent(&app);
        try
        {
            MIKTEX_ASSERT(argv != nullptr && argv[argc] == nullptr);
            std::vector<char*> newargv(argv, argv + argc + 1);
            app.Init(newargv);
            MIKTEX_ASSERT(!newargv.empty() && newargv.back() == nullptr);
            int exitCode = prog.Run(newargv.size() - 1, &newargv[0]);
            app.Finalize2(exitCode);
            return exitCode;
        }
        catch (const MiKTeX::Core::MiKTeXException& ex)
        {
            app.Sorry(argv[0], ex);
            app.Finalize2(1);
            ex.Save();
            return EXIT_FAILURE;
        }
        catch (const std::exception& ex)
        {
            app.Sorry(argv[0], ex);
            app.Finalize2(1);
            return EXIT_FAILURE;
        }
        catch (int exitCode)
        {
            app.Finalize2(exitCode);
            return exitCode;
        }
    }
};

// TODO: MIKTEX_DEFINE_WEBAPP(programname, functionname, webappclass, webappinstance, programclass, programinstance)
#define MIKTEX_DEFINE_WEBAPP(functionname, webappclass, webappinstance, programclass, programinstance)  \
programclass programinstance;                                                                           \
webappclass webappinstance;                                                                             \
extern "C" int MIKTEXCEECALL functionname(int argc, char* argv[])                                       \
{                                                                                                       \
    MiKTeX::TeXAndFriends::ProgramRunner<programclass, webappclass> p;                                  \
    std::string programName = #webappclass;                                                             \
    return p.Run(programinstance, webappinstance, programName, argc, argv);                             \
}

MIKTEX_TEXMF_END_NAMESPACE;
