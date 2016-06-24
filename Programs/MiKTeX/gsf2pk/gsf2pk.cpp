/* gsf2pk.cpp: Gsf-to-Pk converter (based on gsftopk)

   Copyright (C) 2004-2016 Christian Schenk
   Copyright (C) 1993-2000 Paul Vojta

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL PAUL VOJTA BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
   CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <cstdio>

#if defined(_MSC_VER)
#  pragma warning (disable: 4702)
#endif

#include <cstdarg>

#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <miktex/App/Application>
#include <miktex/Core/AutoResource>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Exceptions>
#include <miktex/Core/FileType>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Util/StringUtil>
#include <miktex/Wrappers/PoptWrapper>

#include "gsf2pk-version.h"

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;
using namespace std;

#if defined(_MSC_VER)
#  if (_MSC_VER < 1400)
#    define SScanF sscanf
#  else
#    define SScanF sscanf_s
#  endif
#else
#  define SScanF sscanf
#endif

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).Get()

#define VA_START(arglist, lpszFormat   )        \
va_start(arglist, lpszFormat);                  \
try                                             \
{

#define VA_END(arglist)                         \
}                                               \
catch(...)                                      \
{                                               \
  va_end(arglist);                              \
  throw;                                        \
}                                               \
va_end(arglist);

class Converter
  : public Application
{
public:
  ~Converter();

public:
  void Main(int argc, const char * * argv);

private:
  MIKTEXNORETURN void Error(const char * lpszFormat, ...) const;

private:
  void Verbose(const char * lpszFormat, ...);

private:
  int GetByte(FILE * pfile) const;

private:
  int GetByte() const
  {
    return GetByte(pFileGsf.Get());
  }

private:
  int GetDword(FILE * pfile) const;

private:
  int GetDword() const
  {
    return GetDword(pFileGsf.Get());
  }

private:
  void Read(void * pv, size_t len);

private:
  void Expect(const char * lpszWaitingFor, string * pLine = 0);

private:
  int GetInt();

private:
  void WhiteSpace();

private:
  void PutByte(int by) const;

private:
  void PutDword(int dword) const;

private:
  void PutByteArray(const void * pv, size_t len) const;

private:
  int GetFirstByte(const PathName & file);

private:
  void ReadTFMFile(const char * lpszTeXFontName);

private:
  void tallyup(int n);

private:
  void pk_put_nyb(int n);

private:
  void pk_put_long(int n);

private:
  void pk_put_count(int n);

private:
  void trim_bitmap();

private:
  bool pk_rll_cvt();

private:
  void pk_bm_cvt();

private:
  void putshort(int w);

private:
  void putmed(long w);

private:
  void putlong(long w);

private:
  void putspecl(const char * str1, const char * str2 = 0);

private:
  void Write(const void * pv, size_t len);

private:
  void PutGlyph(int cc);

private:
  void WritePkFile(const char * lpszPkFile);

private:
  Process * StartGhostscript(const char * lpszFontFile, const char * lpszEncFile, const char * lpszFontName, const char * lpszSpecInfo, const char * lpszDPI, FILE ** ppGsOut, FILE ** ppGsErr);

private:
  void StderrReader();

private:
  void Convert(const char * lpszTeXFontName, const char * lpszFontName, const char * lpszSpecInfo, const char * lpszEncFile, const char * lpszFontFile, const char * lpszDPI, const char * lpszPkFile);

public:
  void ShowGhostscriptTranscript() const;

private:
  bool quiet = false;

private:
  bool verbose = false;

private:
  AutoFILE pFilePk;

private:
  AutoFILE pFileGsf;

private:
  AutoFILE pFileGsErr;

private:
  string gsStdErr;

private:
  vector<int> lengths;

private:
  vector<int> widths;

private:
  vector<int> widthIndex;

private:
  string chars;

private:
  int checkSum;

private:
  int designSize;

private:
  int dpi;

private:
  int lh() const { return lengths[1]; }
  int bc() const { return lengths[2]; }
  int ec() const { return lengths[3]; }
  int nw() const { return lengths[4]; }

private:
  bool haveFirstLine = false;

private:
  int llx, lly, urx, ury;

private:
  float charWidth;

private:
  typedef unsigned char Byte;

private:
  vector<Byte> area1;

private:
  Byte * bitmap;
  Byte * bitmap_end;

  int pk_len;

private:
  int idx;

private:
  int hoff, voff;

private:
  int width;

private:
  int height;

private:
  int bytesWide;

private:
  size_t bitmapSize;

private:
  string fontName;

  // area for saving bit counts
private:
  vector<int> counts;

private:
  int skip;

private:
  Byte part;

private:
  Byte flag;

private:
  int pk_dyn_f;

private:
  int pk_dyn_g;

  // cost of this character if pk_dyn_f = 0
private:
  int base;

  // cost of increasing pk_dyn_f from i to i+1
private:
  int deltas[13];

private:
  static bool odd;

private:
  static Byte masks[];

private:
  static const struct poptOption aoption[];
};

enum Option
{
  OPT_AAA = 1, OPT_QUIET, OPT_VERBOSE, OPT_VERSION,
};

const struct poptOption Converter::aoption[] = {

  {
    "quiet", 0, POPT_ARG_NONE, nullptr, OPT_QUIET, T_("Suppress all output (except errors)."), nullptr
  },

  {
    "verbose", 0, POPT_ARG_NONE, nullptr, OPT_VERBOSE, T_("Turn on verbose output mode."), nullptr
  },

  {
    "version", 0, POPT_ARG_NONE, nullptr, OPT_VERSION, T_("Show version information and exit."), nullptr
  },

  POPT_AUTOHELP
  POPT_TABLEEND
};

bool Converter::odd = false;

Converter::Byte Converter::masks[] = {
  0, 1, 3, 7, 017, 037, 077, 0177, 0377
};

Converter::~Converter()
{
#if 0
  try
  {
    pFileGsErr->Reset();
  }
  catch (const exception &)
  {
  }
#endif
}

MIKTEXNORETURN void Converter::Error(const char * lpszFormat, ...) const
{
  va_list arglist;
  VA_START(arglist, lpszFormat);
  cerr << "gsf2pk" << ": " << StringUtil::FormatString(lpszFormat, arglist) << endl;
  VA_END(arglist);
  ShowGhostscriptTranscript();
  throw 1;
}

void Converter::Verbose(const char * lpszFormat, ...)
{
  if (!verbose)
  {
    return;
  }
  va_list arglist;
  VA_START(arglist, lpszFormat);
  cout << StringUtil::FormatString(lpszFormat, arglist) << endl;
  VA_END(arglist);
}

void Converter::ShowGhostscriptTranscript() const
{
  if (gsStdErr.empty())
  {
    return;
  }
  cerr
    << T_("Ghostscript transcript follows:") << endl
    << gsStdErr << endl;
}

int Converter::GetByte(FILE * pfile) const
{
  int c = fgetc(pfile);
  if (c == EOF)
  {
    if (ferror(pfile))
    {
      MIKTEX_FATAL_CRT_ERROR("fgetc");
    }
    else
    {
      Error(T_("Unexecpected end of file."));
    }
  }
  return c & 0xff;
}

int Converter::GetDword(FILE * pfile) const
{
  int ret = GetByte(pfile) << 24;
  ret |= GetByte(pfile) << 16;
  ret |= GetByte(pfile) << 8;
  ret |= GetByte(pfile);
  return ret;
}

void Converter::Read(void * pv, size_t len)
{
  if (fread(pv, 1, len, pFileGsf.Get()) != len)
  {
    MIKTEX_FATAL_CRT_ERROR("fread");
  }
}

void Converter::Write(const void * pv, size_t len)
{
  if (fwrite(pv, 1, len, pFilePk.Get()) != len)
  {
    MIKTEX_FATAL_CRT_ERROR("fwrite");
  }
}

void Converter::Expect(const char * lpszWaitingFor, string * pLine)
{
  bool found = false;
  string line;
  if (pLine == nullptr)
  {
    pLine = &line;
  }
  size_t l = strlen(lpszWaitingFor);
  while (!found)
  {
    if (!Utils::ReadUntilDelim(*pLine, '\n', pFileGsf.Get()))
    {
      Error(T_("Premature end of file."));
    }
    found = (memcmp(pLine->c_str(), lpszWaitingFor, l) == 0);
  }
}

int Converter::GetInt()
{
  int c;
  do
  {
    c = GetByte();
  } while (isspace(c));
  if (c < '0' || c > '9')
  {
    Error(T_("Invalid Gsf file (digit expected)."));
  }
  int i = 0;
  do
  {
    i = i * 10 + (c - '0');
    c = GetByte();
  } while (c >= '0' && c <= '9');
  if (!feof(pFileGsf.Get()))
  {
    ungetc(c, pFileGsf.Get());
  }
  return i;
}

void Converter::WhiteSpace()
{
  bool cont = true;
  while (cont)
  {
    int c = GetByte();
    if (c == '#')
    {
      do
      {
        c = GetByte();
      } while (!feof(pFileGsf.Get()) && c != '\n');
    }
    else if (!isspace(c))
    {
      ungetc(c, pFileGsf.Get());
      cont = false;
    }
  }
}

void Converter::PutByte(int by) const
{
  if (fputc(by, pFilePk.Get()) == EOF)
  {
    MIKTEX_FATAL_CRT_ERROR("fputc");
  }
}

void Converter::PutDword(int dword) const
{
  PutByte((dword >> 24) & 0xff);
  PutByte((dword >> 16) & 0xff);
  PutByte((dword >> 8) & 0xff);
  PutByte(dword & 0xff);
}

void Converter::PutByteArray(const void * pv, size_t len) const
{
  PutByte(static_cast<int>(len));
  if (fwrite(pv, 1, len, pFilePk.Get()) != len)
  {
    MIKTEX_FATAL_CRT_ERROR("fwrite");
  }
}

int Converter::GetFirstByte(const PathName & file)
{
  AutoFILE pFile(File::Open(file, FileMode::Open, FileAccess::Read, false));
  int ch = fgetc(pFile.Get());
  pFile.Reset();
  return ch;
}

void Converter::ReadTFMFile(const char * lpszTeXFontName)
{
  lengths.resize(12, 0);
  widths.resize(256, 0);
  widthIndex.resize(256, 0);
  chars.reserve(1000);

  PathName pathTFMFile;

  if (!session->FindFile(lpszTeXFontName, FileType::TFM, pathTFMFile))
  {
    Error(T_("The TFM file could not be found."));
  }

  Verbose(T_("Reading TFM file %s..."), Q_(pathTFMFile));

  AutoFILE pFile(File::Open(pathTFMFile, FileMode::Open, FileAccess::Read, false));

  for (size_t i = 0; i < 12; ++i)
  {
    int j = GetByte(pFile.Get()) << 8;
    lengths[i] = j | GetByte(pFile.Get());
  }

  checkSum = GetDword(pFile.Get());
  designSize = GetDword(pFile.Get());

  if (fseek(pFile.Get(), 4 * (lh() + 6), SEEK_SET) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fseek");
  }

  for (int cc = bc(); cc <= ec(); ++cc)
  {
    widthIndex[cc] = GetByte(pFile.Get());
    if (widthIndex[cc] != 0)
    {
      if (chars.length() > 0)
      {
        chars += ' ';
      }
      chars += std::to_string(cc);
    }
    GetByte(pFile.Get());
    GetByte(pFile.Get());
    GetByte(pFile.Get());
  }
  chars += T_('\n');

  for (int i = 0; i < nw(); ++i)
  {
    widths[i] = GetDword(pFile.Get());
  }

  pFile.Reset();
}

Process * Converter::StartGhostscript(const char * lpszFontFile, const char * lpszEncFile, const char * lpszFontName, const char * lpszSpecInfo, const char * lpszDPI, FILE ** ppGsOut, FILE ** ppGsErr)
{
  PathName pathGs;

  session->GetGhostscript(pathGs.GetData(), nullptr);

  //
  // build the command-line
  //

  CommandLineBuilder commandLine;

  // - no device output
  commandLine.AppendOption("-dNODISPLAY");

  // - no garbage collection
  commandLine.AppendOption("-dNOGC");

  // - set font substitution
  commandLine.AppendOption("-sSUBSTFONT=", lpszFontName);

  // - be quiet
  commandLine.AppendOption("-q");

  // ???
  commandLine.AppendOption("--");

  // - path to render.ps
  PathName pathRenderPS;
  if (!session->FindFile("render.ps", FileType::PSHEADER, pathRenderPS))
  {
    Error(T_("The driver file render.ps could not be found."));
  }
  commandLine.AppendArgument(pathRenderPS.ToUnix());

  // - font name
  commandLine.AppendArgument(lpszFontName);

  // - font/enc load string
  string loadString;
  PathName pathFont;
  if (!session->FindFile(lpszFontFile, FileType::TYPE1, pathFont))
  {
    Error(T_("The font file could not be found."));
  }
  loadString = "(";
  loadString += pathFont.ToUnix().Get();
  loadString += ") ";
  int by = GetFirstByte(pathFont);
  if (by == 0)
  {
    loadString += "ttload";
  }
  else if (by == 0200)
  {
    loadString += "brun";
  }
  else
  {
    loadString += "run";
  }
  if (lpszEncFile != nullptr && *lpszEncFile != 0)
  {
    PathName pathEnc;
    if (!session->FindFile(lpszEncFile, FileType::ENC, pathEnc))
    {
      Error(T_("The encoding file could not be found."));
    }
    loadString += " (";
    loadString += pathEnc.ToUnix().Get();
    loadString += " ) run";
  }
  commandLine.AppendArgument(loadString.c_str());

  // - special info
  commandLine.AppendArgument(lpszSpecInfo == 0 ? "" : lpszSpecInfo);

  // - DPI
  commandLine.AppendArgument(lpszDPI);

  Verbose(T_("Starting Ghostscript with arguments:\n%s"), commandLine.ToString().c_str());

  ProcessStartInfo startinfo;

  startinfo.FileName = pathGs.ToString();
  startinfo.Arguments = commandLine.ToString();
  startinfo.StandardInput = nullptr;
  startinfo.RedirectStandardInput = true;
  startinfo.RedirectStandardOutput = true;
  startinfo.RedirectStandardError = true;

  unique_ptr<Process> pProcess(Process::Start(startinfo));

  AutoFILE pFileGsIn(pProcess->get_StandardInput());

  *ppGsOut = pProcess->get_StandardOutput();
  AutoFILE pFileGsOut(*ppGsOut);

  *ppGsErr = pProcess->get_StandardError();
  AutoFILE pFileGsErr(*ppGsErr);

  string designSizeString = std::to_string(static_cast<float>(designSize) / (1 << 20));
  designSizeString += '\n';

  // write the design size and character list to Gs stdin
  size_t n;
  if (((n = fwrite(designSizeString.c_str(), 1, designSizeString.length(), pFileGsIn.Get())) != designSizeString.length())
    || ((n = fwrite(chars.c_str(), 1, chars.length(), pFileGsIn.Get())) != chars.length()))
  {
    Error(T_("Ghostscript communication failure."));
  }

  // close input stream
  pFileGsIn.Reset();

  // keep output streams open
  pFileGsOut.Detach();
  pFileGsErr.Detach();

  return pProcess.release();
}

void Converter::StderrReader()
{
  try
  {
#define CHUNK_SIZE 64
    char buf[CHUNK_SIZE];
    size_t n;
    gsStdErr = "";
    while ((n = fread(buf, 1, CHUNK_SIZE, pFileGsErr.Get())) > 0)
    {
      for (size_t i = 0; i < n; ++i)
      {
        gsStdErr += buf[i];
      }
    }
  }
  catch (const exception &)
  {
  }
}

/* _________________________________________________________________________

   Converter::tallyup

   Add up statistics for putting out the given shift count.
   _________________________________________________________________________ */

