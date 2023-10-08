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

#ifndef _LOG4CXX_SPI_DEFAULT_REPOSITORY_SELECTOR_H
#define _LOG4CXX_SPI_DEFAULT_REPOSITORY_SELECTOR_H

#include <log4cxx/spi/repositoryselector.h>
#include <log4cxx/helpers/object.h>
#include <log4cxx/spi/loggerrepository.h>
#include <memory>

namespace log4cxx
{
namespace spi
{
class LOG4CXX_EXPORT DefaultRepositorySelector :
	public virtual RepositorySelector,
	public virtual helpers::Object
{
	public:
		DECLARE_ABSTRACT_LOG4CXX_OBJECT(DefaultRepositorySelector)
		BEGIN_LOG4CXX_CAST_MAP()
		LOG4CXX_CAST_ENTRY(RepositorySelector)
		END_LOG4CXX_CAST_MAP()

		DefaultRepositorySelector(const LoggerRepositoryPtr repository1);
		~DefaultRepositorySelector();
		LoggerRepositoryPtr getLoggerRepository() override;

	private:
		LOG4CXX_DECLARE_PRIVATE_MEMBER_PTR(DefaultRepositorySelectorPrivate, m_priv)
};
}  // namespace spi
} // namespace log4cxx

#endif //_LOG4CXX_SPI_DEFAULT_REPOSITORY_SELECTOR_H
