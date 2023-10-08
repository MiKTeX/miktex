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
#include <log4cxx/filter/locationinfofilter.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/private/filter_priv.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/transcoder.h>

using namespace log4cxx;
using namespace log4cxx::filter;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;

#define priv static_cast<LocationInfoFilterPrivate*>(m_priv.get())

struct LocationInfoFilter::LocationInfoFilterPrivate : public FilterPrivate
{
	LocationInfoFilterPrivate() : FilterPrivate(),
		acceptOnMatch(true),
		mustMatchAll(false),
		lineNumber(-1)
	{}

	bool    acceptOnMatch;
	bool    mustMatchAll; // true = AND; false = OR
	int     lineNumber;
	std::string methodName;
};

IMPLEMENT_LOG4CXX_OBJECT(LocationInfoFilter)

LocationInfoFilter::LocationInfoFilter() :
	Filter(std::make_unique<LocationInfoFilterPrivate>())
{
}

LocationInfoFilter::~LocationInfoFilter() {}

void LocationInfoFilter::setOption(  const LogString& option,
	const LogString& value)
{
	LogLog::warn(option + LOG4CXX_STR(":") + value);
	if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("ACCEPTONMATCH"), LOG4CXX_STR("acceptonmatch")))
	{
		priv->acceptOnMatch = OptionConverter::toBoolean(value, priv->acceptOnMatch);
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("OPERATOR"), LOG4CXX_STR("operator")))
	{
		priv->mustMatchAll = StringHelper::equalsIgnoreCase(value, LOG4CXX_STR("AND"), LOG4CXX_STR("and")) ? true : false;
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("LINENUMBER"), LOG4CXX_STR("linenumber")))
	{
		priv->lineNumber = OptionConverter::toInt(value, -1);
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("METHOD"), LOG4CXX_STR("method")))
	{
		LOG4CXX_ENCODE_CHAR(sName, value);
		priv->methodName = sName;
	}
}

Filter::FilterDecision LocationInfoFilter::decide(
	const log4cxx::spi::LoggingEventPtr& event) const
{
	if (priv->lineNumber == -1 &&
			priv->methodName.empty())
	{
		return Filter::NEUTRAL;
	}

	if (event->getLocationInformation().getLineNumber() == -1 ||
			event->getLocationInformation().getMethodName().compare(LocationInfo::NA_METHOD) == 0){
		return Filter::NEUTRAL;
	}

	bool matched = false;
	bool matchLineNumber = priv->lineNumber == event->getLocationInformation().getLineNumber();
	bool matchMethodName = priv->methodName.compare(event->getLocationInformation().getMethodName()) == 0;

	if(priv->mustMatchAll){
		matched = matchLineNumber && matchMethodName;
	}else{
		matched = matchLineNumber || matchMethodName;
	}

	if (priv->acceptOnMatch)
	{
		return matched ? Filter::ACCEPT : Filter::NEUTRAL;
	}
	else
	{
		return matched ? Filter::DENY : Filter::NEUTRAL;
	}
}

void LocationInfoFilter::setAcceptOnMatch(bool acceptOnMatch1)
{
	priv->acceptOnMatch = acceptOnMatch1;
}

bool LocationInfoFilter::getAcceptOnMatch() const
{
	return priv->acceptOnMatch;
}

bool LocationInfoFilter::getMustMatchAll() const
{
	return priv->mustMatchAll;
}

void LocationInfoFilter::setMustMatchAll(bool mustMatchAll1)
{
	priv->mustMatchAll = mustMatchAll1;
}

void LocationInfoFilter::setLineNumber(int lineNum){
	priv->lineNumber = lineNum;
}

void LocationInfoFilter::setMethodName(const LogString& methodName){
	LOG4CXX_ENCODE_CHAR(sName, methodName);
	priv->methodName = sName;
}
