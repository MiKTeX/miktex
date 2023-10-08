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

#ifndef _LOG4CXX_DB_ODBC_APPENDER_H
#define _LOG4CXX_DB_ODBC_APPENDER_H

#include <log4cxx/log4cxx.h>

#include <log4cxx/helpers/exception.h>
#include <log4cxx/appenderskeleton.h>
#include <log4cxx/spi/loggingevent.h>
#include <list>
#include <memory>

namespace log4cxx
{
namespace db
{
class LOG4CXX_EXPORT SQLException : public log4cxx::helpers::Exception
{
	public:
		SQLException(short fHandleType,
			void* hInput, const char* prolog,
			log4cxx::helpers::Pool& p);
		SQLException(const char* msg);
		SQLException(const SQLException& src);
	private:
		const char* formatMessage(short fHandleType,
			void* hInput, const char* prolog,
			log4cxx::helpers::Pool& p);
};

/**
The ODBCAppender sends log events to a database.

<p>Each append call adds the spi::LoggingEvent to a buffer.
When the buffer is full, values are extracted from each spi::LoggingEvent
and the sql insert statement executed.

The SQL insert statement pattern must be provided
either in the Log4cxx configuration file
using the <b>sql</b> parameter element
or programatically by calling the <code>setSql(String sql)</code> method.

The following <b>param</b> elements are optional:
- one of <b>DSN</b>, <b>URL</b>, <b>ConnectionString</b> -
  The <b>serverName</b> parameter value in the <a href="https://learn.microsoft.com/en-us/sql/odbc/reference/syntax/sqlconnect-function">SQLConnect</a> call.
- <b>User</b> -
  The <b>UserName</b> parameter value in the <a href="https://learn.microsoft.com/en-us/sql/odbc/reference/syntax/sqlconnect-function">SQLConnect</a> call.
- <b>Password</b> -
  The <b>Authentication</b> parameter value in the <a href="https://learn.microsoft.com/en-us/sql/odbc/reference/syntax/sqlconnect-function">SQLConnect</a> call.
- <b>BufferSize</b> -
  Delay executing the sql until this many logging events are available.
  One by default, meaning an sql statement is executed
  whenever a logging event is appended.
- <b>ColumnMapping</b> -
  One element for each "?" in the <b>sql</b> statement
  in a sequence corresponding to the columns in the insert statement.
  The following values are supported:
  - <b>logger</b> - the name of the logger that generated the logging event
  - <b>level</b> - the level of the logging event
  - <b>thread</b> - the thread number as a hex string that generated the logging event
  - <b>threadname</b> - the name assigned to the thread that generated the logging event
  - <b>time</b> - a datetime or datetime2 SQL field type at which the event was generated
  - <b>shortfilename</b> - the basename of the file containing the logging statement
  - <b>fullfilename</b> - the path of the file containing the logging statement
  - <b>line</b> - the position in the file at which the logging event was generated
  - <b>class</b> - the class from which the logging event was generated (\ref usingMacros "1")
  - <b>method</b> - the function in which the logging event was generated (\ref usingMacros "1")
  - <b>message</b> - the data sent by the logging statement
  - <b>mdc</b> - A JSON format string of all entries in the logging thread's mapped diagnostic context
  - <b>mdc{key}</b> - the value associated with the <b>key</b> entry in the logging thread's mapped diagnostic context 
  - <b>ndc</b> - the last entry the logging thread's nested diagnostic context 

\anchor usingMacros 1. Only available when the LOG4CXX_* macros are used to issue the logging request.

<p>For use as a base class:

<ul>

<li>Override getConnection() to pass any connection
you want.  Typically this is used to enable application wide
connection pooling.

<li>Override closeConnection -- if
you override getConnection make sure to implement
<code>closeConnection</code> to handle the connection you
generated.  Typically this would return the connection to the
pool it came from.

</ul>

An example configuration that writes to the data source named "LoggingDSN" is:
~~~{.xml}
<log4j:configuration xmlns:log4j="http://jakarta.apache.org/log4j/">
<appender name="PreparedAppender" class="ODBCAppender">
 <param name="DSN" value="LoggingDSN"/>
 <param name="sql" value="INSERT INTO [SomeDatabaseName].[SomeUserName].[SomeTableName] ([Thread],[LogName],[LogTime],[LogLevel],[FileName],[FileLine],[Message],[MappedContext]) VALUES (?,?,?,?,?,?,?,?)" />
 <param name="ColumnMapping" value="thread"/>
 <param name="ColumnMapping" value="logger"/>
 <param name="ColumnMapping" value="time"/>
 <param name="ColumnMapping" value="level"/>
 <param name="ColumnMapping" value="shortfilename"/>
 <param name="ColumnMapping" value="line"/>
 <param name="ColumnMapping" value="message"/>
 <param name="ColumnMapping" value="mdc"/>
</appender>
<appender name="ASYNC" class="AsyncAppender">
  <param name="BufferSize" value="1000"/>
  <param name="Blocking" value="false"/>
  <appender-ref ref="PreparedAppender"/>
</appender>
<root>
  <priority value ="INFO" />
  <appender-ref ref="ASYNC" />
</root>
</log4j:configuration>
~~~

You may also want to consider the DBAppender class, which uses APR in order to support logging to databases apart from ODBC.
*/

class LOG4CXX_EXPORT ODBCAppender : public AppenderSkeleton
{
	public:
		DECLARE_LOG4CXX_OBJECT(ODBCAppender)
		BEGIN_LOG4CXX_CAST_MAP()
		LOG4CXX_CAST_ENTRY(ODBCAppender)
		LOG4CXX_CAST_ENTRY_CHAIN(AppenderSkeleton)
		END_LOG4CXX_CAST_MAP()

