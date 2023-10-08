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

#include <log4cxx/net/xmlsocketappender.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/outputstreamwriter.h>
#include <log4cxx/helpers/charsetencoder.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/xml/xmllayout.h>
#include <log4cxx/level.h>
#include <log4cxx/helpers/transform.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/socketoutputstream.h>
#include <log4cxx/private/appenderskeleton_priv.h>
#include <log4cxx/private/socketappenderskeleton_priv.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::net;
using namespace log4cxx::xml;

struct XMLSocketAppender::XMLSocketAppenderPriv : public SocketAppenderSkeletonPriv
{
	XMLSocketAppenderPriv(int defaultPort, int reconnectionDelay) :
		SocketAppenderSkeletonPriv(defaultPort, reconnectionDelay) {}

	XMLSocketAppenderPriv(InetAddressPtr address, int defaultPort, int reconnectionDelay) :
		SocketAppenderSkeletonPriv( address, defaultPort, reconnectionDelay ) {}

	XMLSocketAppenderPriv(const LogString& host, int port, int delay) :
		SocketAppenderSkeletonPriv( host, port, delay ) {}

	log4cxx::helpers::WriterPtr writer;
};

IMPLEMENT_LOG4CXX_OBJECT(XMLSocketAppender)

#define _priv static_cast<XMLSocketAppenderPriv*>(m_priv.get())

// The default port number of remote logging server (4560)
int XMLSocketAppender::DEFAULT_PORT                 = 4560;

// The default reconnection delay (30000 milliseconds or 30 seconds).
int XMLSocketAppender::DEFAULT_RECONNECTION_DELAY   = 30000;

const int XMLSocketAppender::MAX_EVENT_LEN          = 1024;

XMLSocketAppender::XMLSocketAppender()
	: SocketAppenderSkeleton(std::make_unique<XMLSocketAppenderPriv>(DEFAULT_PORT, DEFAULT_RECONNECTION_DELAY))
{
	_priv->layout = std::make_shared<XMLLayout>();
}

XMLSocketAppender::XMLSocketAppender(InetAddressPtr address1, int port1)
	: SocketAppenderSkeleton(std::make_unique<XMLSocketAppenderPriv>(address1, port1, DEFAULT_RECONNECTION_DELAY))
{
	_priv->layout = std::make_shared<XMLLayout>();
	Pool p;
	activateOptions(p);
}

XMLSocketAppender::XMLSocketAppender(const LogString& host, int port1)
	: SocketAppenderSkeleton(std::make_unique<XMLSocketAppenderPriv>(host, port1, DEFAULT_RECONNECTION_DELAY))
{
	_priv->layout = std::make_shared<XMLLayout>();
	Pool p;
	activateOptions(p);
}

XMLSocketAppender::~XMLSocketAppender()
{
	finalize();
}


int XMLSocketAppender::getDefaultDelay() const
{
	return DEFAULT_RECONNECTION_DELAY;
}

int XMLSocketAppender::getDefaultPort() const
{
	return DEFAULT_PORT;
}

void XMLSocketAppender::setSocket(log4cxx::helpers::SocketPtr& socket, Pool& p)
{
	OutputStreamPtr os = std::make_shared<SocketOutputStream>(socket);
	CharsetEncoderPtr charset(CharsetEncoder::getUTF8Encoder());
	std::lock_guard<std::recursive_mutex> lock(_priv->mutex);
	_priv->writer = std::make_shared<OutputStreamWriter>(os, charset);
}

void XMLSocketAppender::cleanUp(Pool& p)
{
	if (_priv->writer)
	{
		try
		{
			_priv->writer->close(p);
			_priv->writer = nullptr;
		}
		catch (std::exception&)
		{
		}
	}
}

void XMLSocketAppender::append(const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p)
{
	if (_priv->writer)
	{
		LogString output;
		_priv->layout->format(output, event, p);

		try
		{
			_priv->writer->write(output, p);
			_priv->writer->flush(p);
		}
		catch (std::exception& e)
		{
			_priv->writer = nullptr;
			LogLog::warn(LOG4CXX_STR("Detected problem with connection: "), e);

			if (getReconnectionDelay() > 0)
			{
				fireConnector();
			}
		}
	}
}




