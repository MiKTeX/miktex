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
#include <log4cxx/helpers/outputstreamwriter.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/helpers/charsetencoder.h>
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/helpers/stringhelper.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(OutputStreamWriter)

struct OutputStreamWriter::OutputStreamWriterPrivate{
	OutputStreamWriterPrivate(OutputStreamPtr& out1) : out(out1), enc(CharsetEncoder::getDefaultEncoder()){}

	OutputStreamWriterPrivate(OutputStreamPtr& out1,
							  CharsetEncoderPtr& enc1)
		: out(out1), enc(enc1){}

	OutputStreamPtr out;
	CharsetEncoderPtr enc;
};

OutputStreamWriter::OutputStreamWriter(OutputStreamPtr& out1)
	: m_priv(std::make_unique<OutputStreamWriterPrivate>(out1))
{
	if (out1 == 0)
	{
		throw NullPointerException(LOG4CXX_STR("out parameter may not be null."));
	}
}

OutputStreamWriter::OutputStreamWriter(OutputStreamPtr& out1,
	CharsetEncoderPtr& enc1)
	: m_priv(std::make_unique<OutputStreamWriterPrivate>(out1, enc1))
{
	if (out1 == 0)
	{
		throw NullPointerException(LOG4CXX_STR("out parameter may not be null."));
	}

	if (enc1 == 0)
	{
		throw NullPointerException(LOG4CXX_STR("enc parameter may not be null."));
	}
}

OutputStreamWriter::~OutputStreamWriter()
{
}

void OutputStreamWriter::close(Pool& p)
{
	m_priv->out->close(p);
}

void OutputStreamWriter::flush(Pool& p)
{
	m_priv->out->flush(p);
}

void OutputStreamWriter::write(const LogString& str, Pool& p)
{
	if (str.length() > 0)
	{
#ifdef LOG4CXX_MULTI_PROCESS
		// Why does this need to happen for multiproces??  why??
		size_t bufSize = str.length() * 2;
		char* rawbuf = new char[bufSize];
		ByteBuffer buf(rawbuf, (size_t) bufSize);
#else
		enum { BUFSIZE = 1024 };
		char rawbuf[BUFSIZE];
		ByteBuffer buf(rawbuf, (size_t) BUFSIZE);
#endif
		m_priv->enc->reset();
		LogString::const_iterator iter = str.begin();

		while (iter != str.end())
		{
			CharsetEncoder::encode(m_priv->enc, str, iter, buf);
			buf.flip();
			m_priv->out->write(buf, p);
			buf.clear();
		}

		CharsetEncoder::encode(m_priv->enc, str, iter, buf);
		m_priv->enc->flush(buf);
		buf.flip();
		m_priv->out->write(buf, p);
#ifdef LOG4CXX_MULTI_PROCESS
		delete []rawbuf;
#endif
	}
}

OutputStreamPtr OutputStreamWriter::getOutputStreamPtr() const
{
	return m_priv->out;
}

