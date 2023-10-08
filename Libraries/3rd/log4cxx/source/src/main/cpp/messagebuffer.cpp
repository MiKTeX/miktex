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

#include <log4cxx/log4cxx.h>
/* Prevent error C2491: 'std::numpunct<_Elem>::id': definition of dllimport static data member not allowed */
#if defined(_MSC_VER) && (LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API)
#define __FORCE_INSTANCE
#endif
#include <log4cxx/helpers/messagebuffer.h>
#include <log4cxx/helpers/transcoder.h>
#if !defined(LOG4CXX)
	#define LOG4CXX 1
#endif
#include <log4cxx/private/log4cxx_private.h>

using namespace log4cxx::helpers;

template <typename T>
void ResetStream(std::basic_ostringstream<T>& stream)
{
	stream.seekp(0);
	stream.str(std::basic_string<T>());
	stream.clear();
}

struct CharMessageBuffer::CharMessageBufferPrivate{
	CharMessageBufferPrivate() :
		stream(nullptr){}

	/**
	   * Encapsulated std::string.
	   */
	std::basic_string<char> buf;
	/**
	 *  Encapsulated stream, created on demand.
	 */
	std::basic_ostringstream<char>* stream;
};

CharMessageBuffer::CharMessageBuffer() : m_priv(std::make_unique<CharMessageBufferPrivate>())
{
}

CharMessageBuffer::~CharMessageBuffer()
{
#if !LOG4CXX_HAS_THREAD_LOCAL
	delete m_priv->stream;
#endif
}

CharMessageBuffer& CharMessageBuffer::operator<<(const std::basic_string<char>& msg)
{
	if (m_priv->stream == 0)
	{
		m_priv->buf.append(msg);
	}
	else
	{
		*m_priv->stream << msg;
	}

	return *this;
}

CharMessageBuffer& CharMessageBuffer::operator<<(const char* msg)
{
	const char* actualMsg = msg;

	if (actualMsg == 0)
	{
		actualMsg = "null";
	}

	if (m_priv->stream == 0)
	{
		m_priv->buf.append(actualMsg);
	}
	else
	{
		*m_priv->stream << actualMsg;
	}

	return *this;
}
CharMessageBuffer& CharMessageBuffer::operator<<(char* msg)
{
	return operator<<((const char*) msg);
}

CharMessageBuffer& CharMessageBuffer::operator<<(const char msg)
{
	if (m_priv->stream == 0)
	{
		m_priv->buf.append(1, msg);
	}
	else
	{
		m_priv->buf.assign(1, msg);
		*m_priv->stream << m_priv->buf;
	}

	return *this;
}

CharMessageBuffer::operator std::basic_ostream<char>& ()
{
	if (!m_priv->stream)
	{
#if LOG4CXX_HAS_THREAD_LOCAL
		thread_local static std::basic_ostringstream<char> sStream;
		m_priv->stream = &sStream;
#else
		m_priv->stream = new std::basic_ostringstream<char>();
#endif
		if (!m_priv->buf.empty())
		{
			*m_priv->stream << m_priv->buf;
		}
	}

	return *m_priv->stream;
}

const std::basic_string<char>& CharMessageBuffer::str(std::basic_ostream<char>&)
{
	m_priv->buf = m_priv->stream->str();

	ResetStream(*m_priv->stream);

	return m_priv->buf;
}

const std::basic_string<char>& CharMessageBuffer::str(CharMessageBuffer&)
{
	return m_priv->buf;
}

bool CharMessageBuffer::hasStream() const
{
	return (m_priv->stream != 0);
}

std::ostream& CharMessageBuffer::operator<<(ios_base_manip manip)
{
	std::ostream& s = *this;
	(*manip)(s);
	return s;
}

