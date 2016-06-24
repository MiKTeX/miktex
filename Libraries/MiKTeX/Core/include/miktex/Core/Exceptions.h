/* miktex/Core/Exceptions.h:                            -*- C++ -*-

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

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(DF181020114F48D1A1ED046824CCCE43)
#define DF181020114F48D1A1ED046824CCCE43

#include <miktex/Core/config.h>

#include <exception>
#include <unordered_map>
#include <sstream>
#include <string>

MIKTEX_CORE_BEGIN_NAMESPACE;

struct SourceLocation
{
  SourceLocation()
  {
  }
  MIKTEXCOREEXPORT MIKTEXTHISCALL SourceLocation(const std::string & functionName, const std::string & fileName, int lineNo);
  std::string functionName;
  std::string fileName;
  int lineNo = 0;
  std::string tag;
};

#define MIKTEX_SOURCE_LOCATION()                                \
  MiKTeX::Core::SourceLocation(__func__, __FILE__, __LINE__)

/// The MiKTeX exception base class.
class MiKTeXException : public std::exception
{
public:
  class KVMAP : public std::unordered_map<std::string, std::string>
  {
  public:
    KVMAP()
    {
    }
  public:
    KVMAP(const std::string & key1, const std::string & value1)
    {
      (*this)[key1] = value1;
    }    
  public:
    KVMAP(const std::string & key1, const std::string & value1, const std::string & key2, const std::string & value2)
    {
      (*this)[key1] = value1;
      (*this)[key2] = value2;
    }
  public:
    KVMAP(const std::string & key1, const std::string & value1, const std::string & key2, const std::string & value2, const std::string & key3, const std::string & value3)
    {
      (*this)[key1] = value1;
      (*this)[key2] = value2;
      (*this)[key3] = value3;
    }
  public:
    KVMAP(const std::string & key1, const std::string & value1, const std::string & key2, const std::string & value2, const std::string & key3, const std::string & value3, const std::string & key4, const std::string & value4)
    {
      (*this)[key1] = value1;
      (*this)[key2] = value2;
      (*this)[key3] = value3;
      (*this)[key4] = value4;
    }
  public: // DEPRECATED
    KVMAP(const std::string value1) :
      KVMAP("", value1)
    {
    }
  public:
    std::string ToString() const
    {
      std::ostringstream oss;
      for (const_iterator it = begin(); it != end(); ++it)
      {
        if (it != begin())
        {
          oss << ", ";
        }
        oss << it->first << "=\"" << it->second << "\"";
      }
      return oss.str();
    }
  };

  /// Initializes a new MiKTeX exception.
public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL MiKTeXException();

  /// Initializes a new MiKTeXException object.
  /// @param lpszProgramInvocationName Name of the program where the
  /// exception was thrown.
  /// @param lpszMessage The message to be presented to the user.
  /// @param info Additional info (e.g., a file name).
  /// @param sourceLocation The source location where the exception was thrown.
public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL MiKTeXException(const std::string & programInvocationName, const std::string & message, const KVMAP & info, const SourceLocation & sourceLocation);

public:
  MiKTeXException(const std::string & message) :
    MiKTeXException("", message, KVMAP(), SourceLocation())
  {
  }

public:
  // DEPRECATED
  MiKTeXException(const char * lpszProgramInvocationName, const char * lpszMessage, const char * lpszInfo, const char * lpszSourceFile, int sourceLine) :
    MiKTeXException(lpszProgramInvocationName, lpszMessage, KVMAP("", lpszInfo == nullptr ? "<nullptr>" : lpszInfo), SourceLocation("", lpszSourceFile, sourceLine))
  {
  }

  /// Gets the exception description.
  /// @return A null-terminated string.
public:
  virtual const char * what() const throw()
  {
    return message.c_str();
  }

public:
  // DEPRECATED
  std::string GetInfo() const noexcept
  {
    return info.ToString();
  }

  /// Gets the name of the invoked program.
  /// @return Returns a string object.
public:
  std::string GetProgramInvocationName() const noexcept
  {
    return programInvocationName;
  }

public:
  SourceLocation GetSourceLocation() const noexcept
  {
    return sourceLocation;
  }

  /// Gets the name of the source code file.
  /// @return Returns a string object.
public:
  std::string GetSourceFile() const noexcept
  {
    return sourceLocation.fileName;
  }

  /// Gets the location (line number) in the source code file.
  /// @return Returns the line number.
public:
  int GetSourceLine() const noexcept
  {
    return sourceLocation.lineNo;
  }

private:
  KVMAP info;

private:
  std::string message;

private:
  std::string programInvocationName;

private:
  SourceLocation sourceLocation;
};

class OperationCancelledException : public MiKTeXException
{
public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL OperationCancelledException();

public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL OperationCancelledException(const std::string & programInvocationName, const std::string & message, const KVMAP & info, const SourceLocation & sourceLocation);
};

/// Instances of this class represent I/O exceptions.
class IOException : public MiKTeXException
{
public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL IOException();

  /// Initializes a new IOException object.
  /// @param lpszProgramInvocationName Name of the program where the
  /// exception was thrown.
  /// @param message The message to be presented to the user.
  /// @param info Additional info (e.g., a file name).
  /// @param sourceLocation The source location where the exception was thrown.
public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL IOException(const std::string & programInvocationName, const std::string & message, const KVMAP & info, const SourceLocation & sourceLocation);
};

/// File not found exception class.
/// An instance of this class is thrown when a file could not be found.
class BrokenPipeException : public IOException
{
public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL BrokenPipeException();

  /// Initializes a new BrokenPipeException object.
  /// @param lpszProgramInvocationName Name of the program where the
  /// exception was thrown.
  /// @param message The message to be presented to the user.
  /// @param info Additional info (e.g., a file name).
  /// @param sourceLocation The source location where the exception was thrown.
public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL BrokenPipeException(const std::string & programInvocationName, const std::string & message, const KVMAP & info, const SourceLocation & sourceLocation);
};

/// File not found exception class.
/// An instance of this class is thrown when a file could not be found.
class FileNotFoundException : public IOException
{
public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL FileNotFoundException();

  /// Initializes a new FileNotFoundException object.
  /// @param lpszProgramInvocationName Name of the program where the
  /// exception was thrown.
  /// @param message The message to be presented to the user.
  /// @param info Additional info (e.g., a file name).
  /// @param sourceLocation The source location where the exception was thrown.
public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL FileNotFoundException(const std::string & programInvocationName, const std::string & message, const KVMAP & info, const SourceLocation & sourceLocation);
};

/// Unauthorized access exception class.
/// An instance of this class is thrown when the operating system denies
/// access.
class UnauthorizedAccessException : public MiKTeXException
{
public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL UnauthorizedAccessException();

  /// Initializes a new UnauthorizedAccessException object.
  /// @param lpszProgramInvocationName Name of the program where the
  /// exception was thrown.
  /// @param message The message to be presented to the user.
  /// @param info Additional info (e.g., a file name).
  /// @param sourceLocation The source location where the exception was thrown.
public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL UnauthorizedAccessException(const std::string & programInvocationName, const std::string & message, const KVMAP & info, const SourceLocation & sourceLocation);
};

#if defined(MIKTEX_WINDOWS)
/// Sharing violation exception class.
class SharingViolationException : public IOException
{
public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL SharingViolationException();

  /// Initializes a new SharingViolationException object.
  /// @param lpszProgramInvocationName Name of the program where the
  /// exception was thrown.
  /// @param message The message to be presented to the user.
  /// @param info Additional info (e.g., a file name).
  /// @param sourceLocation The source location where the exception was thrown.
public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL SharingViolationException(const std::string & programInvocationName, const std::string & message, const KVMAP & info, const SourceLocation & sourceLocation);
};
#endif

MIKTEX_CORE_END_NAMESPACE;

#endif
