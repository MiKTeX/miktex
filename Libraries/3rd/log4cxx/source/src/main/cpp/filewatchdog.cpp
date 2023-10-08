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
#define __STDC_CONSTANT_MACROS
#include <log4cxx/logstring.h>
#include <log4cxx/helpers/filewatchdog.h>
#include <log4cxx/helpers/loglog.h>
#include <apr_thread_proc.h>
#include <apr_atomic.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/helpers/threadutility.h>
#include <log4cxx/helpers/stringhelper.h>
#include <functional>
#include <chrono>

using namespace log4cxx;
using namespace log4cxx::helpers;

long FileWatchdog::DEFAULT_DELAY = 60000;

struct FileWatchdog::FileWatchdogPrivate{
	FileWatchdogPrivate(const File& file1) :
		file(file1), delay(DEFAULT_DELAY), lastModif(0),
		warnedAlready(false), interrupted(0), thread(){}

	/**
	The name of the file to observe  for changes.
	*/
	File file;

	/**
	The delay to observe between every check.
	By default set DEFAULT_DELAY.*/
	long delay;
	log4cxx_time_t lastModif;
	bool warnedAlready;
	volatile int interrupted;
	Pool pool;
	std::thread thread;
	std::condition_variable interrupt;
	std::mutex interrupt_mutex;
};

FileWatchdog::FileWatchdog(const File& file1)
	: m_priv(std::make_unique<FileWatchdogPrivate>(file1))
{
}

FileWatchdog::~FileWatchdog()
{
	m_priv->interrupted = 0xFFFF;

	{
		std::unique_lock<std::mutex> lock(m_priv->interrupt_mutex);
		m_priv->interrupt.notify_all();
	}
	m_priv->thread.join();
}

const File& FileWatchdog::file(){
	return m_priv->file;
}

void FileWatchdog::checkAndConfigure()
{
	Pool pool1;

	if (!m_priv->file.exists(pool1))
	{
		if (!m_priv->warnedAlready)
		{
			LogLog::debug(((LogString) LOG4CXX_STR("["))
				+ m_priv->file.getPath()
				+ LOG4CXX_STR("] does not exist."));
			m_priv->warnedAlready = true;
		}
	}
	else
	{
		apr_time_t thisMod = m_priv->file.lastModified(pool1);

		if (thisMod > m_priv->lastModif)
		{
			m_priv->lastModif = thisMod;
			doOnChange();
			m_priv->warnedAlready = false;
		}
	}
}

void FileWatchdog::run()
{
	LogString msg(LOG4CXX_STR("Checking ["));
	msg += m_priv->file.getPath();
	msg += LOG4CXX_STR("] at ");
	StringHelper::toString((int)m_priv->delay, m_priv->pool, msg);
	msg += LOG4CXX_STR(" ms interval");
	LogLog::debug(msg);

	while (m_priv->interrupted != 0xFFFF)
	{
		std::unique_lock<std::mutex> lock( m_priv->interrupt_mutex );
		m_priv->interrupt.wait_for( lock, std::chrono::milliseconds( m_priv->delay ),
			std::bind(&FileWatchdog::is_interrupted, this) );

		checkAndConfigure();
	}

	LogString msg2(LOG4CXX_STR("Stop checking ["));
	msg2 += m_priv->file.getPath();
	msg2 += LOG4CXX_STR("]");
	LogLog::debug(msg2);
}

void FileWatchdog::start()
{
	checkAndConfigure();

	m_priv->thread = ThreadUtility::instance()->createThread( LOG4CXX_STR("FileWatchdog"), &FileWatchdog::run, this );
}

bool FileWatchdog::is_interrupted()
{
	return m_priv->interrupted == 0xFFFF;
}

void FileWatchdog::setDelay(long delay1){
	m_priv->delay = delay1;
}
