/* miktex/TeXAndFriends/WebApp.h:                       -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

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

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(B21F1EB98F524CBDA35F0691A5BB8D38)
#define B21F1EB98F524CBDA35F0691A5BB8D38

#include <miktex/TeXAndFriends/config.h>

#include "Prototypes.h"

#include <exception>
#include <memory>
#include <string>
#include <vector>

#include <miktex/App/Application>

#include <miktex/Core/Exceptions>
#include <miktex/Core/FileType>
#include <miktex/Core/OptionSet>
#include <miktex/Core/PathName>
#include <miktex/Core/Utils>

#include <miktex/Util/StringUtil>

#include <miktex/Wrappers/PoptWrapper>

namespace C4P {
  class Exception31 {};        // aux_done
  class Exception32 {};        // bst_done
  class Exception9932 {};      // no_bst_file
  class Exception9998 {};      // end_of_TEX
  class Exception9999 {};      // final_end
}

MIKTEXMF_BEGIN_NAMESPACE;

enum class Feature
{
  EightBitChars,
  TCX
};

template<class FileType> inline bool miktexopentfmfile(FileType& f, const char* fileName)
{
  return OpenTFMFile(&f, fileName);
}

template<class FileType> inline bool miktexopenvffile(FileType& f, const char* fileName)
{
  return OpenVFFile(&f, fileName);
}

template<class FileType> inline int miktexopenxfmfile(FileType& f, const char* fileName)
{
  return OpenXFMFile(&f, fileName);
}

template<class FileType> inline bool miktexopenxvffile(FileType& f, const char* fileName)
{
  return OpenXVFFile(&f, fileName);
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
  virtual MIKTEXMFTHISAPI(void) Init(const std::string& programInvocationName);

public:
  MIKTEXMFTHISAPI(void) Finalize() override;

public:
  MIKTEXMFTHISAPI(void) SetProgramInfo(const std::string& programName, const std::string& version, const std::string& copyright, const std::string& trademarks);

public:
  virtual MIKTEXMFTHISAPI(std::string) TheNameOfTheGame() const;

public:
  MIKTEXMFTHISAPI(std::string) GetProgramName() const;

protected:
  MIKTEXMFTHISAPI(bool) AmI(const std::string & name) const;

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

public:
  MIKTEXMFTHISAPI(MiKTeX::Core::PathName) GetTcxFileName() const;

protected:
  MIKTEXMFTHISAPI(void) SetTcxFileName(const MiKTeX::Core::PathName& tcxFileName);

protected:
  MIKTEXMFTHISAPI(void) Enable8BitChars(bool enable8BitChars);

public:
  MIKTEXMFTHISAPI(bool) Enable8BitCharsP() const;

#if defined(THEDATA) && defined(IMPLEMENT_TCX)
public:
  void InitializeCharTables()
  {
    unsigned long flags = 0;
    MiKTeX::Core::PathName tcxFileName = GetTcxFileName();
    if (tcxFileName.GetLength() > 0)
    {
      flags |= ICT_TCX;
    }
    if (Enable8BitCharsP())
    {
      flags |= ICT_8BIT;
    }
#if defined(MIKTEX_TEX_COMPILER) || defined(MIKTEX_META_COMPILER)
    MiKTeX::TeXAndFriends::InitializeCharTables(flags,
      tcxFileName,
      THEDATA(xchr),
      THEDATA(xord),
      THEDATA(xprn));
#else
    MiKTeX::TeXAndFriends::InitializeCharTables(flags,
      tcxFileName,
      THEDATA(xchr),
      THEDATA(xord),
      nullptr);
#endif
  }
#endif

private:
  class impl;
  std::unique_ptr<impl> pimpl;
};

#if defined(MIKTEX_COMPONENT_VERSION_STR)
#  if defined(MIKTEX_COMP_TM_STR)
#    define SET_PROGRAM_INFO__423C8217_4CFC_41B7_9F89_EA3C4F729FD1(app) \
      app.SetProgramInfo (app.TheNameOfTheGame(),                       \
                          MIKTEX_COMPONENT_VERSION_STR,                 \
                          MIKTEX_COMP_COPYRIGHT_STR,                    \
                          MIKTEX_COMP_TM_STR)
#  else
#    define SET_PROGRAM_INFO__423C8217_4CFC_41B7_9F89_EA3C4F729FD1(app) \
      app.SetProgramInfo (app.TheNameOfTheGame(),                       \
                          MIKTEX_COMPONENT_VERSION_STR,                 \
                          MIKTEX_COMP_COPYRIGHT_STR,                    \
                          "")
#  endif
#else
#  define SET_PROGRAM_INFO__423C8217_4CFC_41B7_9F89_EA3C4F729FD1(app)
#endif

#define MIKTEX_DEFINE_WEBAPP(dllentry, appclass, app, program)          \
appclass app;                                                           \
extern "C" MIKTEXDLLEXPORT int MIKTEXCEECALL dllentry(int argc, const char** argv) \
{                                                                       \
  SET_PROGRAM_INFO__423C8217_4CFC_41B7_9F89_EA3C4F729FD1(app);          \
  try                                                                   \
  {                                                                     \
    app.Init(argv[0]);                                                  \
    int exitCode = program(argc, argv);                                 \
    app.Finalize();                                                     \
    return exitCode;                                                    \
  }                                                                     \
  catch (const MiKTeX::Core::MiKTeXException & ex)                      \
  {                                                                     \
    MiKTeX::App::Application::Sorry(argv[0], ex);                       \
    return 1;                                                           \
  }                                                                     \
  catch (const std::exception & ex)                                     \
  {                                                                     \
    MiKTeX::App::Application::Sorry(argv[0], ex);                       \
    return 1;                                                           \
  }                                                                     \
}

MIKTEXMF_END_NAMESPACE;

#endif
