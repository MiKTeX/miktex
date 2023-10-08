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

#ifndef _LOG4CXX_LOG_MANAGER_H
#define _LOG4CXX_LOG_MANAGER_H

#include <log4cxx/logstring.h>
#include <vector>
#include <log4cxx/spi/repositoryselector.h>

namespace log4cxx
{
class Logger;
typedef std::shared_ptr<Logger> LoggerPtr;
typedef std::vector<LoggerPtr> LoggerList;

namespace spi
{
class LoggerFactory;
typedef std::shared_ptr<LoggerFactory> LoggerFactoryPtr;
}

/**
* Use the <code>LogManager</code> class to retreive Logger
* instances or to operate on the current
* {@link log4cxx::spi::LoggerRepository LoggerRepository}.
* When the <code>LogManager</code> class is loaded
* into memory the default initialization procedure is inititated.
    */
class LOG4CXX_EXPORT LogManager
{
	private:
		static void* guard;
		static spi::RepositorySelectorPtr getRepositorySelector();

	public:
		/**
		Use \c selector to source the {@link spi::LoggerRepository LoggerRepository}, but only if the correct
		\c guard is passed as parameter.

		<p>Initally the guard is null.  If the guard is
		<code>null</code>, then invoking this method sets the logger
		factory and the guard. Following invocations will throw a
		{@link helpers::IllegalArgumentException IllegalArgumentException},
		unless the previously set \c guard is passed as the second
		parameter.

		<p>This allows a high-level component to set the
		{@link spi::RepositorySelector RepositorySelector}
		used by the LogManager.
		*/

		static void setRepositorySelector(spi::RepositorySelectorPtr selector,
			void* guard);

		static spi::LoggerRepositoryPtr getLoggerRepository();

		/**
		Retrieve the root logger from the {@link spi::LoggerRepository LoggerRepository}.

		Calls {@link spi::LoggerRepository::ensureIsConfigured ensureIsConfigured} passing {@link DefaultConfigurator::configure} to ensure
		the repository is configured.
		*/
		static LoggerPtr getRootLogger();

		/**
		Retrieve the \c name Logger instance from the
		{@link spi::LoggerRepository LoggerRepository}
		using DefaultLoggerFactory to create it if required.

		Calls {@link spi::LoggerRepository::ensureIsConfigured ensureIsConfigured} passing {@link DefaultConfigurator::configure} to ensure
		the repository is configured.
		*/
		static LoggerPtr getLogger(const std::string& name);

		/**
		Retrieve the \c name Logger instance from the
		{@link spi::LoggerRepository LoggerRepository}
		using \c factory to create it if required.

		Calls {@link spi::LoggerRepository::ensureIsConfigured ensureIsConfigured} passing {@link DefaultConfigurator::configure} to ensure
		the repository is configured.
		*/
		static LoggerPtr getLogger(const std::string& name,
			const spi::LoggerFactoryPtr& factory);
		/**
		 Does the logger \c name exist in the hierarchy?
		 */
		static LoggerPtr exists(const std::string& name);
#if LOG4CXX_WCHAR_T_API
		/**
		Retrieve the \c name Logger instance from the
		{@link spi::LoggerRepository LoggerRepository}
		using DefaultLoggerFactory to create it if required.

		Calls {@link spi::LoggerRepository::ensureIsConfigured ensureIsConfigured} passing {@link DefaultConfigurator::configure} to ensure
		the repository is configured.
		*/
		static LoggerPtr getLogger(const std::wstring& name);
		/**
		Retrieve the \c name Logger instance from the
		{@link spi::LoggerRepository LoggerRepository}
		using \c factory to create it if required.

		Calls {@link spi::LoggerRepository::ensureIsConfigured ensureIsConfigured} passing {@link DefaultConfigurator::configure} to ensure
		the repository is configured.
		*/
		static LoggerPtr getLogger(const std::wstring& name,
			const spi::LoggerFactoryPtr& factory);
		/**
		 Does the logger \c name exist in the hierarchy?
		 */
		static LoggerPtr exists(const std::wstring& name);
#endif
#if LOG4CXX_UNICHAR_API
		/**
		Retrieve the \c name Logger instance from the
		{@link spi::LoggerRepository LoggerRepository}
		using DefaultLoggerFactory to create it if required.

		Calls {@link spi::LoggerRepository::ensureIsConfigured ensureIsConfigured}
		passing {@link DefaultConfigurator::configure} to ensure
		the repository is configured.
		*/
		static LoggerPtr getLogger(const std::basic_string<UniChar>& name);
		/**
		Retrieve the \c name Logger instance from the
		{@link spi::LoggerRepository LoggerRepository}
		using \c factory to create it if required.

		Calls {@link spi::LoggerRepository::ensureIsConfigured ensureIsConfigured}
		passing {@link DefaultConfigurator::configure} to ensure
		the repository is configured.
		*/
		static LoggerPtr getLogger(const std::basic_string<UniChar>& name,
			const spi::LoggerFactoryPtr& factory);
		/**
		 Does the logger \c name exist in the hierarchy?
		 */
		static LoggerPtr exists(const std::basic_string<UniChar>& name);
#endif
#if LOG4CXX_CFSTRING_API
		/**
		Retrieve the \c name Logger instance from the
		{@link spi::LoggerRepository LoggerRepository}
		using DefaultLoggerFactory to create it if required.

		Calls {@link spi::LoggerRepository::ensureIsConfigured ensureIsConfigured}
		passing {@link DefaultConfigurator::configure} to ensure
		the repository is configured.
		*/
		static LoggerPtr getLogger(const CFStringRef& name);
		/**
		Retrieve the \c name Logger instance from the
		{@link spi::LoggerRepository LoggerRepository}
		using \c factory to create it if required.

		Calls {@link spi::LoggerRepository::ensureIsConfigured ensureIsConfigured}
		passing {@link DefaultConfigurator::configure} to ensure
		the repository is configured.
		*/
		static LoggerPtr getLogger(const CFStringRef& name,
			const spi::LoggerFactoryPtr& factory);
		/**
		 Does the logger \c name exist in the hierarchy?
		 */
		static LoggerPtr exists(const CFStringRef& name);
#endif


		/**
		Retrieve the \c name Logger instance from the
		{@link spi::LoggerRepository LoggerRepository}
		using DefaultLoggerFactory to create it if required.

		Calls {@link spi::LoggerRepository::ensureIsConfigured ensureIsConfigured}
		passing {@link DefaultConfigurator::configure} to ensure
		the repository is configured.
		*/
		static LoggerPtr getLoggerLS(const LogString& name);
		/**
		Retrieve the \c name Logger instance from the
		{@link spi::LoggerRepository LoggerRepository}
		using \c factory to create it if required.

		Calls {@link spi::LoggerRepository::ensureIsConfigured ensureIsConfigured}
		passing {@link DefaultConfigurator::configure} to ensure
		the repository is configured.
		*/
		static LoggerPtr getLoggerLS(const LogString& name,
			const spi::LoggerFactoryPtr& factory);

		/**
		 Does the logger \c name exist in the hierarchy?
		 */
		static LoggerPtr existsLS(const LogString& name);

		static LoggerList getCurrentLoggers();

		/**
		Safely close and remove all appenders in all loggers including
		the root logger.
		*/
		static void shutdown();

		/**
		Reset all values contained in this current
		{@link spi::LoggerRepository LoggerRepository}61
		to their default.
		*/
		static void resetConfiguration();
}; // class LogManager
}  // namespace log4cxx


#endif //_LOG4CXX_LOG_MANAGER_H
