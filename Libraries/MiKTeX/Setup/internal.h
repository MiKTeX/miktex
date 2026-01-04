/**
 * @file internal.h
 * @author Christian Schenk
 * @brief Internal definitions
 *
 * @copyright Copyright © 2013-2026 Christian Schenk
 *
 * This file is part of the MiKTeX Setup Library.
 *
 * The MiKTeX Setup Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#if defined(MIKTEX_SETUP_SHARED)
#define MIKTEXSETUPEXPORT MIKTEXDLLEXPORT
#else
#define MIKTEXSETUPEXPORT
#endif

#define C09CDC45E4B649EDA745DAF436D18309
#include "miktex/Setup/SetupService.h"

#if defined(MIKTEX_WINDOWS)
#include <comdef.h>
#include <IntShCut.h>
#include <shlobj.h>
#include <VersionHelpers.h>
#endif

#include <fstream>
#include <mutex>
#include <set>

#include <miktex/Configuration/ConfigNames>

#include <miktex/Core/BufferSizes>
#include <miktex/Core/CommandLineBuilder>
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
#include "miktex/Core/win/Registry"
#include "miktex/Core/win/winAutoResource"
#include "miktex/Core/win/WindowsVersion"
#endif

#include <miktex/Archive/Extractor>

#include <miktex/Locale/Translator>

#include <miktex/PackageManager/PackageManager>

#include <miktex/Trace/TraceStream>

#include <miktex/Util/StringUtil>
#include <miktex/Util/Tokenizer>
#include <miktex/Util/inliners.h>

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <nlohmann/json.hpp>

#define UNIMPLEMENTED() MIKTEX_INTERNAL_ERROR()

#define BEGIN_INTERNAL_NAMESPACE                        \
namespace MiKTeX {                                      \
    namespace Setup {                                   \
        namespace B3CB81AE1C634DFBB208D80FA1A264AE {

#define END_INTERNAL_NAMESPACE                          \
        }                                               \
    }                                                   \
}

#define BEGIN_ANONYMOUS_NAMESPACE namespace {
#define END_ANONYMOUS_NAMESPACE }

#include "SetupResources.h"

BEGIN_INTERNAL_NAMESPACE;

#if !defined(UNUSED)
#if !defined(NDEBUG)
#define UNUSED(x)
#else
#define UNUSED(x) static_cast<void>(x)
#endif
#endif

#if !defined(UNUSED_ALWAYS)
#  define UNUSED_ALWAYS(x) static_cast<void>(x)
#endif

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

    void Load(const MiKTeX::Util::PathName& logFileName);

#if defined(MIKTEX_WINDOWS)
    void RemoveRegistrySettings();
    void RemoveStartMenu();
#endif

private:

    void RemoveFiles(const MiKTeX::Util::PathName& prefix);

    std::set<MiKTeX::Util::PathName> files;

#if defined(MIKTEX_WINDOWS)
    struct RegValue
    {
        HKEY hkey;
        std::string strSubKey;
        std::string strValueName;
    };

    std::vector<RegValue> regValues;
#endif

    class SetupServiceImpl* setupService = nullptr;
};

class SetupServiceImpl:
    public SetupService,
    public MiKTeX::Core::IRunProcessCallback,
    public MiKTeX::Packages::PackageInstallerCallback
{

public:

    virtual ~SetupServiceImpl();
    SetupServiceImpl();

    SetupOptions GetOptions() override
    {
        return this->options;
    }

    void Log(const std::string& s) override
    {
        LogInternal(MiKTeX::Trace::TraceLevel::Trace, s);
    }

    bool IsCancelled()
    {
        return cancelled;
    }
    
    void Initialize() override;
    SetupOptions SetOptions(const SetupOptions& options) override;
    void OpenLog() override;
    MiKTeX::Util::PathName CloseLog(bool cancel) override;
    void ReportLine(const std::string& str) override;
    void ULogOpen() override;
    void ULogClose() override;
    MiKTeX::Util::PathName GetULogFileName() override;
    void ULogAddFile(const MiKTeX::Util::PathName& path) override;
    ProgressInfo GetProgressInfo() override;
    void SetCallback(SetupServiceCallback* callback) override;
    void SetCallbacks(std::function<void(const std::string&)> f_ReportLine, std::function<bool(const std::string&)> f_OnRetryableError, std::function<bool(MiKTeX::Setup::Notification)> f_OnProgress, std::function<bool(const void*, size_t)> f_OnProcessOutput) override;
    void Run() override;
    void WriteReport(std::ostream& s, ReportOptionSet options) override;
    void WriteReport(std::ostream& s) override;
    void CollectDiagnosticInfo(const MiKTeX::Util::PathName& outputFileName) override;
    std::vector<Issue> FindIssues(bool checkPath, bool checkPackageIntegrity) override;
    std::vector<Issue> GetIssues() override;

protected:

    std::string T_(const char* msgId)
    {
        return translator->Translate(msgId);
    }

    virtual void RegisterUninstaller()
    {
    }

    virtual void UnregisterPath(bool shared)
    {
    }

    virtual void UnregisterShellFileTypes()
    {
    }

    virtual void RemoveRegistryKeys()
    {
    }

    void LogInternal(MiKTeX::Trace::TraceLevel level, const std::string& s);
    void CompleteOptions(bool allowRemoteCalls);
    void DoTheDownload();
    void DoPrepareMiKTeXDirect();
    void DoTheInstallation();
    void DoFinishSetup();
    void DoFinishUpdate();
    void DoCleanUp();
    bool OnProcessOutput(const void* output, size_t n) override;
    bool OnRetryableError(const std::string& message) override;
    bool OnProgress(MiKTeX::Packages::Notification nf) override;
    MiKTeX::Util::PathName GetInstallRoot() const;
    MiKTeX::Util::PathName GetBinDir() const;
    void ConfigureMiKTeX();
    void RunIniTeXMF(const std::vector<std::string>& args, bool mustSucceed);
    void RunOneMiKTeXUtility(const std::vector<std::string>& args, bool mustSucceed);
    void RunMpm(const std::vector<std::string>& args);
    std::wstring& Expand(const std::string& source, std::wstring& dest);
    bool FindFile(const MiKTeX::Util::PathName& fileName, MiKTeX::Util::PathName& result);
    void RemoveFormatFiles();
    void CollectFiles(std::vector<MiKTeX::Util::PathName>& vec, const MiKTeX::Util::PathName& dir, const char* lpszExt);
    void CreateInfoFile();
    void UnregisterComponents();
    void Warning(const MiKTeX::Core::MiKTeXException& ex);
    std::vector<MiKTeX::Util::PathName> GetRoots();
    void LogHeader();

    class InternalCallbacks:
        public SetupServiceCallback
    {

    public:
        std::function<void(const std::string&)> f_ReportLine;
        void ReportLine(const std::string& str) override
        {
            if (f_ReportLine)
            {
                f_ReportLine(str);
            }
        }
        std::function<bool(const std::string&)> f_OnRetryableError;
        bool OnRetryableError(const std::string& message) override
        {
            return f_OnRetryableError ? f_OnRetryableError(message) : true;
        }
        std::function<bool(MiKTeX::Setup::Notification)> f_OnProgress;
        bool OnProgress(MiKTeX::Setup::Notification nf) override
        {
            return f_OnProgress ? f_OnProgress(nf) : true;
        }
        std::function<bool(const void*, size_t)> f_OnProcessOutput;
        bool OnProcessOutput(const void* output, size_t n) override
        {
            return f_OnProcessOutput ? f_OnProcessOutput(output, n) : true;
        }
    } myCallbacks;

    enum Section { None, Files, HKLM, HKCU };

    SetupServiceCallback* callback = &myCallbacks;
    bool cancelled = false;
    bool initialized = false;
    MiKTeX::Util::PathName intermediateLogFile;
    LogFile logFile;
    std::ofstream logStream;
    std::mutex logStreamMutex;
    bool logging = false;
    SetupOptions options;
    std::shared_ptr<MiKTeX::Packages::PackageInstaller> packageInstaller;
    std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;
    Section section = None;
    std::shared_ptr<MiKTeX::Core::Session> session;
    std::unique_ptr<MiKTeX::Trace::TraceStream> traceStream;
    std::unique_ptr<MiKTeX::Locale::Translator> translator;
    std::ofstream uninstStream;

    static SetupResources resources;
};

END_INTERNAL_NAMESPACE;

using namespace MiKTeX::Setup::B3CB81AE1C634DFBB208D80FA1A264AE;
