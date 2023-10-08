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
#ifndef _LOG4CXX_FILTER_LOCATIONINFOFILTER_H
#define _LOG4CXX_FILTER_LOCATIONINFOFILTER_H

#include <log4cxx/spi/filter.h>

namespace log4cxx
{

namespace filter
{

/**
 * When location information is available, individual log statements can be turned on or off
 * depending on their source location.
 *
 * This filter allows for filtering messages based off of either the line number of the
 * message, or the name of the method that the log mesage is in.  The 'operator' parameter
 * may be used to determine if both the method name and line number must match.
 * If 'operator' is set to 'AND', then both the line number and method name must match,
 * otherwise only one needs to match.  By default, 'operator' is set to 'OR'.
 *
 * If location information is not available, this filter does nothing.
 *
 */
class LOG4CXX_EXPORT LocationInfoFilter: public log4cxx::spi::Filter
{
		struct LocationInfoFilterPrivate;
	public:
		DECLARE_LOG4CXX_OBJECT(LocationInfoFilter)
		BEGIN_LOG4CXX_CAST_MAP()
		LOG4CXX_CAST_ENTRY(LocationInfoFilter)
		LOG4CXX_CAST_ENTRY_CHAIN(log4cxx::spi::Filter)
		END_LOG4CXX_CAST_MAP()

		LocationInfoFilter();

		~LocationInfoFilter();

		void setOption(const LogString& option, const LogString& value) override;

		void setLineNumber(int lineNum);

		void setMethodName(const LogString& methodName);

		void setAcceptOnMatch(bool acceptOnMatch1);

		bool getAcceptOnMatch() const;

		bool getMustMatchAll() const;

		void setMustMatchAll(bool mustMatchAll1);

		/**
		 * If this event does not already contain location information,
		 * evaluate the event against the expression.
		 *
		 * If the expression evaluates to true, generate a LocationInfo instance
		 * by creating an exception and set this LocationInfo on the event.
		 *
		 * Returns {@link log4cxx::spi::Filter#NEUTRAL}
		 */
		FilterDecision decide(const spi::LoggingEventPtr& event) const override;

};

LOG4CXX_PTR_DEF(LocationInfoFilter);

}
}
#endif
