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

#ifndef _LOG4CXX_SPI_LOCATION_LOCATIONINFO_H
#define _LOG4CXX_SPI_LOCATION_LOCATIONINFO_H

#include <log4cxx/log4cxx.h>
#include <string>

#if __cpp_lib_string_view || (_MSVC_LANG >= 201703L)
#include <string_view>
#define LOG4CXX_HAS_STRING_VIEW
#else
#include <string.h>
#endif

#if defined(_WIN32)
#define LOG4CXX_SHORT_FILENAME_SPLIT_CHAR '\\'
#else
#define LOG4CXX_SHORT_FILENAME_SPLIT_CHAR '/'
#endif

namespace log4cxx
{
namespace spi
{
/**
 * This class represents the location of a logging statement.
 *
 */
class LOG4CXX_EXPORT LocationInfo
{
	public:



		/**
		  *   When location information is not available the constant
		  * <code>NA</code> is returned. Current value of this string constant is <b>?</b>.
		  */
		static const char* const NA;
		static const char* const NA_METHOD;

		static const LocationInfo& getLocationUnavailable();

#ifdef LOG4CXX_HAS_STRING_VIEW
		static constexpr const char* calcShortFileName(const char* fileName){
			std::string_view view(fileName);
			// If the separator is not found, rfind will return -1.  Adding 1 to
			// that will have it pointing at fileName, which is a good fallback.
			return fileName + view.rfind(LOG4CXX_SHORT_FILENAME_SPLIT_CHAR) + 1;
		}
#else
		static const char* calcShortFileName(const char* fileName){
			const char* location = strrchr(fileName, LOG4CXX_SHORT_FILENAME_SPLIT_CHAR);
			return location == nullptr ? fileName : location + 1;
		}
#endif

		/**
		 *   Constructor.
		 *   @remarks Used by LOG4CXX_LOCATION to generate
		 *       location info for current code site
		 */
		LocationInfo( const char* const fileName,
					  const char* const shortFileName,
					  const char* const functionName,
					  int lineNumber);

		/**
		 *   Default constructor.
		 */
		LocationInfo();

		/**
		 *   Copy constructor.
		 *   @param src source location
		 */
		LocationInfo( const LocationInfo& src );

		/**
		 *  Assignment operator.
		 * @param src source location
		 */
		LocationInfo& operator = ( const LocationInfo& src );

		/**
		 *   Resets location info to default state.
		 */
		void clear();


		/** Return the class name of the call site. */
		const std::string getClassName() const;

		/**
		 *   Return the file name of the caller.
		 *   @returns file name, may be null.
		 */
		const char* getFileName() const;

		/**
		 *   Return the short file name of the caller.
		 *   @returns file name.  Note that this will fallback to the full filename when using
		 *    calcShortFileName to calculate the filename at compile-time.
		 */
		const char* getShortFileName() const;

		/**
		  *   Returns the line number of the caller.
		  * @returns line number, -1 if not available.
		  */
		int getLineNumber() const;

		/** Returns the method name of the caller. */
		const std::string getMethodName() const;


	private:
		/** Caller's line number. */
		int lineNumber;

		/** Caller's file name. */
		const char* fileName;

  		/** Caller's short file name. */
		const char* shortFileName;

		/** Caller's method name. */
		const char* methodName;


};
}
}

#if !defined(LOG4CXX_LOCATION) && !LOG4CXX_DISABLE_LOCATION_INFO
#if defined(_MSC_VER)
	#if _MSC_VER >= 1300
		#define __LOG4CXX_FUNC__ __FUNCSIG__
	#endif
#else
	#if defined(__GNUC__)
		#define __LOG4CXX_FUNC__ __PRETTY_FUNCTION__
	#else
		#if defined(__BORLANDC__)
			#define __LOG4CXX_FUNC__ __FUNC__
		#endif
	#endif
#endif
#if !defined(__LOG4CXX_FUNC__)
	#define __LOG4CXX_FUNC__ ""
#endif


#define LOG4CXX_LOCATION ::log4cxx::spi::LocationInfo(__FILE__,         \
	::log4cxx::spi::LocationInfo::calcShortFileName(__FILE__), \
	__LOG4CXX_FUNC__, \
	__LINE__)

#else
#define LOG4CXX_LOCATION ::log4cxx::spi::LocationInfo::getLocationUnavailable()
#endif // LOG4CXX_LOCATION

#endif //_LOG4CXX_SPI_LOCATION_LOCATIONINFO_H
