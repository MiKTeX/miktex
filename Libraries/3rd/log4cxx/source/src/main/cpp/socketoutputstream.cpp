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
#include <log4cxx/helpers/socketoutputstream.h>
#include <log4cxx/helpers/socket.h>
#include <log4cxx/helpers/bytebuffer.h>

#include <cstdio>
#include <cstring>

using namespace log4cxx;
using namespace log4cxx::helpers;

struct SocketOutputStream::SocketOutputStreamPrivate
{
	ByteList array;
	SocketPtr socket;
};

IMPLEMENT_LOG4CXX_OBJECT(SocketOutputStream)

SocketOutputStream::SocketOutputStream(const SocketPtr& socket1)
	: m_priv(std::make_unique<SocketOutputStreamPrivate>())
{
	m_priv->socket = socket1;
}

SocketOutputStream::~SocketOutputStream()
{
}

void SocketOutputStream::close(Pool& p)
{
	flush(p);
	m_priv->socket->close();
}

void SocketOutputStream::flush(Pool& /* p */)
{
	if (m_priv->array.size() > 0)
	{
		ByteBuffer buf((char*) &m_priv->array[0], m_priv->array.size());
		m_priv->socket->write(buf);
		m_priv->array.resize(0);
	}
}

void SocketOutputStream::write(ByteBuffer& buf, Pool& /* p */ )
{
	if (buf.remaining() > 0)
	{
		size_t sz = m_priv->array.size();
		m_priv->array.resize(sz + buf.remaining());
		memcpy(&m_priv->array[sz], buf.current(), buf.remaining());
		buf.position(buf.limit());
	}
}


