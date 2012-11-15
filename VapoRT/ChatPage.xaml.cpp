//
// ChatPage.xaml.cpp
// Implementation of the ChatPage class
//

#include "pch.h"
#include "ChatPage.xaml.h"

using namespace VapoRT;
using namespace VapoRT::Common;

using namespace concurrency;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Split Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234234

ChatPage::ChatPage()
{
	InitializeComponent();

	m_SteamData	= ref new SteamDataVM();

}

#pragma region Page state management

/// <summary>
/// Populates the page with content passed during navigation.  Any saved state is also
/// provided when recreating a page from a prior session.
/// </summary>
/// <param name="navigationParameter">The parameter value passed to
/// <see cref="Frame::Navigate(Type, Object)"/> when this page was initially requested.
/// </param>
/// <param name="pageState">A map of state preserved by this page during an earlier
/// session.  This will be null the first time a page is visited.</param>
void ChatPage::LoadState(Object^ navigationParameter, IMap<String^, Object^>^ pageState)
{
	// TODO: Set a bindable group using DefaultViewModel->Insert("Group", <value>)
	// TODO: Set a collection of bindable items using DefaultViewModel->Insert("Items", <value>)

	if (pageState == nullptr)
	{
		// When this is a new page, select the first item automatically unless logical page
		// navigation is being used (see the logical page navigation #region below.)
		if (!UsingLogicalPageNavigation() && itemsViewSource->View != nullptr)
		{
			itemsViewSource->View->MoveCurrentToFirst();
		}
	}
	else
	{
		// Restore the previously saved state associated with this page
		if (pageState->HasKey("SelectedItem") && itemsViewSource->View != nullptr)
		{
			// TODO: Invoke itemsViewSource->View->MoveCurrentTo() with the selected
			//       item as specified by the value of pageState->Lookup("SelectedItem")
		}
	}
}

/// <summary>
/// Preserves state associated with this page in case the application is suspended or the
/// page is discarded from the navigation cache.  Values must conform to the serialization
/// requirements of <see cref="SuspensionManager::SessionState"/>.
/// </summary>
/// <param name="pageState">An empty map to be populated with serializable state.</param>
void ChatPage::SaveState(IMap<String^, Object^>^ pageState)
{
	if (itemsViewSource->View != nullptr)
	{
		auto selectedItem = itemsViewSource->View->CurrentItem;
		// TODO: Derive a serializable navigation parameter and pass it to
		//       pageState->Insert("SelectedItem", <value>)
	}
}

#pragma endregion

#pragma region Logical page navigation

// Visual state management typically reflects the four application view states directly (full
// screen landscape and portrait plus snapped and filled views.)  The split page is designed so
// that the snapped and portrait view states each have two distinct sub-states: either the item
// list or the details are displayed, but not both at the same time.
//
// This is all implemented with a single physical page that can represent two logical pages.
// The code below achieves this goal without making the user aware of the distinction.

/// <summary>
/// Invoked to determine whether the page should act as one logical page or two.
/// </summary>
/// <returns>True when the current view state is portrait or snapped, false
/// otherwise.</returns>
bool ChatPage::UsingLogicalPageNavigation()
{
	return UsingLogicalPageNavigation(ApplicationView::Value);
}

/// <summary>
/// Invoked to determine whether the page should act as one logical page or two.
/// </summary>
/// <param name="viewState">The view state for which the question is being posed.</param>
/// <returns>True when the view state in question is portrait or snapped, false
/// otherwise.</returns>
bool ChatPage::UsingLogicalPageNavigation(ApplicationViewState viewState)
{
	return viewState == ApplicationViewState::FullScreenPortrait ||
		viewState == ApplicationViewState::Snapped;
}

/// <summary>
/// Invoked when an item within the list is selected.
/// </summary>
/// <param name="sender">The GridView (or ListView when the application is Snapped)
/// displaying the selected item.</param>
/// <param name="e">Event data that describes how the selection was changed.</param>
void ChatPage::ItemListView_SelectionChanged(Object^ sender, SelectionChangedEventArgs^ e)
{
	(void) sender;	// Unused parameter
	(void) e;	// Unused parameter

	// Invalidate the view state when logical page navigation is in effect, as a change in
	// selection may cause a corresponding change in the current logical page.  When an item is
	// selected this has the effect of changing from displaying the item list to showing the
	// selected item's details.  When the selection is cleared this has the opposite effect.
	if (UsingLogicalPageNavigation()) InvalidateVisualState();
}

/// <summary>
/// Invoked when the page's back button is pressed.
/// </summary>
/// <param name="sender">The back button instance.</param>
/// <param name="e">Event data that describes how the back button was clicked.</param>
void ChatPage::GoBack(Object^ sender, RoutedEventArgs^ e)
{
	//if (UsingLogicalPageNavigation() && itemListView->SelectedItem != nullptr)
	//{
	//	// When logical page navigation is in effect and there's a selected item that item's
	//	// details are currently displayed.  Clearing the selection will return to the item list.
	//	// From the user's point of view this is a logical backward navigation.
	//	itemListView->SelectedItem = nullptr;
	//}
	//else
	{
		// When logical page navigation is not in effect, or when there is no selected item, use
		// the default back button behavior.
		LayoutAwarePage::GoBack(sender, e);
	}
}

