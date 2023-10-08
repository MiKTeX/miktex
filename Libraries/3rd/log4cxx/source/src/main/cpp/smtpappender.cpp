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
#include <log4cxx/net/smtpappender.h>
#include <log4cxx/level.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/private/string_c11.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/stringtokenizer.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/loader.h>
#if !defined(LOG4CXX)
	#define LOG4CXX 1
#endif
#include <log4cxx/private/log4cxx_private.h>
#include <log4cxx/private/appenderskeleton_priv.h>


#include <apr_strings.h>
#include <vector>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::net;
using namespace log4cxx::spi;

#if LOG4CXX_HAVE_LIBESMTP
	#include <auth-client.h>
	#include <libesmtp.h>
#endif

namespace log4cxx
{
namespace net
{
//
//   The following two classes implement an C++ SMTP wrapper over libesmtp.
//   The same signatures could be implemented over different SMTP implementations
//   or libesmtp could be combined with libgmime to enable support for non-ASCII
//   content.

#if LOG4CXX_HAVE_LIBESMTP
/**
 *   SMTP Session.
 */
class SMTPSession
{
	public:
		/**
		*   Create new instance.
		*/
		SMTPSession(const LogString& smtpHost,
			int smtpPort,
			const LogString& smtpUsername,
			const LogString& smtpPassword,
			Pool& p) : session(0), authctx(0),
			user(toAscii(smtpUsername, p)),
			pwd(toAscii(smtpPassword, p))
		{
			auth_client_init();
			session = smtp_create_session();

			if (session == 0)
			{
				throw Exception("Could not initialize session.");
			}

			std::string host(toAscii(smtpHost, p));
			host.append(1, ':');
			host.append(p.itoa(smtpPort));
			smtp_set_server(session, host.c_str());

			authctx = auth_create_context();
			auth_set_mechanism_flags(authctx, AUTH_PLUGIN_PLAIN, 0);
			auth_set_interact_cb(authctx, authinteract, (void*) this);

			if (*user || *pwd)
			{
				smtp_auth_set_context(session, authctx);
			}
		}

		~SMTPSession()
		{
			smtp_destroy_session(session);
			auth_destroy_context(authctx);
		}

		void send(Pool& p)
		{
			int status = smtp_start_session(session);

			if (!status)
			{
				size_t bufSize = 128;
				char* buf = p.pstralloc(bufSize);
				smtp_strerror(smtp_errno(), buf, bufSize);
				throw Exception(buf);
			}
		}

		operator smtp_session_t()
		{
			return session;
		}

		static char* toAscii(const LogString& str, Pool& p)
		{
			char* buf = p.pstralloc(str.length() + 1);
			char* current = buf;

			for (LogString::const_iterator iter = str.begin();
				iter != str.end();
				iter++)
			{
				unsigned int c = *iter;

				if (c > 0x7F)
				{
					c = '?';
				}

				*current++ = c;
			}

			*current = 0;
			return buf;
		}

	private:
		SMTPSession(SMTPSession&);
		SMTPSession& operator=(SMTPSession&);
		smtp_session_t session;
		auth_context_t authctx;
		char* user;
		char* pwd;

		/**
		 *   This method is called if the SMTP server requests authentication.
		 */
		static int authinteract(auth_client_request_t request, char** result, int fields,
			void* arg)
		{
			SMTPSession* pThis = (SMTPSession*) arg;

			for (int i = 0; i < fields; i++)
			{
				int flag = request[i].flags & 0x07;

				if (flag == AUTH_USER)
				{
					result[i] = pThis->user;
				}
				else if (flag == AUTH_PASS)
				{
					result[i] = pThis->pwd;
				}
			}

			return 1;
		}


};

/**
 *  A message in an SMTP session.
 */
class SMTPMessage
{
	public:
		SMTPMessage(SMTPSession& session,
			const LogString& from,
			const LogString& to,
			const LogString& cc,
			const LogString& bcc,
			const LogString& subject,
			const LogString msg, Pool& p)
		{
			message = smtp_add_message(session);
			current_len = msg.length();
			body = current = toMessage(msg, p);
			messagecbState = 0;
			smtp_set_reverse_path(message, toAscii(from, p));
			addRecipients(to, "To", p);
			addRecipients(cc, "Cc", p);
			addRecipients(bcc, "Bcc", p);

			if (!subject.empty())
			{
				smtp_set_header(message, "Subject", toAscii(subject, p));
			}

			smtp_set_messagecb(message, messagecb, this);
		}
		~SMTPMessage()
		{
		}

