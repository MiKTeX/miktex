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

#if (defined(WIN32) || defined(_WIN32)) && !defined(_WIN32_WCE)

#include <apr_strings.h>

#include <log4cxx/nt/nteventlogappender.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/level.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/private/appenderskeleton_priv.h>

using namespace log4cxx;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;
using namespace log4cxx::nt;

#define priv static_cast<NTEventLogAppenderPrivate*>(m_priv.get())

struct NTEventLogAppender::NTEventLogAppenderPrivate : public AppenderSkeleton::AppenderSkeletonPrivate {
	NTEventLogAppenderPrivate() :
		hEventLog(nullptr),
		pCurrentUserSID(nullptr) {}

	NTEventLogAppenderPrivate( LayoutPtr layout ) :
		AppenderSkeletonPrivate(layout),
		hEventLog(nullptr),
		pCurrentUserSID(nullptr) {}

	// Data
	LogString server;
	LogString log;
	LogString source;
	HANDLE hEventLog;
	SID* pCurrentUserSID;
};

class CCtUserSIDHelper
{
	public:
		static bool FreeSid(SID* pSid)
		{
			return ::HeapFree(GetProcessHeap(), 0, (LPVOID)pSid) != 0;
		}

		static bool CopySid(SID * * ppDstSid, SID* pSrcSid)
		{
			bool bSuccess = false;

			DWORD dwLength = ::GetLengthSid(pSrcSid);
			*ppDstSid = (SID*) ::HeapAlloc(GetProcessHeap(),
					HEAP_ZERO_MEMORY, dwLength);

			if (::CopySid(dwLength, *ppDstSid, pSrcSid))
			{
				bSuccess = true;
			}
			else
			{
				FreeSid(*ppDstSid);
			}

			return bSuccess;
		}

		static bool GetCurrentUserSID(SID * * ppSid)
		{
			bool bSuccess = false;

			// Pseudohandle so don't need to close it
			HANDLE hProcess = ::GetCurrentProcess();
			HANDLE hToken = NULL;

			if (::OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
			{
				// Get the required size
				DWORD tusize = 0;
				GetTokenInformation(hToken, TokenUser, NULL, 0, &tusize);
				TOKEN_USER* ptu = (TOKEN_USER*)new BYTE[tusize];

				if (GetTokenInformation(hToken, TokenUser, (LPVOID)ptu, tusize, &tusize))
				{
					bSuccess = CopySid(ppSid, (SID*)ptu->User.Sid);
				}

				CloseHandle(hToken);
				delete [] ptu;
			}

			return bSuccess;
		}
};

IMPLEMENT_LOG4CXX_OBJECT(NTEventLogAppender)

NTEventLogAppender::NTEventLogAppender() :
	AppenderSkeleton(std::make_unique<NTEventLogAppenderPrivate>())
{
}

NTEventLogAppender::NTEventLogAppender(const LogString& server, const LogString& log, const LogString& source, const LayoutPtr& layout)
	:	AppenderSkeleton(std::make_unique<NTEventLogAppenderPrivate>(layout))
{
	Pool pool;
	priv->server = server;
	priv->log = log;
	priv->source = source;
	activateOptions(pool);
}

NTEventLogAppender::~NTEventLogAppender()
{
	finalize();
}


void NTEventLogAppender::close()
{
	if (priv->hEventLog != NULL)
	{
		::DeregisterEventSource(priv->hEventLog);
		priv->hEventLog = NULL;
	}

	if (priv->pCurrentUserSID != NULL)
	{
		CCtUserSIDHelper::FreeSid((::SID*) priv->pCurrentUserSID);
		priv->pCurrentUserSID = NULL;
	}
}

void NTEventLogAppender::setOption(const LogString& option, const LogString& value)
{
	if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("SERVER"), LOG4CXX_STR("server")))
	{
		priv->server = value;
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("LOG"), LOG4CXX_STR("log")))
	{
		priv->log = value;
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("SOURCE"), LOG4CXX_STR("source")))
	{
		priv->source = value;
	}
	else
	{
		AppenderSkeleton::setOption(option, value);
	}
}

void NTEventLogAppender::activateOptions(Pool&)
{
	if (priv->source.empty())
	{
		LogLog::warn(
			((LogString) LOG4CXX_STR("Source option not set for appender ["))
			+ this->m_priv->name + LOG4CXX_STR("]."));
		return;
	}

	if (priv->log.empty())
	{
		priv->log = LOG4CXX_STR("Application");
	}

	close();

	// current user security identifier
	CCtUserSIDHelper::GetCurrentUserSID((::SID**) &priv->pCurrentUserSID);

	addRegistryInfo();

	LOG4CXX_ENCODE_WCHAR(wsource, priv->source);
	LOG4CXX_ENCODE_WCHAR(wserver, priv->server);
	priv->hEventLog = ::RegisterEventSourceW(
			wserver.empty() ? NULL : wserver.c_str(),
			wsource.c_str());

	if (priv->hEventLog == NULL)
	{
		LogString msg(LOG4CXX_STR("Cannot register NT EventLog -- server: '"));
		msg.append(priv->server);
		msg.append(LOG4CXX_STR("' source: '"));
		msg.append(priv->source);
		LogLog::error(msg);
		LogLog::error(getErrorString(LOG4CXX_STR("RegisterEventSource")));
	}
}

