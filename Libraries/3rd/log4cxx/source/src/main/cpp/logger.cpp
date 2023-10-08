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
#include <log4cxx/logger.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/spi/loggerfactory.h>
#include <log4cxx/appender.h>
#include <log4cxx/level.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/hierarchy.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/appenderattachableimpl.h>
#include <log4cxx/helpers/exception.h>
#if !defined(LOG4CXX)
	#define LOG4CXX 1
#endif
#include <log4cxx/private/log4cxx_private.h>
#include <log4cxx/helpers/aprinitializer.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;

struct Logger::LoggerPrivate
{
	LoggerPrivate(Pool& p, const LogString& name1):
		name(name1),
		repositoryRaw(0),
		aai(p),
		additive(true) {}

	/**
	The name of this logger.
	*/
	LogString name;

	/**
	The assigned level of this logger.  The
	<code>level</code> variable need not be assigned a value in
	which case it is inherited form the hierarchy.  */
	LevelPtr level;

	/**
	The parent of this logger. All loggers have at least one
	ancestor which is the root logger. */
	LoggerPtr parent;

	/** The resourceBundle for localized messages.

	@see setResourceBundle, getResourceBundle
	*/
	helpers::ResourceBundlePtr resourceBundle;


	// Loggers need to know what Hierarchy they are in
	log4cxx::spi::LoggerRepository* repositoryRaw;

	helpers::AppenderAttachableImpl aai;

	/** Additivity is set to true by default, that is children inherit
	        the appenders of their ancestors by default. If this variable is
	        set to <code>false</code> then the appenders found in the
	        ancestors of this logger are not used. However, the children
	        of this logger will inherit its appenders, unless the children
	        have their additivity flag set to <code>false</code> too. See
	        the user manual for more details. */
	bool additive;
};

IMPLEMENT_LOG4CXX_OBJECT(Logger)

Logger::Logger(Pool& p, const LogString& name1)
	: m_priv(std::make_unique<LoggerPrivate>(p, name1))
	, m_threshold(0)
{
}

Logger::~Logger()
{
}

void Logger::addAppender(const AppenderPtr newAppender)
{
	m_priv->aai.addAppender(newAppender);
	if (auto rep = getHierarchy())
	{
		rep->fireAddAppenderEvent(this, newAppender.get());
	}
}

void Logger::reconfigure( const std::vector<AppenderPtr>& appenders, bool additive1 )
{
	m_priv->additive = additive1;

	m_priv->aai.removeAllAppenders();

	for ( std::vector<AppenderPtr>::const_iterator it = appenders.cbegin();
		it != appenders.cend();
		it++ )
	{
		m_priv->aai.addAppender( *it );

		if (auto rep = getHierarchy())
		{
			rep->fireAddAppenderEvent(this, it->get());
		}
	}
}

void Logger::callAppenders(const spi::LoggingEventPtr& event, Pool& p) const
{
	int writes = 0;

	for (const Logger* logger = this;
		logger != 0;
		logger = logger->m_priv->parent.get())
	{
		writes += logger->m_priv->aai.appendLoopOnAppenders(event, p);

		if (!logger->m_priv->additive)
		{
			break;
		}
	}

	auto rep = getHierarchy();

	if (writes == 0 && rep)
	{
		rep->emitNoAppenderWarning(const_cast<Logger*>(this));
	}
}

void Logger::closeNestedAppenders()
{
	AppenderList appenders = getAllAppenders();

	for (AppenderList::iterator it = appenders.begin(); it != appenders.end(); ++it)
	{
		(*it)->close();
	}
}


void Logger::forcedLog(const LevelPtr& level1, const std::string& message,
	const LocationInfo& location) const
{
	if (!getHierarchy()) // Has removeHierarchy() been called?
		return;
	Pool p;
	LOG4CXX_DECODE_CHAR(msg, message);
	auto event = std::make_shared<LoggingEvent>(m_priv->name, level1, msg, location);
	callAppenders(event, p);
}


