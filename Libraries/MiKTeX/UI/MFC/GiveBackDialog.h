/* GiveBackDialog.h:                                    -*- C++ -*-

   Copyright (C) 2011-2016 Christian Schenk

   This file is part of the MiKTeX UI Library.

   The MiKTeX UI Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX UI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX UI Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "resource.h"

class GiveBackDialog : public CDialog
{
private:
  enum { IDD = IDD_GIVE_BACK };

private:
  DECLARE_DYNAMIC(GiveBackDialog);

protected:
  DECLARE_MESSAGE_MAP();

public:
  GiveBackDialog(CWnd * pParent = nullptr);

protected:
  void DoDataExchange(CDataExchange * pDX) override;

protected:
  BOOL OnInitDialog() override;

protected:
  afx_msg void OnClickUrl(NMHDR * pNMHDR, LRESULT * pResult);
};
