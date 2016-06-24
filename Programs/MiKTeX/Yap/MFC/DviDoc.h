/* DviDoc.h:                                            -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of Yap.

   Yap is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Yap is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Yap; if not, write to the Free Software Foundation, 59
   Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#pragma once

class DviDoc :
  public CDocument,
  public IDviCallback
{
protected:
  DECLARE_DYNCREATE(DviDoc);

protected:
  DviDoc();

public:
  virtual ~DviDoc();

public:
  virtual void MIKTEXTHISCALL OnProgress(DviNotification nf);

public:
  int GetPageCount() const;

public:
  int GetRowCount() const;

public:
  int DeterminePageIdx(int rowIdx, bool onRight) const;

public:
  int GetRowIdx(int pageIdx) const;

public:
  bool IsOnLeft(int pageIdx) const;

public:
  bool IsOnRight(int pageIdx) const;

public:
  bool IsFlipSide(int pageIdx) const;

public:
  int GetMagnification() const;

public:
  int GetDisplayShrinkFactor() const;

public:
  DviPageMode GetDviPageMode() const
  {
    return dviPageMode;
  }

public:
  DviPageMode SetDviPageMode(const DviPageMode & dviPageMode);

public:
  PageStatus GetPageStatus(int pageIdx);

public:
  bool IsPrintContext() const;

public:
  int FindPage(int iPageNum) const;

public:
  void BeginDviPrinting(const CDC * pPrinterDC);

public:
  void EndDviPrinting();

public:
  bool FindHyperLabel(const char * lpszFileName, DviPosition & position);

public:
  bool FindSrcSpecial(const char * lpszFileName, int line, DviPosition & position);

public:
  int GetDisplayResolution() const;

public:
  PathName GetDocDir();

public:
  DviPage * GetLoadedPage(int pageIdx);

public:
  int GetMaxPageNum() const;

public:
  unsigned long GetMetafontMode() const;

public:
  int GetMinPageNum() const;

public:
  const char * GetPageName(unsigned pageIdx);

public:
  int GetPageNum(int pageIdx) const;

public:
  CSize GetPaperSize() const;

public:
  PaperSizeInfo GetPaperSizeInfo() const
  {
    MIKTEX_ASSERT(pDvi != nullptr);
    return pDvi->GetPaperSizeInfo();
  }

public:
  int GetPrinterResolution() const;

public:
  bool GetSource(const DviPosition & position, PathName & fileName, int * pLineNum);

public:
  bool IsPortrait() const;

public:
  void Reread();

private:
  void CreateDocument(const char * lpszPathName);

public:
  void MakeFonts();

public:
  int SetDisplayShrinkFactor(int shrinkFactor);

public:
  void Shrink(int d = 1);

public:
  void Lock()
  {
    MIKTEX_ASSERT(pDvi != nullptr);
    pDvi->Lock();
  }

public:
  void Unlock()
  {
    MIKTEX_ASSERT(pDvi != nullptr);
    pDvi->Unlock();
  }

public:
  void Unshrink();

public:
  static DviDoc * GetActiveDocument();

public:
  virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

protected:
  virtual void OnIdle();

public:
  int GetShrinkFactor() const;

public:
  int GetResolution() const;

private:
  int pt2pxl(int pt) const;

public:
  vector<DviFontInfo>
    GetFontTable();

public:
  enum DviFileStatus {
    DVIFILE_NOT_LOADED,
    DVIFILE_LOADED,
    DVIFILE_MODIFIED,
    DVIFILE_LOST
  };

public:
  DviFileStatus GetDviFileStatus();

private:
  bool isPrintContext = false;

private:
  Dvi * pDvi = nullptr;

private:
  Dvi * pDviSave = nullptr;

private:
  int displayShrinkFactor;

private:
  DviPageMode dviPageMode;

private:
  string dvipsPaperName;

private:
  bool landscape = false;

private:
  time_t modificationTime;

private:
  DviFileStatus fileStatus = DVIFILE_NOT_LOADED;

private:
  bool makingFonts = false;

private:
  ProgressDialog * pProgressDialog = nullptr;

private:
  shared_ptr<Session> pSession = Session::Get();

protected:
  DECLARE_MESSAGE_MAP();
};

inline int DviDoc::GetPageCount() const
{
  if (fileStatus == DVIFILE_NOT_LOADED || fileStatus == DVIFILE_LOST)
  {
    return 0;
  }
  MIKTEX_ASSERT(pDvi != nullptr);
  return pDvi->GetNumberOfPages();
}

inline int DviDoc::GetRowCount() const
{
  return (GetPageCount() + 2) / 2;
}

inline int DviDoc::DeterminePageIdx(int rowIdx, bool onRight) const
{
  MIKTEX_ASSERT(pDvi != nullptr);
  MIKTEX_ASSERT(rowIdx >= 0);
  MIKTEX_ASSERT(rowIdx < GetRowCount());
  return onRight && 2 * rowIdx < GetPageCount() ? 2 * rowIdx : (rowIdx == 0 ? 0 : 2 * rowIdx - 1);
}

inline int DviDoc::GetRowIdx(int pageIdx) const
{
  MIKTEX_ASSERT(pDvi != nullptr);
  MIKTEX_ASSERT(pageIdx >= 0);
  MIKTEX_ASSERT(pageIdx < GetPageCount());
  return (pageIdx + 1) / 2;
}

inline bool DviDoc::IsOnLeft(int pageIdx) const
{
  return IsFlipSide(pageIdx);
}

inline bool DviDoc::IsOnRight(int pageIdx) const
{
  return !IsOnLeft(pageIdx);
}

inline bool DviDoc::IsFlipSide(int pageIdx) const
{
  return pageIdx % 2 != 0;
}

inline vector<DviFontInfo> DviDoc::GetFontTable()
{
  MIKTEX_ASSERT(pDvi != nullptr);
  return pDvi->GetFontTable();
}

inline int DviDoc::GetShrinkFactor() const
{
  return isPrintContext ? 1 : GetDisplayShrinkFactor();
}

inline int DviDoc::GetResolution() const
{
  return isPrintContext ? GetPrinterResolution() : GetDisplayResolution();
}

inline int DviDoc::pt2pxl(int pt) const
{
  MIKTEX_ASSERT(GetShrinkFactor() != 0);
  return (static_cast<int>(((GetResolution() * pt) / 72.0) / GetShrinkFactor()));
}

inline bool DviDoc::IsPrintContext() const
{
  return isPrintContext;
}

inline int DviDoc::GetDisplayShrinkFactor() const
{
  return displayShrinkFactor;
}

inline PageStatus DviDoc::GetPageStatus(int pageIdx)
{
  MIKTEX_ASSERT(pDvi != nullptr);
  MIKTEX_ASSERT(pageIdx >= 0 && pageIdx < GetPageCount());
  return pDvi->GetPageStatus(pageIdx);
}

inline int DviDoc::GetMagnification() const
{
  MIKTEX_ASSERT(pDvi != nullptr);
  return pDvi->GetMagnification();
}

class UnlockDviDoc_
{
public:
  void operator() (DviDoc * p)
  {
    p->Unlock();
  }
};

typedef MiKTeX::Core::AutoResource<DviDoc *, UnlockDviDoc_> AutoUnlockDviDoc;

class SetDisplayShrinkFactor_
{
public:
  void operator() (DviDoc * pDviDoc, int shrinkFactor)
  {
    pDviDoc->SetDisplayShrinkFactor(shrinkFactor);
  }
};

typedef AutoResource2<DviDoc *, int, SetDisplayShrinkFactor_> AutoRestoreDisplayShrinkFactor;
