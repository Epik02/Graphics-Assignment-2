/*
 *	Loosely based off this implementation (it was awful though, my god)
 *
 *	The MIT License (MIT)
 *	Copyright (c) 2014 Graeme Hill (http://graemehill.ca)
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *	The above copyright notice and this permission notice shall be included in
 *	all copies or substantial portions of the Software.
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *	THE SOFTWARE.
 *
 *	https://github.com/graeme-hill/crossguid
*/

#pragma once

#include <functional>
#include <iostream>
#include <array>
#include <sstream>
#include <string_view>
#include <utility>
#include <iomanip>

#ifdef GUID_CEREAL_ARCHIVES
#include <cereal/cereal.hpp>
#endif

// Class to represent a GUID/UUID. Each instance acts as a wrapper around a
// 16 byte value that can be passed around by value. It also supports
// conversion to string (via the stream operator <<) and conversion from a
// string via constructor.
class Guid
{
public:
	/// <summary>
	/// Creates an invalid UUID with all zeros. To generate a UUID see New
	/// </summary>
	/// <see cref="New"/>
	Guid() noexcept;
	/// <summary>
	/// Parses a GUID from a string. If the string is invalid, will return an invalid GUID
	/// </summary>
	explicit Guid(std::string_view fromString);

	Guid(const Guid& other) = default;
	Guid(Guid&& other) = default;
	Guid& operator=(const Guid& other) noexcept = default;
	Guid& operator=(Guid&& other) noexcept = default;

	bool operator==(const Guid& other) const;
	bool operator!=(const Guid& other) const;

	/// <summary>
	/// Converts this GUID into a string
	/// </summary>
	/// <returns>A new string containing the dash-separated GUID</returns>
	std::string str() const;
	/// <summary>
	/// Converts this GUID into a string
	/// </summary>
	/// <returns>A new string containing the dash-separated GUID</returns>
	operator std::string() const;
	/// <summary>
	/// Gets the underlying byte array for this GUID (note that the size of this array is 16 bytes)
	/// </summary>
	/// <returns>A pointer to the underlying data store, 16 bytes</returns>
	const uint8_t* bytes() const;
	/// <summary>
	/// Gets whether or not this GUID is currently valid
	/// </summary>
	/// <returns>True if the GUID is a valid UUID, false if otherwise</returns>
	bool isValid() const;
	/// <summary>
	/// Zeroes out this GUID, making it invalid
	/// </summary>
	void Clear();

	/// <summary>
	/// Generates a new GUID
	/// </summary>
	/// <returns>A new unique GUID</returns>
	static Guid New();
	/// <summary>
	/// Creates a new GUID from an array of 16 bytes
	/// </summary>
	/// <param name="data">The data to create the new GUID from, must contain 16 bytes</param>
	/// <returns>A GUID loaded from the data store</returns>
	static Guid FromBytes(unsigned char* data);

private:

	// actual data
	uint8_t _bytes[16];

	// make the << operator a friend so it can access _bytes
	friend std::ostream& operator<<(std::ostream& s, const Guid& guid);
	friend bool operator<(const Guid& lhs, const Guid& rhs);
	friend bool operator>(const Guid& lhs, const Guid& rhs);

	#ifdef GUID_CEREAL_ARCHIVES
	// Allows the cereal library to access our internal data
	friend class cereal::access;

	// Save a GUID into a cereal archive
	template <class Archive>
	void save(Archive& ar) const {
		if constexpr (cereal::traits::is_text_archive<Archive>::value) {
			ar.saveBinaryValue(&_bytes[0], 16);
		} else {
			ar(cereal::binary_data(&_bytes[0], 16));
		}
	}
	// Load a GUID from a cereal archive
	template <class Archive>
	void load(Archive& ar) {
		if constexpr (cereal::traits::is_text_archive<Archive>::value) {
			ar.loadBinaryValue(&_bytes[0], 16);
		} else {
			ar(cereal::binary_data(&_bytes[0], 16));
		}
	}
	#endif
};

// Handles the implementation details for our hashing algorithm
namespace details {
	template <typename...> struct hash;

	template<typename T>
	struct hash<T> : public std::hash<T> {
		using std::hash<T>::hash;
	};


	template <typename T, typename... Rest>
	struct hash<T, Rest...>
	{
		inline std::size_t operator()(const T& v, const Rest&... rest) {
			std::size_t seed = hash<Rest...>{}(rest...);
			seed ^= hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
			return seed;
		}
	};
}

namespace std {
	// Specialization for std::hash<Guid> 
	// Uses the underlying byte field and hashes the values together as a pair of
	// 8 byte integers
	template <>
	struct hash<Guid>
	{
		std::size_t operator()(Guid const& guid) const {
			const uint64_t* p = reinterpret_cast<const uint64_t*>(guid.bytes());
			return details::hash<uint64_t, uint64_t>{}(p[0], p[1]);
		}
	};
}