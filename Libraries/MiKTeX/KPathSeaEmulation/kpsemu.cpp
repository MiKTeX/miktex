/* kpsemu.cpp: kpathsea emulation

   Copyright (C) 1994, 95 Karl Berry
   Copyright (C) 2000-2019 Christian Schenk

   This file is part of the MiKTeX KPSEMU Library.

   The MiKTeX KPSEMU Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX KPSEMU Library is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX KPSEMU Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "config.h"

#include <cstdarg>
#include <cstdlib>

#include <iostream>
#include <memory>
#include <string>

#if defined(MIKTEX_UNIX)
#include <sys/time.h>
#endif

#include <miktex/App/Application>
#include <miktex/C4P/C4P>
#include <miktex/Core/BufferSizes>
#include <miktex/Core/Cfg>
#include <miktex/Core/ConfigNames>
#include <miktex/Core/File>
#include <miktex/Core/FileType>
#include <miktex/Core/Fndb>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Registry>
#include <miktex/KPSE/Emulation>
#include <miktex/TeXAndFriends/Prototypes>
#include <miktex/Util/CharBuffer>
#include <miktex/Version>

#include "internal.h"

using namespace std;

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace MiKTeX;

namespace {
  unsigned kpse_baseResolution = 600;
  std::string kpse_mode;
}

MIKTEXKPSDATA(const char*) miktex_kpathsea_version_string = KPSEVERSION;

MIKTEXKPSDATA(const char*) miktex_kpathsea_bug_address = T_("Visit miktex.org for bug reports.");

#if defined(KPSE_COMPAT_API)

MIKTEXKPSDATA(kpathsea_instance) miktex_kpse_def_inst;

MIKTEXKPSDATA(kpathsea) miktex_kpse_def = &miktex_kpse_def_inst;

#endif

MIKTEXSTATICFUNC(char*) ToUnix(char* s)
{
  char* ret = s;
#if defined(MIKTEX_WINDOWS)
  for (; *s != 0; ++s)
  {
    if (*s == '\\')
    {
      *s = '/';
    }
  }
#endif
  return ret;
}

// FIXME: duplicate
MIKTEXSTATICFUNC(int) magstep(int n, int bdpi)
{
  double t;
  bool neg = false;
  if (n < 0)
  {
    neg = true;
    n = -n;
  }
  if (n & 1)
  {
    n &= ~1;
    t = 1.095445115;
  }
  else
  {
    t = 1.0;
  }
  while (n > 8)
  {
    n -= 8;
    t *= 2.0736;
  }
  while (n > 0)
  {
    n -= 2;
    t *= 1.2;
  }
  if (neg)
  {
    return static_cast<int>(0.5 + bdpi / t);
  }
  else
  {
    return static_cast<int>(0.5 + bdpi * t);
  }
}

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_find_glyph(kpathsea kpseInstance, const char* fontName, unsigned dpi, kpse_file_format_type format, kpse_glyph_file_type* glyph_file)
{
  if (format != kpse_pk_format)
  {
    // TODO
    return nullptr;
  }
  PathName path;
  shared_ptr<Session> session = Session::Get();
  if (!session->FindPkFile(fontName, kpse_mode, dpi, path))
  {
    Application* app = Application::GetApplication();
    if (app == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    PathName pathMakePk;
    vector<std::string> arguments = session->MakeMakePkCommandLine(fontName, dpi, kpse_baseResolution, kpse_mode, pathMakePk, app->GetEnableInstaller());
    if (!(Process::Run(pathMakePk, arguments, nullptr, nullptr, nullptr) && session->FindPkFile(fontName, kpse_mode, dpi, path)))
    {
      return nullptr;
    }
  }
  char* ret = xstrdup(path.GetData());
  if (glyph_file != nullptr)
  {
    glyph_file->name = const_cast<char*>(fontName);
    glyph_file->dpi = dpi;
    glyph_file->format = format;
    glyph_file->source = kpse_glyph_source_normal;
  }
  return ret;
}

MIKTEXSTATICFUNC(const char**) ToStringList(const std::string& str)
{
  vector<std::string> vec = StringUtil::Split(str, PathName::PathNameDelimiter);
  const char** result = XTALLOC(vec.size() + 1, const char*);
  size_t idx = 0;
  for (const std::string& s : vec)
  {
    result[idx++] = xstrdup(s.c_str());
  }
  result[idx] = nullptr;
  return result;
}

MIKTEXSTATICFUNC(FileType) ToFileType(kpse_file_format_type format)
{
  FileType ft(FileType::None);
  switch (format)
  {
  case kpse_afm_format: ft = FileType::AFM; break;
  case kpse_any_glyph_format: ft = FileType::GLYPHFONT; break;
  case kpse_base_format: ft = FileType::BASE; break;
  case kpse_bib_format: ft = FileType::BIB; break;
  case kpse_bst_format: ft = FileType::BST; break;
  case kpse_cid_format: ft = FileType::CID; break;
  case kpse_clua_format: ft = FileType::CLUA; break;
  case kpse_cmap_format: ft = FileType::CMAP; break;
  case kpse_cnf_format: ft = FileType::CNF; break;
  case kpse_cweb_format: ft = FileType::CWEB; break;
  case kpse_db_format: ft = FileType::DB; break;
  case kpse_dvips_config_format: ft = FileType::DVIPSCONFIG; break;
  case kpse_enc_format: ft = FileType::ENC; break;
  case kpse_fea_format: ft = FileType::FEA; break;
  case kpse_fontmap_format: ft = FileType::MAP; break;
  case kpse_fmt_format: ft = FileType::FMT; break;
  case kpse_gf_format: ft = FileType::GF; break;
  case kpse_ist_format: ft = FileType::IST; break;
  case kpse_lig_format: ft = FileType::LIG; break;
  case kpse_lua_format: ft = FileType::LUA; break;
  case kpse_mem_format: ft = FileType::MEM; break;
  case kpse_mf_format: ft = FileType::MF; break;
  case kpse_mfpool_format: ft = FileType::MFPOOL; break;
  case kpse_mft_format: ft = FileType::MFT; break;
  case kpse_mlbib_format: ft = FileType::MLBIB; break;
  case kpse_mlbst_format: ft = FileType::MLBST; break;
  case kpse_miscfonts_format: ft = FileType::MISCFONT; break;
  case kpse_mp_format: ft = FileType::MP; break;
  case kpse_mppool_format: ft = FileType::MPPOOL; break;
  case kpse_mpsupport_format: ft = FileType::MPSUPPORT; break;
  case kpse_ocp_format: ft = FileType::OCP; break;
  case kpse_ofm_format: ft = FileType::OFM; break;
  case kpse_opentype_format: ft = FileType::OTF; break;
  case kpse_opl_format: ft = FileType::OPL; break;
  case kpse_otp_format: ft = FileType::OTP; break;
  case kpse_ovf_format: ft = FileType::OVF; break;
  case kpse_ovp_format: ft = FileType::OVP; break;
  case kpse_pdftex_config_format: ft = FileType::PDFTEXCONFIG; break;
  case kpse_pict_format: ft = FileType::GRAPHICS; break;
  case kpse_pk_format: ft = FileType::PK; break;
  case kpse_program_binary_format: ft = FileType::PROGRAMBINFILE; break;
  case kpse_program_text_format: ft = FileType::PROGRAMTEXTFILE; break;
  case kpse_sfd_format: ft = FileType::SFD; break;
  case kpse_tex_format: ft = FileType::TEX; break;
  case kpse_tex_ps_header_format: ft = FileType::PSHEADER; break;
  case kpse_texpool_format: ft = FileType::TEXPOOL; break;
  case kpse_texsource_format: ft = FileType::TEXSOURCE; break;
  case kpse_texdoc_format: ft = FileType::TEXSYSDOC; break;
  case kpse_texmfscripts_format: ft = FileType::SCRIPT; break;
  case kpse_tfm_format: ft = FileType::TFM; break;
  case kpse_troff_font_format: ft = FileType::TROFF; break;
  case kpse_truetype_format: ft = FileType::TTF; break;
  case kpse_type1_format: ft = FileType::TYPE1; break;
  case kpse_type42_format: ft = FileType::TYPE42; break;
  case kpse_vf_format: ft = FileType::VF; break;
  case kpse_web2c_format: ft = FileType::WEB2C; break;
  case kpse_web_format: ft = FileType::WEB; break;
  default:
    MIKTEX_UNEXPECTED();
  }
  return ft;
}

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_find_file(kpathsea kpseInstance, const char* fileName, kpse_file_format_type format, int mustExist)
{
  MIKTEX_ASSERT(kpseInstance != nullptr);
  MIKTEX_ASSERT(fileName != nullptr);
  bool found = false;
  PathName result;
  shared_ptr<Session> session = Session::Get();
  FileType ft = ToFileType(format);
  Session::FindFileOptionSet options;
  if (mustExist)
  {
    options += Session::FindFileOption::Create;
    options += Session::FindFileOption::TryHard;
  }
  found = session->FindFile(fileName, ft, options, result);
  if (!found)
  {
    return nullptr;
  }
  result.ConvertToUnix();
  return xstrdup(result.GetData());
}

MIKTEXKPSCEEAPI(char**) miktex_kpathsea_find_file_generic(kpathsea kpseInstance, const char* fileName, kpse_file_format_type format, boolean mustExist, boolean all)
{
  MIKTEX_ASSERT(kpseInstance != nullptr);
  MIKTEX_ASSERT(fileName != nullptr);
  bool found = false;
  vector<PathName> result;
  FileType fileType = ToFileType(format);
  Session::FindFileOptionSet options;
  if (all)
  {
    options += Session::FindFileOption::All;
  }
  if (mustExist)
  {
    options += Session::FindFileOption::Create;
    options += Session::FindFileOption::TryHard;
  }
  shared_ptr<Session> session = Session::Get();
  found = session->FindFile(fileName, fileType, options, result);
  if (!found)
  {
    return nullptr;
  }
  char** stringList = XTALLOC(result.size() + 1, char*);
  size_t idx = 0;
  for (PathName& p : result)
  {
    p.ConvertToUnix();
    stringList[idx++] = xstrdup(p.GetData());
  }
  stringList[idx] = nullptr;
  return stringList;
}

MIKTEXSTATICFUNC(bool) IsBinary(kpse_file_format_type format)
{
  switch (format)
  {
  case kpse_tfm_format:
  case kpse_miscfonts_format:
  case kpse_pict_format:
  case kpse_type1_format:
  case kpse_vf_format:
  case kpse_otp_format:
  case kpse_ocp_format:
  case kpse_truetype_format:
  case kpse_opentype_format:
    return true;
  default:
    return false;
  }
}

MIKTEXSTATICFUNC(void) TranslateModeString(const char* modeString, FileMode& mode, FileAccess& access, bool& isTextFile)
{
  if (Utils::Equals(modeString, "r"))
  {
    mode = FileMode::Open;
    access = FileAccess::Read;
    isTextFile = true;
  }
  else if (Utils::Equals(modeString, "w"))
  {
    mode = FileMode::Create;
    access = FileAccess::Write;
    isTextFile = true;
  }
  else if (Utils::Equals(modeString, "rb"))
  {
    mode = FileMode::Open;
    access = FileAccess::Read;
    isTextFile = false;
  }
  else if (Utils::Equals(modeString, "wb"))
  {
    mode = FileMode::Create;
    access = FileAccess::Write;
    isTextFile = false;
  }
  else if (Utils::Equals(modeString, "ab"))
  {
    mode = FileMode::Append;
    access = FileAccess::Write;
    isTextFile = false;
  }
  else
  {
    MIKTEX_UNEXPECTED();
  }
}

MIKTEXSTATICFUNC(FILE*) FOpen(const char* fileName, const char* modeString)
{
  shared_ptr<Session> session = Session::Get();
  FileMode mode(FileMode::Open);
  FileAccess access(FileAccess::Read);
  bool isTextFile;
  TranslateModeString(modeString, mode, access, isTextFile);
  return session->OpenFile(fileName, mode, access, isTextFile);
}

MIKTEXKPSCEEAPI(FILE*) miktex_kpathsea_open_file(kpathsea kpseInstance, const char* fileName, kpse_file_format_type format)
{
  MIKTEX_ASSERT(fileName != nullptr);
  auto path = std::unique_ptr<char, decltype(free)*>{ kpse_find_file(fileName, format, 1), free };
  if (path == nullptr)
  {
    MIKTEX_FATAL_ERROR_2(T_("File '{fileName}' not found."), "fileName", fileName);
  }
  return FOpen(path.get(), IsBinary(format) ? "rb" : "r");
}

MIKTEXKPSCEEAPI(char*) miktex_concatn(const char* s1, ...)
{
  va_list marker;
  va_start(marker, s1);
  CharBuffer<char> buf;
  for (const char* s = s1; s != nullptr; s = va_arg(marker, const char*))
  {
    buf.Append(s);
  }
  char* ret = xstrdup(buf.GetData());
  va_end(marker);
  return ret;
}

MIKTEXKPSCEEAPI(const char*) miktex_xbasename(const char* fileName)
{
  const char* s = fileName + StrLen(fileName);
  while (s != fileName)
  {
    --s;
    if (IsDirectoryDelimiter(*s) || *s == ':')
    {
      return s + 1;
    }
  }
  return fileName;
}

MIKTEXKPSCEEAPI(char*) miktex_xdirname(const char* fileName)
{
  return xstrdup(PathName(fileName).RemoveFileSpec().GetData());
}

MIKTEXKPSCEEAPI(int) miktex_strcasecmp(const char* s1, const char* s2)
{
  return StringCompare(s1, s2, true);
}

MIKTEXKPSCEEAPI(int) miktex_xfseek(FILE* file, long offset, int where, const char* path)
{
  if (fseek(file, offset, where) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("fseek", "path", path);
  }
  return 0;
}

MIKTEXKPSCEEAPI(int) miktex_xfseeko(FILE* file, off_t offset, int where, const char* path)
{
  if (fseek(file, static_cast<long>(offset), where) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("fseek", "path", path);
  }
  return 0;
}


MIKTEXKPSCEEAPI(int) miktex_xfseeko64(FILE* file, MIKTEX_INT64 offset, int where, const char* path)
{
#if defined(_MSC_VER)
  if (_fseeki64(file, offset, where) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("_fseeki64", "path", path);
  }
  return 0;
#elif defined(HAVE_FSEEKO64)
  if (fseeko64(file, offset, where) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("fseeko64", "path", path);
  }
  return 0;
#else
  if (sizeof(off_t) != sizeof(MIKTEX_INT64))
  {
    MIKTEX_UNEXPECTED();
  }
  if (fseeko(file, offset, where) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("fseeko", "path", path);
  }
  return 0;
#endif
}

MIKTEXKPSCEEAPI(long) miktex_xftell(FILE* file, const char* path)
{
  long pos = ftell(file);
  if (pos < 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("ftell", "path", path);
  }
  return pos;
}

MIKTEXKPSCEEAPI(off_t) miktex_xftello(FILE* file, const char* path)
{
  long pos = ftell(file);
  if (pos < 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("ftell", "path", path);
  }
  return pos;
}

MIKTEXKPSCEEAPI(MIKTEX_INT64) miktex_xftello64(FILE* file, const char* path)
{
#if defined(_MSC_VER)
  MIKTEX_INT64 pos = _ftelli64(file);
  if (pos < 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("_ftelli64", "path", path);
  }
  return pos;
#elif defined(HAVE_FTELLO64)
  MIKTEX_INT64 pos = ftello64(file);
  if (pos < 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("ftello64", "path", path);
  }
  return pos;
#else
  if (sizeof(off_t) != sizeof(MIKTEX_INT64))
  {
    MIKTEX_UNEXPECTED();
  }
  MIKTEX_INT64 pos = ftello(file);
  if (pos < 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("ftello", "path", path);
  }
  return pos;
#endif
}

MIKTEXKPSCEEAPI(void) miktex_xfclose(FILE* file, const char* path)
{
  shared_ptr<Session> session = Session::Get();
  session->CloseFile(file);
}

MIKTEXKPSCEEAPI(FILE*) miktex_xfopen(const char* path, const char* modeString)
{
  return FOpen(path, modeString);
}

MIKTEXKPSCEEAPI(char*) miktex_xgetcwd()
{
  PathName path;
  path.SetToCurrentDirectory();
  return xstrdup(path.GetData());
}

// [KB] The tolerances change whether we base things on DPI1 or DPI2.
#define KPSE_BITMAP_TOLERANCE(r) ((r) / 500.0 + 1)

MIKTEXKPSCEEAPI(int) miktex_kpathsea_bitmap_tolerance(kpathsea pKpathseaInstance, double dpi1, double dpi2)
{
  unsigned tolerance = static_cast<unsigned>(KPSE_BITMAP_TOLERANCE(dpi2));
  unsigned lower_bound = (static_cast<int>(dpi2 - tolerance) < 0 ? 0 : static_cast<unsigned>(dpi2 - tolerance));
  unsigned upper_bound = static_cast<unsigned>(dpi2 + tolerance);
  return lower_bound <= dpi1 && dpi1 <= upper_bound;
}

/* [KB]: This is adapted from code written by Tom Rokicki for dvips.
   It's part of Kpathsea now so all the drivers can use it.  The idea
   is to return the true dpi corresponding to DPI with a base
   resolution of BDPI.  If M_RET is non-null, we also set that to the
   mag value.

   [KB]: Don't bother trying to use fabs or some other ``standard''
   routine which can only cause trouble; just roll our own
   simple-minded absolute-value function that is all we need. */