void Logger::forcedLog(const LevelPtr& level1, const std::string& message) const
{
	if (!getHierarchy()) // Has removeHierarchy() been called?
		return;
	Pool p;
	LOG4CXX_DECODE_CHAR(msg, message);
	auto event = std::make_shared<LoggingEvent>(m_priv->name, level1, msg,
			LocationInfo::getLocationUnavailable());
	callAppenders(event, p);
}

void Logger::forcedLogLS(const LevelPtr& level1, const LogString& message,
	const LocationInfo& location) const
{
	if (!getHierarchy()) // Has removeHierarchy() been called?
		return;
	Pool p;
	auto event = std::make_shared<LoggingEvent>(m_priv->name, level1, message, location);
	callAppenders(event, p);
}


bool Logger::getAdditivity() const
{
	return m_priv->additive;
}

AppenderList Logger::getAllAppenders() const
{
	return m_priv->aai.getAllAppenders();
}

AppenderPtr Logger::getAppender(const LogString& name1) const
{
	return m_priv->aai.getAppender(name1);
}

const LevelPtr& Logger::getEffectiveLevel() const
{
	for (const Logger* l = this; l != 0; l = l->m_priv->parent.get())
	{
		if (l->m_priv->level != 0)
		{
			return l->m_priv->level;
		}
	}

	throw NullPointerException(LOG4CXX_STR("No level specified for logger or ancestors."));
#if LOG4CXX_RETURN_AFTER_THROW
	return m_priv->level;
#endif
}

LoggerRepository* Logger::getLoggerRepository() const
{
	return m_priv->repositoryRaw;
}

LoggerRepository* Logger::getHierarchy() const
{
	return m_priv->repositoryRaw;
}

ResourceBundlePtr Logger::getResourceBundle() const
{
	for (const Logger* l = this; l != 0; l = l->m_priv->parent.get())
	{
		if (l->m_priv->resourceBundle != 0)
		{
			return l->m_priv->resourceBundle;
		}
	}

	// It might be the case that there is no resource bundle
	return 0;
}


LogString Logger::getResourceBundleString(const LogString& key) const
{
	ResourceBundlePtr rb = getResourceBundle();

	// This is one of the rare cases where we can use logging in order
	// to report errors from within log4j.
	if (rb == 0)
	{
		return LogString();
	}
	else
	{
		try
		{
			return rb->getString(key);
		}
		catch (MissingResourceException&)
		{
			logLS(Level::getError(), LOG4CXX_STR("No resource is associated with key \"") +
				key + LOG4CXX_STR("\"."), LocationInfo::getLocationUnavailable());

			return LogString();
		}
	}
}


LoggerPtr Logger::getParent() const
{
	return m_priv->parent;
}

const LevelPtr& Logger::getLevel() const
{
	return m_priv->level;
}

bool Logger::isAttached(const AppenderPtr appender) const
{
	return m_priv->aai.isAttached(appender);
}

bool Logger::isTraceEnabled() const
{
	auto rep = getHierarchy();

	if (!rep || rep->isDisabled(Level::TRACE_INT))
	{
		return false;
	}

	return getEffectiveLevel()->toInt() <= Level::TRACE_INT;
}

bool Logger::isDebugEnabled() const
{
	auto rep = getHierarchy();

	if (!rep || rep->isDisabled(Level::DEBUG_INT))
	{
		return false;
	}

	return getEffectiveLevel()->toInt() <= Level::DEBUG_INT;
}

bool Logger::isEnabledFor(const LevelPtr& level1) const
{
	auto rep = getHierarchy();

	if (!rep || rep->isDisabled(level1->toInt()))
	{
		return false;
	}

	return level1->isGreaterOrEqual(getEffectiveLevel());
}


