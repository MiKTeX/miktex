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
#ifndef LOG4CXX_PATTERNCONVERTER_PRIVATE_H
#define LOG4CXX_PATTERNCONVERTER_PRIVATE_H
#include <log4cxx/pattern/patternconverter.h>

namespace log4cxx
{
namespace pattern
{

/**
 * Create a new pattern converter.
 * @param name name for pattern converter.
 * @param style CSS style for formatted output.
 */
struct PatternConverter::PatternConverterPrivate
{
	PatternConverterPrivate( const LogString& _name, const LogString& _style ) :
		name(_name),
		style(_style) {}

	virtual ~PatternConverterPrivate(){}

	/**
	 * Converter name.
	 */
	const LogString name;
	/**
	 * Converter style name.
	 */
	const LogString style;
};

}
}

#endif /* LOG4CXX_PATTERNCONVERTER_PRIVATE_H */
