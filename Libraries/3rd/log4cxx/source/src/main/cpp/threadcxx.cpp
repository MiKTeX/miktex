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
#include <log4cxx/helpers/thread.h>
#include <log4cxx/helpers/exception.h>
#include <apr_thread_proc.h>
#include <apr_atomic.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/threadlocal.h>
#include <log4cxx/helpers/synchronized.h>
#include <apr_thread_cond.h>

using namespace log4cxx::helpers;
using namespace log4cxx;

#if APR_HAS_THREADS
namespace
{
/**
 *   This class is used to encapsulate the parameters to
 *   Thread::run when they are passed to Thread::launcher.
 *
 */
class LaunchPackage
{
	public:
		/**
		 *  Placement new to create LaunchPackage in specified pool.
		 *  LaunchPackage needs to be dynamically allocated since
		 *  since a stack allocated instance may go out of scope
		 *  before thread is launched.
		 */
		static void* operator new (size_t sz, Pool& p)
		{
			return p.palloc(sz);
		}
		/**
		*  operator delete would be called if exception during construction.
		*/
		static void operator delete (void*, Pool& p)
		{
		}
		/**
		 *  Create new instance.
		 */
		LaunchPackage(Thread* t, Runnable r, void* d) : thread(t), runnable(r), data(d)
		{
		}
		/**
		 * Gets thread parameter.
		 * @return thread.
		 */
		Thread* getThread() const
		{
			return thread;
		}

		/**
		 *  Gets runnable parameter.
		 *  @return runnable.
		 */
		Runnable getRunnable() const
		{
			return runnable;
		}
		/**
		 *  gets data parameter.
		 *  @return thread.
		 */
		void* getData() const
		{
			return data;
		}
	private:
		LaunchPackage(const LaunchPackage&);
		LaunchPackage& operator=(const LaunchPackage&);
		Thread* thread;
		Runnable runnable;
		void* data;
};

/**
 *  This object atomically sets the specified memory location
 *  to non-zero on construction and to zero on destruction.
 *  Used to maintain Thread.alive.
 */
class LaunchStatus
{
	public:
		/*
		 *  Construct new instance.
		 *  @param p address of memory to set to non-zero on construction, zero on destruction.
		 */
		LaunchStatus(volatile unsigned int* p) : alive(p)
		{
			apr_atomic_set32(alive, 0xFFFFFFFF);
		}
		/**
		 *  Destructor.
		 */
		~LaunchStatus()
		{
			apr_atomic_set32(alive, 0);
		}

	private:
		LaunchStatus(const LaunchStatus&);
		LaunchStatus& operator=(const LaunchStatus&);
		volatile unsigned int* alive;
};

/**
 *   Get a key to the thread local storage used to hold the reference to
 *   the corresponding Thread object.
 */
ThreadLocal& getThreadLocal()
{
	static ThreadLocal tls;
	return tls;
}

}

void* LOG4CXX_THREAD_FUNC ThreadLaunch::launcher(apr_thread_t* thread, void* data)
{
	LaunchPackage* package = (LaunchPackage*) data;
	ThreadLocal& tls = getThreadLocal();
	tls.set(package->getThread());
	{
		(package->getRunnable())(thread, package->getData());
		package->getThread()->ending();
	}
	apr_thread_exit(thread, 0); // this function never returns !
	return 0;
}
#endif

Thread::Thread() : thread(NULL), alive(0), interruptedStatus(0),
	interruptedMutex(NULL), interruptedCondition(NULL)
{
}

Thread::~Thread()
{
	join();
}



void Thread::run(Runnable start, void* data)
{
#if APR_HAS_THREADS

	// Try to join first if previous instance did exit
	if ( isActive() && !isAlive() )
	{
		join();
	}

	// now we're ready to create the thread again
	//
	//    if attempting a second run method on the same Thread object
	//         throw an exception
	//
	if (thread != NULL)
	{
		throw IllegalStateException();
	}

	apr_threadattr_t* attrs;
	apr_status_t stat = apr_threadattr_create(&attrs, p.getAPRPool());

	if (stat != APR_SUCCESS)
	{
		throw ThreadException(stat);
	}

	stat = apr_thread_cond_create(&interruptedCondition, p.getAPRPool());

	if (stat != APR_SUCCESS)
	{
		throw ThreadException(stat);
	}

	stat = apr_thread_mutex_create(&interruptedMutex, APR_THREAD_MUTEX_NESTED,
			p.getAPRPool());

	if (stat != APR_SUCCESS)
	{
		throw ThreadException(stat);
	}

	//   create LaunchPackage on the thread's memory pool
	LaunchPackage* package = new (p) LaunchPackage(this, start, data);
	stat = apr_thread_create(&thread, attrs,
			ThreadLaunch::launcher, package, p.getAPRPool());

	if (stat != APR_SUCCESS)
	{
		throw ThreadException(stat);
	}

	// we need to set alive here already, since we use isAlive() to check
	// if run() has been called in a thread-safe way.
	apr_atomic_set32(&alive, 0xFFFFFFFF);
#else
	throw ThreadException(LOG4CXX_STR("APR_HAS_THREADS is not true"));
#endif
}

void Thread::join()
{
#if APR_HAS_THREADS

	if (thread != NULL)
	{
		apr_status_t startStat;
		apr_status_t stat = apr_thread_join(&startStat, thread);
		thread = NULL;

		if (stat != APR_SUCCESS)
		{
			throw ThreadException(stat);
		}
	}

#endif
}

void Thread::currentThreadInterrupt()
{
#if APR_HAS_THREADS
	void* tls = getThreadLocal().get();

	if (tls != 0)
	{
		((Thread*) tls)->interrupt();
	}

#endif
}

void Thread::interrupt()
{
	apr_atomic_set32(&interruptedStatus, 0xFFFFFFFF);
#if APR_HAS_THREADS

	if (interruptedMutex != NULL)
	{
		synchronized sync(interruptedMutex);
		apr_status_t stat = apr_thread_cond_signal(interruptedCondition);

		if (stat != APR_SUCCESS)
		{
			throw ThreadException(stat);
		}
	}

#endif
}

bool Thread::interrupted()
{
#if APR_HAS_THREADS
	void* tls = getThreadLocal().get();

	if (tls != 0)
	{
		return apr_atomic_xchg32(&(((Thread*) tls)->interruptedStatus), 0) != 0;
	}

#endif
	return false;
}

bool Thread::isCurrentThread() const
{
#if APR_HAS_THREADS
	const void* tls = getThreadLocal().get();
	return (tls == this);
#else
	return true;
#endif
}

bool Thread::isAlive()
{
	return apr_atomic_read32(&alive) != 0;
}

void Thread::ending()
{
	apr_atomic_set32(&alive, 0);
}


void Thread::sleep(int duration)
{
#if APR_HAS_THREADS

	if (interrupted())
	{
		throw InterruptedException();
	}

	if (duration > 0)
	{
		Thread* pThis = (Thread*) getThreadLocal().get();

		if (pThis == NULL)
		{
			apr_sleep(duration * 1000);
		}
		else
		{
			synchronized sync(pThis->interruptedMutex);
			apr_status_t stat = apr_thread_cond_timedwait(pThis->interruptedCondition,
					pThis->interruptedMutex, duration * 1000);

			if (stat != APR_SUCCESS && !APR_STATUS_IS_TIMEUP(stat))
			{
				throw ThreadException(stat);
			}

			if (interrupted())
			{
				throw InterruptedException();
			}
		}
	}

#else

	if (duration > 0)
	{
		apr_sleep(duration * 1000);
	}

#endif
}
