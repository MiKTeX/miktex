#pragma once
#include <map>
#include <functional>
#include <memory>
#include "MessageIssue.h"
struct LspMessage;
struct NotificationInMessage;
struct lsBaseOutMessage;
struct RequestInMessage;

using GenericResponseHandler = std::function<bool(std::unique_ptr<LspMessage>)>;
using GenericRequestHandler = std::function<bool(std::unique_ptr<LspMessage>)>;
using GenericNotificationHandler = std::function<bool(std::unique_ptr<LspMessage>)>;

class Endpoint
{
public:
    virtual ~Endpoint() = default;
    virtual bool onRequest(std::unique_ptr<LspMessage>) = 0;
    virtual bool notify(std::unique_ptr<LspMessage>) = 0;

    virtual bool onResponse(std::string const&, std::unique_ptr<LspMessage>) = 0;
    virtual void registerRequestHandler(std::string const&, GenericResponseHandler) = 0;
    virtual void registerNotifyHandler(std::string const&, GenericNotificationHandler) = 0;
};

class GenericEndpoint : public Endpoint
{

public:
    GenericEndpoint(lsp::Log& l) : log(l)
    {
    }
    bool notify(std::unique_ptr<LspMessage>) override;
    bool onResponse(std::string const&, std::unique_ptr<LspMessage>) override;

    bool onRequest(std::unique_ptr<LspMessage>) override;
    std::map<std::string, GenericRequestHandler> method2request;
    std::map<std::string, GenericResponseHandler> method2response;
    std::map<std::string, GenericNotificationHandler> method2notification;

    void registerRequestHandler(std::string const& method, GenericResponseHandler cb) override
    {
        method2request[method] = cb;
    }

    void registerNotifyHandler(std::string const& method, GenericNotificationHandler cb) override
    {
        method2notification[method] = cb;
    }
    lsp::Log& log;
};
