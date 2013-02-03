#include "pch.h"

#include "StandardUtils.h"

#include <shcore.h>
#include <string>
#include <cassert>

namespace Util
{
	std::wstring FormatWstr(const wchar_t *pwzFormat, ...)
	{
		std::wstring  wstrRet;
		va_list       arg;
		int           cch;

		va_start(arg, pwzFormat);

		// Check the required size
		cch = _vscwprintf(pwzFormat, arg);
		wstrRet.resize(cch);

		va_start(arg, pwzFormat);
		_vsnwprintf_s(&wstrRet[0], cch+1, cch, pwzFormat, arg);

		va_end(arg);

		return wstrRet;
	}

	std::string FormatStr(const char *pszFormat, ...)
	{
		std::string   strRet;
		va_list       arg;
		int           cch;

		va_start(arg, pszFormat);

		// Check the required size
		cch = _vscprintf(pszFormat, arg);
		strRet.resize(cch);

		va_start(arg, pszFormat);
		_vsnprintf_s(&strRet[0], cch+1, cch, pszFormat, arg);

		va_end(arg);

		return strRet;
	}

	std::string StrWideToMulti(const wchar_t *pwz, int codePage)
	{
		int cchRequired = WideCharToMultiByte(codePage, 0, pwz, -1, nullptr, 0, nullptr, NULL);

		std::string stwResult;
		stwResult.resize(cchRequired);

		WideCharToMultiByte(codePage, 0, pwz, -1, &stwResult[0], cchRequired, nullptr, NULL);
		return stwResult;
	}

	std::wstring StrMultiToWide(const char *psz, int codePage)
	{
		int cchRequired = MultiByteToWideChar(codePage, 0, psz, -1, nullptr, 0);

		std::wstring wstResult;
		wstResult.resize(cchRequired);

		MultiByteToWideChar(codePage, 0, psz, -1, &wstResult[0], cchRequired);
		return wstResult;

	}

	bool JsonHasValue(http::json::value & jsonValue, const casablanca::string_t & valueName)
	{
		return jsonValue.fields().find(valueName) != end(jsonValue.fields());
	}

	casablanca::string_t JsonStringGetValueWithDefault(http::json::value & jsonValue, const casablanca::string_t & valueName, const casablanca::string_t & defaultValue)
	{
		if (!JsonHasValue(jsonValue, valueName))
			return defaultValue;
		else
			return jsonValue[valueName].as_string();
	}

	std::wstring EscapeSQLString(const wchar_t *pwz)
	{
		std::wstring result;
		while (*pwz)
		{
			if (*pwz == L'\'')
				result += L"''";
			else
				result += *pwz;
			pwz++;
		}
		return result;
	}

	std::wstring UrlEncodeString(const wchar_t *pwz)
	{
		std::wstring result;
		while (*pwz)
		{
			wchar_t ch = *pwz;

			const wchar_t c_wzUnreservedSpecialChars[] = L"-_.~";
			
			if ( iswalnum(ch) || wcschr(c_wzUnreservedSpecialChars, ch) != nullptr)
			{
				result += ch;
			}
			else if (ch == L' ')
			{
				result += L'+';
			}
			else
			{
				wchar_t wszPercentEncoded[16];
				assert(ch >= 0x10 && ch <= 0xFF);
				swprintf_s(wszPercentEncoded, L"%%%02X", (int)ch);
				result += wszPercentEncoded;
			}
			pwz++;
		}
		return result;
	}

	LONGLONG ParseLongLongFromString(const char *psz)
	{
		std::istringstream iss(psz);
		LONGLONG result;
		iss >> result;
		return result;
	}
}