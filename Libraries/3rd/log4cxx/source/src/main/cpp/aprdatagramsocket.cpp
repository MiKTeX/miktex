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

#include <log4cxx/private/aprdatagramsocket.h>
#include <log4cxx/private/datagramsocket_priv.h>
#include <log4cxx/helpers/transcoder.h>
#include <apr_network_io.h>

namespace log4cxx
{
namespace helpers
{

#define _priv static_cast<APRDatagramSocketPriv*>(m_priv.get())

struct APRDatagramSocket::APRDatagramSocketPriv : public DatagramSocketPriv {
	APRDatagramSocketPriv() :
		DatagramSocketPriv(),
		socket(nullptr)
	{}

	APRDatagramSocketPriv(int port) :
		DatagramSocketPriv(port),
		socket(nullptr)
	{}

	APRDatagramSocketPriv(int port, InetAddressPtr localAddress) :
		DatagramSocketPriv(port, localAddress),
		socket(nullptr)
	{}

	/** The APR socket */
	apr_socket_t* socket;

	/** The memory pool for the socket */
	Pool socketPool;
};

APRDatagramSocket::APRDatagramSocket() :
	DatagramSocket(std::make_unique<APRDatagramSocketPriv>()){
	init();
}

APRDatagramSocket::APRDatagramSocket(int port) :
	DatagramSocket(std::make_unique<APRDatagramSocketPriv>(port)){
	init();
}

APRDatagramSocket::APRDatagramSocket(int port, InetAddressPtr laddr) :
	DatagramSocket(std::make_unique<APRDatagramSocketPriv>(port, laddr)){
	init();
}

void APRDatagramSocket::init()
{
	apr_socket_t* newSocket;
	apr_status_t status =
		apr_socket_create(&newSocket, APR_INET, SOCK_DGRAM,
			APR_PROTO_UDP, _priv->socketPool.getAPRPool());
	_priv->socket = newSocket;

	if (status != APR_SUCCESS)
	{
		throw SocketException(status);
	}
}

void APRDatagramSocket::receive(DatagramPacketPtr& p)
{
	Pool addrPool;

	// Create the address from which to receive the datagram packet
	LOG4CXX_ENCODE_CHAR(hostAddr, p->getAddress()->getHostAddress());
	apr_sockaddr_t* addr;
	apr_status_t status =
		apr_sockaddr_info_get(&addr, hostAddr.c_str(), APR_INET,
			p->getPort(), 0, addrPool.getAPRPool());

	if (status != APR_SUCCESS)
	{
		throw SocketException(status);
	}

	// receive the datagram packet
	apr_size_t len = p->getLength();
	status = apr_socket_recvfrom(addr, _priv->socket, 0,
			(char*)p->getData(), &len);

	if (status != APR_SUCCESS)
	{
		throw IOException(status);
	}
}

void APRDatagramSocket::send(DatagramPacketPtr& p)
{
	Pool addrPool;

	// create the adress to which to send the datagram packet
	LOG4CXX_ENCODE_CHAR(hostAddr, p->getAddress()->getHostAddress());
	apr_sockaddr_t* addr;
	apr_status_t status =
		apr_sockaddr_info_get(&addr, hostAddr.c_str(), APR_INET, p->getPort(),
			0, addrPool.getAPRPool());

	if (status != APR_SUCCESS)
	{
		throw SocketException(status);
	}

	// send the datagram packet
	apr_size_t len = p->getLength();
	status = apr_socket_sendto(_priv->socket, addr, 0,
			(char*)p->getData(), &len);

	if (status != APR_SUCCESS)
	{
		throw IOException(status);
	}
}

void APRDatagramSocket::close(){
	if (_priv->socket != 0)
	{
		apr_status_t status = apr_socket_close(_priv->socket);

		if (status != APR_SUCCESS)
		{
			throw SocketException(status);
		}

		_priv->socket = 0;
		_priv->localPort = 0;
	}
}

void APRDatagramSocket::bind(int localPort1, InetAddressPtr localAddress1)
{
	Pool addrPool;

	// Create server socket address (including port number)
	LOG4CXX_ENCODE_CHAR(hostAddr, localAddress1->getHostAddress());
	apr_sockaddr_t* server_addr;
	apr_status_t status =
		apr_sockaddr_info_get(&server_addr, hostAddr.c_str(), APR_INET,
			localPort1, 0, addrPool.getAPRPool());

	if (status != APR_SUCCESS)
	{
		throw BindException(status);
	}

	// bind the socket to the address
	status = apr_socket_bind(_priv->socket, server_addr);

	if (status != APR_SUCCESS)
	{
		throw BindException(status);
	}

	m_priv->localPort = localPort1;
	m_priv->localAddress = localAddress1;
}


void APRDatagramSocket::connect(InetAddressPtr address1, int port1)
{
	m_priv->address = address1;
	m_priv->port = port1;

	Pool addrPool;

	// create socket address
	LOG4CXX_ENCODE_CHAR(hostAddr, address1->getHostAddress());
	apr_sockaddr_t* client_addr;
	apr_status_t status =
		apr_sockaddr_info_get(&client_addr, hostAddr.c_str(), APR_INET,
			m_priv->port, 0, addrPool.getAPRPool());

	if (status != APR_SUCCESS)
	{
		throw ConnectException(status);
	}

	// connect the socket
	status = apr_socket_connect(_priv->socket, client_addr);

	if (status != APR_SUCCESS)
	{
		throw ConnectException(status);
	}
}


bool APRDatagramSocket::isClosed() const
{
	return _priv->socket != nullptr;
}

} //namespace helpers
} //namespace log4cxx