#undef ABS
#define ABS(expr) ((expr) < 0 ? -(expr) : (expr))

#define MAGSTEP_MAX 40

MIKTEXKPSCEEAPI(unsigned) miktex_kpathsea_magstep_fix(kpathsea pKpathseaInstance, unsigned dpi, unsigned bdpi, int* m_ret)
{
  int m;
  int mdpi = -1;
  unsigned real_dpi = 0;
  int sign = dpi < bdpi ? -1 : 1;

  for (m = 0; !real_dpi && m < MAGSTEP_MAX; ++m)
  {
    mdpi = magstep(m * sign, bdpi);
    if (ABS(mdpi - (int)dpi) <= 1)
    {
      real_dpi = mdpi;
    }
    else if ((mdpi - (int)dpi) * sign > 0)
    {
      real_dpi = dpi;
    }
  }

  if (m_ret)
  {
    *m_ret = real_dpi == (unsigned)(mdpi ? (m - 1) * sign : 0);
  }

  return real_dpi ? real_dpi : dpi;
}

MIKTEXKPSCEEAPI(void) miktex_kpathsea_init_prog(kpathsea pKpathseaInstance, const char* prefix, unsigned dpi, const char* modeString, const char* fallback)
{
  kpse_baseResolution = dpi;
  if (modeString != nullptr)
  {
    kpse_mode = modeString;
  }
}

