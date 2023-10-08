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

#ifndef _LOG4CXX_HELPERS_DATE_H
#define _LOG4CXX_HELPERS_DATE_H

#include <log4cxx/helpers/object.h>
#include <log4cxx/log4cxx.h>
#include <functional>

namespace log4cxx
{
namespace helpers
{
/**
*    Simple transcoder for converting between
*      external char and wchar_t strings and
*      internal strings.
*
*/
class LOG4CXX_EXPORT Date : public Object
{
		const log4cxx_time_t time;

	public:
		DECLARE_LOG4CXX_OBJECT(Date)
		BEGIN_LOG4CXX_CAST_MAP()
		LOG4CXX_CAST_ENTRY(Date)
		END_LOG4CXX_CAST_MAP()

		Date();
		Date(log4cxx_time_t time);
		virtual ~Date();

		inline log4cxx_time_t getTime() const
		{
			return time;
		}

		/**
		 *   Get start of next second
		 */
		log4cxx_time_t getNextSecond() const;


		static log4cxx_time_t getMicrosecondsPerDay();
		static log4cxx_time_t getMicrosecondsPerSecond();
		static log4cxx_time_t getCurrentTimeStd();
		static log4cxx_time_t currentTime();

		/**
		 * A function that will return the current time(in microseconds) when called
		 */
		typedef std::function<log4cxx_time_t()> GetCurrentTimeFn;

		/**
		 * Set the function that is used to get the current time.
		 * This is used only for testing purposes and should never be called
		 * under normal circumstances.
		 *
		 * @param fn
		 */
		static void setGetCurrentTimeFunction(GetCurrentTimeFn fn);

};

LOG4CXX_PTR_DEF(Date);

}
}


#endif
