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

#ifndef _LOG4CXX_JSON_LAYOUT_H
#define _LOG4CXX_JSON_LAYOUT_H

#include <log4cxx/layout.h>
#include <log4cxx/helpers/iso8601dateformat.h>
#include <log4cxx/spi/loggingevent.h>


namespace log4cxx
{
/**
This layout outputs events in a JSON dictionary.
*/
class LOG4CXX_EXPORT JSONLayout : public Layout
{
	private:
		LOG4CXX_DECLARE_PRIVATE_MEMBER_PTR(JSONLayoutPrivate, m_priv)

	protected:
		void appendQuotedEscapedString(LogString& buf, const LogString& input) const;
		void appendSerializedMDC(LogString& buf,
			const spi::LoggingEventPtr& event) const;
		void appendSerializedNDC(LogString& buf,
			const spi::LoggingEventPtr& event) const;
		void appendSerializedLocationInfo(LogString& buf,
			const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p) const;

	public:
		static void appendItem(const LogString& item, LogString& toAppendTo);
		DECLARE_LOG4CXX_OBJECT(JSONLayout)
		BEGIN_LOG4CXX_CAST_MAP()
		LOG4CXX_CAST_ENTRY(JSONLayout)
		LOG4CXX_CAST_ENTRY_CHAIN(Layout)
		END_LOG4CXX_CAST_MAP()

		JSONLayout();

		~JSONLayout();

		/**
		The <b>LocationInfo</b> option takes a boolean value. By
		default, it is set to false which means there will be no location
		information output by this layout. If the the option is set to
		true, then the file name and line number of the statement
		at the origin of the log statement will be output.
		*/
		void setLocationInfo(bool locationInfoFlag);


		/**
		Returns the current value of the <b>LocationInfo</b> option.
		*/
		bool getLocationInfo() const;

		/**
		The <b>PrettyPrint</b> option takes a boolean value. By
		default, it is set to false which means output by this layout will
		be one line per log event.  If the option is set to true, then
		then each log event will produce multiple lines, each indented
		for readability.
		*/
		void setPrettyPrint(bool prettyPrintFlag);

		/**
		Returns the current value of the <b>PrettyPrint</b> option.
		*/
		bool getPrettyPrint() const;


		/**
		Returns the content type output by this layout, i.e "application/json".
		*/
		LogString getContentType() const override;

		void activateOptions(helpers::Pool& /* p */) override;

		/**
		Set options
		*/
		void setOption(const LogString& option, const LogString& value) override;

		void format(LogString& output,
			const spi::LoggingEventPtr& event, helpers::Pool& pool) const override;

		/**
		The JSON layout handles the throwable contained in logging
		events. Hence, this method return <code>false</code>.  */
		bool ignoresThrowable() const override
		{
			return false;
		}

}; // class JSONLayout
LOG4CXX_PTR_DEF(JSONLayout);
}  // namespace log4cxx

#endif // _LOG4CXX_JSON_LAYOUT_H
