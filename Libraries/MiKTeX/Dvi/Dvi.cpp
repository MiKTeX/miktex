/* dvi.cpp:

   Copyright (C) 1996-2020 Christian Schenk

   This file is part of the MiKTeX DVI Library.

   The MiKTeX DVI Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2, or (at your option) any later version.

   The MiKTeX DVI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the MiKTeX DVI Library; if not, write to the
   Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
   USA.  */

   /* code is heavily borrowed from DVItype and other celebs */

#include "config.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Quoter>

#include "internal.h"

void DviImpl::PushState()
{
  stateStack.push(currentState);
}

void DviImpl::PopState()
{
  MIKTEX_ASSERT(!stateStack.empty());
  currentState = stateStack.top();
  stateStack.pop();
}

// Round a DVI unit to the nearest pixel value.
int DviImpl::PixelRound(int du)
{
  if (du >= 0)
  {
    return static_cast<int>(du * conv + 0.5);
  }
  else
  {
    return static_cast<int>(du * conv - 0.5);
  }
}

// Compute the number of pixels in the height or width of a rule.
int DviImpl::RulePixels(int x)
{
  int n = static_cast<int>(conv * x);
  if (static_cast<double>(n) < (conv * x))
  {
    return n + 1;
  }
  else
  {
    return n;
  }
}

DviImpl::DviImpl(const char* fileName, const char* metafontMode, int resolution, int shrinkFactor, DviAccess dviAccess, DviPageMode pageMode, const PaperSizeInfo & paperSizeInfo, bool landscape, IDviCallback* dviCallback, TraceCallback* traceCallback) :
  currentColor(rgbDefaultColor),
  dviAccess(dviAccess),
  dviFileName(fileName),
  landscape(landscape),
  metafontMode(metafontMode),
  dviCallback(dviCallback),
  traceCallback(traceCallback),
  pageMode(pageMode),
  paperSizeInfo(paperSizeInfo),
  resolution(resolution),
  defaultShrinkFactor(shrinkFactor)
{
  trace_color = TraceStream::Open(MIKTEX_TRACE_DVICOLOR, traceCallback);
  trace_dvifile = TraceStream::Open(MIKTEX_TRACE_DVIFILE, traceCallback);
  trace_dvipage = TraceStream::Open(MIKTEX_TRACE_DVIPAGE, traceCallback);
  trace_error = TraceStream::Open(MIKTEX_TRACE_ERROR, traceCallback);
  trace_gc = TraceStream::Open(MIKTEX_TRACE_DVIGC, traceCallback);
  trace_hypertex = TraceStream::Open(MIKTEX_TRACE_DVIHYPERTEX, traceCallback);
  trace_search = TraceStream::Open(MIKTEX_TRACE_DVISEARCH, traceCallback);

  fqDviFileName = dviFileName;

  fqDviFileName.MakeFullyQualified();

  dviInfo.lastWriteTime = 0;
  fontMap = new FontMap;
  hByeByeEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
  if (hByeByeEvent == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CreateEventW");
  }
  hNewPageEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
  if (hNewPageEvent == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CreateEventW");
  }
  hScannedEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
  if (hScannedEvent == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CreateEventW");
  }
  if (dviAccess == DviAccess::Random)
  {
    garbageCollectorThread = thread(&DviImpl::GarbageCollector, this);
    pageLoaderThread = thread(&DviImpl::PageLoader, this);
  }
}

DviImpl::~DviImpl()
{
  try
  {
    Dispose();
  }
  catch (const exception&)
  {
  }
}

void DviImpl::Dispose()
{
  if (hByeByeEvent != nullptr)
  {
    if (!SetEvent(hByeByeEvent))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("SetEvent");
    }
  }
  if (garbageCollectorThread.joinable())
  {
    garbageCollectorThread.join();
  }
  if (pageLoaderThread.joinable())
  {
    pageLoaderThread.join();
  }
  BEGIN_CRITICAL_SECTION(dviMutex)
  {
    FreeContents();
    if (fontMap != nullptr)
    {
      delete fontMap;
      fontMap = nullptr;
    }
    if (hByeByeEvent != nullptr)
    {
      CloseHandle(hByeByeEvent);
      hByeByeEvent = nullptr;
    }
    if (hNewPageEvent != nullptr)
    {
      CloseHandle(hNewPageEvent);
      hNewPageEvent = nullptr;
    }
    if (hScannedEvent != nullptr)
    {
      CloseHandle(hScannedEvent);
      hScannedEvent = nullptr;
    }
  }
  END_CRITICAL_SECTION();
  if (trace_dvifile != nullptr)
  {
    trace_dvifile->Close();
    trace_dvifile = nullptr;
  }
  if (trace_color != nullptr)
  {
    trace_color->Close();
    trace_color = nullptr;
  }
  if (trace_dvipage != nullptr)
  {
    trace_dvipage->Close();
    trace_dvipage = nullptr;
  }
  if (trace_error != nullptr)
  {
    trace_error->Close();
    trace_error = nullptr;
  }
  if (trace_gc != nullptr)
  {
    trace_gc->Close();
    trace_gc = nullptr;
  }
  if (trace_hypertex != nullptr)
  {
    trace_hypertex->Close();
    trace_hypertex = nullptr;
  }
  if (trace_search != nullptr)
  {
    trace_search->Close();
    trace_search = nullptr;
  }
}

void DviImpl::FreeContents(bool keepFonts)
{
  for (vector<DviPageImpl*>::iterator itPagePtr = pages.begin(); itPagePtr != pages.end(); ++itPagePtr)
  {
    delete *itPagePtr;
  }
  pages.clear();
  if (fontMap == nullptr || keepFonts)
  {
    return;
  }
  for (FontMap::iterator it = fontMap->begin(); it != fontMap->end(); ++it)
  {
    delete it->second;
    it->second = nullptr;
  }
  fontMap->clear();
  tempFiles.clear();
}

