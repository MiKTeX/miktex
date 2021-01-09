/* miktex/Core/Cfg.h:                                   -*- C++ -*-

   Copyright (C) 1996-2021 Christian Schenk

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

#if !defined(BD129D5E0E8B45CEAAF056C54F9E6806)
#define BD129D5E0E8B45CEAAF056C54F9E6806

#include <miktex/Core/config.h>

#include <cstdio>

#include <istream>
#include <memory>
#include <string>
#include <vector>

#include <miktex/Util/OptionSet>
#include <miktex/Util/PathName>

#include "MD5.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

/// Access a private key file.
class MIKTEXNOVTABLE IPrivateKeyProvider
{
  /// Gets the file system path to the private key file.
  /// @return Return the file system path to the private key file.
public:
  virtual MiKTeX::Util::PathName MIKTEXTHISCALL GetPrivateKeyFile() = 0;

  /// Gets the passphrase which protects the private key.
  /// @param[out] passphrase The passphrase.
  /// @return Returns `true`, if the the passphrase is available.
public:
  virtual bool MIKTEXTHISCALL GetPassphrase(std::string& passphrase) = 0;
};

/// INI container.
class MIKTEXNOVTABLE Cfg
{
  /// An instance of this class provides the interface to an INI value.
public:
  class MIKTEXNOVTABLE Value
  {
  public:
    virtual MIKTEXTHISCALL ~Value() noexcept = 0;
  public:
    virtual std::string MIKTEXTHISCALL GetName() const = 0;
  public:
    virtual std::string MIKTEXTHISCALL AsString() const = 0;
  public:
    virtual std::vector<std::string> AsStringVector() const = 0;
  public:
    virtual std::vector<std::string>::const_iterator MIKTEXTHISCALL begin() const = 0;
  public:
    virtual std::vector<std::string>::const_iterator MIKTEXTHISCALL end() const = 0;
  public:
    virtual std::string MIKTEXTHISCALL GetDocumentation() const = 0;
  public:
    virtual bool IsCommentedOut() const = 0;
  };

  /// Implements an interator for INI values.
public:
  class ValueIterator
  {
  public:
    MIKTEXCOREEXPORT MIKTEXTHISCALL ValueIterator();
  public:
    ValueIterator(const ValueIterator& other) = delete;
  public:
    ValueIterator& operator=(const ValueIterator& other) = delete;
  public:
    MIKTEXCOREEXPORT MIKTEXTHISCALL ValueIterator(ValueIterator&& other);
  public:
    MIKTEXCORETHISAPI(ValueIterator&) operator=(ValueIterator&& other);
  public:
    virtual MIKTEXCOREEXPORT MIKTEXTHISCALL ~ValueIterator();
  public:
    MIKTEXCORETHISAPI(std::shared_ptr<Value>) operator*() const;
  public:
    MIKTEXCORETHISAPI(ValueIterator&) operator++();
  public:
    MIKTEXCORETHISAPI(bool) operator==(const ValueIterator& other);
  public:
    MIKTEXCORETHISAPI(bool) operator!=(const ValueIterator& other);
  private:
    class impl;
    std::unique_ptr<impl> pimpl;
  public:
    impl& GetImpl()
    {
      return *pimpl;
    }
  };

  /// An instance of this class provides the interface to an INI section.
public:
  class MIKTEXNOVTABLE Key
  {
  public:
    virtual MIKTEXTHISCALL ~Key() noexcept = 0;
  public:
    virtual std::string MIKTEXTHISCALL GetName() const = 0;
  public:
    virtual std::shared_ptr<Value> MIKTEXTHISCALL GetValue(const std::string& valueName) const = 0;
  public:
    virtual ValueIterator MIKTEXTHISCALL begin() = 0;
  public:
    virtual ValueIterator MIKTEXTHISCALL end() = 0;
  };

  /// Implements an interator for INI sections.
public:
  class KeyIterator
  {
  public:
    MIKTEXCOREEXPORT MIKTEXTHISCALL KeyIterator();
  public:
    KeyIterator(const KeyIterator& other) = delete;
  public:
    KeyIterator& operator=(const KeyIterator& other) = delete;
  public:
    MIKTEXCOREEXPORT MIKTEXTHISCALL KeyIterator(KeyIterator&& other);
  public:
    MIKTEXCORETHISAPI(KeyIterator&) operator=(KeyIterator&& other);
  public:
    virtual MIKTEXCOREEXPORT MIKTEXTHISCALL ~KeyIterator();
  public:
    MIKTEXCORETHISAPI(std::shared_ptr<Key>) operator*() const;
  public:
    MIKTEXCORETHISAPI(KeyIterator&) operator++();
  public:
    MIKTEXCORETHISAPI(bool) operator==(const KeyIterator& other);
  public:
    MIKTEXCORETHISAPI(bool) operator!=(const KeyIterator& other);
  private:
    class impl;
    std::unique_ptr<impl> pimpl;
  public:
    impl& GetImpl()
    {
      return *pimpl;
    }
  };

public:
  /// Creates a new INI container.
  /// @return Returns a smart pointer to the new INI container.
  static MIKTEXCORECEEAPI(std::unique_ptr<Cfg>) Create();

public:
  virtual MIKTEXTHISCALL ~Cfg() noexcept = 0;

  /// INI options.
public:
  enum class Option
  {
    NoOverwriteKeys,
    NoOverwriteValues
  };

public:
  typedef MiKTeX::Util::OptionSet<Option> Options;

  /// Gets current INI options.
  /// @return Returns the current INI options.
  /// @see SetOptions
public:
  virtual Options MIKTEXTHISCALL GetOptions() const = 0;

  /// Sets current INI options.
  /// @param The options to set.
  /// @see GetOptions
public:
  virtual void MIKTEXTHISCALL SetOptions(Options options) = 0;

  /// Tests whether this INI container is empty.
public:
  virtual bool MIKTEXTHISCALL Empty() const = 0;

  /// Calculates the digest (MD5) of this INI container.
  /// @returns Returns the digest.
public:
  virtual MD5 MIKTEXTHISCALL GetDigest() const = 0;

  /// Gets an INI value.
  /// @param keyName The name of the section.
  /// @param valueName The name of the value.
  /// @return Returns a smart pointer to the INI value.
  /// @see PutValue
public:
  virtual std::shared_ptr<Value> MIKTEXTHISCALL GetValue(const std::string& keyName, const std::string& valueName) const = 0;

  /// Gets an INI value.
  /// @param keyName The name of the section.
  /// @param valueName The name of the value.
  /// @param[out] value The value as a string.
  /// @return Returns `true`, if the value was found.
  /// @see PutValue
public:
  virtual bool MIKTEXTHISCALL TryGetValueAsString(const std::string& keyName, const std::string& valueName, std::string& value) const = 0;

  /// Gets an INI value.
  /// @param keyName The name of the section.
  /// @param valueName The name of the value.
  /// @param[out] value The value as a string vector.
  /// @return Returns `true`, if the value was found.
  /// @see PutValue
public:
  virtual bool MIKTEXTHISCALL TryGetValueAsStringVector(const std::string& keyName, const std::string& valueName, std::vector<std::string>& value) const = 0;

  /// Marks this INI container as modified/unmodified.
  /// @param b Indicates wheter the container should be marked as modified.
public:
  virtual void MIKTEXTHISCALL SetModified(bool b) = 0;

  /// Tests whether this INI container has been modified.
public:
  virtual bool MIKTEXTHISCALL IsModified() const = 0;

  /// Sets an INI value.
  /// @param keyName The name of the section.
  /// @param valueName The name of the value.
  /// @param value The value as a string.
  /// @see GetValue
  /// @see GetValueAsString
public:
  virtual void MIKTEXTHISCALL PutValue(const std::string& keyName, const std::string& valueName, const std::string& value) = 0;

  /// Sets an INI value.
  /// @param keyName The name of the section.
  /// @param valueName The name of the value.
  /// @param value The value as a string.
  /// @param documentation The value docstring.
  /// @param commentedOut Indicates whether te value is disabled.
  /// @see GetValue
  /// @see GetValueAsString
public:
  virtual void MIKTEXTHISCALL PutValue(const std::string& keyName, const std::string& valueName, const std::string& value, const std::string& documentation, bool commentedOut) = 0;

  /// Reads from an INI text file.
  /// @param path The path to the INI file.
public:
  virtual void MIKTEXTHISCALL Read(const MiKTeX::Util::PathName& path) = 0;

  /// Reads from an INI text stream.
  /// @param reader The text stream.
public:
  virtual void MIKTEXTHISCALL Read(std::istream& reader) = 0;

  /// Reads from an INI text file and optionally verifies the signature.
  /// @param path The path to the INI file.
  /// @param mustBeSigned Indicates whether the text file must be signed.
public:
  virtual void MIKTEXTHISCALL Read(const MiKTeX::Util::PathName& path, bool mustBeSigned) = 0;

  /// Reads from an INI text stream and optionally verifies the signature.
  /// @param reader The text stream.
  /// @param mustBeSigned Indicates whether the text file must be signed.
public:
  virtual void MIKTEXTHISCALL Read(std::istream& reader, bool mustBeSigned) = 0;

  /// Reads from an INI text file and verifies the signature.
  /// @param path The path to the INI file.
  /// @param publicKeyFile The path to the public key file
public:
  virtual void MIKTEXTHISCALL Read(const MiKTeX::Util::PathName& path, const MiKTeX::Util::PathName& publicKeyFile) = 0;

  /// Reads from an INI text stream and verifies the signature.
  /// @param reader The text stream.
  /// @param publicKeyFile The path to the public key file
public:
  virtual void MIKTEXTHISCALL Read(std::istream& reader, const MiKTeX::Util::PathName& publicKeyFile) = 0;

  /// Writes this INI container into a file.
  /// @param path The path to the INIF file.
public:
  virtual void MIKTEXTHISCALL Write(const MiKTeX::Util::PathName& path) = 0;

  /// Writes this INI container into a file.
  /// @param path The path to the INIF file.
  /// @param header The header to be written as a comment.
public:
  virtual void MIKTEXTHISCALL Write(const MiKTeX::Util::PathName& path, const std::string& header) = 0;

  /// Writes this INI container into a file and signs it.
  /// @param path The path to the INIF file.
  /// @param header The header to be written as a comment.
  /// @param privateKeyProvider The private key which is used to sign the container.
public:
  virtual void MIKTEXTHISCALL Write(const MiKTeX::Util::PathName& path, const std::string& header, IPrivateKeyProvider* privateKeyProvider) = 0;

  /// Gets an INI section.
  /// @param keyName Identifies the section.
  /// @return Returns a smart pointer to the section.
public:
  virtual std::shared_ptr<Key> GetKey(const std::string& keyName) const = 0;

  /// Deletes an INI section.
  /// @param keyName Identifies the section.
public:
  virtual void MIKTEXTHISCALL DeleteKey(const std::string& keyName) = 0;

  /// Deletes an INI value.
  /// @param keyName Identifies the section.
  /// @param keyName Identifies the value within the section.
public:
  virtual void MIKTEXTHISCALL DeleteValue(const std::string& keyName, const std::string& valueName) = 0;

  /// Clears an INI value.
  /// @param keyName Identifies the section.
  /// @param keyName Identifies the value within the section.
public:
  virtual bool MIKTEXTHISCALL ClearValue(const std::string& keyName, const std::string& valueName) = 0;

  /// Checks whether this INI container is signed.
  /// @return Returns `true`, if the container is signed.
public:
  virtual bool MIKTEXTHISCALL IsSigned() const = 0;

  /// Gets an iterator to the first container section.
public:
  virtual KeyIterator MIKTEXTHISCALL begin() = 0;

  /// Gets an iterator referring to the past-the-end section.
public:
  virtual KeyIterator MIKTEXTHISCALL end() = 0;

  /// Gets the numeber of sections.
  /// @eturn Returns the number of sections.  
public:
  virtual std::size_t GetSize() const = 0;
};

MIKTEX_CORE_END_NAMESPACE;

#endif
