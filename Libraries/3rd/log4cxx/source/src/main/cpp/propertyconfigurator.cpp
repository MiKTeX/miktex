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
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/spi/loggerfactory.h>
#include <log4cxx/helpers/properties.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/level.h>
#include <log4cxx/defaultloggerfactory.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/appender.h>
#include <log4cxx/logger.h>
#include <log4cxx/layout.h>
#include <log4cxx/config/propertysetter.h>
#include <log4cxx/spi/loggerrepository.h>
#include <log4cxx/helpers/stringtokenizer.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/fileinputstream.h>
#include <log4cxx/helpers/loader.h>
#include <log4cxx/helpers/threadutility.h>
#include <log4cxx/rolling/rollingfileappender.h>

#define LOG4CXX 1
#include <log4cxx/helpers/aprinitializer.h>
#include <apr.h>


using namespace log4cxx;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;
using namespace log4cxx::config;
using namespace log4cxx::rolling;

#if APR_HAS_THREADS
#include <log4cxx/helpers/filewatchdog.h>
namespace log4cxx
{
class PropertyWatchdog  : public FileWatchdog
{
	public:
		PropertyWatchdog(const File& filename) : FileWatchdog(filename)
		{
		}

		/**
		Call PropertyConfigurator#doConfigure(const String& configFileName,
		const spi::LoggerRepositoryPtr& hierarchy) with the
		<code>filename</code> to reconfigure log4cxx.
		*/
		void doOnChange()
		{
			PropertyConfigurator().doConfigure(file(),
				LogManager::getLoggerRepository());
		}
};
}

PropertyWatchdog* PropertyConfigurator::pdog = NULL;

#endif

IMPLEMENT_LOG4CXX_OBJECT(PropertyConfigurator)

PropertyConfigurator::PropertyConfigurator()
	: registry(new std::map<LogString, AppenderPtr>()), loggerFactory(new DefaultLoggerFactory())
{
}

PropertyConfigurator::~PropertyConfigurator()
{
	delete registry;
}

spi::ConfigurationStatus PropertyConfigurator::doConfigure(const File& configFileName,
	spi::LoggerRepositoryPtr hierarchy)
{
	hierarchy->setConfigured(true);

	Properties props;

	try
	{
		InputStreamPtr inputStream = InputStreamPtr( new FileInputStream(configFileName) );
		props.load(inputStream);
	}
	catch (const IOException& ex)
	{
		LOG4CXX_DECODE_CHAR(lsMsg, ex.what());
		LogLog::error(((LogString) LOG4CXX_STR("Could not read configuration file ["))
			+ configFileName.getPath() + LOG4CXX_STR("]: ") + lsMsg);
		return spi::ConfigurationStatus::NotConfigured;
	}

	try
	{
		LogString debugMsg = LOG4CXX_STR("Loading configuration file [")
				+ configFileName.getPath() + LOG4CXX_STR("].");
		LogLog::debug(debugMsg);
		return doConfigure(props, hierarchy);
	}
	catch (const std::exception& ex)
	{
		LogLog::error(((LogString) LOG4CXX_STR("Could not parse configuration file ["))
			+ configFileName.getPath() + LOG4CXX_STR("]: "), ex);
	}

	return spi::ConfigurationStatus::NotConfigured;
}

spi::ConfigurationStatus PropertyConfigurator::configure(const File& configFilename)
{
	return PropertyConfigurator().doConfigure(configFilename, LogManager::getLoggerRepository());
}

spi::ConfigurationStatus PropertyConfigurator::configure(helpers::Properties& properties)
{
	return PropertyConfigurator().doConfigure(properties, LogManager::getLoggerRepository());
}

#if APR_HAS_THREADS
spi::ConfigurationStatus PropertyConfigurator::configureAndWatch(const File& configFilename)
{
	return configureAndWatch(configFilename, FileWatchdog::DEFAULT_DELAY);
}



spi::ConfigurationStatus PropertyConfigurator::configureAndWatch(
	const File& configFilename, long delay)
{
	if (pdog)
	{
		APRInitializer::unregisterCleanup(pdog);
		delete pdog;
	}

	spi::ConfigurationStatus stat = PropertyConfigurator().doConfigure(configFilename, LogManager::getLoggerRepository());

	pdog = new PropertyWatchdog(configFilename);
	APRInitializer::registerCleanup(pdog);
	pdog->setDelay(delay);
	pdog->start();

	return stat;
}
#endif

