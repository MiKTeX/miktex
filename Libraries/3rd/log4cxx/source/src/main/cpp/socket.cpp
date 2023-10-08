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
#include <log4cxx/helpers/socket.h>
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/helpers/transcoder.h>

#include <log4cxx/private/socket_priv.h>
#include <log4cxx/private/aprsocket.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(Socket)

Socket::Socket(std::unique_ptr<Socket::SocketPrivate> priv) :
	m_priv(std::move(priv)){

}

Socket::~Socket()
{
}

InetAddressPtr Socket::getInetAddress() const
{
	return m_priv->address;
}

int Socket::getPort() const
{
	return m_priv->port;
}

SocketUniquePtr Socket::create(InetAddressPtr& address, int port){
	return std::make_unique<APRSocket>(address, port);
}


