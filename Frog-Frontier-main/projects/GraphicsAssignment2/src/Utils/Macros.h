#pragma once

#include <memory>

#define MAKE_PTRS(Type) \
	typedef std::shared_ptr<Type> Sptr; \
	typedef std::unique_ptr<Type> Uptr; \
	typedef std::weak_ptr<Type>   Wptr;

#define NO_MOVE(Type) \
	Type& operator =(Type&& other) = delete; \
	Type(Type&&) = delete;

#define NO_COPY(Type) \
	Type& operator =(const Type& other) = delete; \
	Type(const Type&) = delete;

#define DEFINE_RESOURCE(Type) \
	MAKE_PTRS(Type); \
	NO_MOVE(Type); \
	NO_COPY(Type) 