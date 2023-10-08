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

#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/appenderskeleton.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/onlyonceerrorhandler.h>
#include <log4cxx/level.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/private/appenderskeleton_priv.h>
#include <mutex>

using namespace log4cxx;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(AppenderSkeleton)

AppenderSkeleton::AppenderSkeleton( std::unique_ptr<AppenderSkeletonPrivate> priv )
	:   m_priv(std::move(priv))
{

}

AppenderSkeleton::AppenderSkeleton()
	:   m_priv(std::make_unique<AppenderSkeletonPrivate>())
{

}

AppenderSkeleton::AppenderSkeleton(const LayoutPtr& layout1)
	:   m_priv(std::make_unique<AppenderSkeletonPrivate>())
{

}

AppenderSkeleton::~AppenderSkeleton() {}

void AppenderSkeleton::finalize()
{
	// An appender might be closed then garbage collected. There is no
	// point in closing twice.
	if (m_priv->closed)
	{
		return;
	}

	close();
}

void AppenderSkeleton::addFilter(const spi::FilterPtr newFilter)
{
	std::lock_guard<std::recursive_mutex> lock(m_priv->mutex);

	if (m_priv->headFilter == nullptr)
	{
		m_priv->headFilter = m_priv->tailFilter = newFilter;
	}
	else
	{
		m_priv->tailFilter->setNext(newFilter);
		m_priv->tailFilter = newFilter;
	}
}

void AppenderSkeleton::clearFilters()
{
	std::lock_guard<std::recursive_mutex> lock(m_priv->mutex);
	m_priv->headFilter = m_priv->tailFilter = nullptr;
}

bool AppenderSkeleton::isAsSevereAsThreshold(const LevelPtr& level) const
{
	return ((level == 0) || level->isGreaterOrEqual(m_priv->threshold));
}

void AppenderSkeleton::doAppend(const spi::LoggingEventPtr& event, Pool& pool1)
{
	std::lock_guard<std::recursive_mutex> lock(m_priv->mutex);

	doAppendImpl(event, pool1);
}

void AppenderSkeleton::doAppendImpl(const spi::LoggingEventPtr& event, Pool& pool1)
{
	if (m_priv->closed)
	{
		LogLog::error(((LogString) LOG4CXX_STR("Attempted to append to closed appender named ["))
			+ m_priv->name + LOG4CXX_STR("]."));
		return;
	}

	if (!isAsSevereAsThreshold(event->getLevel()))
	{
		return;
	}

	FilterPtr f = m_priv->headFilter;


	while (f != 0)
	{
		switch (f->decide(event))
		{
			case Filter::DENY:
				return;

			case Filter::ACCEPT:
				f = nullptr;
				break;

			case Filter::NEUTRAL:
				f = f->getNext();
		}
	}

	append(event, pool1);
}

void AppenderSkeleton::setErrorHandler(const spi::ErrorHandlerPtr errorHandler1)
{
	std::lock_guard<std::recursive_mutex> lock(m_priv->mutex);

	if (errorHandler1 == nullptr)
	{
		// We do not throw exception here since the cause is probably a
		// bad config file.
		LogLog::warn(LOG4CXX_STR("You have tried to set a null error-handler."));
	}
	else
	{
		m_priv->errorHandler = errorHandler1;
	}
}

void AppenderSkeleton::setThreshold(const LevelPtr& threshold1)
{
	std::lock_guard<std::recursive_mutex> lock(m_priv->mutex);
	m_priv->threshold = threshold1;
}

void AppenderSkeleton::setOption(const LogString& option,
	const LogString& value)
{
	if (StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("THRESHOLD"), LOG4CXX_STR("threshold")))
	{
		setThreshold(Level::toLevelLS(value));
	}
}

const spi::ErrorHandlerPtr AppenderSkeleton::getErrorHandler() const
{
	return m_priv->errorHandler;
}

spi::FilterPtr AppenderSkeleton::getFilter() const
{
	return m_priv->headFilter;
}

const spi::FilterPtr AppenderSkeleton::getFirstFilter() const
{
	return m_priv->headFilter;
}

LayoutPtr AppenderSkeleton::getLayout() const
{
	return m_priv->layout;
}

LogString AppenderSkeleton::getName() const
{
	return m_priv->name;
}

const LevelPtr AppenderSkeleton::getThreshold() const
{
	return m_priv->threshold;
}

void AppenderSkeleton::setLayout(const LayoutPtr layout1)
{
	m_priv->layout = layout1;
}

void AppenderSkeleton::setName(const LogString& name1)
{
	m_priv->name.assign(name1);
}