const int set_char = 0;
const int set1 = 128;
const int set2 = 129;
const int set3 = 130;
const int set4 = 131;
const int set_rule = 132;
const int put1 = 133;
const int put2 = 134;
const int put3 = 135;
const int put4 = 136;
const int put_rule = 137;
const int nop = 138;
const int bop = 139;
const int eop = 140;
const int p_ush = 141;
const int p_op = 142;
const int right1 = 143;
const int right2 = 144;
const int right3 = 145;
const int right4 = 146;
const int w0 = 147;
const int w1 = 148;
const int w2 = 149;
const int w3 = 150;
const int w4 = 151;
const int x0 = 152;
const int x1 = 153;
const int x2 = 154;
const int x3 = 155;
const int x4 = 156;
const int down1 = 157;
const int down2 = 158;
const int down3 = 159;
const int down4 = 160;
const int y_0 = 161;
const int y_1 = 162;
const int y2 = 163;
const int y3 = 164;
const int y4 = 165;
const int z0 = 166;
const int z1 = 167;
const int z2 = 168;
const int z3 = 169;
const int z4 = 170;
const int fnt_num = 171;
const int fnt1 = 235;
const int fnt2 = 236;
const int fnt3 = 237;
const int fnt4 = 238;
const int xxx1 = 239;
const int xxx2 = 240;
const int xxx3 = 241;
const int xxx4 = 242;
const int fnt_def1 = 243;
const int fnt_def2 = 244;
const int fnt_def3 = 245;
const int fnt_def4 = 246;
const int pre = 247;
const int post = 248;
const int post_post = 249;

#define FOUR_CASES(x) x: case x+1: case x+2: case x+3
#define SIX_CASES(x) FOUR_CASES(x): case x+4: case x+5

#define undefined_commands SIX_CASES(250)

int DviImpl::FirstParam(InputStream & inputStream, int opCode)
{
  if (opCode >= set_char && opCode <= set_char + 127)
  {
    return opCode - set_char;
  }

  switch (opCode)
  {

  case set1: case put1: case fnt1: case xxx1: case fnt_def1:

    return inputStream.ReadByte();

  case set1 + 1: case put1 + 1: case fnt1 + 1: case xxx1 + 1: case fnt_def1 + 1:

    return inputStream.ReadPair();

  case set1 + 2: case put1 + 2: case fnt1 + 2: case xxx1 + 2: case fnt_def1 + 2:

    return inputStream.ReadTrio();

  case right1: case w1: case x1: case down1: case y_1: case z1:

    return inputStream.ReadSignedByte();

  case right1 + 1: case w1 + 1: case x1 + 1: case down1 + 1: case y_1 + 1: case z1 + 1:

    return inputStream.ReadSignedPair();

  case right1 + 2: case w1 + 2: case x1 + 2: case down1 + 2: case y_1 + 2: case z1 + 2:

    return inputStream.ReadSignedTrio();

  case set1 + 3: case set_rule: case put1 + 3: case put_rule: case right1 + 3:
  case w1 + 3: case x1 + 3: case down1 + 3: case y_1 + 3: case z1 + 3: case fnt1 + 3:
  case xxx1 + 3: case fnt_def1 + 3:

    return inputStream.ReadSignedQuad();

  case nop: case bop: case eop: case p_ush: case p_op: case pre: case post:
  case post_post: case undefined_commands:

    return 0;

  case w0:

    return currentState.w;

  case x0:

    return currentState.x;

  case y_0:

    return currentState.y;

  case z0:

    return currentState.z;
  }

  if (opCode >= fnt_num && opCode <= fnt_num + 63)
  {
    return opCode - fnt_num;
  }

  MIKTEX_UNEXPECTED();
}

const int dvi_id = 2;

