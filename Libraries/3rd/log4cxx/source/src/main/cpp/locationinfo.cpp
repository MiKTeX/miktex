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

#include <log4cxx/spi/location/locationinfo.h>
#include <log4cxx/helpers/pool.h>

using namespace ::log4cxx::spi;
using namespace log4cxx::helpers;

/**
  When location information is not available the constant
  <code>NA</code> is returned. Current value of this string
  constant is <b>?</b>.  */
const char* const LocationInfo::NA = "?";
const char* const LocationInfo::NA_METHOD = "?::?";

const LocationInfo& LocationInfo::getLocationUnavailable()
{
	static const LocationInfo unavailable;
	return unavailable;
}

/**
*   Constructor.
*   @remarks Used by LOG4CXX_LOCATION to generate
*       location info for current code site
*/
LocationInfo::LocationInfo( const char* const fileName1,
	const char* const shortFileName1,
	const char* const methodName1,
	int lineNumber1 )
	:  lineNumber( lineNumber1 ),
	   fileName( fileName1 ),
	   shortFileName(shortFileName1),
	   methodName( methodName1 )
{
}

/**
*   Default constructor.
*/
LocationInfo::LocationInfo()
	: lineNumber( -1 ),
	  fileName(LocationInfo::NA),
	  shortFileName(LocationInfo::NA),
	  methodName(LocationInfo::NA_METHOD)
{
}

/**
*   Copy constructor.
*   @param src source location
*/
LocationInfo::LocationInfo( const LocationInfo& src )
	:  lineNumber( src.lineNumber ),
	   fileName( src.fileName ),
	   shortFileName( src.shortFileName ),
	   methodName( src.methodName )
{
}

/**
*  Assignment operator.
* @param src source location
*/
LocationInfo& LocationInfo::operator = ( const LocationInfo& src )
{
	fileName = src.fileName;
	methodName = src.methodName;
	lineNumber = src.lineNumber;
	return * this;
}

/**
 *   Resets location info to default state.
 */
void LocationInfo::clear()
{
	fileName = NA;
	methodName = NA_METHOD;
	lineNumber = -1;
}


/**
 *   Return the file name of the caller.
 *   @returns file name, may be null.
 */
const char* LocationInfo::getFileName() const
{
	return fileName;
}

const char* LocationInfo::getShortFileName() const{
	return shortFileName;
}

/**
  *   Returns the line number of the caller.
  * @returns line number, -1 if not available.
  */
int LocationInfo::getLineNumber() const
{
	return lineNumber;
}

/** Returns the method name of the caller. */
const std::string LocationInfo::getMethodName() const
{
	std::string tmp(methodName);
	size_t parenPos = tmp.find('(');

	if (parenPos != std::string::npos)
	{
		tmp.erase(parenPos);
	}

	size_t colonPos = tmp.rfind("::");

	if (colonPos != std::string::npos)
	{
		tmp.erase(0, colonPos + 2);
	}
	else
	{
		size_t spacePos = tmp.find(' ');

		if (spacePos != std::string::npos)
		{
			tmp.erase(0, spacePos + 1);
		}
	}

	return tmp;
}


const std::string LocationInfo::getClassName() const
{
	std::string tmp(methodName);
	size_t parenPos = tmp.find('(');

	if (parenPos != std::string::npos)
	{
		tmp.erase(parenPos);
	}

	size_t colonPos = tmp.rfind("::");

	if (colonPos != std::string::npos)
	{
		tmp.erase(colonPos);
		size_t spacePos = tmp.find_last_of(' ');

		if (spacePos != std::string::npos)
		{
			tmp.erase(0, spacePos + 1);
		}

		return tmp;
	}

	tmp.erase(0, tmp.length() );
	return tmp;
}


