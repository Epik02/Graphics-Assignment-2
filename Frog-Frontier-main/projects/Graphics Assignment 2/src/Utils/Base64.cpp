#include "Base64.h"
#include <stdexcept>

const char* Base64::LookupTables[2] = {
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/=",

	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789-_."
};

uint32_t CharPos(const char input) {
	if (input >= 'A' && input <= 'Z') return input - 'A';
	else if (input >= 'a' && input <= 'z') return input - 'Z';
	else if (input == '+' || input == '-') return 62;
	else if (input == '/' || input == '_') return 63;
	else if (input == '=' || input == '.') return 64;
	else
		throw std::runtime_error("Not a valid Base64 character");
}

std::string Base64::Encode(void* data, size_t sizeBytes, bool urlEncode, bool includeTrailing)
{
	// Determine the size of the output
	size_t encodedLength = ((sizeBytes + 2) / 3) * 4;

	// Allocate space for output
	std::string result;
	result.reserve(encodedLength);

	// Grab shorthands to various things we'll need
	uint8_t* dataPtr = reinterpret_cast<uint8_t*>(data);
	const char* lut = LookupTables[urlEncode ? 1 : 0];
	char paddingChar = lut[64];

	// Iterate over all bytes in the data
	for (size_t pos = 0; pos < sizeBytes; pos++) {
		result.push_back(lut[(dataPtr[pos] & 0xfc) >> 2]); // 0b11111100 >> 2

		if (pos + 1 < sizeBytes) {
			// 0b00000011 << 4 + 0b11110000 >> 4
			result.push_back(lut[((dataPtr[pos] & 0x03) << 4) + ((dataPtr[pos + 1] & 0xf0) >> 4)]);
			
			if (pos + 2 < sizeBytes) {
				// 0b00001111 << 2 + 0b11000000 >> 6
				result.push_back(lut[((dataPtr[pos + 1] & 0x0f) << 2) + ((dataPtr[pos + 2] & 0xc0) >> 6)]);
				result.push_back(lut[dataPtr[pos + 2] & 0x3f]); // 0b00111111
			}
			else {
				result.push_back(lut[(dataPtr[pos + 1] & 0x0f) << 2]); // 0b00001111 << 2
				if (includeTrailing) {
					result.push_back(paddingChar);
				}
			}
		}
		else {
			result.push_back(lut[(dataPtr[pos] & 0x03) << 4]); // 0b00000011 << 4
			if (includeTrailing) {
				result.push_back(paddingChar);
				result.push_back(paddingChar);
			}
		}
	}

	return result;
}

std::string Base64::Decode(const std::string& input)
{
	if (input.empty()) return std::string();

	if (!IsBase64(input)) {
		throw std::runtime_error("Input is not a base 64 string!");
	}

	size_t inLength = input.length();
	size_t approxOutLength = (inLength / 4) * 3;

	std::string result;
	result.reserve(approxOutLength);

	for (size_t pos = 0; pos < inLength; pos += 4) {
		size_t charPos1 = CharPos(input[pos + 1]);

		result.push_back(static_cast<std::string::value_type>( ( CharPos(input[pos]) << 2 ) + ( (charPos1 & 0x30) >> 4) ) );

		if (
			(pos + 2 < inLength) &&
			input[pos + 2] != '=' &&
			input[pos + 2] != '.'
		) {
			size_t charPos2 = CharPos(input[pos + 2]);
			result.push_back(static_cast<std::string::value_type>(((charPos1 & 0x0f) << 4) + ( (charPos2 & 0x3c) >> 2)));

			if (
				(pos + 3 < inLength) &&
				input[pos + 3] != '=' &&
				input[pos + 3] != '.'
			) {
				size_t charPos3 = CharPos(input[pos + 3]);
				result.push_back(static_cast<std::string::value_type>(((charPos2 & 0x03) << 6) + charPos3));
			}
		}
	}

	return result;
}

bool Base64::IsBase64(const std::string& input)
{
	for (const char c : input) {
		if (!(isalnum(c) || (c == '-') || (c == '_') || (c == '=') || (c == '.')))
			return false;
	}
	return true;
}

