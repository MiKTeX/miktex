/* miktex/TeXAndFriends/WebApp.h:                       -*- C++ -*-

   Copyright (C) 1996-2020 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#pragma once

#if !defined(B21F1EB98F524CBDA35F0691A5BB8D38)
#define B21F1EB98F524CBDA35F0691A5BB8D38

#include <miktex/TeXAndFriends/config.h>

#include "Prototypes.h"

#include <exception>
#include <memory>
#include <string>
#include <vector>

#include <miktex/App/Application>

#include <miktex/C4P/C4P>

#include <miktex/Core/Exceptions>
#include <miktex/Core/FileType>
#include <miktex/Core/OptionSet>
#include <miktex/Core/PathName>
#include <miktex/Core/Utils>

#include <miktex/Util/StringUtil>

#include <miktex/Wrappers/PoptWrapper>

/// @cond
namespace C4P {
  class Exception31 {};        // aux_done
  class Exception32 {};        // bst_done
  class Exception9932 {};      // no_bst_file
  class Exception9998 {};      // end_of_TEX
  class Exception9999 {};      // final_end
}
/// @endcond

MIKTEX_TEXMF_BEGIN_NAMESPACE;

class ICharacterConverter
{
public:
  virtual char* xchr() = 0;
  virtual char* xord() = 0;
  virtual char* xprn() = 0;
};

class IInitFinalize
{
public:
  virtual C4P::C4P_signed8& history() = 0;
};

enum class Feature
{
  EightBitChars,
  TCX
};

template<class FileType> inline bool miktexopentfmfile(FileType& f, const char* fileName)
{
  return OpenTFMFile(&f, MiKTeX::Core::PathName(fileName));
}

template<class FileType> inline bool miktexopenvffile(FileType& f, const char* fileName)
{
  return OpenVFFile(&f, MiKTeX::Core::PathName(fileName));
}

template<class FileType> inline int miktexopenxfmfile(FileType& f, const char* fileName)
{
  return OpenXFMFile(&f, MiKTeX::Core::PathName(fileName));
}

template<class FileType> inline bool miktexopenxvffile(FileType& f, const char* fileName)
{
  return OpenXVFFile(&f, MiKTeX::Core::PathName(fileName));
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

public:
  WebApp(const WebApp& other) = delete;

public:
  WebApp& operator=(const WebApp& other) = delete;

public:
  WebApp(WebApp&& other) = delete;

public:
  WebApp& operator=(WebApp&& other) = delete;

public:
  virtual MIKTEXMFEXPORT MIKTEXTHISCALL ~WebApp() noexcept;

public:
  static WebApp* GetWebApp()
  {
    MIKTEX_ASSERT(dynamic_cast<WebApp*>(Application::GetApplication()) != nullptr);
    return reinterpret_cast<WebApp*>(Application::GetApplication());
  }

public:
  MIKTEXMFTHISAPI(void) Init(std::vector<char*>& args) override;

public:
  MIKTEXMFTHISAPI(void) Finalize() override;

public:
  MIKTEXMFTHISAPI(void) SetProgramInfo(const std::string& programName, const std::string& version, const std::string& copyright, const std::string& trademarks);

public:
  virtual MIKTEXMFTHISAPI(std::string) TheNameOfTheGame() const;

public:
  MIKTEXMFTHISAPI(std::string) GetProgramName() const;

public:
  MIKTEXMFTHISAPI(bool) AmI(const std::string& name) const;

protected:
  virtual MIKTEXMFTHISAPI(void) AddOptions();

protected:
  enum {
    OPT_UNSUPPORTED = INT_MAX - 100,
    OPT_NOOP,
  };

protected:
  MIKTEXMFTHISAPI(void) AddOption(const std::string& name, const std::string& help, int opt, int argInfo = POPT_ARG_NONE, const std::string& argDescription = "", void* arg = nullptr, char shortName = 0);

  // DEPRECATED
protected:
  void AddOption(const char* nameAndHelp, int opt, int argInfo = POPT_ARG_NONE, const std::string& argDescription = "", void * arg = nullptr, char shortName = 0)
  {
    AddOption(nameAndHelp, nameAndHelp + strlen(nameAndHelp) + 1, opt, argInfo, argDescription, arg, shortName);
  }

protected:
  MIKTEXMFTHISAPI(void) AddOption(const std::string& aliasName, const std::string& name);


protected:
  MIKTEXMFTHISAPI(void) AddOptionShortcut(const std::string& longName, const std::vector<std::string>& args);

protected:
  MIKTEXMFTHISAPI(std::vector<poptOption>) GetOptions() const;

public:
  virtual MIKTEXMFTHISAPI(void) ProcessCommandLineOptions();

protected:
  virtual MIKTEXMFTHISAPI(bool) ProcessOption(int opt, const std::string& optArg);

protected:
  MIKTEXMFTHISAPI(void) ShowProgramVersion() const;

protected:
  MIKTEXMFTHISAPI(void) ShowHelp(bool usageOnly = false) const;

protected:
  MIKTEXMFTHISAPI(void) BadUsage() const;

protected:
  virtual std::string GetUsage() const
  {
    // must be implemented in sub-classes
    MIKTEX_UNEXPECTED();
  }

public:
  virtual unsigned long GetHelpId() const
  {
    return 0;
  }

public:
  virtual MiKTeX::Core::FileType GetInputFileType() const
  {
    // must be implemented in sub-classes
    MIKTEX_UNEXPECTED();
  }

public:
  MIKTEXMFTHISAPI(void) EnableFeature(Feature f);

public:
  MIKTEXMFTHISAPI(bool) IsFeatureEnabled(Feature f) const;

protected:
  MIKTEXMFTHISAPI(void) SetTeX();

public:
  MIKTEXMFTHISAPI(bool) AmITeX() const;

public:
  MIKTEXMFTHISAPI(bool) AmIMETAFONT() const;

public:
  MIKTEXMFTHISAPI(MiKTeX::Core::PathName) GetTcxFileName() const;

protected:
  MIKTEXMFTHISAPI(void) SetTcxFileName(const MiKTeX::Core::PathName& tcxFileName);

protected:
  MIKTEXMFTHISAPI(void) Enable8BitChars(bool enable8BitChars);

public:
  MIKTEXMFTHISAPI(bool) Enable8BitCharsP() const;

public:
  MIKTEXMFTHISAPI(void) InitializeCharTables() const;

public:
  MIKTEXMFTHISAPI(void) SetCharacterConverter(ICharacterConverter* characterConverter);

public:
  MIKTEXMFTHISAPI(ICharacterConverter*) GetCharacterConverter() const;

public:
  MIKTEXMFTHISAPI(void) SetInitFinalize(IInitFinalize* initFinalize);

public:
  MIKTEXMFTHISAPI(IInitFinalize*) GetInitFinalize() const;

public:
  MIKTEXMFTHISAPI(bool) GetVerboseFlag() const;

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

inline void miktexinitializechartables()
{
  WebApp::GetWebApp()->InitializeCharTables();
}

inline bool miktexhavetcxfilename()
{
  return !WebApp::GetWebApp()->GetTcxFileName().Empty();
}

inline bool miktexenableeightbitcharsp()
{
  return WebApp::GetWebApp()->Enable8BitCharsP();
}

inline void miktexprocesscommandlineoptions()
{
  WebApp::GetWebApp()->ProcessCommandLineOptions();
}

template<class PROGRAM_CLASS, class WEBAPP_CLASS> class ProgramRunner
{
public:
  int Run(PROGRAM_CLASS& prog, WEBAPP_CLASS& app, const std::string& programName, int argc, char* argv[])
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
    app.SetProgramInfo(programName, componentVersion, componentCopyright, componentTrademark);
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
      MiKTeX::App::Application::Sorry(argv[0], ex);
      app.Finalize2(1);
      ex.Save();
      return 1;
    }
    catch (const std::exception& ex)
    {
      MiKTeX::App::Application::Sorry(argv[0], ex);
      app.Finalize2(1);
      return 1;
    }
  }
};

// TODO: MIKTEX_DEFINE_WEBAPP(programname, functionname, webappclass, webappinstance, programclass, programinstance)
#define MIKTEX_DEFINE_WEBAPP(functionname, webappclass, webappinstance, programclass, programinstance) \
programclass programinstance;                                                                          \
webappclass webappinstance;                                                                            \
extern "C" int MIKTEXCEECALL functionname(int argc, char* argv[])                                      \
{                                                                                                      \
  MiKTeX::TeXAndFriends::ProgramRunner<programclass, webappclass> p;                                   \
  std::string programName = #webappclass;                                                              \
  return p.Run(programinstance, webappinstance, programName, argc, argv);                              \
}

MIKTEX_TEXMF_END_NAMESPACE;

#endif
