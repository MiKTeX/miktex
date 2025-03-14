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

#ifndef _LOG4CXX_SPI_HIERARCHY_EVENT_LISTENER_H
#define _LOG4CXX_SPI_HIERARCHY_EVENT_LISTENER_H

#include <log4cxx/helpers/object.h>
#include <vector>

namespace log4cxx
{
class Logger;
class Appender;


namespace spi
{

/** Listen to events occuring within a Hierarchy.*/
class LOG4CXX_EXPORT HierarchyEventListener :
	public virtual log4cxx::helpers::Object
{
	public:
		virtual ~HierarchyEventListener() {}

		virtual void addAppenderEvent(
			const Logger* logger,
			const Appender* appender) = 0;

		virtual void removeAppenderEvent(
			const Logger* logger,
			const Appender* appender) = 0;
};
LOG4CXX_PTR_DEF(HierarchyEventListener);
LOG4CXX_LIST_DEF(HierarchyEventListenerList, HierarchyEventListenerPtr);

}  // namespace spi
} // namespace log4cxx

#endif //_LOG4CXX_SPI_HIERARCHY_EVENT_LISTENER_H