	private:
		SMTPMessage(const SMTPMessage&);
		SMTPMessage& operator=(const SMTPMessage&);
		smtp_message_t message;
		const char* body;
		const char* current;
		size_t current_len;
		int messagecbState;
		void addRecipients(const LogString& addresses, const char* field, Pool& p)
		{
			if (!addresses.empty())
			{
				char* str = p.pstrdup(toAscii(addresses, p));;
				smtp_set_header(message, field, NULL, str);
				char* last;

				for (char* next = apr_strtok(str, ",", &last);
					next;
					next = apr_strtok(NULL, ",", &last))
				{
					smtp_add_recipient(message, next);
				}
			}
		}
		static const char* toAscii(const LogString& str, Pool& p)
		{
			return SMTPSession::toAscii(str, p);
		}

		/**
		 *   Message bodies can only contain US-ASCII characters and
		 *   CR and LFs can only occur together.
		 */
		static const char* toMessage(const LogString& str, Pool& p)
		{
			//
			//    count the number of carriage returns and line feeds
			//
			int feedCount = 0;

			for (size_t pos = str.find_first_of(LOG4CXX_STR("\n\r"));
				pos != LogString::npos;
				pos = str.find_first_of(LOG4CXX_STR("\n\r"), ++pos))
			{
				feedCount++;
			}

			//
			//   allocate sufficient space for the modified message
			char* retval = p.pstralloc(str.length() + feedCount + 1);
			char* current = retval;
			char* startOfLine = current;

			//
			//    iterator through message
			//
			for (LogString::const_iterator iter = str.begin();
				iter != str.end();
				iter++)
			{
				unsigned int c = *iter;

				//
				//   replace non-ASCII characters with '?'
				//
				if (c > 0x7F)
				{
					*current++ = 0x3F; // '?'
				}
				else if (c == 0x0A || c == 0x0D)
				{
					//
					//   replace any stray CR or LF with CRLF
					//      reset start of line
					*current++ = 0x0D;
					*current++ = 0x0A;
					startOfLine = current;
					LogString::const_iterator next = iter + 1;

					if (next != str.end() && (*next == 0x0A || *next == 0x0D))
					{
						iter++;
					}
				}
				else
				{
					//
					//    truncate any lines to 1000 characters (including CRLF)
					//       as required by RFC.
					if (current < startOfLine + 998)
					{
						*current++ = (char) c;
					}
				}
			}

			*current = 0;
			return retval;
		}

		/**
		 *  Callback for message.
		 */
		static const char* messagecb(void** ctx, int* len, void* arg)
		{
			*ctx = 0;
			const char* retval = 0;
			SMTPMessage* pThis = (SMTPMessage*) arg;

			//   rewind message
			if (len == NULL)
			{
				pThis->current = pThis->body;
			}
			else
			{
				// we are asked for headers, but we don't have any
				if ((pThis->messagecbState)++ == 0)
				{
					return NULL;
				}

				if (pThis->current)
				{
					*len = strnlen_s(pThis->current, pThis->current_len);
				}

				retval = pThis->current;
				pThis->current = 0;
			}

			return retval;
		}

};
#endif

class LOG4CXX_EXPORT DefaultEvaluator :
	public virtual spi::TriggeringEventEvaluator,
	public virtual helpers::Object
{
	public:
		DECLARE_LOG4CXX_OBJECT(DefaultEvaluator)
		BEGIN_LOG4CXX_CAST_MAP()
		LOG4CXX_CAST_ENTRY(DefaultEvaluator)
		LOG4CXX_CAST_ENTRY(spi::TriggeringEventEvaluator)
		END_LOG4CXX_CAST_MAP()

		DefaultEvaluator();

		/**
		Is this <code>event</code> the e-mail triggering event?
		<p>This method returns <code>true</code>, if the event level
		has ERROR level or higher. Otherwise it returns
		<code>false</code>.
		*/
		bool isTriggeringEvent(const spi::LoggingEventPtr& event) override;
	private:
		DefaultEvaluator(const DefaultEvaluator&);
		DefaultEvaluator& operator=(const DefaultEvaluator&);
}; // class DefaultEvaluator

}
}

