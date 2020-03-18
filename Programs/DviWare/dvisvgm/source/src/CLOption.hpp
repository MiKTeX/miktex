/*************************************************************************
** CLOption.hpp                                                         **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2020 Martin Gieseking <martin.gieseking@uos.de>   **
**                                                                      **
** This program is free software; you can redistribute it and/or        **
** modify it under the terms of the GNU General Public License as       **
** published by the Free Software Foundation; either version 3 of       **
** the License, or (at your option) any later version.                  **
**                                                                      **
** This program is distributed in the hope that it will be useful, but  **
** WITHOUT ANY WARRANTY; without even the implied warranty of           **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         **
** GNU General Public License for more details.                         **
**                                                                      **
** You should have received a copy of the GNU General Public License    **
** along with this program; if not, see <http://www.gnu.org/licenses/>. **
*************************************************************************/

#ifndef CL_OPTION_HPP
#define CL_OPTION_HPP

#include <iomanip>
#include <sstream>
#include <string>

namespace CL {

class Option
{
	friend class CommandLine;
	public:
		enum class ArgMode {NONE, OPTIONAL, REQUIRED};

	public:
		Option (const char *longname, char shortname, const char *summary)
			: _shortName(shortname), _longName(longname), _summary(summary), _given(false) {}

		virtual ~Option () = default;
		virtual std::string typeString () const  {return "";}
		virtual ArgMode argMode () const         {return ArgMode::NONE;}
		virtual bool given () const              {return _given;}
		virtual char shortName () const          {return _shortName;}
		virtual std::string longName () const    {return _longName;}
		virtual std::string summary () const     {return _summary;}
		virtual std::string argName () const     {return "";}
		virtual std::string valueString () const {return "";}

		virtual std::string helpline () const {
			std::string line;
			if (_shortName != '\0')
				line = std::string("-") + _shortName + ", ";
			else
				line = "    ";
			line += "--" + std::string(_longName);
			if (!argName().empty()) {
				if (argMode() == ArgMode::OPTIONAL)
					line += '[';
				line += '=';
				line += argName();
				if (argMode() == ArgMode::OPTIONAL)
					line += ']';
			}
			if (_summary) {
				line += '\t';
				line += _summary;
			}
			if (argMode() != ArgMode::NONE && !valueString().empty())
				line += std::string(" [") + valueString() + "]";
			return line;
		}

	protected:
		virtual bool parse (std::istream &is, bool longopt) {
			if (is.eof())
				return given(argMode() != ArgMode::REQUIRED);
			if (argMode() == ArgMode::OPTIONAL && is.peek() == ' ')
				return given(false);
			if (longopt && is.get() != '=')
				return given(false);
			return given(parseValue(is));
		}

		virtual bool parseValue (std::istream &is) {
			return is.peek() == EOF || argMode() == ArgMode::NONE;
		}

		bool given (bool val) {return _given = val;}

	private:
		char _shortName;
		const char *_longName;
		const char*_summary;
		bool _given;
};


template <typename T>
constexpr const char* typeString () {return "unknown";}

template<> constexpr const char* typeString<bool> ()        {return "boolean";}
template<> constexpr const char* typeString<int> ()         {return "integer";}
template<> constexpr const char* typeString<unsigned> ()    {return "non-negative integer";}
template<> constexpr const char* typeString<double> ()      {return "floating point";}
template<> constexpr const char* typeString<std::string> () {return "string";}

template <typename T>
inline T parseValue (std::istream &is) {
	T value;
	is >> value;
	return value;
}

template<>
inline std::string parseValue (std::istream &is) {
	is >> std::ws;
	std::string str;
	std::getline(is, str);
	return str;
}


template <typename T, Option::ArgMode mode>
class TypedOption : public Option
{
	public:
		TypedOption (const char *longName, char shortName, const char *argName, T val, const char *summary)
			: Option(longName, shortName, summary), _argName(argName), _value(val) {}

		TypedOption (const char *longName, char shortName, const char *argName, const char *summary)
			: Option(longName, shortName, summary), _argName(argName), _value() {}

		T value () const {return _value;}
		std::string typeString () const override {return CL::typeString<T>();}
		std::string argName() const override     {return _argName ? _argName : "";}

		std::string valueString () const override {
			std::ostringstream oss;
			oss << _value;
			return oss.str();
		}

		ArgMode argMode () const override  {return mode;}

	protected:
		bool parseValue (std::istream &is) override {
			T value = CL::parseValue<T>(is);
			if (!is.fail())
				_value = std::move(value);
			return !is.fail() || (argMode() == ArgMode::OPTIONAL && is.eof());
		}

	private:
		const char *_argName;
		T _value;
};


template <Option::ArgMode mode>
class TypedOption<bool, mode> : public Option
{
	public:
		TypedOption (const char *longName, char shortName, const char *argName, bool val, const char *summary)
			: Option(longName, shortName, summary), _argName(argName), _value(val) {}

		TypedOption (const char *longName, char shortName, const char *argName, const char *summary)
			: Option(longName, shortName, summary), _argName(argName), _value(false) {}

		bool value () const {return _value;}
		std::string valueString () const override {return _value ? "yes" : "no";}
		ArgMode argMode () const override  {return mode;}
		std::string typeString () const override {return CL::typeString<bool>();}
		std::string argName() const override     {return _argName ? _argName : "";}

	protected:
		bool parseValue (std::istream &is) override {
			std::string str;
			is >> str;
			if (is.fail())
				return argMode() != ArgMode::REQUIRED;
			if (str == "yes" || str == "y" || str == "true" || str == "1")
				_value = true;
			else if (str == "no" || str == "n" || str == "false" || str == "0")
				_value = false;
			else
				return false;
			return true;
		}

	private:
		const char *_argName;
		bool _value;
};

} // namespace CL

#endif
