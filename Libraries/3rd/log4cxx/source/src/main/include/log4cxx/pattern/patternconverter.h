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

#ifndef _LOG4CXX_PATTERN_PATTERN_CONVERTER_H
#define _LOG4CXX_PATTERN_PATTERN_CONVERTER_H


#include <log4cxx/helpers/object.h>
#include <log4cxx/logstring.h>
#include <vector>

#define DECLARE_LOG4CXX_PATTERN(cls) DECLARE_ABSTRACT_LOG4CXX_OBJECT(cls)

namespace log4cxx
{
namespace pattern
{

typedef std::vector<LogString> OptionsList;

/**

   <p>PatternConverter is an abstract class that provides the
   formatting functionality that derived classes need.

   <p>Conversion specifiers in a conversion patterns are parsed to
   individual PatternConverters. Each of which is responsible for
   converting an object in a converter specific manner.

 */
class LOG4CXX_EXPORT PatternConverter : public virtual helpers::Object
{
	protected:
		LOG4CXX_DECLARE_PRIVATE_MEMBER_PTR(PatternConverterPrivate, m_priv)
		PatternConverter(LOG4CXX_PRIVATE_PTR(PatternConverterPrivate) priv);

		/**
		 * Create a new pattern converter.  Use this constructor when you have a subclass
		 * that does not have any private data.
		 * @param name name for pattern converter.
		 * @param style CSS style for formatted output.
		 */
		PatternConverter(const LogString& name,
			const LogString& style);

		virtual ~PatternConverter();

	public:
        DECLARE_LOG4CXX_PATTERN(PatternConverter)
        BEGIN_LOG4CXX_CAST_MAP()
        LOG4CXX_CAST_ENTRY(PatternConverter)
        END_LOG4CXX_CAST_MAP()

		/**
		 * Formats an object into a string buffer.
		 * @param obj event to format, may not be null.
		 * @param toAppendTo string buffer to which the formatted event will be appended.  May not be null.
		 * @param p pool for any allocations necessary during formatting.
		 */
		virtual void format(const helpers::ObjectPtr& obj,
			LogString& toAppendTo,
			helpers::Pool& p) const = 0;

		/**
		 * This method returns the name of the conversion pattern.
		 *
		 * The name can be useful to certain Layouts such as HTMLLayout.
		 *
		 * @return        the name of the conversion pattern
		 */
		LogString getName() const;

		/**
		 * This method returns the CSS style class that should be applied to
		 * the LoggingEvent passed as parameter, which can be null.
		 *
		 * This information is currently used only by HTMLLayout.
		 *
		 * @param e null values are accepted
		 * @return  the name of the conversion pattern
		 */
		virtual LogString getStyleClass(const helpers::ObjectPtr& e) const;

	protected:
		/**
		* Appends content in the locale code page to a LogString.
		* @param toAppendTo string to which content is appended.
		* @param src content.
		*/
		static void append(LogString& toAppendTo, const std::string& src);
};


LOG4CXX_PTR_DEF(PatternConverter);

}
}


#endif
