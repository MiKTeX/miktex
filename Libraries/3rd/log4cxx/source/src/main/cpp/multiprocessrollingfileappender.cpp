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

#include <apr_portable.h>
#include <apr_file_io.h>
#include <apr_atomic.h>
#include <apr_mmap.h>
#ifndef MAX_FILE_LEN
	#define MAX_FILE_LEN 2048
#endif
#include <log4cxx/pattern/filedatepatternconverter.h>
#include <log4cxx/helpers/date.h>

#include <log4cxx/rolling/multiprocessrollingfileappender.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/rolling/rolloverdescription.h>
#include <log4cxx/helpers/fileoutputstream.h>
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/rolling/fixedwindowrollingpolicy.h>
#include <log4cxx/rolling/manualtriggeringpolicy.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/private/fileappender_priv.h>
#include <log4cxx/rolling/timebasedrollingpolicy.h>
#include <log4cxx/private/boost-std-configuration.h>
#include <mutex>

using namespace log4cxx;
using namespace log4cxx::rolling;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;

struct MultiprocessRollingFileAppender::MultiprocessRollingFileAppenderPriv : public FileAppenderPriv
{
	MultiprocessRollingFileAppenderPriv() :
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

#define _priv static_cast<MultiprocessRollingFileAppenderPriv*>(m_priv.get())

IMPLEMENT_LOG4CXX_OBJECT(MultiprocessRollingFileAppender)


/**
 * Construct a new instance.
 */
MultiprocessRollingFileAppender::MultiprocessRollingFileAppender() :
	FileAppender (std::make_unique<MultiprocessRollingFileAppenderPriv>())
{
}

/**
 * Prepare instance of use.
 */
void MultiprocessRollingFileAppender::activateOptions(Pool& p)
{
	if (_priv->rollingPolicy == NULL)
	{
		auto fwrp = std::make_shared<FixedWindowRollingPolicy>();
		fwrp->setFileNamePattern(getFile() + LOG4CXX_STR(".%i"));
		_priv->rollingPolicy = fwrp;
	}

	//
	//  if no explicit triggering policy and rolling policy is both.
	//
	if (_priv->triggeringPolicy == NULL)
	{
		TriggeringPolicyPtr trig = log4cxx::cast<TriggeringPolicy>(_priv->rollingPolicy);

		if (trig != NULL)
		{
			_priv->triggeringPolicy = trig;
		}
	}

	if (_priv->triggeringPolicy == NULL)
	{
		_priv->triggeringPolicy = std::make_shared<ManualTriggeringPolicy>();
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

void MultiprocessRollingFileAppender::releaseFileLock(apr_file_t* lock_file)
{
	if (lock_file)
	{
		apr_status_t stat = apr_file_unlock(lock_file);

		if (stat != APR_SUCCESS)
		{
			LogLog::warn(LOG4CXX_STR("flock: unlock failed"));
		}

		apr_file_close(lock_file);
		lock_file = NULL;
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
bool MultiprocessRollingFileAppender::rollover(Pool& p)
{
	std::lock_guard<std::recursive_mutex> lock(_priv->mutex);
	return rolloverInternal(p);
}

bool MultiprocessRollingFileAppender::rolloverInternal(Pool& p)
{
	//
	//   can't roll without a policy
	//
	if (_priv->rollingPolicy != NULL)
	{

		{
			LogString fileName(getFile());
			RollingPolicyBasePtr basePolicy = log4cxx::cast<RollingPolicyBase>(_priv->rollingPolicy);
			apr_time_t n = apr_time_now();
			ObjectPtr obj = std::make_shared<Date>(n);
			LogString fileNamePattern;

			if (basePolicy)
			{
				if (basePolicy->getPatternConverterList().size())
				{
					(*(basePolicy->getPatternConverterList().begin()))->format(obj, fileNamePattern, p);
					fileName = std::string(fileNamePattern);
				}
			}

			bool bAlreadyRolled = true;
			char szDirName[MAX_FILE_LEN] = {'\0'};
			char szBaseName[MAX_FILE_LEN] = {'\0'};
			char szUid[MAX_FILE_LEN] = {'\0'};
			memcpy(szDirName, fileName.c_str(), fileName.size() > MAX_FILE_LEN ? MAX_FILE_LEN : fileName.size());
			memcpy(szBaseName, fileName.c_str(), fileName.size() > MAX_FILE_LEN ? MAX_FILE_LEN : fileName.size());
			apr_uid_t uid;
			apr_gid_t groupid;
			apr_status_t stat = apr_uid_current(&uid, &groupid, p.getAPRPool());

			if (stat == APR_SUCCESS)
			{
				snprintf(szUid, MAX_FILE_LEN, "%u", uid);
			}

			log4cxx::filesystem::path path = szDirName;
			const LogString lockname = path.parent_path() / (path.filename().string() + szUid + ".lock");
			apr_file_t* lock_file;
			stat = apr_file_open(&lock_file, lockname.c_str(), APR_CREATE | APR_READ | APR_WRITE, APR_OS_DEFAULT, p.getAPRPool());

			if (stat != APR_SUCCESS)
			{
				LogString err = LOG4CXX_STR("lockfile return error: open lockfile failed. ");
				err += (strerror(errno));
				LogLog::warn(err);
				bAlreadyRolled = false;
				lock_file = NULL;
			}
			else
			{
				stat = apr_file_lock(lock_file, APR_FLOCK_EXCLUSIVE);

				if (stat != APR_SUCCESS)
				{
					LogString err = LOG4CXX_STR("apr_file_lock: lock failed. ");
					err += (strerror(errno));
					LogLog::warn(err);
					bAlreadyRolled = false;
				}
				else
				{
					if (_priv->_event)
					{
						_priv->triggeringPolicy->isTriggeringEvent(this, _priv->_event, getFile(), getFileLength());
					}
				}
			}

			if (bAlreadyRolled)
			{
				apr_finfo_t finfo1, finfo2;
				apr_status_t st1, st2;
				const WriterPtr writer = getWriter();
				const FileOutputStreamPtr fos = log4cxx::cast<FileOutputStream>( writer );
				if( !fos ){
					LogLog::error( LOG4CXX_STR("Can't cast writer to FileOutputStream") );
					return false;
				}
				apr_file_t* _fd = fos->getFilePtr();
				st1 = apr_file_info_get(&finfo1, APR_FINFO_IDENT, _fd);

				if (st1 != APR_SUCCESS)
				{
					LogLog::warn(LOG4CXX_STR("apr_file_info_get failed"));
				}

				LogString fname = getFile();
				st2 = apr_stat(&finfo2, fname.c_str(), APR_FINFO_IDENT, p.getAPRPool());

				if (st2 != APR_SUCCESS)
				{
					LogLog::warn(LOG4CXX_STR("apr_stat failed."));
				}

				bAlreadyRolled = ((st1 == APR_SUCCESS) && (st2 == APR_SUCCESS)
						&& ((finfo1.device != finfo2.device) || (finfo1.inode != finfo2.inode)));
			}

			if (!bAlreadyRolled)
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
									LogLog::warn(LOG4CXX_STR("Exception on rollover"));
									LogString exmsg;
									log4cxx::helpers::Transcoder::decode(ex.what(), exmsg);
									_priv->errorHandler->error(exmsg, ex, 0);
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
									LogLog::warn(LOG4CXX_STR("Exception during rollover"));
									LogString exmsg;
									log4cxx::helpers::Transcoder::decode(ex.what(), exmsg);
									_priv->errorHandler->error(exmsg, ex, 0);
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

						releaseFileLock(lock_file);
						return true;
					}
				}
				catch (std::exception& ex)
				{
					LogLog::warn(LOG4CXX_STR("Exception during rollover"));
					LogString exmsg;
					log4cxx::helpers::Transcoder::decode(ex.what(), exmsg);
					_priv->errorHandler->error(exmsg, ex, 0);
				}

			}
			else
			{
				reopenLatestFile(p);
			}

			releaseFileLock(lock_file);
		}
	}

	return false;
}

/**
 * re-open current file when its own handler has been renamed
 */
void MultiprocessRollingFileAppender::reopenLatestFile(Pool& p)
{
	closeWriter();
	OutputStreamPtr os = std::make_shared<FileOutputStream>(getFile(), true);
	WriterPtr newWriter(createWriter(os));
	setFile(getFile());
	setWriter(newWriter);
	_priv->fileLength = File().setPath(getFile()).length(p);
	writeHeader(p);
}


/**
 * {@inheritDoc}
*/
void MultiprocessRollingFileAppender::subAppend(const LoggingEventPtr& event, Pool& p)
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
			LogLog::warn(LOG4CXX_STR("Exception during rollover attempt."));
			LogString exmsg;
			log4cxx::helpers::Transcoder::decode(ex.what(), exmsg);
			_priv->errorHandler->error(exmsg);
		}
	}

	//do re-check before every write
	//
	apr_finfo_t finfo1, finfo2;
	apr_status_t st1, st2;
	const WriterPtr writer = getWriter();
	const FileOutputStreamPtr fos = log4cxx::cast<FileOutputStream>( writer );
	if( !fos ){
		LogLog::error( LOG4CXX_STR("Can't cast writer to FileOutputStream") );
		return;
	}
	apr_file_t* _fd = fos->getFilePtr();
	st1 = apr_file_info_get(&finfo1, APR_FINFO_IDENT, _fd);

	if (st1 != APR_SUCCESS)
	{
		LogLog::warn(LOG4CXX_STR("apr_file_info_get failed"));
	}

	st2 = apr_stat(&finfo2, std::string(getFile()).c_str(), APR_FINFO_IDENT, p.getAPRPool());

	if (st2 != APR_SUCCESS)
	{
		LogString err = "apr_stat failed. file:" + getFile();
		LogLog::warn(err);
	}

	bool bAlreadyRolled = ((st1 == APR_SUCCESS) && (st2 == APR_SUCCESS)
			&& ((finfo1.device != finfo2.device) || (finfo1.inode != finfo2.inode)));

	if (bAlreadyRolled)
	{
		reopenLatestFile(p);
	}

	FileAppender::subAppend(event, p);
}