std::ostream& CharMessageBuffer::operator<<(bool val)
{
	return ((std::ostream&) * this).operator << (val);
}
std::ostream& CharMessageBuffer::operator<<(short val)
{
	return ((std::ostream&) * this).operator << (val);
}
std::ostream& CharMessageBuffer::operator<<(int val)
{
	return ((std::ostream&) * this).operator << (val);
}
std::ostream& CharMessageBuffer::operator<<(unsigned int val)
{
	return ((std::ostream&) * this).operator << (val);
}
std::ostream& CharMessageBuffer::operator<<(long val)
{
	return ((std::ostream&) * this).operator << (val);
}
std::ostream& CharMessageBuffer::operator<<(unsigned long val)
{
	return ((std::ostream&) * this).operator << (val);
}
std::ostream& CharMessageBuffer::operator<<(float val)
{
	return ((std::ostream&) * this).operator << (val);
}
std::ostream& CharMessageBuffer::operator<<(double val)
{
	return ((std::ostream&) * this).operator << (val);
}
std::ostream& CharMessageBuffer::operator<<(long double val)
{
	return ((std::ostream&) * this).operator << (val);
}
std::ostream& CharMessageBuffer::operator<<(void* val)
{
	return ((std::ostream&) * this).operator << (val);
}

#if LOG4CXX_WCHAR_T_API
struct WideMessageBuffer::WideMessageBufferPrivate{
	WideMessageBufferPrivate() :
		stream(nullptr){}

	/**
	   * Encapsulated std::string.
	   */
	std::basic_string<wchar_t> buf;
	/**
	 *  Encapsulated stream, created on demand.
	 */
	std::basic_ostringstream<wchar_t>* stream;
};


WideMessageBuffer::WideMessageBuffer() :
	m_priv(std::make_unique<WideMessageBufferPrivate>())
{
}

WideMessageBuffer::~WideMessageBuffer()
{
#if !LOG4CXX_HAS_THREAD_LOCAL
	delete m_priv->stream;
#endif
}

WideMessageBuffer& WideMessageBuffer::operator<<(const std::basic_string<wchar_t>& msg)
{
	if (m_priv->stream == 0)
	{
		m_priv->buf.append(msg);
	}
	else
	{
		*m_priv->stream << msg;
	}

	return *this;
}

WideMessageBuffer& WideMessageBuffer::operator<<(const wchar_t* msg)
{
	const wchar_t* actualMsg = msg;

	if (actualMsg == 0)
	{
		actualMsg = L"null";
	}

	if (m_priv->stream == 0)
	{
		m_priv->buf.append(actualMsg);
	}
	else
	{
		*m_priv->stream << actualMsg;
	}

	return *this;
}

WideMessageBuffer& WideMessageBuffer::operator<<(wchar_t* msg)
{
	return operator<<((const wchar_t*) msg);
}

WideMessageBuffer& WideMessageBuffer::operator<<(const wchar_t msg)
{
	if (m_priv->stream == 0)
	{
		m_priv->buf.append(1, msg);
	}
	else
	{
		m_priv->buf.assign(1, msg);
		*m_priv->stream << m_priv->buf;
	}

	return *this;
}

WideMessageBuffer::operator std::basic_ostream<wchar_t>& ()
{
	if (!m_priv->stream)
	{
#if LOG4CXX_HAS_THREAD_LOCAL
		thread_local static std::basic_ostringstream<wchar_t> sStream;
		m_priv->stream = &sStream;
#else
		m_priv->stream = new std::basic_ostringstream<wchar_t>();
#endif
		if (!m_priv->buf.empty())
		{
			*m_priv->stream << m_priv->buf;
		}
	}

	return *m_priv->stream;
}

const std::basic_string<wchar_t>& WideMessageBuffer::str(std::basic_ostream<wchar_t>&)
{
	m_priv->buf = m_priv->stream->str();

	ResetStream(*m_priv->stream);

	return m_priv->buf;
}

const std::basic_string<wchar_t>& WideMessageBuffer::str(WideMessageBuffer&)
{
	return m_priv->buf;
}

bool WideMessageBuffer::hasStream() const
{
	return (m_priv->stream != 0);
}

std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(ios_base_manip manip)
{
	std::basic_ostream<wchar_t>& s = *this;
	(*manip)(s);
	return s;
}

