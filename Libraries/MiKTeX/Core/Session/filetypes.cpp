/* filetypes.cpp: MiKTeX file type registry

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/CSVList.h"
#include "miktex/Core/Environment.h"
#include "miktex/Core/Paths.h"

#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

#if defined(MIKTEX_WINDOWS)
#  define DELIM ";"
#else
#  define DELIM ":"
#endif

#define R_(x) S_(x) "//"
#define r_(x) s_(x) "//"
#define S_(x) "%R" x
#define s_(x) "%r" x

#define P2_(a, b) a DELIM b
#define P3_(a, b, c) a DELIM b DELIM c
#define P4_(a, b, c, d) a DELIM b DELIM c DELIM d
#define P5_(a, b, c, d, e) a DELIM b DELIM c DELIM d DELIM e
#define P6_(a, b, c, d, e, f) a DELIM b DELIM c DELIM d DELIM e DELIM f
#define P7_(a, b, c, d, e, f, g) a DELIM b DELIM c DELIM d DELIM e DELIM f DELIM g
#define P8_(a, b, c, d, e, f, g, h) a DELIM b DELIM c DELIM d DELIM e DELIM f DELIM g DELIM h
#define P9_(a, b, c, d, e, f, g, h, i) a DELIM b DELIM c DELIM d DELIM e DELIM f DELIM g DELIM h DELIM i
#define P10_(a, b, c, d, e, f, g, h, i, j) a DELIM b DELIM c DELIM d DELIM e DELIM f DELIM g DELIM h DELIM i DELIM j

#define P_ ListBuilder

class ListBuilder :
  protected CharBuffer<char, 512>
{
public:
  ListBuilder()
  {
  }

public:
  ListBuilder(const char * lpszElement1, const char * lpszElement2 = nullptr, const char * lpszElement3 = nullptr, const char * lpszElement4 = nullptr, const char * lpszElement5 = nullptr, const char * lpszElement6 = nullptr, const char * lpszElement7 = nullptr, const char * lpszElement8 = nullptr)
  {
    Reserve(0
      + strlen(lpszElement1)
      + (lpszElement2 == nullptr ? 0 : strlen(lpszElement2) + 1)
      + (lpszElement3 == nullptr ? 0 : strlen(lpszElement3) + 1)
      + (lpszElement4 == nullptr ? 0 : strlen(lpszElement4) + 1)
      + (lpszElement5 == nullptr ? 0 : strlen(lpszElement5) + 1)
      + (lpszElement6 == nullptr ? 0 : strlen(lpszElement6) + 1)
      + (lpszElement7 == nullptr ? 0 : strlen(lpszElement7) + 1)
      + (lpszElement8 == nullptr ? 0 : strlen(lpszElement8) + 1)
      + 1
      );
    Set(lpszElement1);
    if (lpszElement2 != nullptr && *lpszElement2 != 0)
    {
      Append(PATH_DELIMITER);
      Append(lpszElement2);
    }
    if (lpszElement3 != nullptr && *lpszElement3 != 0)
    {
      Append(PATH_DELIMITER);
      Append(lpszElement3);
    }
    if (lpszElement4 != nullptr && *lpszElement4 != 0)
    {
      Append(PATH_DELIMITER);
      Append(lpszElement4);
    }
    if (lpszElement5 != nullptr && *lpszElement5 != 0)
    {
      Append(PATH_DELIMITER);
      Append(lpszElement5);
    }
    if (lpszElement6 != nullptr && *lpszElement6 != 0)
    {
      Append(PATH_DELIMITER);
      Append(lpszElement6);
    }
    if (lpszElement7 != nullptr && *lpszElement7 != 0)
    {
      Append(PATH_DELIMITER);
      Append(lpszElement7);
    }
    if (lpszElement8 != nullptr && *lpszElement8 != 0)
    {
      Append(PATH_DELIMITER);
      Append(lpszElement8);
    }
  }

public:
  operator const char * () const
  {
    return GetData();
  }
};

void SessionImpl::RegisterFileType(FileType fileType, const char * lpszFileType, const char * lpszApplication, const char * lpszFileNameExtensions, const char * lpszAlternateExtensions, const char * lpszDefaultSearchPath, const char * lpszEnvVarNames)
{
  MIKTEX_ASSERT_STRING(lpszFileType);
  MIKTEX_ASSERT_STRING_OR_NIL(lpszApplication);
  InternalFileTypeInfo fti;
  fti.fileType = fileType;
  fti.fileTypeString = lpszFileType;
  if (lpszApplication != nullptr)
  {
    fti.applicationName = lpszApplication;
  }
  string section = "ft.";
  section += lpszFileType;
  fti.fileNameExtensions = GetConfigValue(section.c_str(), "extensions", lpszFileNameExtensions);
  fti.alternateExtensions = GetConfigValue(section.c_str(), "alternate_extensions", lpszAlternateExtensions == nullptr ? "" : lpszAlternateExtensions);
  fti.searchPath = GetConfigValue(section.c_str(), "path", lpszDefaultSearchPath);
  fti.envVarNames = GetConfigValue(section.c_str(), "env", lpszEnvVarNames);
  fileTypes.resize((size_t)FileType::E_N_D);
  fileTypes[(size_t)fileType] = fti;
}

void SessionImpl::RegisterFileType(FileType fileType)
{
  if ((size_t)fileType >= fileTypes.size())
  {
    fileTypes.resize((size_t)FileType::E_N_D);
  }

  if (fileTypes[(size_t)fileType].fileType == fileType)
  {
    // already registered
    return;
  }

  switch (fileType)
  {

  case FileType::AFM:
    RegisterFileType(FileType::AFM, "afm", nullptr, ".afm", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_AFM_DIR)), P2_("AFMFONTS", "TEXFONTS"));
    break;

  case FileType::BASE:
    RegisterFileType(FileType::BASE, "base", "METAFONT", ".base", P2_(CURRENT_DIRECTORY, s_(MIKTEX_PATH_BASE_DIR)), "");
    break;

  case FileType::BIB:
    RegisterFileType(FileType::BIB, "bib", "BibTeX", ".bib", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_BIBTEX_DIR "/bib")), P2_("BIBINPUTS", "TEXBIB"));
    break;

  case FileType::BST:
    RegisterFileType(FileType::BST, "bst", "BibTeX", ".bst", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_BIBTEX_DIR "/{bst,csf}")), "BSTINPUTS");
    break;

  case FileType::CID:
    RegisterFileType(FileType::CID, "cid maps", nullptr, P2_(".cid", ".cidmap"), P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_CIDMAP_DIR)), "FONTCIDMAPS");
    break;

  case FileType::CLUA:
    RegisterFileType(FileType::CLUA, "clua", nullptr, P2_(".dll", ".so"), P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_SCRIPT_DIR "/{$progname,$engine,}/lua")), "CLUAINPUTS");
    break;

  case FileType::CNF:
    RegisterFileType(FileType::CNF, "cnf", nullptr, ".cnf", S_(MIKTEX_PATH_WEB2C_DIR), "TEXMFCNF");
    break;

  case FileType::CMAP:
    RegisterFileType(FileType::CMAP, "cmap files", nullptr, "", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_CMAP_DIR)), P2_("CMAPFONTS", "TEXFONTS"));
    break;

  case FileType::CWEB:
    RegisterFileType(FileType::CWEB, "cweb", "CWeb", ".w", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_CWEB_DIR)), "CWEBINPUTS");
    break;

  case FileType::DB:
    RegisterFileType(FileType::DB, "ls-R", nullptr, "", "", "TEXMFDBS");
    break;

  case FileType::DVI:
    RegisterFileType(FileType::DVI, "dvi", nullptr, ".dvi", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_DOC_DIR)), "");
    break;

  case FileType::DVIPSCONFIG:
    RegisterFileType(FileType::DVIPSCONFIG, "dvips config", "Dvips", "", R_(MIKTEX_PATH_DVIPS_DIR), "TEXCONFIG");
    break;

  case FileType::ENC:
    RegisterFileType(FileType::ENC, "enc", nullptr, ".enc", P6_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_ENC_DIR), R_(MIKTEX_PATH_MIKTEX_CONFIG_DIR), R_(MIKTEX_PATH_DVIPS_DIR), R_(MIKTEX_PATH_PDFTEX_DIR), R_(MIKTEX_PATH_DVIPDFM_DIR)), P2_("ENCFONTS", "TEXFONTS"));
    break;

  case FileType::EXE:
#if defined(MIKTEX_WINDOWS)
  case FileType::WindowsCommandScriptFile:
#endif
  {
    string extensions;
#if defined(MIKTEX_WINDOWS)
    if (!Utils::GetEnvironmentString("PATHEXT", extensions) || extensions.empty())
    {
      extensions = P3_(".com", ".exe", ".bat");
    }
#elif defined(MIKTEX_EXE_FILE_SUFFIX)
    extensions = MIKTEX_EXE_FILE_SUFFIX;
#endif
    string exePath;
    PathName userBinDir = GetSpecialPath(SpecialPath::UserInstallRoot);
    userBinDir /= MIKTEX_PATH_BIN_DIR;
    userBinDir.Canonicalize();
    if (!IsAdminMode() && !StringUtil::Contains(exePath.c_str(), userBinDir.Get(), PATH_DELIMITER_STRING))
    {
      if (!exePath.empty())
      {
        exePath += PATH_DELIMITER;
      }
      exePath += userBinDir.Get();
    }
    PathName commonBinDir = GetSpecialPath(SpecialPath::CommonInstallRoot);
    commonBinDir /= MIKTEX_PATH_BIN_DIR;
    commonBinDir.Canonicalize();
    if (!StringUtil::Contains(exePath.c_str(), commonBinDir.Get(), PATH_DELIMITER_STRING))
    {
      if (!exePath.empty())
      {
        exePath += PATH_DELIMITER;
      }
      exePath += commonBinDir.Get();
    }
    string str;
    if (Utils::GetEnvironmentString(MIKTEX_ENV_BIN_DIR, str))
    {
      PathName binDir = str;
      binDir.Canonicalize();
      if (!StringUtil::Contains(exePath.c_str(), binDir.Get(), PATH_DELIMITER_STRING))
      {
        if (!exePath.empty())
        {
          exePath += PATH_DELIMITER;
        }
        exePath += binDir.Get();
      }
    }
    PathName myLocation = GetMyLocation(true);
    if (!StringUtil::Contains(exePath.c_str(), myLocation.Get(), PATH_DELIMITER_STRING))
    {
      if (!exePath.empty())
      {
        exePath += PATH_DELIMITER;
      }
      exePath += myLocation.Get();
    }
    if (fileType == FileType::EXE)
    {
      RegisterFileType(FileType::EXE, "exe", nullptr, extensions.c_str(), exePath.c_str(), "");
    }
#if defined(MIKTEX_WINDOWS)
    else if (fileType == FileType::WindowsCommandScriptFile)
    {
      RegisterFileType(FileType::WindowsCommandScriptFile, "Windows command script file", nullptr, P2_(".bat", ".cmd"), P_(R_(MIKTEX_PATH_SCRIPT_DIR), exePath.c_str()), "");
    }
#endif
    break;
  }

  case FileType::FEA:
    RegisterFileType(FileType::FEA, "font feature files", nullptr, ".fea", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_FONT_FEATURE_DIR)), "FONTFEATURES");
    break;

  case FileType::FMT:
    RegisterFileType(FileType::FMT, "fmt", "TeX", ".fmt", P2_(CURRENT_DIRECTORY, s_(MIKTEX_PATH_FMT_DIR "/{$engine,}")), "");
    break;

  case FileType::GF:
    RegisterFileType(
      FileType::GF, "gf", nullptr, ".gf", // TODO
      P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_FONT_DIR)), // TODO
      P3_("GFFONTS", "GLYPHFONTS", "TEXFONTS"));
    break;

  case FileType::GLYPHFONT:
    RegisterFileType(FileType::GLYPHFONT, "bitmap font", nullptr, "", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_FONT_DIR)), P2_("GLYPHFONTS", "TEXFONTS"));
    break;

  case FileType::GRAPHICS:
    RegisterFileType(FileType::GRAPHICS, "graphic/figure", nullptr, P3_(".eps", ".epsi", ".png"), P4_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_DVIPS_DIR), R_(MIKTEX_PATH_PDFTEX_DIR), R_(MIKTEX_PATH_TEX_DIR)), P2_("TEXPICTS", "TEXINPUTS"));
    break;

  case FileType::HBF:
    RegisterFileType(FileType::HBF, "hbf", nullptr, ".hbf", P3_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_HBF_DIR), R_(MIKTEX_PATH_TYPE1_DIR)), "");
    break;

  case FileType::IST:
    RegisterFileType(FileType::IST, "ist", "MakeIndex", ".ist", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_MAKEINDEX_DIR)), P2_("TEXINDEXSTYLE", "INDEXSTYLE"));
    break;

  case FileType::LIG:
    RegisterFileType(FileType::LIG, "lig files", nullptr, ".lig", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_LIG_DIR)), "TEXFONTS");
    break;

  case FileType::LUA:
    RegisterFileType(FileType::LUA, "lua", nullptr, P7_(".lua", ".luatex", ".luc", ".luctex", ".texlua", ".texluc", ".tlu"), P_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_SCRIPT_DIR "/{$progname,$engine,}/{lua,}"), GetFileTypeInfo(FileType::TEX).searchPath.c_str()), "LUAINPUTS");
    break;

  case FileType::MAP:
    RegisterFileType(FileType::MAP, "map", nullptr, ".map", P6_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_MAP_DIR "/{$progname,pdftex,dvips,}"), R_(MIKTEX_PATH_MIKTEX_CONFIG_DIR), R_(MIKTEX_PATH_DVIPS_DIR), R_(MIKTEX_PATH_PDFTEX_DIR), R_(MIKTEX_PATH_DVIPDFM_DIR)), P2_("TEXFONTMAPS", "TEXFONTS"));
    break;

  case FileType::MEM:
    RegisterFileType(FileType::MEM, "mem", "MetaPost", ".mem", CURRENT_DIRECTORY, "");
    break;

  case FileType::MF:
    RegisterFileType(FileType::MF, "mf", "METAFONT", ".mf", P3_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_METAFONT_DIR), R_(MIKTEX_PATH_FONT_SOURCE_DIR)), "MFINPUTS");
    break;

  case FileType::MFPOOL:
    RegisterFileType(FileType::MFPOOL, "mfpool", nullptr, ".pool", CURRENT_DIRECTORY, P2_("MFPOOL", "TEXMFINI"));
    break;

  case FileType::MFT:
    RegisterFileType(FileType::MFT, "mft", nullptr, ".mft", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_MFT_DIR)), "MFTINPUTS");
    break;

  case FileType::MISCFONT:
    RegisterFileType(FileType::MISCFONT, "misc fonts", nullptr, "", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_MISCFONTS_DIR)), P2_("MISCFONTS", "TEXFONTS"));
    break;

  case FileType::MLBIB:
    RegisterFileType(FileType::MLBIB, "mlbib", nullptr, P2_(".mlbib", ".bib"), P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_BIBTEX_DIR "/bib/{mlbib,}")), P3_("MLBIBINPUTS", "BIBINPUTS", "TEXBIB"));
    break;

  case FileType::MLBST:
    RegisterFileType(FileType::MLBST, "mlbst", nullptr, ".bst", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_BIBTEX_DIR "/{mlbst,bst}")), P2_("MLBSTINPUTS", "BSTINPUTS"));
    break;

  case FileType::MP:
    RegisterFileType(FileType::MP, "mp", "MetaPost", ".mp", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_METAPOST_DIR)), "MPINPUTS");
    break;

  case FileType::MPPOOL:
    RegisterFileType(FileType::MPPOOL, "mppool", nullptr, ".pool", CURRENT_DIRECTORY, P2_("MPPOOL", "TEXMFINI"));
    break;

  case FileType::MPSUPPORT:
    RegisterFileType(FileType::MPSUPPORT, "MetaPost support", nullptr, "", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_MPSUPPORT_DIR)), "MPSUPPORT");
    break;

  case FileType::OCP:
    RegisterFileType(FileType::OCP, "ocp", "Omega", ".ocp", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_OCP_DIR)), "OCPINPUTS");
    break;

  case FileType::OFM:
    RegisterFileType(FileType::OFM, "ofm", "Omega", P2_(".ofm", ".tfm"), P3_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_OFM_DIR), R_(MIKTEX_PATH_TFM_DIR)), P2_("OFMFONTS", "TEXFONTS"));
    break;

  case FileType::OPL:
    RegisterFileType(FileType::OPL, "opl", nullptr, ".opl", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_OPL_DIR)), P2_("OPLFONTS", "TEXFONTS"));
    break;

  case FileType::OTP:
    RegisterFileType(FileType::OTP, "otp", "otp2ocp", ".otp", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_OTP_DIR)), "OTPINPUTS");
    break;

  case FileType::OTF:
    RegisterFileType(FileType::OTF, "opentype fonts", nullptr, ".otf", P_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_OPENTYPE_DIR), GetLocalFontDirectories().c_str()), P2_("OPENTYPEFONTS", "TEXFONTS"));
    break;

  case FileType::OVF:
    RegisterFileType(FileType::OVF, "ovf", nullptr, ".ovf", P3_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_OVF_DIR), R_(MIKTEX_PATH_VF_DIR)), P2_("OVFFONTS", "TEXFONTS"));
    break;

  case FileType::OVP:
    RegisterFileType(FileType::OVP, "ovp", nullptr, ".ovp", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_OVP_DIR)), P2_("OVPFONTS", "TEXFONTS"));
    break;

  case FileType::PDFTEXCONFIG:
    RegisterFileType(FileType::PDFTEXCONFIG, "pdftex config", nullptr, "", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_PDFTEX_DIR "/{$progname,}")), "PDFTEXCONFIG");
    break;

  case FileType::PERLSCRIPT:
    RegisterFileType(FileType::PERLSCRIPT, "perlscript", nullptr, ".pl", P8_(R_(MIKTEX_PATH_SCRIPT_DIR), R_(MIKTEX_PATH_CONTEXT_DIR), R_(MIKTEX_PATH_MIKTEX_DIR), R_(MIKTEX_PATH_NTS_DIR), R_(MIKTEX_PATH_PDFTEX_DIR), R_(MIKTEX_PATH_PSUTILS_DIR), R_(MIKTEX_PATH_SOURCE_DIR), R_(MIKTEX_PATH_TEX_DIR)), "");
    break;

  case FileType::PK:
    RegisterFileType(
      FileType::PK, "pk", nullptr, ".pk", // TODO
      P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_FONT_DIR)), // TODO
      "");
    break;

  case FileType::PROGRAMBINFILE:
    RegisterFileType(FileType::PROGRAMBINFILE, "other binary files", nullptr, "", P2_(CURRENT_DIRECTORY, R_("/$progname")), "");
    break;

  case FileType::PROGRAMTEXTFILE:
    RegisterFileType(FileType::PROGRAMTEXTFILE, "other text files", nullptr, "", P2_(CURRENT_DIRECTORY, R_("/$progname")), "");
    break;

  case FileType::PSHEADER:
    RegisterFileType(FileType::PSHEADER, "PostScript header", nullptr, P2_(".pro", ".enc"), P10_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_MIKTEX_CONFIG_DIR), R_(MIKTEX_PATH_DVIPS_DIR), R_(MIKTEX_PATH_PDFTEX_DIR), R_(MIKTEX_PATH_DVIPDFM_DIR), R_(MIKTEX_PATH_ENC_DIR), R_(MIKTEX_PATH_TYPE1_DIR), R_(MIKTEX_PATH_TYPE42_DIR), R_(MIKTEX_PATH_TYPE3_DIR), "$psfontdirs"), P2_("TEXPSHEADERS", "PSHEADERS"));
    break;

  case FileType::SCRIPT:
    RegisterFileType(FileType::SCRIPT, "texmfscripts", nullptr, "", R_(MIKTEX_PATH_SCRIPT_DIR "/{$progname,$engine,}"), "TEXMFSCRIPTS");
    break;

  case FileType::SFD:
    RegisterFileType(FileType::SFD, "subfont definition files", nullptr, ".sfd", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_SFD_DIR)), P2_("SFDFONTS", "TEXFONTS"));
    break;

  case FileType::TCX:
    RegisterFileType(FileType::TCX, "tcx", nullptr, ".tcx", P3_(CURRENT_DIRECTORY, S_(MIKTEX_PATH_MIKTEX_CONFIG_DIR), S_(MIKTEX_PATH_WEB2C_DIR)), "");
    break;

  case FileType::TEX:
    RegisterFileType(FileType::TEX, "tex", "TeX", ".tex", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_TEX_DIR "/{$progname,generic,}")), "TEXINPUTS");
    break;

  case FileType::TEXPOOL:
    RegisterFileType(FileType::TEXPOOL, "texpool", nullptr, ".pool", CURRENT_DIRECTORY, P2_("TEXPOOL", "TEXMFINI"));
    break;

  case FileType::TEXSOURCE:
    RegisterFileType(FileType::TEXSOURCE, "TeX system sources", nullptr, "", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_SOURCE_DIR)), "TEXSOURCES");
    break;

  case FileType::TEXSYSDOC:
    RegisterFileType(FileType::TEXSYSDOC, "TeX system documentation", nullptr,
#if defined(MIKTEX_WINDOWS)
      P6_(".chm", ".dvi", ".html", ".txt", ".pdf", ".ps"),
#else
      P5_(".dvi", ".html", ".txt", ".pdf", ".ps"),
#endif
      P2_(R_(MIKTEX_PATH_MIKTEX_DOC_DIR), R_(MIKTEX_PATH_DOC_DIR)), "TEXDOCS");
    break;

  case FileType::TFM:
    RegisterFileType(FileType::TFM, "tfm", nullptr, ".tfm", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_TFM_DIR)), P2_("TFMFONTS", "TEXFONTS"));
    break;

  case FileType::TROFF:
    RegisterFileType(FileType::TROFF, "troff fonts", nullptr, "", "", "TRFONTS");
    break;

  case FileType::TTF:
    RegisterFileType(FileType::TTF, "truetype fonts", nullptr, P2_(".ttf", ".ttc"), P_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_TRUETYPE_DIR), GetLocalFontDirectories().c_str()), P2_("TTFONTS", "TEXFONTS"));
    break;

  case FileType::TYPE1:
    RegisterFileType(FileType::TYPE1, "type1 fonts", nullptr, P2_(".pfb", ".pfa"), P_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_TYPE1_DIR), GetLocalFontDirectories().c_str()), P5_("T1FONTS", "T1INPUTS", "TEXFONTS", "TEXPSHEADERS", "PSHEADERS"));
    break;

  case FileType::TYPE42:
    RegisterFileType(FileType::TYPE42, "type42 fonts", nullptr,
#if defined(MIKTEX_WINDOWS)
      ".t42",
#else
      P2_(".t42", ".T42"),
#endif
      P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_TYPE42_DIR)), P2_("T42FONTS", "TEXFONTS"));
    break;

  case FileType::VF:
    RegisterFileType(FileType::VF, "vf", nullptr, ".vf", P2_(CURRENT_DIRECTORY, R_(MIKTEX_PATH_VF_DIR)), P2_("VFFONTS", "TEXFONTS"));
    break;

  case FileType::WEB:
    RegisterFileType(FileType::WEB, "web", nullptr, ".web", R_(MIKTEX_PATH_WEB_DIR), "WEBINPUTS");
    break;

  case FileType::WEB2C:
    RegisterFileType(FileType::WEB2C, "web2c files", nullptr, "", R_(MIKTEX_PATH_WEB2C_DIR), "");
    break;

  default:
    MIKTEX_UNEXPECTED();
  }
}

void SessionImpl::RegisterFileTypes()
{
  for (int ft = (int)FileType::None + 1; ft < (int)FileType::E_N_D; ++ft)
  {
    RegisterFileType((FileType)ft);
  }
}

InternalFileTypeInfo * SessionImpl::GetInternalFileTypeInfo(FileType fileType)
{
  RegisterFileType(fileType);
  return &fileTypes[(size_t)fileType];
}

FileTypeInfo SessionImpl::GetFileTypeInfo(FileType fileType)
{
  return *GetInternalFileTypeInfo(fileType);
}

FileType SessionImpl::DeriveFileType(const char * lpszPath)
{
  MIKTEX_ASSERT_STRING(lpszPath);

  RegisterFileTypes();

  const char * lpszExt = GetFileNameExtension(lpszPath);

  for (int idx = 1; idx < fileTypes.size(); ++idx)
  {
    if (lpszExt == nullptr)
    {
      if (Utils::Equals(fileTypes[idx].fileTypeString.c_str(), lpszPath))
      {
        return fileTypes[idx].fileType;
      }
    }
    else
    {
      for (CSVList ext(fileTypes[idx].fileNameExtensions, PATH_DELIMITER); ext.GetCurrent() != nullptr; ++ext)
      {
        if (PathName::Compare(ext.GetCurrent(), lpszExt) == 0)
        {
          return fileTypes[idx].fileType;
        }
      }
    }
  }

  return FileType::None;
}

vector<FileTypeInfo> SessionImpl::GetFileTypes()
{
  vector<FileTypeInfo> result;
  for (int ft = (int)FileType::None + 1; ft < (int)FileType::E_N_D; ++ft)
  {
    result.push_back(GetFileTypeInfo((FileType)ft));
  }
  return result;
}

void SessionImpl::ClearSearchVectors()
{
  for (InternalFileTypeInfo & info : fileTypes)
  {
    info.searchVec.clear();
  }
}
