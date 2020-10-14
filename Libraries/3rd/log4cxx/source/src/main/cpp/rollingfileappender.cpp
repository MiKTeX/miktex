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

#if defined(_MSC_VER)
	#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

#ifdef LOG4CXX_MULTI_PROCESS
	#include <apr_portable.h>
	#include <libgen.h>
	#include <apr_file_io.h>
	#include <apr_atomic.h>
	#include <apr_mmap.h>
	#ifndef MAX_FILE_LEN
		#define MAX_FILE_LEN 2048
	#endif
	#include <log4cxx/pattern/filedatepatternconverter.h>
	#include <log4cxx/helpers/date.h>
#endif

#include <log4cxx/rolling/rollingfileappender.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/synchronized.h>
#include <log4cxx/rolling/rolloverdescription.h>
#include <log4cxx/helpers/fileoutputstream.h>
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/rolling/fixedwindowrollingpolicy.h>
#include <log4cxx/rolling/manualtriggeringpolicy.h>

using namespace log4cxx;
using namespace log4cxx::rolling;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;


IMPLEMENT_LOG4CXX_OBJECT(RollingFileAppenderSkeleton)
IMPLEMENT_LOG4CXX_OBJECT(RollingFileAppender)


/**
 * Construct a new instance.
 */
RollingFileAppenderSkeleton::RollingFileAppenderSkeleton() : _event(NULL)
{
}

RollingFileAppender::RollingFileAppender()
{
}

/**
 * Prepare instance of use.
 */
void RollingFileAppenderSkeleton::activateOptions(Pool& p)
{
	if (rollingPolicy == NULL)
	{
		FixedWindowRollingPolicy* fwrp = new FixedWindowRollingPolicy();
		fwrp->setFileNamePattern(getFile() + LOG4CXX_STR(".%i"));
		rollingPolicy = fwrp;
	}

	//
	//  if no explicit triggering policy and rolling policy is both.
	//
	if (triggeringPolicy == NULL)
	{
		TriggeringPolicyPtr trig(rollingPolicy);

		if (trig != NULL)
		{
			triggeringPolicy = trig;
		}
	}

	if (triggeringPolicy == NULL)
	{
		triggeringPolicy = new ManualTriggeringPolicy();
	}

	{
		LOCK_W sync(mutex);
		triggeringPolicy->activateOptions(p);
		rollingPolicy->activateOptions(p);

		try
		{
			RolloverDescriptionPtr rollover1 =
				rollingPolicy->initialize(getFile(), getAppend(), p);

			if (rollover1 != NULL)
			{
				ActionPtr syncAction(rollover1->getSynchronous());

				if (syncAction != NULL)
				{
					syncAction->execute(p);
				}

				setFile(rollover1->getActiveFileName());
				setAppend(rollover1->getAppend());

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
				fileLength = activeFile.length(p);
			}
			else
			{
				fileLength = 0;
			}

			FileAppender::activateOptions(p);
		}
		catch (std::exception&)
		{
			LogLog::warn(
				LogString(LOG4CXX_STR("Exception will initializing RollingFileAppender named "))
				+ getName());
		}
	}
}

