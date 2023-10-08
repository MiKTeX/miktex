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

#include <log4cxx/logstring.h>
#include <log4cxx/helpers/syslogwriter.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/inetaddress.h>
#include <log4cxx/helpers/datagramsocket.h>
#include <log4cxx/helpers/datagrampacket.h>
#include <log4cxx/helpers/transcoder.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

struct SyslogWriter::SyslogWriterPrivate {
	SyslogWriterPrivate(const LogString& syslogHost1, int syslogHostPort1)
		: syslogHost(syslogHost1), syslogHostPort(syslogHostPort1){}

	LogString syslogHost;
	int syslogHostPort;
	InetAddressPtr address;
	DatagramSocketPtr ds;
};

SyslogWriter::SyslogWriter(const LogString& syslogHost1, int syslogHostPort1)
	: m_priv(std::make_unique<SyslogWriterPrivate>(syslogHost1, syslogHostPort1))
{
	try
	{
		m_priv->address = InetAddress::getByName(syslogHost1);
	}
	catch (UnknownHostException& e)
	{
		LogLog::error(((LogString) LOG4CXX_STR("Could not find ")) + syslogHost1 +
			LOG4CXX_STR(". All logging will FAIL."), e);
	}

	try
	{
		m_priv->ds = DatagramSocket::create();
	}
	catch (SocketException& e)
	{
		LogLog::error(((LogString) LOG4CXX_STR("Could not instantiate DatagramSocket to ")) + syslogHost1 +
			LOG4CXX_STR(". All logging will FAIL."), e);
	}
}

SyslogWriter::~SyslogWriter(){}

void SyslogWriter::write(const LogString& source)
{
	if (m_priv->ds != 0 && m_priv->address != 0)
	{
		LOG4CXX_ENCODE_CHAR(data, source);

		auto packet = std::make_shared<DatagramPacket>(
				(void*) data.data(), (int)data.length(),
				m_priv->address, m_priv->syslogHostPort);

		m_priv->ds->send(packet);
	}
}