IMPLEMENT_LOG4CXX_OBJECT(DefaultEvaluator)
IMPLEMENT_LOG4CXX_OBJECT(SMTPAppender)

struct SMTPAppender::SMTPPriv : public AppenderSkeletonPrivate
{
	SMTPPriv() :
		AppenderSkeletonPrivate(),
		smtpPort(25),
		bufferSize(512),
		locationInfo(false),
		cb(bufferSize),
		evaluator(new DefaultEvaluator()) {}

	SMTPPriv(spi::TriggeringEventEvaluatorPtr evaluator) :
		AppenderSkeletonPrivate(),
		smtpPort(25),
		bufferSize(512),
		locationInfo(false),
		cb(bufferSize),
		evaluator(evaluator) {}

	LogString to;
	LogString cc;
	LogString bcc;
	LogString from;
	LogString subject;
	LogString smtpHost;
	LogString smtpUsername;
	LogString smtpPassword;
	int smtpPort;
	int bufferSize; // 512
	bool locationInfo;
	helpers::CyclicBuffer cb;
	spi::TriggeringEventEvaluatorPtr evaluator;
};

#define _priv static_cast<SMTPPriv*>(m_priv.get())

DefaultEvaluator::DefaultEvaluator()
{
}

bool DefaultEvaluator::isTriggeringEvent(const spi::LoggingEventPtr& event)
{
	return event->getLevel()->isGreaterOrEqual(Level::getError());
}

SMTPAppender::SMTPAppender()
	: AppenderSkeleton (std::make_unique<SMTPPriv>())
{
}

/**
Use <code>evaluator</code> passed as parameter as the
TriggeringEventEvaluator for this SMTPAppender.  */
SMTPAppender::SMTPAppender(spi::TriggeringEventEvaluatorPtr evaluator)
	: AppenderSkeleton (std::make_unique<SMTPPriv>(evaluator))
{
}

SMTPAppender::~SMTPAppender()
{
	finalize();
}

bool SMTPAppender::requiresLayout() const
{
	return true;
}


LogString SMTPAppender::getFrom() const
{
	return _priv->from;
}

void SMTPAppender::setFrom(const LogString& newVal)
{
	_priv->from = newVal;
}


LogString SMTPAppender::getSubject() const
{
	return _priv->subject;
}

void SMTPAppender::setSubject(const LogString& newVal)
{
	_priv->subject = newVal;
}

LogString SMTPAppender::getSMTPHost() const
{
	return _priv->smtpHost;
}

void SMTPAppender::setSMTPHost(const LogString& newVal)
{
	_priv->smtpHost = newVal;
}

int SMTPAppender::getSMTPPort() const
{
	return _priv->smtpPort;
}

void SMTPAppender::setSMTPPort(int newVal)
{
	_priv->smtpPort = newVal;
}

bool SMTPAppender::getLocationInfo() const
{
	return _priv->locationInfo;
}

void SMTPAppender::setLocationInfo(bool newVal)
{
	_priv->locationInfo = newVal;
}

LogString SMTPAppender::getSMTPUsername() const
{
	return _priv->smtpUsername;
}

void SMTPAppender::setSMTPUsername(const LogString& newVal)
{
	_priv->smtpUsername = newVal;
}

LogString SMTPAppender::getSMTPPassword() const
{
	return _priv->smtpPassword;
}

void SMTPAppender::setSMTPPassword(const LogString& newVal)
{
	_priv->smtpPassword = newVal;
}





