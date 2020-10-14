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
#include <log4cxx/filter/loggermatchfilter.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/optionconverter.h>

using namespace log4cxx;
using namespace log4cxx::filter;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(LoggerMatchFilter)


LoggerMatchFilter::LoggerMatchFilter()
	: acceptOnMatch(true), loggerToMatch(LOG4CXX_STR("root"))
{
}

void LoggerMatchFilter::setLoggerToMatch(const LogString& value)
{
	loggerToMatch = value;
}

LogString LoggerMatchFilter::getLoggerToMatch() const
{
	return loggerToMatch;
}

void LoggerMatchFilter::setOption(const LogString& option,
	const LogString& value)
{

	if (StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("LOGGERTOMATCH"), LOG4CXX_STR("loggertomatch")))
	{
		setLoggerToMatch(value);
	}
	else if (StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("ACCEPTONMATCH"), LOG4CXX_STR("acceptonmatch")))
	{
		acceptOnMatch = OptionConverter::toBoolean(value, acceptOnMatch);
	}
}

Filter::FilterDecision LoggerMatchFilter::decide(
	const spi::LoggingEventPtr& event) const
{
	bool matchOccured = loggerToMatch == event->getLoggerName();

	if (matchOccured)
	{
		if (acceptOnMatch)
		{
			return Filter::ACCEPT;
		}
		else
		{
			return Filter::DENY;
		}
	}
	else
	{
		return Filter::NEUTRAL;
	}
}

