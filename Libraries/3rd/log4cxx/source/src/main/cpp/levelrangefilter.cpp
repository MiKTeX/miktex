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
#include <log4cxx/filter/levelrangefilter.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/level.h>
#include <log4cxx/private/filter_priv.h>

using namespace log4cxx;
using namespace log4cxx::filter;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;

#define priv static_cast<LevelRangeFilterPrivate*>(m_priv.get())

struct LevelRangeFilter::LevelRangeFilterPrivate : public FilterPrivate
{
	LevelRangeFilterPrivate() : acceptOnMatch(false), levelMin(Level::getAll()), levelMax(Level::getOff()) {}

	/**
	Do we return ACCEPT when a match occurs. Default is
	<code>false</code>, so that later filters get run by default
	*/
	bool acceptOnMatch;
	LevelPtr levelMin;
	LevelPtr levelMax;
};

IMPLEMENT_LOG4CXX_OBJECT(LevelRangeFilter)


LevelRangeFilter::LevelRangeFilter()
	: Filter(std::make_unique<LevelRangeFilterPrivate>())
{
}

LevelRangeFilter::~LevelRangeFilter() {}

void LevelRangeFilter::setOption(const LogString& option,
	const LogString& value)
{

	if (StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("LEVELMIN"), LOG4CXX_STR("levelmin")))
	{
		priv->levelMin = OptionConverter::toLevel(value, priv->levelMin);
	}
	else if (StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("LEVELMAX"), LOG4CXX_STR("levelmax")))
	{
		priv->levelMax = OptionConverter::toLevel(value, priv->levelMax);
	}
	else if (StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("ACCEPTONMATCH"), LOG4CXX_STR("acceptonmatch")))
	{
		priv->acceptOnMatch = OptionConverter::toBoolean(value, priv->acceptOnMatch);
	}
}

Filter::FilterDecision LevelRangeFilter::decide(
	const spi::LoggingEventPtr& event) const
{
	if (priv->levelMin != 0 && !event->getLevel()->isGreaterOrEqual(priv->levelMin))
	{
		// level of event is less than minimum
		return Filter::DENY;
	}

	if (priv->levelMax != 0 && event->getLevel()->toInt() > priv->levelMax->toInt())
	{
		// level of event is greater than maximum
		// Alas, there is no Level.isGreater method. and using
		// a combo of isGreaterOrEqual && !Equal seems worse than
		// checking the int values of the level objects..
		return Filter::DENY;
	}

	if (priv->acceptOnMatch)
	{
		// this filter set up to bypass later filters and always return
		// accept if level in range
		return Filter::ACCEPT;
	}
	else
	{
		// event is ok for this filter; allow later filters to have a look..
		return Filter::NEUTRAL;
	}
}

void LevelRangeFilter::setLevelMin(const LevelPtr& levelMin1)
{
	priv->levelMin = levelMin1;
}

const LevelPtr& LevelRangeFilter::getLevelMin() const
{
	return priv->levelMin;
}

void LevelRangeFilter::setLevelMax(const LevelPtr& levelMax1)
{
	priv->levelMax = levelMax1;
}

const LevelPtr& LevelRangeFilter::getLevelMax() const
{
	return priv->levelMax;
}

void LevelRangeFilter::setAcceptOnMatch(bool acceptOnMatch1)
{
	priv->acceptOnMatch = acceptOnMatch1;
}

bool LevelRangeFilter::getAcceptOnMatch() const
{
	return priv->acceptOnMatch;
}
