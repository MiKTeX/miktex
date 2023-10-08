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
#include <log4cxx/helpers/datagramsocket.h>
#include <log4cxx/helpers/datagrampacket.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/private/datagramsocket_priv.h>
#include <log4cxx/private/aprdatagramsocket.h>

#include "apr_network_io.h"
#include "apr_lib.h"

using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(DatagramSocket)

DatagramSocket::DatagramSocket(std::unique_ptr<DatagramSocketPriv> priv) :
	m_priv(std::move(priv))
{}

DatagramSocket::~DatagramSocket()
{
	try
	{
		close();
	}
	catch (SocketException&)
	{
	}
}

/** Close the socket.*/
void DatagramSocket::close()
{
}

InetAddressPtr DatagramSocket::getInetAddress() const
{
	return m_priv->address;
}

InetAddressPtr DatagramSocket::getLocalAddress() const
{
	return m_priv->localAddress;
}

int DatagramSocket::getLocalPort() const
{
	return m_priv->localPort;
}

int DatagramSocket::getPort() const
{
	return m_priv->port;
}

bool DatagramSocket::isBound() const
{
	return m_priv->localPort != 0;
}

bool DatagramSocket::isConnected() const
{
	return m_priv->port != 0;
}

DatagramSocketUniquePtr DatagramSocket::create(){
	return std::make_unique<APRDatagramSocket>();
}

DatagramSocketUniquePtr DatagramSocket::create(int localPort1){
	std::unique_ptr<APRDatagramSocket> sock = std::make_unique<APRDatagramSocket>();
	InetAddressPtr bindAddr = InetAddress::anyAddress();

	sock->bind(localPort1, bindAddr);
	return sock;
}

DatagramSocketUniquePtr DatagramSocket::create(int localPort1, InetAddressPtr localAddress1){
	std::unique_ptr<APRDatagramSocket> sock = std::make_unique<APRDatagramSocket>();

	sock->bind(localPort1, localAddress1);
	return sock;
}