void DviImpl::Scan()
{
  InputStream inputStream(dviFileName.GetData());

  // reset this object
  FreeContents(false);

  trace_dvifile->WriteLine("libdvi", fmt::format(T_("going to scan {0}"), Q_(dviFileName)));

  // prepare reading the Dvi file
  dviInfo.lastWriteTime = File::GetLastWriteTime(dviFileName);

  // process the preamble
  if (inputStream.ReadByte() != pre || inputStream.ReadByte() != dvi_id)
  {
    FATAL_DVI_ERROR_2(T_("Not a DVI file."), "fileName", dviFileName.ToString());
  }

  // compute the conversion factor
  numerator = inputStream.ReadSignedQuad();
  denominator = inputStream.ReadSignedQuad();

  trace_dvifile->WriteLine("libdvi", fmt::format("numerator/denominator: {0}/{1}", numerator, denominator));

  if (numerator <= 0 || denominator <= 0)
  {
    FATAL_DVI_ERROR_2(T_("Invalid DVI file."), "fileName", dviFileName.ToString());
  }

  // get desired magnification
  mag = inputStream.ReadSignedQuad();
  trace_dvifile->WriteLine("libdvi", fmt::format("mag: {0}", mag));

  tfmConv = (((25400000.0 / numerator) * (denominator / 473628672)) / 16.0);

  conv = ((static_cast<double>(numerator) * static_cast<double>(mag) * static_cast<double>(resolution)) / (static_cast<double>(denominator) * 254000000.0));

  trace_dvifile->WriteLine("libdvi", fmt::format("conv: {0}", conv));

  // read the introductory comment
  int len = inputStream.ReadByte();
  char tmp[256];
  inputStream.Read(tmp, len);
  tmp[len] = 0;
  dviInfo.comment = tmp;

  trace_dvifile->WriteLine("libdvi", fmt::format("comment: {0}", dviInfo.comment));

  // find the postamble, working back from the end
  int k;
  inputStream.SetReadPosition(0, SeekOrigin::End);
  int filelength = inputStream.GetReadPosition();
  int m = filelength - 1;
  do
  {
    inputStream.SetReadPosition(m, SeekOrigin::Begin);
    k = inputStream.ReadByte();
    --m;
  } while (k == 223);
  if (k != dvi_id)
  {
    FATAL_DVI_ERROR_2(T_("Invalid DVI file."), "fileName", dviFileName.ToString());
  }
  inputStream.SetReadPosition(m - 3, SeekOrigin::Begin);
  int q = inputStream.ReadSignedQuad();
  if (q < 0 || q > m - 33)
  {
    FATAL_DVI_ERROR_2(T_("Invalid DVI file."), "fileName", dviFileName.ToString());
  }
  inputStream.SetReadPosition(q, SeekOrigin::Begin);
  k = inputStream.ReadByte();
  if (k != post)
  {
    FATAL_DVI_ERROR_2(T_("Invalid DVI file."), "fileName", dviFileName.ToString());
  }

  // process the postamble
  int firstbackpointer =        // pointer to last page
    inputStream.ReadSignedQuad();
  inputStream.ReadSignedQuad(); // postamble_num
  inputStream.ReadSignedQuad(); // postamble_den
  inputStream.ReadSignedQuad(); // postamble_mag
  maxV = inputStream.ReadSignedQuad();
  maxH = inputStream.ReadSignedQuad();
  trace_dvifile->WriteLine("libdvi", fmt::format("maxv: {0}", maxV));
  trace_dvifile->WriteLine("libdvi", fmt::format("maxh: {0}", maxH));
  int maxs = inputStream.ReadPair();
  dviInfo.nPages = inputStream.ReadPair();
  trace_dvifile->WriteLine("libdvi", fmt::format("maxs: {0}", maxs));
  trace_dvifile->WriteLine("libdvi", fmt::format("dviInfo.nPages: {0}", dviInfo.nPages));

  // process the font definitions of the postamble
  do
  {
    k = inputStream.ReadByte();
    if (k >= fnt_def1 && k < fnt_def1 + 4)
    {
      int p = FirstParam(inputStream, k);
      DefineFont(inputStream, p);
      k = nop;
    }
  } while (k == nop);

  if (k != post_post)
  {
    FATAL_DVI_ERROR_2(T_("Invalid DVI file."), "fileName", dviFileName.ToString());
  }

  // build the page table
  int backpointer = firstbackpointer;
  minPageNumber = INT_MAX;
  maxPageNumber = INT_MIN;
  pages.reserve(dviInfo.nPages);
  while (backpointer >= 0 && (dviInfo.nPages-- > 0))
  {
    inputStream.SetReadPosition(backpointer, SeekOrigin::Begin);
    if (inputStream.ReadByte() != bop)
    {
      FATAL_DVI_ERROR_2(T_("Invalid DVI file."), "fileName", dviFileName.ToString());
    }
    int count0 = inputStream.ReadSignedQuad();
    if (count0 < minPageNumber)
    {
      minPageNumber = count0;
    }
    if (count0 > maxPageNumber)
    {
      maxPageNumber = count0;
    }
    int count1 = inputStream.ReadSignedQuad();
    int count2 = inputStream.ReadSignedQuad();
    int count3 = inputStream.ReadSignedQuad();
    int count4 = inputStream.ReadSignedQuad();
    int count5 = inputStream.ReadSignedQuad();
    int count6 = inputStream.ReadSignedQuad();
    int count7 = inputStream.ReadSignedQuad();
    int count8 = inputStream.ReadSignedQuad();
    int count9 = inputStream.ReadSignedQuad();
    backpointer = inputStream.ReadSignedQuad();
    DviPageImpl* dviPage = new DviPageImpl(this, dviInfo.nPages, pageMode, inputStream.GetReadPosition(), count0, count1, count2, count3, count4, count5, count6, count7, count8, count9);
    pages.push_back(dviPage);
  }
  reverse(pages.begin(), pages.end());
  lastChecked = clock();

#if 0
  // load the first DVI page
  if (pages.size() > 0)
  {
    DviPage* dviPage = GetLoadedPage(0);
    dviPage->Unlock();
  }
#endif

  // wake up page loader thread
  SetEvent(hScannedEvent);

  hasDviFileChanged = false;
}

void DviImpl::DefineFont(InputStream & inputStream, int fontNum)
{
  trace_dvifile->WriteLine("libdvi", fmt::format(T_("going to define font {0}"), fontNum));

  int checkSum = inputStream.ReadSignedQuad();
  int scaledSize = inputStream.ReadSignedQuad();
  int designSize = inputStream.ReadSignedQuad();
  int areaNameLen = inputStream.ReadByte();
  int fontNameLen = inputStream.ReadByte();

  char areaName[256];
  if (areaNameLen > 0)
  {
    inputStream.Read(areaName, areaNameLen);
  }
  areaName[areaNameLen] = 0;

  char fontName[256];
  inputStream.Read(fontName, fontNameLen);
  fontName[fontNameLen] = 0;

  trace_dvifile->WriteLine("libdvi", fmt::format("areaName: {0}", areaName));
  trace_dvifile->WriteLine("libdvi", fmt::format("fontname: {0}", fontName));
  trace_dvifile->WriteLine("libdvi", fmt::format("checkSum: {0:o}", checkSum));
  trace_dvifile->WriteLine("libdvi", fmt::format("scaledSize: {0}", scaledSize));
  trace_dvifile->WriteLine("libdvi", fmt::format("designSize: {0}", designSize));

  DviFont* dviFont;
  PathName fileName;
  if (session->FindFile(fontName, FileType::VF, fileName)
    || session->FindFile(fontName, FileType::OVF, fileName))
  {
    trace_dvifile->WriteLine("libdvi", fmt::format(T_("found VF file {0}"), Q_(fileName)));
    dviFont = new VFont(this, checkSum, scaledSize, designSize, areaName, fontName, fileName.GetData(), tfmConv, conv, mag, metafontMode.c_str(), resolution);
  }
  else if (pageMode == DviPageMode::Dvips)
  {
    // no need to display glyph bitmaps
    dviFont = new Tfm(this, checkSum, scaledSize, designSize, areaName, fontName, "", tfmConv, conv);
  }
  else
  {
    dviFont = new PkFont(this, checkSum, scaledSize, designSize, areaName, fontName, "", tfmConv, conv, mag, metafontMode.c_str(), resolution);
  }

  (*fontMap)[fontNum] = dviFont;
}

PageStatus DviImpl::GetPageStatus(int pageIdx)
{
  CheckCondition();
  BEGIN_CRITICAL_SECTION(dviMutex)
  {
    MIKTEX_ASSERT(pageIdx >= 0);

    if (pageIdx >= GetNumberOfPages())
    {
      return PageStatus::Unknown;
    }

    DviPageImpl* dviPage = pages[pageIdx];

    MIKTEX_ASSERT(dviPage != nullptr);

    dviPage->Lock();

    AutoUnlockPage autoUnlockPage(dviPage);

    if (!dviPage->IsFrozen())
    {
      return PageStatus::NotLoaded;
    }

    if (hasDviFileChanged)
    {
      return PageStatus::Changed;
    }

    clock_t now = clock();

    if (now <= lastChecked + CLOCKS_PER_SEC)
    {
      return PageStatus::Loaded;
    }

    lastChecked = now;

    if (session->IsFileAlreadyOpen(dviFileName))
    {
      return PageStatus::Loaded;
    }

    if (!File::Exists(dviFileName))
    {
      return PageStatus::Lost;
    }

    time_t oldTime = dviInfo.lastWriteTime;
    dviInfo.lastWriteTime = File::GetLastWriteTime(dviFileName);
    if (oldTime != dviInfo.lastWriteTime)
    {
      hasDviFileChanged = true;
      return PageStatus::Changed;
    }

    return PageStatus::Loaded;
  }
  END_CRITICAL_SECTION();
}

