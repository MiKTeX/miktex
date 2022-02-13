#pragma once
#include <string>
#include <functional>

class MessageProducer
{
public:
	
	
	typedef  std::function< void(std::string&&) >  MessageConsumer;
	virtual  ~MessageProducer() = default;
	virtual void listen(MessageConsumer) = 0;
};
