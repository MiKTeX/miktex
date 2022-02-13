#pragma once


#include "serializer.h"
#include <atomic>
#include <mutex>
#include "lsRequestId.h"
#include "LibLsp/JsonRpc/message.h"
#include "LibLsp/lsp/method_type.h"
#include "lsResponseMessage.h"

struct RequestInMessage : public LspMessage {
	// number or string, actually no null
	lsRequestId id;
	std::string method;
	Kind GetKid() override
	{
		return  REQUEST_MESSAGE;
	}
	
};



template <class T, class TDerived >
struct lsRequest : public RequestInMessage
{
	lsRequest(MethodType _method)
	{
		method = _method;
	}
	MethodType GetMethodType() const override { return method.c_str(); }
	void SetMethodType(MethodType _method) override
	{
		method = _method;
	}								  \
	void ReflectWriter(Writer& writer) override {
		Reflect(writer, static_cast<TDerived&>(*this));
	}
	
	static std::unique_ptr<LspMessage> ReflectReader(Reader& visitor) {

		TDerived* temp = new TDerived();
		std::unique_ptr<TDerived>  message = std::unique_ptr<TDerived>(temp);
		// Reflect may throw and *message will be partially deserialized.
		Reflect(visitor, static_cast<TDerived&>(*temp));
		return message;
	}
	void swap(lsRequest& arg) noexcept
	{
		id.swap(arg.id);
		method.swap(method);
		std::swap(params, arg.params);
	}
	T params;
};


#define DEFINE_REQUEST_RESPONSE_TYPE(MSG,request_param,response_result,methodInfo)\
namespace  MSG {\
	struct response :public ResponseMessage< response_result, response> {}; \
	struct request : public lsRequest< request_param , request >{\
		static constexpr   MethodType  kMethodInfo = methodInfo;\
		request():lsRequest(kMethodInfo){}                                   \
		 using Response = response;\
	};\
};\
MAKE_REFLECT_STRUCT(MSG::request, jsonrpc, id, method, params);\
MAKE_REFLECT_STRUCT(MSG::response, jsonrpc, id, result);

