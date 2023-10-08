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

#ifndef _LOG4CXX_FILEAPPENDER_PRIV_H
#define _LOG4CXX_FILEAPPENDER_PRIV_H

#include <log4cxx/private/writerappender_priv.h>
#include <log4cxx/fileappender.h>

namespace log4cxx
{

struct FileAppender::FileAppenderPriv : public WriterAppender::WriterAppenderPriv
{
	FileAppenderPriv
		( LayoutPtr _layout = LayoutPtr()
		, const LogString& _fileName = LogString()
		, bool _fileAppend = true
		, bool _bufferedIO = false
		, int _bufferSize = 8 * 1024
		)
		: WriterAppenderPriv(_layout)
		, fileAppend(_fileAppend)
		, fileName(_fileName)
		, bufferedIO(_bufferedIO)
		, bufferSize(_bufferSize)
		{}

	/** Append to or truncate the file? The default value for this
	variable is <code>true</code>, meaning that by default a
	<code>FileAppender</code> will append to an existing file and
	not truncate it.
	<p>This option is meaningful only if the FileAppender opens the
	file.
	*/
	bool fileAppend;

	/**
	The name of the log file. */
	LogString fileName;

	/**
	Do we do bufferedIO? */
	bool bufferedIO;

	/**
	How big should the IO buffer be? Default is 8K. */
	int bufferSize;
};

}

#endif
