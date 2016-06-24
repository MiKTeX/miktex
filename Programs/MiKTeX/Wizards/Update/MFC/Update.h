/* Update.h:                                            -*- C++ -*-

   Copyright (C) 2002-2016 Christian Schenk

   This file is part of the MiKTeX Update Wizard.

   The MiKTeX Update Wizard is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Update Wizard is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Update Wizard; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#include "resource.h"

class UpdateWizardCommandLineInfo;

#define REMOVE_FORMAT_FILES 1

#define T_(x) MIKTEXTEXT(x)

#if !(defined(_UNICODE) && defined(UNICODE))
#  error UNICODE compilation required
#endif

#define TU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#define UT_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()

#define WU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()

template<typename T> class AutoToggle
{
public:
  AutoToggle(T & val, T newVal)
    : val(val),
    oldVal(val)
  {
    val = newVal;
  }
public:
  ~AutoToggle()
  {
    val = oldVal;
  }
private:
  T & val;
  T oldVal;
};

#define AUTO_TOGGLE(var) AutoToggle<bool> autoToggle (var, !var)

class UpdateWizardApplication :
  public CWinApp,
  public TraceCallback
{
public:
  UpdateWizardApplication();

public:
  virtual BOOL InitInstance();

private:
  bool Upgrade(bool & upgrading);

private:
  bool isLog4cxxConfigured = false;

private:
  vector<TraceCallback::TraceMessage> pendingTraceMessages;

private:
  void FlushPendingTraceMessages();

private:
  void TraceInternal(const TraceCallback::TraceMessage & traceMessage);

public:
  virtual void MIKTEXTHISCALL Trace(const TraceCallback::TraceMessage & traceMessage);

public:
  DECLARE_MESSAGE_MAP();

public:
  static std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;

public:
  static bool upgrading;
};

void ReportError(const MiKTeXException & e);

void ReportError(const exception & e);

void SplitUrl(const string & url, string & protocol, string & host);

inline bool IsMiKTeXPackage(const string & deploymentName)
{
  return deploymentName.compare(0, 7, "miktex-") == 0;
}