MIKTEXKPSCEEAPI(void) miktex_kpathsea_set_program_name(kpathsea kpseInstance, const char* argv0_, const char* programName_)
{
  shared_ptr<Session> session = Session::Get();
  if (kpseInstance->invocation_name != nullptr)
  {
    MIKTEX_FREE(kpseInstance->invocation_name);
  }
  PathName argv0(argv0_);
  kpseInstance->invocation_name = xstrdup(argv0.GetFileName().GetData());
  std::string programName;
  if (programName_ == nullptr)
  {
    programName = Utils::GetExeName();
    programName_ = programName.c_str();
  }
  if (kpseInstance->program_name != nullptr)
  {
    MIKTEX_FREE(kpseInstance->program_name);
  }
  kpseInstance->program_name = xstrdup(programName_);
  if (Utils::GetExeName() != programName_)
  {
    session->PushAppName(programName_);
  }
  Utils::SetEnvironmentString("progname", programName_);
}

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_program_basename(const char* argv0_)
{
  PathName argv0(argv0_);
  return xstrdup(argv0.GetFileNameWithoutExtension().GetData());
}

MIKTEXKPSCEEAPI(void) miktex_kpathsea_set_program_enabled(kpathsea kpseInstance, kpse_file_format_type fmt, boolean value, kpse_src_type level)
{
}