bool Logger::isInfoEnabled() const
{
	auto rep = getHierarchy();

	if (!rep || rep->isDisabled(Level::INFO_INT))
	{
		return false;
	}

	return getEffectiveLevel()->toInt() <= Level::INFO_INT;
}

bool Logger::isErrorEnabled() const
{
	auto rep = getHierarchy();

	if (!rep || rep->isDisabled(Level::ERROR_INT))
	{
		return false;
	}

	return getEffectiveLevel()->toInt() <= Level::ERROR_INT;
}

bool Logger::isWarnEnabled() const
{
	auto rep = getHierarchy();

	if (!rep || rep->isDisabled(Level::WARN_INT))
	{
		return false;
	}

	return getEffectiveLevel()->toInt() <= Level::WARN_INT;
}

bool Logger::isFatalEnabled() const
{
	auto rep = getHierarchy();

	if (!rep || rep->isDisabled(Level::FATAL_INT))
	{
		return false;
	}

	return getEffectiveLevel()->toInt() <= Level::FATAL_INT;
}

/*void Logger::l7dlog(const LevelPtr& level, const String& key,
                        const char* file, int line)
{
	auto rep = getHierarchy();

        if (!rep || rep->isDisabled(level->level))
        {
                return;
        }

        if (level->isGreaterOrEqual(getEffectiveLevel()))
        {
                String msg = getResourceBundleString(key);

                // if message corresponding to 'key' could not be found in the
                // resource bundle, then default to 'key'.
                if (msg.empty())
                {
                        msg = key;
                }

                forcedLog(FQCN, level, msg, file, line);
        }
}*/



void Logger::l7dlog(const LevelPtr& level1, const LogString& key,
	const LocationInfo& location, const std::vector<LogString>& params) const
{
	auto rep = getHierarchy();

	if (!rep || rep->isDisabled(level1->toInt()))
	{
		return;
	}

	if (level1->isGreaterOrEqual(getEffectiveLevel()))
	{
		LogString pattern = getResourceBundleString(key);
		LogString msg;

		if (pattern.empty())
		{
			msg = key;
		}
		else
		{
			msg = StringHelper::format(pattern, params);
		}

		forcedLogLS(level1, msg, location);
	}
}

void Logger::l7dlog(const LevelPtr& level1, const std::string& key,
	const LocationInfo& location) const
{
	LOG4CXX_DECODE_CHAR(lkey, key);

	std::vector<LogString> values(0);
	l7dlog(level1, lkey, location, values);
}

void Logger::l7dlog(const LevelPtr& level1, const std::string& key,
	const LocationInfo& location, const std::string& val1) const
{
	LOG4CXX_DECODE_CHAR(lkey, key);
	LOG4CXX_DECODE_CHAR(lval1, val1);

	std::vector<LogString> values(1);
	values[0] = lval1;
	l7dlog(level1, lkey, location, values);
}

void Logger::l7dlog(const LevelPtr& level1, const std::string& key,
	const LocationInfo& location,
	const std::string& val1, const std::string& val2) const
{
	LOG4CXX_DECODE_CHAR(lkey, key);
	LOG4CXX_DECODE_CHAR(lval1, val1);
	LOG4CXX_DECODE_CHAR(lval2, val2);

	std::vector<LogString> values(2);
	values[0] = lval1;
	values[1] = lval2;
	l7dlog(level1, lkey, location, values);
}

void Logger::l7dlog(const LevelPtr& level1, const std::string& key,
	const LocationInfo& location,
	const std::string& val1, const std::string& val2, const std::string& val3) const
{
	LOG4CXX_DECODE_CHAR(lkey, key);
	LOG4CXX_DECODE_CHAR(lval1, val1);
	LOG4CXX_DECODE_CHAR(lval2, val2);
	LOG4CXX_DECODE_CHAR(lval3, val3);

	std::vector<LogString> values(3);
	values[0] = lval1;
	values[1] = lval2;
	values[2] = lval3;
	l7dlog(level1, lkey, location, values);
}

