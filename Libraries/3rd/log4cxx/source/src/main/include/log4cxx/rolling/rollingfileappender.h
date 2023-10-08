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

#if !defined(_LOG4CXX_ROLLING_ROLLING_FILE_APPENDER_H)
#define _LOG4CXX_ROLLING_ROLLING_FILE_APPENDER_H

#include <log4cxx/fileappender.h>
#include <log4cxx/spi/optionhandler.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/rolling/triggeringpolicy.h>
#include <log4cxx/rolling/rollingpolicy.h>
#include <log4cxx/rolling/action.h>

namespace log4cxx
{
namespace rolling
{


/**
 * <code>RollingFileAppender</code> extends {@link log4cxx::FileAppender} to backup the log files
 * depending on {@link log4cxx::rolling::RollingPolicy RollingPolicy} and {@link log4cxx::rolling::TriggeringPolicy TriggeringPolicy}.
 * <p>
 * To be of any use, a <code>RollingFileAppender</code> instance must have both
 * a <code>RollingPolicy</code> and a <code>TriggeringPolicy</code> set up.
 * However, if its <code>RollingPolicy</code> also implements the
 * <code>TriggeringPolicy</code> interface, then only the former needs to be
 * set up. For example, {@link log4cxx::rolling::TimeBasedRollingPolicy TimeBasedRollingPolicy} acts both as a
 * <code>RollingPolicy</code> and a <code>TriggeringPolicy</code>.
 *
 * <p><code>RollingFileAppender</code> can be configured programattically or
 * using {@link log4cxx::xml::DOMConfigurator}. Here is a sample
 * configration file:

<pre>&lt;?xml version="1.0" encoding="UTF-8" ?>
&lt;!DOCTYPE log4j:configuration>

&lt;log4j:configuration debug="true">

  &lt;appender name="ROLL" class="org.apache.log4j.rolling.RollingFileAppender">
    <b>&lt;rollingPolicy class="org.apache.log4j.rolling.TimeBasedRollingPolicy">
      &lt;param name="FileNamePattern" value="/wombat/foo.%d{yyyy-MM}.gz"/>
    &lt;/rollingPolicy></b>

    &lt;layout class="org.apache.log4j.PatternLayout">
      &lt;param name="ConversionPattern" value="%c{1} - %m%n"/>
    &lt;/layout>
  &lt;/appender>

  &lt;root>
    &lt;appender-ref ref="ROLL"/>
  &lt;/root>

&lt;/log4j:configuration>
</pre>

 *<p>This configuration file specifies a monthly rollover schedule including
 * automatic compression of the archived files. See
 * {@link TimeBasedRollingPolicy} for more details.
 *
 *
 *
 *
 * */
class LOG4CXX_EXPORT RollingFileAppender : public FileAppender
{
		DECLARE_LOG4CXX_OBJECT(RollingFileAppender)
		BEGIN_LOG4CXX_CAST_MAP()
		LOG4CXX_CAST_ENTRY(RollingFileAppender)
		LOG4CXX_CAST_ENTRY_CHAIN(FileAppender)
		END_LOG4CXX_CAST_MAP()
	protected:
		struct RollingFileAppenderPriv;

	public:
		RollingFileAppender();

		/** Returns the value of the <b>MaxBackupIndex</b> option. */
		int getMaxBackupIndex() const;

		/** Get the maximum size that the output file is allowed to reach before being rolled over to backup files. */
		size_t getMaximumFileSize() const;


		/**
		Set the maximum number of backup files to keep around.

		<p>The <b>MaxBackupIndex</b> option determines how many backup
		 files are kept before the oldest is erased. This option takes
		 a positive integer value. If set to zero, then there will be no
		 backup files and the log file will be truncated when it reaches <code>MaxFileSize</code>.
		*/
		void setMaxBackupIndex( int maxBackupIndex );

		/**
		Set the maximum size that the output file is allowed to reach before being rolled over to backup files.

		<p>In configuration files, the <b>MaxFileSize</b> option takes an
		 long integer in the range 0 - 2^63. You can specify the value with the suffixes "KB", "MB" or "GB" so that the integer is
		 interpreted being expressed respectively in kilobytes, megabytes
		 or gigabytes. For example, the value "10KB" will be interpreted as 10240.
		*/
		void setMaxFileSize( const LogString& value );

		void setMaximumFileSize( size_t value );

		/**
		   The <b>DatePattern</b> takes a string in the same format as
		   expected by {@link log4cxx::helpers::SimpleDateFormat SimpleDateFormat}. This options determines the
		   rollover schedule.
		 */
		void setDatePattern(const LogString& pattern);

		LogString makeFileNamePattern(const LogString& datePattern);

		void setOption( const LogString& option, const LogString& value ) override;

		/** Prepares RollingFileAppender for use. */
		void activateOptions(helpers::Pool& pool ) override;

		/**
		   Implements the usual roll over behaviour.

		   <p>If <code>MaxBackupIndex</code> is positive, then files
		   {<code>File.1</code>, ..., <code>File.MaxBackupIndex -1</code>}
		   are renamed to {<code>File.2</code>, ...,
		   <code>File.MaxBackupIndex</code>}. Moreover, <code>File</code> is
		   renamed <code>File.1</code> and closed. A new <code>File</code> is
		   created to receive further log output.

		   <p>If <code>MaxBackupIndex</code> is equal to zero, then the
		   <code>File</code> is truncated with no backup files created.

		 */
		bool rollover(log4cxx::helpers::Pool& p);

	protected:

		/**
		 Actual writing occurs here.
		*/
		void subAppend(const spi::LoggingEventPtr& event, helpers::Pool& p) override;

		bool rolloverInternal(log4cxx::helpers::Pool& p);

	public:
		/**
		 * The policy that implements the scheme for rolling over a log file.
		 */
		RollingPolicyPtr getRollingPolicy() const;

		/**
		 * The policy that determine when to trigger a log file rollover.
		 */
		TriggeringPolicyPtr getTriggeringPolicy() const;

		/**
		 * Use \c policy as the scheme for rolling over log files.
		 *
		 * Where the \c policy also implements
		 * {@link TriggeringPolicy}, then \c policy
		 * will be used to determine when to trigger a log file rollover.
		 */
		void setRollingPolicy(const RollingPolicyPtr& policy);

		/**
		 * Use \c policy to determine when to trigger a log file rollover.
		 */
		void setTriggeringPolicy(const TriggeringPolicyPtr& policy);

	public:
		/**
		  * Close appender.  Waits for any asynchronous file compression actions to be completed.
		*/
		void close() override;

	protected:
		/**
		   Returns an OutputStreamWriter when passed an OutputStream.  The
		   encoding used will depend on the value of the
		   <code>encoding</code> property.  If the encoding value is
		   specified incorrectly the writer will be opened using the default
		   system encoding (an error message will be printed to the loglog.
		 @param os output stream, may not be null.
		 @return new writer.
		 */
		helpers::WriterPtr createWriter(helpers::OutputStreamPtr& os) override;

	public:
		/**
		 * Get byte length of current active log file.
		 * @return byte length of current active log file.
		 */
		size_t getFileLength() const;

		/**
		 * Increments estimated byte length of current active log file.
		 * @param increment additional bytes written to log file.
		 */
		void incrementFileLength(size_t increment);

};

LOG4CXX_PTR_DEF(RollingFileAppender);

}
}

#endif