MIKTEXKPSCEEAPI(char*) miktex_find_suffix(const char* path)
{
  const char* ext = nullptr;
  for (; *path != 0; ++path)
  {
    if (IsDirectoryDelimiter(*path))
    {
      ext = nullptr;
    }
#if defined(MIKTEX_WINDOWS)
    else if (*path == ':')
    {
      ext = nullptr;
    }
#endif
    else if (*path == '.')
    {
      ext = path + 1;
    }
  }
  return ext == nullptr ? nullptr : const_cast<char*>(ext);
}

MIKTEXKPSCEEAPI(char*) miktex_remove_suffix(const char* path)
{
  char* ret;
  char* ext = find_suffix(path);
  if (ext == nullptr)
  {
    ret = const_cast<char*>(path);
  }
  else
  {
    MIKTEX_ASSERT(ext > path);
    --ext;
    MIKTEX_ASSERT(*ext == '.');
    size_t n = (ext - path);
    ret = reinterpret_cast<char*>(xmalloc((n + 1) * sizeof(*ret)));
    // FIXME: don't use strncpy
    strncpy(ret, path, n);
    ret[n] = 0;
  }
  return ret;
}

MIKTEXSTATICFUNC(std::string) HideMpmRoot(const std::string& searchPath)
{
  shared_ptr<Session> session = Session::Get();
  PathName mpmRootPath = session->GetMpmRootPath();
  size_t mpmRootPathLen = mpmRootPath.GetLength();
  std::string result;
  for (const std::string& path : StringUtil::Split(searchPath, PathName::PathNameDelimiter))
  {
    if ((PathName::Compare(path, mpmRootPath, mpmRootPathLen) == 0) && (path.length() == mpmRootPathLen || IsDirectoryDelimiter(path[mpmRootPathLen])))
    {
      continue;
    }
    if (!result.empty())
    {
      result += PathName::PathNameDelimiter;
    }
    result += path;
  }
  return result;
}

