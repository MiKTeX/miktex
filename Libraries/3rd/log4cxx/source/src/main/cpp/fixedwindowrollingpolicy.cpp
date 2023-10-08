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
#include <log4cxx/rolling/fixedwindowrollingpolicy.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/integer.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/rolling/rolloverdescription.h>
#include <log4cxx/rolling/filerenameaction.h>
#include <log4cxx/rolling/gzcompressaction.h>
#include <log4cxx/rolling/zipcompressaction.h>
#include <log4cxx/pattern/integerpatternconverter.h>
#include <log4cxx/private/rollingpolicybase_priv.h>

using namespace log4cxx;
using namespace log4cxx::rolling;
using namespace log4cxx::helpers;
using namespace log4cxx::pattern;

#define priv static_cast<FixedWindowRollingPolicyPrivate*>(m_priv.get())

struct FixedWindowRollingPolicy::FixedWindowRollingPolicyPrivate : public RollingPolicyBasePrivate {
	FixedWindowRollingPolicyPrivate() :
		RollingPolicyBasePrivate(),
		minIndex(1),
		maxIndex(7),
		explicitActiveFile(false)
	{}

	int minIndex;
	int maxIndex;
	bool explicitActiveFile;
	bool throwIOExceptionOnForkFailure = true;
};

IMPLEMENT_LOG4CXX_OBJECT(FixedWindowRollingPolicy)

FixedWindowRollingPolicy::FixedWindowRollingPolicy() :
	RollingPolicyBase (std::make_unique<FixedWindowRollingPolicyPrivate>())
{
}

FixedWindowRollingPolicy::~FixedWindowRollingPolicy(){}

void FixedWindowRollingPolicy::setMaxIndex(int maxIndex1)
{
	priv->maxIndex = maxIndex1;
}

void FixedWindowRollingPolicy::setMinIndex(int minIndex1)
{
	priv->minIndex = minIndex1;
}

void FixedWindowRollingPolicy::setOption(const LogString& option,
	const LogString& value)
{
	if (StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("MININDEX"),
			LOG4CXX_STR("minindex")))
	{
		priv->minIndex = OptionConverter::toInt(value, 1);
	}
	else if (StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("MAXINDEX"),
			LOG4CXX_STR("maxindex")))
	{
		priv->maxIndex = OptionConverter::toInt(value, 7);
	}
	else if (StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("THROWIOEXCEPTIONONFORKFAILURE"),
			LOG4CXX_STR("throwioexceptiononforkfailure")))
	{
		priv->throwIOExceptionOnForkFailure = OptionConverter::toBoolean(value, true);
	}
	else
	{
		RollingPolicyBase::setOption(option, value);
	}
}

/**
 * {@inheritDoc}
 */
void FixedWindowRollingPolicy::activateOptions(Pool& p)
{
	RollingPolicyBase::activateOptions(p);

	if (priv->maxIndex < priv->minIndex)
	{
		LogLog::warn(
			LOG4CXX_STR("MaxIndex  cannot be smaller than MinIndex."));
		priv->maxIndex = priv->minIndex;
	}

	if ((priv->maxIndex - priv->minIndex) > MAX_WINDOW_SIZE)
	{
		LogLog::warn(LOG4CXX_STR("Large window sizes are not allowed."));
		priv->maxIndex = priv->minIndex + MAX_WINDOW_SIZE;
	}

	PatternConverterPtr itc = getIntegerPatternConverter();

	if (itc == NULL)
	{
		throw IllegalStateException();
	}
}

/**
 * {@inheritDoc}
 */
RolloverDescriptionPtr FixedWindowRollingPolicy::initialize(
	const   LogString&  currentActiveFile,
	const   bool        append,
	Pool&       pool)
{
	LogString newActiveFile(currentActiveFile);
	priv->explicitActiveFile = false;

	if (currentActiveFile.length() > 0)
	{
		priv->explicitActiveFile = true;
		newActiveFile = currentActiveFile;
	}

	if (!priv->explicitActiveFile)
	{
		LogString buf;
		ObjectPtr obj = std::make_shared<Integer>(priv->minIndex);
		formatFileName(obj, buf, pool);
		newActiveFile = buf;
	}

	ActionPtr noAction;

	return std::make_shared<RolloverDescription>(newActiveFile, append, noAction, noAction);
}

/**
 * {@inheritDoc}
 */