void Converter::tallyup(int n)
{
  int m;
  if (n > 208)
  {
    ++base;
    n -= 192;
    for (m = 0x100; m != 0 && m < n; m <<= 4)
    {
      base += 2;
    }
    if (m != 0 && (m = (m - n) / 15) < 13)
    {
      deltas[m] += 2;
    }
  }
  else if (n > 13)
  {
    ++deltas[(208 - n) / 15];
  }
  else
  {
    --deltas[n - 1];
  }
}

/* _________________________________________________________________________

   Converter::pk_put_nyb

   Routines for storing the shift counts.
   _________________________________________________________________________ */

void Converter::pk_put_nyb(int n)
{
  if (odd)
  {
    *bitmap_end++ = static_cast<Byte>(((part << 4) | n) & 0xff);
    odd = false;
  }
  else
  {
    part = static_cast<Byte>(n & 0xff);
    odd = true;
  }
}

void Converter::pk_put_long(int n)
{
  if (n >= 16)
  {
    pk_put_nyb(0);
    pk_put_long(n / 16);
  }
  pk_put_nyb(n % 16);
}

void Converter::pk_put_count(int n)
{
  if (n > pk_dyn_f)
  {
    if (n > pk_dyn_g)
    {
      pk_put_long(n - pk_dyn_g + 15);
    }
    else
    {
      pk_put_nyb(pk_dyn_f + (n - pk_dyn_f + 15) / 16);
      pk_put_nyb((n - pk_dyn_f - 1) % 16);
    }
  }
  else
  {
    pk_put_nyb(n);
  }
}

