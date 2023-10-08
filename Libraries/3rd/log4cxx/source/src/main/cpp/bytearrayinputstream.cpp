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
#include <log4cxx/helpers/bytearrayinputstream.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/helpers/transcoder.h>
#include <algorithm>
#include <cstring>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace std;

struct ByteArrayInputStream::ByteArrayInputStreamPriv
{
	ByteArrayInputStreamPriv(const ByteList& bytes) :
		buf(bytes),
		pos(0) {}

	ByteList buf;
	size_t pos;
};

IMPLEMENT_LOG4CXX_OBJECT(ByteArrayInputStream)

ByteArrayInputStream::ByteArrayInputStream(const std::vector<unsigned char>& bytes) :
	m_priv(std::make_unique<ByteArrayInputStreamPriv>(bytes))
{
}



ByteArrayInputStream::~ByteArrayInputStream()
{
}


void ByteArrayInputStream::close()
{
}


int ByteArrayInputStream::read(ByteBuffer& dst)
{
	if (m_priv->pos >= m_priv->buf.size())
	{
		return -1;
	}
	else
	{
		size_t bytesCopied = min(dst.remaining(), m_priv->buf.size() - m_priv->pos);
		std::memcpy(dst.current(), &m_priv->buf[m_priv->pos], bytesCopied);
		m_priv->pos += bytesCopied;
		dst.position(dst.position() + bytesCopied);
		return (int)bytesCopied;
	}
}
