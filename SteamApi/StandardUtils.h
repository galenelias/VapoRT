#pragma once

#undef GetMessage

#include <string>

namespace Util
{

	std::wstring FormatWstr(const wchar_t *pwzFormat, ...);
	std::string FormatStr(const char *pszFormat, ...);

	std::string StrWideToMulti(const wchar_t *pwz, int codepage = CP_UTF8);
	std::wstring StrMultiToWide(const char *psz, int codepage = CP_UTF8);

	LONGLONG ParseLongLongFromString(const char *psz);

	//bool JsonHasValue(http::json::value & jsonValue, const casablanca::string_t & valueName);
	//casablanca::string_t JsonStringGetValueWithDefault(http::json::value & jsonValue, const casablanca::string_t & valueName, const casablanca::string_t & defaultValue);
	//
	//std::wstring EscapeSQLString(const wchar_t *pwz);

	std::wstring UrlEncodeString(const wchar_t *pwz);

}