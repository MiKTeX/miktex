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

#ifndef LOG4CXX_DB_DB_APPENDER_H
#define LOG4CXX_DB_DB_APPENDER_H

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

/**
 * The DBAppender lets you log messages to a database.  This utilizes the APR's database support in order
 * to provide a single way of logging to multiple databases, not just ODBC as the ODBCAppender does.
 *
 * The following SQL script is an example of how you may make a table that stores log messages:
 *
 * ~~~{.sql}
 * CREATE TABLE logs (
 *  logger VARCHAR(200),
 *  time DATETIME,
 *  level CHAR(5),
 *  file VARCHAR(200),
 *  line_number INT,
 *  message VARCHAR(1000)
 * );
 * ~~~
 *
 * Once you have defined the table, you must define the order in which the columns are formatted when they are inserted
 * along with the insert statement.
 *
 * Using APR, the following insert statement can be used to insert log statements: <code>INSERT INTO logs (logger, time, level, file, line_number, message) VALUES (%s, %pDa, %s, %s, %d, %s)</code>
 * The values to insert must be formatted appropriately and in the correct order.  In order to do this, the parameter <code>ColumnMapping</code> must be specified as many times
 * as there are columns to insert.
 *
 * The following table shows the conversion specifiers.  These are effectively the same as the conversion patterns provided by PatternLayout but with more descriptive names:
 * |Conversion Specifier|Effect|
 * |---|---|
 * |logger|The name of the logger(e.g. com.foo.bar)|
 * |class|The class that the log message was in|
 * |time|The time of the log message|
 * |shortfilename|The short filename where the log message is from|
 * |fullfilename|The full filename where the log mesasge is from|
 * |location|The location of the log message|
 * |line|The line where the log message appears|
 * |message|The log message|
 * |method|The method where the message was logged|
 * |level|The level of the log message|
 * |thread|The thread where this message was logged|
 * |threadname|The name of the thread that logged the message|
 * |ndc|The NDC(nested diagnostic context) of the log|
 *
 * Other parameters that are important:
 * |Parameter name|Usage|
 * |---|---|
 * |DriverName|The name of the database driver to load(ex: odbc, sqlite3)|
 * |DriverParams|A string of parameters to pass to the driver.  This is database-specific|
 * |DatabaseName|The name of the database to use when connecting to the server|
 *
 * The following code shows how you might connect to an ODBC data source and insert the log messages:
 * ~~~{.xml}
 * <appender name="SqlDBAppender" class="DBAppender">
 *   <param name="drivername" value="odbc"/>
 *   <param name="DriverParams" value="DATASOURCE=MariaDB-server"/>
 *   <param name="DatabaseName" value="example_database"/>
 *   <param name="sql" value="INSERT INTO logs (logger, time, level, file, line_number, message) VALUES (%s, %pDa, %s, %s, %d, %s)" />
 *   <param name="ColumnMapping" value="logger"/>
 *   <param name="ColumnMapping" value="time"/>
 *   <param name="ColumnMapping" value="level"/>
 *   <param name="ColumnMapping" value="shortfilename"/>
 *   <param name="ColumnMapping" value="line"/>
 *   <param name="ColumnMapping" value="message"/>
 * </appender>
 * ~~~
 *
 * A similar configuration can be used for SQLite:
 * ~~~{.xml}
 * <appender name="SqlDBAppender" class="DBAppender">
 *   <param name="drivername" value="sqlite3"/>
 *   <param name="DriverParams" value="/path/to/database.db"/>
 *   <param name="sql" value="INSERT INTO logs (logger, time, level, file, line_number, message) VALUES (%s, %pDa, %s, %s, %d, %s)" />
 *   <param name="ColumnMapping" value="logger"/>
 *   <param name="ColumnMapping" value="time"/>
 *   <param name="ColumnMapping" value="level"/>
 *   <param name="ColumnMapping" value="shortfilename"/>
 *   <param name="ColumnMapping" value="line"/>
 *   <param name="ColumnMapping" value="message"/>
 * </appender>
 * ~~~
 */
class LOG4CXX_EXPORT DBAppender : public AppenderSkeleton
{
        public:
                DECLARE_LOG4CXX_OBJECT(DBAppender)
                BEGIN_LOG4CXX_CAST_MAP()
                LOG4CXX_CAST_ENTRY(DBAppender)
                LOG4CXX_CAST_ENTRY_CHAIN(AppenderSkeleton)
                END_LOG4CXX_CAST_MAP()

                DBAppender();
                virtual ~DBAppender();

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

                void close() override;

                /**
                * DBAppender does not require a layout.
                * */
                bool requiresLayout() const override
                {
                        return false;
                }

                /**
                * Set pre-formated statement eg: insert into LogTable (msg) values (?)
                */
                void setSql(const LogString& s);

                /**
                * Returns pre-formated statement eg: insert into LogTable (msg) values (?)
                */
                const LogString& getSql() const;

        private:
                DBAppender(const DBAppender&);
                DBAppender& operator=(const DBAppender&);

        protected:
                struct DBAppenderPriv;
}; // class DBAppender

LOG4CXX_PTR_DEF(DBAppender);

} // namespace db
} // namespace log4cxx

#endif // LOG4CXX_DB_DB_APPENDER_H
