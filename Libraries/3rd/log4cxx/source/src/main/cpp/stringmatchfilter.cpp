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
#include <log4cxx/filter/stringmatchfilter.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/private/filter_priv.h>

using namespace log4cxx;
using namespace log4cxx::filter;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;

#define priv static_cast<StringMatchFilterPrivate*>(m_priv.get())

struct StringMatchFilter::StringMatchFilterPrivate : public FilterPrivate
{
	StringMatchFilterPrivate() : FilterPrivate(),
		acceptOnMatch(true),
		stringToMatch() {}

	bool acceptOnMatch;
	LogString stringToMatch;
};

IMPLEMENT_LOG4CXX_OBJECT(StringMatchFilter)

StringMatchFilter::StringMatchFilter() :
	Filter(std::make_unique<StringMatchFilterPrivate>())
{
}

StringMatchFilter::~StringMatchFilter() {}

void StringMatchFilter::setOption(const LogString& option,
	const LogString& value)
{

	if (StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("STRINGTOMATCH"), LOG4CXX_STR("stringtomatch")))
	{
		priv->stringToMatch = value;
	}
	else if (StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("ACCEPTONMATCH"), LOG4CXX_STR("acceptonmatch")))
	{
		priv->acceptOnMatch = OptionConverter::toBoolean(value, priv->acceptOnMatch);
	}
}

Filter::FilterDecision StringMatchFilter::decide(
	const log4cxx::spi::LoggingEventPtr& event) const
{
	const LogString& msg = event->getRenderedMessage();

	if (msg.empty() || priv->stringToMatch.empty())
	{
		return Filter::NEUTRAL;
	}


	if ( msg.find(priv->stringToMatch) == LogString::npos )
	{
		return Filter::NEUTRAL;
	}
	else
	{
		// we've got a match
		if (priv->acceptOnMatch)
		{
			return Filter::ACCEPT;
		}
		else
		{
			return Filter::DENY;
		}
	}
}

void StringMatchFilter::setStringToMatch(const LogString& stringToMatch1)
{
	priv->stringToMatch.assign(stringToMatch1);
}

const LogString& StringMatchFilter::getStringToMatch() const
{
	return priv->stringToMatch;
}

void StringMatchFilter::setAcceptOnMatch(bool acceptOnMatch1)
{
	priv->acceptOnMatch = acceptOnMatch1;
}

bool StringMatchFilter::getAcceptOnMatch() const
{
	return priv->acceptOnMatch;
}