std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(bool val)
{
	return ((std::basic_ostream<wchar_t>&) * this).operator << (val);
}
std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(short val)
{
	return ((std::basic_ostream<wchar_t>&) * this).operator << (val);
}
std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(int val)
{
	return ((std::basic_ostream<wchar_t>&) * this).operator << (val);
}
std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(unsigned int val)
{
	return ((std::basic_ostream<wchar_t>&) * this).operator << (val);
}
std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(long val)
{
	return ((std::basic_ostream<wchar_t>&) * this).operator << (val);
}
std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(unsigned long val)
{
	return ((std::basic_ostream<wchar_t>&) * this).operator << (val);
}
std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(float val)
{
	return ((std::basic_ostream<wchar_t>&) * this).operator << (val);
}
std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(double val)
{
	return ((std::basic_ostream<wchar_t>&) * this).operator << (val);
}
std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(long double val)
{
	return ((std::basic_ostream<wchar_t>&) * this).operator << (val);
}
std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(void* val)
{
	return ((std::basic_ostream<wchar_t>&) * this).operator << (val);
}

struct MessageBuffer::MessageBufferPrivate{
	MessageBufferPrivate(){}

	/**
	 *  Character message buffer.
	 */
	CharMessageBuffer cbuf;

	/**
	 * Encapsulated wide message buffer, created on demand.
	 */
	std::unique_ptr<WideMessageBuffer> wbuf;
#if LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API
	/**
	 * Encapsulated wide message buffer, created on demand.
	 */
	std::unique_ptr<UniCharMessageBuffer> ubuf;
#endif
};

MessageBuffer::MessageBuffer()  :
	m_priv(std::make_unique<MessageBufferPrivate>())
{
}

MessageBuffer::~MessageBuffer()
{
}

bool MessageBuffer::hasStream() const
{
	bool retval = m_priv->cbuf.hasStream() || (m_priv->wbuf != 0 && m_priv->wbuf->hasStream());
#if LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API
	retval = retval || (m_priv->ubuf != 0 && m_priv->ubuf->hasStream());
#endif
	return retval;
}

std::ostream& MessageBuffer::operator<<(ios_base_manip manip)
{
	std::ostream& s = *this;
	(*manip)(s);
	return s;
}

MessageBuffer::operator std::ostream& ()
{
	return (std::ostream&) m_priv->cbuf;
}

CharMessageBuffer& MessageBuffer::operator<<(const std::string& msg)
{
	return m_priv->cbuf.operator << (msg);
}

CharMessageBuffer& MessageBuffer::operator<<(const char* msg)
{
	return m_priv->cbuf.operator << (msg);
}
CharMessageBuffer& MessageBuffer::operator<<(char* msg)
{
	return m_priv->cbuf.operator << ((const char*) msg);
}

CharMessageBuffer& MessageBuffer::operator<<(const char msg)
{
	return m_priv->cbuf.operator << (msg);
}

const std::string& MessageBuffer::str(CharMessageBuffer& buf)
{
	return m_priv->cbuf.str(buf);
}

const std::string& MessageBuffer::str(std::ostream& os)
{
	return m_priv->cbuf.str(os);
}

WideMessageBuffer& MessageBuffer::operator<<(const std::wstring& msg)
{
	m_priv->wbuf = std::make_unique<WideMessageBuffer>();
	return (*m_priv->wbuf) << msg;
}

WideMessageBuffer& MessageBuffer::operator<<(const wchar_t* msg)
{
	m_priv->wbuf = std::make_unique<WideMessageBuffer>();
	return (*m_priv->wbuf) << msg;
}
WideMessageBuffer& MessageBuffer::operator<<(wchar_t* msg)
{
	m_priv->wbuf = std::make_unique<WideMessageBuffer>();
	return (*m_priv->wbuf) << (const wchar_t*) msg;
}

WideMessageBuffer& MessageBuffer::operator<<(const wchar_t msg)
{
	m_priv->wbuf = std::make_unique<WideMessageBuffer>();
	return (*m_priv->wbuf) << msg;
}

const std::wstring& MessageBuffer::str(WideMessageBuffer& buf)
{
	return m_priv->wbuf->str(buf);
}

const std::wstring& MessageBuffer::str(std::basic_ostream<wchar_t>& os)
{
	return m_priv->wbuf->str(os);
}

