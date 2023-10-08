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

#ifndef _LOG4CXX_HELPERS_DATAGRAM_SOCKET_H
#define _LOG4CXX_HELPERS_DATAGRAM_SOCKET_H

#include <log4cxx/helpers/object.h>
#include <log4cxx/helpers/inetaddress.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/datagrampacket.h>

namespace log4cxx
{
namespace helpers
{

class DatagramSocket;
LOG4CXX_PTR_DEF(DatagramSocket);
LOG4CXX_UNIQUE_PTR_DEF(DatagramSocket);

/** This class represents a socket for sending and receiving
datagram packets.*/
class LOG4CXX_EXPORT DatagramSocket : public helpers::Object
{
	protected:
		LOG4CXX_DECLARE_PRIVATE_MEMBER_PTR(DatagramSocketPriv, m_priv)
		DatagramSocket(LOG4CXX_PRIVATE_PTR(DatagramSocketPriv) priv);

	public:
		DECLARE_ABSTRACT_LOG4CXX_OBJECT(DatagramSocket)
		BEGIN_LOG4CXX_CAST_MAP()
		LOG4CXX_CAST_ENTRY(DatagramSocket)
		END_LOG4CXX_CAST_MAP()

		/** ensure the socket is closed. */
		virtual ~DatagramSocket();

		/**  Binds a datagram socket to a local port and address.*/
		virtual void bind(int lport, InetAddressPtr laddress) = 0;

		/** Closes this datagram socket */
		virtual void close();

		/** Connects the socket to a remote address for this socket. */
		virtual void connect(InetAddressPtr address, int port) = 0;

		/** Returns the address to which this socket is connected. */
		InetAddressPtr getInetAddress() const;

		/** Gets the local address to which the socket is bound. */
		InetAddressPtr getLocalAddress() const;

		/**  Returns the port number on the local host to which this
		socket is bound. */
		int getLocalPort() const;

		/** Returns the port for this socket */
		int getPort() const;

		/** Returns the binding state of the socket. **/
		bool isBound() const;

		/** Returns wether the socket is closed or not. */
		virtual bool isClosed() const = 0;

		/** Returns the connection state of the socket. */
		bool isConnected() const;

		/**  Receives a datagram packet from this socket. */
		virtual void receive(DatagramPacketPtr& p) = 0;

		/** Sends a datagram packet from this socket. */
		virtual void  send(DatagramPacketPtr& p) = 0;

		/** Constructs a datagram socket and binds it to any available port
		on the local host machine.*/
		static DatagramSocketUniquePtr create();

		/** Constructs a datagram socket and binds it to the specified
		port on the local host machine. */
		static DatagramSocketUniquePtr create(int port);

		/**  Creates a datagram socket, bound to the specified local
		address. */
		static DatagramSocketUniquePtr create(int port, InetAddressPtr laddr);

	private:
		DatagramSocket(const DatagramSocket&);
		DatagramSocket& operator=(const DatagramSocket&);
};

}  // namespace helpers
} // namespace log4cxx

#endif //_LOG4CXX_HELPERS_DATAGRAM_SOCKET_H
