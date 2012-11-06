#include "pch.h"

#include "SteamViewModel.h"

using namespace Platform;
using namespace Windows::UI::Core;
using namespace Util;
using namespace VapoRT::Common;

namespace VapoRT
{

inline Platform::String^ FormatDateRelative(Windows::Foundation::DateTime dt)
{
	Windows::Globalization::Calendar^ calendar = ref new Windows::Globalization::Calendar();
	calendar->SetToNow();

	LONGLONG dtDiff = calendar->GetDateTime().UniversalTime - dt.UniversalTime;

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
		wstDuration = FormatWstr(L"%d Days ago", dtDiff / c_ticksPerDay);
	else if (dtDiff > c_ticksPerHour)
		wstDuration = FormatWstr(L"%d Hours ago", dtDiff / c_ticksPerHour);
	else if (dtDiff > c_ticksPerMinute)
		wstDuration = FormatWstr(L"%d Minutes ago", dtDiff / c_ticksPerMinute);
	else
		wstDuration = L"%Just missed him/her!";

	std::wstring result = L"Last Online: ";
	result += wstDuration;
	return ref new Platform::String(result.data());
}


SteamConnectionVM::SteamConnectionVM(SteamAPI::SteamConnectionPtr connection)
	: m_model(connection)
{
	DoLoginCommand = ref new DelegateCommand(
		ref new ExecuteDelegate([this](Object^) { LoginAsync(UserName, Password); })
		);
	UserName = L"johndoe";
	Password = L"p4ssw0rd";
	SteamGuard = L"31337";
	FInputSteamGuard = false;
}

void SteamConnectionVM::LoginAsync(String^ UserName, String^ Password)
{
	Status = "";
	FConnecting = true;

	std::string username = StrWideToMulti(begin(UserName));
	std::string password = StrWideToMulti(begin(Password));
	std::string steamGuard = StrWideToMulti(begin(SteamGuard));
	auto loginTask = m_model->Login(username.data(), password.data(), steamGuard.data());

	loginTask.then([this](concurrency::task<SteamAPI::LoginResultPtr> t)
	{
		try
		{
			FConnecting = false;
			SteamAPI::LoginResultPtr loginResult = t.get();

			FConnected = loginResult->GetLoginSuccessful();
			if (!loginResult->GetLoginSuccessful())
			{
				std::wstring wstStatus = FormatWstr(L"Error: %s (%s)", loginResult->GetLoginErrorDescription().data(),  loginResult->GetLoginError().data());
				Status = ref new String(wstStatus.data());

				if (loginResult->GetLoginErrorCode() == L"invalid_steamguard_code"
					|| loginResult->GetLoginErrorCode() == L"steamguard_code_required")
				{
					FInputSteamGuard = true; // Ensure the user knows to enter a new steam-guard
				}
			}
			else
			{
				//m_model->ChatLogin().then([this](int lastMessage)
				//{
				//	return m_model->GetMessages(lastMessage, true).then([this, lastMessage]() 
				//	{
				//		return m_model->GetMessages(lastMessage, true);
				//	});
				//});
			}
		}
		catch (std::exception e)
		{
			OutputDebugString(L"std::exception hit:  ");
			OutputDebugStringA(e.what());
			OutputDebugString(L"\n");
		}
		catch (...)
		{
			OutputDebugString(L"generic exception hit\n");
		}
	}, concurrency::task_continuation_context::use_current());
}


VirtualConversationList::VirtualConversationList(const SteamAPI::SteamUserPtr & user, const SteamAPI::SteamConversationPtr & conversation)
	: m_User(user), m_ChatConversation(conversation)
{
	// Cache off the URI of the logged in user's profile image.
	SteamAPI::SteamConnectionWeakPtr connectionWeakPtr = user->GetConnection();
	SteamAPI::SteamConnectionPtr connectionPtr = connectionWeakPtr.lock();

	if (connectionPtr)
		m_LoggedInAvatarURI = ref new Windows::Foundation::Uri(ref new String(connectionPtr->GetLoggedInUser()->GetAvatarImageURI()));

	Platform::WeakReference wrThis(this);
	auto dispatcher = Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher;

	m_ConversationChangedEventListener.AddListener(&m_ChatConversation->GetConversationChangedEvent(), [wrThis, dispatcher](SteamAPI::ISteamConversation * /*sender*/, int notifIndex)
	{
		dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([wrThis, notifIndex]() {

			auto list = wrThis.Resolve<VirtualConversationList>();

			list->VectorChanged(list, ref new ConversationVectorChangedEventArgs(Windows::Foundation::Collections::CollectionChange::ItemInserted , notifIndex ));
		}));
	});

}
unsigned int VirtualConversationList::Size::get()
{
	return m_ChatConversation->GetCount();
}

Platform::Object^ VirtualConversationList::GetAt(unsigned int index)
{
	const SteamAPI::SteamMessagePtr & message = m_ChatConversation->GetMessage(index);
	ConversationItemVM^ convItem = ref new ConversationItemVM();
	convItem->FromMe = message->GetFromMe();
	convItem->Message = ref new Platform::String(FormatWstr(L"%s (%d)", message->GetMessage(), index).c_str());
	convItem->From = ref new Platform::String(m_User->GetPersonaName());

	if (message->GetFromMe())
		convItem->FromPic = m_LoggedInAvatarURI;
	else
		convItem->FromPic = ref new Windows::Foundation::Uri(ref new String(message->GetFromUser()->GetAvatarImageURI()));

	return convItem;
}

