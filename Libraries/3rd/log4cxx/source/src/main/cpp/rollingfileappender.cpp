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

#include <log4cxx/rolling/rollingfileappender.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/rolling/rolloverdescription.h>
#include <log4cxx/helpers/fileoutputstream.h>
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/rolling/fixedwindowrollingpolicy.h>
#include <log4cxx/rolling/timebasedrollingpolicy.h>
#include <log4cxx/rolling/sizebasedtriggeringpolicy.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/private/fileappender_priv.h>
#include <mutex>

using namespace log4cxx;
using namespace log4cxx::rolling;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;

struct RollingFileAppender::RollingFileAppenderPriv : public FileAppenderPriv
{
	RollingFileAppenderPriv() :
		FileAppenderPriv(),
		fileLength(0) {}

	/**
	 * Triggering policy.
	 */
	TriggeringPolicyPtr triggeringPolicy;

	/**
	 * Rolling policy.
	 */
	RollingPolicyPtr rollingPolicy;

	/**
	 * Length of current active log file.
	 */
	size_t fileLength;

	/**
	 *  save the loggingevent
	 */
	spi::LoggingEventPtr _event;
};

#define _priv static_cast<RollingFileAppenderPriv*>(m_priv.get())

IMPLEMENT_LOG4CXX_OBJECT(RollingFileAppender)


/**
 * Construct a new instance.
 */
RollingFileAppender::RollingFileAppender() :
	FileAppender (std::make_unique<RollingFileAppenderPriv>())
{
}

void RollingFileAppender::setOption(const LogString& option, const LogString& value)
{
	if (StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("MAXFILESIZE"), LOG4CXX_STR("maxfilesize"))
		|| StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("MAXIMUMFILESIZE"), LOG4CXX_STR("maximumfilesize")))
	{
		setMaxFileSize(value);
	}
	else if (StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("MAXBACKUPINDEX"), LOG4CXX_STR("maxbackupindex"))
		|| StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("MAXIMUMBACKUPINDEX"), LOG4CXX_STR("maximumbackupindex")))
	{
		setMaxBackupIndex(StringHelper::toInt(value));
	}
	else if (StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("FILEDATEPATTERN"), LOG4CXX_STR("filedatepattern")))
	{
		setDatePattern(value);
	}
	else
	{
		FileAppender::setOption(option, value);
	}
}

int RollingFileAppender::getMaxBackupIndex() const
{
	int result = 1;
	if (auto fwrp = log4cxx::cast<FixedWindowRollingPolicy>(_priv->rollingPolicy))
		result = fwrp->getMaxIndex();
	return result;
}

void RollingFileAppender::setMaxBackupIndex(int maxBackups)
{
	auto fwrp = log4cxx::cast<FixedWindowRollingPolicy>(_priv->rollingPolicy);
	if (!fwrp)
	{
		fwrp = std::make_shared<FixedWindowRollingPolicy>();
		fwrp->setFileNamePattern(getFile() + LOG4CXX_STR(".%i"));
		_priv->rollingPolicy = fwrp;
	}
	fwrp->setMaxIndex(maxBackups);
}

size_t RollingFileAppender::getMaximumFileSize() const
{
	size_t result = 10 * 1024 * 1024;
	if (auto sbtp = log4cxx::cast<SizeBasedTriggeringPolicy>(_priv->triggeringPolicy))
		result = sbtp->getMaxFileSize();
	return result;
}

void RollingFileAppender::setMaximumFileSize(size_t maxFileSize)
{
	auto sbtp = log4cxx::cast<SizeBasedTriggeringPolicy>(_priv->triggeringPolicy);
	if (!sbtp)
	{
		sbtp = std::make_shared<SizeBasedTriggeringPolicy>();
		_priv->triggeringPolicy = sbtp;
	}
	sbtp->setMaxFileSize(maxFileSize);
}

void RollingFileAppender::setMaxFileSize(const LogString& value)
{
	setMaximumFileSize(OptionConverter::toFileSize(value, long(getMaximumFileSize() + 1)));
}

