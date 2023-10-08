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

#ifndef _LOG4CXX_PATTERN_COLOR_START_PATTERN_CONVERTER
#define _LOG4CXX_PATTERN_COLOR_START_PATTERN_CONVERTER

#include <log4cxx/pattern/loggingeventpatternconverter.h>

namespace log4cxx
{
namespace pattern
{


/**
 * Format the event's color start information.
 *
 *
 *
 */
class LOG4CXX_EXPORT ColorStartPatternConverter
	: public LoggingEventPatternConverter
{
	struct ColorPatternConverterPrivate;

	public:
		DECLARE_LOG4CXX_PATTERN(ColorStartPatternConverter)
		BEGIN_LOG4CXX_CAST_MAP()
		LOG4CXX_CAST_ENTRY(ColorStartPatternConverter)
		LOG4CXX_CAST_ENTRY_CHAIN(LoggingEventPatternConverter)
		END_LOG4CXX_CAST_MAP()

		ColorStartPatternConverter();

		/**
		 * Obtains an instance of pattern converter.
		 * @param options options, may be null.
		 * @return instance of pattern converter.
		 */
		static PatternConverterPtr newInstance(
			const std::vector<LogString>& options);

		using LoggingEventPatternConverter::format;

		void format(const spi::LoggingEventPtr& event,
			LogString& toAppendTo,
			helpers::Pool& p) const override;

		void setFatalColor(const LogString& color);
		void setErrorColor(const LogString& color);
		void setWarnColor(const LogString& color);
		void setInfoColor(const LogString& color);
		void setDebugColor(const LogString& color);
		void setTraceColor(const LogString& color);

	private:
		void parseColor(const LogString& color, LogString* result);
};

}
}
#endif