MIKTEXSTATICFUNC(bool) VarValue(const std::string& varName, std::string& varValue)
{
  shared_ptr<Session> session = Session::Get();
  PathName path;
  bool result = false;
  // read-only values
  if (varName == "OPENTYPEFONTS")
  {
    FileTypeInfo fti = session->GetFileTypeInfo(FileType::OTF);
    varValue = StringUtil::Flatten(fti.searchPath, PathName::PathNameDelimiter);
    result = true;
  }
  else if (varName == "TTFONTS")
  {
    FileTypeInfo fti = session->GetFileTypeInfo(FileType::TTF);
    varValue = StringUtil::Flatten(fti.searchPath, PathName::PathNameDelimiter);
    result = true;
  }
  else if (varName == "SELFAUTOLOC")
  {
    path = session->GetMyLocation(false);
    varValue = path.ToUnix().ToString();
    result = true;
  }
  else if (varName == "SELFAUTODIR")
  {
    result = false;
  }
  else if (varName == "SELFAUTOPARENT")
  {
    result = false;
  }
  else if (varName == "TEXMF")
  {
    vector<std::string> roots;
    for (const auto& r : session->GetRootDirectories())
    {
      roots.push_back(r.path.ToUnix().ToString());
    }
    varValue = "{" + StringUtil::Flatten(roots, ',') + "}";
    result = true;
  }
  else if (varName == "TEXMFCONFIG")
  {
    path = session->GetSpecialPath(session->IsAdminMode() ? SpecialPath::CommonConfigRoot : SpecialPath::UserConfigRoot);
    varValue = path.ToUnix().ToString();
    result = true;
  }
  else if (varName == "TEXMFDIST" || varName == "TEXMFMAIN")
  {
    path = session->GetSpecialPath(SpecialPath::DistRoot);
    varValue = path.ToUnix().ToString();
    result = true;
  }
  else if (varName == "TEXMFHOME")
  {
    vector<std::string> userRoots;
    for (const auto& r : session->GetRootDirectories())
    {
      if (!r.IsCommon() && !r.IsManaged())
      {
        userRoots.push_back(r.path.ToUnix().ToString());
      }
    }
    if (userRoots.size() == 1)
    {
      varValue = userRoots[0];
      result = true;
    }
    else if(userRoots.size() > 1)
    {
      varValue = "{" + StringUtil::Flatten(userRoots, ',') + "}";
      result = true;
    }
  }
  else if (varName == "TEXMFLOCAL")
  {
    vector<std::string> commonRoots;
    for (const auto& r : session->GetRootDirectories())
    {
      if (r.IsCommon() && !r.IsManaged())
      {
        commonRoots.push_back(r.path.ToUnix().ToString());
      }
}
    if (commonRoots.size() == 1)
    {
      varValue = commonRoots[0];
      result = true;
    }
    else if (commonRoots.size() > 1)
    {
      varValue = "{" + StringUtil::Flatten(commonRoots, ',') + "}";
      result = true;
    }
  }
  else if (varName == "TEXMFSYSCONFIG")
  {
    path = session->GetSpecialPath(SpecialPath::CommonConfigRoot);
    varValue = path.ToUnix().ToString();
    result = true;
  }
  else if (varName == "TEXMFSYSVAR")
  {
    path = session->GetSpecialPath(SpecialPath::CommonDataRoot);
    varValue = path.ToUnix().ToString();
    result = true;
  }
  else if (varName == "TEXMFVAR")
  {
    path = session->GetSpecialPath(session->IsAdminMode() ? SpecialPath::CommonDataRoot : SpecialPath::UserDataRoot);
    varValue = path.ToUnix().ToString();
    result = true;
  }
  else if (varName == "TEXSYSTEM")
  {
    varValue = "miktex";
    result = true;
  }
  else if (varName == "shell_escape")
  {
    switch (session->GetShellCommandMode())
    {
    case ShellCommandMode::Forbidden:
      varValue = "f";
      break;
    case ShellCommandMode::Query:
    case ShellCommandMode::Restricted:
      varValue = "p";
      break;
    case ShellCommandMode::Unrestricted:
      varValue = "t";
      break;
    }
    result = true;
  }
  else if (varName == "shell_escape_commands")
  {
    varValue = StringUtil::Flatten(session->GetAllowedShellCommands(), ',');
    result = true;
  }
  // configuration files and environment
  else if (session->TryGetConfigValue("", varName, varValue))
  {
    result = true;
  }
#if defined (MIKTEX_WINDOWS)
  else if (varName == "OSFONTDIR")
  {
    // TODO: Unicode API
    if (GetWindowsDirectoryA(path.GetData(), static_cast<UINT>(path.GetCapacity())) > 0)
    {
      path /= "Fonts";
      varValue = path.ToUnix().ToString();
      result = true;
    }
  }
#endif
  return result;
}

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_var_value(kpathsea kpseInstance, const char* varName)
{
  std::string varValue;
  return VarValue(varName, varValue) ? xstrdup(varValue.c_str()) : nullptr;
}

