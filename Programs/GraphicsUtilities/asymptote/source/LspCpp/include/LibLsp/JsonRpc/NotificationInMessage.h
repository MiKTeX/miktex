#pragma once


#include "lsRequestId.h"
#include "LibLsp/JsonRpc/message.h"



// NotificationInMessage does not have |id|.
struct NotificationInMessage : public LspMessage {

	Kind GetKid() override
	{
		return  NOTIFICATION_MESSAGE;
	}
	MethodType GetMethodType() const override
	{
		return method.c_str();
	}
	void SetMethodType(MethodType _t) override
	{
		method = _t;
	}
	std::string method;
};
template <class T, class TDerived >
struct lsNotificationInMessage : NotificationInMessage {

	void ReflectWriter(Writer& writer) override {
		Reflect(writer, static_cast<TDerived&>(*this));
	}
	lsNotificationInMessage(MethodType _method)
	{
		method = _method;
	}

	static std::unique_ptr<LspMessage> ReflectReader(Reader& visitor) {

		TDerived* temp = new TDerived();

		std::unique_ptr<TDerived>  message = std::unique_ptr<TDerived>(temp);
		// Reflect may throw and *message will be partially deserialized.
		Reflect(visitor, static_cast<TDerived&>(*temp));
		return message;

	}
	void swap(lsNotificationInMessage& arg) noexcept
	{
		method.swap(method);
		std::swap(params, arg.params);
	}
	T params;
};

#define DEFINE_NOTIFICATION_TYPE(MSG,paramType,methodInfo)\
namespace  MSG {\
	struct notify : public lsNotificationInMessage< paramType , notify >{\
		static constexpr   MethodType  kMethodInfo = methodInfo;\
		notify():lsNotificationInMessage(kMethodInfo){}                                   \
	};\
};\
MAKE_REFLECT_STRUCT(MSG::notify, jsonrpc,method, params)
