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

#if defined(_MSC_VER)
	#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

#include <log4cxx/asyncappender.h>


#include <log4cxx/helpers/loglog.h>
#include <log4cxx/spi/loggingevent.h>
#include <apr_thread_proc.h>
#include <apr_thread_mutex.h>
#include <apr_thread_cond.h>
#include <log4cxx/helpers/condition.h>
#include <log4cxx/helpers/synchronized.h>
#include <log4cxx/helpers/stringhelper.h>
#include <apr_atomic.h>
#include <log4cxx/helpers/optionconverter.h>


using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;


IMPLEMENT_LOG4CXX_OBJECT(AsyncAppender)


AsyncAppender::AsyncAppender()
	: AppenderSkeleton(),
	  buffer(DEFAULT_BUFFER_SIZE),
	  SHARED_MUTEX_INIT(bufferMutex, pool),
	  bufferNotFull(pool),
	  bufferNotEmpty(pool),
	  discardMap(new DiscardMap()),
	  bufferSize(DEFAULT_BUFFER_SIZE),
	  appenders(new AppenderAttachableImpl(pool)),
	  dispatcher(),
	  locationInfo(false),
	  blocking(true)
{
#if APR_HAS_THREADS
	dispatcher.run(dispatch, this);
#endif
}

AsyncAppender::~AsyncAppender()
{
	finalize();
	delete discardMap;
}

void AsyncAppender::addRef() const
{
	ObjectImpl::addRef();
}

void AsyncAppender::releaseRef() const
{
	ObjectImpl::releaseRef();
}

void AsyncAppender::addAppender(const AppenderPtr& newAppender)
{
	synchronized sync(appenders->getMutex());
	appenders->addAppender(newAppender);
}


void AsyncAppender::setOption(const LogString& option,
	const LogString& value)
{
	if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("LOCATIONINFO"), LOG4CXX_STR("locationinfo")))
	{
		setLocationInfo(OptionConverter::toBoolean(value, false));
	}

	if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("BUFFERSIZE"), LOG4CXX_STR("buffersize")))
	{
		setBufferSize(OptionConverter::toInt(value, DEFAULT_BUFFER_SIZE));
	}

	if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("BLOCKING"), LOG4CXX_STR("blocking")))
	{
		setBlocking(OptionConverter::toBoolean(value, true));
	}
	else
	{
		AppenderSkeleton::setOption(option, value);
	}
}


void AsyncAppender::doAppend(const spi::LoggingEventPtr& event, Pool& pool1)
{
	LOCK_R sync(mutex);

	doAppendImpl(event, pool1);
}

void AsyncAppender::append(const spi::LoggingEventPtr& event, Pool& p)
{
#if APR_HAS_THREADS

	//
	//   if dispatcher has died then
	//      append subsequent events synchronously
	//
	if (!dispatcher.isAlive() || bufferSize <= 0)
	{
		synchronized sync(appenders->getMutex());
		appenders->appendLoopOnAppenders(event, p);
		return;
	}

	// Set the NDC and thread name for the calling thread as these
	// LoggingEvent fields were not set at event creation time.
	LogString ndcVal;
	event->getNDC(ndcVal);
	event->getThreadName();
	// Get a copy of this thread's MDC.
	event->getMDCCopy();


	{
		LOCK_R sync(bufferMutex);

		while (true)
		{

			event->addRef();

			if (buffer.bounded_push(event))
			{
				bufferNotEmpty.signalAll();
				break;
			}
			else
			{
				event->releaseRef();
			}

			//
			//   Following code is only reachable if buffer is full
			//
			//
			//   if blocking and thread is not already interrupted
			//      and not the dispatcher then
			//      wait for a buffer notification
			bool discard = true;

			if (blocking
				&& !Thread::interrupted()
				&& !dispatcher.isCurrentThread())
			{
				try
				{
					bufferNotFull.await();
					discard = false;
				}
				catch (InterruptedException& e)
				{
					//
					//  reset interrupt status so
					//    calling code can see interrupt on
					//    their next wait or sleep.
					Thread::currentThreadInterrupt();
				}
			}

			//
			//   if blocking is false or thread has been interrupted
			//   add event to discard map.
			//
			if (discard)
			{
				discardedCount++;
				break;
			}
		}
	}
#else
	synchronized sync(appenders->getMutex());
	appenders->appendLoopOnAppenders(event, p);
#endif
}


void AsyncAppender::close()
{
	{
		LOCK_W sync(bufferMutex);
		closed = true;
	}

	bufferNotEmpty.signalAll();
	bufferNotFull.signalAll();

#if APR_HAS_THREADS

	try
	{
		dispatcher.join();
	}
	catch (InterruptedException& e)
	{
		Thread::currentThreadInterrupt();
		LogLog::error(LOG4CXX_STR("Got an InterruptedException while waiting for the dispatcher to finish,"), e);
	}

#endif

	{
		synchronized sync(appenders->getMutex());
		AppenderList appenderList = appenders->getAllAppenders();

		for (AppenderList::iterator iter = appenderList.begin();
			iter != appenderList.end();
			iter++)
		{
			(*iter)->close();
		}
	}
}