void SMTPAppender::setOption(const LogString& option,
	const LogString& value)
{
	if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("BUFFERSIZE"), LOG4CXX_STR("buffersize")))
	{
		setBufferSize(OptionConverter::toInt(value, 512));
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("EVALUATORCLASS"), LOG4CXX_STR("evaluatorclass")))
	{
		setEvaluatorClass(value);
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("FROM"), LOG4CXX_STR("from")))
	{
		setFrom(value);
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("SMTPHOST"), LOG4CXX_STR("smtphost")))
	{
		setSMTPHost(value);
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("SMTPUSERNAME"), LOG4CXX_STR("smtpusername")))
	{
		setSMTPUsername(value);
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("SMTPPASSWORD"), LOG4CXX_STR("smtppassword")))
	{
		setSMTPPassword(value);
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("SUBJECT"), LOG4CXX_STR("subject")))
	{
		setSubject(value);
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("TO"), LOG4CXX_STR("to")))
	{
		setTo(value);
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("CC"), LOG4CXX_STR("cc")))
	{
		setCc(value);
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("BCC"), LOG4CXX_STR("bcc")))
	{
		setBcc(value);
	}
	else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("SMTPPORT"), LOG4CXX_STR("smtpport")))
	{
		setSMTPPort(OptionConverter::toInt(value, 25));
	}
	else
	{
		AppenderSkeleton::setOption(option, value);
	}
}


bool SMTPAppender::asciiCheck(const LogString& value, const LogString& field)
{
	for (LogString::const_iterator iter = value.begin();
		iter != value.end();
		iter++)
	{
		if (0x7F < (unsigned int) *iter)
		{
			LogLog::warn(field + LOG4CXX_STR(" contains non-ASCII character"));
			return false;
		}
	}

	return true;
}

/**
Activate the specified options, such as the smtp host, the
recipient, from, etc. */
void SMTPAppender::activateOptions(Pool& p)
{
	bool activate = true;

	if (_priv->layout == 0)
	{
		_priv->errorHandler->error(LOG4CXX_STR("No layout set for appender named [") + _priv->name + LOG4CXX_STR("]."));
		activate = false;
	}

	if (_priv->evaluator == 0)
	{
		_priv->errorHandler->error(LOG4CXX_STR("No TriggeringEventEvaluator is set for appender [") +
			_priv->name + LOG4CXX_STR("]."));
		activate = false;
	}

	if (_priv->smtpHost.empty())
	{
		_priv->errorHandler->error(LOG4CXX_STR("No smtpHost is set for appender [") +
			_priv->name + LOG4CXX_STR("]."));
		activate = false;
	}

	if (_priv->to.empty() && _priv->cc.empty() && _priv->bcc.empty())
	{
		_priv->errorHandler->error(LOG4CXX_STR("No recipient address is set for appender [") +
			_priv->name + LOG4CXX_STR("]."));
		activate = false;
	}

	activate &= asciiCheck(_priv->to, LOG4CXX_STR("to"));
	activate &= asciiCheck(_priv->cc, LOG4CXX_STR("cc"));
	activate &= asciiCheck(_priv->bcc, LOG4CXX_STR("bcc"));
	activate &= asciiCheck(_priv->from, LOG4CXX_STR("from"));

#if !LOG4CXX_HAVE_LIBESMTP
	_priv->errorHandler->error(LOG4CXX_STR("log4cxx built without SMTP support."));
	activate = false;
#endif

	if (activate)
	{
		AppenderSkeleton::activateOptions(p);
	}
}

/**
Perform SMTPAppender specific appending actions, mainly adding
the event to a cyclic buffer and checking if the event triggers
an e-mail to be sent. */
void SMTPAppender::append(const spi::LoggingEventPtr& event, Pool& p)
{
	if (!checkEntryConditions())
	{
		return;
	}

	LogString ndc;
	event->getNDC(ndc);
	event->getThreadName();
	// Get a copy of this thread's MDC.
	event->getMDCCopy();

	_priv->cb.add(event);

	if (_priv->evaluator->isTriggeringEvent(event))
	{
		sendBuffer(p);
	}
}

