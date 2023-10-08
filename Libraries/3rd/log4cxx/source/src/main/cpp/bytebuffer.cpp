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
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/helpers/pool.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

struct ByteBuffer::ByteBufferPriv
{
	ByteBufferPriv(char* data1, size_t capacity) :
		base(data1), pos(0), lim(capacity), cap(capacity) {}

	char* base;
	size_t pos;
	size_t lim;
	size_t cap;
};

ByteBuffer::ByteBuffer(char* data1, size_t capacity)
	: m_priv(std::make_unique<ByteBufferPriv>(data1, capacity))
{
}

ByteBuffer::~ByteBuffer()
{
}

void ByteBuffer::clear()
{
	m_priv->lim = m_priv->cap;
	m_priv->pos = 0;
}

void ByteBuffer::flip()
{
	m_priv->lim = m_priv->pos;
	m_priv->pos = 0;
}

void ByteBuffer::position(size_t newPosition)
{
	if (newPosition < m_priv->lim)
	{
		m_priv->pos = newPosition;
	}
	else
	{
		m_priv->pos = m_priv->lim;
	}
}

void ByteBuffer::limit(size_t newLimit)
{
	if (newLimit > m_priv->cap)
	{
		throw IllegalArgumentException(LOG4CXX_STR("newLimit"));
	}

	m_priv->lim = newLimit;
}


bool ByteBuffer::put(char byte)
{
	if (m_priv->pos < m_priv->lim)
	{
		m_priv->base[m_priv->pos++] = byte;
		return true;
	}

	return false;
}

char* ByteBuffer::data()
{
	return m_priv->base;
}

const char* ByteBuffer::data() const
{
	return m_priv->base;
}

char* ByteBuffer::current()
{
	return m_priv->base + m_priv->pos;
}

const char* ByteBuffer::current() const
{
	return m_priv->base + m_priv->pos;
}

size_t ByteBuffer::limit() const
{
	return m_priv->lim;
}

size_t ByteBuffer::position() const
{
	return m_priv->pos;
}

size_t ByteBuffer::remaining() const
{
	return m_priv->lim - m_priv->pos;
}

