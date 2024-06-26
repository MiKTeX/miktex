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

#if !defined(_LOG4CXX_ROLLING_FILE_RENAME_ACTION_H)
#define _LOG4CXX_ROLLING_FILE_RENAME_ACTION_H

#include <log4cxx/rolling/action.h>
#include <log4cxx/file.h>

namespace log4cxx
{
namespace rolling
{


class FileRenameAction : public Action
{
		struct FileRenameActionPrivate;
	public:
		DECLARE_ABSTRACT_LOG4CXX_OBJECT(FileRenameAction)
		BEGIN_LOG4CXX_CAST_MAP()
		LOG4CXX_CAST_ENTRY(FileRenameAction)
		LOG4CXX_CAST_ENTRY_CHAIN(Action)
		END_LOG4CXX_CAST_MAP()

		/**
		 * Constructor.
		 */
		FileRenameAction(const File& toRename,
			const File& renameTo,
			bool renameEmptyFile);

		/**
		 * Perform action.
		 *
		 * @return true if successful.
		 */
		bool execute(log4cxx::helpers::Pool& pool) const override;
};

LOG4CXX_PTR_DEF(FileRenameAction);

}
}
#endif