/**
This method determines if there is a sense in attempting to append.
<p>It checks whether there is a set output target and also if
there is a set layout. If these checks fail, then the boolean
value <code>false</code> is returned. */
bool SMTPAppender::checkEntryConditions()
{
#if LOG4CXX_HAVE_LIBESMTP

	if ((_priv->to.empty() && _priv->cc.empty() && _priv->bcc.empty()) || _priv->from.empty() || _priv->smtpHost.empty())
	{
		_priv->errorHandler->error(LOG4CXX_STR("Message not configured."));
		return false;
	}

	if (_priv->evaluator == 0)
	{
		_priv->errorHandler->error(LOG4CXX_STR("No TriggeringEventEvaluator is set for appender [") +
			_priv->name + LOG4CXX_STR("]."));
		return false;
	}


	if (_priv->layout == 0)
	{
		_priv->errorHandler->error(LOG4CXX_STR("No layout set for appender named [") + _priv->name + LOG4CXX_STR("]."));
		return false;
	}

	return true;
#else
	return false;
#endif
}



void SMTPAppender::close()
{
	_priv->closed = true;
}

LogString SMTPAppender::getTo() const
{
	return _priv->to;
}

void SMTPAppender::setTo(const LogString& addressStr)
{
	_priv->to = addressStr;
}

LogString SMTPAppender::getCc() const
{
	return _priv->cc;
}

void SMTPAppender::setCc(const LogString& addressStr)
{
	_priv->cc = addressStr;
}

LogString SMTPAppender::getBcc() const
{
	return _priv->bcc;
}

void SMTPAppender::setBcc(const LogString& addressStr)
{
	_priv->bcc = addressStr;
}

/**
Send the contents of the cyclic buffer as an e-mail message.
*/
void SMTPAppender::sendBuffer(Pool& p)
{
#if LOG4CXX_HAVE_LIBESMTP

	// Note: this code already owns the monitor for this
	// appender. This frees us from needing to synchronize on 'cb'.
	try
	{
		LogString sbuf;
		_priv->layout->appendHeader(sbuf, p);

		int len = _priv->cb.length();

		for (int i = 0; i < len; i++)
		{
			LoggingEventPtr event = _priv->cb.get();
			_priv->layout->format(sbuf, event, p);
		}

		_priv->layout->appendFooter(sbuf, p);

		SMTPSession session(_priv->smtpHost, _priv->smtpPort, _priv->smtpUsername, _priv->smtpPassword, p);

		SMTPMessage message(session, _priv->from, _priv->to, _priv->cc,
			_priv->bcc, _priv->subject, sbuf, p);

		session.send(p);

	}
	catch (std::exception& e)
	{
		LogLog::error(LOG4CXX_STR("Error occured while sending e-mail notification."), e);
	}

#endif
}

/**
Returns value of the <b>EvaluatorClass</b> option.
*/
LogString SMTPAppender::getEvaluatorClass()
{
	return _priv->evaluator == 0 ? LogString() : _priv->evaluator->getClass().getName();
}

log4cxx::spi::TriggeringEventEvaluatorPtr SMTPAppender::getEvaluator() const
{
	return _priv->evaluator;
}

void SMTPAppender::setEvaluator(log4cxx::spi::TriggeringEventEvaluatorPtr& trigger)
{
	_priv->evaluator = trigger;
}

/**
The <b>BufferSize</b> option takes a positive integer
representing the maximum number of logging events to collect in a
cyclic buffer. When the <code>BufferSize</code> is reached,
oldest events are deleted as new events are added to the
buffer. By default the size of the cyclic buffer is 512 events.
*/
void SMTPAppender::setBufferSize(int sz)
{
	_priv->bufferSize = sz;
	_priv->cb.resize(sz);
}

/**
The <b>EvaluatorClass</b> option takes a string value
representing the name of the class implementing the {@link
TriggeringEventEvaluator} interface. A corresponding object will
be instantiated and assigned as the triggering event evaluator
for the SMTPAppender.
*/
void SMTPAppender::setEvaluatorClass(const LogString& value)
{
	ObjectPtr obj = ObjectPtr(Loader::loadClass(value).newInstance());
	_priv->evaluator = log4cxx::cast<TriggeringEventEvaluator>(obj);
}

int SMTPAppender::getBufferSize() const
{
	return _priv->bufferSize;
}
