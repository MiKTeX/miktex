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
#include <log4cxx/patternlayout.h>
#include <log4cxx/pattern/patternparser.h>
#include <log4cxx/pattern/loggingeventpatternconverter.h>
#include <log4cxx/pattern/formattinginfo.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/optionconverter.h>

#include <log4cxx/pattern/loggerpatternconverter.h>
#include <log4cxx/pattern/colorendpatternconverter.h>
#include <log4cxx/pattern/colorstartpatternconverter.h>
#include <log4cxx/pattern/shortfilelocationpatternconverter.h>
#include <log4cxx/pattern/literalpatternconverter.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/pattern/classnamepatternconverter.h>
#include <log4cxx/pattern/datepatternconverter.h>
#include <log4cxx/pattern/filedatepatternconverter.h>
#include <log4cxx/pattern/filelocationpatternconverter.h>
#include <log4cxx/pattern/fulllocationpatternconverter.h>
#include <log4cxx/pattern/integerpatternconverter.h>
#include <log4cxx/pattern/linelocationpatternconverter.h>
#include <log4cxx/pattern/messagepatternconverter.h>
#include <log4cxx/pattern/lineseparatorpatternconverter.h>
#include <log4cxx/pattern/methodlocationpatternconverter.h>
#include <log4cxx/pattern/levelpatternconverter.h>
#include <log4cxx/pattern/relativetimepatternconverter.h>
#include <log4cxx/pattern/threadpatternconverter.h>
#include <log4cxx/pattern/mdcpatternconverter.h>
#include <log4cxx/pattern/ndcpatternconverter.h>
#include <log4cxx/pattern/propertiespatternconverter.h>
#include <log4cxx/pattern/throwableinformationpatternconverter.h>
#include <log4cxx/pattern/threadusernamepatternconverter.h>


using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;
using namespace log4cxx::pattern;

struct PatternLayout::PatternLayoutPrivate
{
	PatternLayoutPrivate() {}
	PatternLayoutPrivate(const LogString& pattern) :
		conversionPattern(pattern) {}

	/**
	 * Conversion pattern.
	 */
	LogString conversionPattern;

	/**
	 * Pattern converters.
	 */
	LoggingEventPatternConverterList patternConverters;

	/**
	 * Field widths and alignment corresponding to pattern converters.
	 */
	FormattingInfoList patternFields;

	LogString m_fatalColor = LOG4CXX_STR("\\x1B[35m"); //magenta
	LogString m_errorColor = LOG4CXX_STR("\\x1B[31m"); //red
	LogString m_warnColor = LOG4CXX_STR("\\x1B[33m"); //yellow
	LogString m_infoColor = LOG4CXX_STR("\\x1B[32m"); //green
	LogString m_debugColor = LOG4CXX_STR("\\x1B[36m"); //cyan;
	LogString m_traceColor = LOG4CXX_STR("\\x1B[34m"); //blue;
};

IMPLEMENT_LOG4CXX_OBJECT(PatternLayout)


PatternLayout::PatternLayout() :
	m_priv(std::make_unique<PatternLayoutPrivate>())
{
}

/**
Constructs a PatternLayout using the supplied conversion pattern.
*/
PatternLayout::PatternLayout(const LogString& pattern) :
	m_priv(std::make_unique<PatternLayoutPrivate>(pattern))
{
	Pool pool;
	activateOptions(pool);
}

PatternLayout::~PatternLayout() {}

void PatternLayout::setConversionPattern(const LogString& pattern)
{
	m_priv->conversionPattern = pattern;
	Pool pool;
	activateOptions(pool);
}

void PatternLayout::format(LogString& output,
	const spi::LoggingEventPtr& event,
	Pool& pool) const
{
	std::vector<FormattingInfoPtr>::const_iterator formatterIter =
		m_priv->patternFields.begin();

	for (std::vector<LoggingEventPatternConverterPtr>::const_iterator
		converterIter = m_priv->patternConverters.begin();
		converterIter != m_priv->patternConverters.end();
		converterIter++, formatterIter++)
	{
		int startField = (int)output.length();
		(*converterIter)->format(event, output, pool);
		(*formatterIter)->format(startField, output);
	}

}

void PatternLayout::setOption(const LogString& option, const LogString& value)
{
	if (StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("CONVERSIONPATTERN"),
			LOG4CXX_STR("conversionpattern")))
	{
		m_priv->conversionPattern = OptionConverter::convertSpecialChars(value);
	}else if(StringHelper::equalsIgnoreCase(option,
											LOG4CXX_STR("ERRORCOLOR"),
											LOG4CXX_STR("errorcolor"))){
		m_priv->m_errorColor = value;
		LogLog::debug(LOG4CXX_STR("Setting error color to "));
		LogLog::debug(value);
	}else if(StringHelper::equalsIgnoreCase(option,
											LOG4CXX_STR("FATALCOLOR"),
											LOG4CXX_STR("fatalcolor"))){
		m_priv->m_fatalColor = value;
	}else if(StringHelper::equalsIgnoreCase(option,
											LOG4CXX_STR("WARNCOLOR"),
											LOG4CXX_STR("warncolor"))){
		m_priv->m_warnColor = value;
	}else if(StringHelper::equalsIgnoreCase(option,
											LOG4CXX_STR("INFOCOLOR"),
											LOG4CXX_STR("infocolor"))){
		m_priv->m_infoColor = value;
	}else if(StringHelper::equalsIgnoreCase(option,
											LOG4CXX_STR("DEBUGCOLOR"),
											LOG4CXX_STR("debugcolor"))){
		m_priv->m_debugColor = value;
	}else if(StringHelper::equalsIgnoreCase(option,
											LOG4CXX_STR("TRACECOLOR"),
											LOG4CXX_STR("tracecolor"))){
		m_priv->m_traceColor = value;
	}
}

