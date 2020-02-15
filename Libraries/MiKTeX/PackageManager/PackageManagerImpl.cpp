/* PackageManagerImpl.cpp: MiKTeX Package Manager

   Copyright (C) 2001-2020 Christian Schenk

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

#include "config.h"

#include <algorithm>
#include <fstream>
#include <locale>
#include <stack>
#include <unordered_set>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>
#include <miktex/Core/Environment>
#include <miktex/Core/PathNameParser>
#include <miktex/Core/Registry>
#include <miktex/Core/TemporaryDirectory>
#include <miktex/Core/Uri>
#include <miktex/Core/Utils>
#include <miktex/Trace/StopWatch>
#include <miktex/Trace/Trace>

#include "internal.h"
#include "PackageManagerImpl.h"
#include "PackageInstallerImpl.h"
#include "PackageIteratorImpl.h"
#include "TpmParser.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Extractor;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

using namespace MiKTeX::Packages::D6AAD62216146D44B580E92711724B78;

string PackageManagerImpl::proxyUser;
string PackageManagerImpl::proxyPassword;

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
bool PackageManagerImpl::localServer = false;
#endif

PackageManager::~PackageManager() noexcept
{
}

PackageManagerImpl::PackageManagerImpl(const PackageManager::InitInfo& initInfo) :
  trace_error(TraceStream::Open(MIKTEX_TRACE_ERROR, initInfo.traceCallback)),
  trace_mpm(TraceStream::Open(MIKTEX_TRACE_MPM, initInfo.traceCallback)),
  trace_stopwatch(TraceStream::Open(MIKTEX_TRACE_STOPWATCH, initInfo.traceCallback)),
  repositories(webSession)
{
  trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("initializing MPM library version {0}"), MIKTEX_COMPONENT_VERSION_STR));
}

PackageManagerImpl::~PackageManagerImpl()
{
  try
  {
    Dispose();
  }
  catch (const exception &)
  {
  }
}

shared_ptr<PackageManager> PackageManager::Create(const PackageManager::InitInfo& initInfo)
{
  return make_shared<PackageManagerImpl>(initInfo);
}

void PackageManagerImpl::Dispose()
{
  ClearAll();
  if (webSession != nullptr)
  {
    webSession->Dispose();
    webSession = nullptr;
  }
}

unique_ptr<PackageInstaller> PackageManagerImpl::CreateInstaller(const PackageInstaller::InitInfo& initInfo)
{
  return make_unique<PackageInstallerImpl>(shared_from_this(), initInfo);
}

unique_ptr<PackageInstaller> PackageManagerImpl::CreateInstaller()
{
  return make_unique<PackageInstallerImpl>(shared_from_this(), PackageInstaller::InitInfo());
}

unique_ptr<PackageIterator> PackageManagerImpl::CreateIterator()
{
  return make_unique<PackageIteratorImpl>(shared_from_this());
}

void PackageManagerImpl::LoadDatabase(const PathName& path, bool isArchive)
{
  PathName absPath(path);
  absPath.MakeAbsolute();

  unique_ptr<TemporaryDirectory> tempDir;

  PathName packageManifestsPath;

  if (isArchive)
  {
    // create temporary directory
    tempDir = TemporaryDirectory::Create();

    // extract "package-manifests.ini" from archive
    unique_ptr<MiKTeX::Extractor::Extractor> extractor(MiKTeX::Extractor::Extractor::CreateExtractor(DB_ARCHIVE_FILE_TYPE));
    extractor->Extract(absPath, tempDir->GetPathName());

    packageManifestsPath = tempDir->GetPathName() / MIKTEX_PACKAGE_MANIFESTS_INI_FILENAME;
  }
  else
  {
    packageManifestsPath = absPath;
  }

  // load "package-manifests.ini"
  packageDataStore.Clear();
  packageDataStore.LoadAllPackageManifests(packageManifestsPath);
}

void PackageManagerImpl::ClearAll()
{
  packageDataStore.Clear();
}

void PackageManagerImpl::UnloadDatabase()
{
  ClearAll();
}

bool PackageManagerImpl::TryGetPackageInfo(const string& packageId, PackageInfo& packageInfo)
{
  bool knownPackage;
  tie(knownPackage, packageInfo) = packageDataStore.TryGetPackage(packageId);
  return knownPackage;
}

PackageInfo PackageManagerImpl::GetPackageInfo(const string& packageId)
{
  return packageDataStore.GetPackage(packageId);
}

bool PackageManager::TryGetRemotePackageRepository(string& url, RepositoryReleaseState& repositoryReleaseState)
{
  shared_ptr<Session> session = Session::Get();
  repositoryReleaseState = RepositoryReleaseState::Unknown;
  if (session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REMOTE_REPOSITORY, url))
  {
    string str;
    if (session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REPOSITORY_RELEASE_STATE, str))
    {
      if (str == "stable")
      {
        repositoryReleaseState = RepositoryReleaseState::Stable;
      }
      else if (str == "next")
      {
        repositoryReleaseState = RepositoryReleaseState::Next;
      }
    }
    return true;
  }
  return Utils::GetEnvironmentString(MIKTEX_ENV_REPOSITORY, url)
    && (PackageRepositoryDataStore::DetermineRepositoryType(url) == RepositoryType::Remote);
}

string PackageManager::GetRemotePackageRepository(RepositoryReleaseState& repositoryReleaseState)
{
  string url;
  if (!TryGetRemotePackageRepository(url, repositoryReleaseState))
  {
    MIKTEX_UNEXPECTED();
  }
  return url;
}

void PackageManager::SetRemotePackageRepository(const string& url, RepositoryReleaseState repositoryReleaseState)
{
  shared_ptr<Session> session = Session::Get();
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REMOTE_REPOSITORY, url);
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REPOSITORY_RELEASE_STATE, repositoryReleaseState == RepositoryReleaseState::Stable ? "stable" : (repositoryReleaseState == RepositoryReleaseState::Next ? "next" : "unknown"));
}

bool PackageManager::TryGetLocalPackageRepository(PathName& path)
{
  shared_ptr<Session> session = Session::Get();
  string str;
  if (session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_LOCAL_REPOSITORY, str))
  {
    path = str;
    return true;
  }
  else if (Utils::GetEnvironmentString(MIKTEX_ENV_REPOSITORY, str) && (PackageRepositoryDataStore::DetermineRepositoryType(str) == RepositoryType::Local))
  {
    path = str;
    return true;
  }
  else
  {
    return false;
  }
}

PathName PackageManager::GetLocalPackageRepository()
{
  PathName path;
  if (!TryGetLocalPackageRepository(path))
  {
    MIKTEX_UNEXPECTED();
  }
  return path;
}

void PackageManager::SetLocalPackageRepository(const PathName& path)
{
  Session::Get()->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_LOCAL_REPOSITORY, path.ToString());
}

bool PackageManager::TryGetMiKTeXDirectRoot(PathName& path)
{
  shared_ptr<Session> session = Session::Get();
  string str;
  if (session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_MIKTEXDIRECT_ROOT, str))
  {
    path = str;
    return true;
  }
  else if (Utils::GetEnvironmentString(MIKTEX_ENV_REPOSITORY, str) && (PackageRepositoryDataStore::DetermineRepositoryType(str) == RepositoryType::MiKTeXDirect))
  {
    path = str;
    return true;
  }
  else
  {
    return false;
  }
}

PathName PackageManager::GetMiKTeXDirectRoot()
{
  PathName path;
  if (!TryGetMiKTeXDirectRoot(path))
  {
    MIKTEX_UNEXPECTED();
  }
  return path;
}

void PackageManager::SetMiKTeXDirectRoot(const PathName& path)
{
  shared_ptr<Session> session = Session::Get();
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_MIKTEXDIRECT_ROOT, path.ToString());
}

RepositoryInfo PackageManager::GetDefaultPackageRepository()
{
  RepositoryInfo result;
  shared_ptr<Session> session = Session::Get();
  string str;
  if (session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REPOSITORY_TYPE, str))
  {
    if (str == "remote")
    {
      result.url = GetRemotePackageRepository(result.releaseState);
      result.type = RepositoryType::Remote;
    }
    else if (str == "local")
    {
      result.url = GetLocalPackageRepository().ToString();
      result.type = RepositoryType::Local;
    }
    else if (str == "direct")
    {
      result.url = GetMiKTeXDirectRoot().ToString();
      result.type = RepositoryType::MiKTeXDirect;
    }
    else
    {
      MIKTEX_UNEXPECTED();
    }
  }
  else if (Utils::GetEnvironmentString(MIKTEX_ENV_REPOSITORY, result.url))
  {
    result.type = PackageRepositoryDataStore::DetermineRepositoryType(result.url);
  }
  else
  {
    result.url = "";
    result.type = RepositoryType::Remote;
  }
  return result;
}

bool PackageManager::TryGetDefaultPackageRepository(RepositoryType& repositoryType, RepositoryReleaseState& repositoryReleaseState, string& urlOrPath)
{
  RepositoryInfo defaultRepository = GetDefaultPackageRepository();
  repositoryType = defaultRepository.type;
  repositoryReleaseState = defaultRepository.releaseState;
  urlOrPath = defaultRepository.url;
  return true;
}

void PackageManager::SetDefaultPackageRepository(const RepositoryInfo& repository)
{
  shared_ptr<Session> session = Session::Get();
  string repositoryTypeStr;
  switch (repository.type)
  {
  case RepositoryType::MiKTeXDirect:
    repositoryTypeStr = "direct";
    SetMiKTeXDirectRoot(repository.url);
    break;
  case RepositoryType::Local:
    repositoryTypeStr = "local";
    SetLocalPackageRepository(repository.url);
    break;
  case RepositoryType::Remote:
    repositoryTypeStr = "remote";
    SetRemotePackageRepository(repository.url, repository.releaseState);
    break;
  default:
    MIKTEX_UNEXPECTED();
  }
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REPOSITORY_TYPE, repositoryTypeStr);
}

void PackageManager::SetDefaultPackageRepository(RepositoryType repositoryType, RepositoryReleaseState repositoryReleaseState, const string& urlOrPath)
{
  shared_ptr<Session> session = Session::Get();
  RepositoryInfo repository;
  repository.type = repositoryType != RepositoryType::Unknown ? repositoryType : PackageRepositoryDataStore::DetermineRepositoryType(urlOrPath);
  repository.releaseState = repositoryReleaseState;
  repository.url = urlOrPath;
  SetDefaultPackageRepository(repository);
}

namespace {

  typedef unordered_set<string, hash_path, equal_path> SubDirectoryTable;

  struct DirectoryInfo
  {
    SubDirectoryTable subDirectoryNames;
    vector<string> fileNames;
    vector<string> packageNames;
  };

  typedef unordered_map<string, DirectoryInfo, hash_path, equal_path> DirectoryInfoTable;

  // directory info table: written by
  // PackageManagerImpl::CreateMpmFndb(); read by
  // PackageManagerImpl::ReadDirectory()
  DirectoryInfoTable directoryInfoTable;

}

MPMSTATICFUNC(void) RememberFileNameInfo(const string& prefixedFileName, const string& packageId)
{
  shared_ptr<Session> session = Session::Get();

  string fileName;

  // ignore non-texmf files
  if (!PackageManager::StripTeXMFPrefix(prefixedFileName, fileName))
  {
    return;
  }

  PathNameParser pathtok(fileName);

  if (!pathtok)
  {
    return;
  }

  // initialize root path: "//MiKTeX/[MPM]"
  PathName path = session->GetMpmRootPath();
  //  path += CURRENT_DIRECTORY;

  // s1: current path name component
  string s1 = *pathtok;
  ++pathtok;

  // name: file name component
  string name = s1;

  while (pathtok)
  {
    string s2 = *pathtok;
    ++pathtok;
    directoryInfoTable[path.ToString()].subDirectoryNames.insert(s1);
    name = s2;
#if defined(MIKTEX_WINDOWS)
    // make sure the the rest of the path contains slashes (not
    // backslashes)
    path.AppendAltDirectoryDelimiter();
#else
    path.AppendDirectoryDelimiter();
#endif
    path /= s1;
    s1 = s2;
  }

  DirectoryInfo& directoryInfo = directoryInfoTable[path.ToString()];
  directoryInfo.fileNames.push_back(name);
  directoryInfo.packageNames.push_back(packageId);
}

bool PackageManagerImpl::ReadDirectory(const PathName& path, vector<string>& subDirNames, vector<string>& fileNames, vector<string>& fileNameInfos)
{
  const DirectoryInfo& directoryInfo = directoryInfoTable[path.ToString()];
  for (const string& name : directoryInfo.subDirectoryNames)
  {
    subDirNames.push_back(name);
  }
  fileNames = directoryInfo.fileNames;
  fileNameInfos = directoryInfo.packageNames;
  return true;
}

bool PackageManagerImpl::OnProgress(unsigned level, const PathName& directory)
{
  UNUSED_ALWAYS(level);
  UNUSED_ALWAYS(directory);
  return true;
}

void PackageManagerImpl::CreateMpmFndb()
{
  // collect the file names
  for (const PackageInfo& pi : packageDataStore)
  {
    for (const string& file : pi.runFiles)
    {
      RememberFileNameInfo(file, pi.id);
    }
    for (const string& file : pi.docFiles)
    {
      RememberFileNameInfo(file, pi.id);
    }
    for (const string& file : pi.sourceFiles)
    {
      RememberFileNameInfo(file, pi.id);
    }
  }

  // create the database
  Fndb::Create(session->GetMpmDatabasePathName(), session->GetMpmRootPath(), this, true, true);

  // free memory
  directoryInfoTable.clear();
}

bool PackageManager::IsLocalPackageRepository(const PathName& path)
{
  if (!Directory::Exists(path))
  {
    return false;
  }

  // local mirror of remote package repository?
  PathName file1 = PathName(path, MIKTEX_REPOSITORY_MANIFEST_ARCHIVE_FILE_NAME);
  PathName file2 = PathName(path, MIKTEX_PACKAGE_MANIFESTS_ARCHIVE_FILE_NAME);
  if (File::Exists(file1) && File::Exists(file2))
  {
    return true;
  }

  return false;
}

PackageInfo PackageManager::ReadPackageManifestFile(const PathName& path, const string& texmfPrefix)
{
  unique_ptr<TpmParser> tpmParser = TpmParser::Create();
  tpmParser->Parse(path, texmfPrefix);
  return tpmParser->GetPackageInfo();
}

class XmlWriter
{
public:
  XmlWriter(const PathName& path) :
    stream(File::CreateOutputStream(path))
  {
    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << "\n";
  }

public:
  void Close()
  {
    stream.close();
  }

public:
  void StartElement(const string& name)
  {
    if (freshElement)
    {
      stream << '>';
    }
    stream << fmt::format("<{}", name);
    freshElement = true;
    elements.push(name);
  }

public:
  void AddAttribute(const string& name, const string& value)
  {
    string encodedValue;
    for (const char& ch : value)
    {
      switch (ch)
      {
      case '&':
        encodedValue += "&amp;";
        break;
      case '"':
        encodedValue += "&quot;";
        break;
      default:
        encodedValue += ch;
        break;
      }
    }
    stream << fmt::format(" {}=\"{}\"", name, encodedValue);
  }

public:
  void EndElement()
  {
    if (elements.empty())
    {
      MIKTEX_UNEXPECTED();
    }
    if (freshElement)
    {
      stream << "/>";
      freshElement = false;
    }
    else
    {
      stream << fmt::format("</{}>", elements.top());
    }
    elements.pop();
  }

public:
  void EndAllElements()
  {
    while (!elements.empty())
    {
      EndElement();
    }
  }

public:
  void Text(const string& text)
  {
    if (freshElement)
    {
      stream << '>';
      freshElement = false;
    }
    for (const char& ch : text)
    {
      switch (ch)
      {
      case '&':
        stream << "&amp;";
        break;
      case '<':
        stream << "&lt;";
        break;
      case '>':
        stream << "&gt;";
        break;
      default:
        stream << ch;
        break;
      }
    }
  }

private:
  ofstream stream;

private:
  stack<string> elements;

private:
  bool freshElement = false;
};

void PackageManager::WritePackageManifestFile(const PathName& path, const PackageInfo& packageInfo, time_t timePackaged)
{
  XmlWriter xml(path);

  // create "rdf:Description" node
  xml.StartElement("rdf:RDF");
  xml.AddAttribute("xmlns:rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
  xml.AddAttribute("xmlns:TPM", "http://texlive.dante.de/");
  xml.StartElement("rdf:Description");
  string about("http://www.miktex.org/packages/");
  about += packageInfo.id;
  xml.AddAttribute("about", about);

  // create "TPM:Name" node
  xml.StartElement("TPM:Name");
  xml.Text(packageInfo.displayName);
  xml.EndElement();

  // create "TPM:Creator" node
  xml.StartElement("TPM:Creator");
  xml.Text("mpc");
  xml.EndElement();

  // create "TPM:Title" node
  xml.StartElement("TPM:Title");
  xml.Text(packageInfo.title);
  xml.EndElement();

  // create "TPM:Version" node
  xml.StartElement("TPM:Version");
  xml.Text(packageInfo.version);
  xml.EndElement();

  // create "TPM:TargetSystem" node
  xml.StartElement("TPM:TargetSystem");
  xml.Text(packageInfo.targetSystem);
  xml.EndElement();

  // create "TPM:Description" node
  xml.StartElement("TPM:Description");
  xml.Text(packageInfo.description);
  xml.EndElement();


  // create "TPM:RunFiles" node
  if (!packageInfo.runFiles.empty())
  {
    xml.StartElement("TPM:RunFiles");
    xml.AddAttribute("size", std::to_string(packageInfo.sizeRunFiles));
    bool start = true;
    for (const string& file : packageInfo.runFiles)
    {
      if (start)
      {
        start = false;
      }
      else
      {
        xml.Text(" ");
      }      
      xml.Text(PathName(file).ToDos().ToString());
    }
    xml.EndElement();
  }

  // create "TPM:DocFiles" node
  if (!packageInfo.docFiles.empty())
  {
    xml.StartElement("TPM:DocFiles");
    xml.AddAttribute("size", std::to_string(packageInfo.sizeDocFiles));
    bool start = true;
    for (const string& file : packageInfo.docFiles)
    {
      if (start)
      {
        start = false;
      }
      else
      {
        xml.Text(" ");
      }      
      xml.Text(PathName(file).ToDos().ToString());
    }
    xml.EndElement();
  }

  // create "TPM:SourceFiles" node
  if (!packageInfo.sourceFiles.empty())
  {
    xml.StartElement("TPM:SourceFiles");
    xml.AddAttribute("size", std::to_string(packageInfo.sizeSourceFiles));
    bool start = true;
    for (const string& file : packageInfo.sourceFiles)
    {
      if (start)
      {
        start = false;
      }
      else
      {
        xml.Text(" ");
      }      
      xml.Text(PathName(file).ToDos().ToString());
    }
    xml.EndElement();
  }

  // create "TPM:Requires" node
  if (!packageInfo.requiredPackages.empty())
  {
    xml.StartElement("TPM:Requires");
    for (const string& req : packageInfo.requiredPackages)
    {
      xml.StartElement("TPM:Package");
      xml.AddAttribute("name", req);
      xml.EndElement();
    }
    xml.EndElement();
  }

  // create "TPM:TimePackaged" node
  if (timePackaged != 0)
  {
    xml.StartElement("TPM:TimePackaged");
    xml.Text(std::to_string(timePackaged));
    xml.EndElement();
  }

  // create "TPM:MD5" node
  xml.StartElement("TPM:MD5");
  xml.Text(packageInfo.digest.ToString());
  xml.EndElement();

  if (!packageInfo.ctanPath.empty())
  {
    xml.StartElement("TPM:CTAN");
    xml.AddAttribute("path", packageInfo.ctanPath);
    xml.EndElement();
  }

  if (!(packageInfo.copyrightOwner.empty() && packageInfo.copyrightYear.empty()))
  {
    xml.StartElement("TPM:Copyright");
    xml.AddAttribute("owner", packageInfo.copyrightOwner);
    xml.AddAttribute("year", packageInfo.copyrightYear);
    xml.EndElement();
  }

  if (!packageInfo.licenseType.empty())
  {
    xml.StartElement("TPM:License");
    xml.AddAttribute("type", packageInfo.licenseType);
    xml.EndElement();
  }

  xml.EndAllElements();

  xml.Close();
}

void PackageManager::PutPackageManifest(Cfg& cfg, const PackageInfo& packageInfo, time_t timePackaged)
{
  if (cfg.GetKey(packageInfo.id) != nullptr)
  {
    cfg.DeleteKey(packageInfo.id);
  }
  if (!packageInfo.displayName.empty())
  {
    cfg.PutValue(packageInfo.id, "displayName", packageInfo.displayName);
  }
  cfg.PutValue(packageInfo.id, "creator", "mpc");
  if (!packageInfo.title.empty())
  {
    cfg.PutValue(packageInfo.id, "title", packageInfo.title);
  }
  if (!packageInfo.version.empty())
  {
    cfg.PutValue(packageInfo.id, "version", packageInfo.version);
  }
  if (!packageInfo.targetSystem.empty())
  {
    cfg.PutValue(packageInfo.id, "targetSystem", packageInfo.targetSystem);
  }
  if (!packageInfo.description.empty())
  {
    for (const string& line : StringUtil::Split(packageInfo.description, '\n'))
    {
      cfg.PutValue(packageInfo.id, "description[]", line);
    }
  }
  if (!packageInfo.requiredPackages.empty())
  {
    for (const string& pkg : packageInfo.requiredPackages)
    {
      cfg.PutValue(packageInfo.id, "require[]", pkg);
    }
  }
  if (!packageInfo.runFiles.empty())
  {
    cfg.PutValue(packageInfo.id, "runSize", std::to_string(packageInfo.sizeRunFiles));
    for (const string& file : packageInfo.runFiles)
    {
      cfg.PutValue(packageInfo.id, "run[]", PathName(file).ToUnix().ToString());
    }
  }
  if (!packageInfo.docFiles.empty())
  {
    cfg.PutValue(packageInfo.id, "docSize", std::to_string(packageInfo.sizeDocFiles));
    for (const string& file : packageInfo.docFiles)
    {
      cfg.PutValue(packageInfo.id, "doc[]", PathName(file).ToUnix().ToString());
    }
  }
  if (!packageInfo.sourceFiles.empty())
  {
    cfg.PutValue(packageInfo.id, "sourceSize", std::to_string(packageInfo.sizeSourceFiles));
    for (const string& file : packageInfo.sourceFiles)
    {
      cfg.PutValue(packageInfo.id, "source[]", PathName(file).ToUnix().ToString());
    }
  }
  if (IsValidTimeT(timePackaged))
  {
    cfg.PutValue(packageInfo.id, "timePackaged", std::to_string(timePackaged));
  }
  cfg.PutValue(packageInfo.id, "digest", packageInfo.digest.ToString());
  if (!packageInfo.ctanPath.empty())
  {
    cfg.PutValue(packageInfo.id, "ctanPath", packageInfo.ctanPath);
  }
  if (!packageInfo.copyrightOwner.empty())
  {
    cfg.PutValue(packageInfo.id, "copyrightOwner", packageInfo.copyrightOwner);
  }
  if (!packageInfo.copyrightYear.empty())
  {
    cfg.PutValue(packageInfo.id, "copyrightYear", packageInfo.copyrightYear);
  }
  if (!packageInfo.licenseType.empty())
  {
    cfg.PutValue(packageInfo.id, "licenseType", packageInfo.licenseType);
  }
}

PackageInfo PackageManager::GetPackageManifest(const Cfg& cfg, const string& packageId, const std::string& texmfPrefix)
{
  auto key = cfg.GetKey(packageId);
  if (key == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  PackageInfo packageInfo;
  packageInfo.id = packageId;
  for (auto val : *key)
  {
    if (val->GetName() == "displayName")
    {
      packageInfo.displayName = val->AsString();
    }
    else if (val->GetName() == "creator")
    {
      packageInfo.creator = val->AsString();
    }
    else if (val->GetName() == "title")
    {
      packageInfo.title = val->AsString();
    }
    else if (val->GetName() == "version")
    {
      packageInfo.version = val->AsString();
    }
    else if (val->GetName() == "targetSystem")
    {
      packageInfo.targetSystem = val->AsString();
    }
    else if (val->GetName() == "description[]")
    {
      packageInfo.description = std::for_each(val->begin(), val->end(), Flattener('\n')).result;
    }
    else if (val->GetName() == "require[]")
    {
      packageInfo.requiredPackages = val->AsStringVector();
    }
    else if (val->GetName() == "runSize")
    {
      packageInfo.sizeRunFiles = Utils::ToSizeT(val->AsString());
    }
    else if (val->GetName() == "run[]")
    {
      for (const string& s : *val)
      {
        PathName path(s);
#if defined(MIKTEX_UNIX)
        path.ConvertToUnix();
#endif
        if (texmfPrefix.empty() || (PathName::Compare(texmfPrefix, path, texmfPrefix.length()) == 0))
        {
          packageInfo.runFiles.push_back(path.ToString());
        }
      }
    }
    else if (val->GetName() == "docSize")
    {
      packageInfo.sizeDocFiles = Utils::ToSizeT(val->AsString());
    }
    else if (val->GetName() == "doc[]")
    {
      for (const string& s : *val)
      {
        PathName path(s);
#if defined(MIKTEX_UNIX)
        path.ConvertToUnix();
#endif
        if (texmfPrefix.empty() || (PathName::Compare(texmfPrefix, path, texmfPrefix.length()) == 0))
        {
          packageInfo.docFiles.push_back(path.ToString());
        }
      }
    }
    else if (val->GetName() == "sourceSize")
    {
      packageInfo.sizeSourceFiles = Utils::ToSizeT(val->AsString());
    }
    else if (val->GetName() == "source[]")
    {
      for (const string& s : *val)
      {
        PathName path(s);
#if defined(MIKTEX_UNIX)
        path.ConvertToUnix();
#endif
        if (texmfPrefix.empty() || (PathName::Compare(texmfPrefix, path, texmfPrefix.length()) == 0))
        {
          packageInfo.sourceFiles.push_back(path.ToString());
        }
      }
    }
    else if (val->GetName() == "timePackaged")
    {
      packageInfo.timePackaged = Utils::ToTimeT(val->AsString());
    }
    else if (val->GetName() == "digest")
    {
      packageInfo.digest = MD5::Parse(val->AsString());
    }
    else if (val->GetName() == "ctanPath")
    {
      packageInfo.ctanPath = val->AsString();
    }
    else if (val->GetName() == "copyrightOwner")
    {
      packageInfo.copyrightOwner = val->AsString();
    }
    else if (val->GetName() == "copyrightYear")
    {
      packageInfo.copyrightYear = val->AsString();
    }
    else if (val->GetName() == "licenseType")
    {
      packageInfo.licenseType = val->AsString();
    }
  }
  return packageInfo;
}

bool PackageManager::StripTeXMFPrefix(const string& str, string& result)
{
  if (StripPrefix(str, TEXMF_PREFIX_DIRECTORY, result))
  {
    return true;
  }
  PathName prefix2(".");
  prefix2 /= TEXMF_PREFIX_DIRECTORY;
  return StripPrefix(str, prefix2.GetData(), result);
}

void PackageManager::SetProxy(const ProxySettings& proxySettings)
{
  shared_ptr<Session> session = Session::Get();
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_USE_PROXY, proxySettings.useProxy);
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_PROXY_HOST, proxySettings.proxy);
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_PROXY_PORT, proxySettings.port);
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_PROXY_AUTH_REQ, proxySettings.authenticationRequired);
  PackageManagerImpl::proxyUser = proxySettings.user;
  PackageManagerImpl::proxyPassword = proxySettings.password;
}

bool PackageManager::TryGetProxy(const string& url, ProxySettings& proxySettings)
{
  shared_ptr<Session> session = Session::Get(); 
  proxySettings.useProxy = session->GetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_USE_PROXY, false).GetBool();
  if (proxySettings.useProxy)
  {
    if (!session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_PROXY_HOST, proxySettings.proxy))
    {
      return false;
    }
    proxySettings.port = session->GetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_PROXY_PORT, 8080).GetInt();
    proxySettings.authenticationRequired = session->GetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_PROXY_AUTH_REQ, false).GetBool();
    proxySettings.user = PackageManagerImpl::proxyUser;
    proxySettings.password = PackageManagerImpl::proxyPassword;
    return true;
  }
  string proxyEnv;
  if (!url.empty())
  {
    Uri uri(url.c_str());
    string scheme = uri.GetScheme();
    string envName;
    if (scheme == "https")
    {
      envName = "https_proxy";
    }
    else if (scheme == "http")
    {
      envName = "http_proxy";
    }
    else if (scheme == "ftp")
    {
      envName = "FTP_PROXY";
    }
    else
    {
      MIKTEX_UNEXPECTED();
    }
    Utils::GetEnvironmentString(envName, proxyEnv);
  }
  if (proxyEnv.empty())
  {
    Utils::GetEnvironmentString("ALL_PROXY", proxyEnv);
  }
  if (proxyEnv.empty())
  {
    return false;
  }
  Uri uri(proxyEnv);
  proxySettings.useProxy = true;
  proxySettings.proxy = uri.GetHost();
  proxySettings.port = uri.GetPort();
  string userInfo = uri.GetUserInfo();
  proxySettings.authenticationRequired = !userInfo.empty();
  if (proxySettings.authenticationRequired)
  {
    string::size_type idx = userInfo.find_first_of(":");
    if (idx == string::npos)
    {
      proxySettings.user = userInfo;
      proxySettings.password = "";
    }
    else
    {
      proxySettings.user = userInfo.substr(0, idx);
      proxySettings.password = userInfo.substr(idx + 1);
    }
  }
  else
  {
    proxySettings.user = "";
    proxySettings.password = "";
  }
  return true;
}

bool PackageManager::TryGetProxy(ProxySettings& proxySettings)
{
  return TryGetProxy("", proxySettings);
}

ProxySettings PackageManager::GetProxy(const string& url)
{
  ProxySettings proxySettings;
  if (!TryGetProxy(url, proxySettings))
  {
    MIKTEX_FATAL_ERROR(T_("No proxy host is configured."));
  }
  return proxySettings;
}

ProxySettings PackageManager::GetProxy()
{
  return GetProxy("");
}

void PackageManagerImpl::OnProgress()
{
}

bool PackageManagerImpl::TryGetFileDigest(const PathName& prefix, const string& fileName, bool& haveDigest, MD5& digest)
{
  string unprefixed;
  if (!StripTeXMFPrefix(fileName, unprefixed))
  {
    return true;
  }
  PathName path = prefix;
  path /= unprefixed;
  if (!File::Exists(path))
  {
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("package verification failed: file {0} does not exist"), Q_(path)));
    return false;
  }
  if (path.HasExtension(MIKTEX_PACKAGE_MANIFEST_FILE_SUFFIX))
  {
    haveDigest = false;
  }
  else
  {
    digest = MD5::FromFile(path);
    haveDigest = true;
  }
  return true;
}

bool PackageManagerImpl::TryCollectFileDigests(const PathName& prefix, const vector<string>& files, FileDigestTable& fileDigests)
{
  for (const string& fileName : files)
  {
    bool haveDigest;
    MD5 digest;
    if (!TryGetFileDigest(prefix, fileName, haveDigest, digest))
    {
      return false;
    }
    if (haveDigest)
    {
      fileDigests[fileName] = digest;
    }
  }
  return true;
}

bool PackageManagerImpl::TryVerifyInstalledPackage(const string& packageId)
{
  PackageInfo packageInfo = GetPackageInfo(packageId);

  PathName prefix;

  if (!session->IsAdminMode() && packageInfo.IsInstalled(ConfigurationScope::User))
  {
    prefix = session->GetSpecialPath(SpecialPath::UserInstallRoot);
  }

  if (prefix.Empty())
  {
    prefix = session->GetSpecialPath(SpecialPath::CommonInstallRoot);
  }

  FileDigestTable fileDigests;

  if (!TryCollectFileDigests(prefix, packageInfo.runFiles, fileDigests)
    || !TryCollectFileDigests(prefix, packageInfo.docFiles, fileDigests)
    || !TryCollectFileDigests(prefix, packageInfo.sourceFiles, fileDigests))
  {
    return false;
  }

  MD5Builder md5Builder;

  for (const pair<string, MD5> p : fileDigests)
  {
    PathName path(p.first);
    // we must dosify the path name for backward compatibility
    path.ConvertToDos();
    md5Builder.Update(path.GetData(), path.GetLength());
    md5Builder.Update(p.second.data(), p.second.size());
  }

  bool ok = md5Builder.Final() == packageInfo.digest;

  if (!ok)
  {
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("package {0} verification failed: some files have been modified"), Q_(packageId)));
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("expected digest: {0}"), packageInfo.digest));
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("computed digest: {0}"), md5Builder.GetMD5()));
  }

  return ok;
}

string PackageManagerImpl::GetContainerPath(const string& packageId, bool useDisplayNames)
{
  string path;
  PackageInfo packageInfo = GetPackageInfo(packageId);
  for (const string& reqby : packageInfo.requiredBy)
  {
    PackageInfo packageInfo2 = GetPackageInfo(reqby);
    if (packageInfo2.IsPureContainer())
    {
      // RECUSION
      path = GetContainerPath(reqby, useDisplayNames);
      path += PathName::DirectoryDelimiter;
      if (useDisplayNames)
      {
        path += packageInfo2.displayName;
      }
      else
      {
        path += packageInfo2.id;
      }
      break;
    }
  }
  return path;
}

InstallationSummary PackageManagerImpl::GetInstallationSummary(bool userScope)
{
  InstallationSummary result;
  result.packageCount = packageDataStore.GetNumberOfInstalledPackages(userScope);
  string lastUpdateCheckText;
  if (session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER,
                                 userScope ? MIKTEX_REGVAL_LAST_USER_UPDATE_CHECK : MIKTEX_REGVAL_LAST_ADMIN_UPDATE_CHECK,
                                 lastUpdateCheckText))
  {
    result.lastUpdateCheck = std::stol(lastUpdateCheckText);
  }
  string lastUpdateText;
  if (session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER,
                                 userScope ? MIKTEX_REGVAL_LAST_USER_UPDATE : MIKTEX_REGVAL_LAST_ADMIN_UPDATE,
                                 lastUpdateText))
  {
      result.lastUpdate = std::stol(lastUpdateText);
  }
  string lastUpdateDbText;
  if (session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER,
                                 userScope ? MIKTEX_REGVAL_LAST_USER_UPDATE_DB : MIKTEX_REGVAL_LAST_ADMIN_UPDATE_DB,
                                 lastUpdateDbText))
  {
    result.lastUpdateDb = std::stol(lastUpdateDbText);
  }
  return result;
}

MPM_INTERNAL_BEGIN_NAMESPACE;

bool IsUrl(const string& url)
{
  string::size_type pos = url.find("://");
  if (pos == string::npos)
  {
    return false;
  }
  string scheme = url.substr(0, pos);
  for (const char& ch : scheme)
  {
    if (!isalpha(ch, locale()))
    {
      return false;
    }
  }
  return true;
}

string MakeUrl(const string& base, const string& rel)
{
  string url(base);
  size_t l = url.length();
  if (l == 0)
  {
    MIKTEX_UNEXPECTED();
  }
  if (url[l - 1] != '/')
  {
    url += '/';
  }
  if (rel[0] == '/')
  {
    MIKTEX_UNEXPECTED();
  }
  url += rel;
  return url;
}

MPM_INTERNAL_END_NAMESPACE;