		typedef void* SQLHDBC;
		typedef void* SQLHENV;
		typedef void* SQLHANDLE;
		typedef short SQLSMALLINT;

		ODBCAppender();
		virtual ~ODBCAppender();

		/**
		Set options
		*/
		void setOption(const LogString& option, const LogString& value) override;

		/**
		Activate the specified options.
		*/
		void activateOptions(helpers::Pool& p) override;

		/**
		* Adds the event to the buffer.  When full the buffer is flushed.
		*/
		void append(const spi::LoggingEventPtr& event, helpers::Pool&) override;

	protected:
		/**
		* To be removed.
		*/
		LogString getLogStatement(const spi::LoggingEventPtr& event,
			helpers::Pool& p) const;

		/**
		*
		* To be removed.
		* */
		virtual void execute(const LogString& sql,
			log4cxx::helpers::Pool& p) /*throw(SQLException)*/;

		/**
		* Override this to return the connection to a pool, or to clean up the
		* resource.
		*
		* The default behavior holds a single connection open until the appender
		* is closed (typically when garbage collected).
		*/
		virtual void closeConnection(SQLHDBC con);

		/**
		* Override this to link with your connection pooling system.
		*
		* By default this creates a single connection which is held open
		* until the object is garbage collected.
		*/
		virtual SQLHDBC getConnection(log4cxx::helpers::Pool& p) /*throw(SQLException)*/;

		/**
		* Closes the appender, flushing the buffer first then closing the default
		* connection if it is open.
		*/
	public:
		void close() override;

		/**
		* loops through the buffer of LoggingEvents, gets a
		* sql string from getLogStatement() and sends it to execute().
		* Errors are sent to the errorHandler.
		*
		* If a statement fails the LoggingEvent stays in the buffer!
		*/
		virtual void flushBuffer(log4cxx::helpers::Pool& p);

		/**
		* Does this appender require a layout?
		* */
		bool requiresLayout() const override;

		/**
		* Set pre-formated statement eg: insert into LogTable (msg) values ("%m")
		*/
		void setSql(const LogString& s);

		/**
		* Returns pre-formated statement eg: insert into LogTable (msg) values ("%m")
		*/
		const LogString& getSql() const;


		void setUser(const LogString& user);

		void setURL(const LogString& url);

		void setPassword(const LogString& password);

		void setBufferSize(size_t newBufferSize);

		const LogString& getUser() const;

		const LogString& getURL() const;

		const LogString& getPassword() const;

		size_t getBufferSize() const;
	private:
		ODBCAppender(const ODBCAppender&);
		ODBCAppender& operator=(const ODBCAppender&);
#if LOG4CXX_WCHAR_T_API || LOG4CXX_LOGCHAR_IS_WCHAR_T || defined(WIN32) || defined(_WIN32)
		static void encode(wchar_t** dest, const LogString& src,
			log4cxx::helpers::Pool& p);
#endif
		static void encode(unsigned short** dest, const LogString& src,
			log4cxx::helpers::Pool& p);

	protected:
		struct ODBCAppenderPriv;
}; // class ODBCAppender
LOG4CXX_PTR_DEF(ODBCAppender);

} // namespace db
} // namespace log4cxx

#endif // _LOG4CXX_DB_ODBC_APPENDER_H
