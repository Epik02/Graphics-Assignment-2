#include "Utils/StringUtils.h"

std::string StringTools::SanitizeClassName(const std::string& name)
{
	const std::string classTag = "class ";
	const std::string structTag = "struct ";
	if (strncmp(name.c_str(), classTag.c_str(), classTag.size()) == 0) {
		return name.substr(classTag.size());
	}
	if (strncmp(name.c_str(), structTag.c_str(), structTag.size()) == 0) {
		return name.substr(structTag.size());
	}
	return name;
}

void StringTools::ToLower(std::string& s) {
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
		return std::tolower(c);
	});
}

void StringTools::ToUpper(std::string& s) {
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
		return std::toupper(c);
	});
}

bool StringTools::EndsWith(const std::string& value, const std::string& token)
{
	if (value.length() >= token.length()) {
		return (0 == value.compare(value.length() - token.length(), token.length(), token));
	}
	else {
		return false;
	}
}

std::vector<std::string> StringTools::Split(const std::string& s, const std::string& splitOn) {
	std::vector<std::string> result;
	Split(s, result, splitOn);
	return result;
}

int StringTools::Split(const std::string& s, std::vector<std::string>& results, const std::string& splitOn) {
	int result = 0;
	// Determines the length of our delimiter
	size_t splitLen = splitOn.size();
	// Will store the location of the end last delimiter we found
	size_t lastPos = 0;
	// Find first instance of the delimiter
	size_t seek = s.find(splitOn, 0);

	// As long as there are delimiters in the string
	while (seek != std::string::npos) {
		// Extract the token and push it to results
		results.push_back(s.substr(lastPos, seek - lastPos));
		result++;
		// Update seek position to the end of the delimiter
		lastPos = seek + splitLen;

		// Find the next delimiter
		seek = s.find(splitOn, lastPos);
	}
	// No more delimiters, return whatever's left as another token
	results.push_back(s.substr(lastPos, seek));
	return ++result;
}
