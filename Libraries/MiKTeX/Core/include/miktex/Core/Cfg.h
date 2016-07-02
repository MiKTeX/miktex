/* miktex/Core/Cfg.h:                                   -*- C++ -*-

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

#if !defined(BD129D5E0E8B45CEAAF056C54F9E6806)
#define BD129D5E0E8B45CEAAF056C54F9E6806

#include <miktex/Core/config.h>

#include <cstdio>

#include <memory>
#include <string>
#include <vector>

#include "MD5.h"
#include "PathName.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

class MIKTEXNOVTABLE IPrivateKeyProvider
{
public:
  virtual PathName MIKTEXTHISCALL GetPrivateKeyFile() = 0;

public:
  virtual bool MIKTEXTHISCALL GetPassphrase(std::string & passphrase) = 0;
};

/// Configuration settings.
class MIKTEXNOVTABLE Cfg
{
public:
  class MIKTEXNOVTABLE Value
  {
  public:
    virtual std::string MIKTEXTHISCALL GetName() const = 0;
  public:
    virtual std::string MIKTEXTHISCALL GetValue() const = 0;
  public:
    virtual std::vector<std::string> GetMultiValue() const = 0;
  public:
    virtual std::string MIKTEXTHISCALL GetDocumentation() const = 0;
  public:
    virtual bool IsCommentedOut() const = 0;
  };

public:
  class MIKTEXNOVTABLE Key
  {
  public:
    virtual std::string MIKTEXTHISCALL GetName() const = 0;
  public:
    virtual std::shared_ptr<Value> MIKTEXTHISCALL GetValue(const std::string & valueName) const = 0;
  public:
    virtual std::vector<std::shared_ptr<Value>> MIKTEXTHISCALL GetValues() const = 0;
  };

public:
  /// Creates a new Cfg object. The caller is responsible for deleting
  /// the object.
  static MIKTEXCORECEEAPI(std::unique_ptr<Cfg>) Create();

public:
  virtual MIKTEXTHISCALL ~Cfg() = 0;

  /// Tests whether the Cfg object is empty.
public:
  virtual bool MIKTEXTHISCALL Empty() const = 0;

  /// Calculates the MD5 value of this Cfg object.
public:
  virtual MD5 MIKTEXTHISCALL GetDigest() const = 0;

  /// Gets a configuration value.
public:
  virtual std::string MIKTEXTHISCALL GetValue(const std::string & keyName, const std::string & valueName) const = 0;

  /// Gets a configuration value.
public:
  virtual bool MIKTEXTHISCALL TryGetValue(const std::string & keyName, const std::string & valueName, std::string & value) const = 0;

  /// Gets a configuration value.
public:
  virtual bool MIKTEXTHISCALL TryGetValue(const std::string & keyName, const std::string & valueName, PathName & path) const = 0;

  /// Gets a configuration value.
public:
  virtual bool MIKTEXTHISCALL TryGetValue(const std::string & keyName, const std::string & valueName, std::vector<std::string> & value) const = 0;

  /// Marks this Cfg object as modified.
public:
  virtual void MIKTEXTHISCALL SetModified(bool b) = 0;

  /// Tests whether this Cfg object has been modified.
public:
  virtual bool MIKTEXTHISCALL IsModified() const = 0;

  /// Sets a configuration value.
public:
  virtual void MIKTEXTHISCALL PutValue(const std::string & keyName, const std::string & valueName, const std::string & value) = 0;

  /// Sets a configuration value.
public:
  virtual void MIKTEXTHISCALL PutValue(const std::string & keyName, const std::string & valueName, const std::string & value, const std::string & documentation, bool commentedOut) = 0;

  /// Reads configuration values from a file.
public:
  virtual void MIKTEXTHISCALL Read(const PathName & path) = 0;

  /// Write configuration settings into a file.
public:
  virtual void MIKTEXTHISCALL Write(const PathName & path) = 0;

  /// Write configuration settings into a file.
public:
  virtual void MIKTEXTHISCALL Write(const PathName & path, const std::string & header) = 0;

  /// Gets all keys.
public:
  virtual std::vector<std::shared_ptr<Key>> GetKeys() = 0;

  /// Starts a key iteration.
public:
  virtual std::shared_ptr<Key> MIKTEXTHISCALL FirstKey() = 0;

  /// Continues a key iteration.
public:
  virtual std::shared_ptr<Key> MIKTEXTHISCALL NextKey() = 0;

  /// Deletes a key.
public:
  virtual void MIKTEXTHISCALL DeleteKey(const std::string & keyName) = 0;

  /// Deletes a value (experimental).
public:
  virtual void MIKTEXTHISCALL DeleteValue(const std::string & keyName, const std::string & valueName) = 0;

public:
  virtual void MIKTEXTHISCALL Read(const PathName & path, bool mustBeSigned) = 0;

public:
  virtual bool MIKTEXTHISCALL IsSigned() const = 0;

  /// Write configuration settings into a file and sign the file.
public:
  virtual void MIKTEXTHISCALL Write(const PathName & path, const std::string & header, IPrivateKeyProvider * privateKeyProvider) = 0;
};

MIKTEX_CORE_END_NAMESPACE;

#endif
