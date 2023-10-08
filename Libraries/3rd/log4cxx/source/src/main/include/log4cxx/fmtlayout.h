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

#ifndef LOG4CXX_FMT_LAYOUT_H
#define LOG4CXX_FMT_LAYOUT_H

#if defined(_MSC_VER)
	#pragma warning ( push )
	#pragma warning ( disable: 4231 4251 4275 4786 )
#endif


#include <log4cxx/layout.h>

namespace log4cxx
{
/**
 * The FMTLayout class uses libfmt to layout messages.  This is an alternative to the PatternLayout class.
 * Most of the standard PatternLayout arguments are also accepted as arguments, so that you can easily
 * convert a PatternLayout to a FMTLayout.  For example, given the following PatternLayout:
 *
 * <pre>%c %-5p - %m%n</pre>
 * which outputs something like:
 * <pre>root INFO  - Hello there!</pre>
 *
 * The equivalent FMTLayout can be written as:
 * <pre>{c} {p:<5} - {m}{n}</pre>
 * Or more verbosely as:
 * <pre>{logger} {level:<5} - {message}{newline}</pre>
 *
 * All replacements are done using the named arguments feature of {fmt}.
 *
 * <p>The recognized conversion strings are:</p>
 *
 * <table border="1" cellpadding="8">
 *  <tr>
 *      <th align="center"><strong>Conversion string</strong></th>
 *      <th align="center"><strong>Effect</strong></th>
 *  </tr>
 *  <tr>
 *      <td align="center">
 *        <p><strong>c</strong></p>
 *        <p><strong>logger</strong></p>
 *      </td>
 *      <td>
 *          Used to output the logger of the logging event.  Unlike PatternConverter,
 *          this does not take a parameter to shorten the name of the logger.
 *      </td>
 *  </tr>
 *  <tr>
 *      <td align="center">
 *          <p><strong>C</strong></p>
 *          <p><strong>class</strong></p>
 *      </td>
 *      <td>
 *          Used to output the class of the issuer of the logging event if the compiler
 *          used supports a macro to retrieve the method of the currently compiled line and
 *          if the LOG4CXX_TRACE-like macros are used to issue a logging request. In this
 *          case the macro LOG4CXX_* is expanded at compile time to generate location info
 *          of the logging event and adds the method name, besides file and line, if
 *          available. In most cases the provided method contains the classname and can
 *          therefore be retrieved form the location info as needed.
 *          <p>
 *              Currently supported compilers are those from Microsoft, GNU-C and Borland.
 *          </p>
 *      </td>
 *  </tr>
 *  <tr>
 *      <td align="center">
 *          <p><strong>f</strong></p>
 *          <p><strong>shortfilename</strong></p>
 *      </td>
 *      <td>
 *          Used to output the short file name where the logging request was issued.
 *      </td>
 *  </tr>
 *  <tr>
 *      <td align="center">
 *          <p><strong>F</strong></p>
 *          <p><strong>filename</strong></p>
 *      </td>
 *      <td>
 *          Used to output the file name where the logging request was issued.
 *      </td>
 *  </tr>
 *  <tr>
 *      <td align="center">
 *          <p><strong>l</strong></p>
 *          <p><strong>location</strong></p>
 *      </td>
 *      <td>
 *          Used to output location information of the caller which generated the logging
 *          event.
 *      </td>
 *  </tr>
 *  <tr>
 *      <td align="center">
 *          <p><strong>L</strong></p>
 *          <p><strong>line</strong></p>
 *      </td>
 *      <td>
 *          Used to output the line number from where the logging request was issued.
 *      </td>
 *  </tr>
 *  <tr>
 *      <td align="center">
 *          <p><strong>m</strong></p>
 *          <p><strong>message</strong></p>
 *      </td>
 *      <td>
 *          Used to output the application supplied message associated with the logging
 *          event.
 *      </td>
 *  </tr>
 *  <tr>
 *      <td align="center">
 *          <strong>M</strong>
 *          <p><strong>method</strong></p>
 *      </td>
 *      <td>
 *          Used to output the method of the issuer of the logging event if the compiler
 *          used supports a macro to retrieve the method of the currently compiled line
 *          and if the LOG4CXX_TRACE-like macros are used to issue a logging request. In
 *          this case the macro LOG4CXX_* is expanded at compile time to generate location
 *          info of the logging event and adds the method name, besides file and line, if
 *          available. In most cases the provided method contains the classname which is
 *          ignored in every attempt to retrieve the method from the location info.
 *          <p>
 *              Currently supported compilers are those from Microsoft, GNU-C and Borland.
 *          </p>
 *      </td>
 *  </tr>
 *  <tr>
 *      <td align="center">
 *          <p><strong>n</strong></p>
 *          <p><strong>newline</strong></p>
 *      </td>
 *      <td>
 *          Outputs the platform dependent line separator character or characters.
 *          <p>
 *              This conversion character offers practically the same performance as using
 *              non-portable line separator strings such as "\n", or "\r\n". Thus, it is the
 *              preferred way of specifying a line separator.
 *          </p>
 *      </td>
 *  </tr>
 *  <tr>
 *      <td align="center">
 *          <p><strong>p</strong></p>
 *          <p><strong>level</strong></p>
 *      </td>
 *      <td>Used to output the level of the logging event.</td>
 *  </tr>
 *  <tr>
 *      <td align="center">
 *          <p><strong>r</strong></p>
 *      </td>
 *      <td>
 *          Used to output the number of milliseconds elapsed since the start of the
 *          application until the creation of the logging event.
 *      </td>
 *  </tr>
 *  <tr>
 *      <td align="center">
 *          <p><strong>t</strong></p>
 *          <p><strong>thread</strong></p>
 *      </td>
 *      <td>Used to output the ID of the thread that generated the logging event.</td>
 *  </tr>
 *  <tr>
 *      <td align="center">
 *          <p><strong>T</strong></p>
 *          <p><strong>threadname</strong></p>
 *      </td>
 *      <td>Used to output the name of the thread that generated the logging event.  May not be available on all platforms.</td>
 *  </tr>
 *  <tr>
 *      <td align="center">
 *          <p><strong>x</strong></p>
 *          <p><strong>ndc</strong></p>
 *      </td>
 *      <td>
 *          Used to output the NDC (nested diagnostic context) associated with the thread that
 *          generated the logging event.
 *      </td>
 *  </tr>
 *  <tr>
 *      <td align="center">
 *          <p><strong>mdc[1-6]</strong></p>
 *      </td>
 *      <td>
 *          Used to output the MDC (mapped diagnostic context) associated with the thread that
 *          generated the logging event. The keys must be specified in the configuration for
 *          the layout.  A maximum of 6 MDC keys are available to be output(mdc1, mdc2, ..., mdc6)
 *      </td>
 *  </tr>
 * </table>
 */
class LOG4CXX_EXPORT FMTLayout : public Layout
{
    LOG4CXX_DECLARE_PRIVATE_MEMBER_PTR(FMTLayoutPrivate, m_priv)