LogString RollingFileAppender::makeFileNamePattern(const LogString& datePattern)
{
	LogString result(getFile());
	bool inLiteral = false;
	bool inPattern = false;

	for (size_t i = 0; i < datePattern.length(); i++)
	{
		if (datePattern[i] == 0x27 /* '\'' */)
		{
			inLiteral = !inLiteral;

			if (inLiteral && inPattern)
			{
				result.append(1, (logchar) 0x7D /* '}' */);
				inPattern = false;
			}
		}
		else
		{
			if (!inLiteral && !inPattern)
			{
				const logchar dbrace[] = { 0x25, 0x64, 0x7B, 0 }; // "%d{"
				result.append(dbrace);
				inPattern = true;
			}

			result.append(1, datePattern[i]);
		}
	}

	if (inPattern)
	{
		result.append(1, (logchar) 0x7D /* '}' */);
	}
	return result;
}

void RollingFileAppender::setDatePattern(const LogString& newPattern)
{
	auto tbrp = log4cxx::cast<TimeBasedRollingPolicy>(_priv->rollingPolicy);
	if (!tbrp)
	{
		tbrp = std::make_shared<TimeBasedRollingPolicy>();
		_priv->rollingPolicy = tbrp;
	}
	tbrp->setFileNamePattern(makeFileNamePattern(newPattern));
}

/**
 * Prepare instance of use.
 */
void RollingFileAppender::activateOptions(Pool& p)
{
	if (!_priv->rollingPolicy)
	{
		LogLog::warn(LOG4CXX_STR("No rolling policy configured for the appender named [")
			+ _priv->name + LOG4CXX_STR("]."));
		auto fwrp = std::make_shared<FixedWindowRollingPolicy>();
		fwrp->setFileNamePattern(getFile() + LOG4CXX_STR(".%i"));
		_priv->rollingPolicy = fwrp;
	}

	//
	//  if no explicit triggering policy and rolling policy is both.
	//
	if (!_priv->triggeringPolicy)
	{
		TriggeringPolicyPtr trig = log4cxx::cast<TriggeringPolicy>(_priv->rollingPolicy);

		if (trig != NULL)
		{
			_priv->triggeringPolicy = trig;
		}
	}

	if (!_priv->triggeringPolicy)
	{
		LogLog::warn(LOG4CXX_STR("No triggering policy configured for the appender named [")
			+ _priv->name + LOG4CXX_STR("]."));
		_priv->triggeringPolicy = std::make_shared<SizeBasedTriggeringPolicy>();
	}

	{
		std::lock_guard<std::recursive_mutex> lock(_priv->mutex);
		_priv->triggeringPolicy->activateOptions(p);
		_priv->rollingPolicy->activateOptions(p);

		try
		{
			RolloverDescriptionPtr rollover1 =
				_priv->rollingPolicy->initialize(getFile(), getAppend(), p);

			if (rollover1 != NULL)
			{
				ActionPtr syncAction(rollover1->getSynchronous());

				if (syncAction != NULL)
				{
					syncAction->execute(p);
				}

				_priv->fileName = rollover1->getActiveFileName();
				_priv->fileAppend = rollover1->getAppend();

				//
				//  async action not yet implemented
				//
				ActionPtr asyncAction(rollover1->getAsynchronous());

				if (asyncAction != NULL)
				{
					asyncAction->execute(p);
				}
			}

			File activeFile;
			activeFile.setPath(getFile());

			if (getAppend())
			{
				_priv->fileLength = activeFile.length(p);
			}
			else
			{
				_priv->fileLength = 0;
			}

			FileAppender::activateOptionsInternal(p);
		}
		catch (std::exception&)
		{
			LogLog::warn(
				LogString(LOG4CXX_STR("Exception will initializing RollingFileAppender named "))
				+ getName());
		}
	}
}

/**
   Implements the usual roll over behaviour.

   <p>If <code>MaxBackupIndex</code> is positive, then files
   {<code>File.1</code>, ..., <code>File.MaxBackupIndex -1</code>}
   are renamed to {<code>File.2</code>, ...,
   <code>File.MaxBackupIndex</code>}. Moreover, <code>File</code> is
   renamed <code>File.1</code> and closed. A new <code>File</code> is
   created to receive further log output.

   <p>If <code>MaxBackupIndex</code> is equal to zero, then the
   <code>File</code> is truncated with no backup files created.

 * @return true if rollover performed.
 */
