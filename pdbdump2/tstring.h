#pragma once

#ifndef _STRING_CODECVT_H_
#define _STRING_CODECVT_H_
#include <string>
#include <locale>
#include <sstream>
#include <codecvt>

inline std::wstring to_wstring(const std::string& src)
{
	//std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	std::wstring_convert<std::codecvt<wchar_t, char, mbstate_t>> myconv;
    return myconv.from_bytes(src);
}

inline std::string to_string(const std::wstring& src)
{
	//std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	std::wstring_convert<std::codecvt<wchar_t, char, mbstate_t>> myconv;
    return myconv.to_bytes(src);
}

#endif