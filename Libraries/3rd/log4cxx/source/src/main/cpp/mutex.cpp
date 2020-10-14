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
#include <log4cxx/helpers/exception.h>
#include <log4cxx/helpers/mutex.h>
#include <log4cxx/helpers/pool.h>
#include <apr_thread_mutex.h>
#include <apr_thread_rwlock.h>
#include <assert.h>
#if !defined(LOG4CXX)
	#define LOG4CXX 1
#endif
#include <log4cxx/helpers/aprinitializer.h>

#if defined(NON_BLOCKING)

	#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
		#include <windows.h>
	#else
		// POSIX
		#include <semaphore.h>
	#endif

#endif // NON_BLOCKING

using namespace log4cxx::helpers;
using namespace log4cxx;


Mutex::Mutex(Pool& p)
{
#if APR_HAS_THREADS
	apr_status_t stat = apr_thread_mutex_create(&mutex,
			APR_THREAD_MUTEX_NESTED, p.getAPRPool());

	if (stat != APR_SUCCESS)
	{
		throw MutexException(stat);
	}

#endif
}

Mutex::Mutex(apr_pool_t* p)
{
#if APR_HAS_THREADS
	apr_status_t stat = apr_thread_mutex_create(&mutex,
			APR_THREAD_MUTEX_NESTED, p);

	if (stat != APR_SUCCESS)
	{
		throw MutexException(stat);
	}

#endif
}


Mutex::~Mutex()
{
#if APR_HAS_THREADS

	// LOGCXX-322
	if (APRInitializer::isDestructed)
	{
		return;
	}

	apr_thread_mutex_destroy(mutex);
#endif
}

apr_thread_mutex_t* Mutex::getAPRMutex() const
{
	return mutex;
}

#if defined(RW_MUTEX)

RWMutex::RWMutex(Pool& p)
	: id((apr_os_thread_t) -1)
	, count(0)
{
#if APR_HAS_THREADS
	apr_status_t stat = apr_thread_rwlock_create(&mutex,
			p.getAPRPool());

	if (stat != APR_SUCCESS)
	{
		throw MutexException(stat);
	}

#endif
}

RWMutex::RWMutex(apr_pool_t* p)
	: id((apr_os_thread_t) -1)
	, count(0)
{
#if APR_HAS_THREADS
	apr_status_t stat = apr_thread_rwlock_create(&mutex, p);

	if (stat != APR_SUCCESS)
	{
		throw MutexException(stat);
	}

#endif
}


RWMutex::~RWMutex()
{
#if APR_HAS_THREADS
	apr_thread_rwlock_destroy(mutex);
#endif
}

void RWMutex::rdLock() const
{
#if APR_HAS_THREADS
	apr_status_t stat = apr_thread_rwlock_rdlock(mutex);
#endif
}

void RWMutex::rdUnlock() const
{
#if APR_HAS_THREADS
	apr_status_t stat = apr_thread_rwlock_unlock(mutex);
#endif
}

void RWMutex::wrLock() const
{
#if APR_HAS_THREADS
	apr_os_thread_t self = apr_os_thread_current();

	if (id == self)
	{
		++count;
	}
	else
	{
		apr_status_t stat = apr_thread_rwlock_wrlock(mutex);
		id = self;
		count = 1;
	}

#endif
}

void RWMutex::wrUnlock() const
{
#if APR_HAS_THREADS

	if (--count == 0)
	{
		id = (apr_os_thread_t) -1; // id_ = "not a thread"
		apr_status_t stat = apr_thread_rwlock_unlock(mutex);
	}
	else
	{
	}

#endif
}

#endif // RW_MUTEX

#if defined(NON_BLOCKING)

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)

namespace log4cxx
{
namespace helpers
{
struct SemaphoreImpl
{
	HANDLE semaphore;
};
}
}

static const LONG cMax = 10000; // arbitrary high value

Semaphore::Semaphore(log4cxx::helpers::Pool& p)
	: impl(nullptr)
{
#if APR_HAS_THREADS
	impl = (SemaphoreImpl*)p.palloc(sizeof(SemaphoreImpl));

	if (nullptr == impl)
	{
		throw MutexException(APR_ENOMEM);
	}

	impl->semaphore = CreateSemaphore(
			NULL,  // default security attributes
			0,     // initial count
			cMax,  // maximum count
			NULL); // unnamed semaphore

	if (impl->semaphore == NULL)
	{
		throw MutexException(APR_ENOSHMAVAIL);
	}

#endif
}

Semaphore::~Semaphore()
{
#if APR_HAS_THREADS

	if (impl && impl->semaphore)
	{
		CloseHandle(impl->semaphore);
	}

#endif
}

void Semaphore::await() const
{
#if APR_HAS_THREADS
	DWORD dwWaitResult = WaitForSingleObject(impl->semaphore, INFINITE);

	if (stat != 0)
	{
		throw MutexException(1);
	}

#endif
}

void Semaphore::signalAll() const
{
#if APR_HAS_THREADS
	BOOL stat = ReleaseSemaphore(impl->semaphore, 1, NULL);

	if (!stat)
	{
		throw MutexException(stat);
	}

#endif
}

#else
// POSIX

namespace log4cxx
{
namespace helpers
{
struct SemaphoreImpl
{
	sem_t semaphore;
};
}
}

Semaphore::Semaphore(log4cxx::helpers::Pool& p)
	: impl(nullptr)
{
#if APR_HAS_THREADS
	impl = (SemaphoreImpl*)p.palloc(sizeof(SemaphoreImpl));

	if (nullptr == impl)
	{
		throw MutexException(APR_ENOMEM);
	}

	int stat = sem_init(&impl->semaphore, 0, 0);

	if (stat != 0)
	{
		throw MutexException(APR_ENOSHMAVAIL);
	}

#endif
}

Semaphore::~Semaphore()
{
#if APR_HAS_THREADS

	if (impl)
	{
		int stat = sem_destroy(&impl->semaphore);
	}

#endif
}

void Semaphore::await() const
{
#if APR_HAS_THREADS
	int stat = sem_wait(&impl->semaphore);

	if (stat != 0)
	{
		throw MutexException(stat);
	}

#endif
}

void Semaphore::signalAll() const
{
#if APR_HAS_THREADS
	int stat = sem_post(&impl->semaphore);

	if (stat != 0)
	{
		throw MutexException(stat);
	}

#endif
}

#endif // POSIX

#endif // NON_BLOCKING
