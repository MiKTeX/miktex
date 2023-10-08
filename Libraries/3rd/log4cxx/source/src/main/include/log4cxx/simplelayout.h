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

#ifndef _LOG4CXX_SIMPLE_LAYOUT_H
#define _LOG4CXX_SIMPLE_LAYOUT_H

#include <log4cxx/layout.h>

namespace log4cxx
{
/**
SimpleLayout consists of the level of the log statement,
followed by " - " and then the log message itself. For example,

<pre>
        DEBUG - Hello world
</pre>

<p>

<p>PatternLayout offers a much more powerful alternative.
*/
class LOG4CXX_EXPORT SimpleLayout : public Layout
{
	public:
		DECLARE_LOG4CXX_OBJECT(SimpleLayout)
		BEGIN_LOG4CXX_CAST_MAP()
		LOG4CXX_CAST_ENTRY(SimpleLayout)
		LOG4CXX_CAST_ENTRY_CHAIN(Layout)
		END_LOG4CXX_CAST_MAP()

		/**
		Returns the log statement in a format consisting of the
		<code>level</code>, followed by " - " and then the
		<code>message</code>. For example, <pre> INFO - "A message"
		</pre>

		@return A byte array in SimpleLayout format.
		*/
		void format(LogString& output,
			const spi::LoggingEventPtr& event,
			helpers::Pool& pool) const override;

		/**
		The SimpleLayout does not handle the throwable contained within
		{@link spi::LoggingEvent LoggingEvents}. Thus, it returns
		<code>true</code>.
		*/
		bool ignoresThrowable() const override
		{
			return true;
		}

		void activateOptions(helpers::Pool& /* p */) override {}
		void setOption(const LogString& /* option */,
			const LogString& /* value */) override {}
};
LOG4CXX_PTR_DEF(SimpleLayout);
}  // namespace log4cxx

#endif //_LOG4CXX_SIMPLE_LAYOUT_H
