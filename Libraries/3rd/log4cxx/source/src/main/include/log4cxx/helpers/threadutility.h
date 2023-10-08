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

#ifndef _LOG4CXX_THREADUTILITY_H
#define _LOG4CXX_THREADUTILITY_H

#include <thread>
#include <functional>
#include <memory>

#include "log4cxx/logstring.h"

namespace log4cxx
{
namespace helpers
{

/**
 * A function that will be called before a thread is started.  This can
 * be used to (for example) block all of the signals in the thread, so
 * that when the thread is created it will have a correct signal mask.
 */
typedef std::function<void()> ThreadStartPre;

/**
 * Called when a new thread has started.  This can be used to set
 * parameters for the thread in a platform-specific manner.
 *
 * @param threadName The name of the thread
 * @param threadId The ID of the thread as reported by std::thread::get_id
 * @param nativeHandle The native handle of the thread, as reported by
 * std::thread::native_handle
 */
typedef std::function<void( LogString threadName,
	std::thread::id threadId,
	std::thread::native_handle_type nativeHandle )> ThreadStarted;

/**
 * Called after a thread has started. This can be used to (for example)
 * unblock the signals in the thread.
 */
typedef std::function<void()> ThreadStartPost;

enum class ThreadConfigurationType
{
	NoConfiguration,
	BlockSignalsOnly,
	NameThreadOnly,
	BlockSignalsAndNameThread,
};

class ThreadUtility;
LOG4CXX_PTR_DEF(ThreadUtility);

class LOG4CXX_EXPORT ThreadUtility
{
	private:
		ThreadUtility();

		log4cxx::helpers::ThreadStartPre preStartFunction();
		log4cxx::helpers::ThreadStarted threadStartedFunction();
		log4cxx::helpers::ThreadStartPost postStartFunction();

		LOG4CXX_DECLARE_PRIVATE_MEMBER_PTR(priv_data, m_priv)
	public:
		~ThreadUtility();

		static ThreadUtility* instance();

		/**
		 * Utility method for configuring the ThreadUtility in a standard
		 * configuration.
		 */
		static void configure( ThreadConfigurationType type );

		/**
		 * Configure the thread functions that log4cxx will use.
		 * Note that setting any of these parameters to nullptr is valid,
		 * and simply results in the callback not being called.
		 */
		void configureFuncs( ThreadStartPre pre_start,
			ThreadStarted started,
			ThreadStartPost post_start );

		/**
		 * A pre-start thread function that blocks signals to the new thread
		 * (if the system has pthreads).  If the system does not have pthreads,
		 * does nothing.
		 */
		void preThreadBlockSignals();

		/**
		 * A thread_started function that names the thread using the appropriate
		 * system call.
		 */
		void threadStartedNameThread(LogString threadName,
			std::thread::id thread_id,
			std::thread::native_handle_type native_handle);

		/**
		 * A post-start thread function that unblocks signals that preThreadBlockSignals
		 * blocked before starting the thread.  If the system does not have pthreads,
		 * does nothing.
		 */
		void postThreadUnblockSignals();

		/**
		 * Start a thread
		 */
		template<class Function, class... Args>
		std::thread createThread(LogString name,
			Function&& f,
			Args&& ... args)
		{
			log4cxx::helpers::ThreadStartPre pre_start = preStartFunction();
			log4cxx::helpers::ThreadStarted thread_start = threadStartedFunction();
			log4cxx::helpers::ThreadStartPost post_start = postStartFunction();

			if ( pre_start )
			{
				pre_start();
			}

			std::thread t( f, args... );

			if ( thread_start )
			{
				thread_start( name,
					t.get_id(),
					t.native_handle() );
			}

			if ( post_start )
			{
				post_start();
			}

			return t;
		}
};

} /* namespace helpers */
} /* namespace log4cxx */

#endif
