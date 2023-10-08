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
#include <log4cxx/helpers/cyclicbuffer.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/stringhelper.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;

struct CyclicBuffer::CyclicBufferPriv
{
	CyclicBufferPriv(int maxSize1) :
		ea(maxSize1), first(0), last(0), numElems(0), maxSize(maxSize1) {}

	log4cxx::spi::LoggingEventList ea;
	int first;
	int last;
	int numElems;
	int maxSize;
};

/**
Instantiate a new CyclicBuffer of at most <code>maxSize</code> events.
The <code>maxSize</code> argument must a positive integer.
@param maxSize The maximum number of elements in the buffer.
*/
CyclicBuffer::CyclicBuffer(int maxSize1)
	: m_priv(std::make_unique<CyclicBufferPriv>(maxSize1))
{
	if (maxSize1 < 1)
	{
		LogString msg(LOG4CXX_STR("The maxSize argument ("));
		Pool p;
		StringHelper::toString(maxSize1, p, msg);
		msg.append(LOG4CXX_STR(") is not a positive integer."));
		throw IllegalArgumentException(msg);
	}
}

CyclicBuffer::~CyclicBuffer()
{
}

/**
Add an <code>event</code> as the last event in the buffer.
*/
void CyclicBuffer::add(const spi::LoggingEventPtr& event)
{
	m_priv->ea[m_priv->last] = event;

	if (++m_priv->last == m_priv->maxSize)
	{
		m_priv->last = 0;
	}

	if (m_priv->numElems < m_priv->maxSize)
	{
		m_priv->numElems++;
	}
	else if (++m_priv->first == m_priv->maxSize)
	{
		m_priv->first = 0;
	}
}


/**
Get the <i>i</i>th oldest event currently in the buffer. If
<em>i</em> is outside the range 0 to the number of elements
currently in the buffer, then <code>null</code> is returned.
*/
spi::LoggingEventPtr CyclicBuffer::get(int i)
{
	if (i < 0 || i >= m_priv->numElems)
	{
		return 0;
	}

	return m_priv->ea[(m_priv->first + i) % m_priv->maxSize];
}

/**
Get the oldest (first) element in the buffer. The oldest element
is removed from the buffer.
*/
spi::LoggingEventPtr CyclicBuffer::get()
{
	LoggingEventPtr r;

	if (m_priv->numElems > 0)
	{
		m_priv->numElems--;
		r = m_priv->ea[m_priv->first];
		m_priv->ea[m_priv->first] = 0;

		if (++m_priv->first == m_priv->maxSize)
		{
			m_priv->first = 0;
		}
	}

	return r;
}

/**
Resize the cyclic buffer to <code>newSize</code>.
@throws IllegalArgumentException if <code>newSize</code> is negative.
*/
void CyclicBuffer::resize(int newSize)
{
	if (newSize < 0)
	{
		LogString msg(LOG4CXX_STR("Negative array size ["));
		Pool p;
		StringHelper::toString(newSize, p, msg);
		msg.append(LOG4CXX_STR("] not allowed."));
		throw IllegalArgumentException(msg);
	}

	if (newSize == m_priv->numElems)
	{
		return;    // nothing to do
	}

	LoggingEventList temp(newSize);

	int loopLen = newSize < m_priv->numElems ? newSize : m_priv->numElems;
	int i;

	for (i = 0; i < loopLen; i++)
	{
		temp[i] = m_priv->ea[m_priv->first];
		m_priv->ea[m_priv->first] = 0;

		if (++m_priv->first == m_priv->numElems)
		{
			m_priv->first = 0;
		}
	}

	m_priv->ea = temp;
	m_priv->first = 0;
	m_priv->numElems = loopLen;
	m_priv->maxSize = newSize;

	if (loopLen == newSize)
	{
		m_priv->last = 0;
	}
	else
	{
		m_priv->last = loopLen;
	}
}

int CyclicBuffer::getMaxSize() const
{
	return m_priv->maxSize;
}

int CyclicBuffer::length() const
{
	return m_priv->numElems;
}
