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

#if !defined(_LOG4CXX_ROLLING_ROLLING_POLICY_BASE_H)
#define _LOG4CXX_ROLLING_ROLLING_POLICY_BASE_H

#include <log4cxx/helpers/object.h>
#include <log4cxx/logger.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/rolling/rollingpolicy.h>
#include <log4cxx/pattern/patternconverter.h>
#include <log4cxx/pattern/formattinginfo.h>
#include <log4cxx/pattern/patternparser.h>

namespace log4cxx
{
namespace rolling
{
LOG4CXX_LIST_DEF(PatternConverterList, log4cxx::pattern::PatternConverterPtr);
LOG4CXX_LIST_DEF(FormattingInfoList, log4cxx::pattern::FormattingInfoPtr);

/**
 * Implements methods common to most, it not all, rolling
 * policies.
 *
 *
 *
 */
class LOG4CXX_EXPORT RollingPolicyBase :
	public virtual RollingPolicy,
	public virtual helpers::Object
{
	protected:
		DECLARE_ABSTRACT_LOG4CXX_OBJECT(RollingPolicyBase)
		BEGIN_LOG4CXX_CAST_MAP()
		LOG4CXX_CAST_ENTRY(RollingPolicy)
		LOG4CXX_CAST_ENTRY(spi::OptionHandler)
		END_LOG4CXX_CAST_MAP()

		LOG4CXX_DECLARE_PRIVATE_MEMBER_PTR(RollingPolicyBasePrivate, m_priv)

	public:
		RollingPolicyBase();
		virtual ~RollingPolicyBase();
		void activateOptions(log4cxx::helpers::Pool& p) override;
		virtual log4cxx::pattern::PatternMap getFormatSpecifiers() const = 0;

		void setOption(const LogString& option, const LogString& value) override;

		/**
		 * Set file name pattern.
		 * @param fnp file name pattern.
		 */
		void setFileNamePattern(const LogString& fnp);

		/**
		 * Get file name pattern.
		 * @return file name pattern.
		 */
		LogString getFileNamePattern() const;

		bool getCreateIntermediateDirectories() const;
		void setCreateIntermediateDirectories(bool createIntermediate);

		PatternConverterList getPatternConverterList() const;

	protected:
		RollingPolicyBase(LOG4CXX_PRIVATE_PTR(RollingPolicyBasePrivate) priv);
		/**
		 *   Parse file name pattern.
		 */
		void parseFileNamePattern();

		/**
		 * Format file name.
		 *
		 * @param obj object to be evaluted in formatting, may not be null.
		 * @param buf string buffer to which formatted file name is appended, may not be null.
		 * @param p memory pool.
		 */
		void formatFileName(const helpers::ObjectPtr& obj,
			LogString& buf, helpers::Pool& p) const;

		log4cxx::pattern::PatternConverterPtr getIntegerPatternConverter() const;
		log4cxx::pattern::PatternConverterPtr getDatePatternConverter() const;
};

LOG4CXX_PTR_DEF(RollingPolicyBase);

}
}

#endif
