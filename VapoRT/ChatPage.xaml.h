//
// ChatPage.xaml.h
// Declaration of the ChatPage class
//

#pragma once

#include "Common\LayoutAwarePage.h" // Required by generated header
#include "Common\EnterKeyToCommand.h"
#include "Common\UpdateSourceHelper.h"
#include "ChatPage.g.h"
#include "ViewModel\SteamViewModel.h"

namespace VapoRT
{
	/// <summary>
	/// A page that displays a group title, a list of items within the group, and details for the
	/// currently selected item.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class ChatPage sealed
	{
	public:
		ChatPage();

	private:
		SteamDataVM^ m_SteamData;

	protected:
		virtual void LoadState(Platform::Object^ navigationParameter,
			Windows::Foundation::Collections::IMap<Platform::String^, Platform::Object^>^ pageState) override;
		virtual void SaveState(Windows::Foundation::Collections::IMap<Platform::String^, Platform::Object^>^ pageState) override;
		virtual void GoBack(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) override;
		virtual Platform::String^ DetermineVisualState(Windows::UI::ViewManagement::ApplicationViewState viewState) override;

	private:
		void ItemListView_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		bool UsingLogicalPageNavigation();
		bool UsingLogicalPageNavigation(Windows::UI::ViewManagement::ApplicationViewState viewState);
		void chatPageLoaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void ConversationListView_Loaded_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
