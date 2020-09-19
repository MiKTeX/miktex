/* internal.h: internal definitions                     -*- C++ -*-

   Copyright (C) 2013-2020 Christian Schenk

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

#if defined(MIKTEX_SETUP_SHARED)
#  define MIKTEXSETUPEXPORT MIKTEXDLLEXPORT
#else
#  define MIKTEXSETUPEXPORT
#endif

#define C09CDC45E4B649EDA745DAF436D18309
#include "miktex/Setup/SetupService.h"

#if defined(MIKTEX_WINDOWS)
#  include <comdef.h>
#  include <IntShCut.h>
#  include <shlobj.h>
#endif

#include <fstream>
#include <mutex>
#include <set>

#include <miktex/Core/BufferSizes>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/ConfigNames>
#include <miktex/Core/Cfg>
#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>
#include <miktex/Core/File>
#include <miktex/Core/FileStream>
#include <miktex/Core/FileType>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Core/StreamReader>
#include <miktex/Core/StreamWriter>
#include <miktex/Core/Urls>

#if defined(MIKTEX_WINDOWS)
#  include "miktex/Core/win/Registry"
#  include "miktex/Core/win/winAutoResource"
#  include "miktex/Core/win/WindowsVersion"
#endif

#include <miktex/Extractor/Extractor>

#include <miktex/PackageManager/PackageManager>

#include <miktex/Trace/TraceStream>

#include <miktex/Util/StringUtil>
#include <miktex/Util/Tokenizer>
#include <miktex/Util/inliners.h>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/time.h>

#include <nlohmann/json.hpp>

#define UNIMPLEMENTED() MIKTEX_INTERNAL_ERROR()

#define BEGIN_INTERNAL_NAMESPACE                        \
namespace MiKTeX {                                      \
  namespace Setup {                                     \
    namespace B3CB81AE1C634DFBB208D80FA1A264AE {

#define END_INTERNAL_NAMESPACE                  \
    }                                           \
  }                                             \
}

#define BEGIN_ANONYMOUS_NAMESPACE namespace {
#define END_ANONYMOUS_NAMESPACE }

BEGIN_INTERNAL_NAMESPACE;

#if !defined(UNUSED)
#  if !defined(NDEBUG)
#    define UNUSED(x)
#  else
#    define UNUSED(x) static_cast<void>(x)
#  endif
#endif

#if !defined(UNUSED_ALWAYS)
#  define UNUSED_ALWAYS(x) static_cast<void>(x)
#endif

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

#define TU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#define UT_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()

#define UW_(x) MiKTeX::Util::StringUtil::UTF8ToWideChar(x).c_str()
#define WU_(x) MiKTeX::Util::StringUtil::WideCharToUTF8(x).c_str()

#define SETUPSTATICFUNC(type) static type
#define SETUPINTERNALFUNC(type) type
#define SETUPINTERNALVAR(type) type

#define ARRAY_SIZE(buf) (sizeof(buf)/sizeof(buf[0]))

inline void AssertValidBuf(void* lp, size_t n)
{
#if defined(MIKTEX_DEBUG)
  MIKTEX_ASSERT(lp != nullptr);
#if defined(MIKTEX_WINDOWS)
  MIKTEX_ASSERT(!IsBadWritePtr(lp, n));
#endif
#else
  UNUSED(lp);
  UNUSED(n);
#endif
}

inline void AssertValidString(const char* lp, size_t n = 4096)
{
#if defined(MIKTEX_DEBUG)
  MIKTEX_ASSERT(lp != nullptr);
#if defined(MIKTEX_WINDOWS)
  MIKTEX_ASSERT(!IsBadStringPtrA(lp, n));
#endif
#else
  UNUSED(lp);
  UNUSED(n);
#endif
}

class LogFile
{
public:
  void SetCallback(class SetupServiceImpl* service)
  {
    this->setupService = service;
  }

public:
  void Load(const MiKTeX::Core::PathName& logFileName);

#if defined(MIKTEX_WINDOWS)
public:
  void RemoveRegistrySettings();
#endif

#if defined(MIKTEX_WINDOWS)
public:
  void RemoveStartMenu();
#endif

private:
  void RemoveFiles(const MiKTeX::Core::PathName& prefix);

private:
  std::set<MiKTeX::Core::PathName> files;

#if defined(MIKTEX_WINDOWS)
private:
  struct RegValue
  {
    HKEY hkey;
    std::string strSubKey;
    std::string strValueName;
  };

private:
  std::vector<RegValue> regValues;
#endif

private:
  class SetupServiceImpl* setupService = nullptr;
};

class SetupServiceImpl :
  public SetupService,
  public MiKTeX::Core::IRunProcessCallback,
  public MiKTeX::Packages::PackageInstallerCallback
{
public:
  virtual ~SetupServiceImpl();

public:
  void Initialize() override;

public:
  SetupOptions GetOptions() override
  {
    return this->options;
  }

public:
  SetupOptions SetOptions(const SetupOptions& options) override;

public:
  void OpenLog() override;

public:
  MiKTeX::Core::PathName CloseLog(bool cancel) override;

public:
  void MIKTEXCEECALL Log(const std::string& s) override;

public:
  void ULogOpen() override;

public:
  void ULogClose() override;

public:
  MiKTeX::Core::PathName GetULogFileName() override;

public:
  void ULogAddFile(const MiKTeX::Core::PathName& path) override;

public:
  ProgressInfo GetProgressInfo() override;

public:
  void SetCallback(SetupServiceCallback* callback) override;

public:
  void SetCallbacks(std::function<void(const std::string&)> f_ReportLine, std::function<bool(const std::string&)> f_OnRetryableError, std::function<bool(MiKTeX::Setup::Notification)> f_OnProgress, std::function<bool(const void*, size_t)> f_OnProcessOutput) override;

public:
  void Run() override;

public:
  bool IsCancelled()
  {
    return cancelled;
  }

protected:
  virtual void RegisterUninstaller()
  {
  }

protected:
  virtual void UnregisterPath(bool shared)
  {
  }

protected:
  virtual void UnregisterShellFileTypes()
  {
  };

protected:
  virtual void RemoveRegistryKeys()
  {
  }

public:
  SetupServiceImpl();

protected:
  void CompleteOptions(bool allowRemoteCalls);

protected:
  void DoTheDownload();

protected:
  void DoPrepareMiKTeXDirect();

protected:
  void DoTheInstallation();

protected:
  void DoFinishSetup();

protected:
  void DoFinishUpdate();

protected:
  void DoCleanUp();

protected:
  bool OnProcessOutput(const void* output, size_t n) override;

public:
  void ReportLine(const std::string& str) override;

protected:
  bool OnRetryableError(const std::string& message) override;

protected:
  bool OnProgress(MiKTeX::Packages::Notification nf) override;

protected:
  SetupOptions options;

protected:
  enum Section { None, Files, HKLM, HKCU };

protected:
  Section section = None;

protected:
  std::ofstream logStream;

protected:
  std::mutex logStreamMutex;

protected:
  MiKTeX::Core::PathName intermediateLogFile;

protected:
  std::ofstream uninstStream;

protected:
  std::unique_ptr<MiKTeX::Trace::TraceStream> traceStream;

protected:
  MiKTeX::Core::PathName GetInstallRoot() const;

protected:
  MiKTeX::Core::PathName GetBinDir() const;

protected:
  void ConfigureMiKTeX();

protected:
  void RunIniTeXMF(const std::vector<std::string>& args, bool mustSucceed);

protected:
  void RunMpm(const std::vector<std::string>& args);

protected:
  std::wstring& Expand(const std::string& source, std::wstring& dest);

protected:
  bool FindFile(const MiKTeX::Core::PathName& fileName, MiKTeX::Core::PathName& result);

protected:
  void RemoveFormatFiles();

protected:
  void CollectFiles(std::vector<MiKTeX::Core::PathName>& vec, const MiKTeX::Core::PathName& dir, const char* lpszExt);

protected:
  void CreateInfoFile();

protected:
  void UnregisterComponents();

protected:
  void Warning(const MiKTeX::Core::MiKTeXException& ex);

protected:
  std::vector<MiKTeX::Core::PathName> GetRoots();

protected:
  void LogHeader();

protected:
  bool logging = false;

protected:
  bool cancelled = false;

protected:
  bool initialized = false;

protected:
  std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;

protected:
  std::shared_ptr<MiKTeX::Packages::PackageInstaller> packageInstaller;

protected:
  LogFile logFile;

protected:
  class InternalCallbacks :
    public SetupServiceCallback
  {
  public:
    std::function<void(const std::string&)> f_ReportLine;
  public:
    void ReportLine(const std::string& str) override
    {
      if (f_ReportLine)
      {
        f_ReportLine(str);
      }
    }
  public:
    std::function<bool(const std::string&)> f_OnRetryableError;
  public:
    bool OnRetryableError(const std::string& message) override
    {
      return f_OnRetryableError ? f_OnRetryableError(message) : true;
    }
  public:
    std::function<bool(MiKTeX::Setup::Notification)> f_OnProgress;
  public:
    bool OnProgress(MiKTeX::Setup::Notification nf) override
    {
      return f_OnProgress ? f_OnProgress(nf) : true;
    }
  public:
    std::function<bool(const void*, size_t)> f_OnProcessOutput;
  public:
    bool OnProcessOutput(const void* output, size_t n) override
    {
      return f_OnProcessOutput ? f_OnProcessOutput(output, n) : true;
    }
  } myCallbacks;

protected:
  SetupServiceCallback* callback = &myCallbacks;
};

void RemoveEmptyDirectoryChain(const MiKTeX::Core::PathName& directory);

END_INTERNAL_NAMESPACE;

using namespace MiKTeX::Setup::B3CB81AE1C634DFBB208D80FA1A264AE;
