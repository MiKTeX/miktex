/* mpc.cpp: creating MiKTeX packages

   Copyright (C) 2001-2016 Christian Schenk

   This file is part of MPC.

   MPC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.
   
   MPC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.
   
   You should have received a copy of the GNU General Public License
   along with MPC; if not, write to the Free Software Foundation, 59
   Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include "StdAfx.h"
#include "internal.h"

const char * const TheNameOfTheGame = T_("MiKTeX Package Creator");

#define PROGNAME "mpc"

enum class ArchiveFileType
{
  None,
  MSCab,
  TarBzip2,
  Zip,
  Tar,
  TarLzma
};

class RestoreCurrentDirectory
{
public:
  RestoreCurrentDirectory(const PathName & path)
  {
    oldPath.SetToCurrentDirectory();
    Directory::SetCurrent(path);
  }
public:
  virtual ~RestoreCurrentDirectory()
  {
    try
    {
      Directory::SetCurrent(oldPath);
    }
    catch (const exception &)
    {
    }
  }
private:
  PathName oldPath;
};

struct PackageSpec
{
  string deploymentName; // external package name
  char level = '\0';     // one of: '-', 'S', 'M', 'L', 'T'
  ArchiveFileType archiveFileType = ArchiveFileType::None;
};

typedef map<string, MD5, less_icase_dos> FileDigestTable;
  
enum Option
{
  OPT_AAA = 300,
  OPT_BUILD_TDS,
  OPT_CREATE_PACKAGE,
  OPT_DEFAULT_LEVEL,
  OPT_DISASSEMBLE_PACKAGE,
  OPT_MIKTEX_SERIES,
  OPT_PACKAGE_LIST,
  OPT_RELEASE_STATE,
  OPT_REPOSITORY,
  OPT_STAGING_DIR,
  OPT_STAGING_ROOTS,
  OPT_TEXMF_PARENT,
  OPT_TEXMF_PREFIX,
  OPT_TIME_PACKAGED,
  OPT_TPM_DIR,
  OPT_TPM_FILE,
  OPT_UPDATE_REPOSITORY,
  OPT_VERBOSE,
  OPT_VERSION,
};

struct MpcPackageInfo :
  public PackageInfo
{
  MD5 archiveFileDigest;
  PathName path;
  MpcPackageInfo()
  {
  }
  MpcPackageInfo(const PackageInfo & packageInfo) :
    PackageInfo(packageInfo)
  {
  }
};

class PackagedOnReversed
{
public:
  bool operator() (const MpcPackageInfo & pi1, const MpcPackageInfo & pi2) const
  {
    return pi1.timePackaged >= pi2.timePackaged;
  }
};

class PackageCreator :
  public IRunProcessCallback

{
public:
  void Init(const char * argv0);

public:
  void Finalize();

public:
  void Run(int argc, const char ** argv);

protected:
  ArchiveFileType GetDbArchiveFileType();

protected:
  PathName GetDbFileName(int id, const VersionNumber & versionNumber);

protected:
  PathName GetDbLightFileName();

protected:
  PathName GetDbHeavyFileName();

protected:
  void Verbose(const char * lpszFormat, ...);

protected:
  void FatalError(const char * lpszFormat, ...);

protected:
  void Warning(const char * lpszFormat, ...);

protected:
  MD5 GetTdsDigest(const FileDigestTable & fileDigests);

protected:
  MD5 MD5CopyFile(const PathName & source, const PathName & dest);

protected:
  void MD5WildCopy(const PathName & sourceTemplate, const PathName & destDir, const PathName & prefix, FileDigestTable & fileDigests);

protected:
  void MD5CopyFiles(const vector<string> & files, const PathName & sourceDir, const char * lpszSourceSubDir, const PathName & destDir, const char * lpszDestSubDir, FileDigestTable & fileDigests);

protected:
  void WriteDescriptionFile(const string & description, const PathName & stagingDir);

protected:
  void InitializeStagingDirectory(const PathName & stagingDir, const PackageInfo & packageInfo, const FileDigestTable & fileDigests, const MD5 & digest);

protected:
  void CopyPackage(const MpcPackageInfo & packageinfo, const PathName & destDir);

protected:
  void ReadDescriptionFile(const char * lpszStagingDir, string & description);

protected:
  MpcPackageInfo InitializePackageInfo(const char * lpszStagingDir);

protected:
  char GetPackageLevel(const MpcPackageInfo & packageInfo) const;

protected:
  ArchiveFileType GetPackageArchiveFileType(const MpcPackageInfo & packageInfo);

public:
  static const char * GetFileNameExtension(ArchiveFileType archiveFileType)
  {
    switch (archiveFileType)
    {
    case ArchiveFileType::MSCab:
      return MIKTEX_CABINET_FILE_SUFFIX;
    case ArchiveFileType::TarBzip2:
      return MIKTEX_TARBZIP2_FILE_SUFFIX;
    case ArchiveFileType::TarLzma:
      return MIKTEX_TARLZMA_FILE_SUFFIX;
    case ArchiveFileType::Zip:
      return MIKTEX_ZIP_FILE_SUFFIX;
    case ArchiveFileType::Tar:
      return MIKTEX_TAR_FILE_SUFFIX;
    default:
      MIKTEX_FATAL_CRT_ERROR(T_("Unknown archive file type."));
    }
  }

protected:
  bool IsToBeIgnored(const MpcPackageInfo & packageInfo) const;

protected:
  bool IsPureContainerPackage(const MpcPackageInfo & packageInfo) const;

protected:
  bool IsInTeXMFDirectory(const PathName & relPath, const char * lpszDirectory);

protected:
  void CollectFiles(const PathName & rootDir, const PathName & subDir, vector<string> & runFiles, size_t & sizeRunFiles, vector<string> & docFiles, size_t & sizeDocFiles, vector<string> & sourceFiles, size_t & sizeSourceFiles);

protected:
  void CollectSubTree(const PathName & path, const char * lpszSubDir, vector<string> & runFiles, size_t & sizeRunFiles, vector<string> & docFiles, size_t & sizeDocFiles, vector<string> & sourceFiles, size_t & sizeSourceFiles);

protected:
  void CollectPackage(MpcPackageInfo & packageInfo);

protected:
  void CollectPackages(const PathName & stagingRoot, map<string, MpcPackageInfo> & packageTable);

protected:
  void BuildTDS(const map<string, MpcPackageInfo> & packageTable, const PathName & destDir, Cfg & dbLight);

protected:
  void WritePackageDefinitionFiles(const map<string, MpcPackageInfo> & packageTable, const PathName & destDir, Cfg & dbLight);

protected:
  void ExecuteSystemCommand(const char * lpszCommand, const PathName & workingDirectory);

protected:
  void ExecuteSystemCommand(const char * lpszCommand)
  {
    ExecuteSystemCommand(lpszCommand, PathName());
  }
  
protected:
  void RunArchiver(ArchiveFileType archiveFileType, const PathName & archiveFile, const char * lpszFilter);

protected:
  void CreateRepositoryInformationFile(const PathName & repository, Cfg & dbLight, const map<string, MpcPackageInfo> & packageTable);

protected:
  void CreateFileListFile(const map<string, MpcPackageInfo> & packageTable, const PathName & repository);

protected:
  void CleanUp(const PathName & repository);

protected:
  void WriteDatabase(const map<string, MpcPackageInfo> & packageTable, const PathName & repository, bool removeObsoleteRecords, Cfg & dbLight);

protected:
  void Extract(const PathName & archiveFile, ArchiveFileType archiveFileType, const PathName & outDir);

protected:
  void ExtractFile(const PathName & archiveFile, ArchiveFileType archiveFileType, const PathName & toBeExtracted, const PathName & outFile);

protected:
  void CompressArchive(const PathName & toBeCompressed, ArchiveFileType archiveFileType, const PathName & outFile);

protected:
  ArchiveFileType CreateArchiveFile(MpcPackageInfo & packageInfo, const PathName & repository, Cfg & dblight);

protected:
  bool HavePackageArchiveFile(const PathName & repository, const string & deploymentName, PathName & archiveFile, ArchiveFileType & archiveFileType);

protected:
  unique_ptr<Cfg> LoadDbLight(const PathName & repository);

protected:
  map<string, MpcPackageInfo> LoadDbHeavy(const PathName & repository);

protected:
  void UpdateRepository(map<string, MpcPackageInfo> & packageTable, const PathName & repository, Cfg & dbLight);

protected:
  void ReadList(const PathName & path, map<string, PackageSpec> & mapPackageList);

protected:
  void ReadList(const PathName & path, set<string> & setPackageList);

protected:
  void DisassemblePackage(const PathName & packageDefinitionFile, const PathName & sourceDir, const PathName & stagingDir);

protected:
  PathName FindLzma();

private:
  bool MIKTEXTHISCALL OnProcessOutput(const void * pOutput, size_t n) override;

private:
  shared_ptr<Session> session;

private:
  // verbosity flag
  bool optVerbose = false;

private:
  // value of "TPM:TimePackaged"
  time_t programStartTime = static_cast<time_t>(-1);

private:
  string texmfPrefix = "texmf";

private:
  // accumulated --package-list contents
  map<string, PackageSpec> packageList;

private:
  // default package level
  char defaultLevel = 'T';

private:
  // default MiKTeX series
  VersionNumber miktexSeries = MIKTEX_SERIES_STR;

private:
  string releaseState = "stable";

private:
  // default archive file type
  ArchiveFileType defaultArchiveFileType = ArchiveFileType::TarLzma;

private:
  PathName lzmaExe;

private:
  CharBuffer<char, 512> processOutput;

private:
  // command-line options
  static const struct poptOption options[];
};

const struct poptOption PackageCreator::options[] = {

  {
    "build-tds", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, 0, OPT_BUILD_TDS, T_("Create a TDS hierarchy."), 0
  },

  {
    "create-package", 0, POPT_ARG_NONE, 0, OPT_CREATE_PACKAGE, T_("Create a package archive file and update the database."), 0
  },

  {
    "default-level", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, 0, OPT_DEFAULT_LEVEL, T_("Specify the default package level (one of: S, M, L, T)."), T_("LEVEL")
  },

  {
    "disassemble-package", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, 0, OPT_DISASSEMBLE_PACKAGE, T_("Create a staging directory."), 0
  },

  {
    "miktex-series", 0, POPT_ARG_STRING, 0, OPT_MIKTEX_SERIES, T_("Specify the MiKTeX series (one of: 2.9)."), T_("SERIES")
  },

  {
    "package-list", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, 0, OPT_PACKAGE_LIST, T_("Specify the package list file."), T_("FILE")
  },

  {
    "release-state", 0, POPT_ARG_STRING, 0, OPT_RELEASE_STATE, T_("Set the release state (one of: stable, next)."), T_("STATE")
  },

  {
    "repository", 0, POPT_ARG_STRING, 0, OPT_REPOSITORY, T_("Specify the location of the local package repository."), T_("DIR")
  },

  {
    "staging-roots", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, 0, OPT_STAGING_ROOTS, T_("Specify the staging root directories."), T_("DIRS")
  },

  {
    "staging-dir", 0, POPT_ARG_STRING, 0, OPT_STAGING_DIR, T_("Specify the staging directory."), T_("DIR")
  },

  {
    "texmf-parent", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, 0, OPT_TEXMF_PARENT, T_("Specify the TEXMF parent directory."), T_("DIR")
  },

  {
    "texmf-prefix", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, 0, OPT_TEXMF_PREFIX, T_("Specify the TEXMF prefix."), T_("PREFIX")
  },

  {
    "time-packaged", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, 0, OPT_TIME_PACKAGED, T_("Specify the package time-stamp (in seconds since 1970)."), T_("TIME")
  },

  {
    "tpm-dir", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, 0, OPT_TPM_DIR, T_("Specify the destination directory for package definition files."), T_("DIR")
  },

  {
    "tpm-file", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, 0, OPT_TPM_FILE, T_("Specify the name of a package definition file."), T_("FILE")
  },

  {
    "update-repository", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, 0, OPT_UPDATE_REPOSITORY, T_("Update the package repository."), 0
  },

  {
    "verbose", 0, POPT_ARG_NONE, 0, OPT_VERBOSE, T_("Turn on verbose mode."), 0
  },

  {
    T_("version"), 0, POPT_ARG_NONE, 0, OPT_VERSION, T_("Print version information and exit."), 0
  },

  POPT_AUTOHELP
  POPT_TABLEEND
};

void PackageCreator::Init(const char * argv0)
{
  Session::InitInfo initInfo;
  initInfo.SetProgramInvocationName(argv0);
  session = Session::Create(initInfo);
  lzmaExe = FindLzma();
}

void PackageCreator::Finalize()
{
  session = nullptr;
}

PathName PackageCreator::FindLzma()
{
  PathName lzmaExe;
  string path;
  if (!Utils::GetEnvironmentString("PATH", path))
  {
    FatalError("The lzma utility could not be found.");
  }
  if (!(session->FindFile("lzma_alone" MIKTEX_EXE_FILE_SUFFIX, path.c_str(), lzmaExe) || session->FindFile("lzma" MIKTEX_EXE_FILE_SUFFIX, path.c_str(), lzmaExe)))
  {
    FatalError("The lzma utility could not be found.");
  }
  return lzmaExe;
}

ArchiveFileType PackageCreator::GetDbArchiveFileType()
{
  if (miktexSeries < "2.7")
  {
    return ArchiveFileType::TarBzip2;
  }
  else
  {
    return ArchiveFileType::TarLzma;
  }
}

PathName PackageCreator::GetDbFileName(int id, const VersionNumber & versionNumber)
{
  PathName ret("miktex-zzdb");
  ret.Append(std::to_string(id), false);
  ret.Append("-", false);
  ret.Append(versionNumber.ToString().c_str(), false);
  ret.AppendExtension(PackageCreator::GetFileNameExtension(GetDbArchiveFileType()));
  return ret;
}

PathName PackageCreator::GetDbLightFileName()
{
  return GetDbFileName(1, miktexSeries);
}

PathName PackageCreator::GetDbHeavyFileName()
{
  return GetDbFileName(2, miktexSeries);
}

void PackageCreator::Verbose(const char * lpszFormat, ...)
{
  if (optVerbose)
  {
    va_list arglist;
    va_start(arglist, lpszFormat);
    cout << StringUtil::FormatString(lpszFormat, arglist) << endl;
    va_end(arglist);
  }
}

void PackageCreator::FatalError(const char * lpszFormat, ...)
{
  va_list arglist;
  va_start(arglist, lpszFormat);
  cerr
    << PROGNAME << ": "
    << StringUtil::FormatString(lpszFormat, arglist)
    << endl;
  va_end(arglist);
  throw 1;
}

void PackageCreator::Warning(const char * lpszFormat, ...)
{
  va_list arglist;
  va_start(arglist, lpszFormat);
  cerr
    << PROGNAME << T_(": warning: ")
    << StringUtil::FormatString(lpszFormat, arglist)
    << endl;
  va_end(arglist);
}

MD5 PackageCreator::GetTdsDigest(const FileDigestTable & fileDigests)
{
  MD5Builder md5Builder;
  for (const pair<string, MD5> & p : fileDigests)
  {
    PathName path(p.first);
    // we must dosify the path name for backward compatibility
    path.ToDos();
    md5Builder.Update(path.GetData(), path.GetLength());
    md5Builder.Update(p.second.data(), p.second.size());
  }
  return md5Builder.Final();
}

MD5 PackageCreator::MD5CopyFile(const PathName & source, const PathName & dest)
{
  FileStream fromStream(File::Open(source, FileMode::Open, FileAccess::Read, false));
  FileStream toStream(File::Open(dest, FileMode::Create, FileAccess::Write, false));
  char buf[4096];
  size_t n;
  MD5Builder md5Builder;
  while ((n = fromStream.Read(buf, 4096)) > 0)
  {
    toStream.Write(buf, n);
    md5Builder.Update(buf, n);
  }
  toStream.Close();
  fromStream.Close();
  time_t creationTime;
  time_t lastAccessTime;
  time_t lastWriteTime;
  File::GetTimes(source, creationTime, lastAccessTime, lastWriteTime);
  File::SetTimes(dest, creationTime, lastAccessTime, lastWriteTime);
  return md5Builder.Final();
}

void PackageCreator::MD5WildCopy(const PathName & sourceTemplate, const PathName & destDir, const PathName & prefix, FileDigestTable & fileDigests)
{
  PathName sourceDir(sourceTemplate);
  sourceDir.RemoveFileSpec();

  if (!Directory::Exists(sourceDir))
  {
    return;
  }

  PathName pattern(sourceTemplate);
  pattern.RemoveDirectorySpec();

  // create destination directory
  Directory::Create(destDir);

  DirectoryEntry direntry;

  unique_ptr<DirectoryLister> lister = DirectoryLister::Open(sourceDir, pattern.GetData());

  bool haveSomething = false;

  while (lister->GetNext(direntry))
  {
    haveSomething = true;

    // don't recurse
    if (direntry.isDirectory)
    {
      continue;
    }

    // path to source file
    PathName sourcePath(sourceDir, direntry.name, PathName());

    // path to destination file
    PathName destPath(destDir, direntry.name, PathName());

    // copy file and calculate its digest
    MD5 digest = MD5CopyFile(sourcePath, destPath);

    // package definition files do not contribute to the TDS digest
    if (sourcePath.HasExtension(MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX))
    {
      continue;
    }

    // relativize file name
    const char * lpszRelPath = Utils::GetRelativizedPath(destPath.GetData(), prefix.GetData());

    // remember digest
    fileDigests[lpszRelPath] = digest;
  }

  lister->Close();
  lister = nullptr;

  if (!haveSomething)
  {
    FatalError(T_("No match for %s"), Q_(sourceTemplate));
  }
}

void PackageCreator::MD5CopyFiles(const vector<string> & files, const PathName & sourceDir, const char * lpszSourceSubDir, const PathName & destDir, const char * lpszDestSubDir, FileDigestTable & fileDigests)
{
  // path to source root directory
  PathName sourceRootDir(sourceDir, lpszSourceSubDir);

  // path to destination root directory
  PathName destRootDir(destDir, lpszDestSubDir);

  for (const string & fileName : files)
  {
    // source template
    PathName sourceTemplate(sourceRootDir, fileName);

    // path to destination directory
    PathName destDir(destRootDir, fileName);
    destDir.RemoveFileSpec();

    // copy file
    MD5WildCopy(sourceTemplate, destDir, destRootDir, fileDigests);
  }
}

void PackageCreator::WriteDescriptionFile(const string & description, const PathName & stagingDir)
{
  Directory::Create(stagingDir);
  FileStream stream(File::Open(PathName(stagingDir, "Description", 0), FileMode::Create, FileAccess::Write));
  fputs(description.c_str(), stream.Get());
  stream.Close();
}

void PackageCreator::InitializeStagingDirectory(const PathName & stagingDir, const PackageInfo & packageInfo, const FileDigestTable & fileDigests, const MD5 & digest)
{
  // create staging directory
  Directory::Create(stagingDir);

  FileStream stream;

  // write package.ini
  stream.Attach(File::Open(PathName(stagingDir, "package.ini", PathName()), FileMode::Create, FileAccess::Write));
  fprintf(stream.Get(), "externalname=%s\n", packageInfo.deploymentName.c_str());
  fprintf(stream.Get(), "name=%s\n", packageInfo.displayName.c_str());
  fprintf(stream.Get(), "creator=%s\n", packageInfo.creator.c_str());
  fprintf(stream.Get(), "title=%s\n", packageInfo.title.c_str());
  fprintf(stream.Get(), "version=%s\n", packageInfo.version.c_str());
  fprintf(stream.Get(), "targetsystem=%s\n", packageInfo.targetSystem.c_str());
  fprintf(stream.Get(), "md5=%s\n", digest.ToString().c_str());
#if defined(MIKTEX_EXTENDED_PACKAGEINFO)
  fprintf(stream.Get(), "ctan_path=%s\n", packageInfo.ctanPath.c_str());
  fprintf(stream.Get(), "copyright_owner=%s\n", packageInfo.copyrightOwner.c_str());
  fprintf(stream.Get(), "copyright_year=%s\n", packageInfo.copyrightYear.c_str());
  fprintf(stream.Get(), "license_type=%s\n", packageInfo.licenseType.c_str());
#endif
  bool first = true;
  for (const string & name : packageInfo.requiredPackages)
  {
    if (first)
    {
      fputs("requires=", stream.Get());
      first = false;
    }
    else
    {
      fputc(';', stream.Get());
    }
    fputs(name.c_str(), stream.Get());
  }
  stream.Close();

  // write md5sums.txt
  stream.Attach(File::Open(PathName(stagingDir, "md5sums.txt", PathName()), FileMode::Create, FileAccess::Write));
  for (const pair<string, MD5> & p : fileDigests)
  {
    fprintf(stream.Get(), "%s %s\n", p.second.ToString().c_str(), PathName(p.first).ToUnix().GetData());
  }
  stream.Close();

  // write Description
  if (!packageInfo.description.empty())
  {
    WriteDescriptionFile(packageInfo.description, stagingDir);
  }
}

void PackageCreator::CopyPackage(const MpcPackageInfo & packageinfo, const PathName & destDir)
{
  Verbose(T_("Copying %s ..."), Q_(packageinfo.deploymentName.c_str()));

  // path to package definition directory, e.g.:
  // /miktex/texmf/tpm/packages/
  PathName packageDefinitionDirectory(destDir, PathName(texmfPrefix, MIKTEX_PATH_PACKAGE_DEFINITION_DIR, PathName()), PathName());

  // create package definition directory
  Directory::Create(packageDefinitionDirectory);

  // create the package definition file...
  PackageManager::WritePackageDefinitionFile(PathName(packageDefinitionDirectory, packageinfo.deploymentName, MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX), packageinfo, programStartTime);

  // copy files and calculate digests
  FileDigestTable fileDigests;
  MD5CopyFiles(packageinfo.runFiles, packageinfo.path, "Files", destDir, nullptr, fileDigests);
  MD5CopyFiles(packageinfo.docFiles, packageinfo.path, "Files", destDir, nullptr, fileDigests);
  MD5CopyFiles(packageinfo.sourceFiles, packageinfo.path, "Files", destDir, nullptr, fileDigests);

  // check TDS digest
  if (!(GetTdsDigest(fileDigests) == packageinfo.digest))
  {
    FatalError(T_("Bad TDS digest (%s)."), packageinfo.deploymentName.c_str());
  }
}

void PackageCreator::ReadDescriptionFile(const char * lpszStagingDir, string & description)
{
  PathName descriptionFileName(lpszStagingDir, "Description");
  if (!File::Exists(descriptionFileName))
  {
    description = "";
    return;
  }
  FileStream stream(File::Open(descriptionFileName, FileMode::Open, FileAccess::Read));
  int ch;
  while ((ch = fgetc(stream.Get())) != EOF)
  {
    description += static_cast<char>(ch);
  }
  stream.Close();
}

MpcPackageInfo PackageCreator::InitializePackageInfo(const char * lpszStagingDir)
{
  MpcPackageInfo packageInfo;

  unique_ptr<Cfg> cfg(Cfg::Create());

  // read package.ini
  cfg->Read(PathName(lpszStagingDir, "package.ini", PathName()));

  // get deployment name (mandatory value)
  if (!cfg->TryGetValue("", "externalname", packageInfo.deploymentName))
  {
    FatalError(T_("Invalid package information file (externalname)."));
  }

  // get display name (mandatory value)
  if (!cfg->TryGetValue("", "name", packageInfo.displayName))
  {
    FatalError(T_("Invalid package information file (name)."));
  }

  // get creator (optional value)
  cfg->TryGetValue("", "creator", packageInfo.creator);

  // get title (optional value)
  cfg->TryGetValue("", "title", packageInfo.title);

  // get version (optional value)
  cfg->TryGetValue("", "version", packageInfo.version);

  // get target system (optional value)
  cfg->TryGetValue("", "targetsystem", packageInfo.targetSystem);

  // get required packages (optional value)
  string strReqList;
  if (cfg->TryGetValue("", "requires", strReqList))
  {
    for (CSVList tok(strReqList, ';'); tok.GetCurrent() != nullptr; ++tok)
    {
      packageInfo.requiredPackages.push_back(tok.GetCurrent());
    }
  }

  // get TDS digest (optional value)
  string str;
  if (cfg->TryGetValue("", "MD5", str))
  {
    packageInfo.digest = MD5::Parse(str);
  }

#if defined(MIKTEX_EXTENDED_PACKAGEINFO)
  cfg->TryGetValue("", "ctan_path", packageInfo.ctanPath);
  cfg->TryGetValue("", "copyright_owner", packageInfo.copyrightOwner);
  cfg->TryGetValue("", "copyright_year", packageInfo.copyrightYear);
  cfg->TryGetValue("", "license_type", packageInfo.licenseType);
#endif

  // read extra description file
  ReadDescriptionFile(lpszStagingDir, packageInfo.description);

  // remember the staging directory
  packageInfo.path = lpszStagingDir;

  return packageInfo;
}

char PackageCreator::GetPackageLevel(const MpcPackageInfo & packageInfo) const
{
  map<string, PackageSpec>::const_iterator it = packageList.find(packageInfo.deploymentName);
  if (it == packageList.end())
  {
    // assume default level, if the package is not listed
    return defaultLevel;
  }
  return it->second.level;
}

#if 0
ArchiveFileType PackageCreator::GetPackageArchiveFileType(const MpcPackageInfo & packageInfo)
{
  map<string, PackageSpec>::const_iterator it = packageList.find(packageInfo.deploymentName);
  if (it == packageList.end())
  {
    // assume default archive file type, if the package is not listed
    return defaultArchiveFileType;
  }
  return it->second.archiveFileType;
}
#endif

bool PackageCreator::IsToBeIgnored(const MpcPackageInfo & packageInfo) const
{
  return GetPackageLevel(packageInfo) == '-';
}

bool PackageCreator::IsPureContainerPackage(const MpcPackageInfo & packageInfo) const
{
  if ((packageInfo.docFiles.size() + packageInfo.sourceFiles.size()) == 0)
  {
    size_t n = packageInfo.runFiles.size();
    if (n == 0)
    {
      return true;
    }
    else if (n == 1)
    {
      return PathName(packageInfo.runFiles[0]).HasExtension(MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX);
    }
  }
  return false;
}

bool PackageCreator::IsInTeXMFDirectory(const PathName & relPath, const char * lpszDirectory)
{
  PathName texmfDirectory(texmfPrefix);
  texmfDirectory /= lpszDirectory;
  return PathName::Compare(texmfDirectory, relPath, texmfDirectory.GetLength()) == 0;
}

void PackageCreator::CollectFiles(const PathName & rootDir, const PathName & subDir, vector<string> & runFiles, size_t & sizeRunFiles, vector<string> & docFiles, size_t & sizeDocFiles, vector<string> & sourceFiles, size_t & sizeSourceFiles)
{
  // directory to be inspected, e.g.:
  // /mypackages/a0poster/RunFiles/texmf/tex/latex/a0poster/
  PathName directory(rootDir, subDir);

  if (!Directory::Exists(directory))
  {
    return;
  }

  unique_ptr<DirectoryLister> lister = DirectoryLister::Open(directory);

  DirectoryEntry2 dirEntry;

  while (lister->GetNext(dirEntry))
  {
    // path relative to root directory
    PathName relPath(subDir);
    relPath /= dirEntry.name;

    // check to see if it is a sub-directory
    if (dirEntry.isDirectory)
    {
      // recurse into sub-directory
      CollectFiles(rootDir, relPath, runFiles, sizeRunFiles, docFiles, sizeDocFiles, sourceFiles, sizeSourceFiles);
    }
    else
    {
      // store file name
      // add file size to total size
      if (IsInTeXMFDirectory(relPath, "doc"))
      {
        docFiles.push_back(relPath.ToString());
        sizeDocFiles += dirEntry.size;
      }
      else if (IsInTeXMFDirectory(relPath, "source"))
      {
        sourceFiles.push_back(relPath.ToString());
        sizeSourceFiles += dirEntry.size;
      }
      else
      {
        runFiles.push_back(relPath.ToString());
        sizeRunFiles += dirEntry.size;
      }
    }
  }

  lister->Close();
  lister.reset();
}

void PackageCreator::CollectSubTree(const PathName & path, const char * lpszSubDir, vector<string> & runFiles, size_t & sizeRunFiles, vector<string> & docFiles, size_t & sizeDocFiles, vector<string> & sourceFiles, size_t & sizeSourceFiles)
{
  PathName sourceDir(path, lpszSubDir);
  CollectFiles(sourceDir, 0, runFiles, sizeRunFiles, docFiles, sizeDocFiles, sourceFiles, sizeSourceFiles);
}

void PackageCreator::CollectPackage(MpcPackageInfo & packageInfo)
{
  // clear file lists
  packageInfo.sizeRunFiles = 0;
  packageInfo.runFiles.clear();
  packageInfo.sizeDocFiles = 0;
  packageInfo.docFiles.clear();
  packageInfo.sizeSourceFiles = 0;
  packageInfo.sourceFiles.clear();

  // collect files
  CollectSubTree(packageInfo.path, "Files", packageInfo.runFiles, packageInfo.sizeRunFiles, packageInfo.docFiles, packageInfo.sizeDocFiles, packageInfo.sourceFiles, packageInfo.sizeSourceFiles);
}

void PackageCreator::CollectPackages(const PathName & stagingRoot, map<string, MpcPackageInfo> & packageTable)
{
  if (!Directory::Exists(stagingRoot))
  {
    return;
  }

  unique_ptr<DirectoryLister> lister = DirectoryLister::Open(stagingRoot);

  DirectoryEntry dirEntry;

  while (lister->GetNext(dirEntry))
  {
    if (!dirEntry.isDirectory)
    {
      continue;
    }

    // path to staging directory
    PathName stagingDir(stagingRoot, dirEntry.name);

    // check to see if package.ini exists
    if (!File::Exists(PathName(stagingDir, "package.ini")))
    {
      continue;
    }

    // read package.ini and Description
    MpcPackageInfo packageInfo = InitializePackageInfo(stagingDir.GetData());

    if (IsToBeIgnored(packageInfo))
    {
      continue;
    }

    Verbose(T_("Collecting %s..."), Q_(packageInfo.deploymentName));

    // ignore duplicates
    map<string, MpcPackageInfo>::const_iterator it = packageTable.find(packageInfo.deploymentName);
    if (it != packageTable.end())
    {
      Warning(T_("%s already collected."), Q_(packageInfo.deploymentName));
      continue;
    }

    // collect package
    CollectPackage(packageInfo);

    // store package
    packageTable[packageInfo.deploymentName] = packageInfo;
  }

  lister->Close();
}

void PackageCreator::BuildTDS(const map<string, MpcPackageInfo> & packageTable, const PathName & destDir, Cfg & dbLight)
{
  for (const pair<string, MpcPackageInfo> & p : packageTable)
  {
    if (IsToBeIgnored(p.second))
    {
      continue;
    }

    // assemble package
    CopyPackage(p.second, destDir);

    // update database records
    string level;
    level = GetPackageLevel(p.second);
    dbLight.PutValue(p.second.deploymentName, "Level", level);
    dbLight.PutValue(p.second.deploymentName, "MD5", p.second.digest.ToString());
    dbLight.PutValue(p.second.deploymentName, "TimePackaged", std::to_string(programStartTime));
    if (!p.second.version.empty())
    {
      dbLight.PutValue(p.second.deploymentName, "Version", p.second.version);
    }
    if (!p.second.targetSystem.empty())
    {
      dbLight.PutValue(p.second.deploymentName, "TargetSystem", p.second.targetSystem);
    }
  }
}

void PackageCreator::WritePackageDefinitionFiles(const map<string, MpcPackageInfo> & packageTable, const PathName & destDir, Cfg & dbLight)
{
  // create package definition directory
  Directory::Create(destDir);

  for (const pair<string, MpcPackageInfo> & p : packageTable)
  {
    if (IsToBeIgnored(p.second))
    {
      continue;
    }

    // path to package definition file
    PathName packageDefinitionFile(destDir, p.second.deploymentName, MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX);

    // remove existing package definition file
    if (File::Exists(packageDefinitionFile))
    {
      File::Delete(packageDefinitionFile);
    }

    // write the package definition file
    string str;
    time_t timePackaged;
    if (dbLight.TryGetValue(p.second.deploymentName, "TimePackaged", str))
    {
      timePackaged = std::stoi(str);
    }
    else
    {
      timePackaged = 0;
    }
    PackageManager::WritePackageDefinitionFile(packageDefinitionFile, p.second, timePackaged);
  }
}

bool PackageCreator::OnProcessOutput(const void * pOutput, size_t n)
{
  processOutput.Append(reinterpret_cast<const char*>(pOutput), n);
  return true;
}

void PackageCreator::ExecuteSystemCommand(const char * lpszCommand, const PathName & workingDirectory)
{
  processOutput.Clear();
  int exitCode = 0;
  if (!Process::ExecuteSystemCommand(lpszCommand, &exitCode, this, workingDirectory.Empty() ? nullptr : workingDirectory.GetData()) || exitCode != 0)
  {
    cerr << lpszCommand << ':' << endl;
    cerr << processOutput.GetData() << endl;
    FatalError(T_("A system command failed."));
  }
}

void PackageCreator::RunArchiver(ArchiveFileType archiveFileType, const PathName & archiveFile, const char * lpszFilter)
{
  string command;
  switch (archiveFileType)
  {
  case ArchiveFileType::TarBzip2:
    command = "tar -cjf ";
    command += Q_(archiveFile);
    command += " ";
    command += lpszFilter;
    break;
  case ArchiveFileType::TarLzma:
    command = "tar -cf - ";
    command += lpszFilter;
    command += " | ";
    command += Q_(lzmaExe);
    command += " e -si ";
    command += Q_(archiveFile);
    break;
  default:
    FatalError(T_("Unsupported archive file type."));
  }
  if (File::Exists(archiveFile))
  {
    File::Delete(archiveFile);
  }
  ExecuteSystemCommand(command.c_str());
}

void PackageCreator::CreateRepositoryInformationFile(const PathName & repository, Cfg & dbLight, const map<string, MpcPackageInfo> & packageTable)
{
  int numberOfPackages = 0;
  for (const shared_ptr<Cfg::Key> & key : dbLight.GetKeys())
  {
    numberOfPackages += 1;
  }
  set<MpcPackageInfo, PackagedOnReversed> packagedOnReversed;
  for (const pair<string, MpcPackageInfo> & p: packageTable)
  {
    MpcPackageInfo pi =p.second;
    string str;
    if (dbLight.TryGetValue(p.second.deploymentName, "TimePackaged", str))
    {
      pi.timePackaged = std::stoi(str);
    }
    else
    {
      pi.timePackaged = 0;
    }
    packagedOnReversed.insert(pi);
  }
  int count = 0;
  string lastupd;
  for (const MpcPackageInfo & pi : packagedOnReversed)
  {
    if (count > 0)
    {
      lastupd += " ";
    }
    lastupd += pi.deploymentName;
    if (++count == 20)
    {
      break;
    }
  }
  const time_t t2000 = 946681200;
  int days = static_cast<int>((programStartTime - t2000) / (60 * 60 * 24));
  unique_ptr<Cfg> cfg(Cfg::Create());
  cfg->PutValue("repository", "date", std::to_string(programStartTime));
  cfg->PutValue("repository", "version", std::to_string(days));
  cfg->PutValue("repository", "lstdigest", MD5Builder().Final().ToString());
  cfg->PutValue("repository", "numpkg", std::to_string(numberOfPackages));
  cfg->PutValue("repository", "lastupd", lastupd);
  cfg->PutValue("repository", "relstate", releaseState);
  PathName path(repository);
  path /= "pr.ini";
  if (File::Exists(path))
  {
    File::Delete(path);
  }
  cfg->Write(path);
  MD5Builder lstDigest;
  unique_ptr<DirectoryLister> lister = DirectoryLister::Open(repository);
  DirectoryEntry2 dirEntry;
  vector<string> lst;
  while (lister->GetNext(dirEntry))
  {
    string line = dirEntry.name;
    line += ';';
    line += std::to_string(static_cast<int>(dirEntry.size));
    line += '\n';
    lst.push_back(line);
  }
  lister->Close();
  lister = nullptr;
  sort(lst.begin(), lst.end());
  for (const string & s : lst)
  {
    lstDigest.Update(s.c_str(), s.length());
  }
  cfg->PutValue("repository", "lstdigest", lstDigest.Final().ToString());
  cfg->Write(path);
}

void PackageCreator::CreateFileListFile(const map<string, MpcPackageInfo> & packageTable, const PathName & repository)
{
  vector<string> lines;
  size_t prefixLength = texmfPrefix.length();
  if (!PathName(texmfPrefix).EndsWithDirectoryDelimiter())
  {
    prefixLength += 1;
  }
  for (const pair<string, MpcPackageInfo> & p : packageTable)
  {
    if (IsToBeIgnored(p.second))
    {
      continue;
    }
    for (const string & fileName : p.second.docFiles)
    {
      string line = fileName.substr(prefixLength);
      line += ';';
      line += p.second.deploymentName;
      lines.push_back(line);
    }
    for (const string & fileName : p.second.runFiles)
    {
      string line = fileName.substr(prefixLength);
      line += ';';
      line += p.second.deploymentName;
      lines.push_back(line);
    }
    for (const string & fileName : p.second.sourceFiles)
    {
      string line = fileName.substr(prefixLength);
      line += ';';
      line += p.second.deploymentName;
      lines.push_back(line);
    }
  }
  sort(lines.begin(), lines.end());
  PathName filesCsv(repository);
  filesCsv /= "files.csv";
  PathName filesCsvLzma(filesCsv);
  filesCsvLzma.AppendExtension(".lzma");
  StreamWriter writer(filesCsv);
  for (const string & line : lines)
  {
    writer.WriteLine(line);
  }
  writer.Close();
  string command = Q_(lzmaExe);
  command += " e ";
  command += Q_(filesCsv);
  command += " ";
  command += Q_(filesCsvLzma);
  if (File::Exists(filesCsvLzma))
  {
    File::Delete(filesCsvLzma);
  }
  ExecuteSystemCommand(command.c_str());
  File::Delete(filesCsv);
}

void PackageCreator::CleanUp(const PathName & repository)
{
  vector<string> toBeDeleted;
  unique_ptr<DirectoryLister> lister = DirectoryLister::Open(repository);
  DirectoryEntry2 dirEntry;
  while (lister->GetNext(dirEntry))
  {
    PathName path(repository);
    path /= dirEntry.name;
    PathName path2;
    if (path.HasExtension(".cab"))
    {
      path2 = path;
      path2.SetExtension(".tar.bz2");
      if (File::Exists(path2))
      {
        toBeDeleted.push_back(path.GetData());
      }
      path2 = path;
      path2.SetExtension(".tar.lzma");
      if (File::Exists(path2))
      {
        toBeDeleted.push_back(path.GetData());
      }
    }
    else if (path.HasExtension(".bz2"))
    {
      path2 = path;
      path2.SetExtension(".lzma");
      if (File::Exists(path2))
      {
        toBeDeleted.push_back(path.GetData());
      }
    }
  }
  for (const string & fileName : toBeDeleted)
  {
    Verbose("Removing %s...", Q_(fileName));
    File::Delete(fileName);
  }
}

void PackageCreator::WriteDatabase(const map<string, MpcPackageInfo> & packageTable, const PathName & repository, bool removeObsoleteRecords, Cfg & dbLight)
{
  // create repository
  Directory::Create(repository);

  // change into repository
  Directory::SetCurrent(repository);

  if (removeObsoleteRecords)
  {
    // find obsolete packages
    vector<string> obsoletePackages;
    for (const shared_ptr<Cfg::Key> & key : dbLight.GetKeys())
    {
      map<string, MpcPackageInfo>::const_iterator it = packageTable.find(key->GetName());
      if (it == packageTable.end() || IsToBeIgnored(it->second))
      {
        obsoletePackages.push_back(key->GetName());
      }
    }

    // remove obsolete package records
    for (const string & fileName : obsoletePackages)
    {
      dbLight.DeleteKey(fileName);
    }
  }

  // create temporary mpm.ini
  unique_ptr<TemporaryFile> tempIni = TemporaryFile::Create(PathName(repository, MIKTEX_MPM_INI_FILENAME, PathName()));
  dbLight.Write(tempIni->GetPathName());

  // create light-weight database
  PathName dbPath1 = GetDbLightFileName();
  RunArchiver(GetDbArchiveFileType(), dbPath1, MIKTEX_MPM_INI_FILENAME);

  // delete temporary mpm.ini
  tempIni = nullptr;

  // create temporary package definition directory
  unique_ptr<TemporaryDirectory> tempDir = TemporaryDirectory::Create(PathName(repository, texmfPrefix, PathName()));
  PathName packageDefinitionDir = tempDir->GetPathName();
  packageDefinitionDir /= MIKTEX_PATH_PACKAGE_DEFINITION_DIR;
  Directory::Create(packageDefinitionDir);

  // write all package definition files
  WritePackageDefinitionFiles(packageTable, packageDefinitionDir, dbLight);

  // create heavy-weight database
  PathName dbPath2 = GetDbHeavyFileName();
  RunArchiver(GetDbArchiveFileType(), dbPath2, texmfPrefix.c_str());

  // delete package definition files
  tempDir = nullptr;

  CreateFileListFile(packageTable, repository);

  CleanUp(repository);

  // create pr.ini
  CreateRepositoryInformationFile(repository, dbLight, packageTable);
}

void PackageCreator::Extract(const PathName & archiveFile, ArchiveFileType archiveFileType, const PathName & outDir)
{
  string command;
  switch (archiveFileType)
  {
  case ArchiveFileType::MSCab:
    command = "cabextract ";
    command += Q_(archiveFile);
    break;
  case ArchiveFileType::TarBzip2:
    command = "tar --force-local -xjf ";
    command += Q_(archiveFile);
    break;
  case ArchiveFileType::TarLzma:
    command = Q_(lzmaExe);
    command += " d ";
    command += Q_(archiveFile);
    command += " -so | tar --force-local -xf -";
    break;
  default:
    FatalError(T_("Unsupported archive file type."));
  }
  ExecuteSystemCommand(command.c_str(), outDir);
}

void PackageCreator::ExtractFile(const PathName & archiveFile, ArchiveFileType archiveFileType, const PathName & toBeExtracted, const PathName & outFile)
{
  string command;
  switch (archiveFileType)
  {
  case ArchiveFileType::MSCab:
    command = "cabextract --filter ";
    command += Q_(toBeExtracted);
    command += " --pipe ";
    command += Q_(archiveFile);
    command += " > ";
    command += Q_(outFile);
    break;
  case ArchiveFileType::TarBzip2:
    command = "tar --force-local --to-stdout -xjf ";
    command += Q_(archiveFile);
    command += " ";
    command += Q_(toBeExtracted);
    command += " > ";
    command += Q_(outFile);
    break;
  case ArchiveFileType::TarLzma:
    command = Q_(lzmaExe);
    command += " d ";
    command += Q_(archiveFile);
    command += " -so | tar --force-local --to-stdout -xf - ";
    command += Q_(toBeExtracted);
    command += " > ";
    command += Q_(outFile);
    break;
  default:
    FatalError(T_("Unsupported archive file type."));
  }
  ExecuteSystemCommand(command.c_str());
}

void PackageCreator::CompressArchive(const PathName & toBeCompressed, ArchiveFileType archiveFileType, const PathName & outFile)
{
  string command;
  switch (archiveFileType)
  {
  case ArchiveFileType::TarBzip2:
    command = "bzip2 --keep --compress --stdout ";
    command += Q_(toBeCompressed);
    command += " > ";
    command += Q_(outFile);
    break;
  case ArchiveFileType::TarLzma:
    command = Q_(lzmaExe);
    command += " e ";
    command += Q_(toBeCompressed);
    command += " ";
    command += Q_(outFile);
    break;
  default:
    FatalError(T_("Unsupported archive file type."));
  }
  ExecuteSystemCommand(command.c_str());
  File::Delete(toBeCompressed);
}

bool PackageCreator::HavePackageArchiveFile(const PathName & repository, const string & deploymentName, PathName & archiveFile, ArchiveFileType & archiveFileType)
{
  PathName archiveFile2;

  archiveFileType = ArchiveFileType::None;

  // check to see whether a cabinet file exists
  archiveFile2.Set(repository, deploymentName.c_str(), MIKTEX_CABINET_FILE_SUFFIX);
  if (File::Exists(archiveFile2))
  {
    archiveFile = archiveFile2;
    archiveFileType = ArchiveFileType::MSCab;
  }

  // check to see whether a .tar.bz2 file exists
  archiveFile2.Set(repository, deploymentName.c_str(), MIKTEX_TARBZIP2_FILE_SUFFIX);
  if (File::Exists(archiveFile2))
  {
    archiveFile = archiveFile2;
    archiveFileType = ArchiveFileType::TarBzip2;
  }

  // check to see whether a .tar.lzma file exists
  archiveFile2.Set(repository, deploymentName.c_str(), MIKTEX_TARLZMA_FILE_SUFFIX);
  if (File::Exists(archiveFile2))
  {
    archiveFile = archiveFile2;
    archiveFileType = ArchiveFileType::TarLzma;
  }

  return archiveFileType != ArchiveFileType::None;
}

ArchiveFileType PackageCreator::CreateArchiveFile(MpcPackageInfo & packageInfo, const PathName & repository, Cfg & dblight)
{
  PathName archiveFile;
  ArchiveFileType archiveFileType(ArchiveFileType::None);

  bool reuseExisting = false;

  if (HavePackageArchiveFile(repository, packageInfo.deploymentName, archiveFile, archiveFileType))
  {
#if 0
    Verbose(T_("Checking %s..."), Q_(archiveFile));
#endif

    // don't remake archive file if there are no changes
    string strMD5;
    string strTimePackaged;
    if (dblight.TryGetValue(packageInfo.deploymentName, "MD5", strMD5)
      && MD5::Parse(strMD5.c_str()) == packageInfo.digest
      && dblight.TryGetValue(packageInfo.deploymentName, "TimePackaged", strTimePackaged))
    {
      packageInfo.timePackaged = atoi(strTimePackaged.c_str());
      reuseExisting = true;
    }
#if 1
    else
    {
      // extract the package definition file
      PathName filter(texmfPrefix);
      filter /= MIKTEX_PATH_PACKAGE_DEFINITION_DIR;
      filter /= packageInfo.deploymentName.c_str();
      filter.AppendExtension(MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX);
#if defined(MIKTEX_WINDOWS)
      filter.ToUnix();
#endif
      unique_ptr<TemporaryFile> packageDefinitionFile = TemporaryFile::Create();
      ExtractFile(archiveFile, archiveFileType, filter, packageDefinitionFile->GetPathName());

      // read the package definition file
      PackageInfo existingPackageInfo = PackageManager::ReadPackageDefinitionFile(packageDefinitionFile->GetPathName(), texmfPrefix);

      // check to see whether we can keep the existing file
      if (packageInfo.digest == existingPackageInfo.digest)
      {
        reuseExisting = true;
        packageInfo.timePackaged = existingPackageInfo.timePackaged;
      }
      else
      {
        archiveFileType = ArchiveFileType::None;
      }
    }
#endif
  }

  if (!reuseExisting)
  {
#if 1
    archiveFileType = defaultArchiveFileType;
#else
    archiveFileType = GetPackageArchiveFileType(packageInfo);
#endif

    PathName packageArchiveFile(packageInfo.deploymentName.c_str());
    packageArchiveFile.AppendExtension
    (PackageCreator::GetFileNameExtension(archiveFileType));

    Verbose(T_("Creating %s..."), Q_(packageArchiveFile));

    // create destination directory
    Directory::Create(repository);

    // change into package directory, e.g.:
    // /mypackages/a0poster/
    Directory::SetCurrent(packageInfo.path);

    // path to package definition directory, e.g.:
    // /mypackages/a0poster/Files/texmf/tpm/packages/
    PathName packageDefinitionDir(packageInfo.path);
    packageDefinitionDir /= "Files";
    packageDefinitionDir /= texmfPrefix;
    packageDefinitionDir /= MIKTEX_PATH_PACKAGE_DEFINITION_DIR;

    // create package definition directory
    Directory::Create(packageDefinitionDir);

    // path to package definition file, e.g.:
    // /mypackages/a0poster/Files/texmf/tpm/packages/a0poster.tpm
    PathName packageDefinitionFile
    (packageDefinitionDir, packageInfo.deploymentName.c_str(), MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX);

#if 1
    // keep the time-stamp, if possible
    string strMD5;
    string strTimePackaged;
    if (dblight.TryGetValue(packageInfo.deploymentName, "MD5", strMD5)
      && MD5::Parse(strMD5.c_str()) == packageInfo.digest
      && dblight.TryGetValue(packageInfo.deploymentName, "TimePackaged", strTimePackaged))
    {
      packageInfo.timePackaged = atoi(strTimePackaged.c_str());
    }
    else
    {
      packageInfo.timePackaged = programStartTime;
    }
#endif

    // create the package definition file
    PackageManager::WritePackageDefinitionFile(packageDefinitionFile, packageInfo, packageInfo.timePackaged);

    string command;

    // path to .tar file
    PathName tarFile(repository, packageInfo.deploymentName.c_str(), MIKTEX_TAR_FILE_SUFFIX);

    // path to compressed .tar file
    archiveFile.Set
    (repository, packageInfo.deploymentName.c_str(), PackageCreator::GetFileNameExtension(archiveFileType));

#if defined(MIKTEX_WINDOWS)
    tarFile.ToUnix();
    archiveFile.ToUnix();
#endif

    // create the .tar file
    command = "tar --force-local -cf ";
    command += tarFile.ToString();
#if defined(MIKTEX_WINDOWS) && 0
    command += " --files-from=nul";
#else
    command += " --files-from=/dev/null";
#endif
    ExecuteSystemCommand(command.c_str());
    if (Directory::Exists("Files"))
    {
      RestoreCurrentDirectory restoreCurrentDirectory("Files");
      command = "tar --force-local -rf ";
      command += tarFile.ToString();
      command += " ";
      command += texmfPrefix;
      ExecuteSystemCommand(command.c_str());
    }

    // compress the tar file
    {
      RestoreCurrentDirectory restoreCurrentDirectory(repository);
      if (File::Exists(archiveFile))
      {
        File::Delete(archiveFile);
      }
      CompressArchive(tarFile, archiveFileType, archiveFile);
    }
  }

  // get size of archive file
  packageInfo.archiveFileSize = File::GetSize(archiveFile);

  // get MD5 of archive file
  packageInfo.archiveFileDigest = MD5::FromFile(archiveFile.GetData());

  // touch the new archive file
  File::SetTimes(archiveFile, reuseExisting ? static_cast<time_t>(-1) : programStartTime, static_cast<time_t>(-1), packageInfo.timePackaged);

  return archiveFileType;
}

unique_ptr<Cfg> PackageCreator::LoadDbLight(const PathName & repository)
{
  // path to the light-weight database file
  PathName pathDbLight = repository;
  pathDbLight /= GetDbLightFileName();
#if defined(MIKTEX_WINDOWS)
  pathDbLight.ToUnix();
#endif

  // check to see if the database file exists
  if (!File::Exists(pathDbLight))
  {
    FatalError("The light-weight database does not exist.");
  }

  // create a temporary file
  unique_ptr<TemporaryFile> tempFile = TemporaryFile::Create();

  // extract mpm.ini:
  ExtractFile(pathDbLight, GetDbArchiveFileType(), MIKTEX_MPM_INI_FILENAME, tempFile->GetPathName());

  // parse mpm.ini
  unique_ptr<Cfg> dbLight(Cfg::Create());
  dbLight->Read(tempFile->GetPathName());

  return dbLight;
}

map<string, MpcPackageInfo> PackageCreator::LoadDbHeavy(const PathName & repository)
{
  map<string, MpcPackageInfo> packageTable;

  // path to the heavy-weight database file
  PathName pathDbHeavy = repository;
  pathDbHeavy /= GetDbHeavyFileName();
#if defined(MIKTEX_WINDOWS)
  pathDbHeavy.ToUnix();
#endif

  // check to see if the database file exists
  if (!File::Exists(pathDbHeavy))
  {
    FatalError("The heavy-weight database does not exist.");
  }

  // create a temporary directory
  unique_ptr<TemporaryDirectory> tempDir;

  // extract all package definition files
  Extract(pathDbHeavy, GetDbArchiveFileType(), tempDir->GetPathName());

  // parse all package definition files
  PathName directory = tempDir->GetPathName() / texmfPrefix / MIKTEX_PATH_PACKAGE_DEFINITION_DIR;
  unique_ptr<DirectoryLister> pLister = DirectoryLister::Open(directory, "*" MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX);
  DirectoryEntry direntry;
  while (pLister->GetNext(direntry))
  {
    PathName packageDefinitionFile(directory);
    packageDefinitionFile /= direntry.name;
    PackageInfo packageInfo = PackageManager::ReadPackageDefinitionFile(packageDefinitionFile.GetData(), texmfPrefix);
    char szDeploymentName[BufferSizes::MaxPath];
    packageDefinitionFile.GetFileNameWithoutExtension(szDeploymentName);
    packageInfo.deploymentName = szDeploymentName;
    packageTable[packageInfo.deploymentName] = packageInfo;
  }

  return packageTable;
}

void PackageCreator::UpdateRepository(map<string, MpcPackageInfo> & packageTable, const PathName & repository, Cfg & dbLight)
{
  for (pair<const string, MpcPackageInfo> & p : packageTable)
  {
    if (IsToBeIgnored(p.second) || IsPureContainerPackage(p.second))
    {
      continue;
    }

    // update level field
    string level;
    level = GetPackageLevel(p.second);
    dbLight.PutValue(p.second.deploymentName, "Level", level);

#if 0
    // get TDS digest of already existing package
    string str;
    if (dbLight.TryGetValue(it->second.deploymentName, "MD5", str))
    {
      // don't remake archive file if there are no changes
      PathName archiveFile;
      ArchiveFileType archiveFileType(ArchiveFileType::None);
      if (MD5::Parse(str.c_str()) == it->second.digest
        && HavePackageArchiveFile(repository, it->second.deploymentName, archiveFile, archiveFileType))
      {
#if 0
        Verbose(T_("%s hasn't changed => skipping"), Q_(it->second.deploymentName.c_str()));
#endif
        continue;
      }
    }
#endif

    // create the archive file
    ArchiveFileType archiveFileType = CreateArchiveFile(p.second, repository, dbLight);

    // update database records
    dbLight.PutValue(p.second.deploymentName, "MD5", p.second.digest.ToString());
    dbLight.PutValue(p.second.deploymentName, "TimePackaged", std::to_string(p.second.timePackaged));
    dbLight.PutValue(p.second.deploymentName, "CabSize", std::to_string(static_cast<int>(p.second.archiveFileSize)));
    dbLight.PutValue(p.second.deploymentName, "CabMD5", p.second.archiveFileDigest.ToString());
    dbLight.PutValue(p.second.deploymentName, "Type",
      (archiveFileType == ArchiveFileType::MSCab ? "MSCab"
        : (archiveFileType == ArchiveFileType::TarBzip2 ? "TarBzip2"
          : (archiveFileType == ArchiveFileType::TarLzma ? "TarLzma"
            : "unknown"))));

    if (p.second.version.empty())
    {
      string oldVersion;
      if (dbLight.TryGetValue(p.second.deploymentName, "Version", oldVersion))
      {
        dbLight.DeleteValue(p.second.deploymentName, "Version");
      }
    }
    else
    {
      dbLight.PutValue(p.second.deploymentName, "Version", p.second.version);
    }
    if (p.second.targetSystem.empty())
    {
      string oldTargetSystem;
      if (dbLight.TryGetValue(p.second.deploymentName, "TargetSystem", oldTargetSystem))
      {
        dbLight.DeleteValue(p.second.deploymentName, "TargetSystem");
      }
    }
    else
    {
      dbLight.PutValue(p.second.deploymentName, "TargetSystem", p.second.targetSystem);
    }
  }
}

void PackageCreator::ReadList(const PathName & path, map<string, PackageSpec> & mapPackageList)
{
  StreamReader reader(path);
  string line;
  while (reader.ReadLine(line))
  {
    if (line.empty())
    {
      continue;
    }
    char ch = line[0];
    const char * lpsz = line.c_str() + 1;
    while (*lpsz != 0 && (*lpsz == ' ' || *lpsz == '\t'))
    {
      ++lpsz;
    }
    if (*lpsz == 0)
    {
      continue;
    }
    if (ch == '@')
    {
      ReadList(lpsz, mapPackageList);
      continue;
    }
    if (ch != 'S' && ch != 'M' && ch != 'L' && ch != 'T' && ch != '-')
    {
      continue;
    }
    Tokenizer tok(lpsz, ";");
    PackageSpec pkgspec;
    pkgspec.deploymentName = tok.GetCurrent();
    pkgspec.level = ch;
    ArchiveFileType archiveFileType = defaultArchiveFileType;
    ++tok;
    if (tok.GetCurrent() != nullptr)
    {
      if (strcmp(tok.GetCurrent(), "MSCab") == 0)
      {
        archiveFileType = ArchiveFileType::MSCab;
      }
      else if (strcmp(tok.GetCurrent(), "TarBzip2") == 0)
      {
        archiveFileType = ArchiveFileType::TarBzip2;
      }
      else if (strcmp(tok.GetCurrent(), "TarLzma") == 0)
      {
        archiveFileType = ArchiveFileType::TarLzma;
      }
      else
      {
        FatalError("Invalid package list file.");
      }
    }
    pkgspec.archiveFileType = archiveFileType;
    mapPackageList[pkgspec.deploymentName] = pkgspec;
  }
  reader.Close();
}

void PackageCreator::ReadList(const PathName & path, set<string> & setPackageList)
{
  FileStream stream(File::Open(path, FileMode::Open, FileAccess::Read));
  string line;
  while (Utils::ReadUntilDelim(line, '\n', stream.Get()))
  {
    size_t l = line.length();
    if (l == 0)
    {
      continue;
    }
    if (line[l - 1] == '\n')
    {
      --l;
      line.erase(l);
    }
    if (l == 0)
    {
      continue;
    }
    setPackageList.insert(line);
  }
  stream.Close();
}

void PackageCreator::DisassemblePackage(const PathName & packageDefinitionFile, const PathName & sourceDir, const PathName & stagingDir)
{
  // parse the package definition file
  Verbose(T_("Parsing %s..."), Q_(packageDefinitionFile.GetData()));
  PackageInfo packageInfo = PackageManager::ReadPackageDefinitionFile(packageDefinitionFile, texmfPrefix);

  // remove the package definition file from the RunFiles list
  const char * lpszRelPath = Utils::GetRelativizedPath(packageDefinitionFile.GetData(), sourceDir.GetData());
  if (lpszRelPath != nullptr)
  {
    vector<string>::iterator it;
    for (it = packageInfo.runFiles.begin(); it != packageInfo.runFiles.end(); ++it)
    {
      if (PathName::Compare(it->c_str(), lpszRelPath) == 0)
      {
        break;
      }
    }
    if (it != packageInfo.runFiles.end())
    {
      packageInfo.runFiles.erase(it);
    }
  }

  // determine the deployment name, e.g.:
  // a0poster
  char szDeploymentName[BufferSizes::MaxPath];
  packageDefinitionFile.GetFileNameWithoutExtension(szDeploymentName);
  packageInfo.deploymentName = szDeploymentName;

  Verbose(" %s (%u files)...", Q_(packageInfo.deploymentName.c_str()), static_cast<unsigned>(packageInfo.GetNumFiles()));

  // copy files and calculate checksums; the package definition file
  // has been removed from the RunFiles list
  FileDigestTable fileDigests;
  MD5CopyFiles(packageInfo.runFiles, sourceDir, nullptr, stagingDir, "Files", fileDigests);
  MD5CopyFiles(packageInfo.docFiles, sourceDir, nullptr, stagingDir, "Files", fileDigests);
  MD5CopyFiles(packageInfo.sourceFiles, sourceDir, nullptr, stagingDir, "Files", fileDigests);

  // calculate TDS digest
  MD5 tdsDigest = GetTdsDigest(fileDigests);

  // write package.ini, md5sums.txt and Description, e.g.:
  // /mypackages/a0poster/package.ini
  // /mypackages/a0poster/md5sums.txt
  // /mypackages/a0poster/Description
  InitializeStagingDirectory(stagingDir, packageInfo, fileDigests, tdsDigest);

  // write new package Definition file, e.g.:
  // /mypackages/a0poster/Files/texmf/tpm/packages/a0poster.tpm
  MpcPackageInfo mpcPackageInfo(packageInfo);
  mpcPackageInfo.digest = tdsDigest;
  mpcPackageInfo.path = stagingDir;
  CollectPackage(mpcPackageInfo);
  PathName packageDefinitionDir(stagingDir);
  packageDefinitionDir /= "Files";
  packageDefinitionDir /= texmfPrefix;
  packageDefinitionDir /= MIKTEX_PATH_PACKAGE_DEFINITION_DIR;
  Directory::Create(packageDefinitionDir);
  PackageManager::WritePackageDefinitionFile(PathName(packageDefinitionDir, packageInfo.deploymentName, MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX), mpcPackageInfo, 0);
}

void PackageCreator::Run(int argc, const char ** argv)
{
  vector<string> stagingRoots;

  PathName packageDefinitionFile;
  PathName repository;
  PathName stagingDir;
  PathName texmfParent;
  PathName tpmDir;

  bool optBuildTDS = false;
  bool optCreatePackage = false;
  bool optDisassemblePackage = false;
  bool optUpdateRepository = false;
  bool optVersion = false;

  optVerbose = false;
  programStartTime = time(nullptr);

  PoptWrapper popt(argc, argv, options);

  int option;

  while ((option = popt.GetNextOpt()) >= 0)
  {
    string optArg = popt.GetOptArg();
    switch (option)
    {
    case OPT_BUILD_TDS:
      optBuildTDS = true;
      break;
    case OPT_CREATE_PACKAGE:
      optCreatePackage = true;
      break;
    case OPT_DEFAULT_LEVEL:
      if (optArg.empty())
      {
        FatalError(T_("Missing package level."));
      }
      defaultLevel = optArg[0];
      break;
    case OPT_DISASSEMBLE_PACKAGE:
      optDisassemblePackage = true;
      break;
    case OPT_MIKTEX_SERIES:
      miktexSeries = optArg.c_str();
      if (miktexSeries.CompareTo(MIKTEX_SERIES_STR) > 0)
      {
        FatalError(T_("Unsupported MiKTeX series."));
      }
      break;
    case OPT_PACKAGE_LIST:
      ReadList(optArg, packageList);
      break;
    case OPT_RELEASE_STATE:
      releaseState = optArg;
      break;
    case OPT_REPOSITORY:
      repository = optArg;
      break;
    case OPT_STAGING_DIR:
      stagingDir = optArg;
      break;
    case OPT_STAGING_ROOTS:
    {
      for (CSVList dir(optArg, PathName::PathNameDelimiter); dir.GetCurrent() != nullptr; ++dir)
      {
        stagingRoots.push_back(dir.GetCurrent());
      }
      break;
    }
    case OPT_TEXMF_PARENT:
      texmfParent = optArg;
      break;
    case OPT_TEXMF_PREFIX:
      texmfPrefix = optArg;
      break;
    case OPT_TIME_PACKAGED:
      programStartTime = std::stoi(optArg);
      break;
    case OPT_TPM_DIR:
      tpmDir = optArg;
      break;
    case OPT_TPM_FILE:
      packageDefinitionFile = optArg;
      break;
    case OPT_UPDATE_REPOSITORY:
      optUpdateRepository = true;
      break;
    case OPT_VERBOSE:
      optVerbose = true;
      break;
    case OPT_VERSION:
      optVersion = true;
      break;
    }
  }

  if (option != -1)
  {
    string msg = popt.BadOption(POPT_BADOPTION_NOALIAS);
    msg += ": ";
    msg += popt.Strerror(option);
    FatalError("%s", msg.c_str());
  }

  if (optVersion)
  {
    cout
      << Utils::MakeProgramVersionString(TheNameOfTheGame, VersionNumber(MIKTEX_MAJOR_VERSION, MIKTEX_MINOR_VERSION, MIKTEX_COMP_J2000_VERSION, 0)) << endl
      << "Copyright (C) 1996-2016 Christian Schenk" << endl
      << "This is free software; see the source for copying conditions.  There is NO" << endl
      << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
  }
  else if (optCreatePackage)
  {
    if (stagingDir.Empty())
    {
      stagingDir.SetToCurrentDirectory();
    }
    if (repository.Empty())
    {
      FatalError(T_("No repository location was specified."));
    }
    Verbose(T_("Loading database from %s..."), Q_(repository));
    unique_ptr<Cfg> dbLight(LoadDbLight(repository));
    map<string, MpcPackageInfo> packageTable = LoadDbHeavy(repository);
    Verbose(T_("Reading staging directory %s..."), stagingDir.GetData());
    MpcPackageInfo packageInfo = InitializePackageInfo(stagingDir.GetData());
    CollectPackage(packageInfo);
    packageTable[packageInfo.deploymentName] = packageInfo;
    UpdateRepository(packageTable, repository, *dbLight);
    Verbose(T_("Writing database to %s..."), Q_(repository));
    WriteDatabase(packageTable, repository, false, *dbLight);
  }
  else if (optDisassemblePackage)
  {
    if (packageDefinitionFile.Empty())
    {
      FatalError(T_("No package definition file has been specified."));
    }
    if (texmfParent.Empty())
    {
      FatalError(T_("No TEXMF parent directory has been specified."));
    }
    if (stagingDir.Empty())
    {
      FatalError(T_("No staging directory has been specified."));
    }
    DisassemblePackage(packageDefinitionFile, texmfParent, stagingDir);
  }
  else if (optUpdateRepository || optBuildTDS)
  {
    if (stagingRoots.empty())
    {
      FatalError(T_("No staging roots were specified."));
    }

    // collect the packages
    map<string, MpcPackageInfo> packageTable;
    for (const string & r : stagingRoots)
    {
      CollectPackages(r, packageTable);
    }

    if (packageTable.empty())
    {
      FatalError(T_("No staging directories were found."));
    }

    if (optBuildTDS)
    {
      // create a TeX directory structure
      if (texmfParent.Empty())
      {
        FatalError(T_("No TEXMF parent directory has been specified."));
      }
      unique_ptr<Cfg> dbLight(Cfg::Create());
      BuildTDS(packageTable, texmfParent, *dbLight);
      if (!tpmDir.Empty())
      {
        WritePackageDefinitionFiles(packageTable, tpmDir, *dbLight);
      }
      // write mpm.ini
      PathName iniFile(texmfParent);
      iniFile /= texmfPrefix;
      iniFile /= MIKTEX_PATH_MPM_INI;
      dbLight->Write(iniFile);
    }
    else if (optUpdateRepository)
    {
      if (repository.Empty())
      {
        FatalError(T_("No repository location was specified."));
      }
      // load light-weight database
      unique_ptr<Cfg> dbLight(LoadDbLight(repository));
      UpdateRepository(packageTable, repository, *dbLight);
      WriteDatabase(packageTable, repository, true, *dbLight);
    }
  }
  else
  {
    FatalError(T_("No task was specified."));
  }
}

int main(int argc, const char ** argv)
{
  int result;

  try
  {
    PackageCreator app;
    app.Init(argv[0]);
    app.Run(argc, argv);
    app.Finalize();
    result = 0;
  }

  catch (const MiKTeXException & e)
  {
    Utils::PrintException(e);
    result = 1;
  }

  catch (const exception & e)
  {
    Utils::PrintException(e);
    result = 1;
  }

  catch (int i)
  {
    result = i;
  }

  return result;
}