void PatternLayout::activateOptions(Pool&)
{
	LogString pat(m_priv->conversionPattern);

	if (pat.empty())
	{
		pat = LOG4CXX_STR("%m%n");
	}

	m_priv->patternConverters.erase(m_priv->patternConverters.begin(), m_priv->patternConverters.end());
	m_priv->patternFields.erase(m_priv->patternFields.begin(), m_priv->patternFields.end());
	std::vector<PatternConverterPtr> converters;
	PatternParser::parse(pat,
		converters,
		m_priv->patternFields,
		getFormatSpecifiers());

	//
	//   strip out any pattern converters that don't handle LoggingEvents
	//
	//
	for (std::vector<PatternConverterPtr>::const_iterator converterIter = converters.begin();
		converterIter != converters.end();
		converterIter++)
	{
		LoggingEventPatternConverterPtr eventConverter =
			log4cxx::cast<LoggingEventPatternConverter>(*converterIter);

		if (eventConverter != NULL)
		{
			m_priv->patternConverters.push_back(eventConverter);
		}
	}
}

#define RULES_PUT(spec, cls) \
	specs.insert(PatternMap::value_type(LogString(LOG4CXX_STR(spec)), cls ::newInstance))


log4cxx::pattern::PatternMap PatternLayout::getFormatSpecifiers()
{
	PatternMap specs;
	RULES_PUT("c", LoggerPatternConverter);
	RULES_PUT("logger", LoggerPatternConverter);

	RULES_PUT("C", ClassNamePatternConverter);
	RULES_PUT("class", ClassNamePatternConverter);

	specs.insert(PatternMap::value_type(LogString(LOG4CXX_STR("Y")), std::bind(&PatternLayout::createColorStartPatternConverter, this, std::placeholders::_1)));
	RULES_PUT("y", ColorEndPatternConverter);

	RULES_PUT("d", DatePatternConverter);
	RULES_PUT("date", DatePatternConverter);

	RULES_PUT("f", ShortFileLocationPatternConverter);

	RULES_PUT("F", FileLocationPatternConverter);
	RULES_PUT("file", FileLocationPatternConverter);

	RULES_PUT("l", FullLocationPatternConverter);

	RULES_PUT("L", LineLocationPatternConverter);
	RULES_PUT("line", LineLocationPatternConverter);

	RULES_PUT("m", MessagePatternConverter);
	RULES_PUT("message", MessagePatternConverter);

	RULES_PUT("n", LineSeparatorPatternConverter);

	RULES_PUT("M", MethodLocationPatternConverter);
	RULES_PUT("method", MethodLocationPatternConverter);

	RULES_PUT("p", LevelPatternConverter);
	RULES_PUT("level", LevelPatternConverter);

	RULES_PUT("r", RelativeTimePatternConverter);
	RULES_PUT("relative", RelativeTimePatternConverter);

	RULES_PUT("t", ThreadPatternConverter);
	RULES_PUT("thread", ThreadPatternConverter);

	RULES_PUT("T", ThreadUsernamePatternConverter);
	RULES_PUT("threadname", ThreadUsernamePatternConverter);

	RULES_PUT("x", NDCPatternConverter);
	RULES_PUT("ndc", NDCPatternConverter);

	RULES_PUT("X", PropertiesPatternConverter);
	RULES_PUT("J", MDCPatternConverter);
	RULES_PUT("properties", PropertiesPatternConverter);

	RULES_PUT("throwable", ThrowableInformationPatternConverter);
	return specs;
}

LogString PatternLayout::getConversionPattern() const
{
	return m_priv->conversionPattern;
}

pattern::PatternConverterPtr PatternLayout::createColorStartPatternConverter(const std::vector<LogString>& options){
	std::shared_ptr<ColorStartPatternConverter> colorPatternConverter = std::make_shared<ColorStartPatternConverter>();

	colorPatternConverter->setErrorColor(m_priv->m_errorColor);
	colorPatternConverter->setFatalColor(m_priv->m_fatalColor);
	colorPatternConverter->setWarnColor(m_priv->m_warnColor);
	colorPatternConverter->setInfoColor(m_priv->m_infoColor);
	colorPatternConverter->setDebugColor(m_priv->m_debugColor);
	colorPatternConverter->setTraceColor(m_priv->m_traceColor);

	return colorPatternConverter;
}



