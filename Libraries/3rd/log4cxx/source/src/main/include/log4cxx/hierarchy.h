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

#ifndef _LOG4CXX_HIERARCHY_H
#define _LOG4CXX_HIERARCHY_H


#include <log4cxx/spi/loggerrepository.h>
#include <log4cxx/spi/loggerfactory.h>
#include <vector>
#include <map>
#include <log4cxx/provisionnode.h>
#include <log4cxx/helpers/object.h>
#include <log4cxx/spi/hierarchyeventlistener.h>
#include <log4cxx/helpers/pool.h>

namespace log4cxx
{

class Hierarchy;
LOG4CXX_PTR_DEF(Hierarchy);

/**
This class is specialized in retrieving loggers by name and also
maintaining the logger hierarchy.

<p><em>The casual user does not have to deal with this class
directly.</em>

<p>The structure of the logger hierarchy is maintained by the
#getLogger method. The hierarchy is such that children link
to their parent but parents do not have any pointers to their
children. Moreover, loggers can be instantiated in any order, in
particular descendant before ancestor.

<p>In case a descendant is created before a particular ancestor,
then it creates a provision node for the ancestor and adds itself
to the provision node. Other descendants of the same ancestor add
themselves to the previously created provision node.
*/
class LOG4CXX_EXPORT Hierarchy : public spi::LoggerRepository
{
	private:
		LOG4CXX_DECLARE_PRIVATE_MEMBER_PTR(HierarchyPrivate, m_priv)

	public:
		DECLARE_ABSTRACT_LOG4CXX_OBJECT(Hierarchy)
		BEGIN_LOG4CXX_CAST_MAP()
		LOG4CXX_CAST_ENTRY(spi::LoggerRepository)
		END_LOG4CXX_CAST_MAP()

	private:
		/**
		Create a new logger hierarchy.
		*/
		Hierarchy();

	public:
		static HierarchyPtr create();

		~Hierarchy();

		void addHierarchyEventListener(const spi::HierarchyEventListenerPtr& listener) override;

		/**
		 * Call \c configurator if not yet configured.
		 */
		void ensureIsConfigured(std::function<void()> configurator) override;

		/**
		This call will clear all logger definitions from the internal
		hashtable. Invoking this method will irrevocably mess up the
		logger hierarchy.

		<p>You should <em>really</em> know what you are doing before
		invoking this method.
		*/
		void clear();

		void emitNoAppenderWarning(const Logger* logger) override;

		/**
		Check if the named logger exists in the hierarchy. If so return
		its reference, otherwise returns <code>null</code>.

		  @param name The name of the logger to search for.

		*/
		LoggerPtr exists(const LogString& name) override;

		/**
		The string form of {@link #setThreshold(const LevelPtr&) setThreshold}.
		*/
		void setThreshold(const LogString& levelStr) override;

		/**
		Enable logging for logging requests with level <code>l</code> or
		higher. By default all levels are enabled.

		        @param l The minimum level for which logging requests are sent to
		their appenders.  */
		void setThreshold(const LevelPtr& l) override;

		void fireAddAppenderEvent(const Logger* logger, const Appender* appender) override;

		void fireRemoveAppenderEvent(const Logger* logger, const Appender* appender) override;

		/**
		Returns a Level representation of the <code>enable</code>
		state.
		*/
		LevelPtr getThreshold() const override;

		/**
		Retrieve the \c name Logger instance using
		the default factory to create it if required.

		If a logger of that name already exists, then it will be
		returned.  Otherwise, a new logger will be instantiated and
		then linked with its existing ancestors as well as children.

		@param name The name of the logger to retrieve.

		*/
		LoggerPtr getLogger(const LogString& name) override;

		/**
		Retrieve the \c name Logger instance using
		<code>factory</code> to create it if required.

		If a logger of that name already exists, then it will be
		returned.  Otherwise, a new logger will be instantiated by the
		<code>factory</code> parameter and linked with its existing
		ancestors as well as children.

		@param name The name of the logger to retrieve.
		@param factory The factory that will make the new logger instance.

		*/
		LoggerPtr getLogger(const LogString& name,
			const spi::LoggerFactoryPtr& factory) override;

