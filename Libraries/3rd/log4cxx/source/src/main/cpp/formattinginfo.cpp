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
#include <log4cxx/pattern/formattinginfo.h>
#include <limits.h>

using namespace log4cxx;
using namespace log4cxx::pattern;

struct FormattingInfo::FormattingInfoPrivate
{
	FormattingInfoPrivate(const bool leftAlign1, const int minLength1, const int maxLength1):
		minLength(minLength1),
		maxLength(maxLength1),
		leftAlign(leftAlign1) {}

	/**
	 * Minimum length.
	 */
	const int minLength;

	/**
	 * Maximum length.
	 */
	const int maxLength;

	/**
	 * Alignment.
	 */
	const bool leftAlign;
};

IMPLEMENT_LOG4CXX_OBJECT(FormattingInfo)

/**
 * Creates new instance.
 * @param leftAlign left align if true.
 * @param minLength minimum length.
 * @param maxLength maximum length.
 */
FormattingInfo::FormattingInfo(
	const bool leftAlign1, const int minLength1, const int maxLength1) :
	m_priv(std::make_unique<FormattingInfoPrivate>(leftAlign1, minLength1, maxLength1))
{
}

FormattingInfo::~FormattingInfo() {}

/**
 * Gets default instance.
 * @return default instance.
 */
FormattingInfoPtr FormattingInfo::getDefault()
{
	static FormattingInfoPtr def= std::make_shared<FormattingInfo>(false, 0, INT_MAX);
	return def;
}

/**
 * Adjust the content of the buffer based on the specified lengths and alignment.
 *
 * @param fieldStart start of field in buffer.
 * @param buffer buffer to be modified.
 */
void FormattingInfo::format(const int fieldStart, LogString& buffer) const
{
	int rawLength = int(buffer.length() - fieldStart);

	if (rawLength > m_priv->maxLength)
	{
		buffer.erase(buffer.begin() + fieldStart,
			buffer.begin() + fieldStart + (rawLength - m_priv->maxLength));
	}
	else if (rawLength < m_priv->minLength)
	{
		if (m_priv->leftAlign)
		{
			buffer.append(m_priv->minLength - rawLength, (logchar) 0x20 /* ' ' */);
		}
		else
		{
			buffer.insert(fieldStart, m_priv->minLength - rawLength, 0x20 /* ' ' */);
		}
	}
}

bool FormattingInfo::isLeftAligned() const
{
	return m_priv->leftAlign;
}

int FormattingInfo::getMinLength() const
{
	return m_priv->minLength;
}

int FormattingInfo::getMaxLength() const
{
	return m_priv->maxLength;
}