void Logger::removeAllAppenders()
{
	m_priv->aai.removeAllAppenders();
}

void Logger::removeAppender(const AppenderPtr appender)
{
	m_priv->aai.removeAppender(appender);
}

void Logger::removeAppender(const LogString& name1)
{
	m_priv->aai.removeAppender(name1);
}

void Logger::removeHierarchy()
{
	m_priv->repositoryRaw = 0;
}

void Logger::setAdditivity(bool additive1)
{
	m_priv->additive = additive1;
}

void Logger::setHierarchy(spi::LoggerRepository* repository1)
{
	m_priv->repositoryRaw = repository1;
}

void Logger::setParent(LoggerPtr parentLogger)
{
	m_priv->parent = parentLogger;
	updateThreshold();
}

void Logger::setLevel(const LevelPtr level1)
{
	m_priv->level = level1;
	updateThreshold();
	if (auto rep = dynamic_cast<Hierarchy*>(getHierarchy()))
		rep->updateChildren(this);
}

void Logger::updateThreshold()
{
	m_threshold = getEffectiveLevel()->toInt();
}

const LogString& Logger::getName() const
{
	return m_priv->name;
}

LoggerPtr Logger::getLogger(const std::string& name)
{
	return LogManager::getLogger(name);
}


LoggerPtr Logger::getLogger(const char* const name)
{
	return LogManager::getLogger(name);
}

void Logger::setResourceBundle(const helpers::ResourceBundlePtr& bundle)
{
	m_priv->resourceBundle = bundle;
}

LoggerPtr Logger::getRootLogger()
{
	return LogManager::getRootLogger();
}

LoggerPtr Logger::getLoggerLS(const LogString& name,
	const spi::LoggerFactoryPtr& factory)
{
	return LogManager::getLoggerLS(name, factory);
}

void Logger::getName(std::string& rv) const
{
	Transcoder::encode(m_priv->name, rv);
}


void Logger::trace(const std::string& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isTraceEnabled())
	{
		forcedLog(log4cxx::Level::getTrace(), msg, location);
	}
}


void Logger::trace(const std::string& msg) const
{
	if (isTraceEnabled())
	{
		forcedLog(log4cxx::Level::getTrace(), msg);
	}
}

void Logger::debug(const std::string& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isDebugEnabled())
	{
		forcedLog(log4cxx::Level::getDebug(), msg, location);
	}
}

void Logger::debug(const std::string& msg) const
{
	if (isDebugEnabled())
	{
		forcedLog(log4cxx::Level::getDebug(), msg);
	}
}


void Logger::error(const std::string& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isErrorEnabled())
	{
		forcedLog(log4cxx::Level::getError(), msg, location);
	}
}


void Logger::error(const std::string& msg) const
{
	if (isErrorEnabled())
	{
		forcedLog(log4cxx::Level::getError(), msg);
	}
}

void Logger::fatal(const std::string& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isFatalEnabled())
	{
		forcedLog(log4cxx::Level::getFatal(), msg, location);
	}
}

void Logger::fatal(const std::string& msg) const
{
	if (isFatalEnabled())
	{
		forcedLog(log4cxx::Level::getFatal(), msg);
	}
}

void Logger::info(const std::string& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isInfoEnabled())
	{
		forcedLog(log4cxx::Level::getInfo(), msg, location);
	}
}

void Logger::info(const std::string& msg) const
{
	if (isInfoEnabled())
	{
		forcedLog(log4cxx::Level::getInfo(), msg);
	}
}

void Logger::log(const LevelPtr& level1, const std::string& message,
	const log4cxx::spi::LocationInfo& location) const
{
	if (isEnabledFor(level1))
	{
		forcedLog(level1, message, location);
	}
}

void Logger::log(const LevelPtr& level1, const std::string& message) const
{
	if (isEnabledFor(level1))
	{
		forcedLog(level1, message);
	}
}

