#pragma once
#include <string>
#include <algorithm>
#include <vector>

// Borrowed from https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
int constexpr const_strlen(const char* str) {
	return *str ? 1 + const_strlen(str + 1) : 0;
}

/// <summary>
/// Provides helper functions for working with std::string
/// </summary>
class StringTools {
public:
	/// <summary>
	/// Takes a class or structure name returned from typeid().name() and
	/// removes the type specifier
	/// ex: "class StringTools" --> "StringTools"
	/// </summary>
	/// <param name="name">The unsanitized name of the type</param>
	static std::string SanitizeClassName(const std::string& name);

	/// <summary>
	/// Trims whitespace characters from the left side of a string, in place
	/// </summary>
	/// <param name="s">A reference to the string to be trimmed</param>
	static inline void LTrim(std::string& s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
			return !std::isspace(ch);
		}));
	}
	/// <summary>
	/// Trims whitespace characters from the right side of a string, in place
	/// </summary>
	/// <param name="s">A reference to the string to be trimmed</param>
	static inline void RTrim(std::string& s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
			return !std::isspace(ch);
		}).base(), s.end());
	}
	/// <summary>
	/// Trims whitespace characters from both ends of a string, in place
	/// </summary>
	/// <param name="s">A reference to the string to be trimmed</param>
	static inline void Trim(std::string& s) {
		LTrim(s);
		RTrim(s);
	}

	/// <summary>
	/// Trims a specified character from the left side of a string, in place
	/// </summary>
	/// <param name="s">A reference to the string to be trimmed</param>
	/// <param name="toTrim">The character to trim from the string</param>
	static inline void LTrim(std::string& s, char toTrim) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [=](int ch) {
			return ch != toTrim;
		}));
	}
	/// <summary>
	/// Trims a specified character from the right side of a string, in place
	/// </summary>
	/// <param name="s">A reference to the string to be trimmed</param>
	/// <param name="toTrim">The character to trim from the string</param>
	static inline void RTrim(std::string& s, char toTrim) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [=](int ch) {
			return ch != toTrim;;
		}).base(), s.end());
	}
	/// <summary>
	/// Trims a specified character from both sides of a string, in place
	/// </summary>
	/// <param name="s">A reference to the string to be trimmed</param>
	/// <param name="toTrim">The character to trim from the string</param>
	static inline void Trim(std::string& s, char toTrim) {
		LTrim(s, toTrim);
		RTrim(s, toTrim);
	}

	/// <summary>
	/// Converts a string to it's lower case version in place,
	/// without allocating any new memory
	/// </summary>
	/// <param name="s">The reference to the string to convert to lowercase</param>
	static void ToLower(std::string& s);
	/// <summary>
	/// Converts a string to it's upper case version in place,
	/// without allocating any new memory
	/// </summary>
	/// <param name="s">The reference to the string to convert to uppercase</param>
	static void ToUpper(std::string& s);

	/// <summary>
	/// Returns true if value ends with token
	/// </summary>
	/// <param name="value"></param>
	/// <param name="token"></param>
	/// <returns></returns>
	static bool EndsWith(const std::string& value, const std::string& token);

	/// <summary>
	/// Splits a string into multiple tokens, based on the given split condition
	/// </summary>
	/// <param name="s">The string that will be split</param>
	/// <param name="splitOn">The delimiter string to split on</param>
	/// <returns>A vector of tokens that have been split from the string</returns>
	static std::vector<std::string> Split(const std::string& s, const std::string& splitOn = ",");
	/// <summary>
	/// Splits a string into multiple tokens, based on the given split condition
	/// This overload appends the results to an existing vector
	/// </summary>
	/// <param name="s">The string that will be split</param>
	/// <param name="results">A reference to a vector of strings to store the resulting tokens</param>
	/// <param name="splitOn">The delimiter string to split on</param>
	/// <returns>The number of tokens this command appended to the results</returns>
	static int Split(const std::string& s, std::vector<std::string>& results, const std::string& splitOn = ",");
};