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

#include "log4cxx/helpers/threadutility.h"
#if !defined(LOG4CXX)
	#define LOG4CXX 1
#endif
#include "log4cxx/private/log4cxx_private.h"
#include "log4cxx/helpers/loglog.h"
#include "log4cxx/helpers/transcoder.h"

#include <signal.h>
#include <mutex>

#if WIN32
	#include <windows.h>
	#include <processthreadsapi.h>
#endif

namespace log4cxx
{
namespace helpers
{

struct ThreadUtility::priv_data
{
	priv_data()
	{
		start_pre = nullptr;
		started = nullptr;
		start_post = nullptr;
	}

	ThreadStartPre start_pre;
	ThreadStarted started;
	ThreadStartPost start_post;
};

#if LOG4CXX_HAS_PTHREAD_SIGMASK
	static thread_local sigset_t old_mask;
	static thread_local bool sigmask_valid;
#endif

ThreadUtility::ThreadUtility() :
	m_priv( new priv_data() )
{
	// Block signals by default.
	configureFuncs( std::bind( &ThreadUtility::preThreadBlockSignals, this ),
		nullptr,
		std::bind( &ThreadUtility::postThreadUnblockSignals, this ) );
}

ThreadUtility::~ThreadUtility() {}

ThreadUtility* ThreadUtility::instance()
{
	static ThreadUtility instance;
	return &instance;
}

void ThreadUtility::configure( ThreadConfigurationType type )
{
	auto utility = instance();

	if ( type == ThreadConfigurationType::NoConfiguration )
	{
		utility->configureFuncs( nullptr, nullptr, nullptr );
	}
	else if ( type == ThreadConfigurationType::NameThreadOnly )
	{
		utility->configureFuncs( nullptr,
			std::bind( &ThreadUtility::threadStartedNameThread, utility,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3 ),
			nullptr );
	}
	else if ( type == ThreadConfigurationType::BlockSignalsOnly )
	{
		utility->configureFuncs( std::bind( &ThreadUtility::preThreadBlockSignals, utility ),
			nullptr,
			std::bind( &ThreadUtility::postThreadUnblockSignals, utility ) );
	}
	else if ( type == ThreadConfigurationType::BlockSignalsAndNameThread )
	{
		utility->configureFuncs( std::bind( &ThreadUtility::preThreadBlockSignals, utility ),
			std::bind( &ThreadUtility::threadStartedNameThread, utility,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3 ),
			std::bind( &ThreadUtility::postThreadUnblockSignals, utility ) );
	}
}

void ThreadUtility::configureFuncs( ThreadStartPre pre_start,
	ThreadStarted started,
	ThreadStartPost post_start )
{
	m_priv->start_pre = pre_start;
	m_priv->started = started;
	m_priv->start_post = post_start;
}

void ThreadUtility::preThreadBlockSignals()
{
#if LOG4CXX_HAS_PTHREAD_SIGMASK
	sigset_t set;
	sigfillset(&set);

	if ( pthread_sigmask(SIG_SETMASK, &set, &old_mask) < 0 )
	{
		LOGLOG_ERROR( LOG4CXX_STR("Unable to set thread sigmask") );
		sigmask_valid = false;
	}
	else
	{
		sigmask_valid = true;
	}

#endif /* LOG4CXX_HAS_PTHREAD_SIGMASK */
}

void ThreadUtility::threadStartedNameThread(LogString threadName,
	std::thread::id /*threadId*/,
	std::thread::native_handle_type nativeHandle)
{
#if LOG4CXX_HAS_PTHREAD_SETNAME
	LOG4CXX_ENCODE_CHAR(sthreadName, threadName);
	if (pthread_setname_np(static_cast<pthread_t>(nativeHandle), sthreadName.c_str()) < 0) {
		LOGLOG_ERROR(LOG4CXX_STR("unable to set thread name"));
	}
#elif WIN32
	typedef HRESULT (WINAPI *TSetThreadDescription)(HANDLE, PCWSTR);
	static struct initialiser
	{
		HMODULE hKernelBase;
		TSetThreadDescription SetThreadDescription;
		initialiser()
			: hKernelBase(GetModuleHandleA("KernelBase.dll"))
			, SetThreadDescription(nullptr)
		{
			if (hKernelBase)
				SetThreadDescription = reinterpret_cast<TSetThreadDescription>(GetProcAddress(hKernelBase, "SetThreadDescription"));
		}
	} win32Func;
	if (win32Func.SetThreadDescription)
	{
		LOG4CXX_ENCODE_WCHAR(wthreadName, threadName);
		if(FAILED(win32Func.SetThreadDescription(static_cast<HANDLE>(nativeHandle), wthreadName.c_str())))
			LOGLOG_ERROR( LOG4CXX_STR("unable to set thread name") );
	}
#endif
}

void ThreadUtility::postThreadUnblockSignals()
{
#if LOG4CXX_HAS_PTHREAD_SIGMASK

	// Only restore the signal mask if we were able to set it in the first place.
	if ( sigmask_valid )
	{
		if ( pthread_sigmask(SIG_SETMASK, &old_mask, nullptr) < 0 )
		{
			LOGLOG_ERROR( LOG4CXX_STR("Unable to set thread sigmask") );
		}
	}

#endif /* LOG4CXX_HAS_PTHREAD_SIGMASK */
}


ThreadStartPre ThreadUtility::preStartFunction()
{
	return m_priv->start_pre;
}

ThreadStarted ThreadUtility::threadStartedFunction()
{
	return m_priv->started;
}

ThreadStartPost ThreadUtility::postStartFunction()
{
	return m_priv->start_post;
}

} //namespace helpers
} //namespace log4cxx
