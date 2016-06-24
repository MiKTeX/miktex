/* MainFrame.h:                                         -*- C++ -*-

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

#define YAP_WND_CLASS _T("MiKTeX_yap")

#include "DviStatusBar.h"

class MainFrame :
  public CMDIFrameWnd
{
private:
  DECLARE_DYNAMIC(MainFrame);

protected:
  DECLARE_MESSAGE_MAP();

public:
  MainFrame()
  {
  }

public:
  virtual BOOL PreCreateWindow(CREATESTRUCT & cs);

protected:
  afx_msg int OnCreate(LPCREATESTRUCT pCreateStruct);

protected:
  afx_msg void OnCancel();

protected:
  afx_msg void OnClose();

protected:
  afx_msg void OnContinuousDouble();

protected:
  afx_msg void OnContinuousSingle();

protected:
  afx_msg void OnDoublePage();

protected:
  afx_msg void OnGetMinMaxInfo(MINMAXINFO * pMMI);

protected:
  afx_msg void OnHelpKeyboard();

protected:
  afx_msg void OnSinglePage();

protected:
  afx_msg void OnUpdateCommandPrefix(CCmdUI * pCmdUI);

protected:
  afx_msg void OnUpdateContinuousDouble(CCmdUI * pCmdUI);

protected:
  afx_msg void OnUpdateContinuousSingle(CCmdUI * pCmdUI);

protected:
  afx_msg void OnUpdateDoublePage(CCmdUI * pCmdUI);

protected:
  afx_msg void OnUpdatePageMofN(CCmdUI * pCmdUI);

protected:
  afx_msg void OnUpdatePoint(CCmdUI * pCmdUI);

protected:
  afx_msg void OnUpdateEffectivePageMode(CCmdUI * pCmdUI);

protected:
  afx_msg void OnUpdateSinglePage(CCmdUI * pCmdUI);

protected:
  afx_msg void OnUpdateSource(CCmdUI * pCmdUI);

protected:
  afx_msg void OnUpdateViewFullScreen(CCmdUI * pCmdUI);

protected:
  afx_msg void OnViewFullScreen();

protected:
  afx_msg void OnViewOptions();

protected:
  afx_msg void OnViewCentimeters();

protected:
  afx_msg void OnUpdateViewCentimeters(CCmdUI *pCmdUI);

protected:
  afx_msg void OnViewInches();

protected:
  afx_msg void OnUpdateViewInches(CCmdUI *pCmdUI);

protected:
  afx_msg void OnViewMillimeters();

protected:
  afx_msg void OnUpdateViewMillimeters(CCmdUI *pCmdUI);

protected:
  afx_msg void OnViewPicas();

protected:
  afx_msg void OnUpdateViewPicas(CCmdUI *pCmdUI);

protected:
  afx_msg void OnViewPoints();

protected:
  afx_msg void OnUpdateViewPoints(CCmdUI *pCmdUI);

public:
  CStatusBar & GetStatusBar()
  {
    return statusBar;
  }

public:
  bool AddCommandPrefixChar(char ch);

public:
  string GetCommandPrefix(bool clear = false);

public:
  bool IsFullScreen() const
  {
    return isFullScreen;
  }

protected:
  DviStatusBar statusBar;

protected:
  CToolBar toolBar;

private:
  string commandPrefix;

private:
  bool isFullScreen = false;

private:
  CRect fullScreenWindowRect;

private:
  WINDOWPLACEMENT previousWindowPlacement;

private:
  CToolBar * pFullScreenBar = nullptr;
};
