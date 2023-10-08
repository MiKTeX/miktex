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
#include <log4cxx/rolling/rollingpolicybase.h>
#include <log4cxx/pattern/formattinginfo.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/pattern/patternparser.h>
#include <log4cxx/pattern/integerpatternconverter.h>
#include <log4cxx/pattern/datepatternconverter.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/private/rollingpolicybase_priv.h>

using namespace log4cxx;
using namespace log4cxx::rolling;
using namespace log4cxx::helpers;
using namespace log4cxx::pattern;

IMPLEMENT_LOG4CXX_OBJECT(RollingPolicyBase)

RollingPolicyBase::RollingPolicyBase() :
	m_priv(std::make_unique<RollingPolicyBasePrivate>())
{
}

RollingPolicyBase::RollingPolicyBase( std::unique_ptr<RollingPolicyBasePrivate> priv ) :
	m_priv(std::move(priv)){
}

RollingPolicyBase::~RollingPolicyBase()
{
}

void RollingPolicyBase::activateOptions(log4cxx::helpers::Pool& /* pool */)
{
	if (m_priv->fileNamePatternStr.length() > 0)
	{
		parseFileNamePattern();
	}
	else
	{
		LogString msg(LOG4CXX_STR("The FileNamePattern option must be set before using FixedWindowRollingPolicy."));
		LogString ref1(LOG4CXX_STR("See also http://logging.apache.org/log4j/codes.html#tbr_fnp_not_set"));
		LogLog::warn(msg);
		LogLog::warn(ref1);
		throw IllegalStateException();
	}
}


void RollingPolicyBase::setOption(const LogString& option, const LogString& value)
{
	if (StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("FILENAMEPATTERN"),
			LOG4CXX_STR("filenamepattern")))
	{
		m_priv->fileNamePatternStr = value;
	}else if (StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("CREATEINTERMEDIATEDIRECTORIES"),
			LOG4CXX_STR("createintermediatedirectories")))
	{
		m_priv->createIntermediateDirectories = OptionConverter::toBoolean(value, false);
	}
}

void RollingPolicyBase::setFileNamePattern(const LogString& fnp)
{
	m_priv->fileNamePatternStr = fnp;
}


LogString RollingPolicyBase::getFileNamePattern() const
{
	return m_priv->fileNamePatternStr;
}

/**
 *   Parse file name pattern.
 */
void RollingPolicyBase::parseFileNamePattern()
{
	m_priv->patternConverters.erase(m_priv->patternConverters.begin(), m_priv->patternConverters.end());
	m_priv->patternFields.erase(m_priv->patternFields.begin(), m_priv->patternFields.end());
	PatternParser::parse(m_priv->fileNamePatternStr,
		m_priv->patternConverters,
		m_priv->patternFields,
		getFormatSpecifiers());
}

/**
 * Format file name.
 *
 * @param obj object to be evaluted in formatting, may not be null.
 * @param buf string buffer to which formatted file name is appended, may not be null.
 */
void RollingPolicyBase::formatFileName(
	const ObjectPtr& obj,
	LogString& toAppendTo,
	Pool& pool) const
{
	std::vector<FormattingInfoPtr>::const_iterator formatterIter =
		m_priv->patternFields.begin();

	for (std::vector<PatternConverterPtr>::const_iterator
		converterIter = m_priv->patternConverters.begin();
		converterIter != m_priv->patternConverters.end();
		converterIter++, formatterIter++)
	{
		auto startField = toAppendTo.length();
		(*converterIter)->format(obj, toAppendTo, pool);
		(*formatterIter)->format((int)startField, toAppendTo);
	}
}


PatternConverterPtr RollingPolicyBase::getIntegerPatternConverter() const
{
	for (std::vector<PatternConverterPtr>::const_iterator
		converterIter = m_priv->patternConverters.begin();
		converterIter != m_priv->patternConverters.end();
		converterIter++)
	{
		IntegerPatternConverterPtr intPattern;
		PatternConverterPtr patternptr = (*converterIter);
		intPattern = log4cxx::cast<IntegerPatternConverter>(patternptr);

		if (intPattern != NULL)
		{
			return *converterIter;
		}
	}

	PatternConverterPtr noMatch;
	return noMatch;
}

PatternConverterPtr RollingPolicyBase::getDatePatternConverter() const
{
	for (std::vector<PatternConverterPtr>::const_iterator
		converterIter = m_priv->patternConverters.begin();
		converterIter != m_priv->patternConverters.end();
		converterIter++)
	{
		DatePatternConverterPtr datePattern;
		PatternConverterPtr patternptr = (*converterIter);
		datePattern = log4cxx::cast<DatePatternConverter>(patternptr);

		if (datePattern != NULL)
		{
			return *converterIter;
		}
	}

	PatternConverterPtr noMatch;
	return noMatch;
}

bool RollingPolicyBase::getCreateIntermediateDirectories() const{
	return m_priv->createIntermediateDirectories;
}

void RollingPolicyBase::setCreateIntermediateDirectories(bool createIntermediate){
	m_priv->createIntermediateDirectories = createIntermediate;
}

PatternConverterList RollingPolicyBase::getPatternConverterList() const
{
	return m_priv->patternConverters;
}
