#pragma once

namespace VapoRT
{

	bool LookupEventToken(Windows::UI::Xaml::DependencyObject^ obj, Platform::String^ eventName, bool fRemove, Windows::Foundation::EventRegistrationToken *pToken);
	void RecordEventToken(Windows::UI::Xaml::DependencyObject^ obj, Platform::String^ eventName, Windows::Foundation::EventRegistrationToken token);

}