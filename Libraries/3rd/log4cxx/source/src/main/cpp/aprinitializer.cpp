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
#if !defined(LOG4CXX)
	#define LOG4CXX 1
#endif
#include <log4cxx/helpers/aprinitializer.h>
#include <apr_pools.h>
#include <apr_atomic.h>
#include <assert.h>
#include <log4cxx/helpers/threadspecificdata.h>
#include <apr_thread_mutex.h>
#include <apr_thread_proc.h>
#include <log4cxx/helpers/synchronized.h>
#include <log4cxx/helpers/filewatchdog.h>

using namespace log4cxx::helpers;
using namespace log4cxx;

bool APRInitializer::isDestructed = false;


namespace
{
extern "C" void tlsDestruct(void* ptr)
{
	delete ((ThreadSpecificData*) ptr);
}
}

APRInitializer::APRInitializer() : p(0), mutex(0), startTime(0), tlsKey(0)
{
	apr_initialize();
	apr_pool_create(&p, NULL);
	apr_atomic_init(p);
	startTime = apr_time_now();
#if APR_HAS_THREADS
	apr_status_t stat = apr_threadkey_private_create(&tlsKey, tlsDestruct, p);
	assert(stat == APR_SUCCESS);
	stat = apr_thread_mutex_create(&mutex, APR_THREAD_MUTEX_NESTED, p);
	assert(stat == APR_SUCCESS);
#endif
}

APRInitializer::~APRInitializer()
{
	{
#if APR_HAS_THREADS
		synchronized sync(mutex);
		apr_threadkey_private_delete(tlsKey);
#endif

		for (std::list<FileWatchdog*>::iterator iter = watchdogs.begin();
			iter != watchdogs.end();
			iter++)
		{
			delete *iter;
		}
	}

	// TODO LOGCXX-322
#ifndef APR_HAS_THREADS
	apr_terminate();
#endif
	isDestructed = true;
}

APRInitializer& APRInitializer::getInstance()
{
	static APRInitializer init;
	return init;
}


log4cxx_time_t APRInitializer::initialize()
{
	return getInstance().startTime;
}

apr_pool_t* APRInitializer::getRootPool()
{
	return getInstance().p;
}

apr_threadkey_t* APRInitializer::getTlsKey()
{
	return getInstance().tlsKey;
}

void APRInitializer::registerCleanup(FileWatchdog* watchdog)
{
	APRInitializer& instance(getInstance());
#if APR_HAS_THREADS
	synchronized sync(instance.mutex);
#endif
	instance.watchdogs.push_back(watchdog);
}

void APRInitializer::unregisterCleanup(FileWatchdog* watchdog)
{
	APRInitializer& instance(getInstance());
#if APR_HAS_THREADS
	synchronized sync(instance.mutex);
#endif

	for (std::list<FileWatchdog*>::iterator iter = instance.watchdogs.begin();
		iter != instance.watchdogs.end();
		iter++)
	{
		if (*iter == watchdog)
		{
			instance.watchdogs.erase(iter);
			return;
		}
	}
}

