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
#include <log4cxx/rolling/filterbasedtriggeringpolicy.h>
#include <log4cxx/spi/filter.h>

using namespace log4cxx;
using namespace log4cxx::rolling;
using namespace log4cxx::spi;

IMPLEMENT_LOG4CXX_OBJECT(FilterBasedTriggeringPolicy)

struct FilterBasedTriggeringPolicy::FilterBasedTriggeringPolicyPrivate{

	/**
	 * The first filter in the filter chain. Set to <code>null</code> initially.
	 */
	log4cxx::spi::FilterPtr headFilter;

	/**
	 * The last filter in the filter chain.
	 */
	log4cxx::spi::FilterPtr tailFilter;
};

FilterBasedTriggeringPolicy::FilterBasedTriggeringPolicy() :
	m_priv(std::make_unique<FilterBasedTriggeringPolicyPrivate>())
{
}


FilterBasedTriggeringPolicy::~FilterBasedTriggeringPolicy()
{
}


bool FilterBasedTriggeringPolicy::isTriggeringEvent(
	Appender* /* appender */,
	const log4cxx::spi::LoggingEventPtr& event,
	const LogString& /* filename */,
	size_t /* fileLength */ )
{
	if (m_priv->headFilter == NULL)
	{
		return false;
	}

	for (log4cxx::spi::FilterPtr f = m_priv->headFilter; f != NULL; f = f->getNext())
	{
		switch (f->decide(event))
		{
			case Filter::DENY:
				return false;

			case Filter::ACCEPT:
				return true;

			case Filter::NEUTRAL:
				break;
		}
	}

	return true;
}

/**
 * Add a filter to end of the filter list.
 * @param newFilter filter to add to end of list.
 */
void FilterBasedTriggeringPolicy::addFilter(const log4cxx::spi::FilterPtr& newFilter)
{
	if (m_priv->headFilter == NULL)
	{
		m_priv->headFilter = newFilter;
		m_priv->tailFilter = newFilter;
	}
	else
	{
		m_priv->tailFilter->setNext(newFilter);
		m_priv->tailFilter = newFilter;
	}
}

void FilterBasedTriggeringPolicy::clearFilters()
{
	log4cxx::spi::FilterPtr empty;
	m_priv->headFilter = empty;
	m_priv->tailFilter = empty;
}

log4cxx::spi::FilterPtr& FilterBasedTriggeringPolicy::getFilter()
{
	return m_priv->headFilter;
}

/**
 *  Prepares the instance for use.
 */
void FilterBasedTriggeringPolicy::activateOptions(log4cxx::helpers::Pool& p)
{
	for (log4cxx::spi::FilterPtr f = m_priv->headFilter; f != NULL; f = f->getNext())
	{
		f->activateOptions(p);
	}
}

void FilterBasedTriggeringPolicy::setOption(const LogString& /* option */, const LogString& /* value */ )
{
}