	public:
		DECLARE_LOG4CXX_OBJECT(FMTLayout)
		BEGIN_LOG4CXX_CAST_MAP()
		LOG4CXX_CAST_ENTRY(FMTLayout)
		LOG4CXX_CAST_ENTRY_CHAIN(Layout)
		END_LOG4CXX_CAST_MAP()

		FMTLayout();

		FMTLayout(const LogString& pattern);

		~FMTLayout();


		/**
		 * Set the <strong>ConversionPattern</strong> option. This is the string which
		 * controls formatting and consists of a mix of literal content and
		 * conversion specifiers.
		 */
		void setConversionPattern(const LogString& conversionPattern);

		/**
		 * Returns the value of the <strong>ConversionPattern</strong> option.
		 */
		LogString getConversionPattern() const;

		/**
		Returns the log statement in a format consisting of the
		<code>level</code>, followed by " - " and then the
		<code>message</code>. For example, <pre> INFO - "A message"
		</pre>

		@return A byte array in SimpleLayout format.
		*/
		void format(LogString& output,
			const spi::LoggingEventPtr& event,
			helpers::Pool& pool) const override;

		bool ignoresThrowable() const override
		{
			return true;
		}

		void activateOptions(helpers::Pool& /* p */) override;
		void setOption(const LogString& /* option */,
		        const LogString& /* value */) override;
};
LOG4CXX_PTR_DEF(FMTLayout);
}  // namespace log4cxx


#if defined(_MSC_VER)
	#pragma warning ( pop )
#endif

#endif //LOG4CXX_FMT_LAYOUT_H
