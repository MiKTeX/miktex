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

#include <log4cxx/helpers/serversocket.h>
#include <log4cxx/private/serversocket_priv.h>
#include <log4cxx/private/aprserversocket.h>

using namespace log4cxx::helpers;

/**  Creates a server socket on a specified port.
*/
ServerSocket::ServerSocket(std::unique_ptr<ServerSocketPrivate> priv) :
	m_priv(std::move(priv))
{
}

ServerSocket::~ServerSocket()
{
	close();
}

void ServerSocket::close()
{
}

/** Retrive setting for SO_TIMEOUT.
*/
int ServerSocket::getSoTimeout() const
{
	return m_priv->timeout;
}

/** Enable/disable SO_TIMEOUT with the specified timeout, in milliseconds.
*/
void ServerSocket::setSoTimeout(int newVal)
{
	m_priv->timeout = newVal;
}

ServerSocketUniquePtr ServerSocket::create(int port){
	return std::make_unique<APRServerSocket>(port);
}
