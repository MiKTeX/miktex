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

#include <log4cxx/writerappender.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/layout.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/private/appenderskeleton_priv.h>
#include <log4cxx/private/writerappender_priv.h>
#include <mutex>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;

#define _priv static_cast<WriterAppenderPriv*>(m_priv.get())

IMPLEMENT_LOG4CXX_OBJECT(WriterAppender)

WriterAppender::WriterAppender() :
	AppenderSkeleton (std::make_unique<WriterAppenderPriv>())
{
}

WriterAppender::WriterAppender(const LayoutPtr& layout1,
	log4cxx::helpers::WriterPtr& writer1)
	: AppenderSkeleton (std::make_unique<WriterAppenderPriv>(layout1, writer1))
{
	Pool p;
	activateOptions(p);
}

WriterAppender::WriterAppender(const LayoutPtr& layout1)
	: AppenderSkeleton (std::make_unique<WriterAppenderPriv>(layout1))
{
}

WriterAppender::WriterAppender(std::unique_ptr<WriterAppenderPriv> priv)
	: AppenderSkeleton (std::move(priv))
{

}

WriterAppender::~WriterAppender()
{
	finalize();
}

void WriterAppender::activateOptions(Pool& p)
{
	int errors = 0;

	if (_priv->layout == 0)
	{
		_priv->errorHandler->error(
			((LogString) LOG4CXX_STR("No layout set for the appender named ["))
			+ _priv->name + LOG4CXX_STR("]."));
		errors++;
	}

	if (_priv->writer == 0)
	{
		_priv->errorHandler->error(
			((LogString) LOG4CXX_STR("No writer set for the appender named ["))
			+ _priv->name + LOG4CXX_STR("]."));
		errors++;
	}

	if (errors == 0)
	{
		AppenderSkeleton::activateOptions(p);
	}
}



void WriterAppender::append(const spi::LoggingEventPtr& event, Pool& pool1)
{

	if (!checkEntryConditions())
	{
		return;
	}

	subAppend(event, pool1);
}

/**
   This method determines if there is a sense in attempting to append.

   <p>It checks whether there is a set output target and also if
   there is a set layout. If these checks fail, then the boolean
   value <code>false</code> is returned. */
bool WriterAppender::checkEntryConditions() const
{
	static bool warnedClosed = false;
	static bool warnedNoWriter = false;

	if (_priv->closed)
	{
		if (!warnedClosed)
		{
			LogLog::warn(LOG4CXX_STR("Not allowed to write to a closed appender."));
			warnedClosed = true;
		}

		return false;
	}

	if (_priv->writer == 0)
	{
		if (warnedNoWriter)
		{
			_priv->errorHandler->error(
				LogString(LOG4CXX_STR("No output stream or file set for the appender named [")) +
				_priv->name + LOG4CXX_STR("]."));
			warnedNoWriter = true;
		}

		return false;
	}

	if (_priv->layout == 0)
	{
		_priv->errorHandler->error(
			LogString(LOG4CXX_STR("No layout set for the appender named [")) +
			_priv->name + LOG4CXX_STR("]."));
		return false;
	}

	return true;
}




/**
   Close this appender instance. The underlying stream or writer is
   also closed.

   <p>Closed appenders cannot be reused.

   @see #setWriter
   */
void WriterAppender::close()
{
	std::lock_guard<std::recursive_mutex> lock(_priv->mutex);

	if (_priv->closed)
	{
		return;
	}

	_priv->closed = true;
	closeWriter();
}

/**
 * Close the underlying {@link java.io.Writer}.
 * */
void WriterAppender::closeWriter()
{
	if (_priv->writer != NULL)
	{
		try
		{
			// before closing we have to output out layout's footer
			//
			//   Using the object's pool since this is a one-shot operation
			//    and pool is likely to be reclaimed soon when appender is destructed.
			//
			writeFooter(_priv->pool);
			_priv->writer->close(_priv->pool);
			_priv->writer = 0;
		}
		catch (IOException& e)
		{
			LogLog::error(LogString(LOG4CXX_STR("Could not close writer for WriterAppender named ")) + _priv->name, e);
		}
	}

}

/**
   Returns an OutputStreamWriter when passed an OutputStream.  The
   encoding used will depend on the value of the
   <code>encoding</code> property.  If the encoding value is
   specified incorrectly the writer will be opened using the default
   system encoding (an error message will be printed to the loglog.  */
WriterPtr WriterAppender::createWriter(OutputStreamPtr& os)
{

	LogString enc(getEncoding());

	CharsetEncoderPtr encoder;

	if (enc.empty())
	{
		encoder = CharsetEncoder::getDefaultEncoder();
	}
	else
	{
		if (StringHelper::equalsIgnoreCase(enc,
				LOG4CXX_STR("utf-16"), LOG4CXX_STR("UTF-16")))
		{
			encoder = CharsetEncoder::getEncoder(LOG4CXX_STR("UTF-16BE"));
		}
		else
		{
			encoder = CharsetEncoder::getEncoder(enc);
		}

		if (encoder == NULL)
		{
			encoder = CharsetEncoder::getDefaultEncoder();
			LogLog::warn(LOG4CXX_STR("Error initializing output writer."));
			LogLog::warn(LOG4CXX_STR("Unsupported encoding?"));
		}
	}

	return WriterPtr(new OutputStreamWriter(os, encoder));
}

LogString WriterAppender::getEncoding() const
{
	return _priv->encoding;
}

void WriterAppender::setEncoding(const LogString& enc)
{
	_priv->encoding = enc;
}

void WriterAppender::subAppend(const spi::LoggingEventPtr& event, Pool& p)
{
	LogString msg;
	_priv->layout->format(msg, event, p);

	if (_priv->writer != NULL)
	{
		_priv->writer->write(msg, p);

		if (_priv->immediateFlush)
		{
			_priv->writer->flush(p);
		}
	}
}


void WriterAppender::writeFooter(Pool& p)
{
	if (_priv->layout != NULL)
	{
		LogString foot;
		_priv->layout->appendFooter(foot, p);
		_priv->writer->write(foot, p);
	}
}

void WriterAppender::writeHeader(Pool& p)
{
	if (_priv->layout != NULL)
	{
		LogString header;
		_priv->layout->appendHeader(header, p);
		_priv->writer->write(header, p);
	}
}


void WriterAppender::setWriter(const WriterPtr& newWriter)
{
	std::unique_lock<std::recursive_mutex> lock(_priv->mutex);
	setWriterInternal(newWriter);
}

void WriterAppender::setWriterInternal(const WriterPtr& newWriter)
{
	_priv->writer = newWriter;
}

bool WriterAppender::requiresLayout() const
{
	return true;
}

void WriterAppender::setOption(const LogString& option, const LogString& value)
{
	if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("ENCODING"), LOG4CXX_STR("encoding")))
	{
		setEncoding(value);
	}
	else
	{
		AppenderSkeleton::setOption(option, value);
	}
}


void WriterAppender::setImmediateFlush(bool value)
{
	_priv->immediateFlush = value;
}

bool WriterAppender::getImmediateFlush() const
{
	return _priv->immediateFlush;
}

const log4cxx::helpers::WriterPtr WriterAppender::getWriter() const{
	return _priv->writer;
}
