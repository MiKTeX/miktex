/* internal.h: internal definitions                     -*- C++ -*-

   Copyright (C) 2001-2016 Christian Schenk

   This file is part of MiKTeX Package Manager.

   MiKTeX Package Manager is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Package Manager is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Package Manager; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(MIKTEX_MPM_SHARED)
#  define MIKTEXMPMEXPORT MIKTEXDLLEXPORT
#else
#  define MIKTEXMPMEXPORT
#endif

#define F927BA187CB94546AB9CA9099D989E81
#include "miktex/PackageManager/config.h"

#include "mpm-version.h"

#include "miktex/PackageManager/PackageManager.h"

#define BEGIN_INTERNAL_NAMESPACE                        \
namespace MiKTeX {                                      \
  namespace Packages {                                  \
    namespace D6AAD62216146D44B580E92711724B78 {

#define END_INTERNAL_NAMESPACE                  \
    }                                           \
  }                                             \
}

#define BEGIN_ANONYMOUS_NAMESPACE namespace {
#define END_ANONYMOUS_NAMESPACE }

#if defined(_MSC_VER) && defined(MIKTEX_WINDOWS)
#  pragma comment(lib, "comsuppw.lib")
#endif

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
namespace MiKTeXPackageManagerLib = MAKE_CURVER_ID(MiKTeXPackageManager);
#endif

#define IGNORE_OTHER_SYSTEMS 1

#if 1
#define ASYNC_LAUNCH_POLICY (std::launch::deferred | std::launch::async)
#else
#define ASYNC_LAUNCH_POLICY std::launch::deferred
#endif

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

#define FATAL_SOAP_ERROR(pSoap) FatalSoapError(pSoap, MIKTEX_SOURCE_LOCATION())

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).Get()

#if defined(MIKTEX_WINDOWS)
#  define WU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#  define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif

#define MPMSTATICFUNC(type) static type
#define MPMINTERNALFUNC(type) type
#define MPMINTERNALVAR(type) type

#define MPM_APSIZE_DB_LIGHT 1024 * 40
#define MPM_APSIZE_DB_FULL 1024 * 245

#define DB_ARCHIVE_FILE_TYPE ArchiveFileType::TarLzma

#include "WebSession.h"

BEGIN_INTERNAL_NAMESPACE;

const time_t Y2000 = 946681200;

const char * const MPM_AGENT = "MPM/" MIKTEX_COMPONENT_VERSION_STR;

const char * const MPMSVC = "mpmsvc";

const size_t MAXURL = 1024;

// the trailing slash should not be removed
#define TEXMF_PREFIX_DIRECTORY \
  "texmf" MIKTEX_PATH_DIRECTORY_DELIMITER_STRING

inline void DbgView(const std::string & s)
{
#if defined(_WIN32)
  OutputDebugStringW(UW_("MiKTeX MPM: " + s));
#endif
}

inline bool StripPrefix(const std::string & str, const char * lpszPrefix, std::string & result)
{
  size_t n = MiKTeX::Util::StrLen(lpszPrefix);
  if (MiKTeX::Core::PathName::Compare(str.c_str(), lpszPrefix, n) != 0)
  {
    return false;
  }
  result = str.c_str() + n;
  return true;
}

struct InstalledFileInfo
{
  unsigned long refCount = 0;
};

#if defined(StrCmp)
#  undef StrCmp
#endif

inline int StrCmp(const char * lpsz1, const char * lpsz2)
{
  return strcmp(lpsz1, lpsz2);
}

inline int StrCmp(const wchar_t * lpsz1, const wchar_t * lpsz2)
{
  return wcscmp(lpsz1, lpsz2);
}

inline int FPutS(const char * lpsz, FILE * stream)
{
  int n = fputs(lpsz, stream);
  if (n < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fputs");
  }
  return n;
  }

inline int FPutC(int ch, FILE * stream)
{
  int chWritten = fputc(ch, stream);
  if (chWritten == EOF)
  {
    MIKTEX_FATAL_CRT_ERROR("fputc");
  }
  return chWritten;
}

struct PathNameComparer :
  public std::binary_function<std::string, std::string, bool>
{
  bool operator() (const std::string & str1, const std::string & str2) const
  {
    return MiKTeX::Core::PathName::Compare(str1, str2) < 0;
  }
};

struct hash_path
{
public:
  std::size_t operator() (const std::string & str) const
  {
    return MiKTeX::Core::PathName(str).GetHash();
  }
};

struct equal_path
{
public:
  bool operator() (const std::string & str1, const std::string & str2) const
  {
    return MiKTeX::Core::PathName::Compare(str1.c_str(), str2.c_str()) == 0;
  }
};

typedef std::map<std::string, MiKTeX::Core::MD5, MiKTeX::Core::less_icase_dos> FileDigestTable;

class PackageManagerImpl :
  public std::enable_shared_from_this<PackageManagerImpl>,
  public MiKTeX::Packages::PackageManager,
  public MiKTeX::Core::ICreateFndbCallback,
  public IProgressNotify_
{
public:
  MIKTEXTHISCALL ~PackageManagerImpl() override;

public:
  std::unique_ptr<MiKTeX::Packages::PackageInstaller> MIKTEXTHISCALL CreateInstaller() override;

public:
  std::unique_ptr<class MiKTeX::Packages::PackageIterator> MIKTEXTHISCALL CreateIterator() override;

public:
  void MIKTEXTHISCALL CreateMpmFndb() override;

public:
  unsigned long MIKTEXTHISCALL GetFileRefCount(const MiKTeX::Core::PathName & path) override;

public:
  MiKTeX::Packages::PackageInfo MIKTEXTHISCALL GetPackageInfo(const std::string & deploymentName);

public:
  void MIKTEXTHISCALL LoadDatabase(const MiKTeX::Core::PathName & path) override;

public:
  void MIKTEXTHISCALL UnloadDatabase() override;

public:
  MiKTeX::Packages::RepositoryReleaseState MIKTEXTHISCALL GetRepositoryReleaseState() override
  {
    return repositoryReleaseState;
  }

public:
  void MIKTEXTHISCALL SetRepositoryReleaseState(MiKTeX::Packages::RepositoryReleaseState repositoryReleaseState) override
  {
    this->repositoryReleaseState = repositoryReleaseState;
  }

public:
  void MIKTEXTHISCALL DownloadRepositoryList() override;

public:
  std::vector<MiKTeX::Packages::RepositoryInfo> MIKTEXTHISCALL GetRepositories() override
  {
    return repositories;
  }

public:
  std::string MIKTEXTHISCALL PickRepositoryUrl() override;

public:
  bool MIKTEXTHISCALL TryGetPackageInfo(const std::string & deploymentName, MiKTeX::Packages::PackageInfo & packageInfo) override;

private:
  bool MIKTEXTHISCALL ReadDirectory(const char * lpszPath, char * * ppSubDirNames, char * * ppFileNames, char * * ppFileNameInfos) override;

private:
  bool MIKTEXTHISCALL OnProgress(unsigned level, const char * lpszDirectory) override;

public:
  void OnProgress() override;

public:
  bool MIKTEXTHISCALL TryGetRepositoryInfo(const std::string & url, MiKTeX::Packages::RepositoryInfo & repositoryInfo) override;

public:
  MiKTeX::Packages::RepositoryInfo MIKTEXTHISCALL VerifyPackageRepository(const std::string & url) override;

public:
  bool MIKTEXTHISCALL TryVerifyInstalledPackage(const std::string & deploymentName) override;

public:
  std::string MIKTEXTHISCALL GetContainerPath(const std::string & deploymentName, bool useDisplayNames) override;

public:
  PackageManagerImpl(const MiKTeX::Packages::PackageManager::InitInfo & initInfo);

public:
  void ClearAll();

public:
  void IncrementFileRefCounts(const std::string & deploymentName);

public:
  void NeedInstalledFileInfoTable();

public:
  void FlushVariablePackageTable();

public:
  void GetAllPackageDefinitions(std::vector<MiKTeX::Packages::PackageInfo> & packages);

public:
 MiKTeX::Packages:: PackageInfo * TryGetPackageInfo(const std::string & deploymentName);

public:
  InstalledFileInfo * GetInstalledFileInfo(const char * lpszPath);

public:
  bool IsRemovable(const std::string & deploymentName);

public:
  time_t GetUserTimeInstalled(const std::string & deploymentName);

public:
  time_t GetCommonTimeInstalled(const std::string & deploymentName);

public:
  time_t GetTimeInstalled(const std::string & deploymentName);

public:
  bool IsPackageInstalled(const std::string & deploymentName);

public:
  bool IsPackageObsolete(const std::string & deploymentName);

public:
  void DeclarePackageObsolete(const std::string & deploymentName, bool obsolete);

public:
  void SetTimeInstalled(const std::string & deploymentName, time_t timeInstalled);

public:
  void SetReleaseState(const std::string & deploymentName, MiKTeX::Packages::RepositoryReleaseState releaseState);

public:
 MiKTeX::Packages:: RepositoryReleaseState GetReleaseState(const std::string & deploymentName);

public:
  MiKTeX::Packages::PackageInfo * DefinePackage(const std::string & deploymentName, const MiKTeX::Packages::PackageInfo & packageinfo);

public:
  void TraceError(const char * lpszFormat, ...);

private:
  void LoadVariablePackageTable();

private:
  void IncrementFileRefCounts(const std::vector<std::string> & files);

private:
  void ParseAllPackageDefinitionFilesInDirectory(const MiKTeX::Core::PathName & directory);

private:
  void ParseAllPackageDefinitionFiles();

private:
  bool TryGetFileDigest(const MiKTeX::Core::PathName & prefix, const std::string & fileName, bool & haveDigest, MiKTeX::Core::MD5 & digest);

private:
  bool TryCollectFileDigests(const MiKTeX::Core::PathName & prefix, const std::vector<std::string> & files, FileDigestTable & fileDigests);

private:
  void Dispose();

public:
  static MiKTeX::Packages::RepositoryType DetermineRepositoryType(const std::string & repository);

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_error;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mpm;

private:
  MiKTeX::Packages::RepositoryReleaseState repositoryReleaseState = MiKTeX::Packages::RepositoryReleaseState::Stable;

private:
  typedef std::unordered_map<std::string, MiKTeX::Packages::PackageInfo, MiKTeX::Core::hash_icase, MiKTeX::Core::equal_icase> PackageDefinitionTable;

private:
  PackageDefinitionTable packageTable;

private:
  typedef std::unordered_map<std::string, InstalledFileInfo, hash_path, equal_path> InstalledFileInfoTable;

private:
  InstalledFileInfoTable installedFileInfoTable;

private:
  bool parsedAllPackageDefinitionFiles = false;

private:
  std::unique_ptr<MiKTeX::Core::Cfg> commonVariablePackageTable;

private:
  std::unique_ptr<MiKTeX::Core::Cfg> userVariablePackageTable;

private:
  std::shared_ptr<MiKTeX::Core::Session> pSession;

private:
  std::unique_ptr<WebSession> webSession;

public:
  WebSession * GetWebSession() const
  {
    return webSession.get();
  }

private:
  std::vector<MiKTeX::Packages::RepositoryInfo> repositories;

public:
  static std::string proxyUser;

public:
  static std::string proxyPassword;

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
public:
  static bool localServer;
#endif
};

class DbLight
{
private:
  std::unique_ptr<MiKTeX::Core::Cfg> pcfg;

public:
  DbLight() :
    pcfg(MiKTeX::Core::Cfg::Create())
  {
  }

public:
  virtual ~DbLight()
  {
  }

public:
  MiKTeX::Core::MD5 GetDigest()
  {
    return pcfg->GetDigest();
  }

private:
  bool TryGetValue(const std::string & deploymentName, const std::string & valueName, std::string & value)
  {
    return pcfg->TryGetValue(deploymentName, valueName, value);
  }

public:
  void Read(const MiKTeX::Core::PathName & path)
  {
    pcfg->Read(path);
  }

public:
  std::string FirstPackage()
  {
    std::shared_ptr<MiKTeX::Core::Cfg::Key> key = pcfg->FirstKey();
    return key == nullptr ? "" : key->GetName();
  }

public:
  std::string NextPackage()
  {
    std::shared_ptr<MiKTeX::Core::Cfg::Key> key = pcfg->NextKey();
    return key == nullptr ? "" : key->GetName();
  }

public:
  void Clear()
  {
    pcfg = MiKTeX::Core::Cfg::Create();
  }

public:
  int GetArchiveFileSize(const std::string & deploymentName)
  {
    std::string str;
    if (!TryGetValue(deploymentName, "CabSize", str))
    {
      MIKTEX_FATAL_ERROR_2(T_("Unknown archive file size."), "package", deploymentName);
    }
    return atoi(str.c_str());
  }

public:
  MiKTeX::Core::MD5 GetArchiveFileDigest(const std::string & deploymentName)
  {
    std::string str;
    if (!pcfg->TryGetValue(deploymentName, "CabMD5", str))
    {
      MIKTEX_FATAL_ERROR_2(T_("Unknown archive file digest."), "package", deploymentName);
    }
    return MiKTeX::Core::MD5::Parse(str.c_str());
  }

public:
  MiKTeX::Core::MD5 GetPackageDigest(const std::string & deploymentName)
  {
    std::string str;
    if (!pcfg->TryGetValue(deploymentName, "MD5", str))
    {
      MIKTEX_FATAL_ERROR_2(T_("Unknown package digest."), "package", deploymentName);
    }
    return MiKTeX::Core::MD5::Parse(str.c_str());
  }

public:
  time_t GetTimePackaged(const std::string & deploymentName)
  {
    std::string str;
    if (!TryGetValue(deploymentName, "TimePackaged", str))
    {
      MIKTEX_FATAL_ERROR_2(T_("Unknown package time-stamp."), "package", deploymentName);
    }
    unsigned time = static_cast<unsigned>(atoi(str.c_str()));
    if (time < Y2000)
    {
      MIKTEX_FATAL_ERROR_2(T_("Invalid package time-stamp."), "package", deploymentName);
    }
    return time;
  }

public:
  PackageLevel GetPackageLevel(const std::string & deploymentName)
  {
    std::string str;
    if (!TryGetValue(deploymentName, "Level", str))
    {
      MIKTEX_FATAL_ERROR_2(T_("Unknown package level."), "package", deploymentName);
    }
    return CharToPackageLevel(str[0]);
  }

public:
  std::string GetPackageVersion(const std::string & deploymentName)
  {
    std::string version;
    if (!TryGetValue(deploymentName, "Version", version))
    {
      version = "";
    }
    return version;
  }

public:
  std::string GetPackageTargetSystem(const std::string & deploymentName)
  {
    std::string targetSystem;
    if (!TryGetValue(deploymentName, "TargetSystem", targetSystem))
    {
      targetSystem = "";
    }
    return targetSystem;
    }

public:
  MiKTeX::Extractor::ArchiveFileType GetArchiveFileType(const std::string & deploymentName)
  {
    std::string str;
    if (!TryGetValue(deploymentName, "Type", str))
    {
      return MiKTeX::Extractor::ArchiveFileType::MSCab;
    }
    if (str == "MSCab")
    {
      return MiKTeX::Extractor::ArchiveFileType::MSCab;
    }
    else if (str == "TarBzip2")
    {
      return MiKTeX::Extractor::ArchiveFileType::TarBzip2;
    }
    else if (str == "TarLzma")
    {
      return MiKTeX::Extractor::ArchiveFileType::TarLzma;
    }
    else
    {
      MIKTEX_FATAL_ERROR_2(T_("Unknown archive file type."), "package", deploymentName);
    }
  }

public:
  static PackageLevel CharToPackageLevel(int ch)
  {
    switch (toupper(ch))
    {
    case 'S': return PackageLevel::Essential;
    case 'M': return PackageLevel::Basic;
    case 'L': return PackageLevel::Advanced;
    case 'T': return PackageLevel::Complete;
    default:
      MIKTEX_FATAL_ERROR_2(T_("Invalid package level."), "level", std::to_string(ch));
    }
  }
  };

class PackageInstallerImpl :
  public MiKTeX::Packages::PackageInstaller,
  public IProgressNotify_,
  public MiKTeX::Core::ICreateFndbCallback,
#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
  public MiKTeXPackageManagerLib::IPackageInstallerCallback,
#endif
  public MiKTeX::Extractor::IExtractCallback,
  public MiKTeX::Core::IRunProcessCallback
{
private:
  enum ErrorCode {
    ERROR_CANNOT_DELETE,
    ERROR_MISSING_PACKAGE, ERROR_CORRUPTED_PACKAGE,
    ERROR_SOURCE_FILE_NOT_FOUND,
    ERROR_SIZE_MISMATCH,
  };

public:
  PackageInstallerImpl(std::shared_ptr<PackageManagerImpl> manager);

public:
  MIKTEXTHISCALL ~PackageInstallerImpl() override;

public:
  void MIKTEXTHISCALL Dispose() override;

public:
  void MIKTEXTHISCALL SetRepository(const std::string & repository) override
  {
    repositoryType = PackageManagerImpl::DetermineRepositoryType(repository);
    this->repository = repository;
  }

public:
  void MIKTEXTHISCALL SetDownloadDirectory(const MiKTeX::Core::PathName & directory) override
  {
    this->downloadDirectory = directory;
  }

public:
  void MIKTEXTHISCALL UpdateDb() override;

public:
  void MIKTEXTHISCALL UpdateDbAsync() override;

public:
  void MIKTEXTHISCALL FindUpdates() override;

public:
  void MIKTEXTHISCALL FindUpdatesAsync() override;

public:
  std::vector<UpdateInfo> MIKTEXTHISCALL GetUpdates() override
  {
    return updates;
  }

public:
  void MIKTEXTHISCALL InstallRemove() override;

public:
  void MIKTEXTHISCALL InstallRemoveAsync() override;

public:
  void MIKTEXTHISCALL WaitForCompletion() override;

public:
  void MIKTEXTHISCALL Download() override;

public:
  void MIKTEXTHISCALL DownloadAsync() override;

private:
  void StartWorkerThread(void (PackageInstallerImpl::*method) ());

public:
  ProgressInfo MIKTEXTHISCALL GetProgressInfo() override
  {
    std::lock_guard<std::mutex> lockGuard(progressIndicatorMutex);
    if (progressInfo.numErrors > 0)
    {
      throw threadMiKTeXException;
    }
    return progressInfo;
  }

public:
  void MIKTEXTHISCALL SetCallback(PackageInstallerCallback * pCallback) override;

public:
  void MIKTEXTHISCALL SetFileLists(const std::vector<std::string> & tbi, const std::vector<std::string> & tbr) override
  {
    this->toBeInstalled = tbi;
    this->toBeRemoved = tbr;
  }

public:
  void MIKTEXTHISCALL SetPackageLevel(PackageLevel packageLevel) override
  {
    taskPackageLevel = packageLevel;
  }

public:
  void MIKTEXTHISCALL SetFileList(const std::vector<std::string> & tbi) override
  {
    SetFileLists(tbi, std::vector<std::string>());
  }

public:
  void OnProgress() override;

public:
  void MIKTEXTHISCALL OnBeginFileExtraction(const std::string & fileName, size_t uncompressedSize) override;

public:
  void MIKTEXTHISCALL OnEndFileExtraction(const std::string & fileName, size_t uncompressedSize) override;

public:
  bool MIKTEXTHISCALL OnError(const std::string & message) override;

public:
  bool MIKTEXTHISCALL OnProcessOutput(const void * pOutput, size_t n) override;

private:
  void NeedRepository();

private:
  void UpdateMpmFndb(const std::vector<std::string> & installedFiles, const std::vector<std::string> & removedFiles, const char * lpszPackageName);

private:
  void CalculateExpenditure(bool downloadOnly = false);

private:
  std::string FatalError(ErrorCode error);

private:
  std::thread workerThread;

public:
  bool MIKTEXTHISCALL IsRunning() const override
  {
    if (!workerThread.joinable())
    {
      return false;
    }
    return !progressInfo.ready;
  }

public:
  void MIKTEXTHISCALL RegisterComponents(bool doRegister) override;

public:
  void MIKTEXTHISCALL SetNoPostProcessing(bool noPostProcessing) override
  {
    this->noPostProcessing = noPostProcessing;
  }

public:
  void MIKTEXTHISCALL SetNoLocalServer(bool noLocalServer) override
  {
    this->noLocalServer = noLocalServer;
  }

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
public:
  STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppvObj);

public:
  STDMETHOD_(ULONG, AddRef) ();

public:
  STDMETHOD_(ULONG, Release) ();

public:
  HRESULT __stdcall ReportLine(BSTR line) override;

public:
  HRESULT __stdcall OnRetryableError(BSTR message, VARIANT_BOOL * pDoContinue) override;

public:
  HRESULT __stdcall OnProgress(LONG nf, VARIANT_BOOL * pDoContinue) override;
#endif

private:
  std::string repository;

private:
  MiKTeX::Core::PathName downloadDirectory;

private:
  DbLight dbLight;

private:
  void InstallDbLight();

private:
  void LoadDbLight(bool download);

private:
  MiKTeX::Packages::RepositoryType repositoryType = MiKTeX::Packages::RepositoryType::Unknown;

private:
 MiKTeX::Packages:: RepositoryReleaseState repositoryReleaseState = MiKTeX::Packages::RepositoryReleaseState::Unknown;

private:
  void ReportLine(const char * lpsz, ...);

private:
  std::string MakeUrl(const char * lpszBase, const char * lpszRel);

private:
  std::string MakeUrl(const char * lpszRel);

private:
  bool AbortOrRetry(const std::string & message)
  {
    return pCallback == nullptr || !pCallback->OnRetryableError(message);
  }

private:
  void Notify(MiKTeX::Packages::Notification nf = MiKTeX::Packages::Notification::None)
  {
    if (pCallback != nullptr && !pCallback->OnProgress(nf))
    {
      trace_mpm->WriteLine("libmpm", T_("client wants to cancel"));
      trace_mpm->WriteLine("libmpm", T_("throwing OperationCancelledException"));
      throw MiKTeX::Core::OperationCancelledException();
    }
  }

#if defined(MIKTEX_WINDOWS)
private:
  void RegisterComponent (bool doRegister, const MiKTeX::Core::PathName & path, bool mustSucceed);
#endif

private:
  void RegisterComponents (bool doRegister, const std::vector<std::string> & packages);

private:
  void RegisterComponents (bool doRegister, const std::vector<std::string> & packages, const std::vector<std::string> & packages2)
  {
    RegisterComponents(doRegister, packages);
    RegisterComponents(doRegister, packages2);
  }

private:
  void RunIniTeXMF(const char * lpszArguments);

private:
  bool autoFndbSync;

private:
  void SetAutoFndbSync(bool autoFndbSync)
  {
    if (this->autoFndbSync == autoFndbSync)
    {
      return;
    }
    trace_mpm->WriteFormattedLine("libmpm", T_("turning %s autoFndbSync"), autoFndbSync ? "on" : "off");
    this->autoFndbSync = autoFndbSync;
  }

private:
  bool noPostProcessing = false;

private:
  bool noLocalServer = false;

private:
  ProgressInfo progressInfo;

private:
  std::shared_ptr<MiKTeX::Core::Session> pSession;

private:
  MiKTeX::Core::MiKTeXException threadMiKTeXException;

private:
  clock_t timeStarted;

private:
  std::vector<std::string> toBeInstalled;

private:
  std::vector<std::string> toBeRemoved;

private:
  std::mutex progressIndicatorMutex;

private:
  std::mutex thisMutex;

private:
  MiKTeX::Packages::PackageLevel taskPackageLevel = MiKTeX::Packages::PackageLevel::None;

private:
  void InstallRemoveThread();

private:
  void DownloadThread();

private:
  void UpdateDbThread();

private:
  void FindUpdatesThread();

private:
  void DoInstall();

private:
  void SetUpPackageDefinitionFiles (const MiKTeX::Core::PathName & directory);

private:
  void HandleObsoletePackageDefinitionFiles (const MiKTeX::Core::PathName & temporaryDirectory);

private:
  void CleanUpUserDatabase();

private:
  void Download(const std::string & url, const MiKTeX::Core::PathName & dest, size_t expectedSize = 0);

private:
  void Download(const MiKTeX::Core::PathName & fileName, size_t expectedSize = 0);

private:
  void RemoveFiles(const std::vector<std::string> & toBeRemoved, bool silently = false);

private:
  void ExtractFiles(const MiKTeX::Core::PathName & archiveFileName, MiKTeX::Extractor::ArchiveFileType archiveFileType);

private:
  void CopyFiles(const MiKTeX::Core::PathName & pathSourceRoot, const std::vector<std::string> & fileList);

private:
  void AddToFileList(std::vector<std::string> & fileList, const MiKTeX::Core::PathName & fileName) const;

private:
  void RemoveFromFileList(std::vector<std::string> & fileList, const MiKTeX::Core::PathName & fileName) const;

private:
  void CopyPackage(const MiKTeX::Core::PathName & pathSourceRoot, const std::string & deploymentName);

private:
  bool MIKTEXTHISCALL ReadDirectory(const char * lpszPath, char * * ppSubDirNames, char * * ppFileNames, char * * ppFileNameInfos) override;

private:
  bool MIKTEXTHISCALL OnProgress(unsigned level, const char * lpszDirectory) override;

private:
  void RemovePackage(const std::string & deploymentName);

private:
  void InstallPackage(const std::string & deploymentName);

private:
  void MyCopyFile(const MiKTeX::Core::PathName & source, const MiKTeX::Core::PathName & dest, size_t & size);

private:
  void DownloadPackage(const std::string & deploymentName);

private:
  bool CheckArchiveFile(const std::string & deploymentName, const MiKTeX::Core::PathName & archiveFileName, bool mustBeOk);

private:
  void CheckDependencies(std::set<std::string> & packages, const std::string & deploymentName, bool force, int level);

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
private:
  bool UseLocalServer();
#endif

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
private:
  void ConnectToServer();
#endif

#if defined(MIKTEX_WINDOWS)
private:
  void MyCoInitialize();
#endif

#if defined(MIKTEX_WINDOWS)
private:
  void MyCoUninitialize();
#endif

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_error;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mpm;

private:
  std::shared_ptr<PackageManagerImpl> pManager;

private:
  MiKTeX::Packages::PackageInstallerCallback * pCallback = nullptr;

private:
  std::vector<UpdateInfo> updates;

#if defined(MIKTEX_WINDOWS)
private:
  int numCoInitialize = 0;
#endif

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
private:
  struct
  {
    ATL::CComQIPtr<MiKTeXPackageManagerLib::IPackageManager> pManager;
    ATL::CComPtr<MiKTeXPackageManagerLib::IPackageInstaller> pInstaller;
  } localServer;
#endif
};

class PackageIteratorImpl : public MiKTeX::Packages::PackageIterator
{
public:
  MIKTEXTHISCALL ~PackageIteratorImpl() override;

public:
  void MIKTEXTHISCALL Dispose() override;

public:
  void MIKTEXTHISCALL AddFilter(PackageFilterSet filter, const char * lpsz = nullptr) override
  {
    if (filter[PackageFilter::RequiredBy])
    {
      MIKTEX_ASSERT(lpsz != nullptr);
      requiredBy = lpsz;
    }
    this->filter += filter;
  }

public:
  bool MIKTEXTHISCALL GetNext(MiKTeX::Packages::PackageInfo & packageinfo) override;

public:
  PackageIteratorImpl(std::shared_ptr<PackageManagerImpl> pManager);

private:
  std::shared_ptr<PackageManagerImpl> pManager;

private:
  std::vector<MiKTeX::Packages::PackageInfo> snapshot;

private:
  std::vector<MiKTeX::Packages::PackageInfo>::const_iterator iter;

private:
  std::string requiredBy;

private:
  PackageFilterSet filter;
};

END_INTERNAL_NAMESPACE;

using namespace MiKTeX::Packages::D6AAD62216146D44B580E92711724B78;
