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
#include <log4cxx/db/dbappender.h>
#include <log4cxx/appenderskeleton.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/pattern/loggerpatternconverter.h>
#include <log4cxx/pattern/classnamepatternconverter.h>
#include <log4cxx/pattern/datepatternconverter.h>
#include <log4cxx/pattern/filelocationpatternconverter.h>
#include <log4cxx/pattern/fulllocationpatternconverter.h>
#include <log4cxx/pattern/shortfilelocationpatternconverter.h>
#include <log4cxx/pattern/linelocationpatternconverter.h>
#include <log4cxx/pattern/messagepatternconverter.h>
#include <log4cxx/pattern/methodlocationpatternconverter.h>
#include <log4cxx/pattern/levelpatternconverter.h>
#include <log4cxx/pattern/threadpatternconverter.h>
#include <log4cxx/pattern/threadusernamepatternconverter.h>
#include <log4cxx/pattern/ndcpatternconverter.h>
#include <log4cxx/private/appenderskeleton_priv.h>
#include <apr_dbd.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::db;
using namespace log4cxx::spi;
using namespace log4cxx::pattern;

IMPLEMENT_LOG4CXX_OBJECT(DBAppender)

#define _priv static_cast<DBAppenderPriv*>(m_priv.get())

struct DBAppender::DBAppenderPriv : public AppenderSkeleton::AppenderSkeletonPrivate
{
    DBAppenderPriv() :
        AppenderSkeletonPrivate()
        {}

    apr_dbd_driver_t* m_driver = nullptr;
    apr_dbd_t* m_databaseHandle = nullptr;
    apr_dbd_prepared_t* preparedStmt = nullptr;
    std::vector<LogString> mappedName;
    std::string driverName;
    std::string driverParams;
    std::string databaseName;
    std::string sqlStatement;
    Pool m_pool;
    std::vector<pattern::LoggingEventPatternConverterPtr> converters;
};

#define RULES_PUT(spec, cls) \
    specs.insert(PatternMap::value_type(LogString(LOG4CXX_STR(spec)), cls ::newInstance))

static PatternMap getFormatSpecifiers()
{
    PatternMap specs;
    if (specs.empty())
    {
        RULES_PUT("logger", LoggerPatternConverter);
        RULES_PUT("class", ClassNamePatternConverter);
        RULES_PUT("time", DatePatternConverter);
        RULES_PUT("shortfilename", ShortFileLocationPatternConverter);
        RULES_PUT("fullfilename", FileLocationPatternConverter);
        RULES_PUT("location", FullLocationPatternConverter);
        RULES_PUT("line", LineLocationPatternConverter);
        RULES_PUT("message", MessagePatternConverter);
        RULES_PUT("method", MethodLocationPatternConverter);
        RULES_PUT("level", LevelPatternConverter);
        RULES_PUT("thread", ThreadPatternConverter);
        RULES_PUT("threadname", ThreadUsernamePatternConverter);
        RULES_PUT("ndc", NDCPatternConverter);
    }
    return specs;
}

DBAppender::DBAppender()
    : AppenderSkeleton (std::make_unique<DBAppenderPriv>())
{
}

DBAppender::~DBAppender()
{
    close();
}

void DBAppender::close(){
    if(_priv->m_driver && _priv->m_databaseHandle){
        apr_dbd_close(_priv->m_driver, _priv->m_databaseHandle);
    }
    _priv->m_driver = nullptr;
    _priv->m_databaseHandle = nullptr;
}

void DBAppender::setOption(const LogString& option, const LogString& value){
    if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("COLUMNMAPPING"), LOG4CXX_STR("columnmapping")))
    {
        _priv->mappedName.push_back(value);
    }
    else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("DRIVERNAME"), LOG4CXX_STR("drivername")))
    {
        Transcoder::encodeUTF8(value, _priv->driverName);
    }
    else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("DRIVERPARAMS"), LOG4CXX_STR("driverparams")))
    {
        Transcoder::encodeUTF8(value, _priv->driverParams);
    }
    else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("DATABASENAME"), LOG4CXX_STR("databasename")))
    {
        Transcoder::encodeUTF8(value, _priv->databaseName);
    }
    else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("SQL"), LOG4CXX_STR("sql")))
    {
        Transcoder::encodeUTF8(value, _priv->sqlStatement);
    }
    else
    {
        AppenderSkeleton::setOption(option, value);
    }
}