/**
 * Get rolling policy.
 * @return rolling policy.
 */
RollingPolicyPtr MultiprocessRollingFileAppender::getRollingPolicy() const
{
	return _priv->rollingPolicy;
}

/**
 * Get triggering policy.
 * @return triggering policy.
 */
TriggeringPolicyPtr MultiprocessRollingFileAppender::getTriggeringPolicy() const
{
	return _priv->triggeringPolicy;
}

/**
 * Sets the rolling policy.
 * @param policy rolling policy.
 */
void MultiprocessRollingFileAppender::setRollingPolicy(const RollingPolicyPtr& policy)
{
	_priv->rollingPolicy = policy;

	TimeBasedRollingPolicyPtr timeBased = log4cxx::cast<TimeBasedRollingPolicy>(policy);
	if( timeBased ){
		timeBased->setMultiprocess(true);
	}
}

/**
 * Set triggering policy.
 * @param policy triggering policy.
 */
void MultiprocessRollingFileAppender::setTriggeringPolicy(const TriggeringPolicyPtr& policy)
{
	_priv->triggeringPolicy = policy;
}

/**
 * Close appender.  Waits for any asynchronous file compression actions to be completed.
 */
void MultiprocessRollingFileAppender::close()
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
		MultiprocessRollingFileAppender* rfa;

	public:
		/**
		 * Constructor.
		 * @param os output stream to wrap.
		 * @param rfa rolling file appender to inform.
		 */
		CountingOutputStream(
			OutputStreamPtr& os1, MultiprocessRollingFileAppender* rfa1) :
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
				rfa->setFileLength(File().setPath(rfa->getFile()).length(p));
			}
		}

		OutputStream& getFileOutPutStreamPtr()
		{
			return *os;
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
WriterPtr MultiprocessRollingFileAppender::createWriter(OutputStreamPtr& os)
{
	OutputStreamPtr cos = std::make_shared<CountingOutputStream>(os, this);
	return FileAppender::createWriter(cos);
}

/**
 * Get byte length of current active log file.
 * @return byte length of current active log file.
 */
size_t MultiprocessRollingFileAppender::getFileLength() const
{
	return _priv->fileLength;
}

void MultiprocessRollingFileAppender::setFileLength(size_t length)
{
	_priv->fileLength = length;
}

/**
 * Increments estimated byte length of current active log file.
 * @param increment additional bytes written to log file.
 */
void MultiprocessRollingFileAppender::incrementFileLength(size_t increment)
{
	_priv->fileLength += increment;
}
