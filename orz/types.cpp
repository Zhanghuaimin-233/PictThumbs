#include "types.h"

#include <algorithm>
#include <cwctype>
#include <memory>
#include <sstream>
#include <Windows.h>

int RoundCast(float rhs) {
	return static_cast<int>(rhs + 0.5f);
}

std::wstring UTF8ToWString(const std::string& utf8) {
	if (utf8.empty()) return std::wstring();
	int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
	if (size <= 0) return std::wstring();
	std::wstring result(size - 1, 0);
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &result[0], size);
	return result;
}

std::string WStringToUTF8(const std::wstring& utf16) {
	if (utf16.empty()) return std::string();
	int size = WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (size <= 0) return std::string();
	std::string result(size - 1, 0);
	WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), -1, &result[0], size, nullptr, nullptr);
	return result;
}

std::wstring ToWString( uint32_t i ) {
	std::wstringstream ss;
	ss << i;
	return ss.str();
}

std::wstring ToWString( int i ) {
	std::wstringstream ss;
	ss << i;
	return ss.str();
}

std::wstring ToWString( uint8_t i ) {
	std::wstringstream ss;
	ss << i;
	return ss.str();
}

std::wstring ToWString( const char* i ) {
	std::wstringstream ss;
	ss << i;
	return ss.str();
}

std::string ToAString(const std::wstring& s) {
	return std::string(s.begin(), s.end());
}

std::string ToAString(int i) {
	std::stringstream ss;
	ss << i;
	return ss.str();
}
