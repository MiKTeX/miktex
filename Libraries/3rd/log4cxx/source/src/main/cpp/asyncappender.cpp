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


#include <log4cxx/asyncappender.h>


#include <log4cxx/helpers/loglog.h>
#include <log4cxx/spi/loggingevent.h>
#include <apr_thread_proc.h>
#include <apr_thread_mutex.h>
#include <apr_thread_cond.h>
#include <log4cxx/helpers/stringhelper.h>
#include <apr_atomic.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/threadutility.h>
#include <log4cxx/private/appenderskeleton_priv.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;

/**
 * The default buffer size is set to 128 events.
*/
enum { DEFAULT_BUFFER_SIZE = 128 };

class DiscardSummary
{
	private:
		/**
		 * First event of the highest severity.
		*/
		::log4cxx::spi::LoggingEventPtr maxEvent;

		/**
		* Total count of messages discarded.
		*/
		int count;

	public:
		/**
		 * Create new instance.
		 *
		 * @param event event, may not be null.
		*/
		DiscardSummary(const ::log4cxx::spi::LoggingEventPtr& event);
		/** Copy constructor.  */
		DiscardSummary(const DiscardSummary& src);
		/** Assignment operator. */
		DiscardSummary& operator=(const DiscardSummary& src);

		/**
		 * Add discarded event to summary.
		 *
		 * @param event event, may not be null.
		*/
		void add(const ::log4cxx::spi::LoggingEventPtr& event);

		/**
		 * Create event with summary information.
		 *
		 * @return new event.
		 */
		::log4cxx::spi::LoggingEventPtr createEvent(::log4cxx::helpers::Pool& p);

		static
		::log4cxx::spi::LoggingEventPtr createEvent(::log4cxx::helpers::Pool& p,
			size_t discardedCount);
};

typedef std::map<LogString, DiscardSummary> DiscardMap;

struct AsyncAppender::AsyncAppenderPriv : public AppenderSkeleton::AppenderSkeletonPrivate
{
	AsyncAppenderPriv() :
		AppenderSkeletonPrivate(),
		buffer(),
		bufferSize(DEFAULT_BUFFER_SIZE),
		appenders(std::make_shared<AppenderAttachableImpl>(pool)),
		dispatcher(),
		locationInfo(false),
		blocking(true) {}

	/**
	 * Event buffer.
	*/
#if defined(NON_BLOCKING)
	boost::lockfree::queue<log4cxx::spi::LoggingEvent* > buffer;
	std::atomic<size_t> discardedCount;
#else
	LoggingEventList buffer;
#endif

	/**
	 *  Mutex used to guard access to buffer and discardMap.
	 */
	std::mutex bufferMutex;

#if defined(NON_BLOCKING)
	::log4cxx::helpers::Semaphore bufferNotFull;
	::log4cxx::helpers::Semaphore bufferNotEmpty;
#else
	std::condition_variable bufferNotFull;
	std::condition_variable bufferNotEmpty;
#endif

	/**
	  * Map of DiscardSummary objects keyed by logger name.
	*/
	DiscardMap discardMap;

	/**
	 * Buffer size.
	*/
	int bufferSize;

	/**
	 * Nested appenders.
	*/
	helpers::AppenderAttachableImplPtr appenders;

	/**
	 *  Dispatcher.
	 */
	std::thread dispatcher;

	/**
	 * Should location info be included in dispatched messages.
	*/
	bool locationInfo;

	/**
	 * Does appender block when buffer is full.
	*/
	bool blocking;
};


IMPLEMENT_LOG4CXX_OBJECT(AsyncAppender)

#define priv static_cast<AsyncAppenderPriv*>(m_priv.get())

AsyncAppender::AsyncAppender()
	: AppenderSkeleton(std::make_unique<AsyncAppenderPriv>())
{
}

AsyncAppender::~AsyncAppender()
{
	finalize();
}

void AsyncAppender::addAppender(const AppenderPtr newAppender)
{
	priv->appenders->addAppender(newAppender);
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
	std::lock_guard<std::recursive_mutex> lock(priv->mutex);

	doAppendImpl(event, pool1);
}

void AsyncAppender::append(const spi::LoggingEventPtr& event, Pool& p)
{
	if (priv->bufferSize <= 0)
	{
		priv->appenders->appendLoopOnAppenders(event, p);
	}
	if (!priv->dispatcher.joinable())
	{
		priv->dispatcher = ThreadUtility::instance()->createThread( LOG4CXX_STR("AsyncAppender"), &AsyncAppender::dispatch, this );
	}

	// Set the NDC and MDC for the calling thread as these
	// LoggingEvent fields were not set at event creation time.
	LogString ndcVal;
	event->getNDC(ndcVal);
	// Get a copy of this thread's MDC.
	event->getMDCCopy();


	{
		std::unique_lock<std::mutex> lock(priv->bufferMutex);

		while (true)
		{
			size_t previousSize = priv->buffer.size();

			if (previousSize < (size_t)priv->bufferSize)
			{
				priv->buffer.push_back(event);

				if (previousSize == 0)
				{
					priv->bufferNotEmpty.notify_all();
				}

				break;
			}

			//
			//   Following code is only reachable if buffer is full
			//
			//
			//   if blocking and thread is not already interrupted
			//      and not the dispatcher then
			//      wait for a buffer notification
			bool discard = true;

			if (priv->blocking
				&& !priv->closed
				&& (priv->dispatcher.get_id() != std::this_thread::get_id()) )
			{
				priv->bufferNotFull.wait(lock);
				discard = false;
			}

			//
			//   if blocking is false or thread has been interrupted
			//   add event to discard map.
			//
			if (discard)
			{
				LogString loggerName = event->getLoggerName();
				DiscardMap::iterator iter = priv->discardMap.find(loggerName);

				if (iter == priv->discardMap.end())
				{
					DiscardSummary summary(event);
					priv->discardMap.insert(DiscardMap::value_type(loggerName, summary));
				}
				else
				{
					(*iter).second.add(event);
				}

				break;
			}
		}
	}
}


