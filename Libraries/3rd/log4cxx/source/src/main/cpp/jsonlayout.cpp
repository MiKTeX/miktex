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

#include <log4cxx/logstring.h>
#include <log4cxx/jsonlayout.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/level.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/iso8601dateformat.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/transcoder.h>

#include <apr_time.h>
#include <apr_strings.h>
#include <string.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;

IMPLEMENT_LOG4CXX_OBJECT(JSONLayout)


JSONLayout::JSONLayout() :
	locationInfo(false),
	prettyPrint(false),
	dateFormat(),
	ppIndentL1(LOG4CXX_STR("  ")),
	ppIndentL2(LOG4CXX_STR("    "))
{
}


void JSONLayout::setOption(const LogString& option, const LogString& value)
{
	if (StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("LOCATIONINFO"), LOG4CXX_STR("locationinfo")))
	{
		setLocationInfo(OptionConverter::toBoolean(value, false));
	}

	if (StringHelper::equalsIgnoreCase(option,
			LOG4CXX_STR("PRETTYPRINT"), LOG4CXX_STR("prettyprint")))
	{
		setPrettyPrint(OptionConverter::toBoolean(value, false));
	}
}
void JSONLayout::format(LogString& output,
	const spi::LoggingEventPtr& event,
	Pool& p) const
{
	output.append(LOG4CXX_STR("{"));
	output.append(prettyPrint ? LOG4CXX_EOL : LOG4CXX_STR(" "));

	if (prettyPrint)
	{
		output.append(ppIndentL1);
	}

	appendQuotedEscapedString(output, LOG4CXX_STR("timestamp"));
	output.append(LOG4CXX_STR(": "));
	LogString timestamp;
	dateFormat.format(timestamp, event->getTimeStamp(), p);
	appendQuotedEscapedString(output, timestamp);
	output.append(LOG4CXX_STR(","));
	output.append(prettyPrint ? LOG4CXX_EOL : LOG4CXX_STR(" "));

	if (prettyPrint)
	{
		output.append(ppIndentL1);
	}

	appendQuotedEscapedString(output, LOG4CXX_STR("level"));
	output.append(LOG4CXX_STR(": "));
	LogString level;
	event->getLevel()->toString(level);
	appendQuotedEscapedString(output, level);
	output.append(LOG4CXX_STR(","));
	output.append(prettyPrint ? LOG4CXX_EOL : LOG4CXX_STR(" "));

	if (prettyPrint)
	{
		output.append(ppIndentL1);
	}

	appendQuotedEscapedString(output, LOG4CXX_STR("logger"));
	output.append(LOG4CXX_STR(": "));
	appendQuotedEscapedString(output, event->getLoggerName());
	output.append(LOG4CXX_STR(","));
	output.append(prettyPrint ? LOG4CXX_EOL : LOG4CXX_STR(" "));

	if (prettyPrint)
	{
		output.append(ppIndentL1);
	}

	appendQuotedEscapedString(output, LOG4CXX_STR("message"));
	output.append(LOG4CXX_STR(": "));
	appendQuotedEscapedString(output, event->getMessage());

	appendSerializedMDC(output, event);
	appendSerializedNDC(output, event);

	if (locationInfo)
	{
		output.append(LOG4CXX_STR(","));
		output.append(prettyPrint ? LOG4CXX_EOL : LOG4CXX_STR(" "));
		appendSerializedLocationInfo(output, event, p);
	}

	output.append(prettyPrint ? LOG4CXX_EOL : LOG4CXX_STR(" "));
	output.append(LOG4CXX_STR("}"));
	output.append(LOG4CXX_EOL);
}

void JSONLayout::appendQuotedEscapedString(LogString& buf,
	const LogString& input) const
{
	/* add leading quote */
	buf.push_back(0x22);

	logchar specialChars[] =
	{
		0x08,   /* \b backspace         */
		0x09,   /* \t tab               */
		0x0a,   /* \n newline           */
		0x0c,   /* \f form feed         */
		0x0d,   /* \r carriage return   */
		0x22,   /* \" double quote      */
		0x5c    /* \\ backslash         */
	};

	size_t start = 0;
	size_t found = input.find_first_of(specialChars, start);

	while (found != LogString::npos)
	{
		if (found > start)
		{
			buf.append(input, start, found - start);
		}

		switch (input[found])
		{
			case 0x08:
				/* \b backspace */
				buf.push_back(0x5c);
				buf.push_back('b');
				break;

			case 0x09:
				/* \t tab */
				buf.push_back(0x5c);
				buf.push_back('t');
				break;

			case 0x0a:
				/* \n newline */
				buf.push_back(0x5c);
				buf.push_back('n');
				break;

			case 0x0c:
				/* \f form feed */
				buf.push_back(0x5c);
				buf.push_back('f');
				break;

			case 0x0d:
				/* \r carriage return */
				buf.push_back(0x5c);
				buf.push_back('r');
				break;

			case 0x22:
				/* \" double quote */
				buf.push_back(0x5c);
				buf.push_back(0x22);
				break;

			case 0x5c:
				/* \\ backslash */
				buf.push_back(0x5c);
				buf.push_back(0x5c);
				break;

			default:
				buf.push_back(input[found]);
				break;
		}

		start = found + 1;

		if (found < input.size())
		{
			found = input.find_first_of(specialChars, start);
		}
		else
		{
			found = LogString::npos;
		}
	}

	if (start < input.size())
	{
		buf.append(input, start, input.size() - start);
	}

	/* add trailing quote */
	buf.push_back(0x22);
}