spi::ConfigurationStatus PropertyConfigurator::doConfigure(helpers::Properties& properties,
	spi::LoggerRepositoryPtr hierarchy)
{
	hierarchy->setConfigured(true);

	static const LogString DEBUG_KEY(LOG4CXX_STR("log4j.debug"));
	LogString value(properties.getProperty(DEBUG_KEY));

	if (!value.empty())
	{
		LogLog::setInternalDebugging(OptionConverter::toBoolean(value, true));
	}

	static const LogString THRESHOLD_PREFIX(LOG4CXX_STR("log4j.threshold"));
	LogString thresholdStr =
		OptionConverter::findAndSubst(THRESHOLD_PREFIX, properties);

	if (!thresholdStr.empty())
	{
		hierarchy->setThreshold(OptionConverter::toLevel(thresholdStr, Level::getAll()));
		LogLog::debug(((LogString) LOG4CXX_STR("Hierarchy threshold set to ["))
			+ hierarchy->getThreshold()->toString()
			+ LOG4CXX_STR("]."));
	}

	LogString threadConfigurationValue(properties.getProperty(LOG4CXX_STR("log4j.threadConfiguration")));

	if ( threadConfigurationValue == LOG4CXX_STR("NoConfiguration") )
	{
		helpers::ThreadUtility::configure( ThreadConfigurationType::NoConfiguration );
	}
	else if ( threadConfigurationValue == LOG4CXX_STR("BlockSignalsOnly") )
	{
		helpers::ThreadUtility::configure( ThreadConfigurationType::BlockSignalsOnly );
	}
	else if ( threadConfigurationValue == LOG4CXX_STR("NameThreadOnly") )
	{
		helpers::ThreadUtility::configure( ThreadConfigurationType::NameThreadOnly );
	}
	else if ( threadConfigurationValue == LOG4CXX_STR("BlockSignalsAndNameThread") )
	{
		helpers::ThreadUtility::configure( ThreadConfigurationType::BlockSignalsAndNameThread );
	}

	configureRootLogger(properties, hierarchy);
	configureLoggerFactory(properties);
	parseCatsAndRenderers(properties, hierarchy);

	LogLog::debug(LOG4CXX_STR("Finished configuring."));

	// We don't want to hold references to appenders preventing their
	// destruction.
	registry->clear();

	return spi::ConfigurationStatus::Configured;
}

void PropertyConfigurator::configureLoggerFactory(helpers::Properties& props)
{
	static const LogString LOGGER_FACTORY_KEY(LOG4CXX_STR("log4j.loggerFactory"));

	LogString factoryClassName =
		OptionConverter::findAndSubst(LOGGER_FACTORY_KEY, props);

	if (!factoryClassName.empty())
	{
		LogString msg(LOG4CXX_STR("Setting logger factory to ["));
		msg += factoryClassName;
		msg += LOG4CXX_STR("].");
		LogLog::debug(msg);
		std::shared_ptr<Object> instance = std::shared_ptr<Object>(
				Loader::loadClass(factoryClassName).newInstance() );

		loggerFactory = log4cxx::cast<LoggerFactory>( instance );
		static const LogString FACTORY_PREFIX(LOG4CXX_STR("log4j.factory."));
		Pool p;
		PropertySetter::setProperties(loggerFactory, props, FACTORY_PREFIX, p);
	}
}

void PropertyConfigurator::configureRootLogger(helpers::Properties& props,
	spi::LoggerRepositoryPtr& hierarchy)
{
	static const LogString ROOT_CATEGORY_PREFIX(LOG4CXX_STR("log4j.rootCategory"));
	static const LogString ROOT_LOGGER_PREFIX(LOG4CXX_STR("log4j.rootLogger"));



	LogString effectiveFrefix(ROOT_LOGGER_PREFIX);
	LogString value = OptionConverter::findAndSubst(ROOT_LOGGER_PREFIX, props);

	if (value.empty())
	{
		value = OptionConverter::findAndSubst(ROOT_CATEGORY_PREFIX, props);
		effectiveFrefix = ROOT_CATEGORY_PREFIX;
	}

	if (value.empty())
	{
		LogLog::debug(LOG4CXX_STR("Could not find root logger information. Is this OK?"));
	}
	else
	{
		LoggerPtr root = hierarchy->getRootLogger();

		static const LogString INTERNAL_ROOT_NAME(LOG4CXX_STR("root"));
		parseLogger(props, root, effectiveFrefix, INTERNAL_ROOT_NAME, value, true);
	}
}