// we insist that abs(hh-pixelRound(h)) <= max_drift
const int max_drift = 1;

void DviImpl::DoPage(int pageIdx)
{
  currentFontNumber = 0;
  currentFont = nullptr;
  currentChar = nullptr;
  currentState = DviState();
  currentFontNumber = 0;

  DviPageImpl & page = *pages[pageIdx];

  page.Lock();

  AutoUnlockPage autoUnlockPage(&page);

  Progress(DviNotification::BeginLoadPage, fmt::format(T_("loading page #{0}..."), pageIdx));

  bool background = pageLoaderThread.joinable() && this_thread::get_id() == pageLoaderThread.get_id();

  if (background)
  {
    trace_dvipage->WriteLine("libdvi", fmt::format(T_("doing page #{0} (background)"), pageIdx));
  }
  else
  {
    trace_dvipage->WriteLine("libdvi", fmt::format(T_("doing page #{0}"), pageIdx));
  }

  try
  {
    if (session->IsFileAlreadyOpen(dviFileName))
    {
      trace_error->WriteLine("libdvi", T_("the DVI file is used by another process"));
      throw DviFileInUseException("", T_("The DVI file is used by another process."), MiKTeXException::KVMAP(), MIKTEX_SOURCE_LOCATION());
    }

    InputStream inputStream(dviFileName.GetData());

    inputStream.SetReadPosition(page.GetReadPosition(), SeekOrigin::Begin);

    pointTable.clear();

    while (DoNextCommand(inputStream, page))
    {
      ;
    }

    page.Freeze();
  }

  catch (const DviFileInUseException&)
  {
    page.Freeze();
    throw;
  }
}

bool DviImpl::DoNextCommand(InputStream & inputStream, DviPageImpl & page)
{
  MIKTEX_ASSERT(hByeByeEvent != nullptr);

  DWORD wait = WaitForSingleObject(hByeByeEvent, 0);
  if (wait == WAIT_FAILED)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("WaitForSingleObject");
  }
  if (wait == WAIT_OBJECT_0)
  {
    throw OperationCancelledException();
  }

  int opCode = inputStream.ReadByte();

  int p, q;             // parameters of the current command
  p = FirstParam(inputStream, opCode);

  if (opCode <= set4 || opCode >= put1 && opCode <= put4)
  {

    // translate a char command

    if (currentFont == nullptr)
    {
      FATAL_DVI_ERROR_2(T_("Invalid DVI file."), "fileName", dviFileName.ToString());
    }

    VFont* pVFont = dynamic_cast<VFont*>(currentFont);

    if (pVFont != nullptr)
    {
      const int maxRecursion = 20;

      if (recursion >= maxRecursion)
      {
        trace_error->WriteLine("libdvi", T_("infinite VF recursion?"));
        FATAL_DVI_ERROR_2(T_("Invalid DVI file."), "fileName", dviFileName.ToString());
      }

      VfChar* pVfChar = pVFont->GetCharAt(p);

      if (pVfChar == nullptr)
      {
        MIKTEX_UNEXPECTED();
      }

      AutoRestore<FontMap *> autoRestorFontMap(fontMap);
      AutoRestore<int> autoRestoreFontNumber(currentFontNumber);
      AutoRestore<DviFont *> autoRestoreCurrentFont(currentFont);
      AutoRestore<VfChar *> autoRestoreCurrentVfChar(currentVfChar);

      fontMap = const_cast<FontMap*>(&(pVFont->GetFontMap()));

      if (fontMap->size() > 0)
      {
        const pair<int, DviFont*> & pair = *(fontMap->begin());
        currentFontNumber = pair.first;
        currentFont = pair.second;
      }
      else
      {
        currentFontNumber = 0;
        currentFont = nullptr;
      }

      currentVfChar = pVfChar;

      PushState();

      currentState.w = 0;
      currentState.x = 0;
      currentState.y = 0;
      currentState.z = 0;

      unsigned long pl;
      const BYTE* p = pVfChar->GetPacket(pl);

      InputStream inputStream(p, pl);

      {
        AutoRestore<int> autoRestoreRecursion(recursion);
        ++recursion;
        while (!inputStream.IsEndOfStream())
        {
          DoNextCommand(inputStream, page);
        }
      }

      PopState();

      currentChar = pVfChar;

      goto fin_set;
    }
    else
    {
      currentFont->Read();
      PkFont* pPkFont = dynamic_cast<PkFont*>(currentFont);
      if (pPkFont != nullptr)
      {
        PkChar* pkChar = (*pPkFont)[p];
        currentChar = pkChar;
        if (currentChar == nullptr)
        {
          MIKTEX_UNEXPECTED();
        }
        DviItem item;
        item.x = currentState.hh + resolution;
        item.y = currentState.vv + resolution;
        item.pkChar = pkChar;
        item.rgbForeground = currentColor;
        item.rgbBackground = 0x00ffffff;
        page.AddItem(item);
        ExpandBoundingBox(item.GetLeftUns(), item.GetBottomUns(), item.GetRightUns(), item.GetTopUns());
      }
      else
      {
        Tfm* pTfm = dynamic_cast<Tfm*>(currentFont);
        if (pTfm == nullptr)
        {
          MIKTEX_UNEXPECTED();
        }
        currentChar = (*pTfm)[p];
        if (currentChar == nullptr)
        {
          MIKTEX_UNEXPECTED();
        }
        int x = currentState.hh + resolution;
        int y = currentState.vv + resolution;
        ExpandBoundingBox
          (x, y, x + currentChar->GetWidth(), y - PixelRound(currentFont->GetScaledAt()) + 1);


      }

      goto fin_set;
    }
  }
  else
  {

    // translate a non-char command

    switch (opCode)
    {

    case set_rule:

      goto fin_rule;

    case put_rule:

      goto fin_rule;

    case nop:

      return true;

    case bop:

      FATAL_DVI_ERROR_2(T_("Invalid DVI file."), "fileName", dviFileName.ToString());
      break;

    case eop:

      if (!stateStack.empty())
      {
        FATAL_DVI_ERROR_2(T_("Invalid DVI file."), "fileName", dviFileName.ToString());
      }
      if (recursion > 0)
      {
        FATAL_DVI_ERROR_2(T_("Invalid DVI file."), "fileName", dviFileName.ToString());
      }
      return false;

    case p_ush:

      PushState();
      return true;

    case p_op:

      PopState();
      return true;

    case w0:
    case FOUR_CASES(w1):

      if (recursion > 0)
      {
        p =
          static_cast<int>
          (ScaleFix(p, static_cast<int>(currentVfChar->GetScaledAt()
            / tfmConv))
            * tfmConv);
      }

      currentState.w = p;

      goto out_space;

    case x0:
    case FOUR_CASES(x1):

      if (recursion > 0)
      {
        p =
          static_cast<int>
          (ScaleFix(p, static_cast<int>(currentVfChar->GetScaledAt()
            / tfmConv))
            * tfmConv);
      }

      currentState.x = p;

      goto out_space;

    case FOUR_CASES(right1):

      if (recursion > 0)
      {
        p =
          static_cast<int>
          (ScaleFix(p, static_cast<int>(currentVfChar->GetScaledAt()
            / tfmConv))
            * tfmConv);
      }

    out_space:
      if (currentFont != nullptr
        && (p >= currentFont->GetInterWordSpacing()
          || p <= -currentFont->GetBackSpacing()))
      {
        currentState.hh = PixelRound(currentState.h + p);
      }
      else
      {
        currentState.hh += PixelRound(p);
      }
      q = p;
      goto move_right;

    default:

      SpecialCases(inputStream, opCode, p, page);
      return true;
    }
  }

