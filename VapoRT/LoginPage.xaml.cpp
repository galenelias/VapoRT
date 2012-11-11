﻿//
// LoginPage.xaml.cpp
// Implementation of the LoginPage class
//

#include "pch.h"
#include "LoginPage.xaml.h"
#include "ChatPage.xaml.h"

using namespace VapoRT;

using namespace Platform;

using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Basic Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234237

LoginPage::LoginPage()
{
	InitializeComponent();

	SteamAPI::SteamConnectionPtr steamConnection = safe_cast<App^>(App::Current)->ActiveConnection;
	SteamConnectionVM^ connection = ref new SteamConnectionVM(steamConnection);

	this->DataContext = connection;

	Platform::WeakReference wrThis(this);
	auto dispatcher = Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher;

	m_autoConnectionChangeListener.AddListener(&steamConnection->GetConnectionChangeEvent(), [wrThis, dispatcher](SteamAPI::ISteamConnection * /*sender*/, SteamAPI::EConnectionStatus status) {
		dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([wrThis]() {
			auto page = wrThis.Resolve<LoginPage>();
			if (page)
				page->Frame->Navigate(Windows::UI::Xaml::Interop::TypeName(ChatPage::typeid));
		}));
	});

}

/// <summary>
/// Populates the page with content passed during navigation.  Any saved state is also
/// provided when recreating a page from a prior session.
/// </summary>
/// <param name="navigationParameter">The parameter value passed to
/// <see cref="Frame::Navigate(Type, Object)"/> when this page was initially requested.
/// </param>
/// <param name="pageState">A map of state preserved by this page during an earlier
/// session.  This will be null the first time a page is visited.</param>
void LoginPage::LoadState(Object^ navigationParameter, IMap<String^, Object^>^ pageState)
{
	(void) navigationParameter;	// Unused parameter
	(void) pageState;	// Unused parameter
}

/// <summary>
/// Preserves state associated with this page in case the application is suspended or the
/// page is discarded from the navigation cache.  Values must conform to the serialization
/// requirements of <see cref="SuspensionManager::SessionState"/>.
/// </summary>
/// <param name="pageState">An empty map to be populated with serializable state.</param>
void LoginPage::SaveState(IMap<String^, Object^>^ pageState)
{
	(void) pageState;	// Unused parameter
}
