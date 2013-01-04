#pragma once

#include <ppltasks.h>
#include <functional>
#include "Event.h"

namespace SteamAPI
{
	// Basic Types
	typedef std::wstring SteamID_t;
	typedef std::vector<SteamID_t> FriendsList_t;

	// Smart Pointer Types
	typedef std::shared_ptr<__interface ISteamUser> SteamUserPtr;
	typedef std::weak_ptr<__interface ISteamUser> SteamUserWeakPtr;
	typedef std::shared_ptr<__interface ISteamConnection> SteamConnectionPtr;
	typedef std::weak_ptr<__interface ISteamConnection> SteamConnectionWeakPtr;
	typedef std::shared_ptr<__interface ISteamUserMessage> SteamMessagePtr;
	typedef std::vector<SteamMessagePtr> SteamConversation_t;
	typedef std::shared_ptr<__interface ISteamConversation> SteamConversationPtr;

	__interface ISteamUserMessage
	{
		virtual bool              GetFromMe() = 0;
		virtual SteamUserPtr      GetFromUser() = 0;
		virtual const wchar_t*    GetMessage() = 0;
	};

	__interface ISteamConversation
	{
		virtual SteamConversation_t GetMessages() = 0;
		virtual IEvent<ISteamConversation, int>& GetConversationChangedEvent() = 0;
		virtual int GetCount() = 0;
		virtual SteamMessagePtr GetMessage(int index) = 0;
	};

	__interface ISteamUser
	{
	public:	
		enum OnlineStatus 
		{
			Offline = 0,
			Online = 1,
			Busy = 2,
			Away = 3,
			Snooze = 4,
			LookingForTrade = 5,
			LookingToPlay = 6,
		};

		// Miscellaneous property setters
		virtual void SetOnlineStatus(OnlineStatus status) = 0;
		virtual void SetLastLogOffTime(time_t time) = 0;

		virtual const wchar_t * GetSteamID() = 0;
		virtual const wchar_t * GetPersonaName() = 0;
		//virtual const wchar_t * GetProfileURI() = 0;
		virtual const wchar_t * GetAvatarImageURI() = 0;
		virtual OnlineStatus    GetOnlineStatus() = 0;
		virtual time_t          GetLastLogOffTime() = 0;
		virtual std::tuple<bool, std::wstring> GetCurrentGame() = 0;

		virtual SteamConversation_t GetConversationHistory() = 0;
		virtual SteamAPI::SteamConversationPtr GetConversation() = 0;

		virtual concurrency::task<bool> SendMessage(const wchar_t * pwzMessage) = 0;

		virtual SteamConnectionWeakPtr GetConnection() = 0;

		virtual IEvent<ISteamUser, int>& GetStatusChangedEvent() = 0;
	};

	__interface ISteamFriendsList
	{
		virtual bool Refresh() = 0;
		virtual int  GetCount() = 0;
		virtual SteamUserPtr GetFriend(int index) = 0;
	};

	//class SteamFriendsList : public ISteamFriendsList
	//{
	//public:

	//private:
	//	std::map<SteamID_t, SteamUserPtr> m_users;
	//};

	typedef std::vector<SteamUserPtr> UserDataList_t;

	enum EConnectionStatus
	{
		Connected,
		Disconnected,
		Error
	};

	__interface ILoginResult
	{
		virtual bool GetLoginSuccessful() = 0;
		virtual const std::wstring & GetLoginError() = 0;
		virtual const std::wstring & GetLoginErrorCode() = 0;
		virtual const std::wstring & GetLoginErrorDescription() = 0;
	};

	typedef std::shared_ptr<ILoginResult> LoginResultPtr;

	__interface ISteamConnection
	{
	public:
		virtual concurrency::task<LoginResultPtr> Login(const char *pszUserName, const char *pszPassword, const char *pszAuthCode) = 0;

		virtual std::wstring            LookupSteamGuard(const wchar_t *pwzUserName);
		virtual void                    SaveSteamGuard(const wchar_t *pwzUserName, const wchar_t *pwzSteamGuard);
		virtual bool                    IsLoggedInToChat() = 0;
		virtual concurrency::task<int>  ChatLogin() = 0;
		virtual concurrency::task<void> GetMessages(int message, bool fSecure) = 0;

		virtual concurrency::task<UserDataList_t> GetUsersDataAsync(const FriendsList_t & rgFriendsList) = 0;
		virtual concurrency::task<FriendsList_t>  GetFriendsListAsync() = 0;
		virtual concurrency::task<FriendsList_t>  GetFriendsListAsync(const SteamID_t & pwszSteamId) = 0;

		virtual SteamUserPtr                      GetLoggedInUser() = 0;

		// Events
		virtual IEvent<ISteamConnection, EConnectionStatus>& GetConnectionChangeEvent() = 0;
	};

	// Global public constructors
	SteamConnectionPtr CreateSteamConnection(const wchar_t *pwzDatabaseFile, bool fFakeDataSource);

	SteamMessagePtr CreateSteamUserMessageX(SteamUserPtr & user, const wchar_t * pwzMessage, bool fFromMe);

} // namespace SteamAPI
