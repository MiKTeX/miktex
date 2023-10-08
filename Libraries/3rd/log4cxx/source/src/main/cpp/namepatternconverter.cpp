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
#include <log4cxx/pattern/namepatternconverter.h>
#include <log4cxx/pattern/nameabbreviator.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/private/patternconverter_priv.h>

using namespace log4cxx;
using namespace log4cxx::pattern;
using namespace log4cxx::spi;

#define priv static_cast<NamePatternConverterPrivate*>(m_priv.get())

struct NamePatternConverter::NamePatternConverterPrivate : public PatternConverterPrivate
{
	NamePatternConverterPrivate( const LogString& name, const LogString& style, const NameAbbreviatorPtr abbrev ) :
		PatternConverterPrivate( name, style ),
		abbreviator(abbrev) {}

	/**
	 * Abbreviator.
	 */
	const NameAbbreviatorPtr abbreviator;
};

IMPLEMENT_LOG4CXX_OBJECT(NamePatternConverter)

NamePatternConverter::NamePatternConverter(
	const LogString& name1,
	const LogString& style1,
	const std::vector<LogString>& options) :
	LoggingEventPatternConverter(std::make_unique<NamePatternConverterPrivate>(name1, style1,
			getAbbreviator(options)))
{
}

NameAbbreviatorPtr NamePatternConverter::getAbbreviator(
	const std::vector<LogString>& options)
{
	if (options.size() > 0)
	{
		return NameAbbreviator::getAbbreviator(options[0]);
	}

	return NameAbbreviator::getDefaultAbbreviator();
}

/**
 * Abbreviate name in string buffer.
 * @param nameStart starting position of name to abbreviate.
 * @param buf string buffer containing name.
 */
void NamePatternConverter::abbreviate(LogString::size_type nameStart, LogString& buf) const
{
	priv->abbreviator->abbreviate(nameStart, buf);
}