void NTEventLogAppender::append(const LoggingEventPtr& event, Pool& p)
{
	if (priv->hEventLog == NULL)
	{
		LogLog::warn(LOG4CXX_STR("NT EventLog not opened."));
		return;
	}

	LogString oss;
	this->m_priv->layout->format(oss, event, p);
	wchar_t* msgs = Transcoder::wencode(oss, p);
	BOOL bSuccess = ::ReportEventW(
			priv->hEventLog,
			getEventType(event),
			getEventCategory(event),
			0x1000,
			priv->pCurrentUserSID,
			1,
			0,
			(LPCWSTR*) &msgs,
			NULL);

	if (!bSuccess)
	{
		LogLog::error(getErrorString(LOG4CXX_STR("ReportEvent")));
	}
}

/*
 * Add this source with appropriate configuration keys to the registry.
 */
void NTEventLogAppender::addRegistryInfo()
{
	DWORD disposition = 0;
	::HKEY hkey = 0;
	LogString subkey(LOG4CXX_STR("SYSTEM\\CurrentControlSet\\Services\\EventLog\\"));
	subkey.append(priv->log);
	subkey.append(1, (logchar) 0x5C /* '\\' */);
	subkey.append(priv->source);
	LOG4CXX_ENCODE_WCHAR(wsubkey, subkey);

	long stat = RegCreateKeyExW(HKEY_LOCAL_MACHINE, wsubkey.c_str(), 0, NULL,
			REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL,
			&hkey, &disposition);

	if (stat == ERROR_SUCCESS && disposition == REG_CREATED_NEW_KEY)
	{
		HMODULE hmodule = GetModuleHandleW(L"log4cxx");

		if (hmodule == NULL)
		{
			hmodule = GetModuleHandleW(0);
		}

		wchar_t modpath[_MAX_PATH];
		DWORD modlen = GetModuleFileNameW(hmodule, modpath, _MAX_PATH - 1);

		if (modlen > 0)
		{
			modpath[modlen] = 0;
			RegSetValueExW(hkey, L"EventMessageFile", 0, REG_SZ,
				(LPBYTE) modpath, (DWORD)(wcslen(modpath) * sizeof(wchar_t)));
			RegSetValueExW(hkey, L"CategoryMessageFile", 0, REG_SZ,
				(LPBYTE) modpath, (DWORD)(wcslen(modpath) * sizeof(wchar_t)));
			DWORD typesSupported = 7;
			DWORD categoryCount = 6;
			RegSetValueExW(hkey, L"TypesSupported", 0, REG_DWORD,
				(LPBYTE)&typesSupported, sizeof(DWORD));
			RegSetValueExW(hkey, L"CategoryCount", 0, REG_DWORD,
				(LPBYTE)&categoryCount, sizeof(DWORD));
		}
	}

	RegCloseKey(hkey);
	return;
}

WORD NTEventLogAppender::getEventType(const LoggingEventPtr& event)
{
	int priority = event->getLevel()->toInt();
	WORD type = EVENTLOG_SUCCESS;

	if (priority >= Level::INFO_INT)
	{
		type = EVENTLOG_INFORMATION_TYPE;

		if (priority >= Level::WARN_INT)
		{
			type = EVENTLOG_WARNING_TYPE;

			if (priority >= Level::ERROR_INT)
			{
				type = EVENTLOG_ERROR_TYPE;
			}
		}
	}

	return type;
}

WORD NTEventLogAppender::getEventCategory(const LoggingEventPtr& event)
{
	int priority = event->getLevel()->toInt();
	WORD category = 1;

	if (priority >= Level::DEBUG_INT)
	{
		category = 2;

		if (priority >= Level::INFO_INT)
		{
			category = 3;

			if (priority >= Level::WARN_INT)
			{
				category = 4;

				if (priority >= Level::ERROR_INT)
				{
					category = 5;

					if (priority >= Level::FATAL_INT)
					{
						category = 6;
					}
				}
			}
		}
	}

	return category;
}

LogString NTEventLogAppender::getErrorString(const LogString& function)
{
	Pool p;
	enum { MSGSIZE = 5000 };

	wchar_t* lpMsgBuf = (wchar_t*) p.palloc(MSGSIZE * sizeof(wchar_t));
	DWORD dw = GetLastError();

	FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		lpMsgBuf,
		MSGSIZE, NULL );

	LogString msg(function);
	msg.append(LOG4CXX_STR(" failed with error "));
	StringHelper::toString((size_t) dw, p, msg);
	msg.append(LOG4CXX_STR(": "));
	Transcoder::decode(lpMsgBuf, msg);

	return msg;
}

void NTEventLogAppender::setSource(const LogString& source)
{
	priv->source.assign(source);
}

const LogString& NTEventLogAppender::getSource() const
{
	return priv->source;
}

void NTEventLogAppender::setLog(const LogString& log)
{
	priv->log.assign(log);
}

const LogString& NTEventLogAppender::getLog() const
{
	return priv->log;
}

void NTEventLogAppender::setServer(const LogString& server)
{
	priv->server.assign(server);
}

const LogString& NTEventLogAppender::getServer() const
{
	return priv->server;
}

#endif // WIN32
