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
#include <log4cxx/pattern/propertiespatternconverter.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/spi/location/locationinfo.h>
#include <log4cxx/private/patternconverter_priv.h>

#include <iterator>

using namespace log4cxx;
using namespace log4cxx::pattern;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;

#define priv static_cast<PropertiesPatternConverterPrivate*>(m_priv.get())

struct PropertiesPatternConverter::PropertiesPatternConverterPrivate : public PatternConverterPrivate
{
	PropertiesPatternConverterPrivate( const LogString& name, const LogString& style, const LogString& propertyName ) :
		PatternConverterPrivate( name, style ),
		option(propertyName) {}

	/**
	 * Name of property to output.
	 */
	const LogString option;
};

IMPLEMENT_LOG4CXX_OBJECT(PropertiesPatternConverter)

PropertiesPatternConverter::PropertiesPatternConverter(const LogString& name1,
	const LogString& propertyName) :
	LoggingEventPatternConverter(
		std::make_unique<PropertiesPatternConverterPrivate>(name1, LOG4CXX_STR("property"), propertyName))
{
}

PatternConverterPtr PropertiesPatternConverter::newInstance(
	const std::vector<LogString>& options)
{
	if (options.size() == 0)
	{
		static PatternConverterPtr def = std::make_shared<PropertiesPatternConverter>(
				LOG4CXX_STR("Properties"), LOG4CXX_STR(""));
		return def;
	}

	LogString converterName(LOG4CXX_STR("Property{"));
	converterName.append(options[0]);
	converterName.append(LOG4CXX_STR("}"));
	return std::make_shared<PropertiesPatternConverter>(converterName, options[0]);
}

void PropertiesPatternConverter::format(
	const LoggingEventPtr& event,
	LogString& toAppendTo,
	Pool& /* p */) const
{
	if (priv->option.length() == 0)
	{
		toAppendTo.append(1, (logchar) 0x7B /* '{' */);

		LoggingEvent::KeySet keySet(event->getMDCKeySet());

		for (LoggingEvent::KeySet::const_iterator iter = keySet.begin();
			iter != keySet.end();
			iter++)
		{
			toAppendTo.append(1, (logchar) 0x7B /* '{' */);
			toAppendTo.append(*iter);
			toAppendTo.append(1, (logchar) 0x2C /* ',' */);
			event->getMDC(*iter, toAppendTo);
			toAppendTo.append(1, (logchar) 0x7D /* '}' */);
		}

		toAppendTo.append(1, (logchar) 0x7D /* '}' */);

	}
	else
	{
		event->getMDC(priv->option, toAppendTo);
	}
}

