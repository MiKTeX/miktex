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
#include <log4cxx/pattern/patternconverter.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/private/patternconverter_priv.h>

using namespace log4cxx;
using namespace log4cxx::pattern;

IMPLEMENT_LOG4CXX_OBJECT(PatternConverter)

PatternConverter::PatternConverter(
	std::unique_ptr<PatternConverterPrivate> priv) :
	m_priv(std::move(priv))
{
}

PatternConverter::PatternConverter(const LogString& name,
	const LogString& style) :
	m_priv(std::make_unique<PatternConverterPrivate>(name, style))
{

}

PatternConverter::~PatternConverter()
{
}

LogString PatternConverter::getName() const
{
	return m_priv->name;
}

LogString PatternConverter::getStyleClass(const log4cxx::helpers::ObjectPtr& /* e */) const
{
	return m_priv->style;
}

void PatternConverter::append(LogString& toAppendTo, const std::string& src)
{
	LOG4CXX_DECODE_CHAR(decoded, src);
	toAppendTo.append(decoded);
}