void PropertyConfigurator::parseCatsAndRenderers(helpers::Properties& props,
	spi::LoggerRepositoryPtr& hierarchy)
{
	static const LogString CATEGORY_PREFIX(LOG4CXX_STR("log4j.category."));
	static const LogString LOGGER_PREFIX(LOG4CXX_STR("log4j.logger."));

	std::vector<LogString> names = props.propertyNames();

	std::vector<LogString>::iterator it = names.begin();
	std::vector<LogString>::iterator itEnd = names.end();

	while (it != itEnd)
	{
		LogString key = *it++;

		if (key.find(CATEGORY_PREFIX) == 0 || key.find(LOGGER_PREFIX) == 0)
		{
			LogString loggerName;

			if (key.find(CATEGORY_PREFIX) == 0)
			{
				loggerName = key.substr(CATEGORY_PREFIX.length());
			}
			else if (key.find(LOGGER_PREFIX) == 0)
			{
				loggerName = key.substr(LOGGER_PREFIX.length());
			}

			LogString value = OptionConverter::findAndSubst(key, props);
			LoggerPtr logger = hierarchy->getLogger(loggerName, loggerFactory);

			bool additivity = parseAdditivityForLogger(props, logger, loggerName);
			parseLogger(props, logger, key, loggerName, value, additivity);

		}
	}
}

bool PropertyConfigurator::parseAdditivityForLogger(helpers::Properties& props,
	LoggerPtr& cat, const LogString& loggerName)
{

	static const LogString ADDITIVITY_PREFIX(LOG4CXX_STR("log4j.additivity."));



	LogString value(OptionConverter::findAndSubst(ADDITIVITY_PREFIX + loggerName, props));
	LogLog::debug((LogString) LOG4CXX_STR("Handling ") + ADDITIVITY_PREFIX
		+ loggerName + LOG4CXX_STR("=[") +  value + LOG4CXX_STR("]"));

	// touch additivity only if necessary
	if (!value.empty())
	{
		bool additivity = OptionConverter::toBoolean(value, true);
		LogLog::debug(((LogString) LOG4CXX_STR("Setting additivity for \""))
			+ loggerName
			+ ((additivity) ?  LOG4CXX_STR("\" to true") :
				LOG4CXX_STR("\" to false")));

		return additivity;
	}

	return true;
}

/**
        This method must work for the root logger as well.
*/
void PropertyConfigurator::parseLogger(
	helpers::Properties& props, LoggerPtr& logger, const LogString& /* optionKey */,
	const LogString& loggerName, const LogString& value, bool additivity)
{
	LogLog::debug(((LogString) LOG4CXX_STR("Parsing for ["))
		+ loggerName
		+ LOG4CXX_STR("] with value=[")
		+ value + LOG4CXX_STR("]."));

	// We must skip over ',' but not white space
	StringTokenizer st(value, LOG4CXX_STR(","));

	// If value is not in the form ", appender.." or "", then we should set
	// the level of the logger.
	if (!(value.find(LOG4CXX_STR(",")) == 0 || value.empty()))
	{
		// just to be on the safe side...
		if (!st.hasMoreTokens())
		{
			return;
		}

		LogString levelStr = st.nextToken();
		LogLog::debug((LogString) LOG4CXX_STR("Level token is [")
			+ levelStr +  LOG4CXX_STR("]."));


		// If the level value is inherited, set logger level value to
		// null. We also check that the user has not specified inherited for the
		// root logger.
		if (StringHelper::equalsIgnoreCase(levelStr, LOG4CXX_STR("INHERITED"), LOG4CXX_STR("inherited"))
			|| StringHelper::equalsIgnoreCase(levelStr, LOG4CXX_STR("NULL"), LOG4CXX_STR("null")))
		{
			static const LogString INTERNAL_ROOT_NAME(LOG4CXX_STR("root"));

			if (loggerName == INTERNAL_ROOT_NAME)
			{
				LogLog::warn(LOG4CXX_STR("The root logger cannot be set to null."));
			}
			else
			{
				logger->setLevel(0);
				LogLog::debug((LogString) LOG4CXX_STR("Logger ")
					+ loggerName + LOG4CXX_STR(" set to null"));
			}
		}
		else
		{
			logger->setLevel(OptionConverter::toLevel(levelStr, Level::getDebug()));

			LogLog::debug((LogString) LOG4CXX_STR("Logger ")
				+ loggerName + LOG4CXX_STR(" set to ")
				+ logger->getLevel()->toString());
		}

	}

	AppenderPtr appender;
	LogString appenderName;
	std::vector<AppenderPtr> newappenders;

	while (st.hasMoreTokens())
	{
		appenderName = StringHelper::trim(st.nextToken());

		if (appenderName.empty() || appenderName == LOG4CXX_STR(","))
		{
			continue;
		}

		LogLog::debug(LOG4CXX_STR("Parsing appender named ")
			+ appenderName + LOG4CXX_STR("\"."));
		appender = parseAppender(props, appenderName);

		if (appender != 0)
		{
			newappenders.push_back(appender);
		}
	}

	logger->reconfigure( newappenders, additivity );
}

