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

#include <log4cxx/net/syslogappender.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/datagramsocket.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/level.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/optionconverter.h>
#if !defined(LOG4CXX)
	#define LOG4CXX 1
#endif
#include <apr_strings.h>
#include <log4cxx/private/syslogappender_priv.h>

#define LOG_UNDEF -1

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::net;

IMPLEMENT_LOG4CXX_OBJECT(SyslogAppender)

#define _priv static_cast<SyslogAppenderPriv*>(m_priv.get())

SyslogAppender::SyslogAppender()
	: AppenderSkeleton (std::make_unique<SyslogAppenderPriv>())
{
	this->initSyslogFacilityStr();

}

SyslogAppender::SyslogAppender(const LayoutPtr& layout1,
	int syslogFacility1)
	: AppenderSkeleton (std::make_unique<SyslogAppenderPriv>(layout1, syslogFacility1))
{
	this->initSyslogFacilityStr();
}

SyslogAppender::SyslogAppender(const LayoutPtr& layout1,
	const LogString& syslogHost1, int syslogFacility1)
	: AppenderSkeleton (std::make_unique<SyslogAppenderPriv>(layout1, syslogHost1, syslogFacility1))
{
	this->initSyslogFacilityStr();
	setSyslogHost(syslogHost1);
}

SyslogAppender::~SyslogAppender()
{
	finalize();
}

/** Release any resources held by this SyslogAppender.*/
void SyslogAppender::close()
{
	_priv->closed = true;

	if (_priv->sw)
	{
		_priv->sw = nullptr;
	}
}

void SyslogAppender::initSyslogFacilityStr()
{
	_priv->facilityStr = getFacilityString(_priv->syslogFacility);

	if (_priv->facilityStr.empty())
	{
		Pool p;
		LogString msg(LOG4CXX_STR("\""));
		StringHelper::toString(_priv->syslogFacility, p, msg);
		msg.append(LOG4CXX_STR("\" is an unknown syslog facility. Defaulting to \"USER\"."));
		LogLog::error(msg);
		_priv->syslogFacility = LOG_USER;
		_priv->facilityStr = LOG4CXX_STR("user:");
	}
	else
	{
		_priv->facilityStr += LOG4CXX_STR(":");
	}
}

/**
Returns the specified syslog facility as a lower-case String,
e.g. "kern", "user", etc.
*/
LogString SyslogAppender::getFacilityString(
	int syslogFacility)
{
	switch (syslogFacility)
	{
		case LOG_KERN:
			return LOG4CXX_STR("kern");

		case LOG_USER:
			return LOG4CXX_STR("user");

		case LOG_MAIL:
			return LOG4CXX_STR("mail");

		case LOG_DAEMON:
			return LOG4CXX_STR("daemon");

		case LOG_AUTH:
			return LOG4CXX_STR("auth");

		case LOG_SYSLOG:
			return LOG4CXX_STR("syslog");

		case LOG_LPR:
			return LOG4CXX_STR("lpr");

		case LOG_NEWS:
			return LOG4CXX_STR("news");

		case LOG_UUCP:
			return LOG4CXX_STR("uucp");

		case LOG_CRON:
			return LOG4CXX_STR("cron");
#ifdef LOG_AUTHPRIV

		case LOG_AUTHPRIV:
			return LOG4CXX_STR("authpriv");
#endif
#ifdef LOG_FTP

		case LOG_FTP:
			return LOG4CXX_STR("ftp");
#endif

		case LOG_LOCAL0:
			return LOG4CXX_STR("local0");

		case LOG_LOCAL1:
			return LOG4CXX_STR("local1");

		case LOG_LOCAL2:
			return LOG4CXX_STR("local2");

		case LOG_LOCAL3:
			return LOG4CXX_STR("local3");

		case LOG_LOCAL4:
			return LOG4CXX_STR("local4");

		case LOG_LOCAL5:
			return LOG4CXX_STR("local5");

		case LOG_LOCAL6:
			return LOG4CXX_STR("local6");

		case LOG_LOCAL7:
			return LOG4CXX_STR("local7");

		default:
			return LogString();
	}
}

