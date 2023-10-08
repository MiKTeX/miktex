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
#include <log4cxx/db/odbcappender.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/pattern/mdcpatternconverter.h>
#include <apr_strings.h>
#include <apr_time.h>
#include <cmath> // std::pow

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

#if !defined(LOG4CXX)
	#define LOG4CXX 1
#endif
#include <log4cxx/private/log4cxx_private.h>
#if LOG4CXX_HAVE_ODBC
	#if defined(WIN32) || defined(_WIN32)
		#include <windows.h>
	#endif
	#include <sqlext.h>
#else
	typedef void* SQLHSTMT;
#endif
#include <log4cxx/private/odbcappender_priv.h>
#if defined(min)
	#undef min
#endif
#include <cstring>
#include <algorithm>


using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::db;
using namespace log4cxx::spi;
using namespace log4cxx::pattern;

SQLException::SQLException(short fHandleType,
	void* hInput, const char* prolog,
	log4cxx::helpers::Pool& p)
	: Exception(formatMessage(fHandleType, hInput, prolog, p))
{
}


SQLException::SQLException(const char* msg)
	: Exception(msg)
{
}

SQLException::SQLException(const SQLException& src)
	: Exception(src)
{
}

const char* SQLException::formatMessage(short fHandleType,
	void* hInput, const char* prolog, log4cxx::helpers::Pool& p)
{
	std::string strReturn(prolog);
	strReturn.append(" - ");
#if LOG4CXX_HAVE_ODBC
	SQLCHAR       SqlState[6];
	SQLCHAR       Msg[SQL_MAX_MESSAGE_LENGTH];
	SQLINTEGER    NativeError;
	SQLSMALLINT   i;
	SQLSMALLINT   MsgLen;
	SQLRETURN     rc2;

	// Get the status records.
	i = 1;

	while ((rc2 = SQLGetDiagRecA(fHandleType, hInput, i, SqlState, &NativeError,
					Msg, sizeof(Msg), &MsgLen)) != SQL_NO_DATA)
	{
		strReturn.append((char*) Msg);
		i++;
	}

#else
	strReturn.append("log4cxx built without ODBC support");
#endif

	return apr_pstrdup((apr_pool_t*) p.getAPRPool(), strReturn.c_str());
}


IMPLEMENT_LOG4CXX_OBJECT(ODBCAppender)

#define _priv static_cast<ODBCAppenderPriv*>(m_priv.get())

ODBCAppender::ODBCAppender()
	: AppenderSkeleton (std::make_unique<ODBCAppenderPriv>())
{
}

ODBCAppender::~ODBCAppender()
{
	finalize();
}

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
		RULES_PUT("mdc", MDCPatternConverter);
		RULES_PUT("ndc", NDCPatternConverter);
	}
	return specs;
}

void ODBCAppender::setOption(const LogString& option, const LogString& value)
{
	if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("BUFFERSIZE"), LOG4CXX_STR("buffersize")))
	{
		setBufferSize((size_t)OptionConverter::toInt(value, 1));
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("PASSWORD"), LOG4CXX_STR("password")))
	{
		setPassword(value);
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("SQL"), LOG4CXX_STR("sql")))
	{
		setSql(value);
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("URL"), LOG4CXX_STR("url"))
		|| StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("DSN"), LOG4CXX_STR("dsn"))
		|| StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("CONNECTIONSTRING"), LOG4CXX_STR("connectionstring"))  )
	{
		setURL(value);
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("USER"), LOG4CXX_STR("user")))
	{
		setUser(value);
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("COLUMNMAPPING"), LOG4CXX_STR("columnmapping")))
	{
		_priv->mappedName.push_back(value);
	}
	else
	{
		AppenderSkeleton::setOption(option, value);
	}
}

//* Does ODBCAppender require a layout?

bool ODBCAppender::requiresLayout() const
{
	return false;
}