void Logger::logLS(const LevelPtr& level1, const LogString& message,
	const log4cxx::spi::LocationInfo& location) const
{
	if (isEnabledFor(level1))
	{
		forcedLogLS(level1, message, location);
	}
}

void Logger::warn(const std::string& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isWarnEnabled())
	{
		forcedLog(log4cxx::Level::getWarn(), msg, location);
	}
}

void Logger::warn(const std::string& msg) const
{
	if (isWarnEnabled())
	{
		forcedLog(log4cxx::Level::getWarn(), msg);
	}
}

LoggerPtr Logger::getLoggerLS(const LogString& name)
{
	return LogManager::getLoggerLS(name);
}




#if LOG4CXX_WCHAR_T_API
void Logger::forcedLog(const LevelPtr& level1, const std::wstring& message,
	const LocationInfo& location) const
{
	if (!getHierarchy()) // Has removeHierarchy() been called?
		return;
	Pool p;
	LOG4CXX_DECODE_WCHAR(msg, message);
	auto event = std::make_shared<LoggingEvent>(m_priv->name, level1, msg, location);
	callAppenders(event, p);
}

void Logger::forcedLog(const LevelPtr& level1, const std::wstring& message) const
{
	if (!getHierarchy()) // Has removeHierarchy() been called?
		return;
	Pool p;
	LOG4CXX_DECODE_WCHAR(msg, message);
	auto event = std::make_shared<LoggingEvent>(m_priv->name, level1, msg,
			LocationInfo::getLocationUnavailable());
	callAppenders(event, p);
}

void Logger::getName(std::wstring& rv) const
{
	Transcoder::encode(m_priv->name, rv);
}

LoggerPtr Logger::getLogger(const std::wstring& name)
{
	return LogManager::getLogger(name);
}

LoggerPtr Logger::getLogger(const wchar_t* const name)
{
	return LogManager::getLogger(name);
}

void Logger::trace(const std::wstring& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isTraceEnabled())
	{
		forcedLog(log4cxx::Level::getTrace(), msg, location);
	}
}


void Logger::trace(const std::wstring& msg) const
{
	if (isTraceEnabled())
	{
		forcedLog(log4cxx::Level::getTrace(), msg);
	}
}

void Logger::debug(const std::wstring& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isDebugEnabled())
	{
		forcedLog(log4cxx::Level::getDebug(), msg, location);
	}
}

void Logger::debug(const std::wstring& msg) const
{
	if (isDebugEnabled())
	{
		forcedLog(log4cxx::Level::getDebug(), msg);
	}
}

void Logger::error(const std::wstring& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isErrorEnabled())
	{
		forcedLog(log4cxx::Level::getError(), msg, location);
	}
}

void Logger::error(const std::wstring& msg) const
{
	if (isErrorEnabled())
	{
		forcedLog(log4cxx::Level::getError(), msg);
	}
}

void Logger::fatal(const std::wstring& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isFatalEnabled())
	{
		forcedLog(log4cxx::Level::getFatal(), msg, location);
	}
}

void Logger::fatal(const std::wstring& msg) const
{
	if (isFatalEnabled())
	{
		forcedLog(log4cxx::Level::getFatal(), msg);
	}
}

void Logger::info(const std::wstring& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isInfoEnabled())
	{
		forcedLog(log4cxx::Level::getInfo(), msg, location);
	}
}

void Logger::info(const std::wstring& msg) const
{
	if (isInfoEnabled())
	{
		forcedLog(log4cxx::Level::getInfo(), msg);
	}
}

void Logger::log(const LevelPtr& level1, const std::wstring& message,
	const log4cxx::spi::LocationInfo& location) const
{
	if (isEnabledFor(level1))
	{
		forcedLog(level1, message, location);
	}
}

void Logger::log(const LevelPtr& level1, const std::wstring& message) const
{
	if (isEnabledFor(level1))
	{
		forcedLog(level1, message);
	}
}