fin_set:

  // finish a command that either sets or puts a character, then goto
  // move_right or return

  if (p < 0)
  {
    p = 255 - ((-1 - p) % 256);
  }
  else if (p >= 256)
  {
    p %= 256;         // width computation for oriental fonts
  }

  q = currentChar->GetDviWidth();

  if (opCode >= put1)
  {
    return true;
  }

  currentState.hh += currentChar->GetWidth();

  goto move_right;

fin_rule:

  // finish a command that either sets or puts a rule, then goto
  // move_right or return

  q = inputStream.ReadSignedQuad();
  if (p > 0 && q > 0)
  {
    if (recursion > 0)
    {
      p =
        static_cast<int>
        (ScaleFix(p, static_cast<int>(currentVfChar->GetScaledAt()
          / tfmConv))
          * tfmConv);
      q =
        static_cast<int>
        (ScaleFix(q, static_cast<int>(currentVfChar->GetScaledAt()
          / tfmConv))
          * tfmConv);
    }

    page.AddRule(new DviRuleImpl(this, currentState.hh + resolution, currentState.vv + resolution, RulePixels(q), RulePixels(p), currentColor));
  }

  if (opCode == put_rule)
  {
    return true;
  }

  currentState.hh = currentState.hh + RulePixels(q);

  goto move_right;

move_right:

  // finish a command that sets h=h+q, then return
  {
    int hhh = PixelRound(currentState.h + q); // h, rounded to the nearest pxl
    if (abs(hhh - currentState.hh) > max_drift)
    {
      if (hhh > currentState.hh)
      {
        currentState.hh = hhh - max_drift;
      }
      else
      {
        currentState.hh = hhh + max_drift;
      }
    }
    currentState.h += q;
    // MIKTEX_ASSERT (currentState.h < maxH);
  }
  return true;
}

void DviImpl::SpecialCases(InputStream & inputStream, int opCode, int p, DviPageImpl & page)
{
  if (opCode >= fnt_num && opCode <= fnt_num + 63)
  {
    goto change_font;
  }

  switch (opCode)
  {
  case y_0:
  case FOUR_CASES(y_1):
    if (recursion > 0)
    {
      p =
        static_cast<int>
        (ScaleFix(p, static_cast<int>(currentVfChar->GetScaledAt()
          / tfmConv))
          * tfmConv);
    }
    currentState.y = p;
    goto out_vmove;

  case z0:
  case FOUR_CASES(z1):
    if (recursion > 0)
    {
      p =
        static_cast<int>
        (ScaleFix(p, static_cast<int>(currentVfChar->GetScaledAt()
          / tfmConv))
          * tfmConv);
    }
    currentState.z = p;
    goto out_vmove;

  case down1: case down2: case down3: case down4:
    if (recursion > 0)
    {
      p =
        static_cast<int>
        (ScaleFix(p, static_cast<int>(currentVfChar->GetScaledAt()
          / tfmConv))
          * tfmConv);
    }
  out_vmove:
    if (currentFont != nullptr && abs(p) >= currentFont->GetLineSpacing())
    {
      currentState.vv = PixelRound(currentState.v + p);
    }
    else
    {
      currentState.vv += PixelRound(p);
    }
    goto move_down;

  case fnt1: case fnt2: case fnt3: case fnt4:
    goto change_font;

  case FOUR_CASES(fnt_def1):
    // skip a font definition
  {
    inputStream.SkipBytes(12);
    int areaNameLen = inputStream.ReadByte();
    int fontNameLen = inputStream.ReadByte();
    inputStream.SkipBytes(areaNameLen + fontNameLen);
  }
  return;

  case FOUR_CASES(xxx1):
    // translate an xxx command and return
  {
    if (p < 0)
    {
      FATAL_DVI_ERROR_2(T_("Invalid DVI file."), "fileName", dviFileName.ToString());
    }
    int x = currentState.hh + resolution;
    int y = currentState.vv + resolution;
    DviSpecial* special = nullptr;
    if (InterpretSpecial(&page, x, y, inputStream, p, special)
      && special != nullptr)
    {
      page.AddSpecial(special);
    }
  }
  return;

  case pre:
    FATAL_DVI_ERROR_2(T_("Invalid DVI file."), "fileName", dviFileName.ToString());

  case post:
  case post_post:
    FATAL_DVI_ERROR_2(T_("Invalid DVI file."), "fileName", dviFileName.ToString());

  default:
    FATAL_DVI_ERROR_2(T_("Invalid DVI file."), "fileName", dviFileName.ToString());
  }

move_down:
  // finish a command that sets v=v+p, then return
  {
    int vvv =
      PixelRound(currentState.v + p); // v, rounded to the nearest pixel
    if (abs(vvv - currentState.vv) > max_drift)
    {
      if (vvv > currentState.vv)
      {
        currentState.vv = vvv - max_drift;
      }
      else
      {
        currentState.vv = vvv + max_drift;
      }
    }
    currentState.v += p;
    // MIKTEX_ASSERT (currentState.v < maxV);
    return;
  }

change_font:
  // finish a command that changes the current font, then return
  currentFontNumber = p;
  currentFont = (*fontMap)[p];
  if (currentFont == nullptr)
  {
    FATAL_DVI_ERROR_2(T_("Invalid DVI file."), "fileName", dviFileName.ToString());
  }
}

