/**
 * @file miktex/Configuration/ConfigValue.h
 * @author Christian Schenk
 * @brief Configuration value
 *
 * @copyright Copyright © 1996-2022 Christian Schenk
 *
 * This file is part of the MiKTeX Configuration Library.
 *
 * The MiKTeX Configuration Library is licensed under GNU General Public License
 * version 2 or any later version.
 */

#pragma once

#include <miktex/Configuration/config.h>

#include <climits>
#include <ctime>

#include <exception>
#include <string>
#include <vector>

#include <miktex/Util/PathNameUtil>
#include <miktex/Util/StringUtil>

#include "TriState.h"

MIKTEX_CONFIG_BEGIN_NAMESPACE;

class Exception :
    public std::exception
{

public:

    Exception(const std::string& msg) :
        msg(msg)
    {
    }

    const char* what() const noexcept override
    {
        return msg.c_str();
    }

private:

    std::string msg;
};

class ConfigurationError :
    public MiKTeX::Configuration::Exception
{

public:

    ConfigurationError(const std::string& msg) :
        Exception("Configuration error: " + msg)
    {
    }
};


/// MiKTeX configuration value.
class ConfigValue
{

public:

    ConfigValue()
    {
    };

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

    ConfigValue& operator=(const ConfigValue& other) = delete;

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

    explicit ConfigValue(const std::string& s)
    {
        new(&this->s) std::string(s);
        type = Type::String;
    }

    explicit ConfigValue(const char* lpsz)
    {
        new(&this->s) std::string(lpsz == nullptr ? "" : lpsz);
        type = Type::String;
    }

    explicit ConfigValue(int i)
    {
        this->i = i;
        type = Type::Int;
    }

    explicit ConfigValue(bool b)
    {
        this->b = b;
        type = Type::Bool;
    }

    explicit ConfigValue(TriState t)
    {
        this->t = t;
        type = Type::Tri;
    }

    explicit ConfigValue(char c)
    {
        this->c = c;
        type = Type::Char;
    }

    explicit ConfigValue(const std::vector<std::string>& sa)
    {
        new(&this->sa) std::vector<std::string>(sa);
        type = Type::StringArray;
    }

    /// Gets the configuration value as a string.
    /// @return Returns the configuration value.
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
            throw ConfigurationError("no conversion from undefined configuration value to string.");
        default:
            throw ConfigurationError("no conversion to string from type: " + std::to_string(static_cast<int>(this->type)));
        }
    }

    /// Gets the configuration value as an integer.
    /// @return Returns the configuration value.
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
            throw ConfigurationError("no conversion from undefined configuration value to integer.");
        default:
            throw ConfigurationError("no conversion to integer from type: " + std::to_string(static_cast<int>(this->type)));
        }
    }

    /// Gets the configuration value as a boolean.
    /// @return Returns the configuration value.
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
                throw ConfigurationError("cannot convert to boolean from string: " + this->s);
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
                throw ConfigurationError("cannot convert to boolean from int: " + std::to_string(this->i));
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
                throw ConfigurationError("cannot convert to boolean from tri-state: " + std::to_string(static_cast<int>(this->t)));
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
                throw ConfigurationError("cannot convert to boolean from char: " + std::string(1, this->c));
            }
        case Type::None:
            throw ConfigurationError("no conversion from undefined configuration value to boolean.");
        default:
            throw ConfigurationError("no conversion to boolean from type: " + std::to_string(static_cast<int>(this->type)));
        }
    }

    /// Gets the configuration value as a tri-state.
    /// @return Returns the configuration value.
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
                throw ConfigurationError("cannot convert to tri-state from string: " + this->s);
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
                throw ConfigurationError("cannot convert to tri-state from int: " + std::to_string(this->i));
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
                throw ConfigurationError("cannot convert to tri-state from char: " + std::string(1, this->c));
            }
        case Type::None:
            throw ConfigurationError("no conversion from undefined configuration value to tri-state.");
        default:
            throw ConfigurationError("no conversion to tri-state from type: " + std::to_string(static_cast<int>(this->type)));
        }
    }

    /// Gets the configuration value as a character.
    /// @return Returns the configuration value.
    char GetChar() const
    {
        switch (type)
        {
        case Type::String:
            if (this->s.length() != 1)
            {
                throw ConfigurationError("cannot convert to char from string: " + this->s);
            }
            return this->s[0];
        case Type::Int:
            if (this->i < CHAR_MIN || this->i > CHAR_MAX)
            {
                throw ConfigurationError("cannot convert to char from int: " + std::to_string(this->i));
            }
            return (char)this->i;
        case Type::Bool:
            return this->b ? 't' : 'f';
        case Type::Tri:
            return this->t == TriState::Undetermined ? '?' : this->t == TriState::False ? 'f' : 't';
        case Type::Char:
            return this->c;
        case Type::None:
            throw ConfigurationError("no conversion from undefined configuration value to char.");
        default:
            throw ConfigurationError("no conversion from type to char from type: " + std::to_string(static_cast<int>(this->type)));
        }
    }

    /// Gets the configuration value as a time_t.
    /// @return Returns the configuration value.
    std::time_t GetTimeT() const
    {
        switch (type)
        {
        case Type::String:
            return std::stoll(this->s);
        case Type::Int:
            return (std::time_t)this->i;
        case Type::None:
            throw ConfigurationError("no conversion from undefined configuration value to time_t.");
        default:
            throw ConfigurationError("no conversion from time_t from type: " + std::to_string(static_cast<int>(this->type)));
        }
    }

    /// Gets the configuration value as a string list.
    /// @return Returns the configuration value.
    std::vector<std::string> GetStringArray() const
    {
        switch (type)
        {
        case Type::String:
            return MiKTeX::Util::StringUtil::Split(this->s, MiKTeX::Util::PathNameUtil::PathNameDelimiter);
        case Type::StringArray:
            return this->sa;
        case Type::None:
            throw ConfigurationError("no conversion from undefined configuration value to string array.");
        default:
            throw ConfigurationError("no conversion from to string array from type: " + std::to_string(static_cast<int>(this->type)));
        }
    }

    /// Value type.
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

    Type type = Type::None;

    /// Gets the value type.
    /// @return Returns the value type
    Type GetType() const;

    /// Tests whether the configuration value is defined.
    /// @return Returns `true`, if the configuration value is defined.
    bool HasValue() const
    {
        return type != Type::None;
    }

    /// Gets the section of the configuration value.
    /// @return Returns the section name.
    std::string GetSection() const;

    /// Gets the description of the configuration value.
    /// @return Returns the description.
    std::string GetDescription() const;

private:
    union
    {
        bool b;
        char c;
        int i;
        std::string s;
        std::vector<std::string> sa;
        TriState t;
    };

    std::string description;
    std::string section;
};

inline ConfigValue::Type ConfigValue::GetType() const
{
    return type;
}

inline std::string ConfigValue::GetSection() const
{
    return section;
}

inline  std::string ConfigValue::GetDescription() const
{
    return description;
}

MIKTEX_CONFIG_END_NAMESPACE;
