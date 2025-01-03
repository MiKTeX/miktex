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

#ifndef _LOG4CXX_HELPERS_STRING_TOKENIZER_H
#define _LOG4CXX_HELPERS_STRING_TOKENIZER_H

#include <log4cxx/logstring.h>
#include <log4cxx/helpers/exception.h>

namespace log4cxx
{
namespace helpers
{
class LOG4CXX_EXPORT StringTokenizer
{
	public:
		StringTokenizer(const LogString& str, const LogString& delim);
		~StringTokenizer();
		bool hasMoreTokens() const;
		LogString nextToken();

	private:
		//   prevent copy and assignment statements
		StringTokenizer(const StringTokenizer&);
		StringTokenizer& operator=(const StringTokenizer&);

		LOG4CXX_DECLARE_PRIVATE_MEMBER_PTR(StringTokenizerPrivate, m_priv)
}; // class StringTokenizer
}  // namespace helpers;
} // namespace log4cxx;

#endif //_LOG4CXX_HELPERS_STRING_TOKENIZER_H
