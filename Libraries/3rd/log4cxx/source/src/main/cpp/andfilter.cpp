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

#include <log4cxx/filter/andfilter.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/private/filter_priv.h>

using namespace log4cxx;
using namespace log4cxx::filter;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;

#define priv static_cast<AndFilterPrivate*>(m_priv.get())

struct AndFilter::AndFilterPrivate : public FilterPrivate
{
	AndFilterPrivate() : FilterPrivate(),
		headFilter(),
		tailFilter(),
		acceptOnMatch(true) {}

	log4cxx::spi::FilterPtr headFilter;
	log4cxx::spi::FilterPtr tailFilter;
	bool acceptOnMatch;
};

IMPLEMENT_LOG4CXX_OBJECT(AndFilter)

AndFilter::AndFilter()
	: Filter( std::make_unique<AndFilterPrivate>() )
{
}

AndFilter::~AndFilter() {}

void AndFilter::addFilter(const FilterPtr& filter)
{
	if (priv->headFilter == NULL)
	{
		priv->headFilter = filter;
		priv->tailFilter = filter;
	}
	else
	{
		priv->tailFilter->setNext(filter);
	}
}


void AndFilter::setAcceptOnMatch(bool newValue)
{
	priv->acceptOnMatch = newValue;
}

Filter::FilterDecision AndFilter::decide(
	const spi::LoggingEventPtr& event) const
{
	bool accepted = true;
	FilterPtr f(priv->headFilter);

	while (f != NULL)
	{
		accepted = accepted && (Filter::ACCEPT == f->decide(event));
		f = f->getNext();
	}

	if (accepted)
	{
		if (priv->acceptOnMatch)
		{
			return Filter::ACCEPT;
		}

		return Filter::DENY;
	}

	return Filter::NEUTRAL;
}

