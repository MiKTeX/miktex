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

#include <log4cxx/pattern/mdcpatternconverter.h>
#include <log4cxx/private/patternconverter_priv.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/jsonlayout.h>

using namespace log4cxx;
using namespace log4cxx::pattern;

IMPLEMENT_LOG4CXX_OBJECT(MDCPatternConverter)

MDCPatternConverter::MDCPatternConverter
	( const LogString&              name
	, const LogString&              style
	, const std::vector<LogString>& options
	)
	: LoggingEventPatternConverter(std::make_unique<PatternConverter::PatternConverterPrivate>(name, style))
{
}

PatternConverterPtr MDCPatternConverter::newInstance(
	const std::vector<LogString>& options)
{
	if (options.empty())
	{
		static PatternConverterPtr def = std::make_shared<MDCPatternConverter>();
		return def;
	}
	return std::make_shared<MDCPatternConverter>(LogString(), options.front());
}

void MDCPatternConverter::format
	( const spi::LoggingEventPtr& event
	, LogString&                  toAppendTo
	, helpers::Pool&           /* p */
	) const
{
	size_t startIndex = toAppendTo.size();
	if (m_priv->name.empty()) // Full MDC required?
	{
		bool first = true;
		for (auto key : event->getMDCKeySet())
		{
			toAppendTo.append(first ? LOG4CXX_STR("{") : LOG4CXX_STR(","));
			JSONLayout::appendItem(key, toAppendTo);
			toAppendTo.append(LOG4CXX_STR(":"));
			LogString value;
			event->getMDC(key, value);
			JSONLayout::appendItem(value, toAppendTo);
			first = false;
		}
		if (!first)
			toAppendTo.append(LOG4CXX_STR("}"));
	}
	else
		event->getMDC(m_priv->name, toAppendTo);
	if (!m_priv->style.empty()) // In a quoted context?
	{
		auto quote = m_priv->style.front();
		size_t endIndex;
		while ((endIndex = toAppendTo.find(quote, startIndex)) != toAppendTo.npos)
		{
			toAppendTo.insert(endIndex + 1, 1, quote);
			startIndex = endIndex + 2;
		}
	}
}
