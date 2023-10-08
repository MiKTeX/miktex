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

#define __STDC_CONSTANT_MACROS
#include <log4cxx/net/socketappenderskeleton.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/bytearrayoutputstream.h>
#include <log4cxx/helpers/threadutility.h>
#include <log4cxx/private/appenderskeleton_priv.h>
#include <log4cxx/private/socketappenderskeleton_priv.h>
#include <functional>
#include <chrono>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::net;

#define _priv static_cast<SocketAppenderSkeletonPriv*>(m_priv.get())

SocketAppenderSkeleton::SocketAppenderSkeleton(int defaultPort, int reconnectionDelay)
    : AppenderSkeleton(std::make_unique<SocketAppenderSkeletonPriv>(defaultPort, reconnectionDelay))
{
}

SocketAppenderSkeleton::SocketAppenderSkeleton(helpers::InetAddressPtr address, int port, int reconnectionDelay)
    : AppenderSkeleton(std::make_unique<SocketAppenderSkeletonPriv>(address, port, reconnectionDelay))
{
}

SocketAppenderSkeleton::SocketAppenderSkeleton(const LogString& host, int port, int reconnectionDelay)
    : AppenderSkeleton(std::make_unique<SocketAppenderSkeletonPriv>(host, port, reconnectionDelay))
{
}

SocketAppenderSkeleton::SocketAppenderSkeleton(std::unique_ptr<SocketAppenderSkeletonPriv> priv)
	:  AppenderSkeleton (std::move(priv))
{
}

SocketAppenderSkeleton::~SocketAppenderSkeleton()
{
	finalize();
}

void SocketAppenderSkeleton::activateOptions(Pool& p)
{
	AppenderSkeleton::activateOptions(p);
	connect(p);
}

void SocketAppenderSkeleton::close()
{
	std::lock_guard<std::recursive_mutex> lock(_priv->mutex);

	if (_priv->closed)
	{
		return;
	}

	_priv->closed = true;
	cleanUp(_priv->pool);

	{
		std::unique_lock<std::mutex> lock2(_priv->interrupt_mutex);
		_priv->interrupt.notify_all();
	}

	if ( _priv->thread.joinable() )
	{
		_priv->thread.join();
	}
}

void SocketAppenderSkeleton::connect(Pool& p)
{
	if (_priv->address == 0)
	{
		LogLog::error(LogString(LOG4CXX_STR("No remote host is set for Appender named \"")) +
			_priv->name + LOG4CXX_STR("\"."));
	}
	else
	{
		cleanUp(p);

		try
		{
			SocketPtr socket = Socket::create(_priv->address, _priv->port);
			setSocket(socket, p);
		}
		catch (SocketException& e)
		{
			LogString msg = LOG4CXX_STR("Could not connect to [")
				+ _priv->address->getHostName() + LOG4CXX_STR(":");
			StringHelper::toString(_priv->port, p, msg);
			msg += LOG4CXX_STR("].");

			if (_priv->reconnectionDelay > 0)
			{
				msg += LOG4CXX_STR(" We will try again in ");
				StringHelper::toString(_priv->reconnectionDelay, p, msg);
				msg += LOG4CXX_STR(" ms");
			}

			fireConnector(); // fire the connector thread
			LogLog::error(msg, e);
		}
	}
}

void SocketAppenderSkeleton::setOption(const LogString& option, const LogString& value)
{
	if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("REMOTEHOST"), LOG4CXX_STR("remotehost")))
	{
		setRemoteHost(value);
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("PORT"), LOG4CXX_STR("port")))
	{
		setPort(OptionConverter::toInt(value, getDefaultPort()));
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("LOCATIONINFO"), LOG4CXX_STR("locationinfo")))
	{
		setLocationInfo(OptionConverter::toBoolean(value, false));
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("RECONNECTIONDELAY"), LOG4CXX_STR("reconnectiondelay")))
	{
		setReconnectionDelay(OptionConverter::toInt(value, getDefaultDelay()));
	}
	else
	{
		AppenderSkeleton::setOption(option, value);
	}
}

void SocketAppenderSkeleton::fireConnector()
{
	std::lock_guard<std::recursive_mutex> lock(_priv->mutex);

	if ( !_priv->thread.joinable() )
	{
		LogLog::debug(LOG4CXX_STR("Connector thread not alive: starting monitor."));

		_priv->thread = ThreadUtility::instance()->createThread( LOG4CXX_STR("SocketAppend"), &SocketAppenderSkeleton::monitor, this );
	}
}

void SocketAppenderSkeleton::monitor()
{
	SocketPtr socket;
	bool isClosed = _priv->closed;

	while (!isClosed)
	{
		try
		{
			if (!_priv->closed)
			{
				LogLog::debug(LogString(LOG4CXX_STR("Attempting connection to "))
					+ _priv->address->getHostName());
				socket = Socket::create(_priv->address, _priv->port);
				Pool p;
				setSocket(socket, p);
				LogLog::debug(LOG4CXX_STR("Connection established. Exiting connector thread."));
				return;
			}
		}
		catch (ConnectException&)
		{
			LogLog::debug(LOG4CXX_STR("Remote host ")
				+ _priv->address->getHostName()
				+ LOG4CXX_STR(" refused connection."));
		}
		catch (IOException& e)
		{
			LogString exmsg;
			log4cxx::helpers::Transcoder::decode(e.what(), exmsg);

			LogLog::debug(((LogString) LOG4CXX_STR("Could not connect to "))
				+ _priv->address->getHostName()
				+ LOG4CXX_STR(". Exception is ")
				+ exmsg);
		}

		std::unique_lock<std::mutex> lock( _priv->interrupt_mutex );
		_priv->interrupt.wait_for( lock, std::chrono::milliseconds( _priv->reconnectionDelay ),
			std::bind(&SocketAppenderSkeleton::is_closed, this) );

		isClosed = _priv->closed;
	}

	LogLog::debug(LOG4CXX_STR("Exiting Connector.run() method."));
}

bool SocketAppenderSkeleton::is_closed()
{
	return _priv->closed;
}

void SocketAppenderSkeleton::setRemoteHost(const LogString& host)
{
	_priv->address = helpers::InetAddress::getByName(host);
	_priv->remoteHost.assign(host);
}

const LogString& SocketAppenderSkeleton::getRemoteHost() const
{
	return _priv->remoteHost;
}

void SocketAppenderSkeleton::setPort(int port1)
{
	_priv->port = port1;
}

int SocketAppenderSkeleton::getPort() const
{
	return _priv->port;
}

void SocketAppenderSkeleton::setLocationInfo(bool locationInfo1)
{
	_priv->locationInfo = locationInfo1;
}

bool SocketAppenderSkeleton::getLocationInfo() const
{
	return _priv->locationInfo;
}

void SocketAppenderSkeleton::setReconnectionDelay(int reconnectionDelay1)
{
	_priv->reconnectionDelay = reconnectionDelay1;
}

int SocketAppenderSkeleton::getReconnectionDelay() const
{
	return _priv->reconnectionDelay;
}
