/* papersize.cpp: paper size info

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

#include "miktex/Core/Directory.h"
#include "miktex/Core/Paths.h"
#include "miktex/Core/StreamReader.h"

#include "Session/SessionImpl.h"
#include "Utils/inliners.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

constexpr int inch2bp(double inch)
{
  return (inch * 72.0) + 0.5;
}

constexpr int mm2bp(int mm)
{
  return inch2bp(mm * (1.0 / 25.4));
}

MIKTEXSTATICFUNC(bool) IsKnownPaperSize(int width, int height, string & name)
{
  static const struct
  {
    string name;
    int width;
    int height;
  } knownPaperSizes[] =
  {
    "A3", mm2bp(297), mm2bp(420),
    "A4", mm2bp(210), mm2bp(297),
    "A5", mm2bp(148), mm2bp(210),
    "A6", mm2bp(257), mm2bp(364),
    "Ledger", inch2bp(17), inch2bp(11),
    "Legal", inch2bp(8.5), inch2bp(14),
    "Letter", inch2bp(8.5), inch2bp(11),
    "Tabloid", inch2bp(11), inch2bp(17),
  };
  for (const auto & paperSize : knownPaperSizes)
  {
    if (paperSize.width == width && paperSize.height == height)
    {
      name = paperSize.name;
      return true;
    }
  }
  return false;
}

MIKTEXSTATICFUNC(int) CalculatePostScriptPoints(double value, const string & unit)
{
  static const struct
  {
    string unit;
    double factor;
  } unittable[] =
  {
    { "pt", 72.27 / 72.0 },
    { "pc", (72.27 / 12) / 72.0 },
    { "in", 1.0 / 72.0 },
    { "bp", 1.0 },
    { "cm", 2.54 / 72.0 },
    { "mm", 25.4 / 72.0 },
    { "dd", (72.27 / (1238.0 / 1157)) / 72.0 },
    { "cc", (72.27 / 12 / (1238.0 / 1157)) / 72.0 },
    { "sp", (72.27 * 65536) / 72.0 },
  };
  for (const auto & u : unittable)
  {
    if (u.unit == unit)
    {
      return (value / u.factor) + 0.5;
    }
  }
  MIKTEX_UNEXPECTED();
}

MIKTEXSTATICFUNC(bool) ChopFloat(char * & lpsz, double & ret)
{
  SkipSpace(lpsz);
  bool gotDot = false;
  char * lpszStart = lpsz;
  string str;
  while (*lpsz != 0 && (IsDigit(*lpsz) || (*lpsz == '.' && !gotDot)))
  {
    str += *lpsz;
    if (*lpsz == '.')
    {
      gotDot = true;
    }
    *lpsz = 0;
    ++lpsz;
  }
  if (lpsz == lpszStart)
  {
    return false;
  }
  ret = atof(str.c_str());
  return true;
}

MIKTEXSTATICFUNC(bool) ChopToken(char * & lpsz, string & ret)
{
  SkipSpace(lpsz);
  char * lpszStart = lpsz;
  ret = "";
  while (*lpsz != 0 && !IsSpace(*lpsz) && *lpsz != ',')
  {
    ret += *lpsz;
    *lpsz = 0;
    ++lpsz;
  }
  return lpsz != lpszStart;
}

void SessionImpl::AddDvipsPaperSize(const DvipsPaperSizeInfo & dvipsPaperSizeInfo)
{
  for (DvipsPaperSizeInfo & siz : dvipsPaperSizes)
  {
    if (Utils::EqualsIgnoreCase(siz.dvipsName.c_str(), dvipsPaperSizeInfo.dvipsName.c_str()))
    {
      siz = dvipsPaperSizeInfo;
      return;
    }
  }
  dvipsPaperSizes.push_back(dvipsPaperSizeInfo);
}

void SessionImpl::ReadDvipsPaperSizes()
{
  for (unsigned r = GetNumberOfTEXMFRoots(); r > 0; --r)
  {
    if (!IsManagedRoot(r - 1))
    {
      continue;
    }
    PathName configPs(GetRootDirectory(r - 1), MIKTEX_PATH_CONFIG_PS);
    if (File::Exists(configPs))
    {
      StreamReader reader(configPs);
      string line;
      bool inDefinition = false;
      DvipsPaperSizeInfo current;
      while (reader.ReadLine(line))
      {
        if (line.empty() || line[0] != '@')
        {
          if (inDefinition)
          {
            AddDvipsPaperSize(current);
            current.definition.clear();
            inDefinition = false;
          }
          continue;
        }
        CharBuffer<char> buf(line.c_str() + 1);
        char * lpsz = buf.GetData();
        SkipSpace(lpsz);
        if (*lpsz == 0)
        {
          dvipsPaperSizes.clear();
          inDefinition = false;
          continue;
        }
        if (*lpsz == '+')
        {
          if (!inDefinition)
          {
            MIKTEX_UNEXPECTED();
          }
        }
        else if (!inDefinition)
        {
          inDefinition = true;
          if (!ChopToken(lpsz, current.dvipsName))
          {
            MIKTEX_UNEXPECTED();
          }
          double texWidth;
          if (!ChopFloat(lpsz, texWidth))
          {
            MIKTEX_UNEXPECTED();
          }
          string unit1;
          if (!ChopToken(lpsz, unit1))
          {
            MIKTEX_UNEXPECTED();
          }
          double texHeight;
          if (!ChopFloat(lpsz, texHeight))
          {
            MIKTEX_UNEXPECTED();
          }
          string unit2;
          if (!ChopToken(lpsz, unit2))
          {
            MIKTEX_UNEXPECTED();
          }
          current.width = CalculatePostScriptPoints(texWidth, unit1);
          current.height = CalculatePostScriptPoints(texHeight, unit2);
          if (!IsKnownPaperSize(current.width, current.height, current.name))
          {
            current.name = current.dvipsName;
          }
        }
        current.definition.push_back(line);
      }
      if (inDefinition)
      {
        AddDvipsPaperSize(current);
      }
    }
  }
  if (dvipsPaperSizes.empty())
  {
    MIKTEX_UNEXPECTED();
  }
}

bool SessionImpl::GetPaperSizeInfo(int idx, PaperSizeInfo & paperSizeInfo)
{
  if (dvipsPaperSizes.empty())
  {
    ReadDvipsPaperSizes();
  }

  MIKTEX_ASSERT(!dvipsPaperSizes.empty());

  if (idx < 0)
  {
    // get default paper size
    idx = 0;
  }
  else if (idx == dvipsPaperSizes.size())
  {
    return false;
  }
  else if (idx > dvipsPaperSizes.size())
  {
    INVALID_ARGUMENT("index", std::to_string(idx));
  }

  paperSizeInfo = dvipsPaperSizes[idx];

  return true;
}

PaperSizeInfo SessionImpl::GetPaperSizeInfo(const char * lpszDvipsName)
{
  PaperSizeInfo paperSizeInfo;
  for (int idx = 0; GetPaperSizeInfo(idx, paperSizeInfo); ++idx)
  {
    if (Utils::EqualsIgnoreCase(paperSizeInfo.dvipsName.c_str(), lpszDvipsName))
    {
      return paperSizeInfo;
    }
  }
  MIKTEX_FATAL_ERROR_2(T_("Unknown paper size."), "name", lpszDvipsName);
}

PaperSizeInfo PaperSizeInfo::Parse(const char * lpszSpec)
{
  double texWidth;

  CharBuffer<char> buf(lpszSpec);

  char * lpsz = buf.GetData();

  if (!ChopFloat(lpsz, texWidth))
  {
    MIKTEX_UNEXPECTED();
  }

  string unit1;

  if (!ChopToken(lpsz, unit1))
  {
    MIKTEX_UNEXPECTED();
  }

  if (*lpsz != ',')
  {
    MIKTEX_UNEXPECTED();
  }

  ++lpsz;

  double texHeight;

  if (!ChopFloat(lpsz, texHeight))
  {
    MIKTEX_UNEXPECTED();
  }

  string unit2;

  if (!ChopToken(lpsz, unit2))
  {
    MIKTEX_UNEXPECTED();
  }

  int width = CalculatePostScriptPoints(texWidth, unit1);
  int height = CalculatePostScriptPoints(texHeight, unit2);

  PaperSizeInfo paperSizeInfo;

  for (int idx = 0; SessionImpl::GetSession()->GetPaperSizeInfo(idx, paperSizeInfo); ++idx)
  {
    if (paperSizeInfo.width == width && paperSizeInfo.height == height)
    {
      return paperSizeInfo;
    }
  }

  paperSizeInfo.dvipsName = "custom";
  paperSizeInfo.name = "custom";
  paperSizeInfo.width = width;
  paperSizeInfo.height = height;

  return paperSizeInfo;
}

void SessionImpl::SetDefaultPaperSize(const char * lpszDvipsName)
{
  UNUSED_ALWAYS(lpszDvipsName);

  if (dvipsPaperSizes.empty())
  {
    ReadDvipsPaperSizes();
  }

  MIKTEX_ASSERT(!dvipsPaperSizes.empty());

  vector<DvipsPaperSizeInfo>::iterator it;

  for (it = dvipsPaperSizes.begin(); it != dvipsPaperSizes.end(); ++it)
  {
    if (Utils::EqualsIgnoreCase(it->dvipsName, lpszDvipsName))
    {
      break;
    }
  }

  if (it == dvipsPaperSizes.end())
  {
    MIKTEX_FATAL_ERROR_2(T_("Unknown paper size."), "name", lpszDvipsName);
  }

  if (it == dvipsPaperSizes.begin())
  {
    return;
  }

  swap(*dvipsPaperSizes.begin(), *it);

  WriteDvipsPaperSizes();
  WriteDvipdfmPaperSize();
  WriteDvipdfmxPaperSize();
  WritePdfTeXPaperSize();
}

class StreamEditor
{
public:
  StreamEditor(const PathName & path) :
    path(path)
  {
    bak = path;
    bak.Append(".bak", false);
    File::Move(path, bak);
    reader.Attach(File::Open(bak, FileMode::Open, FileAccess::Read));
    writer.Attach(File::Open(path, FileMode::Create, FileAccess::Write));
  }

public:
  virtual ~StreamEditor()
  {
    try
    {
      reader.Close();
      writer.Close();
      File::Delete(bak);
      if (!Fndb::FileExists(path))
      {
        Fndb::Add(path);
      }
    }
    catch (const exception &)
    {
    }
  }

public:
  bool ReadLine(string & line)
  {
    return reader.ReadLine(line);
  }

public:
  void WriteLine(const string & line)
  {
    writer.WriteLine(line);
  }

public:
  void WriteLine()
  {
    writer.WriteLine();
  }

public:
  void WriteFormattedLine(const char * lpszFormat, ...)
  {
    va_list marker;
    va_start(marker, lpszFormat);
    WriteLine(StringUtil::FormatString(lpszFormat, marker));
    va_end(marker);
  }

private:
  PathName path;

private:
  PathName bak;

private:
  StreamReader reader;

private:
  StreamWriter writer;
};

bool SessionImpl::TryCreateFromTemplate(const PathName & path)
{
  unsigned r = TryDeriveTEXMFRoot(path);
  if (r == INVALID_ROOT_INDEX)
  {
    return false;
  }
  const char * lpszRelPath = Utils::GetRelativizedPath(path.Get(), GetRootDirectory(r).Get());
  if (lpszRelPath == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  PathName configTemplate(GetSpecialPath(SpecialPath::InstallRoot));
  configTemplate /= lpszRelPath;
  configTemplate.Append(".template", false);
  if (File::Exists(configTemplate))
  {
    Directory::Create(PathName(path).RemoveFileSpec());
    File::Copy(configTemplate, path);
    FileAttributeSet attr = File::GetAttributes(path);
    attr -= FileAttribute::ReadOnly;
    File::SetAttributes(path, attr);
    if (!Fndb::FileExists(path))
    {
      Fndb::Add(path);
    }
    return true;
  }
  else
  {
    return false;
  }
}

void SessionImpl::WriteDvipsPaperSizes()
{
  PathName configFile(GetSpecialPath(SpecialPath::ConfigRoot), MIKTEX_PATH_CONFIG_PS);

  if (!File::Exists(configFile))
  {
    if (!TryCreateFromTemplate(configFile))
    {
      Directory::Create(PathName(configFile).RemoveFileSpec());
      StreamWriter writer(configFile);
      writer.Close();
    }
  }

  StreamEditor editor(configFile);

  string line;
  string lastLine;

  for (; editor.ReadLine(line); lastLine = line)
  {
    if (!line.empty() && line[0] == '@')
    {
      line = "";
    }
    if (!(line.empty() && lastLine.empty()))
    {
      editor.WriteLine(line);
    }
  }

  editor.WriteLine("@");
  editor.WriteLine();

  for (const DvipsPaperSizeInfo & dvipsPaper : dvipsPaperSizes)
  {
    for (const string & def : dvipsPaper.definition)
    {
      editor.WriteLine(def);
    }
    editor.WriteLine();
  }
}

void SessionImpl::WriteDvipdfmPaperSize()
{
  if (dvipsPaperSizes.empty())
  {
    MIKTEX_UNEXPECTED();
  }

  DvipsPaperSizeInfo paperSizeInfo = dvipsPaperSizes[0];

  PathName configFile(GetSpecialPath(SpecialPath::ConfigRoot), MIKTEX_PATH_DVIPDFM_CONFIG);

  if (!File::Exists(configFile))
  {
    if (!TryCreateFromTemplate(configFile))
    {
      Directory::Create(PathName(configFile).RemoveFileSpec());
      StreamWriter writer(configFile);
      writer.Close();
    }
  }

  StreamEditor editor(configFile);
  string line;
  while (editor.ReadLine(line))
  {
    if (!(line.compare(0, 2, "p ") == 0))
    {
      editor.WriteLine(line);
    }
  }

  editor.WriteFormattedLine("p %s", Utils::MakeLower(paperSizeInfo.name).c_str());
}

void SessionImpl::WriteDvipdfmxPaperSize()
{
  if (dvipsPaperSizes.empty())
  {
    MIKTEX_UNEXPECTED();
  }

  DvipsPaperSizeInfo paperSizeInfo = dvipsPaperSizes[0];

  PathName configFile(GetSpecialPath(SpecialPath::ConfigRoot), MIKTEX_PATH_DVIPDFMX_CONFIG);

  if (!File::Exists(configFile))
  {
    if (!TryCreateFromTemplate(configFile))
    {
      Directory::Create(PathName(configFile).RemoveFileSpec());
      StreamWriter writer(configFile);
      writer.Close();
    }
  }

  StreamEditor editor(configFile);
  string line;
  while (editor.ReadLine(line))
  {
    if (!(line.compare(0, 2, "p ") == 0))
    {
      editor.WriteLine(line);
    }
  }

  editor.WriteFormattedLine("p %dbp,%dbp", paperSizeInfo.width, paperSizeInfo.height);
}

void SessionImpl::WritePdfTeXPaperSize()
{
  if (dvipsPaperSizes.empty())
  {
    MIKTEX_UNEXPECTED();
  }

  DvipsPaperSizeInfo paperSizeInfo = dvipsPaperSizes[0];

  PathName configFile(GetSpecialPath(SpecialPath::ConfigRoot), MIKTEX_PATH_PDFTEX_CFG);

  if (!File::Exists(configFile))
  {
    if (!TryCreateFromTemplate(configFile))
    {
      Directory::Create(PathName(configFile).RemoveFileSpec());
      StreamWriter writer(configFile);
      writer.Close();
    }
  }

  StreamEditor editor(configFile);
  string line;
  while (editor.ReadLine(line))
  {
    if (!(line.compare(0, 11, "page_width ") == 0 || line.compare(0, 12, "page_height ") == 0))
    {
      editor.WriteLine(line);
    }
  }

  editor.WriteFormattedLine("page_width %d true bp", paperSizeInfo.width);

  editor.WriteFormattedLine("page_height %d true bp", paperSizeInfo.height);
}
