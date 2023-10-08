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

#include <log4cxx/helpers/syslogwriter.h>

#include "appenderskeleton_priv.h"

#include <log4cxx/private/log4cxx_private.h>

#if LOG4CXX_HAVE_SYSLOG
	#include <syslog.h>
#else
	/* facility codes */
	#define   LOG_KERN (0<<3)   /* kernel messages */
	#define   LOG_USER (1<<3)   /* random user-level messages */
	#define   LOG_MAIL (2<<3)   /* mail system */
	#define   LOG_DAEMON  (3<<3)   /* system daemons */
	#define   LOG_AUTH (4<<3)   /* security/authorization messages */
	#define   LOG_SYSLOG  (5<<3)   /* messages generated internally by syslogd */
	#define   LOG_LPR     (6<<3)   /* line printer subsystem */
	#define   LOG_NEWS (7<<3)   /* network news subsystem */
	#define   LOG_UUCP (8<<3)   /* UUCP subsystem */
	#define   LOG_CRON (9<<3)   /* clock daemon */
	#define   LOG_AUTHPRIV   (10<<3)  /* security/authorization messages (private) */
	#define   LOG_FTP     (11<<3)  /* ftp daemon */

	/* other codes through 15 reserved for system use */
	#define   LOG_LOCAL0  (16<<3)  /* reserved for local use */
	#define   LOG_LOCAL1  (17<<3)  /* reserved for local use */
	#define   LOG_LOCAL2  (18<<3)  /* reserved for local use */
	#define   LOG_LOCAL3  (19<<3)  /* reserved for local use */
	#define   LOG_LOCAL4  (20<<3)  /* reserved for local use */
	#define   LOG_LOCAL5  (21<<3)  /* reserved for local use */
	#define   LOG_LOCAL6  (22<<3)  /* reserved for local use */
	#define   LOG_LOCAL7  (23<<3)  /* reserved for local use */
#endif

namespace log4cxx
{
namespace net
{

struct SyslogAppender::SyslogAppenderPriv : public AppenderSkeleton::AppenderSkeletonPrivate
{
	SyslogAppenderPriv() :
		AppenderSkeletonPrivate(),
		syslogFacility(LOG_USER),
		facilityPrinting(false),
		maxMessageLength(1024)
	{

	}

	SyslogAppenderPriv(const LayoutPtr& layout, int syslogFacility) :
		AppenderSkeletonPrivate (layout),
		syslogFacility(syslogFacility),
		facilityPrinting(false),
		maxMessageLength(1024)
	{

	}

	SyslogAppenderPriv(const LayoutPtr& layout,
		const LogString& syslogHost, int syslogFacility) :
		AppenderSkeletonPrivate(layout),
		syslogFacility(syslogFacility),
		facilityPrinting(false),
		maxMessageLength(1024)
	{

	}

	int syslogFacility; // Have LOG_USER as default
	LogString facilityStr;
	bool facilityPrinting;
	std::unique_ptr<helpers::SyslogWriter> sw;
	LogString syslogHost;
	int syslogHostPort;
	int maxMessageLength;
};

}
}