class VarExpand : public HasNamedValues
{
public:
  bool TryGetValue(const std::string& valueName, std::string& varValue) override
  {
    return VarValue(valueName, varValue);
  }
  std::string GetValue(const std::string& valueName) override
  {
    MIKTEX_UNEXPECTED();
  }
};

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_var_expand(kpathsea kpseInstance, const char* source)
{
  VarExpand varExpand;
  return xstrdup(Session::Get()->Expand(source, &varExpand).c_str());
}

MIKTEXKPSCEEAPI(void) miktex_kpathsea_xputenv(kpathsea kpseInstance, const char* varName, const char* value)
{
  Utils::SetEnvironmentString(varName, value);
}

MIKTEXKPSCEEAPI(int) miktex_kpathsea_in_name_ok(kpathsea kpseInstance, const char* fileName, int silent)
{
  int ret = Session::Get()->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_ALLOWUNSAFEINPUTFILES).GetBool() || Utils::IsSafeFileName(fileName, true)
    ? 1
    : 0;
  if (ret == 0 && silent == 0)
  {
    MIKTEX_ASSERT(kpseInstance != nullptr &&  kpseInstance->invocation_name != nullptr);
    cerr << "\n" << kpseInstance->invocation_name << ": " << T_("file not readable for security reasons:") << " " << fileName << endl;
  }
  return ret;
}

MIKTEXKPSCEEAPI(int) miktex_kpathsea_out_name_ok(kpathsea kpseInstance, const char* fileName, int silent)
{
  int ret = Session::Get()->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_ALLOWUNSAFEOUTPUTFILES).GetBool() || Utils::IsSafeFileName(fileName, false)
    ? 1
    : 0;
  if (ret == 0 && silent == 0)
  {
    MIKTEX_ASSERT(kpseInstance != nullptr &&  kpseInstance->invocation_name != nullptr);
    cerr << "\n" << kpseInstance->invocation_name << ": " << T_("file not writable for security reasons:") << " " << fileName << endl;
  }
  return ret;
}

MIKTEXKPSCEEAPI(boolean) miktex_kpathsea_absolute_p(kpathsea kpseInstance, const char* fileName, boolean relativeOk)
{
  MIKTEX_ASSERT(relativeOk == 0);
  return Utils::IsAbsolutePath(fileName);
}

MIKTEXKPSCEEAPI(void) miktex_str_list_add(str_list_type* stringLIst, char* s)
{
  ++stringLIst->length;
  stringLIst->list = reinterpret_cast<char**>(xrealloc(stringLIst->list, sizeof(char*) * stringLIst->length));
  stringLIst->list[stringLIst->length - 1] = s;
}