void DBAppender::activateOptions(helpers::Pool& p){
    apr_status_t stat = apr_dbd_get_driver(_priv->m_pool.getAPRPool(),
                                           _priv->driverName.c_str(),
                                           const_cast<const apr_dbd_driver_t**>(&_priv->m_driver));

    if(stat != APR_SUCCESS){
        LogString errMsg = LOG4CXX_STR("Unable to get driver named ");
        LOG4CXX_DECODE_CHAR(driverName, _priv->driverName);
        errMsg.append(driverName);
        LogLog::error(errMsg);
        _priv->errorHandler->error(errMsg);
        return;
    }

    stat = apr_dbd_open(_priv->m_driver,
                        _priv->m_pool.getAPRPool(),
                        _priv->driverParams.c_str(),
                        &_priv->m_databaseHandle);
    if(stat != APR_SUCCESS){
        LogLog::error(LOG4CXX_STR("Unable to open database"));
        _priv->errorHandler->error(LOG4CXX_STR("Unable to open database"));
        return;
    }

    if(!_priv->databaseName.empty()){
        apr_dbd_set_dbname(_priv->m_driver,
                           _priv->m_pool.getAPRPool(),
                           _priv->m_databaseHandle,
                           _priv->databaseName.c_str());
    }

    stat = apr_dbd_prepare(_priv->m_driver,
                           _priv->m_pool.getAPRPool(),
                           _priv->m_databaseHandle,
                           _priv->sqlStatement.c_str(),
                           "log_insert",
                           &_priv->preparedStmt);
    if(stat != APR_SUCCESS){
        LogString error = LOG4CXX_STR("Unable to prepare statement: ");
        std::string dbdErr(apr_dbd_error(_priv->m_driver, _priv->m_databaseHandle, stat));
        LOG4CXX_DECODE_CHAR(dbdErrLS, dbdErr);
        error.append(dbdErrLS);
        LogLog::error(error);
        _priv->errorHandler->error(error);
        return;
    }

    auto specs = getFormatSpecifiers();
    for (auto& name : _priv->mappedName)
    {
        auto pItem = specs.find(StringHelper::toLowerCase(name));
        if (specs.end() == pItem)
            LogLog::error(name + LOG4CXX_STR(" is not a supported ColumnMapping value"));
        else
        {
            std::vector<LogString> options;
            if (LOG4CXX_STR("time") == pItem->first)
                options.push_back(LOG4CXX_STR("yyyy-MM-ddTHH:mm:ss.SSS"));
            pattern::LoggingEventPatternConverterPtr converter = log4cxx::cast<LoggingEventPatternConverter>((pItem->second)(options));
            _priv->converters.push_back(converter);
        }
    }
}

void DBAppender::append(const spi::LoggingEventPtr& event, helpers::Pool& p){
	std::vector<std::string> ls_args;
    std::vector<const char*> args;
    int stat;
    int num_rows;

    if(_priv->m_driver == nullptr ||
            _priv->m_databaseHandle == nullptr ||
            _priv->preparedStmt == nullptr){
        _priv->errorHandler->error(LOG4CXX_STR("DBAppender not initialized properly: logging not available"));
        return;
    }

    for(auto& converter : _priv->converters){
        LogString str_data;
        converter->format(event, str_data, p);
		LOG4CXX_ENCODE_CHAR(new_str_data, str_data);
		ls_args.push_back(new_str_data);
    }

	for(std::string& str : ls_args){
        args.push_back(str.data());
    }
    args.push_back(nullptr);

    stat = apr_dbd_pquery(_priv->m_driver,
                          _priv->m_pool.getAPRPool(),
                          _priv->m_databaseHandle,
                          &num_rows,
                          _priv->preparedStmt,
                          args.size(),
                          args.data());
    if(stat != APR_SUCCESS){
        LogString error = LOG4CXX_STR("Unable to insert: ");
		LOG4CXX_DECODE_CHAR(local_error, apr_dbd_error(_priv->m_driver, _priv->m_databaseHandle, stat));
		error.append(local_error);
        LogLog::error(error);
        _priv->errorHandler->error(error);
    }
}
