#pragma once

template<class>
struct sfinae_true : std::true_type {};

namespace detail {
	template<class T, class A0>
	static auto test_json(int)->sfinae_true<decltype(std::declval<T>().FromJson(std::declval<A0>()))>;
	template<class, class A0>
	static auto test_json(long)->std::false_type;
} // detail::

template<class T, class Arg>
struct test_json : decltype(detail::test_json<T, Arg>(0)){};