void Logger::warn(const std::wstring& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isWarnEnabled())
	{
		forcedLog(log4cxx::Level::getWarn(), msg, location);
	}
}

void Logger::warn(const std::wstring& msg) const
{
	if (isWarnEnabled())
	{
		forcedLog(log4cxx::Level::getWarn(), msg);
	}
}

#endif


#if LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API
void Logger::forcedLog(const LevelPtr& level1, const std::basic_string<UniChar>& message,
	const LocationInfo& location) const
{
	if (!getHierarchy()) // Has removeHierarchy() been called?
		return;
	Pool p;
	LOG4CXX_DECODE_UNICHAR(msg, message);
	auto event = std::make_shared<LoggingEvent>(m_priv->name, level1, msg, location);
	callAppenders(event, p);
}

void Logger::forcedLog(const LevelPtr& level1, const std::basic_string<UniChar>& message) const
{
	if (!getHierarchy()) // Has removeHierarchy() been called?
		return;
	Pool p;
	LOG4CXX_DECODE_UNICHAR(msg, message);
	auto event = std::make_shared<LoggingEvent>(m_priv->name, level1, msg,
			LocationInfo::getLocationUnavailable());
	callAppenders(event, p);
}
#endif

#if LOG4CXX_UNICHAR_API
void Logger::getName(std::basic_string<UniChar>& rv) const
{
	Transcoder::encode(m_priv->name, rv);
}

LoggerPtr Logger::getLogger(const std::basic_string<UniChar>& name)
{
	return LogManager::getLogger(name);
}

void Logger::trace(const std::basic_string<UniChar>& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isTraceEnabled())
	{
		forcedLog(log4cxx::Level::getTrace(), msg, location);
	}
}


void Logger::trace(const std::basic_string<UniChar>& msg) const
{
	if (isTraceEnabled())
	{
		forcedLog(log4cxx::Level::getTrace(), msg);
	}
}

void Logger::debug(const std::basic_string<UniChar>& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isDebugEnabled())
	{
		forcedLog(log4cxx::Level::getDebug(), msg, location);
	}
}

void Logger::debug(const std::basic_string<UniChar>& msg) const
{
	if (isDebugEnabled())
	{
		forcedLog(log4cxx::Level::getDebug(), msg);
	}
}

void Logger::error(const std::basic_string<UniChar>& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isErrorEnabled())
	{
		forcedLog(log4cxx::Level::getError(), msg, location);
	}
}

void Logger::error(const std::basic_string<UniChar>& msg) const
{
	if (isErrorEnabled())
	{
		forcedLog(log4cxx::Level::getError(), msg);
	}
}

void Logger::fatal(const std::basic_string<UniChar>& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isFatalEnabled())
	{
		forcedLog(log4cxx::Level::getFatal(), msg, location);
	}
}

void Logger::fatal(const std::basic_string<UniChar>& msg) const
{
	if (isFatalEnabled())
	{
		forcedLog(log4cxx::Level::getFatal(), msg);
	}
}

void Logger::info(const std::basic_string<UniChar>& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isInfoEnabled())
	{
		forcedLog(log4cxx::Level::getInfo(), msg, location);
	}
}

void Logger::info(const std::basic_string<UniChar>& msg) const
{
	if (isInfoEnabled())
	{
		forcedLog(log4cxx::Level::getInfo(), msg);
	}
}

void Logger::log(const LevelPtr& level1, const std::basic_string<UniChar>& message,
	const log4cxx::spi::LocationInfo& location) const
{
	if (isEnabledFor(level1))
	{
		forcedLog(level1, message, location);
	}
}

void Logger::log(const LevelPtr& level1, const std::basic_string<UniChar>& message) const
{
	if (isEnabledFor(level1))
	{
		forcedLog(level1, message);
	}
}

