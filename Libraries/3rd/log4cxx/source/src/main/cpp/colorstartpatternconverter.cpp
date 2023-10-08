/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <log4cxx/logstring.h>
#include <log4cxx/pattern/colorstartpatternconverter.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/spi/location/locationinfo.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/private/patternconverter_priv.h>

using namespace log4cxx;
using namespace log4cxx::pattern;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(ColorStartPatternConverter)

#define priv static_cast<ColorPatternConverterPrivate*>(m_priv.get())

static LogString colorToANSISequence(const LogString& color, bool isForeground, Pool& pool){
	int numberToConvert = 0;

	if(StringHelper::equalsIgnoreCase(color, LOG4CXX_STR("BLACK"), LOG4CXX_STR("black"))){
		numberToConvert = 30;
	}else if(StringHelper::equalsIgnoreCase(color, LOG4CXX_STR("RED"), LOG4CXX_STR("red"))){
		numberToConvert = 31;
	}else if(StringHelper::equalsIgnoreCase(color, LOG4CXX_STR("GREEN"), LOG4CXX_STR("green"))){
		numberToConvert = 32;
	}else if(StringHelper::equalsIgnoreCase(color, LOG4CXX_STR("YELLOW"), LOG4CXX_STR("yellow"))){
		numberToConvert = 33;
	}else if(StringHelper::equalsIgnoreCase(color, LOG4CXX_STR("BLUE"), LOG4CXX_STR("blue"))){
		numberToConvert = 34;
	}else if(StringHelper::equalsIgnoreCase(color, LOG4CXX_STR("MAGENTA"), LOG4CXX_STR("magenta"))){
		numberToConvert = 35;
	}else if(StringHelper::equalsIgnoreCase(color, LOG4CXX_STR("CYAN"), LOG4CXX_STR("cyan"))){
		numberToConvert = 36;
	}else if(StringHelper::equalsIgnoreCase(color, LOG4CXX_STR("WHITE"), LOG4CXX_STR("white"))){
		numberToConvert = 37;
	}

	if( numberToConvert == 0 ){
		return LOG4CXX_STR("");
	}
	LogString ret;
	if( isForeground == false ){
		numberToConvert += 10;
	}
	StringHelper::toString(numberToConvert, pool, ret);
	return ret;
}

static LogString graphicsModeToANSISequence(const LogString& graphicsMode, Pool& pool){
	int numberToConvert = 0;

	if(StringHelper::equalsIgnoreCase(graphicsMode, LOG4CXX_STR("BOLD"), LOG4CXX_STR("bold"))){
		numberToConvert = 1;
	}else if(StringHelper::equalsIgnoreCase(graphicsMode, LOG4CXX_STR("DIM"), LOG4CXX_STR("dim"))){
		numberToConvert = 2;
	}else if(StringHelper::equalsIgnoreCase(graphicsMode, LOG4CXX_STR("ITALIC"), LOG4CXX_STR("italic"))){
		numberToConvert = 3;
	}else if(StringHelper::equalsIgnoreCase(graphicsMode, LOG4CXX_STR("UNDERLINE"), LOG4CXX_STR("underline"))){
		numberToConvert = 4;
	}else if(StringHelper::equalsIgnoreCase(graphicsMode, LOG4CXX_STR("BLINKING"), LOG4CXX_STR("blinking"))){
		numberToConvert = 5;
	}else if(StringHelper::equalsIgnoreCase(graphicsMode, LOG4CXX_STR("INVERSE"), LOG4CXX_STR("inverse"))){
		numberToConvert = 7;
	}else if(StringHelper::equalsIgnoreCase(graphicsMode, LOG4CXX_STR("STRIKETHROUGH"), LOG4CXX_STR("strikethrough"))){
		numberToConvert = 9;
	}

	if( numberToConvert == 0 ){
		return LOG4CXX_STR("");
	}
	LogString ret;
	StringHelper::toString(numberToConvert, pool, ret);
	return ret;
}

static LogString convertSingleSequence(const LogString& sequence, Pool& pool){
	LogString strInParens;
	bool inParens = false;
	bool hasParens = false;
	size_t x = 0;

	for(x = 0; x < sequence.length(); x++){
		if( sequence[x] == '(' && !inParens ){
			inParens = true;
			hasParens = true;
			continue;
		}else if( sequence[x] == '(' && inParens ){
			// Unbalanced parens - parse invalid
			return LOG4CXX_STR("");
		}

		if( sequence[x] == ')' && inParens ){
			hasParens = true;
			inParens = false;
			break;
		}

		if( inParens ){
			strInParens.push_back(sequence[x]);
		}
	}

	if( (x != (sequence.length() - 1) || inParens) && hasParens ){
		// Unbalanced parens, or more data in the string than we expected - parse invalid
		return LOG4CXX_STR("");
	}

	if(StringHelper::startsWith(sequence, LOG4CXX_STR("fg("))){
		// Parse foreground
		return colorToANSISequence(strInParens, true, pool);
	}else if(StringHelper::startsWith(sequence, LOG4CXX_STR("bg("))){
		return colorToANSISequence(strInParens, false, pool);
	}else{
		return graphicsModeToANSISequence(sequence, pool);
	}
}

