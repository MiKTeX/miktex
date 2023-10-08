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

#ifndef _LOG4CXX_HELPERS_APRINITIALIZER_H
#define _LOG4CXX_HELPERS_APRINITIALIZER_H

#ifndef LOG4CXX
	#error "aprinitializer.h should only be included by log4cxx implementation"
#endif

#include <log4cxx/helpers/object.h>
#include <list>
#include <log4cxx/helpers/date.h>

extern "C" {
	typedef struct apr_thread_mutex_t apr_thread_mutex_t;
	typedef struct apr_threadkey_t apr_threadkey_t;
	struct apr_pool_t;
}

#include <mutex>
#include <functional>

namespace log4cxx
{
namespace helpers
{
class FileWatchdog;

class APRInitializer
{
	public:
		static log4cxx_time_t initialize();
		static apr_pool_t* getRootPool();
		static apr_threadkey_t* getTlsKey();
		static bool isDestructed;

		/**
		 *  Register a FileWatchdog for deletion prior to termination.
		 *    FileWatchdog must be
		 *    allocated on heap and not deleted elsewhere.
		 */
		static void registerCleanup(FileWatchdog* watchdog);
		static void unregisterCleanup(FileWatchdog* watchdog);
		static void unregisterAll();
		/**
		 *  Store a single instance type ObjectPtr for deletion prior to termination
		 */
		template <class T> static void setUnique(const std::shared_ptr<T>& pObject)
		{
			getInstance().addObject(typeid(T).hash_code(), pObject);
		}
		/**
		 *  Fetch or add a single instance type ObjectPtr for deletion prior to termination
		 */
		template <class T> static std::shared_ptr<T> getOrAddUnique(std::function<ObjectPtr()> creator)
		{
			return cast<T>(getInstance().findOrAddObject(typeid(T).hash_code(), creator));
		}


	private: // Constructors
		APRInitializer();
		APRInitializer(const APRInitializer&) = delete;
		APRInitializer& operator=(const APRInitializer&) = delete;
	private: // Modifiers
		void addObject(size_t key, const ObjectPtr& pObject);
		const ObjectPtr& findOrAddObject(size_t key, std::function<ObjectPtr()> creator);
		void stopWatchDogs();
	private: // Attributes
		LOG4CXX_DECLARE_PRIVATE_MEMBER_PTR(APRInitializerPrivate, m_priv)
	private: // Class methods
		static APRInitializer& getInstance();

	public: // Destructor
		~APRInitializer();
};
} // namespace helpers
} // namespace log4cxx

#endif //_LOG4CXX_HELPERS_APRINITIALIZER_H
