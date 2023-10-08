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

#ifndef _LOG4CXX_NET_SYSLOG_APPENDER_H
#define _LOG4CXX_NET_SYSLOG_APPENDER_H

#include <log4cxx/appenderskeleton.h>
#include <log4cxx/helpers/syslogwriter.h>

namespace log4cxx
{
namespace net
{
/**
 * Use SyslogAppender to send log messages to a remote syslog daemon.
 *
 * Note that by default, this appender will split up messages that are
 * more than 1024 bytes long, for compatability with BSD syslog(see
 * RFC 3164).  Modern syslog implementations(e.g. syslog-ng) can accept
 * messages much larger, and may have a default of 8k.  You may modify
 * the default size of the messages by setting the MaxMessageLength option.
 *
 * When the message is too large for the current MaxMessageLength,
 * the packet number and total # will be appended to the end of the
 * message like this: (5/10)
 */
class LOG4CXX_EXPORT SyslogAppender : public AppenderSkeleton
{
	public:
		DECLARE_LOG4CXX_OBJECT(SyslogAppender)
		BEGIN_LOG4CXX_CAST_MAP()
		LOG4CXX_CAST_ENTRY(SyslogAppender)
		LOG4CXX_CAST_ENTRY_CHAIN(AppenderSkeleton)
		END_LOG4CXX_CAST_MAP()



		SyslogAppender();
		SyslogAppender(const LayoutPtr& layout, int syslogFacility);
		SyslogAppender(const LayoutPtr& layout,
			const LogString& syslogHost, int syslogFacility);
		~SyslogAppender();
		/** Release any resources held by this SyslogAppender.*/
		void close() override;

		/**
		Returns the specified syslog facility as a lower-case String,
		e.g. "kern", "user", etc.
		*/
		static LogString getFacilityString(int syslogFacility);

		/**
		Returns the integer value corresponding to the named syslog
		facility, or -1 if it couldn't be recognized.
		@param facilityName one of the strings KERN, USER, MAIL, DAEMON,
		AUTH, SYSLOG, LPR, NEWS, UUCP, CRON, AUTHPRIV, FTP, LOCAL0,
		LOCAL1, LOCAL2, LOCAL3, LOCAL4, LOCAL5, LOCAL6, LOCAL7.
		The matching is case-insensitive.
		*/
		static int getFacility(const LogString& facilityName);

		void append(const spi::LoggingEventPtr& event, helpers::Pool& p) override;

		/**
		This method returns immediately as options are activated when they
		are set.
		*/
		void activateOptions(helpers::Pool& p) override;
		void setOption(const LogString& option, const LogString& value) override;

		/**
		The SyslogAppender requires a layout. Hence, this method returns
		<code>true</code>.
		*/
		bool requiresLayout() const override
		{
			return true;
		}

		/**
		The <b>SyslogHost</b> option is the name of the the syslog host
		where log output should go.
		<b>WARNING</b> If the SyslogHost is not set, then this appender
		will fail.
		*/
		void setSyslogHost(const LogString& syslogHost);

		/**
		Returns the value of the <b>SyslogHost</b> option.
		*/
		const LogString& getSyslogHost() const;

		/**
		Set the syslog facility. This is the <b>Facility</b> option.

		<p>The <code>facilityName</code> parameter must be one of the
		strings KERN, USER, MAIL, DAEMON, AUTH, SYSLOG, LPR, NEWS, UUCP,
		CRON, AUTHPRIV, FTP, LOCAL0, LOCAL1, LOCAL2, LOCAL3, LOCAL4,
		LOCAL5, LOCAL6, LOCAL7. Case is unimportant.
		*/
		void setFacility(const LogString& facilityName);

		/**
		Returns the value of the <b>Facility</b> option.
		*/
		LogString getFacility() const;

		/**
		If the <b>FacilityPrinting</b> option is set to true, the printed
		message will include the facility name of the application. It is
		<em>false</em> by default.
		*/
		void setFacilityPrinting(bool facilityPrinting1);

		/**
		Returns the value of the <b>FacilityPrinting</b> option.
		*/
		bool getFacilityPrinting() const;

		void setMaxMessageLength(int maxMessageLength1);

		int getMaxMessageLength() const;

	protected:
		void initSyslogFacilityStr();

	private:
		struct SyslogAppenderPriv;
		SyslogAppender(const SyslogAppender&);
		SyslogAppender& operator=(const SyslogAppender&);
}; // class SyslogAppender
LOG4CXX_PTR_DEF(SyslogAppender);
} // namespace net
} // namespace log4cxx

#endif // _LOG4CXX_NET_SYSLOG_APPENDER_H

