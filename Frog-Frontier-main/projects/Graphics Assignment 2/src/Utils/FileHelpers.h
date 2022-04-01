#pragma once

#include <string>
#include <vector>

class FileHelpers {
public:
	FileHelpers() = delete;
	/// <summary>
	/// Reads the entire contents of a file into a string
	/// </summary>
	/// <param name="filename">The path of the file to load</param>
	/// <returns>The entire contents of the file stored in a string</returns>
	static std::string ReadFile(const std::string& filename);

	/// <summary>
	/// Reads the entire contents of a file, and will also recursively include
	/// any other files needed as indicated by a #include fileName on a line
	/// </summary>
	/// <param name="filename">The path of the file to load</param>
	/// <param name="resolvedPaths">The list of paths that have already been included</param>
	/// <returns>The entire contents of the file, with includes resolved, stored in a string</returns>
	static std::string ReadResolveIncludes(const std::string& filename, std::vector<std::string> resolvedPaths = std::vector<std::string>());

	/// <summary>
	/// Helper for writing the contents of a string into a file
	/// </summary>
	/// <param name="filename">The path to write the content to</param>
	/// <param name="contents">The contents of the file to write</param>
	/// <param name="append">True if contents should be appended to end of existing files</param>
	static void WriteContentsToFile(const std::string& filename, const std::string& contents, bool append = false);
};