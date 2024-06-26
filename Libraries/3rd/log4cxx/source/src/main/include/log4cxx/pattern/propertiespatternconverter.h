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

#ifndef _LOG4CXX_PATTERN_PROPERTIES_PATTERN_CONVERTER
#define _LOG4CXX_PATTERN_PROPERTIES_PATTERN_CONVERTER

#include <log4cxx/pattern/loggingeventpatternconverter.h>

namespace log4cxx
{
namespace pattern
{


/**
 * Able to handle the contents of the LoggingEvent's Property bundle and either
 * output the entire contents of the properties in a similar format to the
 * java.util.Hashtable.toString(), or to output the value of a specific key
 * within the property bundle
 * when this pattern converter has the option set.
 *
 *
 *
 */
class LOG4CXX_EXPORT PropertiesPatternConverter
	: public LoggingEventPatternConverter
{
		struct PropertiesPatternConverterPrivate;

	public:
		DECLARE_LOG4CXX_PATTERN(PropertiesPatternConverter)
		BEGIN_LOG4CXX_CAST_MAP()
		LOG4CXX_CAST_ENTRY(PropertiesPatternConverter)
		LOG4CXX_CAST_ENTRY_CHAIN(LoggingEventPatternConverter)
		END_LOG4CXX_CAST_MAP()

		/**
		 * A Property bundle converter.
		 *
		 * @param name of the converter.
		 * @param option may be null.
		 */
		PropertiesPatternConverter(const LogString& name, const LogString& option);

		/**
		 * Obtains an instance of PropertiesPatternConverter.
		 * @param options options, may be null or first element contains name of property to format.
		 * @return instance of PropertiesPatternConverter.
		 */
		static PatternConverterPtr newInstance(
			const std::vector<LogString>& options);

		using LoggingEventPatternConverter::format;

		void format(const spi::LoggingEventPtr& event,
			LogString& toAppendTo,
			log4cxx::helpers::Pool& p) const override;
};
}
}
#endif

