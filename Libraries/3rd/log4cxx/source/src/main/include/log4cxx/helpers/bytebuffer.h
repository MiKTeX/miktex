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

#ifndef _LOG4CXX_HELPERS_BYTEBUFFER_H
#define _LOG4CXX_HELPERS_BYTEBUFFER_H

#include <log4cxx/log4cxx.h>
#include <stdio.h>

namespace log4cxx
{

namespace helpers
{

/**
* A byte buffer.
*/
class LOG4CXX_EXPORT ByteBuffer
{
	private:
		LOG4CXX_DECLARE_PRIVATE_MEMBER_PTR(ByteBufferPriv, m_priv)

	public:
		ByteBuffer(char* data, size_t capacity);
		~ByteBuffer();

		void clear();
		void flip();

		char* data();
		const char* data() const;
		char* current();
		const char* current() const;
		size_t limit() const;
		void limit(size_t newLimit);
		size_t position() const;
		size_t remaining() const;
		void position(size_t newPosition);

		bool put(char byte);


	private:
		ByteBuffer(const ByteBuffer&);
		ByteBuffer& operator=(const ByteBuffer&);
};
} // namespace helpers

}  //namespace log4cxx

#endif //_LOG4CXX_HELPERS_BYTEBUFFER_H
