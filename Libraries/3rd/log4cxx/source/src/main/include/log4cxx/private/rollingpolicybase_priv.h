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

#include <log4cxx/helpers/writer.h>
#include <log4cxx/writerappender.h>
#include <log4cxx/rolling/rollingpolicybase.h>
#include <atomic>

#ifndef _LOG4CXX_ROLLINGPOLICYBASE_PRIV_H
#define _LOG4CXX_ROLLINGPOLICYBASE_PRIV_H

namespace log4cxx
{
namespace rolling{

struct RollingPolicyBase::RollingPolicyBasePrivate {
    /**
     * File name pattern converters.
     */
    PatternConverterList patternConverters;

    /**
     * File name field specifiers.
     */
    FormattingInfoList patternFields;

    /**
     * File name pattern.
     */
    LogString fileNamePatternStr;

	bool createIntermediateDirectories = true;
};

}

}

#endif /* _LOG4CXX_ROLLINGPOLICYBASE_PRIV_H */
