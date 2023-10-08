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
#include <log4cxx/pattern/throwableinformationpatternconverter.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/spi/location/locationinfo.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/private/patternconverter_priv.h>

using namespace log4cxx;
using namespace log4cxx::pattern;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;

struct ThrowableInformationPatternConverter::ThrowableInformationPatternConverterPrivate :
	public PatternConverterPrivate
{
	ThrowableInformationPatternConverterPrivate( const LogString& name, const LogString& style, bool shortReport ) :
		PatternConverterPrivate( name, style ),
		shortReport(shortReport) {}

	/**
	 * If "short", only first line of throwable report will be formatted.
	 */
	const bool shortReport;
};

IMPLEMENT_LOG4CXX_OBJECT(ThrowableInformationPatternConverter)

ThrowableInformationPatternConverter::ThrowableInformationPatternConverter(bool shortReport1) :
	LoggingEventPatternConverter(
		std::make_unique<ThrowableInformationPatternConverterPrivate>(
			LOG4CXX_STR("Throwable"),
			LOG4CXX_STR("throwable"),
			shortReport1))
{
}

PatternConverterPtr ThrowableInformationPatternConverter::newInstance(
	const std::vector<LogString>& options)
{
	if (options.size() > 0 && options[0].compare(LOG4CXX_STR("short")) == 0)
	{
		static PatternConverterPtr shortConverter = std::make_shared<ThrowableInformationPatternConverter>(true);
		return shortConverter;
	}

	static PatternConverterPtr converter = std::make_shared<ThrowableInformationPatternConverter>(false);
	return converter;
}

void ThrowableInformationPatternConverter::format(
	const LoggingEventPtr& /* event */,
	LogString& /* toAppendTo */,
	Pool& /* p */) const
{
}

/**
 * This converter obviously handles throwables.
 * @return true.
 */
bool ThrowableInformationPatternConverter::handlesThrowable() const
{
	return true;
}
