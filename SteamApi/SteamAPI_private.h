#include "SteamAPI.h"

#include "SteamDataBaseStorage.h"
#include "http_client.h"
#include <agents.h>

#include <set>

namespace MyFirstAppUnitTests
{
	class SteamConnectionTests;
}

namespace SteamAPI
{
	typedef std::set<SteamID_t> SteamUserSet_t;

	// More specific smart pointer types
	typedef std::shared_ptr<class SteamConnection> PrivateSteamConnectionPtr;
	typedef std::weak_ptr<class SteamConnection> PrivateSteamConnectionWeakPtr;

	// Smart Pointer Types
	typedef std::shared_ptr<class SteamUser> PrivateSteamUserPtr;


	__interface ISteamDataConnection
	{
		// Requests
		virtual pplx::task<http::http_response> Login(const char *pszUserName, const char *pszPassword, const char *pszAuthCode) = 0;
		virtual pplx::task<http::http_response> SendUserMessage(const wchar_t *pwzSteamID, const wchar_t * pwzMessage) = 0;
		virtual pplx::task<http::http_response> ChatLogin() = 0;
		virtual pplx::task<http::http_response> GetMessages(int baseMessage, const wchar_t * pwzSteamID, bool fSecure) = 0;
		virtual pplx::task<http::http_response> GetUsersData(const wchar_t *pwzSteamIds) = 0;
		virtual pplx::task<http::http_response> GetFriendsList(const wchar_t *pwzSteamID) = 0;

		// Setters
		virtual void SetAccessToken(const wchar_t * pwzDataAccess) = 0;
		virtual void SetUMQID(int umqid) = 0;
	};

	typedef std::shared_ptr<ISteamDataConnection> SteamDataConnectionPtr;

	SteamDataConnectionPtr MakeSteamDataConnection(bool fFake);

	class CSteamDBConnection
	{
	public:
		static Util::CSQLite_Connection & GetInstance();
	private:
		Util::CSQLite_Connection m_db;
	};

	class SteamUserMessage : public ISteamUserMessage
	{
	public:
		virtual bool              GetFromMe() override { return m_fFromMe; }
		virtual SteamUserPtr      GetFromUser() override { return m_fromUser; }
		virtual const wchar_t*    GetMessage() override { return m_wstMessage.data(); }

		SteamUserMessage(SteamUserPtr & user, const wchar_t * pwzMessage, bool fFromMe);

		static SteamMessagePtr CreateSteamUserMessage(SteamUserPtr & user, const wchar_t * pwzMessage, bool fFromMe);
	private:

		bool           m_fFromMe;
		SteamUserPtr   m_fromUser;
		std::wstring   m_wstMessage;
	};

	class SteamConnection : public ISteamConnection, public std::enable_shared_from_this<SteamConnection>
	{
	public:
		SteamConnection(const wchar_t *pwzDatabaseFile, const SteamDataConnectionPtr & dataConnection);

		virtual concurrency::task<LoginResultPtr> Login(const char *pszUserName, const char *pszPassword, const char *pszAuthCode) override;
		virtual concurrency::task<FriendsList_t> GetFriendsListAsync() override;
		virtual concurrency::task<FriendsList_t> GetFriendsListAsync(const SteamID_t & pwszSteamId) override;
		virtual SteamUserPtr                     GetLoggedInUser() override;

		virtual concurrency::task<UserDataList_t> GetUsersDataAsync(const FriendsList_t & rgFriendsList) override;

		virtual IEvent<ISteamConnection, EConnectionStatus>& GetConnectionChangeEvent() override { return m_ConnectionChangeEvent; }

		virtual bool                    IsLoggedInToChat() override;
		virtual concurrency::task<int>  ChatLogin() override;
		virtual concurrency::task<void> GetMessages(int message, bool fSecure) override;

	public:
		// Not publicly advertised helper methods
		concurrency::task<bool> SendUserMessage(const wchar_t *pwzSteamID, const wchar_t * pwzMessage);

	private: // Functions
		bool VerifyLoggedIn();
		void NotifyConnectionStateChanged(EConnectionStatus status);

		void SetupDatabase();

		void SetupChatPolling();
		void PollMessages(int);

		void AddPendingUserLookup(const SteamID_t & steamID);
		void AddPendingUserLookup(const FriendsList_t & rgSteamIDs);