bool RollingFileAppender::rollover(Pool& p)
{
	std::lock_guard<std::recursive_mutex> lock(_priv->mutex);
	return rolloverInternal(p);
}

bool RollingFileAppender::rolloverInternal(Pool& p)
{
	//
	//   can't roll without a policy
	//
	if (_priv->rollingPolicy != NULL)
	{

		{
				try
				{
					RolloverDescriptionPtr rollover1(_priv->rollingPolicy->rollover(this->getFile(), this->getAppend(), p));

					if (rollover1 != NULL)
					{
						if (rollover1->getActiveFileName() == getFile())
						{
							closeWriter();

							bool success = true;

							if (rollover1->getSynchronous() != NULL)
							{
								success = false;

								try
								{
									success = rollover1->getSynchronous()->execute(p);
								}
								catch (std::exception& ex)
								{
									LOG4CXX_DECODE_CHAR(lsMsg, ex.what());
									LogString errorMsg = LOG4CXX_STR("Exception on rollover: ");
									errorMsg.append(lsMsg);
									LogLog::error(errorMsg);
									_priv->errorHandler->error(lsMsg, ex, 0);
								}
							}

							if (success)
							{
								if (rollover1->getAppend())
								{
									_priv->fileLength = File().setPath(rollover1->getActiveFileName()).length(p);
								}
								else
								{
									_priv->fileLength = 0;
								}

								//
								//  async action not yet implemented
								//
								ActionPtr asyncAction(rollover1->getAsynchronous());

								if (asyncAction != NULL)
								{
									asyncAction->execute(p);
								}

								setFileInternal(
									rollover1->getActiveFileName(), rollover1->getAppend(),
									_priv->bufferedIO, _priv->bufferSize, p);
							}
							else
							{
								setFileInternal(
									rollover1->getActiveFileName(), true, _priv->bufferedIO, _priv->bufferSize, p);
							}
						}
						else
						{
							closeWriter();
							setFileInternal(rollover1->getActiveFileName());
							// Call activateOptions to create any intermediate directories(if required)
							FileAppender::activateOptionsInternal(p);
							OutputStreamPtr os(new FileOutputStream(
									rollover1->getActiveFileName(), rollover1->getAppend()));
							WriterPtr newWriter(createWriter(os));
							setWriterInternal(newWriter);

							bool success = true;

							if (rollover1->getSynchronous() != NULL)
							{
								success = false;

								try
								{
									success = rollover1->getSynchronous()->execute(p);
								}
								catch (std::exception& ex)
								{
									LOG4CXX_DECODE_CHAR(lsMsg, ex.what());
									LogString errorMsg = LOG4CXX_STR("Exception during rollover: ");
									errorMsg.append(lsMsg);
									LogLog::warn(errorMsg);
									_priv->errorHandler->error(lsMsg, ex, 0);
								}
							}

							if (success)
							{
								if (rollover1->getAppend())
								{
									_priv->fileLength = File().setPath(rollover1->getActiveFileName()).length(p);
								}
								else
								{
									_priv->fileLength = 0;
								}

								//
								//   async action not yet implemented
								//
								ActionPtr asyncAction(rollover1->getAsynchronous());

								if (asyncAction != NULL)
								{
									asyncAction->execute(p);
								}
							}

							writeHeader(p);
						}
						return true;
					}
				}
				catch (std::exception& ex)
				{
					LOG4CXX_DECODE_CHAR(lsMsg, ex.what());
					LogString errorMsg = LOG4CXX_STR("Exception during rollover: ");
					errorMsg.append(lsMsg);
					LogLog::warn(errorMsg);
					_priv->errorHandler->error(lsMsg, ex, 0);
				}
		}
	}

	return false;
}