void JSONLayout::appendSerializedMDC(LogString& buf,
	const LoggingEventPtr& event) const
{
	LoggingEvent::KeySet keys = event->getMDCKeySet();

	if (keys.empty())
	{
		return;
	}

	buf.append(LOG4CXX_STR(","));
	buf.append(prettyPrint ? LOG4CXX_EOL : LOG4CXX_STR(" "));

	if (prettyPrint)
	{
		buf.append(ppIndentL1);
	}

	appendQuotedEscapedString(buf, LOG4CXX_STR("context_map"));
	buf.append(LOG4CXX_STR(": {"));
	buf.append(prettyPrint ? LOG4CXX_EOL : LOG4CXX_STR(" "));

	for (LoggingEvent::KeySet::iterator it = keys.begin();
		it != keys.end(); ++it)
	{
		if (prettyPrint)
		{
			buf.append(ppIndentL2);
		}

		appendQuotedEscapedString(buf, *it);
		buf.append(LOG4CXX_STR(": "));
		LogString value;
		event->getMDC(*it, value);
		appendQuotedEscapedString(buf, value);

		/* if this isn't the last k:v pair, we need a comma */
		if (it + 1 != keys.end())
		{
			buf.append(LOG4CXX_STR(","));
			buf.append(prettyPrint ? LOG4CXX_EOL : LOG4CXX_STR(" "));
		}
		else
		{
			buf.append(prettyPrint ? LOG4CXX_EOL : LOG4CXX_STR(" "));
		}
	}

	if (prettyPrint)
	{
		buf.append(ppIndentL1);
	}

	buf.append(LOG4CXX_STR("}"));
}

void JSONLayout::appendSerializedNDC(LogString& buf,
	const LoggingEventPtr& event) const
{
	LogString ndcVal;

	if (!event->getNDC(ndcVal))
	{
		return;
	}

	buf.append(LOG4CXX_STR(","));
	buf.append(prettyPrint ? LOG4CXX_EOL : LOG4CXX_STR(" "));

	if (prettyPrint)
	{
		buf.append(ppIndentL1);
	}

	appendQuotedEscapedString(buf, LOG4CXX_STR("context_stack"));
	buf.append(LOG4CXX_STR(": ["));
	buf.append(prettyPrint ? LOG4CXX_EOL : LOG4CXX_STR(" "));

	if (prettyPrint)
	{
		buf.append(ppIndentL2);
	}

	appendQuotedEscapedString(buf, ndcVal);
	buf.append(prettyPrint ? LOG4CXX_EOL : LOG4CXX_STR(" "));

	if (prettyPrint)
	{
		buf.append(ppIndentL1);
	}

	buf.append(LOG4CXX_STR("]"));
}

void JSONLayout::appendSerializedLocationInfo(LogString& buf,
	const LoggingEventPtr& event, Pool& p) const
{
	if (prettyPrint)
	{
		buf.append(ppIndentL1);
	}

	appendQuotedEscapedString(buf, LOG4CXX_STR("location_info"));
	buf.append(LOG4CXX_STR(": {"));
	buf.append(prettyPrint ? LOG4CXX_EOL : LOG4CXX_STR(" "));
	const LocationInfo& locInfo = event->getLocationInformation();

	if (prettyPrint)
	{
		buf.append(ppIndentL2);
	}

	appendQuotedEscapedString(buf, LOG4CXX_STR("file"));
	buf.append(LOG4CXX_STR(": "));
	LOG4CXX_DECODE_CHAR(fileName, locInfo.getFileName());
	appendQuotedEscapedString(buf, fileName);
	buf.append(LOG4CXX_STR(","));
	buf.append(prettyPrint ? LOG4CXX_EOL : LOG4CXX_STR(" "));

	if (prettyPrint)
	{
		buf.append(ppIndentL2);
	}

	appendQuotedEscapedString(buf, LOG4CXX_STR("line"));
	buf.append(LOG4CXX_STR(": "));
	LogString lineNumber;
	StringHelper::toString(locInfo.getLineNumber(), p, lineNumber);
	appendQuotedEscapedString(buf, lineNumber);
	buf.append(LOG4CXX_STR(","));
	buf.append(prettyPrint ? LOG4CXX_EOL : LOG4CXX_STR(" "));

	if (prettyPrint)
	{
		buf.append(ppIndentL2);
	}

	appendQuotedEscapedString(buf, LOG4CXX_STR("class"));
	buf.append(LOG4CXX_STR(": "));
	LOG4CXX_DECODE_CHAR(className, locInfo.getClassName());
	appendQuotedEscapedString(buf, className);
	buf.append(LOG4CXX_STR(","));
	buf.append(prettyPrint ? LOG4CXX_EOL : LOG4CXX_STR(" "));

	if (prettyPrint)
	{
		buf.append(ppIndentL2);
	}

	appendQuotedEscapedString(buf, LOG4CXX_STR("method"));
	buf.append(LOG4CXX_STR(": "));
	LOG4CXX_DECODE_CHAR(methodName, locInfo.getMethodName());
	appendQuotedEscapedString(buf, methodName);
	buf.append(prettyPrint ? LOG4CXX_EOL : LOG4CXX_STR(" "));

	if (prettyPrint)
	{
		buf.append(ppIndentL1);
	}

	buf.append(LOG4CXX_STR("}"));
}