void ODBCAppender::activateOptions(log4cxx::helpers::Pool&)
{
#if !LOG4CXX_HAVE_ODBC
	LogLog::error(LOG4CXX_STR("Can not activate ODBCAppender unless compiled with ODBC support."));
#else
	if (_priv->mappedName.empty())
	{
		LogLog::error(LOG4CXX_STR("ODBCAppender column mappings not defined, logging events will not be inserted"));
	}
	auto specs = getFormatSpecifiers();
	for (auto& name : _priv->mappedName)
	{
		auto lowerName = StringHelper::toLowerCase(name);
		auto pItem = specs.find(lowerName);
		if (specs.end() == pItem)
		{
			if (lowerName.size() < 5
			 || lowerName.substr(0, 4) != LOG4CXX_STR("mdc{"))
				LogLog::error(name + LOG4CXX_STR(" is not a supported ColumnMapping value"));
			else // A single MDC entry
			{
				auto index = lowerName.find(0x7D /* '}' */, 4);
				auto len = (lowerName.npos == index ? lowerName.size() : index) - 4;
				ODBCAppenderPriv::DataBinding paramData{ 0, 0, 0, 0, 0 };
				paramData.converter = std::make_shared<MDCPatternConverter>(lowerName.substr(4, len));
				_priv->parameterValue.push_back(paramData);
			}
		}
		else
		{
			ODBCAppenderPriv::DataBinding paramData{ 0, 0, 0, 0, 0 };
			std::vector<LogString> options;
			if (LOG4CXX_STR("time") == pItem->first)
				options.push_back(LOG4CXX_STR("yyyy-MM-dd HH:mm:ss.SSSSSS"));
			paramData.converter = log4cxx::cast<LoggingEventPatternConverter>((pItem->second)(options));
			_priv->parameterValue.push_back(paramData);
		}
	}
#endif
}


void ODBCAppender::append(const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p)
{
#if LOG4CXX_HAVE_ODBC
	_priv->buffer.push_back(event);

	if (_priv->buffer.size() >= _priv->bufferSize)
	{
		flushBuffer(p);
	}

#endif
}

LogString ODBCAppender::getLogStatement(const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p) const
{
    return LogString();
}

void ODBCAppender::execute(const LogString& sql, log4cxx::helpers::Pool& p)
{
}

/* The default behavior holds a single connection open until the appender
is closed (typically when garbage collected).*/
void ODBCAppender::closeConnection(ODBCAppender::SQLHDBC /* con */)
{
}

ODBCAppender::SQLHDBC ODBCAppender::getConnection(log4cxx::helpers::Pool& p)
{
#if LOG4CXX_HAVE_ODBC
	SQLRETURN ret;

	if (_priv->env == SQL_NULL_HENV)
	{
		ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_priv->env);

		if (ret < 0)
		{
			SQLException ex(SQL_HANDLE_ENV, _priv->env, "Failed to allocate SQL handle", p);
			_priv->env = SQL_NULL_HENV;
			throw ex;
		}

		ret = SQLSetEnvAttr(_priv->env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER);

		if (ret < 0)
		{
			SQLException ex(SQL_HANDLE_ENV, _priv->env, "Failed to set odbc version", p);
			SQLFreeHandle(SQL_HANDLE_ENV, _priv->env);
			_priv->env = SQL_NULL_HENV;
			throw ex;
		}
	}

	if (_priv->connection == SQL_NULL_HDBC)
	{
		ret = SQLAllocHandle(SQL_HANDLE_DBC, _priv->env, &_priv->connection);

		if (ret < 0)
		{
			SQLException ex(SQL_HANDLE_DBC, _priv->connection, "Failed to allocate sql handle", p);
			_priv->connection = SQL_NULL_HDBC;
			throw ex;
		}

#if LOG4CXX_LOGCHAR_IS_WCHAR
		SQLWCHAR *wUser = nullptr, *wPwd = nullptr;
		if (!_priv->databaseUser.empty())
			wUser = (SQLWCHAR*)_priv->databaseUser.c_str();
		if (!_priv->databasePassword.empty())
			wPwd = (SQLWCHAR*)_priv->databasePassword.c_str();
		ret = SQLConnectW(_priv->connection
			, (SQLWCHAR*)_priv->databaseURL.c_str(), SQL_NTS
			, wUser, SQL_NTS
			, wPwd, SQL_NTS
			);
#elif LOG4CXX_LOGCHAR_IS_UTF8
		SQLCHAR *wUser = nullptr, *wPwd = nullptr;
		if (!_priv->databaseUser.empty())
			wUser = (SQLCHAR*)_priv->databaseUser.c_str();
		if (!_priv->databasePassword.empty())
			wPwd = (SQLCHAR*)_priv->databasePassword.c_str();
		ret = SQLConnectA(_priv->connection
			, (SQLCHAR*)_priv->databaseURL.c_str(), SQL_NTS
			, wUser, SQL_NTS
			, wPwd, SQL_NTS
			);
#else
		SQLWCHAR* wURL, *wUser = nullptr, *wPwd = nullptr;
		encode(&wURL, _priv->databaseURL, p);
		if (!_priv->databaseUser.empty())
			encode(&wUser, _priv->databaseUser, p);
		if (!_priv->databasePassword.empty())
			encode(&wPwd, _priv->databasePassword, p);

		ret = SQLConnectW( _priv->connection
			, wURL, SQL_NTS
			, wUser, SQL_NTS
			, wPwd, SQL_NTS
			);
#endif

		if (ret < 0)
		{
			SQLException ex(SQL_HANDLE_DBC, _priv->connection, "Failed to connect to database", p);
			SQLFreeHandle(SQL_HANDLE_DBC, _priv->connection);
			_priv->connection = SQL_NULL_HDBC;
			throw ex;
		}
	}

	return _priv->connection;