std::ostream& MessageBuffer::operator<<(bool val)
{
	return m_priv->cbuf.operator << (val);
}
std::ostream& MessageBuffer::operator<<(short val)
{
	return m_priv->cbuf.operator << (val);
}
std::ostream& MessageBuffer::operator<<(int val)
{
	return m_priv->cbuf.operator << (val);
}
std::ostream& MessageBuffer::operator<<(unsigned int val)
{
	return m_priv->cbuf.operator << (val);
}
std::ostream& MessageBuffer::operator<<(long val)
{
	return m_priv->cbuf.operator << (val);
}
std::ostream& MessageBuffer::operator<<(unsigned long val)
{
	return m_priv->cbuf.operator << (val);
}
std::ostream& MessageBuffer::operator<<(float val)
{
	return m_priv->cbuf.operator << (val);
}
std::ostream& MessageBuffer::operator<<(double val)
{
	return m_priv->cbuf.operator << (val);
}
std::ostream& MessageBuffer::operator<<(long double val)
{
	return m_priv->cbuf.operator << (val);
}
std::ostream& MessageBuffer::operator<<(void* val)
{
	return m_priv->cbuf.operator << (val);
}

#if LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API
UniCharMessageBuffer& MessageBuffer::operator<<(const std::basic_string<log4cxx::UniChar>& msg)
{
	m_priv->ubuf = std::make_unique<UniCharMessageBuffer>();
	return (*m_priv->ubuf) << msg;
}

UniCharMessageBuffer& MessageBuffer::operator<<(const log4cxx::UniChar* msg)
{
	m_priv->ubuf = std::make_unique<UniCharMessageBuffer>();
	return (*m_priv->ubuf) << msg;
}
UniCharMessageBuffer& MessageBuffer::operator<<(log4cxx::UniChar* msg)
{
	m_priv->ubuf = std::make_unique<UniCharMessageBuffer>();
	return (*m_priv->ubuf) << (const log4cxx::UniChar*) msg;
}

UniCharMessageBuffer& MessageBuffer::operator<<(const log4cxx::UniChar msg)
{
	m_priv->ubuf = std::make_unique<UniCharMessageBuffer>();
	return (*m_priv->ubuf) << msg;
}

const std::basic_string<log4cxx::UniChar>& MessageBuffer::str(UniCharMessageBuffer& buf)
{
	return m_priv->ubuf->str(buf);
}

const std::basic_string<log4cxx::UniChar>& MessageBuffer::str(std::basic_ostream<log4cxx::UniChar>& os)
{
	return m_priv->ubuf->str(os);
}
#endif //LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API
#endif // LOG4CXX_WCHAR_T_API

#if LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API
struct UniCharMessageBuffer::UniCharMessageBufferPrivate {
	UniCharMessageBufferPrivate() :
		stream(nullptr){}

	/**
	   * Encapsulated std::string.
	   */
	std::basic_string<UniChar> buf;
	/**
	 *  Encapsulated stream, created on demand.
	 */
	std::basic_ostringstream<UniChar>* stream;
};

UniCharMessageBuffer::UniCharMessageBuffer() :
	m_priv(std::make_unique<UniCharMessageBufferPrivate>())
{
}

UniCharMessageBuffer::~UniCharMessageBuffer()
{
#if !LOG4CXX_HAS_THREAD_LOCAL
	delete m_priv->stream;
#endif
}


UniCharMessageBuffer& UniCharMessageBuffer::operator<<(const std::basic_string<log4cxx::UniChar>& msg)
{
	if (!m_priv->stream)
	{
		m_priv->buf.append(msg);
	}
	else
	{
		*m_priv->stream << m_priv->buf;
	}

	return *this;
}

UniCharMessageBuffer& UniCharMessageBuffer::operator<<(const log4cxx::UniChar* msg)
{
	const log4cxx::UniChar* actualMsg = msg;
	static log4cxx::UniChar nullLiteral[] = { 0x6E, 0x75, 0x6C, 0x6C, 0};

	if (actualMsg == 0)
	{
		actualMsg = nullLiteral;
	}

	if (!m_priv->stream)
	{
		m_priv->buf.append(actualMsg);
	}
	else
	{
		*m_priv->stream << actualMsg;
	}

	return *this;
}

