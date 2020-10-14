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
	#include <libgen.h>
#endif

#include <log4cxx/logstring.h>
#include <log4cxx/rolling/timebasedrollingpolicy.h>
#include <log4cxx/pattern/filedatepatternconverter.h>
#include <log4cxx/helpers/date.h>
#include <log4cxx/rolling/filerenameaction.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/rolling/gzcompressaction.h>
#include <log4cxx/rolling/zipcompressaction.h>
#include <log4cxx/rolling/rollingfileappenderskeleton.h>
#include<iostream>

#ifndef INT64_C
	#define INT64_C(x) x ## LL
#endif

#include <apr_time.h>

using namespace log4cxx;
using namespace log4cxx::rolling;
using namespace log4cxx::helpers;
using namespace log4cxx::pattern;

IMPLEMENT_LOG4CXX_OBJECT(TimeBasedRollingPolicy)

#ifdef LOG4CXX_MULTI_PROCESS
#define MMAP_FILE_SUFFIX ".map"
#define LOCK_FILE_SUFFIX ".maplck"
#define MAX_FILE_LEN 2048

bool TimeBasedRollingPolicy::isMapFileEmpty(log4cxx::helpers::Pool& pool)
{
	apr_finfo_t finfo;
	apr_status_t st = apr_stat(&finfo, _mapFileName.c_str(), APR_FINFO_SIZE, pool.getAPRPool());

	if (st != APR_SUCCESS)
	{
		LogLog::warn(LOG4CXX_STR("apr_stat failed."));
	}

	if (st == APR_SUCCESS && !finfo.size)
	{
		return true;
	}

	return false;
}

void TimeBasedRollingPolicy::initMMapFile(const LogString& lastFileName, log4cxx::helpers::Pool& pool)
{
	int iRet = 0;

	if (!_mmap)
	{
		iRet = createMMapFile(std::string(_fileNamePattern), pool);
	}

	if (!iRet && isMapFileEmpty(pool))
	{
		lockMMapFile(APR_FLOCK_EXCLUSIVE);
		memset(_mmap->mm, 0, MAX_FILE_LEN);
		memcpy(_mmap->mm, std::string(lastFileName).c_str(), std::string(lastFileName).size());
		unLockMMapFile();
	}
}

const std::string TimeBasedRollingPolicy::createFile(const std::string& fileName, const std::string& suffix, log4cxx::helpers::Pool& pool)
{
	char szUid[MAX_FILE_LEN] = {'\0'};
	char szBaseName[MAX_FILE_LEN] = {'\0'};
	char szDirName[MAX_FILE_LEN] = {'\0'};
	memcpy(szDirName, fileName.c_str(), fileName.size() > MAX_FILE_LEN ? MAX_FILE_LEN : fileName.size());
	memcpy(szBaseName, fileName.c_str(), fileName.size() > MAX_FILE_LEN ? MAX_FILE_LEN : fileName.size());

	apr_uid_t uid;
	apr_gid_t groupid;
	apr_status_t stat = apr_uid_current(&uid, &groupid, pool.getAPRPool());

	if (stat == APR_SUCCESS)
	{
		snprintf(szUid, MAX_FILE_LEN, "%u", uid);
	}

	return std::string(::dirname(szDirName)) + "/." + ::basename(szBaseName) + szUid + suffix;
}

int TimeBasedRollingPolicy::createMMapFile(const std::string& fileName, log4cxx::helpers::Pool& pool)
{
	_mapFileName = createFile(fileName, MMAP_FILE_SUFFIX, pool);

	apr_status_t stat = apr_file_open(&_file_map, _mapFileName.c_str(), APR_CREATE | APR_READ | APR_WRITE, APR_OS_DEFAULT, _mmapPool->getAPRPool());

	if (stat != APR_SUCCESS)
	{
		std::string err(std::string("open mmap file failed. ") + std::string(strerror(errno)) + ". Check the privilege or try to remove " + _mapFileName + " if exist.");
		LogLog::warn(LOG4CXX_STR(err.c_str()));
		return -1;
	}

	if (isMapFileEmpty(pool))
	{
		stat = apr_file_trunc(_file_map, MAX_FILE_LEN + 1);

		if (stat != APR_SUCCESS)
		{
			LogLog::warn(LOG4CXX_STR("apr_file_trunc failed."));
			apr_file_close(_file_map);
			return -1;
		}
	}

	stat = apr_mmap_create(&_mmap, _file_map, 0, MAX_FILE_LEN, APR_MMAP_WRITE | APR_MMAP_READ, _mmapPool->getAPRPool());

	if (stat != APR_SUCCESS)
	{
		LogLog::warn(LOG4CXX_STR("mmap failed."));
		apr_file_close(_file_map);
		return -1;
	}

	return 0;
}

