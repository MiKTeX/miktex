/* miktex/Core/ConfigurationProvider.h:

   Copyright (C) 1996-2021 Christian Schenk

   This file is part of the MiKTeX Configuration Library.

   The MiKTeX Configuration Library is free software; you can
   redistribute it and/or modify it under the terms of the GNU General
   Public License as published by the Free Software Foundation; either
   version 2, or (at your option) any later version.
   
   The MiKTeX Configuration Library is distributed in the hope that it
   will be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX Configuration Library; if not, write to the
   Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#include <string>

#include <miktex/Configuration/config.h>

#include "ConfigValue.h"
#include "HasNamedValues.h"

/// @namespace MiKTeX::Config
/// @brief The Config namespace.
MIKTEX_CONFIG_BEGIN_NAMESPACE;

/// The MiKTeX ConfigurationProvider interface.
class MIKTEXNOVTABLE ConfigurationProvider
{
  /// Tries to get a configuration value.
  /// @param sectionName Identifies the configuration section.
  /// @param valueName Identifies the value within the section.
  /// @param callback The pointer to an object which implements the `HasNamedValue` interface.
  /// @param[out] value The configuration value as a string.
  /// @return Returns `true`, if the value was found.
public:
  virtual bool MIKTEXTHISCALL TryGetConfigValue(const std::string& sectionName, const std::string& valueName, HasNamedValues* callback, std::string& value) = 0;

  /// Tries to get a configuration value.
  /// @param sectionName Identifies the configuration section.
  /// @param valueName Identifies the value within the section.
  /// @param[out] value The configuration value as a string.
  /// @return Returns `true`, if the value was found.
public:
  virtual bool MIKTEXTHISCALL TryGetConfigValue(const std::string& sectionName, const std::string& valueName, std::string& value) = 0;

  /// Gets a configuration value.
  /// @param sectionName Identifies the configuration section.
  /// @param valueName Identifies the value within the section.
  /// @param defaultValue Value to be returned if the requested value was not found.
  /// @param callback The pointer to an object which implements the `HasNamedValue` interface.
  /// @return Returns the configuration value.
  /// @see SetConfigValue
public:
  virtual ConfigValue MIKTEXTHISCALL GetConfigValue(const std::string& sectionName, const std::string& valueName, const ConfigValue& defaultValue, HasNamedValues* callback) = 0;

  /// Gets a configuration value.
  /// @param sectionName Identifies the configuration section.
  /// @param valueName Identifies the value within the section.
  /// @param defaultValue Value to be returned if the requested value was not found.
  /// @return Returns the configuration value.
  /// @see SetConfigValue
public:
  virtual ConfigValue MIKTEXTHISCALL GetConfigValue(const std::string& sectionName, const std::string& valueName, const ConfigValue& defaultValue) = 0;

  /// Gets a configuration value.
  /// @param sectionName Identifies the configuration section.
  /// @param valueName Identifies the value within the section.
  /// @param callback The pointer to an object which implements the `HasNamedValue` interface.
  /// @return Returns the configuration value.
  /// @see SetConfigValue
public:
  virtual ConfigValue MIKTEXTHISCALL GetConfigValue(const std::string& sectionName, const std::string& valueName, HasNamedValues* callback) = 0;

  /// Gets a configuration value.
  /// @param sectionName Identifies the configuration section.
  /// @param valueName Identifies the value within the section.
  /// @return Returns the configuration value.
  /// @see SetConfigValue
public:
  virtual ConfigValue MIKTEXTHISCALL GetConfigValue(const std::string& sectionName, const std::string& valueName) = 0;

  /// Sets a configuration value.
  /// @param sectionName Identifies the configuration section.
  /// @param valueName Identifies the value within the section.
  /// @param The configuration value.
  /// @see GetConfigValue
public:
  virtual void MIKTEXTHISCALL SetConfigValue(const std::string& sectionName, const std::string& valueName, const ConfigValue& value) = 0;
};

MIKTEX_CONFIG_END_NAMESPACE;
