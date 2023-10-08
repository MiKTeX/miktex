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

#include <log4cxx/private/aprsocket.h>
#include <log4cxx/private/socket_priv.h>
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/helpers/transcoder.h>

#include "apr_network_io.h"
#include "apr_signal.h"

namespace log4cxx
{
namespace helpers
{

struct APRSocket::APRSocketPriv : public Socket::SocketPrivate {
	APRSocketPriv() :
		socket(nullptr)
	{}

	APRSocketPriv(apr_socket_t* sock, apr_pool_t* p) :
		pool(p, true),
		socket(sock)
	{}

	Pool pool;
	apr_socket_t* socket;
};

#define _priv static_cast<APRSocketPriv*>(m_priv.get())

APRSocket::APRSocket(InetAddressPtr& address, int port) :
	Socket(std::make_unique<APRSocketPriv>()){
	apr_status_t status =
		apr_socket_create(&_priv->socket, APR_INET, SOCK_STREAM,
			APR_PROTO_TCP, _priv->pool.getAPRPool());

	if (status != APR_SUCCESS)
	{
		throw SocketException(status);
	}

	LOG4CXX_ENCODE_CHAR(host, address->getHostAddress());

	// create socket address (including port)
	apr_sockaddr_t* client_addr;
	status =
		apr_sockaddr_info_get(&client_addr, host.c_str(), APR_INET,
			port, 0, _priv->pool.getAPRPool());

	if (status != APR_SUCCESS)
	{
		throw ConnectException(status);
	}

	// connect the socket
	status =  apr_socket_connect(_priv->socket, client_addr);

	if (status != APR_SUCCESS)
	{
		throw ConnectException(status);
	}
}

APRSocket::APRSocket(apr_socket_t* s, apr_pool_t* pool) :
	Socket(std::make_unique<APRSocketPriv>(s, pool)){
	apr_sockaddr_t* sa;
	apr_status_t status = apr_socket_addr_get(&sa, APR_REMOTE, s);

	if (status == APR_SUCCESS)
	{
		_priv->port = sa->port;
		LogString remotename;
		LogString remoteip;

		if (sa->hostname != NULL)
		{
			Transcoder::decode(sa->hostname, remotename);
		}

		char* buf = 0;
		status = apr_sockaddr_ip_get(&buf, sa);

		if (status == APR_SUCCESS)
		{
			Transcoder::decode(buf, remoteip);
		}

		_priv->address = std::make_shared<InetAddress>(remotename, remoteip);
	}
}

size_t APRSocket::write(ByteBuffer& buf)
{
	if (_priv->socket == 0)
	{
		throw ClosedChannelException();
	}

	size_t totalWritten = 0;

	while (buf.remaining() > 0)
	{
		apr_size_t written = buf.remaining();

		// while writing to the socket, we need to ignore the SIGPIPE
		// signal. Otherwise, when the client has closed the connection,
		// the send() function would not return an error but call the
		// SIGPIPE handler.
#if APR_HAVE_SIGACTION
		apr_sigfunc_t* old = apr_signal(SIGPIPE, SIG_IGN);
		apr_status_t status = apr_socket_send(_priv->socket, buf.current(), &written);
		apr_signal(SIGPIPE, old);
#else
		apr_status_t status = apr_socket_send(_priv->socket, buf.current(), &written);
#endif

		buf.position(buf.position() + written);
		totalWritten += written;

		if (status != APR_SUCCESS)
		{
			throw SocketException(status);
		}
	}

	return totalWritten;
}


void APRSocket::close()
{
	if (_priv->socket != 0)
	{
		apr_status_t status = apr_socket_close(_priv->socket);

		if (status != APR_SUCCESS)
		{
			throw SocketException(status);
		}

		_priv->socket = 0;
	}
}

} //namespace helpers
} //namespace log4cxx