MIKTEXKPSCEEAPI(kpathsea) miktex_kpathsea_new()
{
  kpathsea kpseInstance = reinterpret_cast<kpathsea>(xmalloc(sizeof(kpathsea_instance)));
  memset(kpseInstance, 0, sizeof(*kpseInstance));
  return kpseInstance;
}

#if 1
#  define DOIT(x)
#else
#  define DOIT(x) (x)
#endif

MIKTEXSTATICFUNC(void) Free(char** stringLIst)
{
  if (stringLIst != nullptr)
  {
    for (char** p = stringLIst; *p != nullptr; ++p)
    {
      MIKTEX_FREE(p);
    }
    MIKTEX_FREE(stringLIst);
  }
}

MIKTEXSTATICFUNC(void) Reset(kpse_format_info_type* formatInfoTable, size_t size)
{
  for (int idx = 0; idx < size; ++idx)
  {
    kpse_format_info_type& formatInfo = formatInfoTable[idx];
    MIKTEX_FREE(const_cast<char*>(formatInfo.path));
    MIKTEX_FREE(const_cast<char*>(formatInfo.type));
    Free(const_cast<char**>(formatInfo.suffix));
    Free(const_cast<char**>(formatInfo.alt_suffix));
    memset(&formatInfo, 0, sizeof(FormatInfo));
  }
}

MIKTEXKPSCEEAPI(void) miktex_kpathsea_finish(kpathsea kpseInstance)
{
  DOIT(MIKTEX_FREE(const_cast<char*>(kpseInstance->fallback_resolutions_string)));
  DOIT(MIKTEX_FREE(const_cast<char*>(kpseInstance->program_name)));
  DOIT(MIKTEX_FREE(const_cast<char*>(kpseInstance->invocation_name)));
  DOIT(Reset(kpseInstance->format_info, sizeof(kpseInstance->format_info) / sizeof(kpseInstance->format_info[0])));
  if (kpseInstance != &miktex_kpse_def_inst)
  {
    MIKTEX_FREE(kpseInstance);
  }
}

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_brace_expand(kpathsea kpseInstance, const char* path)
{
  shared_ptr<Session> session = Session::Get();
  VarExpand expander;
  std::string result = session->Expand(path, { ExpandOption::Values, ExpandOption::Braces }, &expander);
  return xstrdup(result.c_str());
}

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_path_expand(kpathsea kpseInstance, const char* path)
{
  shared_ptr<Session> session = Session::Get();
  VarExpand expander;
  std::string result = session->Expand(path, { ExpandOption::Values, ExpandOption::Braces, ExpandOption::PathPatterns }, &expander);
  return xstrdup(result.c_str());
}

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_readable_file(kpathsea kpseInstance, const char* fileName)
{
  if (File::Exists(fileName))
  {
    return const_cast<char*>(fileName);
  }
  else
  {
    return nullptr;
  }
}

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_path_search(kpathsea kpseInstance, const char* path, const char* fileName, boolean mustExist)
{
  shared_ptr<Session> session = Session::Get();
  PathName result;
  Session::FindFileOptionSet options;
  if (mustExist)
  {
    options += Session::FindFileOption::Create;
    options += Session::FindFileOption::TryHard;
  }
  if (!session->FindFile(fileName, path, options, result))
  {
    return nullptr;
  }
  return xstrdup(result.ToUnix().GetData());
}

MIKTEXKPSCEEAPI(char**) miktex_kpathsea_all_path_search(kpathsea kpseInstance, const char* path, const char* fileName)
{
  shared_ptr<Session> session = Session::Get();
  vector<PathName> result;
  Session::FindFileOptionSet options;
  options += Session::FindFileOption::All;
  options += Session::FindFileOption::Create;
  options += Session::FindFileOption::TryHard;
  if (!session->FindFile(fileName, path, options, result))
  {
    return nullptr;
  }
  char** stringList = XTALLOC(result.size() + 1, char*);
  for (int idx = 0; idx < result.size(); ++idx)
  {
    result[idx].ConvertToUnix();
    stringList[idx] = xstrdup(result[idx].GetData());
  }
  stringList[result.size()] = nullptr;
  return stringList;
}

MIKTEXKPSCEEAPI(void) miktex_kpathsea_maketex_option(kpathsea kpseInstance, const char* formatName, boolean value)
{
  // TODO
}

MIKTEXKPSCEEAPI(char*) miktex_kpathsea_selfdir(kpathsea kpseInstance, const char* argv0)
{
  shared_ptr<Session> session = Session::Get();
  return xstrdup(session->GetMyLocation(false).GetData());
}

MIKTEXKPSCEEAPI(char*) miktex_uppercasify(const char* s)
{
  char* result = xstrdup(s);
  for (char* s2 = result; *s2 != 0; ++s2)
  {
    *s2 = toupper(*s2);
  }
  return result;
}

MIKTEXKPSCEEAPI(char*) miktex_read_line(FILE* file)
{
  std::string line;
  Utils::ReadLine(line, file, false);
  return xstrdup(line.c_str());
}

