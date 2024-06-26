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

#ifndef _LOG4CXX_SYSLOG_WRITER_H
#define _LOG4CXX_SYSLOG_WRITER_H

#include <log4cxx/helpers/object.h>
#include <log4cxx/helpers/inetaddress.h>
#include <log4cxx/helpers/datagramsocket.h>

namespace log4cxx
{
namespace helpers
{
/**
SyslogWriter is a wrapper around the DatagramSocket class
it writes text to the specified host on the port 514 (UNIX syslog)
*/
class LOG4CXX_EXPORT SyslogWriter
{
	public:
#define SYSLOG_PORT 514
		SyslogWriter(const LogString& syslogHost, int syslogHostPort = SYSLOG_PORT);
		~SyslogWriter();
		void write(const LogString& string);

	private:
		LOG4CXX_DECLARE_PRIVATE_MEMBER_PTR(SyslogWriterPrivate, m_priv)
};
}  // namespace helpers
} // namespace log4cxx

#endif