void Converter::trim_bitmap()
{
  Byte * p;
  Byte mask;

  // clear out garbage bits in bitmap
  if (width % 8 != 0)
  {
    mask = ~masks[8 - width % 8];
    for (p = bitmap + bytesWide - 1; p < bitmap_end; p += bytesWide)
    {
      *p &= mask;
    }
  }

  /*
   * Find the bounding box of the bitmap.
   */

   // trim top
  skip = 0;
  mask = 0;
  for (;;)
  {
    if (bitmap >= bitmap_end)
    {                       // if bitmap is empty
      width = height = hoff = voff = 0;
      return;
    }
    p = bitmap + bytesWide;
    while (p > bitmap)
    {
      mask |= *--p;
    }
    if (mask)
    {
      break;
    }
    ++skip;
    bitmap += bytesWide;
  }
  height -= skip;
  voff -= skip;

  // trim bottom
  skip = 0;
  mask = 0;
  for (;;)
  {
    p = bitmap_end - bytesWide;
    while (p < bitmap_end)
    {
      mask |= *p++;
    }
    if (mask)
    {
      break;
    }
    ++skip;
    bitmap_end -= bytesWide;
  }
  height -= skip;

  // trim right
  skip = 0;
  --width;
  for (;;)
  {
    mask = 0;
    for (p = bitmap + width / 8; p < bitmap_end; p += bytesWide)
    {
      mask |= *p;
    }
    if (mask & (0x80 >> (width % 8)))
    {
      break;
    }
    --width;
    ++skip;
  }
  ++width;

  // trim left
  skip = 0;
  for (;;)
  {
    mask = 0;
    for (p = bitmap + skip / 8; p < bitmap_end; p += bytesWide)
    {
      mask |= *p;
    }
    if (mask & (0x80 >> (skip % 8)))
    {
      break;
    }
    ++skip;
  }
  width -= skip;
  hoff -= skip;
  bitmap += skip / 8;
  skip = skip % 8;
}