#else
	return 0;
#endif
}

void ODBCAppender::close()
{
	if (_priv->closed)
	{
		return;
	}

	Pool p;

	try
	{
		flushBuffer(p);
	}
	catch (SQLException& e)
	{
		_priv->errorHandler->error(LOG4CXX_STR("Error closing connection"),
			e, ErrorCode::GENERIC_FAILURE);
	}

#if LOG4CXX_HAVE_ODBC

	if (_priv->connection != SQL_NULL_HDBC)
	{
		SQLDisconnect(_priv->connection);
		SQLFreeHandle(SQL_HANDLE_DBC, _priv->connection);
	}

	if (_priv->env != SQL_NULL_HENV)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, _priv->env);
	}

#endif
	_priv->closed = true;
}

#if LOG4CXX_HAVE_ODBC
void ODBCAppender::ODBCAppenderPriv::setPreparedStatement(SQLHDBC con, Pool& p)
{
	auto ret = SQLAllocHandle( SQL_HANDLE_STMT, con, &this->preparedStatement);
	if (ret < 0)
	{
		throw SQLException( SQL_HANDLE_DBC, con, "Failed to allocate statement handle.", p);
	}

#if LOG4CXX_LOGCHAR_IS_WCHAR
	ret = SQLPrepareW(this->preparedStatement, (SQLWCHAR*)this->sqlStatement.c_str(), SQL_NTS);
#elif LOG4CXX_LOGCHAR_IS_UTF8
	ret = SQLPrepareA(this->preparedStatement, (SQLCHAR*)this->sqlStatement.c_str(), SQL_NTS);
#else
	SQLWCHAR* wsql;
	encode(&wsql, this->sqlStatement, p);
	ret = SQLPrepareW(this->preparedStatement, wsql, SQL_NTS);
#endif
	if (ret < 0)
	{
		throw SQLException(SQL_HANDLE_STMT, this->preparedStatement, "Failed to prepare sql statement.", p);
	}

	int parameterNumber = 0;
	for (auto& item : this->parameterValue)
	{
		++parameterNumber;
		SQLSMALLINT  targetType;
		SQLULEN      targetMaxCharCount;
		SQLSMALLINT  decimalDigits;
		SQLSMALLINT  nullable;
		auto ret = SQLDescribeParam
			( this->preparedStatement
			, parameterNumber
			, &targetType
			, &targetMaxCharCount
			, &decimalDigits
			, &nullable
			);
		if (ret < 0)
		{
			throw SQLException(SQL_HANDLE_STMT, this->preparedStatement, "Failed to describe parameter", p);
		}
		if (SQL_CHAR == targetType || SQL_VARCHAR == targetType || SQL_LONGVARCHAR == targetType)
		{
			item.paramType = SQL_C_CHAR;
			item.paramMaxCharCount = targetMaxCharCount;
			item.paramValueSize = (SQLINTEGER)(item.paramMaxCharCount) * sizeof(char) + sizeof(char);
			item.paramValue = (SQLPOINTER)p.palloc(item.paramValueSize + sizeof(char));
		}
		else if (SQL_WCHAR == targetType || SQL_WVARCHAR == targetType || SQL_WLONGVARCHAR == targetType)
		{
			item.paramType = SQL_C_WCHAR;
			item.paramMaxCharCount = targetMaxCharCount;
			item.paramValueSize = (SQLINTEGER)(targetMaxCharCount) * sizeof(wchar_t) + sizeof(wchar_t);
			item.paramValue = (SQLPOINTER)p.palloc(item.paramValueSize + sizeof(wchar_t));
		}
		else if (SQL_TYPE_TIMESTAMP == targetType || SQL_TYPE_DATE == targetType || SQL_TYPE_TIME == targetType
			|| SQL_DATETIME == targetType)
		{
			item.paramType = SQL_C_TYPE_TIMESTAMP;
			item.paramMaxCharCount = (0 <= decimalDigits) ? decimalDigits : 6;
			item.paramValueSize = sizeof(SQL_TIMESTAMP_STRUCT);
			item.paramValue = (SQLPOINTER)p.palloc(item.paramValueSize);
		}
		else
		{
			if (SQL_INTEGER != targetType)
			{
				LogString msg(LOG4CXX_STR("Unexpected targetType ("));
				helpers::StringHelper::toString(targetType, p, msg);
				msg += LOG4CXX_STR(") at parameter ");
				helpers::StringHelper::toString(parameterNumber, p, msg);
				msg += LOG4CXX_STR(" while preparing SQL");
				LogLog::warn(msg);
			}
			item.paramMaxCharCount = 30;
#if LOG4CXX_LOGCHAR_IS_UTF8
			item.paramType = SQL_C_CHAR;
			item.paramValueSize = (SQLINTEGER)(item.paramMaxCharCount) * sizeof(char);
			item.paramValue = (SQLPOINTER)p.palloc(item.paramValueSize + sizeof(char));
#else
			item.paramType = SQL_C_WCHAR;
			item.paramValueSize = (SQLINTEGER)(item.paramMaxCharCount) * sizeof(wchar_t);
			item.paramValue = (SQLPOINTER)p.palloc(item.paramValueSize + sizeof(wchar_t));
#endif
		}
		item.strLen_or_Ind = SQL_NTS;
		ret = SQLBindParameter
			( this->preparedStatement
			, parameterNumber
			, SQL_PARAM_INPUT
			, item.paramType  // ValueType
			, targetType
			, targetMaxCharCount
			, decimalDigits
			, item.paramValue
			, item.paramValueSize
			, &item.strLen_or_Ind
			);
		if (ret < 0)
		{
			throw SQLException(SQL_HANDLE_STMT, this->preparedStatement, "Failed to bind parameter", p);
		}
	}
}

