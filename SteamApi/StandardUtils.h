#pragma once

#undef GetMessage

#include <string>

namespace Util
{

	std::wstring FormatWstr(const wchar_t *pwzFormat, ...);
	std::string FormatStr(const char *pszFormat, ...);

	std::string StrWideToMulti(const wchar_t *pwz, int codepage = CP_UTF8);
	std::wstring StrMultiToWide(const char *psz, int codepage = CP_UTF8);

	std::wstring UrlEncodeString(const wchar_t *pwz);

}