#ifdef LOG4CXX_MULTI_PROCESS
void RollingFileAppenderSkeleton::releaseFileLock(apr_file_t* lock_file)
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
#endif
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
bool RollingFileAppenderSkeleton::rollover(Pool& p)
{
	//
	//   can't roll without a policy
	//
	if (rollingPolicy != NULL)
	{

		{
			LOCK_W sync(mutex);

#ifdef LOG4CXX_MULTI_PROCESS
			std::string fileName(getFile());
			RollingPolicyBase* basePolicy = dynamic_cast<RollingPolicyBase* >(&(*rollingPolicy));
			apr_time_t n = apr_time_now();
			ObjectPtr obj(new Date(n));
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
			apr_status_t stat = apr_uid_current(&uid, &groupid, pool.getAPRPool());

			if (stat == APR_SUCCESS)
			{
				snprintf(szUid, MAX_FILE_LEN, "%u", uid);
			}

			const std::string lockname = std::string(::dirname(szDirName)) + "/." + ::basename(szBaseName) + szUid + ".lock";
			apr_file_t* lock_file;
			stat = apr_file_open(&lock_file, lockname.c_str(), APR_CREATE | APR_READ | APR_WRITE, APR_OS_DEFAULT, p.getAPRPool());

			if (stat != APR_SUCCESS)
			{
				std::string err = "lockfile return error: open lockfile failed. ";
				err += (strerror(errno));
				LogLog::warn(LOG4CXX_STR(err.c_str()));
				bAlreadyRolled = false;
				lock_file = NULL;
			}
			else
			{
				stat = apr_file_lock(lock_file, APR_FLOCK_EXCLUSIVE);

				if (stat != APR_SUCCESS)
				{
					std::string err = "apr_file_lock: lock failed. ";
					err += (strerror(errno));
					LogLog::warn(LOG4CXX_STR(err.c_str()));
					bAlreadyRolled = false;
				}
				else
				{
					if (_event)
					{
						triggeringPolicy->isTriggeringEvent(this, *_event, getFile(), getFileLength());
					}
				}
			}

			if (bAlreadyRolled)
			{
				apr_finfo_t finfo1, finfo2;
				apr_status_t st1, st2;
				apr_file_t* _fd = getWriter()->getOutPutStreamPtr()->getFileOutPutStreamPtr().getFilePtr();
				st1 = apr_file_info_get(&finfo1, APR_FINFO_IDENT, _fd);

				if (st1 != APR_SUCCESS)
				{
					LogLog::warn(LOG4CXX_STR("apr_file_info_get failed"));
				}

				st2 = apr_stat(&finfo2, std::string(getFile()).c_str(), APR_FINFO_IDENT, p.getAPRPool());

				if (st2 != APR_SUCCESS)
				{
					LogLog::warn(LOG4CXX_STR("apr_stat failed."));
				}

				bAlreadyRolled = ((st1 == APR_SUCCESS) && (st2 == APR_SUCCESS)
						&& ((finfo1.device != finfo2.device) || (finfo1.inode != finfo2.inode)));
			}

			if (!bAlreadyRolled)
			{
#endif

				try
				{
					RolloverDescriptionPtr rollover1(rollingPolicy->rollover(this->getFile(), this->getAppend(), p));

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
								catch (std::exception&)
								{
									LogLog::warn(LOG4CXX_STR("Exception on rollover"));
								}
							}

							if (success)
							{
								if (rollover1->getAppend())
								{
									fileLength = File().setPath(rollover1->getActiveFileName()).length(p);
								}
								else
								{
									fileLength = 0;
								}

								//
								//  async action not yet implemented
								//
								ActionPtr asyncAction(rollover1->getAsynchronous());

								if (asyncAction != NULL)
								{
									asyncAction->execute(p);
								}

								setFile(
									rollover1->getActiveFileName(), rollover1->getAppend(),
									bufferedIO, bufferSize, p);
							}
							else
							{
								setFile(
									rollover1->getActiveFileName(), true, bufferedIO, bufferSize, p);
							}
						}
						else
						{
							OutputStreamPtr os(new FileOutputStream(
									rollover1->getActiveFileName(), rollover1->getAppend()));
							WriterPtr newWriter(createWriter(os));
							closeWriter();
							setFile(rollover1->getActiveFileName());
							setWriter(newWriter);

							bool success = true;

							if (rollover1->getSynchronous() != NULL)
							{
								success = false;

								try
								{
									success = rollover1->getSynchronous()->execute(p);
								}
								catch (std::exception&)
								{
									LogLog::warn(LOG4CXX_STR("Exception during rollover"));
								}
							}

							if (success)
							{
								if (rollover1->getAppend())
								{
									fileLength = File().setPath(rollover1->getActiveFileName()).length(p);
								}
								else
								{
									fileLength = 0;
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

#ifdef LOG4CXX_MULTI_PROCESS
						releaseFileLock(lock_file);
#endif
						return true;
					}
				}
				catch (std::exception&)
				{
					LogLog::warn(LOG4CXX_STR("Exception during rollover"));
				}

#ifdef LOG4CXX_MULTI_PROCESS
			}
			else
			{
				reopenLatestFile(p);
			}

			releaseFileLock(lock_file);
#endif
		}
	}

	return false;
}

#ifdef LOG4CXX_MULTI_PROCESS
/**
 * re-open current file when its own handler has been renamed
 */
void RollingFileAppenderSkeleton::reopenLatestFile(Pool& p)
{
	closeWriter();
	OutputStreamPtr os(new FileOutputStream(getFile(), true));
	WriterPtr newWriter(createWriter(os));
	setFile(getFile());
	setWriter(newWriter);
	fileLength = File().setPath(getFile()).length(p);
	writeHeader(p);
}

#endif