/* _________________________________________________________________________

   Converter::pk_rll_cvt

   Pack the bitmap using the rll method.  (Return false if it's better
   to just pack the bits.)
   _________________________________________________________________________ */

bool Converter::pk_rll_cvt()
{
  unsigned int ncounts;         // max to allow this time
  int * nextcount;              // next count value
  int * counts_end;             // pointer to end
  Byte * rowptr;
  Byte * p;
  Byte mask;
  Byte * rowdup;                // last row checked for dup
  Byte paint_switch;            // 0 or 0xff
  int bits_left;                // bits left in row
  int cost;
  int i;

  /*
   * Allocate space for bit counts.
   */

  ncounts = (width * height + 3) / 4;
  if (ncounts > counts.size())
  {
    counts.resize(ncounts);
  }
  counts_end = &counts[0] + ncounts;

  /*
   * Form bit counts and collect statistics
   */

  base = 0;
  memset(deltas, 0, sizeof(deltas));
  rowdup = 0;                   // last row checked for duplicates
  p = rowptr = bitmap;
  mask = static_cast<Byte>(0x80 >> skip);
  flag = 0;
  paint_switch = 0;
  if (*p & mask)
  {
    flag = 8;
    paint_switch = 0xff;
  }
  bits_left = width;
  nextcount = &counts[0];
  while (rowptr < bitmap_end)
  {                           // loop over shift counts
    int shift_count = bits_left;

    for (;;)
    {
      if (bits_left == 0)
      {
        if ((p = rowptr += bytesWide) >= bitmap_end) break;
        mask = static_cast<Byte>(0x80 >> skip);
        bits_left = width;
        shift_count += width;
      }
      if (((*p ^ paint_switch) & mask) != 0)
      {
        break;
      }
      --bits_left;
      mask >>= 1;
      if (mask == 0)
      {
        ++p;
        while (*p == paint_switch && bits_left >= 8)
        {
          ++p;
          bits_left -= 8;
        }
        mask = 0x80;
      }
    }
    if (nextcount >= counts_end)
    {
      return false;
    }
    shift_count -= bits_left;
    *nextcount++ = shift_count;
    tallyup(shift_count);
    // check for duplicate rows
    if (rowptr != rowdup && bits_left != width)
    {
      Byte * p1 = rowptr;
      Byte * q = rowptr + bytesWide;
      int repeat_count;

      while (q < bitmap_end && *p1 == *q)
      {
        ++p1;
        ++q;
      }
      repeat_count = static_cast<int>((p1 - rowptr) / bytesWide);
      if (repeat_count > 0)
      {
        *nextcount++ = -repeat_count;
        if (repeat_count == 1)
        {
          --base;
        }
        else
        {
          ++base;
          tallyup(repeat_count);
        }
        rowptr += repeat_count * bytesWide;
      }
      rowdup = rowptr;
    }
    paint_switch = ~paint_switch;
  }

  /*
   * Determine the best pk_dyn_f
   */

  pk_dyn_f = 0;
  cost = base += static_cast<int>(2 * (nextcount - &counts[0]));
  for (i = 1; i < 14; ++i)
  {
    base += deltas[i - 1];
    if (base < cost)
    {
      pk_dyn_f = i;
      cost = base;
    }
  }
  // last chance to bail out
  if (cost * 4 > width * height)
  {
    return false;
  }

  /*
   * Pack the bit counts
   */

  pk_dyn_g = 208 - 15 * pk_dyn_f;
  flag |= pk_dyn_f << 4;
  bitmap_end = bitmap;
  *nextcount = 0;
  nextcount = &counts[0];
  while (*nextcount != 0)
  {
    if (*nextcount > 0)
    {
      pk_put_count(*nextcount);
    }
    else
    {
      if (*nextcount == -1)
      {
        pk_put_nyb(15);
      }
      else
      {
        pk_put_nyb(14);
        pk_put_count(-*nextcount);
      }
    }
    ++nextcount;
  }
  if (odd)
  {
    pk_put_nyb(0);
    ++cost;
  }
  if (cost != 2 * (bitmap_end - bitmap))
  {
    printf("Cost miscalculation:  expected %d, got %d\n", cost, (int)(2 * (bitmap_end - bitmap)));
  }
  pk_len = static_cast<int>(bitmap_end - bitmap);
  return true;
}

