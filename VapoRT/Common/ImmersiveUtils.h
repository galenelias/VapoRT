#pragma once

inline Windows::Foundation::DateTime UnixTimeToDateTime(time_t time)
{
	Windows::Foundation::DateTime datetime;
	datetime.UniversalTime = ((time + 11644473600) * 10000000);
	return datetime;
}
