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

#ifndef _LOG4CXX_HELPERS_SYNCHRONIZED_H
#define _LOG4CXX_HELPERS_SYNCHRONIZED_H
#include <log4cxx/log4cxx.h>

extern "C" {
	typedef struct apr_thread_mutex_t apr_thread_mutex_t;
}

namespace log4cxx
{
namespace helpers
{
class Mutex;

/** utility class for objects multi-thread synchronization.*/
class LOG4CXX_EXPORT synchronized
{
	public:
		synchronized(const Mutex& mutex);
		synchronized(apr_thread_mutex_t* mutex);
		~synchronized();


	private:
		void* mutex;
		//  prevent use of copy and assignment
		synchronized(const synchronized&);
		synchronized& operator=(const synchronized&);
};
}
}

#if defined(RW_MUTEX)

namespace log4cxx
{
namespace helpers
{
class RWMutex;

// utility class for objects multi-thread synchronization.
class LOG4CXX_EXPORT synchronized_read
{
	public:
		synchronized_read(const RWMutex& mutex);
		~synchronized_read();


	private:
		const RWMutex& mutex;
		//  prevent use of copy and assignment
		synchronized_read(const synchronized_read&);
		synchronized_read& operator=(const synchronized_read&);
};
}
}

namespace log4cxx
{
namespace helpers
{
class RWMutex;

// utility class for objects multi-thread synchronization.
class LOG4CXX_EXPORT synchronized_write
{
	public:
		synchronized_write(const RWMutex& mutex);
		~synchronized_write();


	private:
		const RWMutex& mutex;
		//  prevent use of copy and assignment
		synchronized_write(const synchronized_write&);
		synchronized_write& operator=(const synchronized_write&);
};
}
}

#define LOCK_R synchronized_read
#define LOCK_W synchronized_write

#else

#define LOCK_R synchronized
#define LOCK_W synchronized

#endif // RW_MUTEX

#endif //_LOG4CXX_HELPERS_SYNCHRONIZED_H