void Converter::pk_bm_cvt()
{
  Byte * rowptr;
  Byte * p;
  int blib1;                    // bits left in byte
  int bits_left;                // bits left in row
  Byte * q;
  int blib2;
  Byte nextbyte;

  flag = 14 << 4;
  q = bitmap;
  blib2 = 8;
  nextbyte = 0;
  for (rowptr = bitmap; rowptr < bitmap_end; rowptr += bytesWide)
  {
    p = rowptr;
    blib1 = 8 - skip;
    bits_left = width;
    if (blib2 != 8)
    {
      int n;

      if (blib1 < blib2)
      {
        nextbyte |= *p << (blib2 - blib1);
        n = blib1;
      }
      else
      {
        nextbyte |= *p >> (blib1 - blib2);
        n = blib2;
      }
      blib2 -= n;
      if ((bits_left -= n) < 0)
      {
        blib2 -= bits_left;
        continue;
      }
      if ((blib1 -= n) == 0)
      {
        blib1 = 8;
        ++p;
        if (blib2 > 0)
        {
          nextbyte |= *p >> (8 - blib2);
          blib1 -= blib2;
          bits_left -= blib2;
          if (bits_left < 0)
          {
            blib2 = -bits_left;
            continue;
          }
        }
      }
      *q++ = nextbyte;
    }
    // fill up whole (destination) bytes
    while (bits_left >= 8)
    {
      nextbyte = static_cast<Byte>(((*p++ << (8 - blib1)) & 0xff) & 0xff);
      *q++ = static_cast<Byte>(((nextbyte | (*p >> blib1)) & 0xff) & 0xff);
      bits_left -= 8;
    }
    // now do the remainder
    nextbyte = static_cast<Byte>((*p << (8 - blib1)) & 0xff);
    if (bits_left > blib1)
    {
      nextbyte |= p[1] >> blib1;
    }
    blib2 = 8 - bits_left;
  }
  if (blib2 != 8)
  {
    *q++ = nextbyte;
  }
  pk_len = static_cast<int>(q - bitmap);
}

