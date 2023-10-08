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

#ifndef _LOG4CXX_FILTER_LEVEL_RANGE_FILTER_H
#define _LOG4CXX_FILTER_LEVEL_RANGE_FILTER_H

#include <log4cxx/spi/filter.h>
#include <log4cxx/level.h>

namespace log4cxx
{
namespace filter
{
/**
This is a very simple filter based on level matching, which can be
used to reject messages with priorities outside a certain range.

<p>The filter admits three options <code>levelMin</code>, <code>levelMax</code>
and <code>acceptOnMatch</code>.

<p>If the level of the {@link spi::LoggingEvent LoggingEvent} is not
between Min and Max (inclusive), then {@link spi::Filter#DENY DENY}
is returned.

<p> If the Logging event level is within the specified range, then if
<code>acceptOnMatch</code> is true, {@link spi::Filter#ACCEPT ACCEPT} is
returned, and if <code>acceptOnMatch</code> is false,
{@link spi::Filter#NEUTRAL NEUTRAL} is returned.

<p>If <code>levelMin</code>w is not defined, then there is no
minimum acceptable level (ie a level is never rejected for
being too "low"/unimportant).  If <code>levelMax</code> is not
defined, then there is no maximum acceptable level (ie a
level is never rejected for beeing too "high"/important).

<p>Refer to the {@link AppenderSkeleton#setThreshold setThreshold} method
available to <code>all</code> appenders extending
AppenderSkeleton for a more convenient way to
filter out events by level.
*/

class LOG4CXX_EXPORT LevelRangeFilter : public spi::Filter
{
	private:
		struct LevelRangeFilterPrivate;

	public:
		typedef spi::Filter BASE_CLASS;
		DECLARE_LOG4CXX_OBJECT(LevelRangeFilter)
		BEGIN_LOG4CXX_CAST_MAP()
		LOG4CXX_CAST_ENTRY(LevelRangeFilter)
		LOG4CXX_CAST_ENTRY_CHAIN(BASE_CLASS)
		END_LOG4CXX_CAST_MAP()

		LevelRangeFilter();
		~LevelRangeFilter();

		/**
		Set options
		*/
		void setOption(const LogString& option, const LogString& value) override;

		/**
		Set the <code>levelMin</code> option.
		*/
		void setLevelMin(const LevelPtr& levelMin1);

		/**
		Get the value of the <code>levelMin</code> option.
		*/
		const LevelPtr& getLevelMin() const;

		/**
		Set the <code>levelMax</code> option.
		*/
		void setLevelMax(const LevelPtr& levelMax1);

		/**
		Get the value of the <code>levelMax</code> option.
		*/
		const LevelPtr& getLevelMax() const;

		/**
		Set the <code>acceptOnMatch</code> option.
		*/
		void setAcceptOnMatch(bool acceptOnMatch1);

		/**
		Get the value of the <code>acceptOnMatch</code> option.
		*/
		bool getAcceptOnMatch() const;

		/**
		Return the decision of this filter.

		Returns {@link spi::Filter#NEUTRAL NEUTRAL} if the
		<code>levelToMatch</code> option is not set or if there is not match.
		Otherwise, if there is a match, then the returned decision is
		{@link spi::Filter#ACCEPT ACCEPT} if the
		<code>acceptOnMatch</code> property is set to <code>true</code>. The
		returned decision is {@link spi::Filter#DENY DENY} if the
		<code>acceptOnMatch</code> property is set to false.
		*/
		FilterDecision decide(const spi::LoggingEventPtr& event) const override;
}; // class LevelRangeFilter
LOG4CXX_PTR_DEF(LevelRangeFilter);
}  // namespace filter
} // namespace log4cxx

#endif // _LOG4CXX_FILTER_LEVEL_RANGE_FILTER_H
