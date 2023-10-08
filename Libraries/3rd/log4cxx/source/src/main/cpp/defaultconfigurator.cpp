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
#include <log4cxx/defaultconfigurator.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/spi/loggerrepository.h>
#include <log4cxx/file.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/propertyconfigurator.h>

using namespace log4cxx;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;

namespace
{
	LogString DefaultConfiguratorPath;
	int DefaultConfiguratorWatchSeconds = 0;
}

void DefaultConfigurator::setConfigurationFileName(const LogString& path)
{
	DefaultConfiguratorPath = path;
}


void DefaultConfigurator::setConfigurationWatchSeconds(int seconds)
{
	DefaultConfiguratorWatchSeconds = seconds;
}

static const int MillisecondsPerSecond = 1000;

void DefaultConfigurator::configure(LoggerRepositoryPtr repository)
{
	repository->setConfigured(true);
	const LogString configuratorClassName(getConfiguratorClass());

	LogString configurationFileName = DefaultConfiguratorPath;
	if (configurationFileName.empty())
		configurationFileName = getConfigurationFileName();
	Pool pool;
	File configuration;

	if (configurationFileName.empty())
	{
		LogString names[4] =
			{ LOG4CXX_STR("log4cxx.xml")
			, LOG4CXX_STR("log4cxx.properties")
			, LOG4CXX_STR("log4j.xml")
			, LOG4CXX_STR("log4j.properties")
			};

		for (int i = 0; i < 4; i++)
		{
			File candidate(names[i]);

			LogString debugMsg = LOG4CXX_STR("Checking file ");
			debugMsg.append(names[i]);
			LogLog::debug(debugMsg);
			if (candidate.exists(pool))
			{
				configuration = candidate;
				break;
			}
		}
	}
	else
	{
		configuration.setPath(configurationFileName);
	}

	if (configuration.exists(pool))
	{
		LogString msg(LOG4CXX_STR("Using configuration file ["));
		msg += configuration.getPath();
		msg += LOG4CXX_STR("] for automatic log4cxx configuration");
		LogLog::debug(msg);

		LoggerRepositoryPtr repo(repository);
		OptionConverter::selectAndConfigure(
			configuration,
			configuratorClassName,
			repo,
			0 < DefaultConfiguratorWatchSeconds
				? DefaultConfiguratorWatchSeconds * MillisecondsPerSecond
				: getConfigurationWatchDelay()
			);
	}
	else
	{
		if (configurationFileName.empty())
		{
			LogLog::debug(LOG4CXX_STR("Could not find default configuration file."));
		}
		else
		{
			LogString msg(LOG4CXX_STR("Could not find configuration file: ["));
			msg += configurationFileName;
			msg += LOG4CXX_STR("].");
			LogLog::debug(msg);
		}
	}

}

const LogString DefaultConfigurator::getConfiguratorClass()
{

	// Use automatic configration to configure the default hierarchy
	const LogString log4jConfiguratorClassName(
		OptionConverter::getSystemProperty(LOG4CXX_STR("log4j.configuratorClass"), LOG4CXX_STR("")));
	const LogString configuratorClassName(
		OptionConverter::getSystemProperty(LOG4CXX_STR("LOG4CXX_CONFIGURATOR_CLASS"),
			log4jConfiguratorClassName));
	return configuratorClassName;
}


const LogString DefaultConfigurator::getConfigurationFileName()
{
	static const LogString LOG4CXX_DEFAULT_CONFIGURATION_KEY(LOG4CXX_STR("LOG4CXX_CONFIGURATION"));
	static const LogString LOG4J_DEFAULT_CONFIGURATION_KEY(LOG4CXX_STR("log4j.configuration"));
	const LogString log4jConfigurationFileName(
		OptionConverter::getSystemProperty(LOG4J_DEFAULT_CONFIGURATION_KEY, LOG4CXX_STR("")));
	const LogString configurationFileName(
		OptionConverter::getSystemProperty(LOG4CXX_DEFAULT_CONFIGURATION_KEY,
			log4jConfigurationFileName));
	return configurationFileName;
}


int DefaultConfigurator::getConfigurationWatchDelay()
{
	static const LogString LOG4CXX_DEFAULT_CONFIGURATION_WATCH_KEY(LOG4CXX_STR("LOG4CXX_CONFIGURATION_WATCH_SECONDS"));
	LogString optionStr = OptionConverter::getSystemProperty(LOG4CXX_DEFAULT_CONFIGURATION_WATCH_KEY, LogString());
	int milliseconds = 0;
	if (!optionStr.empty())
		milliseconds = StringHelper::toInt(optionStr) * MillisecondsPerSecond;
	return milliseconds;
}

log4cxx::spi::ConfigurationStatus DefaultConfigurator::tryLoadFile(const LogString& filename){
	if(helpers::StringHelper::endsWith(filename, LOG4CXX_STR(".xml"))){
		return log4cxx::xml::DOMConfigurator::configure(filename);
	}else if(helpers::StringHelper::endsWith(filename, LOG4CXX_STR(".properties"))){
		return log4cxx::PropertyConfigurator::configure(filename);
	}

	return log4cxx::spi::ConfigurationStatus::NotConfigured;
}

std::tuple<log4cxx::spi::ConfigurationStatus,LogString>
DefaultConfigurator::configureFromFile(const std::vector<LogString>& directories, const std::vector<LogString>& filenames){
	using ResultType = std::tuple<log4cxx::spi::ConfigurationStatus, LogString>;
	log4cxx::helpers::Pool pool;

	for( LogString dir : directories ){
		for( LogString fname : filenames ){
			LogString canidate_str = dir + LOG4CXX_STR("/") + fname;
			File candidate(canidate_str);

			LogString debugMsg = LOG4CXX_STR("Checking file ");
			debugMsg.append(canidate_str);
			LogLog::debug(debugMsg);
			if (candidate.exists(pool))
			{
				log4cxx::spi::ConfigurationStatus configStatus = tryLoadFile(canidate_str);
				if( configStatus == log4cxx::spi::ConfigurationStatus::Configured ){
					return ResultType{configStatus, canidate_str};
				}
				LogLog::debug(LOG4CXX_STR("Unable to load file: trying next"));
			}
		}
	}

	return ResultType{log4cxx::spi::ConfigurationStatus::NotConfigured, LogString()};
}