void Converter::putshort(int w)
{
  PutByte(w >> 8);
  PutByte(w);
}

void Converter::putmed(long w)
{
  PutByte(w >> 16);
  PutByte(w >> 8);
  PutByte(w);
}

void Converter::putlong(long w)
{
  PutByte(w >> 24);
  PutByte(w >> 16);
  PutByte(w >> 8);
  PutByte(w);
}

void Converter::PutGlyph(int cc)
{
  if (!haveFirstLine)
  {
    string line;
    Expect("#^", &line);
    if (SScanF(line.c_str(), "#^ %d %d %d %d %d %f\n", &idx, &llx, &lly, &urx, &ury, &charWidth) != 6)
    {
      Error(T_("Invalid Gsf file."));
    }
  }
  if (idx < cc)
  {
    Error(T_("Invalid Gsf file."));
  }
  if (idx > cc)
  {
    cerr << T_("Missing character: ") << cc << endl;
    haveFirstLine = true;
    return;
  }
  haveFirstLine = false;
  hoff = -llx + 2;
  voff = ury + 2 - 1;
  Expect("P4\n");
  WhiteSpace();
  width = GetInt();
  WhiteSpace();
  height = GetInt();
  GetByte();
  if (width != urx - llx + 4 || height != ury - lly + 4)
  {
    Error(T_("Dimensions do not match"));
  }
  bytesWide = (width + 7) / 8;
  bitmapSize = bytesWide * height;
  area1.resize(bitmapSize);
  for (Byte * p = &area1[(height - 1) * bytesWide]; p >= &area1[0]; p -= bytesWide)
  {
    Read(p, bytesWide);
  }
  bitmap = &area1[0];
  bitmap_end = bitmap + bitmapSize;
  trim_bitmap();
  if (height == 0 || !pk_rll_cvt())
  {
    pk_bm_cvt();
  }
  int tfm_wid = widths[widthIndex[cc]];
  int dm =
    static_cast<int>((charWidth + 0.5) - (charWidth < -0.5));
  if (pk_len + 8 < 4 * 256 && tfm_wid < (1 << 24)
    && dm >= 0 && dm < 256 && width < 256 && height < 256
    && hoff >= -128 && hoff < 128 && voff >= -128 && voff < 128)
  {
    PutByte(flag | ((pk_len + 8) >> 8));
    PutByte(pk_len + 8);
    PutByte(cc);
    putmed(tfm_wid);
    PutByte(dm);
    PutByte(width);
    PutByte(height);
    PutByte(hoff);
    PutByte(voff);
  }
  else if (pk_len + 13 < 3 * 65536L && tfm_wid < (1 << 24)
    && dm >= 0 && dm < 65536 && width < 65536 && height < 65536
    && hoff >= -65536 && hoff < 65536
    && voff >= -65536 && voff < 65536)
  {
    PutByte(flag | 4 | ((pk_len + 13) >> 16));
    putshort(pk_len + 13);
    PutByte(cc);
    putmed(tfm_wid);
    putshort(dm);
    putshort(width);
    putshort(height);
    putshort(hoff);
    putshort(voff);
  }
  else
  {
    PutByte(flag | 7);
    putlong(pk_len + 28);
    putlong(cc);
    putlong(tfm_wid);
    putlong(static_cast<int>((charWidth * 65536.0 + 0.5)
      - (charWidth < -0.5)));
    putlong(0);
    putlong(width);
    putlong(height);
    putlong(hoff);
    putlong(voff);
  }

  Write(bitmap, pk_len);
}