int SyslogAppender::getFacility(
	const LogString& s)
{
	if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("KERN"), LOG4CXX_STR("kern")))
	{
		return LOG_KERN;
	}
	else if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("USER"), LOG4CXX_STR("user")))
	{
		return LOG_USER;
	}
	else if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("MAIL"), LOG4CXX_STR("mail")))
	{
		return LOG_MAIL;
	}
	else if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("DAEMON"), LOG4CXX_STR("daemon")))
	{
		return LOG_DAEMON;
	}
	else if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("AUTH"), LOG4CXX_STR("auth")))
	{
		return LOG_AUTH;
	}
	else if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("SYSLOG"), LOG4CXX_STR("syslog")))
	{
		return LOG_SYSLOG;
	}
	else if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("LPR"), LOG4CXX_STR("lpr")))
	{
		return LOG_LPR;
	}
	else if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("NEWS"), LOG4CXX_STR("news")))
	{
		return LOG_NEWS;
	}
	else if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("UUCP"), LOG4CXX_STR("uucp")))
	{
		return LOG_UUCP;
	}
	else if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("CRON"), LOG4CXX_STR("cron")))
	{
		return LOG_CRON;
	}

#ifdef LOG_AUTHPRIV
	else if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("AUTHPRIV"), LOG4CXX_STR("authpriv")))
	{
		return LOG_AUTHPRIV;
	}

#endif
#ifdef LOG_FTP
	else if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("FTP"), LOG4CXX_STR("ftp")))
	{
		return LOG_FTP;
	}

#endif
	else if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("LOCAL0"), LOG4CXX_STR("local0")))
	{
		return LOG_LOCAL0;
	}
	else if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("LOCAL1"), LOG4CXX_STR("local1")))
	{
		return LOG_LOCAL1;
	}
	else if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("LOCAL2"), LOG4CXX_STR("local2")))
	{
		return LOG_LOCAL2;
	}
	else if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("LOCAL3"), LOG4CXX_STR("local3")))
	{
		return LOG_LOCAL3;
	}
	else if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("LOCAL4"), LOG4CXX_STR("local4")))
	{
		return LOG_LOCAL4;
	}
	else if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("LOCAL5"), LOG4CXX_STR("local5")))
	{
		return LOG_LOCAL5;
	}
	else if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("LOCAL6"), LOG4CXX_STR("local6")))
	{
		return LOG_LOCAL6;
	}
	else if (StringHelper::equalsIgnoreCase(s, LOG4CXX_STR("LOCAL7"), LOG4CXX_STR("local7")))
	{
		return LOG_LOCAL7;
	}
	else
	{
		return LOG_UNDEF;
	}
}

void SyslogAppender::append(const spi::LoggingEventPtr& event, Pool& p)
{
	if  (!isAsSevereAsThreshold(event->getLevel()))
	{
		return;
	}

	LogString msg;
	std::string encoded;
	_priv->layout->format(msg, event, p);

	Transcoder::encode(msg, encoded);

	// Split up the message if it is over maxMessageLength in size.
	// According to RFC 3164, the max message length is 1024, however
	// newer systems(such as syslog-ng) can go up to 8k in size for their
	// messages.  We will append (x/y) at the end of each message
	// to indicate how far through the message we are
	std::vector<LogString> packets;

	if ( msg.size() > _priv->maxMessageLength )
	{
		LogString::iterator start = msg.begin();

		while ( start != msg.end() )
		{
			LogString::iterator end = start + _priv->maxMessageLength - 12;

			if ( end > msg.end() )
			{
				end = msg.end();
			}

			LogString newMsg = LogString( start, end );
			packets.push_back( newMsg );
			start = end;
		}

		int current = 1;

		for ( std::vector<LogString>::iterator it = packets.begin();
			it != packets.end();
			it++, current++ )
		{
			char buf[12];
			apr_snprintf( buf, sizeof(buf), "(%d/%d)", current, (int)packets.size() );
			LOG4CXX_DECODE_CHAR(str, buf);
			it->append( str );
		}
	}
	else
	{
		packets.push_back( msg );
	}

	// On the local host, we can directly use the system function 'syslog'
	// if it is available
#if LOG4CXX_HAVE_SYSLOG

	if (_priv->sw == 0)
	{
		for ( std::vector<LogString>::iterator it = packets.begin();
			it != packets.end();
			it++ )
		{
			// use of "%s" to avoid a security hole
			::syslog(_priv->syslogFacility | event->getLevel()->getSyslogEquivalent(),
				"%s", it->c_str());
		}

		return;
	}

#endif

	// We must not attempt to append if sw is null.
	if (_priv->sw == 0)
	{
		_priv->errorHandler->error(LOG4CXX_STR("No syslog host is set for SyslogAppedender named \"") +
			_priv->name + LOG4CXX_STR("\"."));
		return;
	}

	for ( std::vector<LogString>::iterator it = packets.begin();
		it != packets.end();
		it++ )
	{
		LogString sbuf(1, 0x3C /* '<' */);
		StringHelper::toString((_priv->syslogFacility | event->getLevel()->getSyslogEquivalent()), p, sbuf);
		sbuf.append(1, (logchar) 0x3E /* '>' */);

		if (_priv->facilityPrinting)
		{
			sbuf.append(_priv->facilityStr);
		}

		sbuf.append(*it);
		_priv->sw->write(sbuf);
	}
}