int TimeBasedRollingPolicy::lockMMapFile(int type)
{
	apr_status_t stat = apr_file_lock(_lock_file, type);

	if (stat != APR_SUCCESS)
	{
		LogLog::warn(LOG4CXX_STR("apr_file_lock for mmap failed."));
	}
}
int TimeBasedRollingPolicy::unLockMMapFile()
{
	apr_status_t stat = apr_file_unlock(_lock_file);

	if (stat != APR_SUCCESS)
	{
		LogLog::warn(LOG4CXX_STR("apr_file_unlock for mmap failed."));
	}
}

#endif

TimeBasedRollingPolicy::TimeBasedRollingPolicy()
#ifdef LOG4CXX_MULTI_PROCESS
	: _mmap(NULL), _file_map(NULL), bAlreadyInitialized(false), _mmapPool(new Pool()), _lock_file(NULL), bRefreshCurFile(false)
#endif
{
}

#ifdef LOG4CXX_MULTI_PROCESS
TimeBasedRollingPolicy::~TimeBasedRollingPolicy()
{
	//no-need to delete mmap
	delete _mmapPool;
}
#endif

void TimeBasedRollingPolicy::addRef() const
{
	TriggeringPolicy::addRef();
}

void TimeBasedRollingPolicy::releaseRef() const
{
	TriggeringPolicy::releaseRef();
}

void TimeBasedRollingPolicy::activateOptions(log4cxx::helpers::Pool& pool)
{
	// find out period from the filename pattern
	if (getFileNamePattern().length() > 0)
	{
		parseFileNamePattern();
	}
	else
	{
		LogLog::warn(
			LOG4CXX_STR("The FileNamePattern option must be set before using TimeBasedRollingPolicy. "));
		throw IllegalStateException();
	}

	PatternConverterPtr dtc(getDatePatternConverter());

	if (dtc == NULL)
	{
		throw IllegalStateException();
	}

	apr_time_t n = apr_time_now();
	LogString buf;
	ObjectPtr obj(new Date(n));
	formatFileName(obj, buf, pool);
	lastFileName = buf;

#ifdef LOG4CXX_MULTI_PROCESS

	if (getPatternConverterList().size())
	{
		(*(getPatternConverterList().begin()))->format(obj, _fileNamePattern, pool);
	}
	else
	{
		_fileNamePattern = lastFileName;
	}

	if (!_lock_file)
	{
		const std::string lockname = createFile(std::string(_fileNamePattern), LOCK_FILE_SUFFIX, *_mmapPool);
		apr_status_t stat = apr_file_open(&_lock_file, lockname.c_str(), APR_CREATE | APR_READ | APR_WRITE, APR_OS_DEFAULT, (*_mmapPool).getAPRPool());

		if (stat != APR_SUCCESS)
		{
			LogLog::warn(LOG4CXX_STR("open lock file failed."));
		}
	}

	initMMapFile(lastFileName, *_mmapPool);
#endif

	suffixLength = 0;

	if (lastFileName.length() >= 3)
	{
		if (lastFileName.compare(lastFileName.length() - 3, 3, LOG4CXX_STR(".gz")) == 0)
		{
			suffixLength = 3;
		}
		else if (lastFileName.length() >= 4 && lastFileName.compare(lastFileName.length() - 4, 4, LOG4CXX_STR(".zip")) == 0)
		{
			suffixLength = 4;
		}
	}
}


#define RULES_PUT(spec, cls) \
	specs.insert(PatternMap::value_type(LogString(LOG4CXX_STR(spec)), (PatternConstructor) cls ::newInstance))

log4cxx::pattern::PatternMap TimeBasedRollingPolicy::getFormatSpecifiers() const
{
	PatternMap specs;
	RULES_PUT("d", FileDatePatternConverter);
	RULES_PUT("date", FileDatePatternConverter);
	return specs;
}

/**
 * {@inheritDoc}
 */
