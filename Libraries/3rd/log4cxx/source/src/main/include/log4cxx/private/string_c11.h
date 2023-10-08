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

#ifndef LOG4CXX_STRING_C11_H
#define LOG4CXX_STRING_C11_H
#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>

#if !defined(__STDC_LIB_EXT1__) && !defined(__STDC_SECURE_LIB__)
#include <limits.h>
#if !defined(RSIZE_MAX) && defined(SSIZE_MAX)
#define RSIZE_MAX (SSIZE_MAX >> 4)
#else // !defined(RSIZE_MAX)
#define RSIZE_MAX (2 << 20)
#endif

static size_t strnlen_s( const char *str, size_t strsz )
{
	size_t result = 0;
	if (!str)
		;
	else while (*str++ != 0 && result < strsz)
		++result;
	return result;
}
static int strcat_s(char* destArg, size_t destsz, const char* src)
{
	if (!src || !destArg || RSIZE_MAX < destsz)
		return -1;
	if (0 == destsz)
		return -2;
	--destsz;
	char* dest = destArg;
	size_t index = 0;
	while (*dest && index < destsz)
		++index, ++dest;
	while (*src && index < destsz)
	{
		*dest++ = *src++;
		++index;
	}
	*dest = 0;
	if (*src) // longer than destsz?
	{
		*destArg = 0; // Do not return a partial result
		return -3;
	}
	return 0;
}
#endif

#endif /* LOG4CXX_STRING_C11_H */