void SyslogAppender::activateOptions(Pool&)
{
}

void SyslogAppender::setOption(const LogString& option, const LogString& value)
{
	if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("SYSLOGHOST"), LOG4CXX_STR("sysloghost")))
	{
		setSyslogHost(value);
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("FACILITY"), LOG4CXX_STR("facility")))
	{
		setFacility(value);
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("MAXMESSAGELENGTH"), LOG4CXX_STR("maxmessagelength")))
	{
		setMaxMessageLength(OptionConverter::toInt(value, 1024));
	}
	else
	{
		AppenderSkeleton::setOption(option, value);
	}
}

void SyslogAppender::setSyslogHost(const LogString& syslogHost1)
{
	if (_priv->sw != 0)
	{
		_priv->sw = nullptr;
	}

	LogString slHost = syslogHost1;
	int slHostPort = -1;

	LogString::size_type colonPos = 0;
	colonPos = slHost.rfind(':');

	if (colonPos != LogString::npos)
	{
		slHostPort = StringHelper::toInt(slHost.substr(colonPos + 1));
		// Erase the :port part of the host name
		slHost.erase( colonPos );
	}

	// On the local host, we can directly use the system function 'syslog'
	// if it is available (cf. append)
#if LOG4CXX_HAVE_SYSLOG

	if (syslogHost1 != LOG4CXX_STR("localhost") && syslogHost1 != LOG4CXX_STR("127.0.0.1")
		&& !syslogHost1.empty())
#endif
	{
		if (slHostPort >= 0)
		{
			_priv->sw = std::make_unique<SyslogWriter>(slHost, slHostPort);
		}
		else
		{
			_priv->sw = std::make_unique<SyslogWriter>(slHost);
		}
	}

	_priv->syslogHost = slHost;
	_priv->syslogHostPort = slHostPort;
}


void SyslogAppender::setFacility(const LogString& facilityName)
{
	if (facilityName.empty())
	{
		return;
	}

	_priv->syslogFacility = getFacility(facilityName);

	if (_priv->syslogFacility == LOG_UNDEF)
	{
		LogLog::error(LOG4CXX_STR("[") + facilityName +
			LOG4CXX_STR("] is an unknown syslog facility. Defaulting to [USER]."));
		_priv->syslogFacility = LOG_USER;
	}

	this->initSyslogFacilityStr();
}

const LogString& SyslogAppender::getSyslogHost() const
{
	return _priv->syslogHost;
}

LogString SyslogAppender::getFacility() const
{
	return getFacilityString(_priv->syslogFacility);
}

void SyslogAppender::setFacilityPrinting(bool facilityPrinting1)
{
	_priv->facilityPrinting = facilityPrinting1;
}

bool SyslogAppender::getFacilityPrinting() const
{
	return _priv->facilityPrinting;
}

void SyslogAppender::setMaxMessageLength(int maxMessageLength1)
{
	_priv->maxMessageLength = maxMessageLength1;
}

int SyslogAppender::getMaxMessageLength() const
{
	return _priv->maxMessageLength;
}

