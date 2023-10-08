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
#include <log4cxx/spi/loggerfactory.h>
#include <log4cxx/hierarchy.h>
#include <log4cxx/defaultloggerfactory.h>
#include <log4cxx/logger.h>
#include <log4cxx/spi/hierarchyeventlistener.h>
#include <log4cxx/level.h>
#include <algorithm>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/appender.h>
#include <log4cxx/logstring.h>
#include <log4cxx/helpers/stringhelper.h>
#if !defined(LOG4CXX)
	#define LOG4CXX 1
#endif
#include <log4cxx/helpers/aprinitializer.h>
#include <log4cxx/spi/rootlogger.h>
#include <mutex>
#include <apr.h>
#include "assert.h"


using namespace log4cxx;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;


typedef std::map<LogString, LoggerPtr> LoggerMap;
typedef std::map<LogString, ProvisionNode> ProvisionNodeMap;

struct Hierarchy::HierarchyPrivate
{
	HierarchyPrivate()
		: configured(false)
		, emittedNoAppenderWarning(false)
		, emittedNoResourceBundleWarning(false)
		, thresholdInt(Level::ALL_INT)
	{
	}

	helpers::Pool pool;
	mutable std::mutex mutex;
	mutable std::mutex configuredMutex;
	bool configured;
	bool emittedNoAppenderWarning;
	bool emittedNoResourceBundleWarning;
	int thresholdInt;

	spi::HierarchyEventListenerList listeners;
	LoggerPtr root;
	LevelPtr threshold;
	LoggerMap loggers;
	ProvisionNodeMap provisionNodes;

	std::vector<AppenderPtr> allAppenders;
};

IMPLEMENT_LOG4CXX_OBJECT(Hierarchy)

Hierarchy::Hierarchy() :
	m_priv(std::make_unique<HierarchyPrivate>())
{
}

Hierarchy::~Hierarchy()
{
	std::unique_lock<std::mutex> lock(m_priv->mutex);
	for (auto& item : m_priv->loggers)
	{
		if (auto& pLogger = item.second)
		{
			pLogger->removeHierarchy();
			pLogger->removeAllAppenders();
		}
	}
	if (m_priv->root)
	{
		m_priv->root->removeHierarchy();
		m_priv->root->removeAllAppenders();
	}
}

void Hierarchy::addHierarchyEventListener(const spi::HierarchyEventListenerPtr& listener)
{
	std::unique_lock<std::mutex> lock(m_priv->mutex);

	if (std::find(m_priv->listeners.begin(), m_priv->listeners.end(), listener) != m_priv->listeners.end())
	{
		LogLog::warn(LOG4CXX_STR("Ignoring attempt to add an existent listener."));
	}
	else
	{
		m_priv->listeners.push_back(listener);
	}
}

void Hierarchy::clear()
{
	std::unique_lock<std::mutex> lock(m_priv->mutex);
	m_priv->loggers.clear();
}

void Hierarchy::emitNoAppenderWarning(const Logger* logger)
{
	bool emitWarning = false;
	{
		std::unique_lock<std::mutex> lock(m_priv->mutex);
		emitWarning = !m_priv->emittedNoAppenderWarning;
		m_priv->emittedNoAppenderWarning = true;
	}

	// No appender in hierarchy, warn user only once.
	if (emitWarning)
	{
		LogLog::warn(((LogString) LOG4CXX_STR("No appender could be found for logger ("))
			+ logger->getName() + LOG4CXX_STR(")."));
		LogLog::warn(LOG4CXX_STR("Please initialize the log4cxx system properly."));
	}
}


LoggerPtr Hierarchy::exists(const LogString& name)
{
	std::unique_lock<std::mutex> lock(m_priv->mutex);

	LoggerPtr logger;
	LoggerMap::iterator it = m_priv->loggers.find(name);

	if (it != m_priv->loggers.end())
	{
		logger = it->second;
	}


	return logger;
}

void Hierarchy::setThreshold(const LevelPtr& l)
{
	if (l != 0)
	{
		std::unique_lock<std::mutex> lock(m_priv->mutex);
		setThresholdInternal(l);
	}
}

