#pragma once

#include "LibLsp/JsonRpc/serializer.h"
#include <string>
struct OverridableMethod
{
	std::string bindingKey;
	std::string name;
	std::vector<std::string> parameters;
	bool unimplemented = false;
	std::string declaringClass;
	std::string declaringClassType;

	void swap(OverridableMethod& arg) noexcept
	{
		bindingKey.swap(arg.bindingKey);
		name.swap(arg.name);
		parameters.swap(arg.parameters);
		declaringClass.swap(arg.declaringClass);
		declaringClassType.swap(arg.declaringClassType);
		std::swap(unimplemented, arg.unimplemented);
	}
};
MAKE_REFLECT_STRUCT(OverridableMethod, bindingKey, name, parameters, unimplemented, declaringClass, declaringClassType);

struct  OverridableMethodsResponse
{
	std::string type;
	std::vector<OverridableMethod> methods;

	MAKE_SWAP_METHOD(OverridableMethodsResponse, type, methods)
};

MAKE_REFLECT_STRUCT(OverridableMethodsResponse, type, methods)