void Converter::putspecl(const char * str1, const char * str2)
{
  int len1 = static_cast<int>(strlen(str1));
  int len2 = 0;
  if (str2 != 0)
  {
    len2 = static_cast<int>(strlen(str2));
  }
  if (len1 + len2 > 255)
  {
    return;
  }
  const int PK_SPC = 240;
  PutByte(PK_SPC);
  PutByte(len1 + len2);
  Write(str1, len1);
  if (len2 != 0)
  {
    Write(str2, len2);
  }
}

void Converter::WritePkFile(const char * lpszPkFile)
{
  Verbose(T_("Writing Pk file %s..."), Q_(lpszPkFile));

  pFilePk.Reset(File::Open(lpszPkFile, FileMode::Create, FileAccess::Write, false));

  const int PK_PRE = 247;
  const int PK_ID = 89;
  const int PK_POST = 245;
  const int PK_NOP = 246;

  PutByte(PK_PRE);
  PutByte(PK_ID);

  string line;

  // read version number / write id string
  Expect("V", &line);
  string id = "gsf2pk ";
  id += MIKTEX_COMPONENT_VERSION_STR;
  int gsVersion;
  if (SScanF(line.c_str(), "V %d", &gsVersion) == 1)
  {
    id += '/';
    id += std::to_string(gsVersion);
  }
  PutByteArray(id.c_str(), id.length());

  // write design size / checksum / page size
  PutDword(designSize);
  PutDword(checkSum);
  int ppp = static_cast<int>(dpi / 72.27 * 65536.0 + 0.5);
  PutDword(ppp);               // hppp
  PutDword(ppp);               // vppp

  // write bitmaps
  for (int cc = bc(); cc <= ec(); ++cc)
  {
    if (widthIndex[cc] != 0)
    {
      PutGlyph(cc);
    }
  }

  // write info specials
  putspecl("jobname=", fontName.c_str());
  putspecl("mag=1");
  putspecl("mode=modeless");
  putspecl("pixels_per_inch=", std::to_string(dpi).c_str());

  // write post-amble
  PutByte(PK_POST);
  while (ftell(pFilePk.Get()) % 4 != 0)
  {
    PutByte(PK_NOP);
  }

  pFilePk.Reset();
}

