/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _LOG4CXX_HELPERS_OBJECT_H
#define _LOG4CXX_HELPERS_OBJECT_H

#include <log4cxx/logstring.h>
#include <log4cxx/helpers/class.h>
#include <log4cxx/helpers/classregistration.h>


#define DECLARE_LOG4CXX_CLAZZ_OBJECT(object)\
	public:\
	class Clazz##object : public helpers::Class\
	{\
		public:\
			Clazz##object() : helpers::Class() {}\
			virtual ~Clazz##object() {}\
			virtual log4cxx::LogString getName() const { return LOG4CXX_STR(#object); } \
	};\
	static const helpers::Class& getStaticClass(); \
	static const log4cxx::helpers::ClassRegistration& registerClass();

#define DECLARE_ABSTRACT_LOG4CXX_OBJECT(object)\
	DECLARE_LOG4CXX_CLAZZ_OBJECT(object)\
	const helpers::Class& getClass() const override;

#define DECLARE_LOG4CXX_OBJECT(object)\
	public:\
	class Clazz##object : public helpers::Class\
	{\
		public:\
			Clazz##object() : helpers::Class() {}\
			virtual ~Clazz##object() {}\
			virtual log4cxx::LogString getName() const { return LOG4CXX_STR(#object); } \
			virtual object* newInstance() const\
			{\
				return new object();\
			}\
	};\
	const helpers::Class& getClass() const override;\
	static const helpers::Class& getStaticClass(); \
	static const log4cxx::helpers::ClassRegistration& registerClass();

#define DECLARE_LOG4CXX_OBJECT_WITH_CUSTOM_CLASS(object, class)\
	public:\
	const helpers::Class& getClass() const override;\
	static const helpers::Class& getStaticClass();\
	static const log4cxx::helpers::ClassRegistration&  registerClass();

#define IMPLEMENT_LOG4CXX_OBJECT(object)\
	const ::log4cxx::helpers::Class& object::getClass() const { return getStaticClass(); }\
	const ::log4cxx::helpers::Class& object::getStaticClass() { \
		static Clazz##object theClass;                         \
		return theClass;                                       \
	}                                                                      \
	const log4cxx::helpers::ClassRegistration& object::registerClass() {   \
		static log4cxx::helpers::ClassRegistration classReg(object::getStaticClass); \
		return classReg; \
	}\
	namespace log4cxx { namespace classes { \
	const ::log4cxx::helpers::ClassRegistration& object##Registration = object::registerClass(); \
	} }


#define IMPLEMENT_LOG4CXX_OBJECT_WITH_CUSTOM_CLASS(object, class)\
	const log4cxx::helpers::Class& object::getClass() const { return getStaticClass(); }\
	const log4cxx::helpers::Class& object::getStaticClass() { \
		static class theClass;                                 \
		return theClass;                                       \
	}                                                         \
	const log4cxx::helpers::ClassRegistration& object::registerClass() {   \
		static log4cxx::helpers::ClassRegistration classReg(object::getStaticClass); \
		return classReg; \
	}\
	namespace log4cxx { namespace classes { \
	const log4cxx::helpers::ClassRegistration& object##Registration = object::registerClass(); \
	} }

namespace log4cxx
{
class AppenderSkeleton;
class Logger;

namespace helpers
{
class Pool;

/** base class for java-like objects.*/
class LOG4CXX_EXPORT Object
{
	public:
		virtual ~Object() {}
		virtual const helpers::Class& getClass() const = 0;
		virtual bool instanceof(const Class& clazz) const = 0;
		virtual const void* cast(const Class& clazz) const = 0;
		DECLARE_LOG4CXX_CLAZZ_OBJECT(Object)
};
LOG4CXX_PTR_DEF(Object);
}

/**
 * Attempt to cast one Object to another kind of Object.
 *
 * On success, returns a new shared pointer that points at incoming.
 * On failure, returns an invalid shared pointer.
 */
template<typename Ret,
	typename Type,
	bool = std::is_base_of<Ret, helpers::Object>::value,
	bool = std::is_base_of<Type, helpers::Object>::value>
std::shared_ptr<Ret> cast(const std::shared_ptr<Type>& incoming)
{
	if(!incoming)
	{
		return std::shared_ptr<Ret>();
	}

	Ret* casted = reinterpret_cast<Ret*>(const_cast<void*>(incoming->cast(Ret::getStaticClass())));

	if ( casted )
	{
		return std::shared_ptr<Ret>( incoming, casted );
	}

	return std::shared_ptr<Ret>();
}

}

#define BEGIN_LOG4CXX_CAST_MAP()\
	const void * cast(const helpers::Class& clazz) const override\
	{\
		const void * object = 0;\
		if (&clazz == &helpers::Object::getStaticClass()) return (const helpers::Object *)this;

#define END_LOG4CXX_CAST_MAP()\
	return object;\
	}\
	bool instanceof(const helpers::Class& clazz) const override\
	{ return cast(clazz) != 0; }

#define LOG4CXX_CAST_ENTRY(Interface)\
	if (&clazz == &Interface::getStaticClass()) return (const Interface *)this;

#define LOG4CXX_CAST_ENTRY2(Interface, interface2)\
	if (&clazz == &Interface::getStaticClass()) return (Interface *)(interface2 *)this;

#define LOG4CXX_CAST_ENTRY_CHAIN(Interface)\
	object = Interface::cast(clazz);\
	if (object != 0) return object;

#endif //_LOG4CXX_HELPERS_OBJECT_H