void AsyncAppender::close()
{
	{
		std::lock_guard<std::mutex> lock(priv->bufferMutex);
		priv->closed = true;
		priv->bufferNotEmpty.notify_all();
		priv->bufferNotFull.notify_all();
	}

	if ( priv->dispatcher.joinable() )
	{
		priv->dispatcher.join();
	}

	{
		AppenderList appenderList = priv->appenders->getAllAppenders();

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
	return priv->appenders->getAllAppenders();
}

AppenderPtr AsyncAppender::getAppender(const LogString& n) const
{
	return priv->appenders->getAppender(n);
}

bool AsyncAppender::isAttached(const AppenderPtr appender) const
{
	return priv->appenders->isAttached(appender);
}

bool AsyncAppender::requiresLayout() const
{
	return false;
}

void AsyncAppender::removeAllAppenders()
{
	priv->appenders->removeAllAppenders();
}

void AsyncAppender::removeAppender(const AppenderPtr appender)
{
	priv->appenders->removeAppender(appender);
}

void AsyncAppender::removeAppender(const LogString& n)
{
	priv->appenders->removeAppender(n);
}

bool AsyncAppender::getLocationInfo() const
{
	return priv->locationInfo;
}

void AsyncAppender::setLocationInfo(bool flag)
{
	priv->locationInfo = flag;
}


void AsyncAppender::setBufferSize(int size)
{
	if (size < 0)
	{
		throw IllegalArgumentException(LOG4CXX_STR("size argument must be non-negative"));
	}

	std::lock_guard<std::mutex> lock(priv->bufferMutex);
	priv->bufferSize = (size < 1) ? 1 : size;
	priv->bufferNotFull.notify_all();
}

int AsyncAppender::getBufferSize() const
{
	return priv->bufferSize;
}

void AsyncAppender::setBlocking(bool value)
{
	std::lock_guard<std::mutex> lock(priv->bufferMutex);
	priv->blocking = value;
	priv->bufferNotFull.notify_all();
}

bool AsyncAppender::getBlocking() const
{
	return priv->blocking;
}

DiscardSummary::DiscardSummary(const LoggingEventPtr& event) :
	maxEvent(event), count(1)
{
}

DiscardSummary::DiscardSummary(const DiscardSummary& src) :
	maxEvent(src.maxEvent), count(src.count)
{
}

DiscardSummary& DiscardSummary::operator=(const DiscardSummary& src)
{
	maxEvent = src.maxEvent;
	count = src.count;
	return *this;
}

void DiscardSummary::add(const LoggingEventPtr& event)
{
	if (event->getLevel()->toInt() > maxEvent->getLevel()->toInt())
	{
		maxEvent = event;
	}

	count++;
}

LoggingEventPtr DiscardSummary::createEvent(Pool& p)
{
	LogString msg(LOG4CXX_STR("Discarded "));
	StringHelper::toString(count, p, msg);
	msg.append(LOG4CXX_STR(" messages due to a full event buffer including: "));
	msg.append(maxEvent->getMessage());
	return std::make_shared<LoggingEvent>(
				maxEvent->getLoggerName(),
				maxEvent->getLevel(),
				msg,
				LocationInfo::getLocationUnavailable() );
}

::log4cxx::spi::LoggingEventPtr
DiscardSummary::createEvent(::log4cxx::helpers::Pool& p,
	size_t discardedCount)
{
	LogString msg(LOG4CXX_STR("Discarded "));
	StringHelper::toString(discardedCount, p, msg);
	msg.append(LOG4CXX_STR(" messages due to a full event buffer"));

	return std::make_shared<LoggingEvent>(
				LOG4CXX_STR(""),
				log4cxx::Level::getError(),
				msg,
				LocationInfo::getLocationUnavailable() );
}

void AsyncAppender::dispatch()
{
	bool isActive = true;

	while (isActive)
	{
		//
		//   process events after lock on buffer is released.
		//
		Pool p;
		LoggingEventList events;
		{
			std::unique_lock<std::mutex> lock(priv->bufferMutex);
			priv->bufferNotEmpty.wait(lock, [this]() -> bool
				{ return 0 < priv->buffer.size() || priv->closed; }
			);
			isActive = !priv->closed;

			for (LoggingEventList::iterator eventIter = priv->buffer.begin();
				eventIter != priv->buffer.end();
				eventIter++)
			{
				events.push_back(*eventIter);
			}

			for (DiscardMap::iterator discardIter = priv->discardMap.begin();
				discardIter != priv->discardMap.end();
				discardIter++)
			{
				events.push_back(discardIter->second.createEvent(p));
			}

			priv->buffer.clear();
			priv->discardMap.clear();
			priv->bufferNotFull.notify_all();
		}

		for (LoggingEventList::iterator iter = events.begin();
			iter != events.end();
			iter++)
		{
			try
			{
				priv->appenders->appendLoopOnAppenders(*iter, p);
			}
			catch (std::exception& ex)
			{
				priv->errorHandler->error(LOG4CXX_STR("async dispatcher"), ex, 0, *iter);
				isActive = false;
			}
			catch (...)
			{
				priv->errorHandler->error(LOG4CXX_STR("async dispatcher"));
				isActive = false;
			}
		}
	}

}
