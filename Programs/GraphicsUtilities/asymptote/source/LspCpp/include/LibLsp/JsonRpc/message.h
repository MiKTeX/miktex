#pragma once

#include <atomic>
#include <string>
#include <iostream>
#include <LibLsp/JsonRpc/serializer.h>
#include "LibLsp/lsp/method_type.h"

struct LspMessage
{
public:
	std::string jsonrpc = "2.0";
	virtual void ReflectWriter(Writer&)   = 0;

	// Send the message to the language client by writing it to stdout.
	void Write(std::ostream& out);
	

	virtual MethodType GetMethodType() const = 0;
	virtual void SetMethodType(MethodType) = 0;
	
	virtual ~LspMessage()=default;
	enum Kind
	{
		REQUEST_MESSAGE,
		RESPONCE_MESSAGE,
		NOTIFICATION_MESSAGE
	};

	virtual  Kind GetKid() = 0;
	virtual std::string ToJson()  ;
	
};

