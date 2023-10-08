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

#ifndef LOG4CXX_HELPERS_APRSOCKET_H
#define LOG4CXX_HELPERS_APRSOCKET_H

#include <log4cxx/helpers/socket.h>

struct apr_socket_t;

namespace log4cxx
{
namespace helpers
{

class LOG4CXX_EXPORT APRSocket : public helpers::Socket
{
	public:
		/** Creates a stream socket and connects it to the specified port
		number at the specified IP address.
		*/
		APRSocket(InetAddressPtr& address, int port);
		APRSocket(apr_socket_t*, apr_pool_t* pool);

		virtual size_t write(ByteBuffer&);

		/** Closes this socket. */
		virtual void close();

	private:
		struct APRSocketPriv;
};

}
}

#endif /* LOG4CXX_HELPERS_APRSOCKET_H */