/**
 * {@inheritDoc}
*/
void RollingFileAppenderSkeleton::subAppend(const LoggingEventPtr& event, Pool& p)
{
	// The rollover check must precede actual writing. This is the
	// only correct behavior for time driven triggers.
	if (
		triggeringPolicy->isTriggeringEvent(
			this, event, getFile(), getFileLength()))
	{
		//
		//   wrap rollover request in try block since
		//    rollover may fail in case read access to directory
		//    is not provided.  However appender should still be in good
		//     condition and the append should still happen.
		try
		{
			_event = &(const_cast<LoggingEventPtr&>(event));
			rollover(p);
		}
		catch (std::exception&)
		{
			LogLog::warn(LOG4CXX_STR("Exception during rollover attempt."));
		}
	}

#ifdef LOG4CXX_MULTI_PROCESS
	//do re-check before every write
	//
	apr_finfo_t finfo1, finfo2;
	apr_status_t st1, st2;
	apr_file_t* _fd = getWriter()->getOutPutStreamPtr()->getFileOutPutStreamPtr().getFilePtr();
	st1 = apr_file_info_get(&finfo1, APR_FINFO_IDENT, _fd);

	if (st1 != APR_SUCCESS)
	{
		LogLog::warn(LOG4CXX_STR("apr_file_info_get failed"));
	}

	st2 = apr_stat(&finfo2, std::string(getFile()).c_str(), APR_FINFO_IDENT, p.getAPRPool());

	if (st2 != APR_SUCCESS)
	{
		std::string err = "apr_stat failed. file:" + std::string(getFile());
		LogLog::warn(LOG4CXX_STR(err.c_str()));
	}

	bool bAlreadyRolled = ((st1 == APR_SUCCESS) && (st2 == APR_SUCCESS)
			&& ((finfo1.device != finfo2.device) || (finfo1.inode != finfo2.inode)));

	if (bAlreadyRolled)
	{
		reopenLatestFile(p);
	}

#endif

	FileAppender::subAppend(event, p);
}

/**
 * Get rolling policy.
 * @return rolling policy.
 */
RollingPolicyPtr RollingFileAppenderSkeleton::getRollingPolicy() const
{
	return rollingPolicy;
}

/**
 * Get triggering policy.
 * @return triggering policy.
 */
TriggeringPolicyPtr RollingFileAppenderSkeleton::getTriggeringPolicy() const
{
	return triggeringPolicy;
}

/**
 * Sets the rolling policy.
 * @param policy rolling policy.
 */
void RollingFileAppenderSkeleton::setRollingPolicy(const RollingPolicyPtr& policy)
{
	rollingPolicy = policy;
}

/**
 * Set triggering policy.
 * @param policy triggering policy.
 */
void RollingFileAppenderSkeleton::setTriggeringPolicy(const TriggeringPolicyPtr& policy)
{
	triggeringPolicy = policy;
}

/**
 * Close appender.  Waits for any asynchronous file compression actions to be completed.
 */
void RollingFileAppenderSkeleton::close()
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
		RollingFileAppenderSkeleton* rfa;

	public:
		/**
		 * Constructor.
		 * @param os output stream to wrap.
		 * @param rfa rolling file appender to inform.
		 */
		CountingOutputStream(
			OutputStreamPtr& os1, RollingFileAppenderSkeleton* rfa1) :
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
#ifndef LOG4CXX_MULTI_PROCESS
				rfa->incrementFileLength(buf.limit());
#else
				rfa->setFileLength(File().setPath(rfa->getFile()).length(p));
#endif
			}
		}

#ifdef LOG4CXX_MULTI_PROCESS
		OutputStream& getFileOutPutStreamPtr()
		{
			return *os;
		}
#endif
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
WriterPtr RollingFileAppenderSkeleton::createWriter(OutputStreamPtr& os)
{
	OutputStreamPtr cos(new CountingOutputStream(os, this));
	return FileAppender::createWriter(cos);
}

/**
 * Get byte length of current active log file.
 * @return byte length of current active log file.
 */
size_t RollingFileAppenderSkeleton::getFileLength() const
{
	return fileLength;
}

#ifdef LOG4CXX_MULTI_PROCESS
void RollingFileAppenderSkeleton::setFileLength(size_t length)
{
	fileLength = length;
}
#endif

/**
 * Increments estimated byte length of current active log file.
 * @param increment additional bytes written to log file.
 */
void RollingFileAppenderSkeleton::incrementFileLength(size_t increment)
{
	fileLength += increment;
}