		void ProcessMessages(http::json::value & jsonResponse);

		const SteamUserSet_t & GetPendingUserLookupSet();

		PrivateSteamUserPtr GetUserPrivate(const SteamID_t & steamID);
		static FriendsList_t ParseGetFriendsListJson(http::json::value & jsonResponse);

		friend MyFirstAppUnitTests::SteamConnectionTests;

	private:
		Event<ISteamConnection, EConnectionStatus> m_ConnectionChangeEvent;

		std::wstring                       m_wstDatabaseFile;
		std::string                        m_stwUserName;
		//std::wstring                       m_wstAccessToken;
		SteamID_t                          m_PlayerSteamID;
		//int                                m_umqid;  //Messaging unique ID
		int                                m_MessagePollNumber;
		SteamUserSet_t                     m_setPendingUserLookups;
		std::map<SteamID_t, PrivateSteamUserPtr>  m_UserMap;  // Persistently store user objects
		std::shared_ptr<concurrency::timer<int>> m_MessagePollTimer;
		std::shared_ptr<concurrency::call<int>>  m_MessagePollCallback;
		SteamDataConnectionPtr             m_dataConnection;
	};


	class SteamUser : public ISteamUser, public std::enable_shared_from_this<SteamUser>
	{
	public:
		SteamUser(PrivateSteamConnectionPtr & steamConnection );
		~SteamUser(void){}

		virtual const wchar_t *                GetSteamID()        override { return m_SteamID.data(); }
		virtual const wchar_t *                GetPersonaName()    override { return m_PersonaName.data(); }
		//virtual const wchar_t *                GetProfileURI()     override { return m_ProfileURI.data(); }
		virtual const wchar_t *                GetAvatarImageURI() override { return m_AvatarImageURI.data(); }
		virtual OnlineStatus                   GetOnlineStatus()   override { return m_Status; }
		virtual time_t                         GetLastLogOffTime() override { return m_LastLogOff; }
		virtual std::tuple<bool, std::wstring> GetCurrentGame() override { return std::make_tuple(!m_CurrentGameID.empty(), m_CurrentGameName); }
		virtual SteamConversation_t            GetConversationHistory() override;
		virtual SteamConversationPtr           GetConversation() override;

		virtual SteamConnectionWeakPtr GetConnection() override { SteamConnectionWeakPtr xyz(m_connectionWeakPtr); return xyz; }
		virtual concurrency::task<bool> SendMessage(const wchar_t * pwzMessage) override;

		virtual IEvent<ISteamUser, int>& GetStatusChangedEvent() override { return m_StatusChangeEvent; }

		void SyncMessageFromNetwork(const wchar_t * pwzMessage, bool fromMe, time_t timestamp);

		void UpdateStatus(int newStatus);

		// Static utility methods
		static OnlineStatus ConvertIntToStatus(int nStatus);
		static PrivateSteamUserPtr CreateSteamUser(http::json::value & jsonPlayer, PrivateSteamConnectionPtr & steamConnection);

	private:
		// Miscellaneous property setters
		void SetOnlineStatus(OnlineStatus status) { m_Status = status; }
		void SetLastLogOffTime(time_t time) { m_LastLogOff = time; }

		// String property setters
		virtual void SetSteamID(const wchar_t * pwz)         { m_SteamID        = pwz; }
		virtual void SetPersonaName(const wchar_t * pwz)     { m_PersonaName    = pwz; }
		//virtual void SetProfileURI(const wchar_t * pwz)      { m_ProfileURI     = pwz; }
		virtual void SetAvatarImageURI(const wchar_t * pwz)  { m_AvatarImageURI = pwz; }
		virtual void SetCurrentGame(const wchar_t *pwzGameID, const wchar_t *pwzGameName) ;


		std::wstring m_SteamID;
		std::wstring m_PersonaName;
		std::wstring m_RealName;
		//std::wstring m_ProfileURI;
		std::wstring m_AvatarImageURI;
		std::wstring m_CurrentGameName;
		std::wstring m_CurrentGameID;

		OnlineStatus m_Status;
		time_t       m_LastLogOff;

		int          m_LastSignedInTime;

		PrivateSteamConnectionWeakPtr m_connectionWeakPtr;
		std::shared_ptr<class SteamConversation> m_conversation;

		Event<ISteamUser, int> m_StatusChangeEvent;
	};

}
