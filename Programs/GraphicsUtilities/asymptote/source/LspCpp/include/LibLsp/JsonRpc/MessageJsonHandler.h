#pragma once
#include <string>
#include <map>
#include <functional>
#include <LibLsp/JsonRpc/message.h>
class Reader;


using  GenericRequestJsonHandler = std::function< std::unique_ptr<LspMessage>(Reader&) >;
using  GenericResponseJsonHandler = std::function< std::unique_ptr<LspMessage>(Reader&) >;
using  GenericNotificationJsonHandler = std::function< std::unique_ptr<LspMessage>(Reader&) >;

class MessageJsonHandler
{
public:
	std::map< std::string, GenericRequestJsonHandler > method2request;
	std::map< std::string,  GenericResponseJsonHandler > method2response;
	std::map< std::string, GenericNotificationJsonHandler > method2notification;


	const GenericRequestJsonHandler* GetRequestJsonHandler(const char* methodInfo) const
	{
		const auto findIt = method2request.find(methodInfo);
		return  findIt == method2request.end() ? nullptr : &findIt->second;
	}
	
	void SetRequestJsonHandler(const std::string& methodInfo, GenericRequestJsonHandler handler)
	{
		method2request[methodInfo] = handler;
	}
	
	const GenericResponseJsonHandler* GetResponseJsonHandler(const char* methodInfo) const
	{
		const auto findIt = method2response.find(methodInfo);
		return  findIt == method2response.end() ? nullptr : &findIt->second;
	}
	
	void SetResponseJsonHandler(const std::string& methodInfo,GenericResponseJsonHandler handler) 
	{
		method2response[methodInfo] = handler;
	}

	const GenericNotificationJsonHandler* GetNotificationJsonHandler(const char* methodInfo) const
	{
		const auto findIt = method2notification.find(methodInfo);
		return  findIt == method2notification.end() ? nullptr : &findIt->second;
	}

	void SetNotificationJsonHandler(const std::string& methodInfo, GenericNotificationJsonHandler handler)
	{
		method2notification[methodInfo] = handler;
	}


	
	std::unique_ptr<LspMessage> parseResponseMessage(const std::string&, Reader&);
	std::unique_ptr<LspMessage> parseRequstMessage(const std::string&, Reader&);
	bool resovleResponseMessage(Reader&, std::pair<std::string, std::unique_ptr<LspMessage>>& result);
	std::unique_ptr<LspMessage> parseNotificationMessage(const std::string&, Reader&);
};