UniCharMessageBuffer& UniCharMessageBuffer::operator<<(log4cxx::UniChar* msg)
{
	return operator<<((const log4cxx::UniChar*) msg);
}

UniCharMessageBuffer& UniCharMessageBuffer::operator<<(const log4cxx::UniChar msg)
{
	if (!m_priv->stream)
	{
		m_priv->buf.append(1, msg);
	}
	else
	{
		*m_priv->stream << msg;
	}

	return *this;
}

UniCharMessageBuffer::operator UniCharMessageBuffer::uostream& ()
{
	if (!m_priv->stream)
	{
#if LOG4CXX_HAS_THREAD_LOCAL
		thread_local static std::basic_ostringstream<log4cxx::UniChar> sStream;
		m_priv->stream = &sStream;
#else
		m_priv->stream = new std::basic_ostringstream<log4cxx::UniChar>();
#endif
		if (!m_priv->buf.empty())
		{
			*m_priv->stream << m_priv->buf;
		}
	}

	return *m_priv->stream;
}

const std::basic_string<log4cxx::UniChar>& UniCharMessageBuffer::str(UniCharMessageBuffer::uostream&)
{
	m_priv->buf = m_priv->stream->str();
	ResetStream(*m_priv->stream);
	return m_priv->buf;
}

const std::basic_string<log4cxx::UniChar>& UniCharMessageBuffer::str(UniCharMessageBuffer&)
{
	return m_priv->buf;
}

bool UniCharMessageBuffer::hasStream() const
{
	return (m_priv->stream != 0);
}

UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(ios_base_manip manip)
{
	UniCharMessageBuffer::uostream& s = *this;
	(*manip)(s);
	return s;
}

UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(bool val)
{
	return ((UniCharMessageBuffer::uostream&) * this).operator << (val);
}
UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(short val)
{
	return ((UniCharMessageBuffer::uostream&) * this).operator << (val);
}
UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(int val)
{
	return ((UniCharMessageBuffer::uostream&) * this).operator << (val);
}
UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(unsigned int val)
{
	return ((UniCharMessageBuffer::uostream&) * this).operator << (val);
}
UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(long val)
{
	return ((UniCharMessageBuffer::uostream&) * this).operator << (val);
}
UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(unsigned long val)
{
	return ((UniCharMessageBuffer::uostream&) * this).operator << (val);
}
UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(float val)
{
	return ((UniCharMessageBuffer::uostream&) * this).operator << (val);
}
UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(double val)
{
	return ((UniCharMessageBuffer::uostream&) * this).operator << (val);
}
UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(long double val)
{
	return ((UniCharMessageBuffer::uostream&) * this).operator << (val);
}
UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(void* val)
{
	return ((UniCharMessageBuffer::uostream&) * this).operator << (val);
}

#endif // LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API


#if LOG4CXX_CFSTRING_API
#include <CoreFoundation/CFString.h>
#include <vector>

UniCharMessageBuffer& UniCharMessageBuffer::operator<<(const CFStringRef& msg)
{
	const log4cxx::UniChar* chars = CFStringGetCharactersPtr(msg);

	if (chars != 0)
	{
		return operator<<(chars);
	}
	else
	{
		size_t length = CFStringGetLength(msg);
		std::vector<log4cxx::UniChar> tmp(length);
		CFStringGetCharacters(msg, CFRangeMake(0, length), &tmp[0]);

		if (m_priv->stream)
		{
			std::basic_string<UniChar> s(&tmp[0], tmp.size());
			*m_priv->stream << s;
		}
		else
		{
			m_priv->buf.append(&tmp[0], tmp.size());
		}
	}

	return *this;
}


UniCharMessageBuffer& MessageBuffer::operator<<(const CFStringRef& msg)
{
	m_priv->ubuf = std::make_unique<UniCharMessageBuffer>();
	return (*m_priv->ubuf) << msg;
}
#endif // LOG4CXX_CFSTRING_API

