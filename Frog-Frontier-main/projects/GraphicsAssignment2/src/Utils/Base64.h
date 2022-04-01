#pragma once

#include <string>;

class Base64 {
public:
	static std::string Encode(void* data, size_t sizeBytes, bool urlEncode = true, bool includeTrailing = false);
	static std::string Decode(const std::string& input);
	static bool IsBase64(const std::string& input);

	static const char* LookupTables[2];
};