Dvi* Dvi::Create(const char* fileName, const char* metafontMode, int resolution, int shrinkFactor, DviAccess dviAccess, IDviCallback* dviCallback, TraceCallback* traceCallback)
{
  shared_ptr<Session> session = Session::Get();
  PaperSizeInfo defaultPaperSizeInfo;
  if (!session->GetPaperSizeInfo(-1, defaultPaperSizeInfo))
  {
    MIKTEX_UNEXPECTED();
  }
  return Create(fileName, metafontMode, resolution, shrinkFactor, dviAccess, DEFAULT_PAGE_MODE, defaultPaperSizeInfo, false, dviCallback, traceCallback);
}

Dvi* Dvi::Create(const char* fileName, const char* metafontMode, int resolution, int shrinkFactor, DviAccess dviAccess, DviPageMode pageMode, const PaperSizeInfo & paperSizeInfo, bool landscape, IDviCallback* dviCallback, TraceCallback* traceCallback)
{
  DviImpl* dviImpl = new DviImpl(fileName, metafontMode, resolution, shrinkFactor, dviAccess, pageMode, paperSizeInfo, landscape, dviCallback, traceCallback);
  return dviImpl;
}

Dvi::~Dvi()
{
}

DviRuleImpl::DviRuleImpl(DviImpl* dviImpl, int x, int y, int width, int height, unsigned long rgb) :
  dviImpl(dviImpl),
  x(x),
  y(y),
  width(width),
  height(height),
  rgb(rgb)
{
}

bool DviRuleImpl::IsBlackboard()
{
  return GetFlag(flblackboard);
}

int DviRuleImpl::GetLeft(int shrinkFactor)
{
  return dviImpl->WidthShrink(shrinkFactor, x);
}

int DviRuleImpl::GetRight(int shrinkFactor)
{
  return GetLeft(shrinkFactor) + dviImpl->WidthShrink(shrinkFactor, width) - 1;
}

int DviRuleImpl::GetTop(int shrinkFactor)
{
  return dviImpl->WidthShrink(shrinkFactor, y) - dviImpl->WidthShrink(shrinkFactor, height) + 1;
}

int DviRuleImpl::GetBottom(int shrinkFactor)
{
  return GetTop(shrinkFactor) + dviImpl->WidthShrink(shrinkFactor, height) - 1;
}

unsigned long DviRuleImpl::GetBackgroundColor()
{
  return rgb;
}

unsigned long DviRuleImpl::GetForegroundColor()
{
  return rgb;
}

int DviImpl::GetNumberOfPages()
{
  CheckCondition();
  BEGIN_CRITICAL_SECTION(dviMutex)
  {
    return static_cast<int>(pages.size());
  }
  END_CRITICAL_SECTION();
}

DviPage* DviImpl::GetPage(int pageIdx)
{
  CheckCondition();
  BEGIN_CRITICAL_SECTION(dviMutex)
  {
    if (pageIdx >= GetNumberOfPages())
    {
      return nullptr;
    }
    DviPageImpl* dviPage = pages[pageIdx];
    dviPage->Lock();
    try
    {
      if ((!pageLoaderThread.joinable() || this_thread::get_id() != pageLoaderThread.get_id())
        && (!garbageCollectorThread.joinable() || this_thread::get_id() != garbageCollectorThread.get_id())
        && currentPageIdx != pageIdx)
      {
        trace_dvifile->WriteLine("libdvi", fmt::format(T_("getting page #{0}"), pageIdx));
        if (pageIdx < currentPageIdx)
        {
          direction = -1;
        }
        else if (pageIdx > currentPageIdx)
        {
          direction = 1;
        }
        currentPageIdx = pageIdx;
        if (!SetEvent(hNewPageEvent))
        {
          MIKTEX_FATAL_WINDOWS_ERROR("SetEvent");
        }
      }
      return dviPage;
    }
    catch (const exception&)
    {
      try
      {
        dviPage->Unlock();
      }
      catch (const exception&)
      {
      }
      throw;
    }
  }
  END_CRITICAL_SECTION();
}

int DviImpl::GetMinPageNumber()
{
  CheckCondition();
  return minPageNumber;
}

int DviImpl::GetMaxPageNumber()
{
  CheckCondition();
  return maxPageNumber;
}

int DviImpl::GetMagnification()
{
  CheckCondition();
  return mag;
}

int DviImpl::GetMaxH()
{
  CheckCondition();
  return maxH;
}

int DviImpl::GetMaxV()
{
  CheckCondition();
  return maxV;
}

DviPage* DviImpl::GetLoadedPage(int pageIdx)
{
  CheckCondition();
  BEGIN_CRITICAL_SECTION(dviMutex)
  {
    switch (GetPageStatus(pageIdx))
    {
    case PageStatus::Lost:
    case PageStatus::Unknown:
      return 0;
    case PageStatus::Changed:
      Scan();              // fall through
    case PageStatus::NotLoaded:
      DoPage(pageIdx);     // fall through
    case PageStatus::Loaded:
    {
      DviPageImpl* dviPage =
        reinterpret_cast<DviPageImpl*>(GetPage(pageIdx));
      MIKTEX_ASSERT(dviPage != nullptr);
      dviPage->SetAutoClean(dviAccess == DviAccess::Sequential);
      return dviPage;
    }
    default:
      MIKTEX_ASSERT(false);
      __assume (false);
    }
  }
  END_CRITICAL_SECTION();
}