/**
 * {@inheritDoc}
*/
void RollingFileAppender::subAppend(const LoggingEventPtr& event, Pool& p)
{
	// The rollover check must precede actual writing. This is the
	// only correct behavior for time driven triggers.
	if (
		_priv->triggeringPolicy->isTriggeringEvent(
			this, event, getFile(), getFileLength()))
	{
		//
		//   wrap rollover request in try block since
		//    rollover may fail in case read access to directory
		//    is not provided.  However appender should still be in good
		//     condition and the append should still happen.
		try
		{
			_priv->_event = event;
			rolloverInternal(p);
		}
		catch (std::exception& ex)
		{
			LOG4CXX_DECODE_CHAR(lsMsg, ex.what());
			LogString errorMsg = LOG4CXX_STR("Exception during rollover attempt: ");
			errorMsg.append(lsMsg);
			LogLog::warn(errorMsg);
			_priv->errorHandler->error(lsMsg);
		}
	}

	FileAppender::subAppend(event, p);
}

/**
 * TThe policy that implements the scheme for rolling over a log file.
 */
RollingPolicyPtr RollingFileAppender::getRollingPolicy() const
{
	return _priv->rollingPolicy;
}

/**
 * The policy that determine when to trigger a log file rollover.
 */
TriggeringPolicyPtr RollingFileAppender::getTriggeringPolicy() const
{
	return _priv->triggeringPolicy;
}

/**
 * Set the scheme for rolling over log files.
 */
void RollingFileAppender::setRollingPolicy(const RollingPolicyPtr& policy)
{
	_priv->rollingPolicy = policy;
}

/**
 * Set policy that determine when to trigger a log file rollover.
 */
void RollingFileAppender::setTriggeringPolicy(const TriggeringPolicyPtr& policy)
{
	_priv->triggeringPolicy = policy;
}

/**
 * Close appender.  Waits for any asynchronous file compression actions to be completed.
 */
void RollingFileAppender::close()
{
	FileAppender::close();
}

namespace log4cxx
{
namespace rolling
{
/**
 * Wrapper for OutputStream that will report all write
 * operations back to this class for file length calculations.
 */
class CountingOutputStream : public OutputStream
{
		/**
		 * Wrapped output stream.
		 */
	private:
		OutputStreamPtr os;

		/**
		 * Rolling file appender to inform of stream writes.
		 */
		RollingFileAppender* rfa;

	public:
		/**
		 * Constructor.
		 * @param os output stream to wrap.
		 * @param rfa rolling file appender to inform.
		 */
		CountingOutputStream(
			OutputStreamPtr& os1, RollingFileAppender* rfa1) :
			os(os1), rfa(rfa1)
		{
		}

		/**
		 * {@inheritDoc}
		 */
		void close(Pool& p)
		{
			os->close(p);
			rfa = 0;
		}

		/**
		 * {@inheritDoc}
		 */
		void flush(Pool& p)
		{
			os->flush(p);
		}

		/**
		 * {@inheritDoc}
		 */
		void write(ByteBuffer& buf, Pool& p)
		{
			os->write(buf, p);

			if (rfa != 0)
			{
				rfa->incrementFileLength(buf.limit());
			}
		}
};
}
}

/**
   Returns an OutputStreamWriter when passed an OutputStream.  The
   encoding used will depend on the value of the
   <code>encoding</code> property.  If the encoding value is
   specified incorrectly the writer will be opened using the default
   system encoding (an error message will be printed to the loglog.
 @param os output stream, may not be null.
 @return new writer.
 */
WriterPtr RollingFileAppender::createWriter(OutputStreamPtr& os)
{
	OutputStreamPtr cos = std::make_shared<CountingOutputStream>(os, this);
	return FileAppender::createWriter(cos);
}

/**
 * Get byte length of current active log file.
 * @return byte length of current active log file.
 */
size_t RollingFileAppender::getFileLength() const
{
	return _priv->fileLength;
}

/**
 * Increments estimated byte length of current active log file.
 * @param increment additional bytes written to log file.
 */
void RollingFileAppender::incrementFileLength(size_t increment)
{
	_priv->fileLength += increment;
}
