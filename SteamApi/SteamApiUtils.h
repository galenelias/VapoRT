#pragma once

#include "json.h"

//
//#define NOCOMM 1
//#include <windows.h>

#undef GetMessage

namespace Util
{
	bool JsonHasValue(http::json::value & jsonValue, const casablanca::string_t & valueName);
	casablanca::string_t JsonStringGetValueWithDefault(http::json::value & jsonValue, const casablanca::string_t & valueName, const casablanca::string_t & defaultValue);

	std::wstring EscapeSQLString(const wchar_t *pwz);
}