void DviImpl::Progress(DviNotification nf, const string& msg)
{
  if ((pageLoaderThread.joinable() && this_thread::get_id() == pageLoaderThread.get_id())
    || (garbageCollectorThread.joinable() && this_thread::get_id() == garbageCollectorThread.get_id()))
  {
    return;
  }

  {
    lock_guard<mutex> lockGuard(statusTextMutex);
    progressStatus = msg;
  }

  if (dviCallback != nullptr)
  {
    dviCallback->OnProgress(nf);
  }
}

string DviImpl::GetStatusText()
{
  CheckCondition();
  lock_guard<mutex> lockGuard(statusTextMutex);
  return progressStatus;
}

const unsigned long sleepDurationLowestPrio = 1000; // milliseconds
const unsigned long sleepDurationBelowNormalPrio = 100;
const unsigned long sleepDurationNormalPrio = 10;
const unsigned long sleepDurationAboveNormalPrio = 0;
const unsigned long sleepDurationHighestPrio = 0;

const unsigned long limitBelowNormalPrio = 2 * 1024 * 1024;
const unsigned long limitNormalPrio = 20 * 1024 * 1024;
const unsigned long limitAboveNormalPrio = 50 * 1024 * 1024;
const unsigned long limitHighestPrio = 100 * 1024 * 1024;

void DviImpl::PageLoader()
{
  try
  {
#if 1
    if (SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST) == 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("SetThreadPriority");
    }
#endif

    MIKTEX_ASSERT(hByeByeEvent != nullptr);
    MIKTEX_ASSERT(hNewPageEvent != nullptr);
    MIKTEX_ASSERT(hScannedEvent != nullptr);

    HANDLE handles[2];

    handles[0] = hByeByeEvent;
    handles[1] = hScannedEvent;

    unsigned long wait = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

    if (wait == WAIT_FAILED)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("WaitForMultipleObjects");
    }

    if (wait == WAIT_OBJECT_0)
    {
      return;
    }

    for (int pageIdx = currentPageIdx + 1; (wait = WaitForSingleObject(hByeByeEvent, sleepDurationBelowNormalPrio)) != WAIT_OBJECT_0; pageIdx += direction)
    {
      if (wait == WAIT_FAILED)
      {
        MIKTEX_FATAL_WINDOWS_ERROR("WaitForSingleObject");
      }
      DviPage* dviPage = nullptr;
      AutoUnlockPage autoUnlockPage(nullptr);
      BEGIN_CRITICAL_SECTION(dviMutex)
      {
        wait = WaitForSingleObject(hNewPageEvent, 0);
        if (wait == WAIT_FAILED)
        {
          MIKTEX_FATAL_WINDOWS_ERROR("WaitForSingleObject");
        }
        if (wait == WAIT_OBJECT_0)
        {
          pageIdx = currentPageIdx;
        }
        if (pageIdx < 0 || pageIdx >= GetNumberOfPages())
        {
          pageIdx = currentPageIdx;
          continue;
        }
        if (direction > 0)
        {
          if (!(pageIdx >= currentPageIdx && pageIdx < currentPageIdx + 10))
          {
            continue;
          }
        }
        else
        {
          MIKTEX_ASSERT(direction < 0);
          if (!(pageIdx <= currentPageIdx && pageIdx > currentPageIdx - 10))
          {
            continue;
          }
        }
        DviPageImpl* pPageImpl = pages[pageIdx];
        if (pPageImpl->IsLocked())
        {
          continue;
        }
        dviPage = GetLoadedPage(pageIdx);
        autoUnlockPage.Attach(dviPage);
      }
      END_CRITICAL_SECTION();
      if (dviPage != nullptr)
      {
        dviPage->GetNumberOfDviBitmaps(defaultShrinkFactor);
      }
    }
  }

  catch (const MiKTeXException & e)
  {
    miktexException = e;
    fatalMiKTeXError = true;
  }

  catch (const exception & e)
  {
    stdException = e;
    fatalError = true;
  }
}

const time_t timeKeepBitmapsLowestPrio = 120; // seconds
const time_t timeKeepBitmapsBelowNormalPrio = 60;
const time_t timeKeepBitmapsNormalPrio = 30;
const time_t timeKeepBitmapsAboveNormalPrio = 10;
const time_t timeKeepBitmapsHighestPrio = 5;