void Converter::Convert(const char * lpszTeXFontName, const char * lpszFontName, const char * lpszSpecInfo, const char * lpszEncFile, const char * lpszFontFile, const char * lpszDPI, const char * lpszPkFile)
{
  fontName = lpszTeXFontName;
  dpi = atoi(lpszDPI);

  ReadTFMFile(lpszTeXFontName);

  try
  {
    unique_ptr<Process> pGhostscript(StartGhostscript(lpszFontFile, lpszEncFile, lpszFontName, lpszSpecInfo, lpszDPI, &pFileGsf, &pFileGsErr));

    thread stderrReader(&Converter::StderrReader, this);

    WritePkFile(lpszPkFile);

    pFileGsf.Reset();

    if (!pGhostscript->WaitForExit(10000))
    {
      Error(T_("Ghostscript didn't complete."));
    }

    if (pGhostscript->get_ExitCode() != 0)
    {
      Error(T_("Ghostscript failed."));
    }

    pFileGsErr.Reset();
    stderrReader.join();
    pGhostscript.reset();
  }

  catch (const exception &)
  {
    pFileGsf.Reset();
    throw;
  }

}

void Converter::Main(int argc, const char * * argv)
{
  PoptWrapper popt(argc, argv, aoption);

  // process command-line options
  int option;
  while ((option = popt.GetNextOpt()) >= 0)
  {
    switch (option)
    {
    case OPT_QUIET:
      if (verbose)
      {
        Error(T_("Cannot be --verbose and --quiet at the same time."));
      }
      quiet = true;
      break;
    case OPT_VERBOSE:
      if (quiet)
      {
        Error(T_("Cannot be --verbose and --quiet at the same time."));
      }
      verbose = true;
      break;
    case OPT_VERSION:
      cout
        << Utils::MakeProgramVersionString("gsf2pk", VersionNumber(MIKTEX_MAJOR_VERSION, MIKTEX_MINOR_VERSION, MIKTEX_COMP_J2000_VERSION, 0)) << endl
        << "Copyright (C) 2004-2016 Christian Schenk" << endl
        << "Copyright (C) 1993-2000 Paul Vojta" << endl
        << "This is free software; see the source for copying conditions.  There is NO" << endl
        << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
      return;
    }
  }

  if (option != -1)
  {
    string msg = popt.BadOption(POPT_BADOPTION_NOALIAS);
    msg += ": ";
    msg += popt.Strerror(option);
    Error("%s", msg.c_str());
  }

  vector<string> leftovers = popt.GetLeftovers();

  if (leftovers.size() != 7)
  {
    Error(T_("Wrong number of command-line arguments."));
  }

  Init(argv[0]);

  Convert(leftovers[0].c_str(),  // "utmr8r"
    leftovers[1].c_str(),        // "NimbusRomNo9L-Regu"
    leftovers[2].c_str(),        // " TeXBase1Encoding ReEncodeFont "
    leftovers[3].c_str(),        // "8r.enc"
    leftovers[4].c_str(),        // "utmr8a.pfb"
    leftovers[5].c_str(),        // "600"
    leftovers[6].c_str());       // "utmr8a.pk"

  Finalize();
}

#if defined(_UNICODE)
#  define MAIN wmain
#  define MAINCHAR wchar_t
#else
#  define MAIN main
#  define MAINCHAR char
#endif

int MAIN(int argc, MAINCHAR ** argv)
{
  try
  {
    vector<string> utf8args;
    utf8args.reserve(argc);
    vector<const char *> newargv;
    newargv.reserve(argc + 1);
    for (int idx = 0; idx < argc; ++idx)
    {
#if defined(_UNICODE)
      utf8args.push_back(StringUtil::WideCharToUTF8(argv[idx]));
#elif defined(MIKTEX_WINDOWS)
      utf8args.push_back(StringUtil::AnsiToUTF8(argv[idx]));
#else
      utf8args.push_back(argv[idx]);
#endif
      newargv.push_back(utf8args[idx].c_str());
    }
    newargv.push_back(nullptr);
    Converter conv;
    conv.Main(argc, &newargv[0]);
    return 0;
  }

  catch (const MiKTeXException & e)
  {
    Application::Sorry("gsf2pk", e);
    return 1;
  }

  catch (const exception & e)
  {
    Application::Sorry("gsf2pk", e);
    return 1;
  }

  catch (int exitCode)
  {
    return exitCode;
  }
}
