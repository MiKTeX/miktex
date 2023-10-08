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

#ifndef _LOG4CXX_HELPER_FORMATTING_INFO_H
#define _LOG4CXX_HELPER_FORMATTING_INFO_H


#include <log4cxx/helpers/object.h>
#include <log4cxx/logstring.h>

namespace log4cxx
{
namespace pattern
{


class FormattingInfo;
typedef std::shared_ptr<FormattingInfo> FormattingInfoPtr;


/**
 * Modifies the output of a pattern converter for a specified minimum
 * and maximum width and alignment.
 *
 *
 *
 *
 */
class LOG4CXX_EXPORT FormattingInfo : public virtual log4cxx::helpers::Object
{
		LOG4CXX_DECLARE_PRIVATE_MEMBER_PTR(FormattingInfoPrivate, m_priv)

	public:
		DECLARE_ABSTRACT_LOG4CXX_OBJECT(FormattingInfo)
		BEGIN_LOG4CXX_CAST_MAP()
		LOG4CXX_CAST_ENTRY(FormattingInfo)
		END_LOG4CXX_CAST_MAP()


		/**
		 * Creates new instance.
		 * @param leftAlign left align if true.
		 * @param minLength minimum length.
		 * @param maxLength maximum length.
		 */
		FormattingInfo(
			const bool leftAlign, const int minLength, const int maxLength);
		~FormattingInfo();

		/**
		 * Gets default instance.
		 * @return default instance.
		 */
		static FormattingInfoPtr getDefault();

		/**
		 * Determine if left aligned.
		 * @return true if left aligned.
		 */
		bool isLeftAligned() const;

		/**
		 * Get minimum length.
		 * @return minimum length.
		 */
		int getMinLength() const;

		/**
		 * Get maximum length.
		 * @return maximum length.
		 */
		int getMaxLength() const;

		/**
		 * Adjust the content of the buffer based on the specified lengths and alignment.
		 *
		 * @param fieldStart start of field in buffer.
		 * @param buffer buffer to be modified.
		 */
		void format(const int fieldStart, LogString& buffer) const;
};
LOG4CXX_PTR_DEF(FormattingInfo);
}
}


#endif