void DviImpl::GarbageCollector()
{
  try
  {
    int priority = THREAD_PRIORITY_LOWEST;
    unsigned long sleepDuration = sleepDurationLowestPrio;
    time_t timeKeepBitmaps = timeKeepBitmapsLowestPrio;
    if (!SetThreadPriority(GetCurrentThread(), priority))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("SetThreadPriority");
    }
    DWORD wait;
    while ((wait = WaitForSingleObject(hByeByeEvent, sleepDuration)) != WAIT_OBJECT_0)
    {
      if (wait == WAIT_FAILED)
      {
        MIKTEX_FATAL_WINDOWS_ERROR("WaitForSingleObject");
      }
      size_t sizeBiggest = 0;
      int biggestPageIdx = -1;
      DviPageImpl* dviPage;
      size_t totalSize = 0;
      time_t now = time(nullptr);
      for (int pageIdx = 0; (wait = WaitForSingleObject(hByeByeEvent, 0)) != WAIT_OBJECT_0; ++pageIdx)
      {
        if (wait == WAIT_FAILED)
        {
          MIKTEX_FATAL_WINDOWS_ERROR("WaitForSingleObject");
        }
        BEGIN_CRITICAL_SECTION(dviMutex)
        {
          if (pageIdx >= GetNumberOfPages())
          {
            break;
          }
          if (direction > 0)
          {
            if (pageIdx >= currentPageIdx && pageIdx < currentPageIdx + 10)
            {
              continue;
            }
          }
          else
          {
            MIKTEX_ASSERT(direction < 0);
            if (pageIdx <= currentPageIdx && pageIdx > currentPageIdx - 10)
            {
              continue;
            }
          }
          DviPageImpl* dviPage = pages[pageIdx];
          totalSize += dviPage->GetSize();
          if (!dviPage->IsFrozen())
          {
            continue;
          }
          time_t timeLastVisit = dviPage->GetTimeLastVisit();
          if (timeLastVisit + timeKeepBitmaps < now)
          {
            if (dviPage->GetSize() > sizeBiggest && !dviPage->IsLocked())
            {
              sizeBiggest = dviPage->GetSize();
              biggestPageIdx = pageIdx;
            }
          }
        }
        END_CRITICAL_SECTION();
      }
      wait = WaitForSingleObject(hByeByeEvent, 0);
      if (wait == WAIT_FAILED)
      {
        MIKTEX_FATAL_WINDOWS_ERROR("WaitForSingleObject");
      }
      if (wait == WAIT_OBJECT_0)
      {
        break;
      }
      int newPriority;
      if (totalSize > limitHighestPrio)
      {
#if 0
        newPriority = THREAD_PRIORITY_HIGHEST;
#else
        newPriority = THREAD_PRIORITY_NORMAL;
#endif
        sleepDuration = sleepDurationHighestPrio;
        timeKeepBitmaps = timeKeepBitmapsHighestPrio;
      }
      else if (totalSize > limitAboveNormalPrio)
      {
#if 0
        newPriority = THREAD_PRIORITY_ABOVE_NORMAL;
#else
        newPriority = THREAD_PRIORITY_NORMAL;
#endif
        sleepDuration = sleepDurationAboveNormalPrio;
        timeKeepBitmaps = timeKeepBitmapsAboveNormalPrio;
      }
      else if (totalSize > limitNormalPrio)
      {
        newPriority = THREAD_PRIORITY_NORMAL;
        sleepDuration = sleepDurationBelowNormalPrio;
        timeKeepBitmaps = timeKeepBitmapsNormalPrio;
      }
      else if (totalSize > limitBelowNormalPrio)
      {
        newPriority = THREAD_PRIORITY_BELOW_NORMAL;
        sleepDuration = sleepDurationBelowNormalPrio;
        timeKeepBitmaps = timeKeepBitmapsBelowNormalPrio;
      }
      else
      {
        newPriority = THREAD_PRIORITY_LOWEST;
        sleepDuration = sleepDurationLowestPrio;
        timeKeepBitmaps = timeKeepBitmapsLowestPrio;
      }
      if (newPriority != priority)
      {
        priority = newPriority;
        if (!SetThreadPriority(GetCurrentThread(), priority))
        {
          MIKTEX_FATAL_WINDOWS_ERROR("SetThreadPriority");
        }
        trace_gc->WriteLine("libdvi", fmt::format(T_("gc priority: {0}"), priority));
      }
      if (biggestPageIdx < 0)
      {
        continue;
      }
      BEGIN_CRITICAL_SECTION(dviMutex)
      {
        if (biggestPageIdx >= GetNumberOfPages())
        {
          continue;
        }
        dviPage = pages[biggestPageIdx];
        if (dviPage->IsLocked())
        {
          continue;
        }
        dviPage->Lock();
        AutoUnlockPage autoUnlockPage(dviPage);
        trace_gc->WriteLine("libdvi", fmt::format(T_("freeing page #{0} ({1} bytes)"), biggestPageIdx, dviPage->GetSize()));
        dviPage->FreeContents(false, false);
      }
      END_CRITICAL_SECTION();
    }
  }

  catch (const MiKTeXException & e)
  {
    miktexException = e;
    fatalMiKTeXError = true;
  }

  catch (const exception & e)
  {
    stdException = e;
    fatalError = true;
  }
}

bool DviImpl::MakeFonts(const FontMap & fontMap, int recursion)
{
  bool done = true;
  for (FontMap::const_iterator it = fontMap.begin(); it != fontMap.end(); ++it)
  {
    if (it->second->IsNotLoadable())
    {
      done = false;
      continue;
    }
    it->second->Read();
    if (it->second->IsNotLoadable())
    {
      done = false;
      continue;
    }
    VFont* pVFont = dynamic_cast<VFont*>(it->second);
    if (pVFont == nullptr)
    {
      continue;
    }
    const int maxRecursion = 20;
    if (recursion >= maxRecursion)
    {
      trace_error->WriteLine("libdvi", T_("infinite VF recursion?"));
    }
    done = done && MakeFonts(pVFont->GetFontMap(), recursion + 1);
  }
  return done;
}

bool DviImpl::MakeFonts()
{
  CheckCondition();
  BEGIN_CRITICAL_SECTION(dviMutex)
  {
    if (fontMap == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    return MakeFonts(*fontMap, 0);
  }
  END_CRITICAL_SECTION();
}

void DviImpl::GetFontTable(const FontMap & fontMap, vector<DviFontInfo> & vec, int recursion)
{
  for (FontMap::const_iterator it = fontMap.begin(); it != fontMap.end(); ++it)
  {
    DviFontInfo info;
    it->second->GetInfo(info);
    vec.push_back(info);
    VFont* pVFont = dynamic_cast<VFont*>(it->second);
    if (pVFont == nullptr)
    {
      continue;
    }
    const int maxRecursion = 20;
    if (recursion >= maxRecursion)
    {
      trace_error->WriteLine("libdvi", T_("infinite VF recursion?"));
    }
    GetFontTable(pVFont->GetFontMap(), vec, recursion + 1);
  }
}

vector<DviFontInfo> DviImpl::GetFontTable()
{
  CheckCondition();
  BEGIN_CRITICAL_SECTION(dviMutex)
  {
    vector<DviFontInfo> ret;
    MIKTEX_ASSERT(fontMap != nullptr);
    if (fontMap != nullptr)
    {
      GetFontTable(*fontMap, ret, 0);
    }
    return ret;
  }
  END_CRITICAL_SECTION();
}

bool DviImpl::FindGraphicsFile(const char* fileName, PathName & result)
{
  if (PathNameUtil::IsAbsolutePath(fileName))
  {
    result = fileName;
    return File::Exists(result);
  }
  MIKTEX_ASSERT(dviFileName.GetLength() > 0);
  result = dviFileName;
  result.RemoveFileSpec();
  result /= fileName;
  if (File::Exists(result))
  {
    return true;
  }
  if (!session->FindFile(fileName, FileType::GRAPHICS, result))
  {
    return false;
  }
  return File::Exists(result);
}

void DviImpl::CheckCondition()
{
  if (fatalError)
  {
    fatalError = false;
    throw stdException;
  }
  if (fatalMiKTeXError)
  {
    fatalMiKTeXError = false;
    throw miktexException;
  }
}

void DviImpl::Lock()
{
  dviMutex.lock();
}

void DviImpl::Unlock()
{
  dviMutex.unlock();
}

