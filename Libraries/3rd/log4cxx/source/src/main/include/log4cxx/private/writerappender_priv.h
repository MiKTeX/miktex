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
#include <atomic>

#include "appenderskeleton_priv.h"

#ifndef _LOG4CXX_WRITERAPPENDER_PRIV_H
#define _LOG4CXX_WRITERAPPENDER_PRIV_H

namespace log4cxx
{

struct WriterAppender::WriterAppenderPriv : public AppenderSkeleton::AppenderSkeletonPrivate
{
	WriterAppenderPriv() :
		AppenderSkeletonPrivate(),
		immediateFlush(true) {}

	WriterAppenderPriv(const LayoutPtr& layout1,
		log4cxx::helpers::WriterPtr& writer1) :
		AppenderSkeletonPrivate(layout1),
		immediateFlush(true),
		writer(writer1)
	{
	}

	WriterAppenderPriv(const LayoutPtr& layout1) :
		AppenderSkeletonPrivate(layout1),
		immediateFlush(true)
	{
	}

	/**
	Immediate flush means that the underlying writer or output stream
	will be flushed at the end of each append operation. Immediate
	flush is slower but ensures that each append request is actually
	written. If <code>immediateFlush</code> is set to
	<code>false</code>, then there is a good chance that the last few
	logs events are not actually written to persistent media if and
	when the application crashes.

	<p>The <code>immediateFlush</code> variable is set to
	<code>true</code> by default.

	*/
	std::atomic<bool> immediateFlush;

	/**
	The encoding to use when opening an input stream.
	<p>The <code>encoding</code> variable is set to <code>""</code> by
	default which results in the utilization of the system's default
	encoding.  */
	LogString encoding;

	/**
	*  This is the {@link Writer Writer} where we will write to.
	*/
	log4cxx::helpers::WriterPtr writer;
};

}

#endif /* _LOG4CXX_WRITERAPPENDER_PRIV_H */
