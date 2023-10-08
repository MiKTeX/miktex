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
#include <log4cxx/filter/mapfilter.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/private/filter_priv.h>

using namespace log4cxx;
using namespace log4cxx::filter;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;

#define priv static_cast<MapFilterPrivate*>(m_priv.get())

struct MapFilter::MapFilterPrivate : public FilterPrivate
{
	MapFilterPrivate() : FilterPrivate(),
		acceptOnMatch(true), mustMatchAll(false) {}

	bool    acceptOnMatch;
	bool    mustMatchAll; // true = AND; false = OR
	KeyVals keyVals;
};

IMPLEMENT_LOG4CXX_OBJECT(MapFilter)

MapFilter::MapFilter() : Filter(std::make_unique<MapFilterPrivate>())
{

}

MapFilter::~MapFilter() {}

void MapFilter::setOption(  const LogString& option,
	const LogString& value)
{
	if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("ACCEPTONMATCH"), LOG4CXX_STR("acceptonmatch")))
	{
		priv->acceptOnMatch = OptionConverter::toBoolean(value, priv->acceptOnMatch);
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("OPERATOR"), LOG4CXX_STR("operator")))
	{
		priv->mustMatchAll = StringHelper::equalsIgnoreCase(value, LOG4CXX_STR("AND"), LOG4CXX_STR("and")) ? true : false;
	}
	else if (!option.empty() && !value.empty())
	{
		priv->keyVals[option] = value;
	}
}

Filter::FilterDecision MapFilter::decide(
	const log4cxx::spi::LoggingEventPtr& event) const
{
	if (priv->keyVals.empty())
	{
		return Filter::NEUTRAL;
	}

	bool matched = true;

	for (KeyVals::const_iterator it = priv->keyVals.begin(); it != priv->keyVals.end(); ++it)
	{
		LogString curval;
		event->getMDC(it->first, curval);

		if (curval.empty() || curval != it->second)
		{
			matched = false;
		}
		else
		{
			matched = true;
		}

		if (priv->mustMatchAll != matched)
		{
			break;
		}
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

void MapFilter::setKeyValue(const LogString& strKey, const LogString& strValue)
{
	priv->keyVals[strKey] = strValue;
}

const LogString& MapFilter::getValue(const LogString& strKey) const
{
	static  const LogString                 empty;
	const KeyVals::const_iterator   it(priv->keyVals.find(strKey));

	return (it != priv->keyVals.end() ? it->second : empty);
}

void MapFilter::setAcceptOnMatch(bool acceptOnMatch1)
{
	priv->acceptOnMatch = acceptOnMatch1;
}

bool MapFilter::getAcceptOnMatch() const
{
	return priv->acceptOnMatch;
}

bool MapFilter::getMustMatchAll() const
{
	return priv->mustMatchAll;
}

void MapFilter::setMustMatchAll(bool mustMatchAll1)
{
	priv->mustMatchAll = mustMatchAll1;
}
