#pragma once

namespace VapoRT
{
	public ref class DateConverter sealed : public Windows::UI::Xaml::Data::IValueConverter  
	{
	public:
		virtual Platform::Object^ Convert(Platform::Object^ value,
			Windows::UI::Xaml::Interop::TypeName targetType,
			Platform::Object^ parameter,
			Platform::String^ language)
		{

			Windows::Foundation::DateTime dt = (Windows::Foundation::DateTime) value; 
			Windows::Globalization::DateTimeFormatting::DateTimeFormatter^ dtf;

			Windows::Globalization::Calendar^ calendar = ref new Windows::Globalization::Calendar();
			calendar->SetToNow();

			long long dtDiff = calendar->GetDateTime().UniversalTime - dt.UniversalTime;

			const LONGLONG c_secondsPerMinute = 60;
			const LONGLONG c_minutesPerHour = 60;
			const LONGLONG c_hoursPerDay = 24;
			const LONGLONG c_daysPerMonth = 30;
			const LONGLONG c_monthsPerYear = 12;

			const LONGLONG c_ticksPerSecond = 10000000ULL;
			const LONGLONG c_ticksPerMinute = c_ticksPerSecond * c_secondsPerMinute;
			const LONGLONG c_ticksPerHour = c_ticksPerMinute * c_minutesPerHour;
			const LONGLONG c_ticksPerDay = c_ticksPerHour * c_hoursPerDay;

			std::wstring wstDuration;
			if (dtDiff > c_ticksPerDay)
				//dtf = Windows::Globalization::DateTimeFormatting::DateTimeFormatter::LongDate::get();
				dtf = ref new Windows::Globalization::DateTimeFormatting::DateTimeFormatter(L"{dayofweek.full}, {month.integer(2)}/{day.integer(2)}/{year.full} {hour.integer}:{minute.integer(2)} {period.abbreviated}");
				//dtf = ref new Windows::Globalization::DateTimeFormatting::DateTimeFormatter(L"{dayofweek.full} {day.integer(2)} {month.full} {year.full} ");
				//dtf = ref new Windows::Globalization::DateTimeFormatting::DateTimeFormatter(L"{dayofweek.full} {day.integer(2)} {month.full} {year.full} ");
			else
				dtf = Windows::Globalization::DateTimeFormatting::DateTimeFormatter::ShortTime::get();

			return dtf->Format(dt); 
		}

		virtual Platform::Object^ ConvertBack(Platform::Object^ value,
			Windows::UI::Xaml::Interop::TypeName targetType,
			Platform::Object^ parameter,
			Platform::String^ language)
		{   
			throw ref new Platform::NotImplementedException();
		}
	};
}