void Logger::warn(const std::basic_string<UniChar>& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isWarnEnabled())
	{
		forcedLog(log4cxx::Level::getWarn(), msg, location);
	}
}

void Logger::warn(const std::basic_string<UniChar>& msg) const
{
	if (isWarnEnabled())
	{
		forcedLog(log4cxx::Level::getWarn(), msg);
	}
}

#endif


#if LOG4CXX_CFSTRING_API
void Logger::forcedLog(const LevelPtr& level1, const CFStringRef& message,
	const LocationInfo& location) const
{
	if (!getHierarchy()) // Has removeHierarchy() been called?
		return;
	Pool p;
	LOG4CXX_DECODE_CFSTRING(msg, message);
	auto event = std::make_shared<LoggingEvent>(name, level1, msg, location);
	callAppenders(event, p);
}

void Logger::forcedLog(const LevelPtr& level1, const CFStringRef& message) const
{
	if (!getHierarchy()) // Has removeHierarchy() been called?
		return;
	Pool p;
	LOG4CXX_DECODE_CFSTRING(msg, message);
	auto event = std::make_shared<LoggingEvent>(name, level1, msg,
			LocationInfo::getLocationUnavailable());
	callAppenders(event, p);
}

void Logger::getName(CFStringRef& rv) const
{
	rv = Transcoder::encode(name);
}

LoggerPtr Logger::getLogger(const CFStringRef& name)
{
	return LogManager::getLogger(name);
}

void Logger::trace(const CFStringRef& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isTraceEnabled())
	{
		forcedLog(log4cxx::Level::getTrace(), msg, location);
	}
}


void Logger::trace(const CFStringRef& msg) const
{
	if (isTraceEnabled())
	{
		forcedLog(log4cxx::Level::getTrace(), msg);
	}
}

void Logger::debug(const CFStringRef& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isDebugEnabled())
	{
		forcedLog(log4cxx::Level::getDebug(), msg, location);
	}
}

void Logger::debug(const CFStringRef& msg) const
{
	if (isDebugEnabled())
	{
		forcedLog(log4cxx::Level::getDebug(), msg);
	}
}

void Logger::error(const CFStringRef& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isErrorEnabled())
	{
		forcedLog(log4cxx::Level::getError(), msg, location);
	}
}

void Logger::error(const CFStringRef& msg) const
{
	if (isErrorEnabled())
	{
		forcedLog(log4cxx::Level::getError(), msg);
	}
}

void Logger::fatal(const CFStringRef& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isFatalEnabled())
	{
		forcedLog(log4cxx::Level::getFatal(), msg, location);
	}
}

void Logger::fatal(const CFStringRef& msg) const
{
	if (isFatalEnabled())
	{
		forcedLog(log4cxx::Level::getFatal(), msg);
	}
}

void Logger::info(const CFStringRef& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isInfoEnabled())
	{
		forcedLog(log4cxx::Level::getInfo(), msg, location);
	}
}

void Logger::info(const CFStringRef& msg) const
{
	if (isInfoEnabled())
	{
		forcedLog(log4cxx::Level::getInfo(), msg);
	}
}

void Logger::log(const LevelPtr& level1, const CFStringRef& message,
	const log4cxx::spi::LocationInfo& location) const
{
	if (isEnabledFor(level1))
	{
		forcedLog(level1, message, location);
	}
}

void Logger::log(const LevelPtr& level1, const CFStringRef& message) const
{
	if (isEnabledFor(level1))
	{
		forcedLog(level1, message);
	}
}

void Logger::warn(const CFStringRef& msg, const log4cxx::spi::LocationInfo& location) const
{
	if (isWarnEnabled())
	{
		forcedLog(log4cxx::Level::getWarn(), msg, location);
	}
}

void Logger::warn(const CFStringRef& msg) const
{
	if (isWarnEnabled())
	{
		forcedLog(log4cxx::Level::getWarn(), msg);
	}
}

#endif