void ODBCAppender::ODBCAppenderPriv::setParameterValues(const spi::LoggingEventPtr& event, Pool& p)
{
	for (auto& item : this->parameterValue)
	{
		if (!item.paramValue || item.paramValueSize <= 0)
			;
		else if (SQL_C_WCHAR == item.paramType)
		{
			LogString sbuf;
			item.converter->format(event, sbuf, p);
#if LOG4CXX_LOGCHAR_IS_WCHAR_T
			std::wstring& tmp = sbuf;
#else
			std::wstring tmp;
			Transcoder::encode(sbuf, tmp);
#endif
			auto dst = (wchar_t*)item.paramValue;
			auto charCount = std::min(size_t(item.paramMaxCharCount), tmp.size());
			auto copySize = std::min(size_t(item.paramValueSize - 1), charCount * sizeof(wchar_t));
			std::memcpy(dst, tmp.data(), copySize);
			dst[copySize / sizeof(wchar_t)] = 0;
		}
		else if (SQL_C_CHAR == item.paramType)
		{
			LogString sbuf;
			item.converter->format(event, sbuf, p);
#if LOG4CXX_LOGCHAR_IS_UTF8
			std::string& tmp = sbuf;
#else
			std::string tmp;
			Transcoder::encode(sbuf, tmp);
#endif
			auto dst = (char*)item.paramValue;
			auto sz = std::min(size_t(item.paramMaxCharCount), tmp.size());
			auto copySize = std::min(size_t(item.paramValueSize - 1), sz * sizeof(char));
			std::memcpy(dst, tmp.data(), copySize);
			dst[copySize] = 0;
		}
		else if (SQL_C_TYPE_TIMESTAMP == item.paramType)
		{
			apr_time_exp_t exploded;
			apr_status_t stat = this->timeZone->explode(&exploded, event->getTimeStamp());
			if (stat == APR_SUCCESS)
			{
				auto dst = (SQL_TIMESTAMP_STRUCT*)item.paramValue;
				dst->year = 1900 + exploded.tm_year;
				dst->month = 1 + exploded.tm_mon;
				dst->day = exploded.tm_mday;
				dst->hour = exploded.tm_hour;
				dst->minute = exploded.tm_min;
				dst->second = exploded.tm_sec;
				// Prevent '[ODBC SQL Server Driver]Datetime field overflow' by rounding to the target field precision
				int roundingExponent = 6 - (int)item.paramMaxCharCount;
				if (0 < roundingExponent)
				{
					int roundingDivisor = (int)std::pow(10, roundingExponent);
					dst->fraction = 1000 * roundingDivisor * ((exploded.tm_usec + roundingDivisor / 2) / roundingDivisor);
				}
				else
					dst->fraction = 1000 * exploded.tm_usec;
			}
		}
	}
}
#endif

