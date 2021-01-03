/* miktex/Configuration/ConfigValue.h:

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

#include <miktex/Configuration/config.h>

#include <climits>
#include <ctime>

#include <string>
#include <vector>

#include <miktex/Util/PathNameUtil>
#include <miktex/Util/StringUtil>

#include "TriState.h"

MIKTEX_CONFIG_BEGIN_NAMESPACE;

/// MiKTeX configuration value.
class ConfigValue
{
public:
  ConfigValue()
  {
  };

public:
  ConfigValue(const ConfigValue& other)
  {
    switch (other.type)
    {
    case Type::String:
      new (&this->s) std::string(other.s);
      break;
    case Type::Int:
      this->i = other.i;
      break;
    case Type::Bool:
      this->b = other.b;
      break;
    case Type::Tri:
      this->t = other.t;
      break;
    case Type::Char:
      this->c = other.c;
      break;
    case Type::StringArray:
      new (&this->sa) std::vector<std::string>(other.sa);
      break;
    case Type::None:
      break;
    }
    this->type = other.type;
    this->section = other.section;
    this->description = other.description;
  }

public:
  ConfigValue& operator=(const ConfigValue& other) = delete;

public:
  ConfigValue(ConfigValue&& other) noexcept
  {
    switch (other.type)
    {
    case Type::String:
      new (&this->s) std::string(other.s);
      this->s = std::move(other.s);
      break;
    case Type::Int:
      this->i = other.i;
      break;
    case Type::Bool:
      this->b = other.b;
      break;
    case Type::Tri:
      this->t = other.t;
      break;
    case Type::Char:
      this->c = other.c;
      break;
    case Type::StringArray:
      new (&this->sa) std::vector<std::string>(other.sa);
      this->sa = std::move(other.sa);
      break;
    case Type::None:
      break;
    }
    this->type = other.type;
    this->section = other.section;
  }

public:
  ConfigValue& operator=(ConfigValue&& other) noexcept
  {
    if (this->type == Type::String && other.type != Type::String)
    {
      this->s.~basic_string();
    }
    else if (this->type == Type::StringArray && other.type != Type::StringArray)
    {
      this->sa.~vector();
    }
    switch (other.type)
    {
    case Type::String:
      if (this->type != Type::String)
      {
        new (&this->s) std::string(other.s);
      }
      this->s = std::move(other.s);
      break;
    case Type::Int:
      this->i = other.i;
      break;
    case Type::Bool:
      this->b = other.b;
      break;
    case Type::Tri:
      this->t = other.t;
      break;
    case Type::Char:
      this->c = other.c;
      break;
    case Type::StringArray:
      if (this->type != Type::StringArray)
      {
        new (&this->sa) std::vector<std::string>(other.sa);
      }
      this->sa = std::move(other.sa);
      break;
    case Type::None:
      break;
    }
    this->type = other.type;
    this->section = other.section;
    return *this;
  }

public:
  virtual ~ConfigValue() noexcept
  {
    if (type == Type::String)
    {
      this->s.~basic_string();
    }
    else if (type == Type::StringArray)
    {
      this->sa.~vector();
    }
    type = Type::None;
  }

public:
  explicit ConfigValue(const std::string& s)
  {
    new(&this->s) std::string(s);
    type = Type::String;
  }

public:
  explicit ConfigValue(const char* lpsz)
  {
    new(&this->s) std::string(lpsz == nullptr ? "" : lpsz);
    type = Type::String;
  }

public:
  explicit ConfigValue(int i)
  {
    this->i = i;
    type = Type::Int;
  }

public:
  explicit ConfigValue(bool b)
  {
    this->b = b;
    type = Type::Bool;
  }

public:
  explicit ConfigValue(TriState t)
  {
    this->t = t;
    type = Type::Tri;
  }

public:
  explicit ConfigValue(char c)
  {
    this->c = c;
    type = Type::Char;
  }

public:
  explicit ConfigValue(const std::vector<std::string>& sa)
  {
    new(&this->sa) std::vector<std::string>(sa);
    type = Type::StringArray;
  }

  /// Gets the configuration value as a string.
  /// @return Returns the configuration value.
public:
  std::string GetString() const
  {
    switch (type)
    {
    case Type::String:
      return this->s;
    case Type::Int:
      return std::to_string(this->i);
    case Type::Bool:
      return this->b ? "true" : "false";
    case Type::Tri:
      return this->t == TriState::Undetermined ? "undetermined" : this->t == TriState::False ? "false" : "true";
    case Type::Char:
      return std::string(1, this->c);
    case Type::StringArray:
      return MiKTeX::Util::StringUtil::Flatten(this->sa, MiKTeX::Util::PathNameUtil::PathNameDelimiter);
    case Type::None:
      // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from undefined configuration value to string."));
      throw 1;
    default:
      // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from type {type} to string."), "type", std::to_string(static_cast<int>(this->type)));
      throw 1;
    }
  }

  /// Gets the configuration value as an integer.
  /// @return Returns the configuration value.
public:
  int GetInt() const
  {
    switch (type)
    {
    case Type::String:
      return std::stoi(this->s);
    case Type::Int:
      return this->i;
    case Type::Bool:
      return (int)this->b;
    case Type::Tri:
      return (int)this->t;
    case Type::Char:
      return (int)this->c;
    case Type::None:
      // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from undefined configuration value to integer."));
      throw 1;
    default:
      // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from type {type} to integer."), "type", std::to_string(static_cast<int>(this->type)));
      throw 1;
    }
  }

  /// Gets the configuration value as a boolean.
  /// @return Returns the configuration value.
public:
  bool GetBool() const
  {
    switch (type)
    {
    case Type::String:
      if (this->s == "0"
        || this->s == "disable"
        || this->s == "off"
        || this->s == "f"
        || this->s == "false"
        || this->s == "n"
        || this->s == "no")
      {
        return false;
      }
      else if (this->s == "1"
        || this->s == "enable"
        || this->s == "on"
        || this->s == "t"
        || this->s == "true"
        || this->s == "y"
        || this->s == "yes")
      {
        return true;
      }
      else
      {
        // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: cannot convert '{s}' to boolean."), "s", this->s);
        throw 1;
      }
    case Type::Int:
      if (this->i == 0)
      {
        return false;
      }
      else if (this->i == 1)
      {
        return true;
      }
      else
      {
        // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: cannot convert {i} to boolean."), "i", std::to_string(this->i));
        throw 1;
      }
    case Type::Bool:
      return this->b;
    case Type::Tri:
      if (this->t == TriState::False)
      {
        return false;
      }
      else if (this->t == TriState::True)
      {
        return true;
      }
      else
      {
        // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: cannot convert {t} to boolean."), "t", std::to_string(static_cast<int>(this->t)));
        throw 1;
      }
    case Type::Char:
      if (this->c == '0'
        || this->c == 'f'
        || this->c == 'n')
      {
        return false;
      }
      else if (this->c == '1'
        || this->c == 't'
        || this->c == 'y')
      {
        return true;
      }
      else
      {
        // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: cannot convert '{c}' to boolean."), "c", string(1, this->c));
        throw 1;
      }
    case Type::None:
      // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from undefined configuration value to boolean."));
      throw 1;
    default:
      // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from type {type} to boolean."), "type", std::to_string(static_cast<int>(this->type)));
      throw 1;
    }
  }

  /// Gets the configuration value as a tri-state.
  /// @return Returns the configuration value.
public:
  TriState GetTriState() const
  {
    switch (type)
    {
    case Type::String:
      if (this->s == "0"
        || this->s == "disable"
        || this->s == "off"
        || this->s == "f"
        || this->s == "false"
        || this->s == "n"
        || this->s == "no")
      {
        return TriState::False;
      }
      else if (this->s == "1"
        || this->s == "enable"
        || this->s == "on"
        || this->s == "t"
        || this->s == "true"
        || this->s == "y"
        || this->s == "yes")
      {
        return TriState::True;
      }
      else if (this->s == ""
        || this->s == "2"
        || this->s == "?"
        || this->s == "undetermined")
      {
        return TriState::Undetermined;
      }
      else
      {
        // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: cannot convert '{s}' to tri-state."), "s", this->s);
        throw 1;
      }
    case Type::Int:
      if (this->i == 0)
      {
        return TriState::False;
      }
      else if (this->i == 1)
      {
        return TriState::True;
      }
      else if (this->i == 2)
      {
        return TriState::Undetermined;
      }
      else
      {
        // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: cannot convert {i} to tri-state."), "i", std::to_string(this->i));
        throw 1;
      }
    case Type::Bool:
      return this->b ? TriState::True : TriState::False;
    case Type::Tri:
      return this->t;
    case Type::Char:
      if (this->c == '0'
        || this->c == 'f'
        || this->c == 'n')
      {
        return TriState::False;
      }
      else if (this->c == '1'
        || this->c == 't'
        || this->c == 'y')
      {
        return TriState::True;
      }
      else if (this->c == '2'
        || this->c == '?')
      {
        return TriState::Undetermined;
      }
      else
      {
        // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: cannot convert '{c}' to tri-state."), "c", string(1, this->c));
        throw 1;
      }
    case Type::None:
      // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from undefined configuration value to tri-state."));
      throw 1;
    default:
      // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from type {type} to tri-state."), "type", std::to_string(static_cast<int>(this->type)));
      throw 1;
    }
  }

  /// Gets the configuration value as a character.
  /// @return Returns the configuration value.
public:
  char GetChar() const
  {
    switch (type)
    {
    case Type::String:
      if (this->s.length() != 1)
      {
        // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: cannot convert '{s}' to character."), "s", this->s);
        throw 1;
      }
      return this->s[0];
    case Type::Int:
      if (this->i < CHAR_MIN || this->i > CHAR_MAX)
      {
        // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: cannot convert {i} to character."), "i", std::to_string(this->i));
        throw 1;
      }
      return (char)this->i;
    case Type::Bool:
      return this->b ? 't' : 'f';
    case Type::Tri:
      return this->t == TriState::Undetermined ? '?' : this->t == TriState::False ? 'f' : 't';
    case Type::Char:
      return this->c;
    case Type::None:
      // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from undefined configuration value to character."));
      throw 1;
    default:
      // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from type {type} to character."), "type", std::to_string(static_cast<int>(this->type)));
      throw 1;
    }
  }

  /// Gets the configuration value as a time_t.
  /// @return Returns the configuration value.
public:
  std::time_t GetTimeT() const
  {
    switch (type)
    {
    case Type::String:
      return std::stoll(this->s);
    case Type::Int:
      return (std::time_t)this->i;
    case Type::None:
      // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from undefined configuration value to time_t."));
      throw 1;
    default:
      // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from type {type} to time_t."), "type", std::to_string(static_cast<int>(this->type)));
      throw 1;
    }
  }

  /// Gets the configuration value as a string list.
  /// @return Returns the configuration value.
public:
  std::vector<std::string> GetStringArray() const
  {
    switch (type)
    {
    case Type::String:
      return MiKTeX::Util::StringUtil::Split(this->s, MiKTeX::Util::PathNameUtil::PathNameDelimiter);
    case Type::StringArray:
      return this->sa;
    case Type::None:
      // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from undefined configuration value to string array."));
      throw 1;
    default:
      // TODO: MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from type {type} to string array."), "type", std::to_string(static_cast<int>(this->type)));
      throw 1;
    }
  }

  /// Value type.
public:
  enum class Type
  {
    None,
    String,
    Int,
    Bool,
    Tri,
    Char,
    StringArray
  };

private:
  Type type = Type::None;

  /// Gets the value type.
  /// @return Returns the value type
public:
  Type GetType() const
  {
    return type;
  }

  /// Tests whether the configuration value is defined.
  /// @return Returns `true`, if the configuration value is defined.
public:
  bool HasValue() const
  {
    return type != Type::None;
  }

private:
  std::string section;

  /// Gets the section of the configuration value.
  /// @return Returns the section name.
public:
  std::string GetSection() const
  {
    return section;
  }

private:
  std::string description;

  /// Gets the description of the configuration value.
  /// @return Returns the description.
public:
  std::string GetDescription() const
  {
    return description;
  }

private:
  union
  {
    std::string s;
    int i;
    bool b;
    TriState t;
    char c;
    std::vector<std::string> sa;
  };
};

MIKTEX_CONFIG_END_NAMESPACE;