void Hierarchy::setThreshold(const LogString& levelStr)
{
	LevelPtr l(Level::toLevelLS(levelStr, 0));

	if (l != 0)
	{
		setThreshold(l);
	}
	else
	{
		LogLog::warn(((LogString) LOG4CXX_STR("No level could be found named \""))
			+ levelStr + LOG4CXX_STR("\"."));
	}
}

void Hierarchy::setThresholdInternal(const LevelPtr& l)
{
	m_priv->thresholdInt = l->toInt();
	m_priv->threshold = l;

	if (m_priv->thresholdInt != Level::ALL_INT)
	{
		m_priv->configured = true;
	}
}

void Hierarchy::fireAddAppenderEvent(const Logger* logger, const Appender* appender)
{
	setConfigured(true);
	HierarchyEventListenerList clonedList;
	{
		std::unique_lock<std::mutex> lock(m_priv->mutex);
		clonedList = m_priv->listeners;
	}

	HierarchyEventListenerList::iterator it, itEnd = clonedList.end();
	HierarchyEventListenerPtr listener;

	for (it = clonedList.begin(); it != itEnd; it++)
	{
		listener = *it;
		listener->addAppenderEvent(logger, appender);
	}
}

void Hierarchy::fireRemoveAppenderEvent(const Logger* logger, const Appender* appender)

{
	HierarchyEventListenerList clonedList;
	{
		std::unique_lock<std::mutex> lock(m_priv->mutex);
		clonedList = m_priv->listeners;
	}
	HierarchyEventListenerList::iterator it, itEnd = clonedList.end();
	HierarchyEventListenerPtr listener;

	for (it = clonedList.begin(); it != itEnd; it++)
	{
		listener = *it;
		listener->removeAppenderEvent(logger, appender);
	}
}

LevelPtr Hierarchy::getThreshold() const
{
	return m_priv->threshold ? m_priv->threshold : Level::getAll();
}

LoggerPtr Hierarchy::getLogger(const LogString& name)
{
	static spi::LoggerFactoryPtr defaultFactory = std::make_shared<DefaultLoggerFactory>();
	return getLogger(name, defaultFactory);
}

LoggerPtr Hierarchy::getLogger(const LogString& name,
	const spi::LoggerFactoryPtr& factory)
{
	auto root = getRootLogger();
	std::unique_lock<std::mutex> lock(m_priv->mutex);

	LoggerMap::iterator it = m_priv->loggers.find(name);
	LoggerPtr result;

	if (it != m_priv->loggers.end())
	{
		result = it->second;
	}
	if (!result)
	{
		LoggerPtr logger(factory->makeNewLoggerInstance(m_priv->pool, name));
		logger->setHierarchy(this);
		m_priv->loggers.insert(LoggerMap::value_type(name, logger));

		ProvisionNodeMap::iterator it2 = m_priv->provisionNodes.find(name);

		if (it2 != m_priv->provisionNodes.end())
		{
			updateChildren(it2->second, logger);
			m_priv->provisionNodes.erase(it2);
		}

		updateParents(logger, root);
		result = logger;
	}
	return result;

}

LoggerList Hierarchy::getCurrentLoggers() const
{
	std::unique_lock<std::mutex> lock(m_priv->mutex);

	LoggerList v;
	for (auto& item : m_priv->loggers)
	{
		if (auto pLogger = item.second)
			v.push_back(pLogger);
	}
	return v;
}

LoggerPtr Hierarchy::getRootLogger() const
{
	std::unique_lock<std::mutex> lock(m_priv->mutex);
	if (!m_priv->root)
	{
		m_priv->root = std::make_shared<RootLogger>(m_priv->pool, Level::getDebug());
		m_priv->root->setHierarchy(const_cast<Hierarchy*>(this));
	}

	return m_priv->root;
}

bool Hierarchy::isDisabled(int level) const
{
	return m_priv->thresholdInt > level;
}

void Hierarchy::ensureIsConfigured(std::function<void()> configurator)
{
	std::unique_lock<std::mutex> lock(m_priv->configuredMutex);
	if (!m_priv->configured)
	{
		configurator();
		m_priv->configured = true;
	}
}

