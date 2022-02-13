#pragma once
#include <string>
#include <functional>
#include "MessageProducer.h"
#include <iostream>
#include <memory>
#include "MessageIssue.h"

namespace lsp {
	class istream;
}

class StreamMessageProducer : public MessageProducer
{
public:
	struct Headers
	{
		int contentLength = -1;
		std::string charset;
		void clear()
		{
			contentLength = -1;
			charset.clear();
		}
	};
	bool handleMessage(Headers& headers, MessageConsumer callBack);
	StreamMessageProducer(
		MessageIssueHandler& message_issue_handler, std::shared_ptr < lsp::istream> _input)
		: issueHandler(message_issue_handler),
		  input(_input)
	{
	}
	StreamMessageProducer(
		MessageIssueHandler& message_issue_handler)
		: issueHandler(message_issue_handler)
	{
	}
	
	bool keepRunning = false;
	void listen(MessageConsumer) override;
	void bind(std::shared_ptr < lsp::istream>);
	void parseHeader(std::string& line, Headers& headers);
private:
	MessageIssueHandler& issueHandler;
	std::shared_ptr < lsp::istream> input;
	

};
