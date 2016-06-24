/* miktex/Core/win/DllProc.h:                           -*- C++ -*-

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

#pragma once

#include <miktex/Core/config.h>

#include <Windows.h>

#include <string>

MIKTEX_CORE_BEGIN_NAMESPACE;

class DllProcBase
{
public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL DllProcBase();

public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL DllProcBase(const char * lpszDllName, const char * lpszProcName);

protected:
  MIKTEXCOREEXPORT virtual MIKTEXTHISCALL ~DllProcBase();

protected:
  MIKTEXCOREEXPORT FARPROC MIKTEXTHISCALL GetProc();

private:
  std::string dllName;

private:
  std::string procName;

private:
  HMODULE hModule = nullptr;

private:
  FARPROC proc = nullptr;
};

template<typename RetType_> class DllProc0 : public DllProcBase
{
public:
  DllProc0(const char * lpszDllName, const char * lpszProcName) :
    DllProcBase(lpszDllName, lpszProcName)
  {
  }

protected:
  typedef RetType_(WINAPI * ProcType) ();

public:
  RetType_ operator() ()
  {
    return reinterpret_cast<ProcType>(GetProc())();
  }
};

template<typename RetType_, typename Param1Type_> class DllProc1 : public DllProcBase
{
public:
  DllProc1(const char * lpszDllName, const char * lpszProcName) :
    DllProcBase(lpszDllName, lpszProcName)
  {
  }

protected:
  typedef RetType_(WINAPI * ProcType) (Param1Type_);

public:
  RetType_ operator() (Param1Type_ param1)
  {
    return reinterpret_cast<ProcType>(GetProc())(param1);
  }
};

template<typename RetType_, typename Param1Type_, typename Param2Type_> class DllProc2 : public DllProcBase
{
public:
  DllProc2(const char * lpszDllName, const char * lpszProcName) :
    DllProcBase(lpszDllName, lpszProcName)
  {
  }

protected:
  typedef RetType_(WINAPI * ProcType) (Param1Type_, Param2Type_);

public:
  RetType_ operator() (Param1Type_ param1, Param2Type_ param2)
  {
    return reinterpret_cast<ProcType>(GetProc())(param1, param2);
  }
};

template<typename RetType_, typename Param1Type_, typename Param2Type_, typename Param3Type_> class DllProc3 : public DllProcBase
{
public:
  DllProc3(const char * lpszDllName, const char * lpszProcName) :
    DllProcBase(lpszDllName, lpszProcName)
  {
  }

protected:
  typedef RetType_(WINAPI * ProcType) (Param1Type_, Param2Type_, Param3Type_);

public:
  RetType_ operator() (Param1Type_ param1, Param2Type_ param2, Param3Type_ param3)
  {
    return reinterpret_cast<ProcType>(GetProc())(param1, param2, param3);
  }
};

template<typename RetType_, typename Param1Type_, typename Param2Type_, typename Param3Type_, typename Param4Type_> class DllProc4 : public DllProcBase
{
public:
  DllProc4(const char * lpszDllName, const char * lpszProcName) :
    DllProcBase(lpszDllName, lpszProcName)
  {
  }

protected:
  typedef RetType_(WINAPI * ProcType) (Param1Type_, Param2Type_, Param3Type_, Param4Type_);

public:
  RetType_ operator() (Param1Type_ param1, Param2Type_ param2, Param3Type_ param3, Param4Type_ param4)
  {
    return reinterpret_cast<ProcType>(GetProc())(param1, param2, param3, param4);
  }
};

template<typename RetType_, typename Param1Type_, typename Param2Type_, typename Param3Type_, typename Param4Type_, typename Param5Type_> class DllProc5 : public DllProcBase
{
public:
  DllProc5(const char * lpszDllName, const char * lpszProcName) :
    DllProcBase(lpszDllName, lpszProcName)
  {
  }

protected:
  typedef RetType_(WINAPI * ProcType) (Param1Type_, Param2Type_, Param3Type_, Param4Type_, Param5Type_);

public:
  RetType_ operator() (Param1Type_ param1, Param2Type_ param2, Param3Type_ param3, Param4Type_ param4, Param5Type_ param5)
  {
    return reinterpret_cast<ProcType>(GetProc())(param1, param2, param3, param4, param5);
  }
};

MIKTEX_CORE_END_NAMESPACE;
