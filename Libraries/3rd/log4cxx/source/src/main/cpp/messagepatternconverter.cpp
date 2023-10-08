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
#include <log4cxx/pattern/messagepatternconverter.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/spi/location/locationinfo.h>


using namespace log4cxx;
using namespace log4cxx::pattern;

IMPLEMENT_LOG4CXX_OBJECT(MessagePatternConverter)

/**
 * Formats the message of an logging event for a quoted context
  */
class QuotedMessagePatternConverter : public LoggingEventPatternConverter
{
	logchar m_quote;
	public:
		QuotedMessagePatternConverter(logchar quote)
			: LoggingEventPatternConverter(LOG4CXX_STR("Message"), LOG4CXX_STR("quoted"))
			, m_quote(quote)
			{}

		using LoggingEventPatternConverter::format;

		// Duplicate any quote character in the event message
		void format
			( const spi::LoggingEventPtr& event
			, LogString&                  toAppendTo
			, helpers::Pool&              p
			) const override
		{
			auto& input = event->getRenderedMessage();
			size_t endIndex, startIndex = 0;
			while ((endIndex = input.find(m_quote, startIndex)) != input.npos)
			{
				toAppendTo.append(input.substr(startIndex, endIndex - startIndex + 1));
				toAppendTo += m_quote;
				startIndex = endIndex + 1;
			}
			toAppendTo.append(input.substr(startIndex));
		}
};

MessagePatternConverter::MessagePatternConverter()
	: LoggingEventPatternConverter(LOG4CXX_STR("Message")
	, LOG4CXX_STR("message"))
{
}

PatternConverterPtr MessagePatternConverter::newInstance(
	const std::vector<LogString>& options)
{
	if (options.empty() || options.front().empty())
	{
		static PatternConverterPtr def = std::make_shared<MessagePatternConverter>();
		return def;
	}
	return std::make_shared<QuotedMessagePatternConverter>(options.front().front());
}

void MessagePatternConverter::format
	( const spi::LoggingEventPtr& event
	, LogString&                  toAppendTo
	, helpers::Pool&           /* p */
	) const
{
	toAppendTo.append(event->getRenderedMessage());
}

