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

template<class FileType> inline bool miktexopentfmfile(FileType & f, const char * lpszFileName)
{
  return OpenTFMFile(&f, lpszFileName);
}

template<class FileType> inline bool miktexopenvffile(FileType & f, const char * lpszFileName)
{
  return OpenVFFile(&f, lpszFileName);
}

template<class FileType> inline int miktexopenxfmfile(FileType & f, const char * lpszFileName)
{
  return OpenXFMFile(&f, lpszFileName);
}

template<class FileType> inline bool miktexopenxvffile(FileType & f, const char * lpszFileName)
{
  return OpenXVFFile(&f, lpszFileName);
}

template<class FileType> inline void miktexprintmiktexbanner(FileType & f)
{
  fprintf(f, " (%s)", MiKTeX::Core::Utils::GetMiKTeXBannerString().c_str());
}

class MIKTEXMFTYPEAPI(WebApp) : public MiKTeX::App::Application
{
public:
  virtual MIKTEXMFTHISAPI(void) Init(const char * lpszProgramInvocationName);

public:
  MIKTEXMFTHISAPI(void) Finalize() override;

protected:
  virtual MIKTEXMFTHISAPI(void) AddOptions();

public:
  virtual MiKTeX::Core::FileType GetInputFileType() const
  {
    // must be implemented in sub-classes
    MIKTEX_ASSERT(false);
    return MiKTeX::Core::FileType::None;
  }

public:
  virtual unsigned long GetHelpId() const
  {
    return 0;
  }

protected:
  virtual const char * GetUsage() const
  {
    // must be implemented in sub-classes
    MIKTEX_ASSERT(false);
    return 0;
  }

public:
  virtual MIKTEXMFTHISAPI(void) ProcessCommandLineOptions();

protected:
  virtual MIKTEXMFTHISAPI(bool) ProcessOption(int opt, const std::string & optArg);

public:
  virtual MIKTEXMFTHISAPI(const char *) TheNameOfTheGame() const;

protected:
  enum {
    OPT_UNSUPPORTED = INT_MAX - 100,
    OPT_NOOP,
  };

protected:
  MIKTEXMFTHISAPI(void) AddOption(const char * lpszNameAndHelp, int opt, int argInfo = POPT_ARG_NONE, const char * lpszArgDescription = nullptr, void * pArg = nullptr, char shortName = 0);

protected:
  MIKTEXMFTHISAPI(void) AddOption(const char * lpszAliasName, const char * lpszName);

protected:
  MIKTEXMFTHISAPI(void) BadUsage() const;

public:
  bool IsFeatureEnabled(Feature f) const
  {
    return features[f];
  }

public:
  const char * GetProgramName() const
  {
    return programName.c_str();
  }

protected:
  bool AmI(const char * lpszName) const
  {
    return MiKTeX::Util::StringUtil::Contains(GetProgramName(), lpszName);
  }

public:
  bool Enable8BitCharsP() const
  {
    return enable8BitChars;
  }

public:
  MiKTeX::Core::PathName GetTcxFileName() const
  {
    return tcxFileName;
  }

#if defined(THEDATA) && defined(IMPLEMENT_TCX)
public:
  void InitializeCharTables()
  {
    unsigned long flags = 0;
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
      tcxFileName.Get(),
      THEDATA(xchr),
      THEDATA(xord),
      THEDATA(xprn));
#else
    MiKTeX::TeXAndFriends::InitializeCharTables(flags,
      tcxFileName.Get(),
      THEDATA(xchr),
      THEDATA(xord),
      0);
#endif
  }
#endif

public:
  void EnableFeature(Feature f)
  {
    features += f;
  }

public:
  MIKTEXMFTHISAPI(void) SetProgramInfo(const char * lpszProgramName, const char * lpszVersion, const char * lpszCopyright, const char * lpszTrademarks);

protected:
  void Enable8BitChars(bool enable8BitChars)
  {
    this->enable8BitChars = enable8BitChars;
  }

protected:
  void SetTcxFileName(const char * lpszTcxFileName)
  {
    tcxFileName = lpszTcxFileName;
  }

protected:
  MIKTEXMFTHISAPI(void) ShowHelp(bool usageOnly = false) const;

protected:
  MIKTEXMFTHISAPI(void) ShowProgramVersion() const;

protected:
  const std::vector<poptOption> & GetOptions() const
  {
    return options;
  }

private:
  MiKTeX::Core::OptionSet<Feature> features;

private:
  MiKTeX::Wrappers::PoptWrapper popt;

private:
  std::string copyright;

private:
  MiKTeX::Core::PathName packageListFileName;

private:
  std::string programName;

private:
  MiKTeX::Core::PathName tcxFileName;

private:
  bool enable8BitChars;

private:
  std::string trademarks;

private:
  std::string version;

private:
  std::vector<poptOption> options;

private:
  std::string theNameOfTheGame;

private:
  int optBase;
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
extern "C" MIKTEXDLLEXPORT int MIKTEXCEECALL dllentry(int argc, const char ** argv) \
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