AppenderPtr PropertyConfigurator::parseAppender(
	helpers::Properties& props, const LogString& appenderName)
{
	AppenderPtr appender = registryGet(appenderName);

	if (appender != 0)
	{
		LogLog::debug((LogString) LOG4CXX_STR("Appender \"")
			+ appenderName + LOG4CXX_STR("\" was already parsed."));

		return appender;
	}

	static const LogString APPENDER_PREFIX(LOG4CXX_STR("log4j.appender."));

	// Appender was not previously initialized.
	LogString prefix = APPENDER_PREFIX + appenderName;
	LogString layoutPrefix = prefix + LOG4CXX_STR(".layout");

	std::shared_ptr<Object> obj =
		OptionConverter::instantiateByKey(
			props, prefix, Appender::getStaticClass(), 0);
	appender = log4cxx::cast<Appender>( obj );

	// Map obsolete DailyRollingFileAppender property configuration
	if (!appender &&
		StringHelper::endsWith(OptionConverter::findAndSubst(prefix, props), LOG4CXX_STR("DailyRollingFileAppender")))
	{
		appender = std::make_shared<RollingFileAppender>();
		auto datePattern = OptionConverter::findAndSubst(prefix + LOG4CXX_STR(".datePattern"), props);
		if (!datePattern.empty())
			props.put(prefix + LOG4CXX_STR(".fileDatePattern"), datePattern);
	}

	if (!appender)
	{
		LogLog::error((LogString) LOG4CXX_STR("Could not instantiate appender named \"")
			+ appenderName + LOG4CXX_STR("\"."));
		return 0;
	}

	appender->setName(appenderName);

	if (appender->instanceof(OptionHandler::getStaticClass()))
	{
		Pool p;

		if (appender->requiresLayout())
		{
			LayoutPtr layout;
			std::shared_ptr<Object> obj =
				OptionConverter::instantiateByKey(
					props, layoutPrefix, Layout::getStaticClass(), 0);
			layout = log4cxx::cast<Layout>( obj );

			if (layout != 0)
			{
				appender->setLayout(layout);
				LogLog::debug((LogString) LOG4CXX_STR("Parsing layout options for \"")
					+ appenderName + LOG4CXX_STR("\"."));

				PropertySetter::setProperties(layout, props, layoutPrefix + LOG4CXX_STR("."), p);
				LogLog::debug((LogString) LOG4CXX_STR("End of parsing for \"")
					+ appenderName +  LOG4CXX_STR("\"."));
			}
		}

		RollingFileAppenderPtr rolling = log4cxx::cast<rolling::RollingFileAppender>(appender);
		if (rolling)
		{
			LogString rollingPolicyKey = prefix + LOG4CXX_STR(".rollingPolicy");
			if (!OptionConverter::findAndSubst(rollingPolicyKey, props).empty())
			{
				RollingPolicyPtr rollingPolicy;
				std::shared_ptr<Object> rolling_obj =
					OptionConverter::instantiateByKey(
						props, rollingPolicyKey, RollingPolicy::getStaticClass(), 0);
				rollingPolicy = log4cxx::cast<RollingPolicy>( rolling_obj );
				if(rollingPolicy)
				{
					rolling->setRollingPolicy(rollingPolicy);

					LogLog::debug((LogString) LOG4CXX_STR("Parsing rolling policy options for \"")
						+ appenderName + LOG4CXX_STR("\"."));
					PropertySetter::setProperties(rollingPolicy, props, rollingPolicyKey + LOG4CXX_STR("."), p);
				}
			}

			LogString triggeringPolicyKey = prefix + LOG4CXX_STR(".triggeringPolicy");
			if (!OptionConverter::findAndSubst(triggeringPolicyKey, props).empty())
			{
				TriggeringPolicyPtr triggeringPolicy;
				std::shared_ptr<Object> triggering_obj =
					OptionConverter::instantiateByKey(
						props, triggeringPolicyKey, TriggeringPolicy::getStaticClass(), 0);
				triggeringPolicy = log4cxx::cast<TriggeringPolicy>( triggering_obj );
				if(triggeringPolicy)
				{
					rolling->setTriggeringPolicy(triggeringPolicy);

					LogLog::debug((LogString) LOG4CXX_STR("Parsing triggering policy options for \"")
						+ appenderName + LOG4CXX_STR("\"."));
					PropertySetter::setProperties(triggeringPolicy, props, triggeringPolicyKey + LOG4CXX_STR("."), p);
				}
			}
		}

		PropertySetter::setProperties(appender, props, prefix + LOG4CXX_STR("."), p);
		LogLog::debug((LogString) LOG4CXX_STR("Parsed \"")
			+ appenderName + LOG4CXX_STR("\" options."));
	}

	registryPut(appender);

	return appender;
}

void PropertyConfigurator::registryPut(const AppenderPtr& appender)
{
	(*registry)[appender->getName()] = appender;
}

AppenderPtr PropertyConfigurator::registryGet(const LogString& name)
{
	return (*registry)[name];
}