/// <summary>
/// Invoked to determine the name of the visual state that corresponds to an application view
/// state.
/// </summary>
/// <param name="viewState">The view state for which the question is being posed.</param>
/// <returns>The name of the desired visual state.  This is the same as the name of the view state
/// except when there is a selected item in portrait and snapped views where this additional
/// logical page is represented by adding a suffix of _Detail.</returns>
String^ ChatPage::DetermineVisualState(ApplicationViewState viewState)
{
	// Update the back button's enabled state when the view state changes
	//auto logicalPageBack = UsingLogicalPageNavigation(viewState) && itemListView->SelectedItem != nullptr;
	auto logicalPageBack = false;
	auto physicalPageBack = Frame != nullptr && Frame->CanGoBack;
	DefaultViewModel->Insert("CanGoBack", logicalPageBack || physicalPageBack);

	// Determine visual states for landscape layouts based not on the view state, but
	// on the width of the window.  This page has one layout that is appropriate for
	// 1366 virtual pixels or wider, and another for narrower displays or when a snapped
	// application reduces the horizontal space available to less than 1366.
	if (viewState == ApplicationViewState::Filled ||
		viewState == ApplicationViewState::FullScreenLandscape)
	{
		auto windowWidth = Window::Current->Bounds.Width;
		if (windowWidth >= 1366) return "FullScreenLandscapeOrWide";
		return "FilledOrNarrow";
	}

	// When in portrait or snapped start with the default visual state name, then add a
	// suffix when viewing details instead of the list
	String^ defaultStateName = LayoutAwarePage::DetermineVisualState(viewState);
	return logicalPageBack ? defaultStateName + "_Detail" : defaultStateName;
}
#pragma endregion


void VapoRT::ChatPage::chatPageLoaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	SteamAPI::SteamConnectionPtr steamConnection = safe_cast<App^>(App::Current)->ActiveConnection;;

	auto getFriendsTask = steamConnection->GetFriendsListAsync();
	getFriendsTask.then( [this, steamConnection](SteamAPI::FriendsList_t rgFriendIds) {
		auto task2 = steamConnection->GetUsersDataAsync(rgFriendIds);
		task2.then( [this, steamConnection](SteamAPI::UserDataList_t rgUserList){

			m_SteamData->Items->Clear();

			for (auto iterUser : rgUserList)
			{				
				m_SteamData->AddFriend(iterUser);
			}
			m_SteamData->SortByStatus();

			this->DataContext = m_SteamData;
		}, task_continuation_context::use_current());
	}, task_continuation_context::use_current());

}

void RecurseXamlElements(DependencyObject^ obj, std::vector<DependencyObject^> & allElements)
{
	int childCount = Windows::UI::Xaml::Media::VisualTreeHelper::GetChildrenCount(obj);
	for (int i = 0; i != childCount; ++i)
	{
		allElements.push_back(Windows::UI::Xaml::Media::VisualTreeHelper::GetChild(obj, i));
		RecurseXamlElements(allElements.back(), allElements);
	}
}

template <class T>
T FindXamlElement(DependencyObject^ obj)
{
	std::vector<DependencyObject^> rgElements;
	rgElements.push_back(std::move(obj));

	while (!rgElements.empty())
	{
		DependencyObject^ cur = rgElements.back();
		rgElements.erase(rgElements.end()-1);
		T t = dynamic_cast<T>(cur);
		if (t)
			return t;
		else
		{
			int childCount = Windows::UI::Xaml::Media::VisualTreeHelper::GetChildrenCount(cur);
			for (int i = 0; i != childCount; ++i)
			{
				auto ctrl = Windows::UI::Xaml::Media::VisualTreeHelper::GetChild(cur, i);
				rgElements.push_back(std::move(ctrl));
			}
		}
	}
	return nullptr;
}

void ScrollableHeightChanged(Windows::UI::Xaml::DependencyObject^ d, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e)
{
	if (e->NewValue)
	{
		// TODO: Only scroll to bottom when list was already scrolled to bottom
		//   http://mikaelkoskinen.net/post/WinRT-XAML-Automatically-Scrolling-ListView-to-Bottom-and-Detecting-When-ListView-is-Scrolled.aspx
		ScrollViewer^ scrollViewer = safe_cast<ScrollViewer^>(d);
		scrollViewer->ScrollToVerticalOffset(scrollViewer->ScrollableHeight); //Scroll to bottom
	}
	else
	{
		//assert(false);
	}
}


void RegisterForScrollableHeightChangeNotification(FrameworkElement^ element)
{
	Windows::UI::Xaml::Interop::TypeName objectType = Object::typeid;
	Windows::UI::Xaml::Interop::TypeName scrollViewerOwnerType = {ScrollViewer::typeid->FullName, Windows::UI::Xaml::Interop::TypeKind::Metadata };
	Windows::UI::Xaml::PropertyMetadata^ sourcePropertyMetadata = ref new PropertyMetadata(false, ref new Windows::UI::Xaml::PropertyChangedCallback(&ScrollableHeightChanged));

	static DependencyProperty^ _ScrollableHeightProperty = DependencyProperty::RegisterAttached("ScrollableHeightAttached", objectType, scrollViewerOwnerType, sourcePropertyMetadata);

	Binding^ binding = ref new Binding();
	binding->Source = element;
	binding->Path = ref new PropertyPath(L"ScrollableHeight");
	element->SetBinding(_ScrollableHeightProperty, binding);
}


void VapoRT::ChatPage::ConversationListView_Loaded_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	std::vector<DependencyObject^> rgAllElements;
	//RecurseXamlElements(ConversationListView, rgAllElements);

	ScrollViewer^ scrollViewer = FindXamlElement<ScrollViewer^>(ConversationListView);
	RegisterForScrollableHeightChangeNotification(scrollViewer);

}