RolloverDescriptionPtr FixedWindowRollingPolicy::rollover(
	const   LogString&  currentActiveFile,
	const   bool        append,
	Pool&       pool)
{
	RolloverDescriptionPtr desc;

	if (priv->maxIndex < 0)
	{
		return desc;
	}

	int purgeStart = priv->minIndex;

	if (!priv->explicitActiveFile)
	{
		purgeStart++;
	}

	if (!purge(purgeStart, priv->maxIndex, pool))
	{
		return desc;
	}

	LogString buf;
	ObjectPtr obj = std::make_shared<Integer>(purgeStart);
	formatFileName(obj, buf, pool);

	LogString renameTo(buf);
	LogString compressedName(renameTo);
	ActionPtr compressAction ;

	if(getCreateIntermediateDirectories()){
		File compressedFile(compressedName);
		File compressedParent (compressedFile.getParent(pool));
		compressedParent.mkdirs(pool);
	}

	if (StringHelper::endsWith(renameTo, LOG4CXX_STR(".gz")))
	{
		renameTo.resize(renameTo.size() - 3);
		GZCompressActionPtr comp = std::make_shared<GZCompressAction>(
					File().setPath(renameTo),
					File().setPath(compressedName),
					true);
		comp->setThrowIOExceptionOnForkFailure(priv->throwIOExceptionOnForkFailure);
		compressAction = comp;
	}
	else if (StringHelper::endsWith(renameTo, LOG4CXX_STR(".zip")))
	{
		renameTo.resize(renameTo.size() - 4);
		ZipCompressActionPtr comp = std::make_shared<ZipCompressAction>(
					File().setPath(renameTo),
					File().setPath(compressedName),
					true);
		comp->setThrowIOExceptionOnForkFailure(priv->throwIOExceptionOnForkFailure);
		compressAction = comp;
	}

	auto renameAction = std::make_shared<FileRenameAction>(
				File().setPath(currentActiveFile),
				File().setPath(renameTo),
				false);

	desc = std::make_shared<RolloverDescription>(
				currentActiveFile,  append,
				renameAction,       compressAction);

	return desc;
}

/**
 * Get index of oldest log file to be retained.
 * @return index of oldest log file.
 */
int FixedWindowRollingPolicy::getMaxIndex() const
{
	return priv->maxIndex;
}

/**
 * Get index of most recent log file.
 * @return index of oldest log file.
 */
int FixedWindowRollingPolicy::getMinIndex() const
{
	return priv->minIndex;
}


/**
 * Purge and rename old log files in preparation for rollover
 * @param lowIndex low index
 * @param highIndex high index.  Log file associated with high
 * index will be deleted if needed.
 * @return true if purge was successful and rollover should be attempted.
 */
bool FixedWindowRollingPolicy::purge(int lowIndex, int highIndex, Pool& p) const
{
	int suffixLength = 0;

	std::vector<FileRenameActionPtr> renames;
	LogString buf;
	ObjectPtr obj = std::make_shared<Integer>(lowIndex);
	formatFileName(obj, buf, p);

	LogString lowFilename(buf);

	if (lowFilename.compare(lowFilename.length() - 3, 3, LOG4CXX_STR(".gz")) == 0)
	{
		suffixLength = 3;
	}
	else if (lowFilename.compare(lowFilename.length() - 4, 4, LOG4CXX_STR(".zip")) == 0)
	{
		suffixLength = 4;
	}

	for (int i = lowIndex; i <= highIndex; i++)
	{
		File toRenameCompressed;
		toRenameCompressed.setPath(lowFilename);
		File toRenameBase;
		toRenameBase.setPath(lowFilename.substr(0, lowFilename.length() - suffixLength));
		File* toRename = &toRenameCompressed;
		bool isBase = false;
		bool exists = toRenameCompressed.exists(p);

		if (suffixLength > 0)
		{
			if (exists)
			{
				if (toRenameBase.exists(p))
				{
					toRenameBase.deleteFile(p);
				}
			}
			else
			{
				toRename = &toRenameBase;
				exists = toRenameBase.exists(p);
				isBase = true;
			}
		}

		if (exists)
		{
			//
			//    if at upper index then
			//        attempt to delete last file
			//        if that fails then abandon purge
			if (i == highIndex)
			{
				if (!toRename->deleteFile(p))
				{
					return false;
				}

				break;
			}

			//
			//   if intermediate index
			//     add a rename action to the list
			buf.erase(buf.begin(), buf.end());
			obj = std::make_shared<Integer>(i + 1);
			formatFileName(obj, buf, p);

			LogString highFilename(buf);
			LogString renameTo(highFilename);

			if (isBase)
			{
				renameTo =
					highFilename.substr(0, highFilename.length() - suffixLength);
			}

			renames.push_back(std::make_shared<FileRenameAction>(*toRename, File().setPath(renameTo), true));
			lowFilename = highFilename;
		}
		else
		{
			break;
		}
	}

	//
	//   work renames backwards
	//
	for (std::vector<FileRenameActionPtr>::reverse_iterator iter = renames.rbegin();
		iter != renames.rend();
		iter++)
	{

		try
		{
			if (!(*iter)->execute(p))
			{
				return false;
			}
		}
		catch (std::exception&)
		{
			LogLog::warn(LOG4CXX_STR("Exception during purge in RollingFileAppender"));

			return false;
		}
	}

	return true;
}

#define RULES_PUT(spec, cls) \
	specs.insert(PatternMap::value_type(LogString(LOG4CXX_STR(spec)), (PatternConstructor) cls ::newInstance))


log4cxx::pattern::PatternMap FixedWindowRollingPolicy::getFormatSpecifiers() const
{
	PatternMap specs;
	RULES_PUT("i", IntegerPatternConverter);
	RULES_PUT("index", IntegerPatternConverter);
	return specs;
}