RolloverDescriptionPtr TimeBasedRollingPolicy::initialize(
	const   LogString&  currentActiveFile,
	const   bool        append,
	Pool&       pool)
{
	apr_time_t n = apr_time_now();
	nextCheck = ((n / APR_USEC_PER_SEC) + 1) * APR_USEC_PER_SEC;

	File currentFile(currentActiveFile);

	LogString buf;
	ObjectPtr obj(new Date(currentFile.exists(pool) ? currentFile.lastModified(pool) : n));
	formatFileName(obj, buf, pool);
	lastFileName = buf;

	ActionPtr noAction;

	if (currentActiveFile.length() > 0)
	{
		return new RolloverDescription(
				currentActiveFile, append, noAction, noAction);
	}
	else
	{
		bRefreshCurFile = true;
		return new RolloverDescription(
				lastFileName.substr(0, lastFileName.length() - suffixLength), append,
				noAction, noAction);
	}
}

RolloverDescriptionPtr TimeBasedRollingPolicy::rollover(
	const   LogString&  currentActiveFile,
	const   bool        append,
	Pool&       pool)
{
	apr_time_t n = apr_time_now();
	nextCheck = ((n / APR_USEC_PER_SEC) + 1) * APR_USEC_PER_SEC;

	LogString buf;
	ObjectPtr obj(new Date(n));
	formatFileName(obj, buf, pool);

	LogString newFileName(buf);

#ifdef LOG4CXX_MULTI_PROCESS
	bAlreadyInitialized = true;

	if (_mmap && !isMapFileEmpty(*_mmapPool))
	{
		lockMMapFile(APR_FLOCK_SHARED);
		LogString mapLastFile((char*)_mmap->mm);
		lastFileName = mapLastFile;
		unLockMMapFile();
	}
	else
	{
		_mmap = NULL;
		initMMapFile(lastFileName, *_mmapPool);
	}

#endif

	//
	//  if file names haven't changed, no rollover
	//
	if (newFileName == lastFileName)
	{
		RolloverDescriptionPtr desc;
		return desc;
	}

	ActionPtr renameAction;
	ActionPtr compressAction;
	LogString lastBaseName(
		lastFileName.substr(0, lastFileName.length() - suffixLength));
	LogString nextActiveFile(
		newFileName.substr(0, newFileName.length() - suffixLength));

	//
	//   if currentActiveFile is not lastBaseName then
	//        active file name is not following file pattern
	//        and requires a rename plus maintaining the same name
	if (currentActiveFile != lastBaseName)
	{
		renameAction =
			new FileRenameAction(
			File().setPath(currentActiveFile), File().setPath(lastBaseName), true);
		nextActiveFile = currentActiveFile;
	}

	if (suffixLength == 3)
	{
		compressAction =
			new GZCompressAction(
			File().setPath(lastBaseName), File().setPath(lastFileName), true);
	}

	if (suffixLength == 4)
	{
		compressAction =
			new ZipCompressAction(
			File().setPath(lastBaseName), File().setPath(lastFileName), true);
	}

#ifdef LOG4CXX_MULTI_PROCESS

	if (_mmap && !isMapFileEmpty(*_mmapPool))
	{
		lockMMapFile(APR_FLOCK_EXCLUSIVE);
		memset(_mmap->mm, 0, MAX_FILE_LEN);
		memcpy(_mmap->mm, std::string(newFileName).c_str(), std::string(newFileName).size());
		unLockMMapFile();
	}
	else
	{
		_mmap = NULL;
		initMMapFile(newFileName, *_mmapPool);
	}

#else
	lastFileName = newFileName;
#endif

	return new RolloverDescription(nextActiveFile, append, renameAction, compressAction);
}

bool TimeBasedRollingPolicy::isTriggeringEvent(
	Appender* appender,
	const log4cxx::spi::LoggingEventPtr& /* event */,
	const LogString&  filename,
	size_t /* fileLength */)
{
#ifdef LOG4CXX_MULTI_PROCESS

	if (bRefreshCurFile && _mmap && !isMapFileEmpty(*_mmapPool))
	{
		lockMMapFile(APR_FLOCK_SHARED);
		LogString mapCurrent((char*)_mmap->mm);
		unLockMMapFile();
		LogString mapCurrentBase(mapCurrent.substr(0, mapCurrent.length() - suffixLength));

		if (!mapCurrentBase.empty() && mapCurrentBase != filename)
		{
			dynamic_cast<FileAppender*>(appender)->setFile(mapCurrentBase);
		}
	}

	return ((apr_time_now()) > nextCheck) || (!bAlreadyInitialized);
#else
	return apr_time_now() > nextCheck;
#endif
}