MIKTEXKPSCEEAPI(const char*) miktex_kpathsea_init_format(kpathsea kpseInstance, kpse_file_format_type format)
{
  MIKTEX_ASSERT(kpseInstance != nullptr);
  MIKTEX_ASSERT(format < kpse_last_format);
  kpse_format_info_type& formatInfo = kpseInstance->format_info[format];
  if (formatInfo.path == nullptr)
  {
    shared_ptr<Session> session = Session::Get();
    FileType ft = ToFileType(format);
    FileTypeInfo fti = session->GetFileTypeInfo(ft);
    VarExpand expander;
    std::string searchPath = HideMpmRoot(session->Expand(StringUtil::Flatten(fti.searchPath, PathName::PathNameDelimiter), { ExpandOption::Values, ExpandOption::Braces }, &expander));
    formatInfo.path = ToUnix(xstrdup(searchPath.c_str()));
    formatInfo.type = xstrdup(fti.fileTypeString.c_str());
    formatInfo.suffix = ToStringList(StringUtil::Flatten(fti.fileNameExtensions, PathName::PathNameDelimiter));
  }
  return formatInfo.path;
}

#if WITH_CONTEXT_SUPPORT
MIKTEXKPSCEEAPI(char*) miktex_kpsemu_create_texmf_cnf()
{
  shared_ptr<Session> session = Session::Get();
  PathName texmfcnfdir = session->GetSpecialPath(SpecialPath::UserConfigRoot);
  texmfcnfdir += MIKTEX_PATH_WEB2C_DIR;
  PathName texmfcnf = texmfcnfdir;
  texmfcnf += "texmf.cnf";
  time_t lastMaintenance = static_cast<time_t>(0);
  std::string value;
  if (session->TryGetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_LAST_ADMIN_MAINTENANCE, value))
  {
    long long intValue = _atoi64(value.c_str()); // fixme
    if (static_cast<time_t>(intValue) > lastMaintenance)
    {
      lastMaintenance = static_cast<time_t>(intValue);
    }
  }
  if (session->TryGetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_LAST_USER_MAINTENANCE, value))
  {
    long long intValue = _atoi64(value.c_str()); // fixme
    if (static_cast<time_t>(intValue) > lastMaintenance)
    {
      lastMaintenance = static_cast<time_t>(intValue);
    }
  }
  if (!File::Exists(texmfcnf) || lastMaintenance > File::GetLastWriteTime(texmfcnf))
  {
    Directory::Create(texmfcnfdir);
    Core::StreamWriter stream(texmfcnf);
    static const char* const lpszVars[] = {
      "TEXMFCONFIG", "TEXMFVAR", "TEXMFHOME", "TEXMFSYSCONFIG", "TEXMFSYSVAR", "TEXMFMAIN", "TEXMFLOCAL", "TEXMF", };
    for (int idx = 0; idx != sizeof(lpszVars) / sizeof(lpszVars[0]); ++idx)
    {
      std::string val;
      if (VarValue(lpszVars[idx], val))
      {
        stream.WriteFormattedLine("%s=%s", lpszVars[idx], val.c_str());
      }
    }
    PathName texmfDefaults(session->GetSpecialPath(SpecialPath::DistRoot));
    texmfDefaults += MIKTEX_PATH_MIKTEX_CONFIG_DIR;
    texmfDefaults += "texmf-defaults.ini";
    unique_ptr<Cfg> pCfg = Cfg::Create();
    pCfg->Read(texmfDefaults);
    char szValueName[BufferSizes::MaxCfgName];
    for (char* lpszValueName = pCfg->FirstValue(0, szValueName, BufferSizes::MaxCfgName);
    lpszValueName != 0;
      lpszValueName = pCfg->NextValue(szValueName, BufferSizes::MaxCfgName))
    {
      stream.WriteFormattedLine("%s=%s", lpszValueName, pCfg->GetValue(0, lpszValueName).c_str());
    }
    stream.WriteFormattedLine("TEXFONTMAPS=%s", ".;$TEXMF/fonts/map/{$progname,pdftex,dvips,}//");
    stream.Close();
    if (!Fndb::FileExists(texmfcnf))
    {
      Fndb::Add(texmfcnf);
    }
  }
  PathName texmfcnflua = texmfcnfdir;
  texmfcnflua += "texmfcnf.lua";
  if (!File::Exists(texmfcnflua) || lastMaintenance > File::GetLastWriteTime(texmfcnflua))
  {
    Directory::Create(texmfcnfdir);
    Core::StreamWriter stream(texmfcnflua);
    stream.WriteLine("return {");
    std::string val;
    if (VarValue("TEXMFVAR", val))
    {
      stream.WriteFormattedLine("  TEXMFCACHE=\"%s\"", val.c_str());
    }
    stream.WriteLine("}");
    stream.Close();
    if (!Fndb::FileExists(texmfcnf))
    {
      Fndb::Add(texmfcnf);
    }
  }
  return xstrdup(texmfcnfdir.ToUnix().GetData());
}
#endif
