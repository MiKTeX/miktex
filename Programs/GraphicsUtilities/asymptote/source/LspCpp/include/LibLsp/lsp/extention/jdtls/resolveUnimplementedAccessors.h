#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include <string>
#include "checkHashCodeEqualsStatus.h"


struct AccessorField {
	std::string fieldName;
	bool isStatic =false;
	bool generateGetter = false;
	bool generateSetter = false;
	void swap(AccessorField& arg) noexcept{
		fieldName.swap(arg.fieldName);
		std::swap(isStatic, arg.isStatic);
		std::swap(generateGetter, arg.generateGetter);
		std::swap(generateSetter, arg.generateSetter);
	}
};

MAKE_REFLECT_STRUCT(AccessorField, fieldName,isStatic,generateGetter,generateSetter)

DEFINE_REQUEST_RESPONSE_TYPE(java_resolveUnimplementedAccessors, lsCodeActionParams, std::vector<AccessorField>, "java/resolveUnimplementedAccessors");