struct ColorStartPatternConverter::ColorPatternConverterPrivate : public PatternConverterPrivate
{
	ColorPatternConverterPrivate( const LogString& name, const LogString& style ) :
		PatternConverterPrivate( name, style ){}

	LogString m_fatalColor;
	LogString m_errorColor;
	LogString m_warnColor;
	LogString m_infoColor;
	LogString m_debugColor;
	LogString m_traceColor;
};

ColorStartPatternConverter::ColorStartPatternConverter() :
	LoggingEventPatternConverter(std::make_unique<ColorPatternConverterPrivate>(LOG4CXX_STR("Color Start"),
		LOG4CXX_STR("colorStart")))
{
}

PatternConverterPtr ColorStartPatternConverter::newInstance(
	const std::vector<LogString>& /* options */)
{
	static PatternConverterPtr instance = std::make_shared<ColorStartPatternConverter>();
	return instance;
}

void ColorStartPatternConverter::format(
	const LoggingEventPtr& event,
	LogString& toAppendTo,
	Pool& p) const
{

	log4cxx::LevelPtr lvl = event->getLevel();

	switch (lvl->toInt())
	{
		case log4cxx::Level::FATAL_INT:
			toAppendTo.append(priv->m_fatalColor);
			break;

		case log4cxx::Level::ERROR_INT:
			toAppendTo.append(priv->m_errorColor);
			break;

		case log4cxx::Level::WARN_INT:
			toAppendTo.append(priv->m_warnColor);
			break;

		case log4cxx::Level::INFO_INT:
			toAppendTo.append(priv->m_infoColor);
			break;

		case log4cxx::Level::DEBUG_INT:
			toAppendTo.append(priv->m_debugColor);
			break;

		case log4cxx::Level::TRACE_INT:
			toAppendTo.append(priv->m_traceColor);
			break;

		default:
			break;
	}
}

void ColorStartPatternConverter::setFatalColor(const LogString& color){
	parseColor(color, &(priv->m_fatalColor));
}

void ColorStartPatternConverter::setErrorColor(const LogString& color){
	parseColor(color, &(priv->m_errorColor));
}

void ColorStartPatternConverter::setWarnColor(const LogString& color){
	parseColor(color, &(priv->m_warnColor));
}

void ColorStartPatternConverter::setInfoColor(const LogString& color){
	parseColor(color, &(priv->m_infoColor));
}

void ColorStartPatternConverter::setDebugColor(const LogString& color){
	parseColor(color, &(priv->m_debugColor));
}

void ColorStartPatternConverter::setTraceColor(const LogString& color){
	parseColor(color, &(priv->m_traceColor));
}

void ColorStartPatternConverter::parseColor(const LogString& color, LogString* result){
	LogString lower = StringHelper::toLowerCase(color);
	Pool pool;

	// If the color we are trying to parse is blank, clear our result
	if(StringHelper::trim(color).empty() ||
			StringHelper::equalsIgnoreCase(color,
										   LOG4CXX_STR("NONE"),
										   LOG4CXX_STR("none"))){
		result->clear();
		return;
	}

	if( StringHelper::startsWith(lower, LOG4CXX_STR("\\x1b")) ){
		if( color[color.size() - 1] != 'm' ){
			// In order for this to be a valid ANSI escape sequence,
			// it must end with an 'm'.  If it does not, reject.
			return;
		}
		// We start with an escape sequence, copy the data over after the escape byte
		result->clear();
		result->append(LOG4CXX_STR("\x1b"));
		for( size_t x = 4; x < color.size(); x++ ){
			result->push_back(color[x]);
		}
	}else{
		// We do not start with an escape sequence: try to parse color
		// Escape sequence information:
		// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
		// https://en.wikipedia.org/wiki/ANSI_escape_code
		result->clear();
		result->append(LOG4CXX_STR("\x1b["));
		LogString tmp;
		for( size_t x = 0; x < color.size(); x++ ){
			if(color[x] == '|' ){
				LogString toAppend = convertSingleSequence(tmp, pool);
				tmp.clear();
				if(!toAppend.empty()){
					result->push_back(';');
					result->append(toAppend);
				}
			}else{
				tmp.push_back(color[x]);
			}
		}
		LogString toAppend = convertSingleSequence(tmp, pool);
		tmp.clear();
		if(!toAppend.empty()){
			result->push_back(';');
			result->append(toAppend);
		}
		result->append(LOG4CXX_STR("m"));
	}
}