void Hierarchy::resetConfiguration()
{
	std::unique_lock<std::mutex> lock(m_priv->mutex);

	if (m_priv->root)
	{
		m_priv->root->setLevel(Level::getDebug());
		m_priv->root->setResourceBundle(0);
	}
	setThresholdInternal(Level::getAll());

	shutdownInternal();

	LoggerMap::const_iterator it, itEnd = m_priv->loggers.end();

	for (it = m_priv->loggers.begin(); it != itEnd; it++)
	{
		if (auto pLogger = it->second)
		{
			pLogger->setLevel(0);
			pLogger->setAdditivity(true);
			pLogger->setResourceBundle(0);
		}
	}

	//rendererMap.clear();
}

void Hierarchy::shutdown()
{
	std::unique_lock<std::mutex> lock(m_priv->mutex);

	shutdownInternal();
}

void Hierarchy::shutdownInternal()
{
	m_priv->configured = false;

	// begin by closing nested appenders
	if (m_priv->root)
		m_priv->root->closeNestedAppenders();

	LoggerMap::iterator it, itEnd = m_priv->loggers.end();

	for (it = m_priv->loggers.begin(); it != itEnd; it++)
	{
		if (auto pLogger = it->second)
			pLogger->closeNestedAppenders();
	}

	// then, remove all appenders
	if (m_priv->root)
		m_priv->root->removeAllAppenders();

	for (it = m_priv->loggers.begin(); it != itEnd; it++)
	{
		if (auto pLogger = it->second)
			pLogger->removeAllAppenders();
	}
}

void Hierarchy::updateParents(const LoggerPtr& logger, const LoggerPtr& root)
{
	const LogString name(logger->getName());
	size_t length = name.size();
	bool parentFound = false;


	// if name = "w.x.y.z", loop through "w.x.y", "w.x" and "w", but not "w.x.y.z"
	for (size_t i = name.find_last_of(0x2E /* '.' */, length - 1);
		(i != LogString::npos) && (i != 0);
		i = name.find_last_of(0x2E /* '.' */, i - 1))
	{
		LogString substr = name.substr(0, i);

		LoggerMap::iterator it = m_priv->loggers.find(substr);

		if (it != m_priv->loggers.end())
		{
			parentFound = true;
			logger->setParent( it->second );
			break; // no need to update the ancestors of the closest ancestor
		}
		else
		{
			ProvisionNodeMap::iterator it2 = m_priv->provisionNodes.find(substr);

			if (it2 != m_priv->provisionNodes.end())
			{
				it2->second.push_back(logger);
			}
			else
			{
				ProvisionNode node(1, logger);
				m_priv->provisionNodes.insert(
					ProvisionNodeMap::value_type(substr, node));
			}
		}
	}

	// If we could not find any existing parents, then link with root.
	if (!parentFound)
	{
		logger->setParent( root );
	}
}

void Hierarchy::updateChildren(ProvisionNode& pn, const LoggerPtr& logger)
{
	ProvisionNode::iterator it, itEnd = pn.end();

	for (it = pn.begin(); it != itEnd; it++)
	{
		LoggerPtr& l = *it;

		// Unless this child already points to a correct (lower) parent,
		// make logger.parent point to l.parent and l.parent to logger.
		if (!StringHelper::startsWith(l->getParent()->getName(), logger->getName()))
		{
			logger->setParent( l->getParent() );
			l->setParent( logger );
		}
	}
    
}

void Hierarchy::updateChildren(const Logger* parent)
{
	for (auto& item : m_priv->loggers)
	{
		for (auto l = item.second; l; l = l->getParent())
		{
			if (l->getParent().get() == parent)
			{
				item.second->updateThreshold();
				break;
			}
		}
	}
}

void Hierarchy::setConfigured(bool newValue)
{
	std::unique_lock<std::mutex> lock(m_priv->configuredMutex, std::try_to_lock);
	if (lock.owns_lock()) // Not being auto-configured?
		m_priv->configured = newValue;
}

bool Hierarchy::isConfigured()
{
	std::unique_lock<std::mutex> lock(m_priv->configuredMutex); // Blocks while auto-configuration is active
	return m_priv->configured;
}

HierarchyPtr Hierarchy::create()
{
	HierarchyPtr ret(new Hierarchy);
	return ret;
}

void Hierarchy::clearAppenders()
{
	m_priv->allAppenders.clear();
}

void Hierarchy::addAppender(AppenderPtr appender)
{
	m_priv->allAppenders.push_back(appender);
}