		/**
		Returns all the currently defined loggers in this hierarchy as
		a LoggerList.

		<p>The root logger is <em>not</em> included in the returned
		LoggerList.  */
		LoggerList getCurrentLoggers() const override;

		/**
		Get the root of this hierarchy.
		*/
		LoggerPtr getRootLogger() const override;

		/**
		This method will return <code>true</code> if this repository is
		disabled for <code>level</code> object passed as parameter and
		<code>false</code> otherwise. See also the
		{@link #setThreshold(const LevelPtr&) setThreshold} method.  */
		bool isDisabled(int level) const override;

		/**
		Reset all values contained in this hierarchy instance to their
		default.  This removes all appenders from all categories, sets
		the level of all non-root categories to <code>null</code>,
		sets their additivity flag to <code>true</code> and sets the level
		of the root logger to DEBUG.  Moreover,
		message disabling is set its default "off" value.

		<p>Existing categories are not removed. They are just reset.

		<p>This method should be used sparingly and with care as it will
		block all logging until it is completed.</p>
		*/
		void resetConfiguration() override;

		/**
		Used by subclasses to add a renderer to the hierarchy passed as parameter.
		*/
		/**
		Shutting down a hierarchy will <em>safely</em> close and remove
		all appenders in all categories including the root logger.

		<p>Some appenders such as {@link net::XMLSocketAppender XMLSocketAppender}
		and AsyncAppender need to be closed before the
		application exists. Otherwise, pending logging events might be
		lost.

		<p>The <code>shutdown</code> method is careful to close nested
		appenders before closing regular appenders. This is allows
		configurations where a regular appender is attached to a logger
		and again to a nested appender.
		*/
		void shutdown() override;


		virtual bool isConfigured() override;
		virtual void setConfigured(bool configured) override;

		/**
		Refresh the threshold in children of parent
		*/
		void updateChildren(const Logger* parent);

		void clearAppenders();

		void addAppender(AppenderPtr appender);

	private:

		/**
		 * Set the threshold.  The mutex must already be locked.
		 */
		void setThresholdInternal(const LevelPtr& l);

		/**
		 * Internal shutdown.  The mutex must already be locked.
		 */
		void shutdownInternal();

		/**
		This method loops through all the *potential* parents of
		\c logger using the logger name.
		For example, for a logger named "w.x.y.z",
		loop through "w.x.y", "w.x" and "w", but not "w.x.y.z".
		There 3 possible cases:

		1) No entry for the potential parent of "w.x.y.z" exists

		We create a ProvisionNode for this potential parent and insert
		"w.x.y.z" in that provision node.

		2) There entry is of type Logger for the potential parent.

		The entry is "w.x.y.z"'s nearest existing parent. We update "w.x.y.z"'s
		parent field with this entry. We also break from the loop
		because updating our parent's parent is our parent's
		responsibility.

		3) There entry is of type ProvisionNode for this potential parent.

		We add "w.x.y.z" to the list of children for this potential parent.
		*/
		void updateParents(const LoggerPtr& logger, const LoggerPtr& root);

		/**
		We update the links for all the children that placed themselves
		in the provision node 'pn'. The \c logger argument is a
		newly created Logger, a potential parent of all the
		children in 'pn'

		We loop on all the children 'c' in 'pn':

		If the child 'c' has been already linked to a child of
		'cat' then there is no need to update 'c'.

		Otherwise, we set the \c logger parent to 'c's parent and set
		'c's parent field to \c logger.
		*/
		void updateChildren(ProvisionNode& pn, const LoggerPtr& logger);

		Hierarchy(const Hierarchy&);
		Hierarchy& operator=(const Hierarchy&);

};

}  //namespace log4cxx

#endif //_LOG4CXX_HIERARCHY_H
