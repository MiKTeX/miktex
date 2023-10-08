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

#ifndef _LOG4CXX_HELPERS_SERVER_SOCKET_H
#define _LOG4CXX_HELPERS_SERVER_SOCKET_H

#include <log4cxx/helpers/socket.h>
#include <mutex>

namespace log4cxx
{
namespace helpers
{

class ServerSocket;
LOG4CXX_PTR_DEF(ServerSocket);
LOG4CXX_UNIQUE_PTR_DEF(ServerSocket);

class LOG4CXX_EXPORT ServerSocket
{
	protected:
		LOG4CXX_DECLARE_PRIVATE_MEMBER_PTR(ServerSocketPrivate, m_priv)
		ServerSocket(LOG4CXX_PRIVATE_PTR(ServerSocketPrivate) priv);

	public:

		virtual ~ServerSocket();

		/** Listens for a connection to be made to this socket and
		accepts it
		*/
		virtual SocketPtr accept() = 0;

		/** Closes this socket.
		*/
		virtual void close();

		/** Retrive setting for SO_TIMEOUT.
		*/
		int getSoTimeout() const;

		/** Enable/disable SO_TIMEOUT with the specified timeout, in milliseconds.
		*/
		void setSoTimeout(int timeout);

		static ServerSocketUniquePtr create(int port);

};
}  // namespace helpers
} // namespace log4cxx

#endif //_LOG4CXX_HELPERS_SERVER_SOCKET_H
