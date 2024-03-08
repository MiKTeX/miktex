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

       virtual  void bind(std::shared_ptr<lsp::istream>) = 0 ;

protected:
        MessageIssueHandler& issueHandler;
        std::shared_ptr < lsp::istream> input;
        
};

class LSPStreamMessageProducer : public StreamMessageProducer
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

    LSPStreamMessageProducer(
            MessageIssueHandler& message_issue_handler, std::shared_ptr < lsp::istream> _input)
            : StreamMessageProducer(message_issue_handler,_input)
    {
    }
    LSPStreamMessageProducer(
            MessageIssueHandler& message_issue_handler)
            : StreamMessageProducer(message_issue_handler)
    {
    }


    void listen(MessageConsumer) override;
    void bind(std::shared_ptr<lsp::istream>) override;
    void parseHeader(std::string& line, Headers& headers);


};
class DelimitedStreamMessageProducer : public StreamMessageProducer
{
public:

    DelimitedStreamMessageProducer(
            MessageIssueHandler& message_issue_handler, std::shared_ptr <lsp::istream> _input)
            : StreamMessageProducer(message_issue_handler,_input)
    {
    }
    DelimitedStreamMessageProducer(
            MessageIssueHandler& message_issue_handler)
            : StreamMessageProducer(message_issue_handler)
    {
    }


    void listen(MessageConsumer) override;
    void bind(std::shared_ptr < lsp::istream>) override;

};