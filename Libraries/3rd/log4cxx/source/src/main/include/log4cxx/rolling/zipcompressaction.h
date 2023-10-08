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

#if !defined(_LOG4CXX_ROLLING_ZIP_COMPRESS_ACTION_H)
#define _LOG4CXX_ROLLING_ZIP_COMPRESS_ACTION_H

#include <log4cxx/rolling/action.h>
#include <log4cxx/file.h>

namespace log4cxx
{
namespace rolling
{


class ZipCompressAction : public Action
{
		struct ZipCompressActionPrivate;
	public:
		DECLARE_ABSTRACT_LOG4CXX_OBJECT(ZipCompressAction)
		BEGIN_LOG4CXX_CAST_MAP()
		LOG4CXX_CAST_ENTRY(ZipCompressAction)
		LOG4CXX_CAST_ENTRY_CHAIN(Action)
		END_LOG4CXX_CAST_MAP()

		/**
		 * Constructor.
		 */
		ZipCompressAction(const File& source,
			const File& destination,
			bool deleteSource);

		/**
		 * Perform action.
		 *
		 * @return true if successful.
		 */
		bool execute(log4cxx::helpers::Pool& pool) const override;

		/**
		 * Set to true to throw an IOException on a fork failure.  By default, this
		 * is true.  When an IOException is thrown, this will automatically cause the
		 * error handler to be called(which is the recommended way of handling this
		 * problem).  By setting this to false, the ZipCompressAction effectively
		 * turns into a FileRenameAction if any errors are encountered.
		 *
		 * @param throwIO
		 */
		void setThrowIOExceptionOnForkFailure(bool throwIO);

	private:
		ZipCompressAction(const ZipCompressAction&);
		ZipCompressAction& operator=(const ZipCompressAction&);
};

LOG4CXX_PTR_DEF(ZipCompressAction);

}

}
#endif