SteamUserVM::SteamUserVM(SteamAPI::SteamUserPtr user)
	: m_model(user)
{
	SendCurrentMessage = ref new DelegateCommand(
		ref new ExecuteDelegate([this](Object^) { SendConversationMessage(CurrentMessage); })
		);

	Platform::WeakReference wrThis(this);
	auto dispatcher = Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher;
	m_StatusChangedEventListener.AddListener(&m_model->GetStatusChangedEvent(), [wrThis, dispatcher] (SteamAPI::ISteamUser * /*sender*/, int)
	{
		dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([wrThis]() {
			auto userVM = wrThis.Resolve<SteamUserVM>();

			userVM->OnPropertyChanged(L"StatusString");
			userVM->OnPropertyChanged(L"OnlineStatus");
			userVM->OnPropertyChanged(L"InGame");
			userVM->OnPropertyChanged(L"StatusColor");
		}));	
	});
}

Windows::UI::Xaml::Interop::IBindableObservableVector^ SteamUserVM::ConversationHistory::get()
{
	if (!_ConversationHistory)
		_ConversationHistory = ref new VirtualConversationList(m_model, m_model->GetConversation());
	return _ConversationHistory;
}

void SteamUserVM::SendConversationMessage(String^ message)
{
	SendingMessage = true;
	auto sendTask = m_model->SendMessage(begin(message));

	sendTask.then([this](bool success)
	{
		CurrentMessage = L"";
		SendingMessage = false;
	}, concurrency::task_continuation_context::use_current());
}

Platform::String^ SteamUserVM::StatusString::get()
{
	auto gameStatus = m_model->GetCurrentGame();
	if (std::get<0>(gameStatus))
	{
		std::wstring result;
		if (!std::get<1>(gameStatus).empty())
			result = FormatWstr(L"In-Game: %s", std::get<1>(gameStatus).data());
		else
			result = L"In-Game";

		if (m_model->GetOnlineStatus() != SteamAPI::ISteamUser::Online)
				result = FormatWstr(L"%s (%s)", result.data(), GetStatusString(m_model->GetOnlineStatus()));
		return ref new String(result.data());
	}

	switch (m_model->GetOnlineStatus())
	{
	case SteamAPI::ISteamUser::Offline:
		return FormatDateRelative(LastLogOffTime);
	case SteamAPI::ISteamUser::Online:
	case SteamAPI::ISteamUser::Busy:
	case SteamAPI::ISteamUser::Away:
	case SteamAPI::ISteamUser::Snooze:
		return ref new Platform::String(GetStatusString(m_model->GetOnlineStatus()));
	default:
		return "Unknown State";
	}
}

/*static*/ wchar_t * SteamUserVM::GetStatusString(SteamAPI::ISteamUser::OnlineStatus status)
{
	switch (status)
	{
	case SteamAPI::ISteamUser::Offline:
		return L"Offline";
	case SteamAPI::ISteamUser::Online:
		return L"Online";
	case SteamAPI::ISteamUser::Busy:
		return L"Busy";
	case SteamAPI::ISteamUser::Away:
		return L"Away";
	case SteamAPI::ISteamUser::Snooze:
		return L"Snooze";
	default:
		return L"Unknown State";
	}
}

Windows::UI::Xaml::Media::Brush^ SteamUserVM::StatusColor::get()
{
	const Windows::UI::Color colorOffline = {255,128,128,128};
	const Windows::UI::Color colorOnline =  {255,142,202,252};
	const Windows::UI::Color colorInGame =  {255,167,212,107};

	auto gameStats = m_model->GetCurrentGame();
	SteamAPI::ISteamUser::OnlineStatus onlineStatus = m_model->GetOnlineStatus();
	if (std::get<0>(gameStats))
		return ref new Windows::UI::Xaml::Media::SolidColorBrush(colorInGame);
	else if (onlineStatus == SteamAPI::ISteamUser::Offline)
		return ref new Windows::UI::Xaml::Media::SolidColorBrush(colorOffline);
	else
		return ref new Windows::UI::Xaml::Media::SolidColorBrush(colorOnline);
}

void SteamDataVM::AddFriend(SteamAPI::SteamUserPtr & user)
{
	SteamUserVM^ steamUserVM = CreateUserVMFromUser(user);
	Items->Append(steamUserVM);

	Platform::WeakReference wrThis(this);
	auto dispatcher = Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher;
	m_UserStatusChangedEventListener.AddListener(&user->GetStatusChangedEvent(), [wrThis, dispatcher] (SteamAPI::ISteamUser * /*sender*/, int)
	{
		dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([wrThis]() {
			auto dataVM = wrThis.Resolve<SteamDataVM>();

			dataVM->SortByStatus();
		}));	
	});

}

void SteamDataVM::SortByStatus()
{
	std::sort(begin(Items), end(Items), [](SteamUserVM^ steamUser1, SteamUserVM^ steamUser2) {
		if (steamUser1->OnlineStatus != steamUser2->OnlineStatus)
		{
			if (steamUser1->OnlineStatus == 0)
				return false;
			else if (steamUser2->OnlineStatus == 0)
				return true;
			else
				return steamUser1->OnlineStatus < steamUser2->OnlineStatus;
		}
		else if (steamUser1->OnlineStatus == 0) // Both offline
		{
			return steamUser1->LastLogOffTime.UniversalTime > steamUser2->LastLogOffTime.UniversalTime;
		}
		else if (steamUser1->InGame != steamUser2->InGame)
		{
			return steamUser1->InGame;
		}
		else
		{
			return wcscmp(steamUser1->PersonaName->Begin(), steamUser2->PersonaName->Begin()) < 0;
		}
	});
}

SteamUserVM^ CreateUserVMFromUser(SteamAPI::SteamUserPtr & user)
{
	return ref new SteamUserVM(user);
}

}