void ODBCAppender::flushBuffer(Pool& p)
{
	for (auto& logEvent : _priv->buffer)
	{
		if (_priv->parameterValue.empty())
			_priv->errorHandler->error(LOG4CXX_STR("ODBCAppender column mappings not defined"));
#if LOG4CXX_HAVE_ODBC
		else try
		{
			if (0 == _priv->preparedStatement)
				_priv->setPreparedStatement(getConnection(p), p);
			_priv->setParameterValues(logEvent, p);
			auto ret = SQLExecute(_priv->preparedStatement);
			if (ret < 0)
			{
				throw SQLException(SQL_HANDLE_STMT, _priv->preparedStatement, "Failed to execute prepared statement", p);
			}
		}
		catch (SQLException& e)
		{
			_priv->errorHandler->error(LOG4CXX_STR("Failed to execute sql"), e,
				ErrorCode::FLUSH_FAILURE);
		}
#endif
	}

	// clear the buffer of reported events
	_priv->buffer.clear();
}

void ODBCAppender::setSql(const LogString& s)
{
    _priv->sqlStatement = s;
}

#if LOG4CXX_WCHAR_T_API || LOG4CXX_LOGCHAR_IS_WCHAR_T || defined(WIN32) || defined(_WIN32)
void ODBCAppender::encode(wchar_t** dest, const LogString& src, Pool& p)
{
	*dest = Transcoder::wencode(src, p);
}
#endif

void ODBCAppender::encode(unsigned short** dest,
	const LogString& src, Pool& p)
{
	//  worst case double number of characters from UTF-8 or wchar_t
	*dest = (unsigned short*)
		p.palloc((src.size() + 1) * 2 * sizeof(unsigned short));
	unsigned short* current = *dest;

	for (LogString::const_iterator i = src.begin();
		i != src.end();)
	{
		unsigned int sv = Transcoder::decode(src, i);

		if (sv < 0x10000)
		{
			*current++ = (unsigned short) sv;
		}
		else
		{
			unsigned char u = (unsigned char) (sv >> 16);
			unsigned char w = (unsigned char) (u - 1);
			unsigned short hs = (0xD800 + ((w & 0xF) << 6) + ((sv & 0xFFFF) >> 10));
			unsigned short ls = (0xDC00 + (sv & 0x3FF));
			*current++ = (unsigned short) hs;
			*current++ = (unsigned short) ls;
		}
	}

	*current = 0;
}

const LogString& ODBCAppender::getSql() const
{
	return _priv->sqlStatement;
}

void ODBCAppender::setUser(const LogString& user)
{
	_priv->databaseUser = user;
}

void ODBCAppender::setURL(const LogString& url)
{
	_priv->databaseURL = url;
}

void ODBCAppender::setPassword(const LogString& password)
{
	_priv->databasePassword = password;
}

void ODBCAppender::setBufferSize(size_t newBufferSize)
{
	_priv->bufferSize = newBufferSize;
}

const LogString& ODBCAppender::getUser() const
{
	return _priv->databaseUser;
}

const LogString& ODBCAppender::getURL() const
{
	return _priv->databaseURL;
}

const LogString& ODBCAppender::getPassword() const
{
	return _priv->databasePassword;
}

size_t ODBCAppender::getBufferSize() const
{
	return _priv->bufferSize;
}

