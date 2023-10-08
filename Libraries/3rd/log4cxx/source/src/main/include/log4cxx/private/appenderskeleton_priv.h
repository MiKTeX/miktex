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

#ifndef _LOG4CXX_APPENDERSKELETON_PRIV
#define _LOG4CXX_APPENDERSKELETON_PRIV

#include <log4cxx/appenderskeleton.h>
#include <log4cxx/helpers/onlyonceerrorhandler.h>
#include <memory>

namespace log4cxx
{

struct AppenderSkeleton::AppenderSkeletonPrivate
{
	AppenderSkeletonPrivate() :
		threshold(Level::getAll()),
		errorHandler(std::make_shared<log4cxx::helpers::OnlyOnceErrorHandler>()),
		closed(false) {}

	AppenderSkeletonPrivate( LayoutPtr lay ) :
		layout( lay ),
		threshold(Level::getAll()),
		errorHandler(std::make_shared<log4cxx::helpers::OnlyOnceErrorHandler>()),
		closed(false) {}

	virtual ~AppenderSkeletonPrivate(){}

	/** The layout variable does not need to be set if the appender
	implementation has its own layout. */
	LayoutPtr layout;

	/** Appenders are named. */
	LogString name;

	/**
	There is no level threshold filtering by default.  */
	LevelPtr threshold;

	/**
	It is assumed and enforced that errorHandler is never null.
	*/
	spi::ErrorHandlerPtr errorHandler;

	/** The first filter in the filter chain. Set to <code>null</code>
	initially. */
	spi::FilterPtr headFilter;

	/** The last filter in the filter chain. */
	spi::FilterPtr tailFilter;

	/**
	Is this appender closed?
	*/
	bool closed;

	log4cxx::helpers::Pool pool;
	mutable std::recursive_mutex mutex;
};

}

#endif /* _LOG4CXX_APPENDERSKELETON_PRIV */
