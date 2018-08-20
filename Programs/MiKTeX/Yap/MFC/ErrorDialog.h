/* ErrorDialog.h:                                       -*- C++ -*-

   Copyright (C) 2000-2018 Christian Schenk

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

class MIKTEXNOVTABLE ErrorDialog
{
public:
  ErrorDialog() = delete;

public:
  ErrorDialog(const ErrorDialog& other) = delete;

public:
  ErrorDialog& operator=(const ErrorDialog& other) = delete;

public:
  ErrorDialog(ErrorDialog&& other) = delete;

public:
  ErrorDialog& operator=(ErrorDialog&& other) = delete;

public:
  ~ErrorDialog() = delete;

public:
  static INT_PTR DoModal(CWnd* parent, const MiKTeX::Core::MiKTeXException& e);

public:
  static INT_PTR DoModal(CWnd* parent, const std::exception& e);
};
