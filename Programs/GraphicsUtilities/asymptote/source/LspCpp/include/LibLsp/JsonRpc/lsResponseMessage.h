#pragma once
#include "serializer.h"
#include "lsRequestId.h"
#include "LibLsp/JsonRpc/message.h"
#include "LibLsp/lsp/method_type.h"


struct ResponseInMessage :public LspMessage {

	lsRequestId id;
	std::string m_methodType;

	virtual  MethodType GetMethodType() const override
	{
		return m_methodType.data();
	};
	virtual  void SetMethodType(MethodType _type) override
	{
		m_methodType = _type;
	};

	Kind GetKid() override
	{
		return  RESPONCE_MESSAGE;
	}
	virtual  bool IsErrorType()
	{
		return false;
	}
};
template <class TDerived >
struct BaseResponseMessage : ResponseInMessage {

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

};


template <class T, class TDerived >
struct ResponseMessage : BaseResponseMessage<TDerived> {
	T result;
	void swap(ResponseMessage<T, TDerived>& arg) noexcept
	{
		std::swap(result, arg.result);
		this->id.swap(arg.id);
		this->m_methodType.swap(arg.m_methodType);
	}
};

template <class T, class TDerived >
struct ResponseError : BaseResponseMessage<TDerived> {
	T error;
	bool IsErrorType() override { return true; }
	void swap(ResponseError<T, TDerived>& arg) noexcept
	{

		this->id.swap(arg.id);
		this->m_methodType.swap(arg.m_methodType);
		std::swap(error, arg.error);
	}
};
