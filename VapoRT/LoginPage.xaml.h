//
// LoginPage.xaml.h
// Declaration of the LoginPage class
//

#pragma once

#include "Common\LayoutAwarePage.h" // Required by generated header
#include "LoginPage.g.h"

#include "ViewModel\SteamViewModel.h"

namespace VapoRT
{
	/// <summary>
	/// A basic page that provides characteristics common to most applications.
	/// </summary>
	public ref class LoginPage sealed
	{
	public:
		LoginPage();

	protected:
		virtual void LoadState(Platform::Object^ navigationParameter,
			Windows::Foundation::Collections::IMap<Platform::String^, Platform::Object^>^ pageState) override;
		virtual void SaveState(Windows::Foundation::Collections::IMap<Platform::String^, Platform::Object^>^ pageState) override;
	};
}