AppenderList AsyncAppender::getAllAppenders() const
{
	synchronized sync(appenders->getMutex());
	return appenders->getAllAppenders();
}

AppenderPtr AsyncAppender::getAppender(const LogString& n) const
{
	synchronized sync(appenders->getMutex());
	return appenders->getAppender(n);
}

bool AsyncAppender::isAttached(const AppenderPtr& appender) const
{
	synchronized sync(appenders->getMutex());
	return appenders->isAttached(appender);
}

bool AsyncAppender::requiresLayout() const
{
	return false;
}

void AsyncAppender::removeAllAppenders()
{
	synchronized sync(appenders->getMutex());
	appenders->removeAllAppenders();
}

void AsyncAppender::removeAppender(const AppenderPtr& appender)
{
	synchronized sync(appenders->getMutex());
	appenders->removeAppender(appender);
}

void AsyncAppender::removeAppender(const LogString& n)
{
	synchronized sync(appenders->getMutex());
	appenders->removeAppender(n);
}

bool AsyncAppender::getLocationInfo() const
{
	return locationInfo;
}

void AsyncAppender::setLocationInfo(bool flag)
{
	locationInfo = flag;
}


void AsyncAppender::setBufferSize(int size)
{
	if (size < 0)
	{
		throw IllegalArgumentException(LOG4CXX_STR("size argument must be non-negative"));
	}

	{
		LOCK_W sync(bufferMutex);
		bufferSize = (size < 1) ? 1 : size;
		buffer.reserve_unsafe(bufferSize);
	}

	bufferNotFull.signalAll();
}

int AsyncAppender::getBufferSize() const
{
	return bufferSize;
}

void AsyncAppender::setBlocking(bool value)
{
	{
		LOCK_W sync(bufferMutex);
		blocking = value;
	}
	bufferNotFull.signalAll();
}

bool AsyncAppender::getBlocking() const
{
	return blocking;
}

AsyncAppender::DiscardSummary::DiscardSummary(const LoggingEventPtr& event) :
	maxEvent(event), count(1)
{
}

AsyncAppender::DiscardSummary::DiscardSummary(const DiscardSummary& src) :
	maxEvent(src.maxEvent), count(src.count)
{
}

AsyncAppender::DiscardSummary& AsyncAppender::DiscardSummary::operator=(const DiscardSummary& src)
{
	maxEvent = src.maxEvent;
	count = src.count;
	return *this;
}

void AsyncAppender::DiscardSummary::add(const LoggingEventPtr& event)
{
	if (event->getLevel()->toInt() > maxEvent->getLevel()->toInt())
	{
		maxEvent = event;
	}

	count++;
}

LoggingEventPtr AsyncAppender::DiscardSummary::createEvent(Pool& p)
{
	LogString msg(LOG4CXX_STR("Discarded "));
	StringHelper::toString(count, p, msg);
	msg.append(LOG4CXX_STR(" messages due to a full event buffer including: "));
	msg.append(maxEvent->getMessage());
	return new LoggingEvent(
			maxEvent->getLoggerName(),
			maxEvent->getLevel(),
			msg,
			LocationInfo::getLocationUnavailable());
}

::log4cxx::spi::LoggingEventPtr
AsyncAppender::DiscardSummary::createEvent(::log4cxx::helpers::Pool& p,
	size_t discardedCount)
{
	LogString msg(LOG4CXX_STR("Discarded "));
	StringHelper::toString(discardedCount, p, msg);
	msg.append(LOG4CXX_STR(" messages due to a full event buffer"));

	return new LoggingEvent(
			LOG4CXX_STR(""),
			log4cxx::Level::getError(),
			msg,
			LocationInfo::getLocationUnavailable());
}

#if APR_HAS_THREADS
void* LOG4CXX_THREAD_FUNC AsyncAppender::dispatch(apr_thread_t* /*thread*/, void* data)
{
	AsyncAppender* pThis = (AsyncAppender*) data;

	try
	{
		while (!pThis->closed || !pThis->buffer.empty())
		{

			pThis->bufferNotEmpty.await();

			//
			//   process events after lock on buffer is released.
			//
			Pool p;
			LoggingEventList events;
			{
				LOCK_R sync(pThis->bufferMutex);

				unsigned count = 0;
				log4cxx::spi::LoggingEvent* logPtr = nullptr;

				while (pThis->buffer.pop(logPtr))
				{
					log4cxx::spi::LoggingEventPtr ptr(logPtr);
					events.push_back(ptr);
					logPtr->releaseRef();
					count++;
				}

				if (pThis->blocking)
				{
					pThis->bufferNotFull.signalAll();
				}

				size_t discarded = pThis->discardedCount.exchange(0);

				if (discarded != 0)
				{
					events.push_back(AsyncAppender::DiscardSummary::createEvent(p, discarded));
				}
			}

			for (LoggingEventList::iterator iter = events.begin();
				iter != events.end();
				iter++)
			{
				synchronized sync(pThis->appenders->getMutex());
				pThis->appenders->appendLoopOnAppenders(*iter, p);
			}
		}
	}
	catch (InterruptedException& ex)
	{
		Thread::currentThreadInterrupt();
	}
	catch (...)
	{
	}

	return 0;
}
#endif
