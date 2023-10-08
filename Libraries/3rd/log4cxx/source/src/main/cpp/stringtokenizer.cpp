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
#include <log4cxx/helpers/stringtokenizer.h>
#include <log4cxx/helpers/exception.h>
#if !defined(LOG4CXX)
	#define LOG4CXX 1
#endif
#include <log4cxx/private/log4cxx_private.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

struct StringTokenizer::StringTokenizerPrivate{
	StringTokenizerPrivate(const LogString& str, const LogString& delim1) : src(str), delim(delim1), pos(0){}
	LogString src;
	LogString delim;
	size_t pos;
};


StringTokenizer::StringTokenizer(const LogString& str, const LogString& delim1)
	: m_priv(std::make_unique<StringTokenizerPrivate>(str, delim1))
{
}

StringTokenizer::~StringTokenizer()
{
}

bool StringTokenizer::hasMoreTokens() const
{
	return (m_priv->pos != LogString::npos
			&& m_priv->src.find_first_not_of(m_priv->delim, m_priv->pos) != LogString::npos);
}

LogString StringTokenizer::nextToken()
{
	if (m_priv->pos != LogString::npos)
	{
		size_t nextPos = m_priv->src.find_first_not_of(m_priv->delim, m_priv->pos);

		if (nextPos != LogString::npos)
		{
			m_priv->pos = m_priv->src.find_first_of(m_priv->delim, nextPos);

			if (m_priv->pos == LogString::npos)
			{
				return m_priv->src.substr(nextPos);
			}

			return m_priv->src.substr(nextPos, m_priv->pos - nextPos);
		}
	}

	throw NoSuchElementException();
#if LOG4CXX_RETURN_AFTER_THROW
	return LogString();
#endif
}
