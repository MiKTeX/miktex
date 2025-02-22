#pragma once
#include <string>
#include <map>
#include <functional>
#include <LibLsp/JsonRpc/message.h>
class Reader;

using GenericRequestJsonHandler = std::function<std::unique_ptr<LspMessage>(Reader&)>;
using GenericResponseJsonHandler = std::function<std::unique_ptr<LspMessage>(Reader&)>;
using GenericNotificationJsonHandler = std::function<std::unique_ptr<LspMessage>(Reader&)>;

class MessageJsonHandler
{
public:
    std::map<std::string, GenericRequestJsonHandler> method2request;
    std::map<std::string, GenericResponseJsonHandler> method2response;
    std::map<std::string, GenericNotificationJsonHandler> method2notification;

    GenericRequestJsonHandler const* GetRequestJsonHandler(char const* methodInfo) const
    {
        auto const findIt = method2request.find(methodInfo);
        return findIt == method2request.end() ? nullptr : &findIt->second;
    }

    void SetRequestJsonHandler(std::string const& methodInfo, GenericRequestJsonHandler handler)
    {
        method2request[methodInfo] = handler;
    }

    GenericResponseJsonHandler const* GetResponseJsonHandler(char const* methodInfo) const
    {
        auto const findIt = method2response.find(methodInfo);
        return findIt == method2response.end() ? nullptr : &findIt->second;
    }

    void SetResponseJsonHandler(std::string const& methodInfo, GenericResponseJsonHandler handler)
    {
        method2response[methodInfo] = handler;
    }

    GenericNotificationJsonHandler const* GetNotificationJsonHandler(char const* methodInfo) const
    {
        auto const findIt = method2notification.find(methodInfo);
        return findIt == method2notification.end() ? nullptr : &findIt->second;
    }

    void SetNotificationJsonHandler(std::string const& methodInfo, GenericNotificationJsonHandler handler)
    {
        method2notification[methodInfo] = handler;
    }

    std::unique_ptr<LspMessage> parseResponseMessage(std::string const&, Reader&);
    std::unique_ptr<LspMessage> parseRequstMessage(std::string const&, Reader&);
    bool resovleResponseMessage(Reader&, std::pair<std::string, std::unique_ptr<LspMessage>>& result);
    std::unique_ptr<LspMessage> parseNotificationMessage(std::string const&, Reader&);
};
