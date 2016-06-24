/* source.cpp: source specials

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

#include "StdAfx.h"

#include "yap.h"

#include "Dvidoc.h"
#include "DviView.h"
#include "SourceSpecialsDialog.h"

void DviView::CloseSourceSpecialsDialog()
{
  pSourceSpecialDialog = nullptr;
}

bool DviView::GotoSrcSpecial(int line, const char * lpszFileName)
{
  ClearSearchPosition();

  DviDoc * pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  DviPosition position;

  if (!pDoc->FindSrcSpecial(lpszFileName, line, position))
  {
    return false;
  }

  searchPosition.x = position.x;
  searchPosition.y = position.y;

  pageSearchPosition = position.pageIdx;

  if (curPageIdx != position.pageIdx)
  {
    ChangePage(position.pageIdx);
  }

  position.x = PixelShrink(position.x);
  position.x += GetPagePositionX(position.pageIdx);

  position.y = PixelShrink(position.y);
  position.y += GetPagePositionY(position.pageIdx);

  if (!IsPointInView(CPoint(position.x, position.y)))
  {
    CenterOnPoint(CPoint(position.x, position.y));
  }

  Invalidate();

  return true;
}

void DviView::OnPageEditor()
{
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    PathName fileName;
    int line;
    if (GetSource(fileName, line))
    {
      StartEditor(fileName.Get(), pDoc->GetDocDir().Get(), line);
    }
    else
    {
      MIKTEX_FATAL_ERROR(T_("The source file could not be opened because the page contains no source links."));
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void DviView::OnUpdatePageEditor(CCmdUI * pCmdUI)
{
  BOOL enable = FALSE;
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (pDoc->GetDviFileStatus() == DviDoc::DVIFILE_LOADED)
    {
      enable = TRUE;
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  pCmdUI->Enable(enable);
}

void DviView::OnToolsSourcespecials()
{
  try
  {
    if (pSourceSpecialDialog == nullptr)
    {
      pSourceSpecialDialog = new SourceSpecialsDialog(this, GetDocument());
      pSourceSpecialDialog->Create(IDD_SOURCE_SPECIALS, this);
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void DviView::OnUpdateToolsSourcespecials(CCmdUI * pCmdUI)
{
  BOOL enable = FALSE;
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (pDoc->GetDviFileStatus() == DviDoc::DVIFILE_LOADED)
    {
      enable = TRUE;
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  pCmdUI->Enable(enable);
}

bool DviView::GetSource(PathName & fileName, int & line)
{
  int x, y;
  int pageIdx;
  if (!GetPageUnderCursor(pageIdx, x, y))
  {
    return false;
  }
  MIKTEX_ASSERT(pageIdx >= 0);
  DviDoc * pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  if (pDoc->GetPageStatus(pageIdx) != PageStatus::Loaded)
  {
    return false;
  }
  return pDoc->GetSource(DviPosition(pageIdx, x, y), fileName, &line);
}

bool DviDoc::GetSource(const DviPosition & position, PathName & fileName, int * pLine)
{
  return pDvi->GetSource(position, fileName, pLine);
}

bool DviDoc::FindSrcSpecial(const char * lpszFileName, int line, DviPosition & position)
{
  return pDvi->FindSource(lpszFileName, line, position);
}
