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

#ifndef _LOG4CXX_DEFAULT_CONFIGURATOR_H
#define _LOG4CXX_DEFAULT_CONFIGURATOR_H

#include <log4cxx/spi/configurator.h>
#include <log4cxx/spi/loggerrepository.h>
#include <tuple>

namespace log4cxx
{

/**
 *   Configures the repository from environmental settings and files.
*
*/
class LOG4CXX_EXPORT DefaultConfigurator
{
	private:
		DefaultConfigurator() {}

	public:
		/**
		Configure \c repository.

		If the configuration file name has not been provided by a call to setConfigurationFileName(),
		the environment variables "LOG4CXX_CONFIGURATION" and "log4j.configuration" are examined.
		Unless a custom configurator is specified using the
		"LOG4CXX_CONFIGURATOR_CLASS" or "log4j.configuratorClass"
		environment variable, the PropertyConfigurator will be used to
		configure log4cxx unless the file name ends with the ".xml"
		extension, in which case the DOMConfigurator will be used. If a
		custom configurator is specified, the environment variable should
		contain a fully qualified class name of a class that implements the
		Configurator interface.

		If the configuration file name is not found using any of the previous approaches,
		the current directory is examined for a file with extension ".xml" or ".properties"
		with a base name "log4cxx" or "log4j".

		If a positive number has been provided by a call to setConfigurationWatchSeconds()
		or the environment variables "LOG4CXX_CONFIGURATION_WATCH_SECONDS" contains a positive number
		a background thread is started that will periodically check for a change to the configuration file
		and apply any configuration changes found.
		*/
		static void configure(spi::LoggerRepositoryPtr repository);

		/**
		Make \c path the configuration file used by configure().
		*/
		static void setConfigurationFileName(const LogString& path);

		/**
		Make \c seconds the time a background thread will delay before checking
		for a change to the configuration file used by configure().
		*/
		static void setConfigurationWatchSeconds(int seconds);

		/**
		 * Configure Log4cxx from a file.  This method will attempt to load the configuration files in the
		 * directories given.
		 *
		 * For example, if we want a configuration file named 'myapp-logging.xml' with the default location
		 * for this file in /etc/myapp, but to have this overriden by a file in /usr/local/etc/myapp, we would
		 * call this function as follows:
		 *
		 * configureFromFile( { "/usr/local/etc/myapp", "/etc/myapp" }, { "myapp-logging.xml" );
		 *
		 * This will then search for files in the following order:
		 *
		 * <pre>
		 * /usr/local/etc/myapp/myapp-logging.xml
		 * /etc/myapp/myapp-logging.xml
		 * </pre>
		 *
		 * The status of configuring Log4cxx as well as the eventual filename used is returned.  If a file exists
		 * but it is not able to be used to configure Log4cxx, the next file in the list will be tried until
		 * a valid configuration file is found or the end of the list is reached.
		 *
		 * @param directories The directories to look in.
		 * @param filenamse The names of the files to look for
		 * @return The status of the configuration, and the filename loaded(if a file was found).
		 */
		static std::tuple<log4cxx::spi::ConfigurationStatus,LogString> configureFromFile(const std::vector<LogString>& directories,
																						 const std::vector<LogString>& filenames);

	private:
		static const LogString getConfigurationFileName();
		static const LogString getConfiguratorClass();
		static int getConfigurationWatchDelay();
		static log4cxx::spi::ConfigurationStatus tryLoadFile(const LogString& filename);

};	 // class DefaultConfigurator
}  // namespace log4cxx

#endif //_LOG4CXX_DEFAULT_CONFIGURATOR_H
