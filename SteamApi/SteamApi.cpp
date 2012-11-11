#include "pch.h"
#include "SteamApi.h"

#include "pch.h"

#include <ctime>
#include <set>


#include "SteamAPI.h"
#include "SteamAPI_private.h"
#include "StandardUtils.h"

using namespace Util;

namespace SteamAPI
{
	class SteamConversation : public ISteamConversation
	{
	public:
		SteamConversation(const std::shared_ptr<SteamUser> & user);

		virtual SteamConversation_t GetMessages() override;
		virtual IEvent<ISteamConversation, int>& GetConversationChangedEvent() override;
		virtual int GetCount() override;
		virtual SteamMessagePtr GetMessage(int index) override;

		void SyncMessage(const wchar_t * pwzMessage, bool fromMe);

	private:
		void RefreshMessages();
		void CreateChangeEvent(int x);
		Event<ISteamConversation, int> m_ConversationChangedEvent;

		SteamUserPtr                             m_User;
		SteamConversation_t                      m_ConversationItems;
		std::shared_ptr<concurrency::timer<int>> m_MessagePollTimer;
		std::shared_ptr<concurrency::call<int>>  m_MessagePollCallback;
	};


	/*static*/ CSQLite_Connection & CSteamDBConnection::GetInstance()
	{
		static CSQLite_Connection s_db;
		return s_db;
	}


	// Append friend ids into a single semi-colon delimited string
	template<class InputIterator>
	std::wstring ConcatenateSteamIds(InputIterator first, InputIterator last)
	{
		std::wstring wstFriendIds;
		std::for_each(first, last, [&wstFriendIds] (const SteamID_t & id)
		{
			if (!wstFriendIds.empty())
				wstFriendIds += L";";
			wstFriendIds += id;
		});

		return wstFriendIds;
	}

	///  SteamuserMessage ///

	SteamUserMessage::SteamUserMessage(SteamUserPtr & user, const wchar_t * pwzMessage, bool fFromMe)
		: m_fFromMe(fFromMe), m_wstMessage(pwzMessage), m_fromUser(user)
	{

	}

	SteamMessagePtr CreateSteamUserMessageX(SteamUserPtr & user, const wchar_t * pwzMessage, bool fFromMe)
	{
		return SteamUserMessage::CreateSteamUserMessage(user, pwzMessage, fFromMe);
	}

	/*static*/ SteamMessagePtr SteamUserMessage::CreateSteamUserMessage(SteamUserPtr & user, const wchar_t * pwzMessage, bool fFromMe)
	{
		return std::make_shared<SteamUserMessage>(user, pwzMessage, fFromMe);
	}

	/// SteamUser ///

	SteamUser::SteamUser(PrivateSteamConnectionPtr & steamConnection )
		: m_connectionWeakPtr(steamConnection)
	{

	}


	SteamConversation_t SteamUser::GetConversationHistory()
	{
		SteamConversation_t rgConversationHistory;

		CSQLite_Connection & db = CSteamDBConnection::GetInstance();

		std::string strQuery = FormatStr("SELECT SteamID, FromMe, Message FROM ConversationHistory WHERE SteamID=\"%S\" ORDER BY Sent ASC ", GetSteamID());
		CSQLite_Results results = db.FRunQuery(strQuery.data());

		for (int iRow = 0; iRow != results.NRows(); ++iRow)
		{
			std::wstring stwMessage = StrMultiToWide(results[iRow][2]);
			std::string strFromMe = results[iRow][1];

			SteamUserPtr userPtr = shared_from_this();
			SteamMessagePtr msg = SteamUserMessage::CreateSteamUserMessage(userPtr, stwMessage.data(), strFromMe == "True");
			rgConversationHistory.push_back(msg);
		}

		return rgConversationHistory;
	}

	SteamConversationPtr SteamUser::GetConversation()
	{
		if (!m_conversation)
			m_conversation = std::make_shared<SteamConversation>(shared_from_this());
		return m_conversation;
	}

	concurrency::task<bool> SteamUser::SendMessage(const wchar_t * pwzMessage)
	{
		PrivateSteamConnectionPtr spConnection = m_connectionWeakPtr.lock();

		if (!spConnection)
			throw std::exception("Unexpected");

		std::wstring wstrSteamID = m_SteamID;
		std::wstring wstrMessage = pwzMessage;

		time_t timestamp = time(nullptr);

		SyncMessageFromNetwork(pwzMessage, true, timestamp);

		return spConnection->ChatLogin().then([wstrMessage, wstrSteamID, spConnection](int)
		{
			return spConnection->SendUserMessage(wstrSteamID.data(), wstrMessage.data());
		});
	}

	void SteamUser::SyncMessageFromNetwork(const wchar_t * pwzMessage, bool fromMe, time_t timestamp)
	{
		CSQLite_Connection & db = CSteamDBConnection::GetInstance();

		std::wstring wstEscapedMessage = EscapeSQLString(pwzMessage);
		std::string strQuery = FormatStr("INSERT INTO ConversationHistory VALUES ('%S', '%s', '%S', '%d')", GetSteamID(), fromMe ? "True" : "False", wstEscapedMessage.c_str(), timestamp);
		CSQLite_Results results = db.FRunQuery(strQuery.data());

		if (m_conversation)
			m_conversation->SyncMessage(pwzMessage, fromMe);
	}

	void SteamUser::UpdateStatus(int newStatusInt)
	{
		OnlineStatus newStatus = ConvertIntToStatus(newStatusInt);
		if (GetOnlineStatus() != newStatus)
		{
			SetOnlineStatus(newStatus);

			m_StatusChangeEvent(this, 1 /*unused, property id?*/);
		}
	}

	/// SteamConnection ///

	void SteamConnection::AddPendingUserLookup(const SteamID_t & steamID)
	{
		m_setPendingUserLookups.insert(steamID);
	}

	void SteamConnection::AddPendingUserLookup(const FriendsList_t & rgSteamIDs)
	{
		for (auto& iter : rgSteamIDs)
			AddPendingUserLookup(iter);
	}

	const SteamUserSet_t & SteamConnection::GetPendingUserLookupSet()
	{
		return m_setPendingUserLookups;
	}

	concurrency::task<bool> SteamConnection::SendUserMessage(const wchar_t *pwzSteamID, const wchar_t * pwzMessage)
	{
		return concurrency::create_task([]()->bool
		{
			WaitForSingleObjectEx(GetCurrentThread(), 2000, TRUE);
			return true;
		});

		VerifyLoggedIn();

		WaitForSingleObjectEx(GetCurrentThread(), 2000, TRUE);

		auto reqTask = m_dataConnection->SendUserMessage(pwzSteamID, pwzMessage);
		auto thisPtr = shared_from_this();
		concurrency::task_completion_event<bool> taskCompetion;

		reqTask.then( [taskCompetion, thisPtr](http::http_response response)
		{
			if (response.status_code() == http::status_codes::OK)
			{
				http::json::value & jsonResponse = response.extract_json().get();
				bool success = jsonResponse[L"error"].as_string() == L"OK";
				taskCompetion.set(success);
			}
			else
			{
				taskCompetion.set(false);
			}
		}).then([taskCompetion](pplx::task<void> task)
		{
			try
			{
				task.get();
			}
			catch (...)
			{
				taskCompetion.set_exception(std::current_exception());
			}
		});

		return concurrency::create_task(taskCompetion);
	}

	/// SteamLoginResult ///

	class SteamLoginResult : public ILoginResult
	{
	public:
		virtual bool GetLoginSuccessful() override                           { return m_fSuccessful; }
		virtual const std::wstring & GetLoginError() override                { assert(!m_fSuccessful); return m_strError; }
		virtual const std::wstring & GetLoginErrorCode() override            { assert(!m_fSuccessful); return m_strErrorCode; }
		virtual const std::wstring & GetLoginErrorDescription() override     { assert(!m_fSuccessful); return m_strErrorDescription; }

		SteamLoginResult(bool fSuccessful);
		SteamLoginResult(bool fSuccessful, const std::wstring & strError, const std::wstring &strErrorCode, const std::wstring & strErrorDescription);
		static LoginResultPtr CreateSuccessfulSteamLoginResult();
		static LoginResultPtr CreateErrorSteamLoginResult(const std::wstring & strError, const std::wstring & strErrorCode, const std::wstring & strErrorDescription);

	private:
		bool          m_fSuccessful;
		std::wstring  m_strError;
		std::wstring  m_strErrorCode;
		std::wstring  m_strErrorDescription;
	};


	SteamLoginResult::SteamLoginResult(bool fSuccessful)
		: m_fSuccessful(fSuccessful)
	{	}


	SteamLoginResult::SteamLoginResult(bool fSuccessful, const std::wstring & strError, const std::wstring & strErrorCode, const std::wstring & strErrorDescription)
		: m_fSuccessful(fSuccessful)
		, m_strError(strError)
		, m_strErrorCode(strErrorCode)
		, m_strErrorDescription(strErrorDescription)
	{	}


	/*static*/ LoginResultPtr SteamLoginResult::CreateSuccessfulSteamLoginResult()
	{
		return std::make_shared<SteamLoginResult>(true);
	}


	/*static*/ LoginResultPtr SteamLoginResult::CreateErrorSteamLoginResult(const std::wstring & strError, const std::wstring & strErrorCode, const std::wstring & strErrorDescription)
	{
		return std::make_shared<SteamLoginResult>(false, strError, strErrorCode, strErrorDescription);
	}


	SteamConnectionPtr CreateSteamConnection(const wchar_t *pwzDatabaseFile, bool fFakeDataSource)
	{
		SteamDataConnectionPtr dataConnection = MakeSteamDataConnection(fFakeDataSource);

		return std::make_shared<SteamConnection>(pwzDatabaseFile, dataConnection);
	}

	SteamConnection::SteamConnection(const wchar_t *pwzDatabaseFile, const SteamDataConnectionPtr & dataConnection)
		: m_wstDatabaseFile(pwzDatabaseFile)
		, m_dataConnection(dataConnection)
		, m_MessagePollNumber(-1)
	{
		SetupDatabase();
	}

	void SteamConnection::PollMessages(int)
	{
		OutputDebugString(L"Timer\n");
		assert(m_MessagePollNumber != -1);

		GetMessages(0, false);
	}

	void SteamConnection::SetupDatabase()
	{
		CSQLite_Connection & db = CSteamDBConnection::GetInstance();
		db.FInitializeDatabase(StrWideToMulti(m_wstDatabaseFile.data()).data());

		bool fCreated = db.FCreateTable("ConversationHistory", "SteamID TEXT, FromMe BOOLEAN, Message TEXT, Sent DATETIME");
		fCreated = db.FCreateTable("UserTable", "SteamID TEXT, SteamGuard TEXT");
		assert(fCreated);
	}

	bool SteamConnection::IsLoggedInToChat()
	{
		return true;
	}

	void SteamConnection::SetupChatPolling()
	{
		m_MessagePollCallback = std::make_shared<concurrency::call<int>>([this](int v){PollMessages(v); });
		m_MessagePollTimer = std::make_shared<concurrency::timer<int>>(1000, 0, m_MessagePollCallback.get(), false);

		m_MessagePollTimer->start();
	}

	concurrency::task<int> SteamConnection::ChatLogin()
	{
		VerifyLoggedIn();

		auto reqTask = m_dataConnection->ChatLogin();
		auto thisPtr = shared_from_this();
		concurrency::task_completion_event<int> taskCompetion;

		reqTask.then( [taskCompetion, thisPtr](http::http_response response)
		{
			if (response.status_code() == http::status_codes::OK)
			{
				http::json::value & jsonResponse = response.extract_json().get();
				thisPtr->m_MessagePollNumber = jsonResponse[L"message"].as_integer();

				thisPtr->SetupChatPolling();

				taskCompetion.set(thisPtr->m_MessagePollNumber);
			}
			else
			{
				taskCompetion.set(-1);
			}
		}).then([taskCompetion](pplx::task<void> task)
		{
			try
			{
				task.get();
			}
			catch (...)
			{
				taskCompetion.set_exception(std::current_exception());
			}
		});

		//TODO: set_exception on task completion event
		return concurrency::create_task(taskCompetion);
	}

	concurrency::task<void> SteamConnection::GetMessages(int message, bool fSecure)
	{
		VerifyLoggedIn();

		if (message == 0)
			message = m_MessagePollNumber;

		auto reqTask = m_dataConnection->GetMessages(message, m_PlayerSteamID.c_str(), fSecure);
		auto thisPtr = shared_from_this();
		concurrency::task_completion_event<void> taskCompetion;

		reqTask.then( [taskCompetion, thisPtr, fSecure](http::http_response response)
		{
			if (response.status_code() == http::status_codes::OK)
			{
				http::json::value jsonResponse = response.extract_json().get();

				thisPtr->ProcessMessages(jsonResponse);

				if (JsonHasValue(jsonResponse, L"messagelast"))
					thisPtr->m_MessagePollNumber = jsonResponse[L"messagelast"].as_integer();

			}
			thisPtr->m_MessagePollTimer = std::make_shared<concurrency::timer<int>>(1000, 0, thisPtr->m_MessagePollCallback.get(), false);
			thisPtr->m_MessagePollTimer->start();

			taskCompetion.set();
		}).then([taskCompetion](pplx::task<void> task)
		{
			try
			{
				task.get();
			}
			catch (utilities::win32_exception &)
			{
				taskCompetion.set_exception(std::current_exception());
			}
			catch (std::exception &)
			{
				taskCompetion.set_exception(std::current_exception());
			}
			catch (...)
			{
				taskCompetion.set_exception(std::current_exception());
			}
		});

		//TODO: set_exception on task completion event
		return concurrency::create_task(taskCompetion);
	}

	void SteamConnection::ProcessMessages(http::json::value & jsonData)
	{
		if (!JsonHasValue(jsonData, L"messages"))
			return;

		http::json::value & messages = jsonData[L"messages"];

		for (auto message: messages.elements())
		{
			auto messageType = message[L"type"].as_string();

			time_t timeStamp = message[L"timestamp"].as_integer();
			std::wstring wstSteamId = message[L"steamid_from"].as_string();

			if (messageType == L"personastate")
			{
				int newPersonaState = message[L"persona_state"].as_integer();

				time_t timestamp = time(nullptr);
				if (JsonHasValue(message, L"timestamp"))
					timestamp = message[L"timestamp"].as_integer();

				SteamID_t steamID = message[L"steamid_from"].as_string();
				PrivateSteamUserPtr user = this->GetUserPrivate(steamID);

				if (!user)
					continue;

				user->UpdateStatus(message[L"persona_state"].as_integer());

				// Fake message for testing
				//std::wstring wstMessage = FormatWstr(L"%s updated persona state -> %d", message[L"persona_name"].as_string().c_str(), message[L"persona_state"].as_integer());
				//user->SyncMessageFromNetwork(wstMessage.c_str(), false, timestamp);
			}
			else if (messageType == L"typing")
			{

			}
			else if (messageType == L"saytext")
			{
				SteamID_t steamID = message[L"steamid_from"].as_string();
				PrivateSteamUserPtr user = this->GetUserPrivate(steamID);

				if (JsonHasValue(message, L"secure_message_id"))
					GetMessages(message[L"secure_message_id"].as_integer(), true);

				if (JsonHasValue(message, L"text"))
				{
					time_t timestamp = time(nullptr);
					if (JsonHasValue(message, L"timestamp"))
						timestamp = message[L"timestamp"].as_integer();

					user->SyncMessageFromNetwork(message[L"text"].as_string().c_str(), false, timestamp);
				}
			}
			else if (messageType == L"personarelationship")
			{
				SteamID_t steamID = message[L"steamid_from"].as_string();
				if (JsonHasValue(message, L"persona_state"))
					int personaState = message[L"persona_state"].as_integer();

			}
		}
	}

	concurrency::task<LoginResultPtr> SteamConnection::Login(const char *pszUserName, const char *pszPassword, const char *pszAuthCode)
	{
		concurrency::task_completion_event<LoginResultPtr> taskCompetion;
		auto reqTask = m_dataConnection->Login(pszUserName, pszPassword, pszAuthCode);
		auto thisPtr = shared_from_this();

		reqTask.then( [taskCompetion, thisPtr](http::http_response response)
		{
			WaitForSingleObjectEx(GetCurrentThread(), 2000, TRUE);
			if (response.status_code() == http::status_codes::OK)
			{
				http::json::value jsonResponse = response.extract_json().get();

				if (JsonHasValue(jsonResponse, L"x_steamid") && JsonHasValue(jsonResponse, L"access_token"))
				{
					// TODO: Synchronize
					thisPtr->m_PlayerSteamID = jsonResponse[L"x_steamid"].as_string();

					thisPtr->m_dataConnection->SetAccessToken(jsonResponse[L"access_token"].as_string().c_str());

					thisPtr->AddPendingUserLookup(thisPtr->m_PlayerSteamID);

					srand((int)time(NULL));
					thisPtr->m_dataConnection->SetUMQID(rand() % 100000);

					thisPtr->NotifyConnectionStateChanged(Connected);

					thisPtr->ChatLogin();

					taskCompetion.set(SteamLoginResult::CreateSuccessfulSteamLoginResult());
				}
				else if (JsonHasValue(jsonResponse, L"error"))
				{
					taskCompetion.set(SteamLoginResult::CreateErrorSteamLoginResult(jsonResponse[L"error"].as_string(), jsonResponse[L"x_errorcode"].as_string(), jsonResponse[L"error_description"].as_string()));
				}
				else
				{
					assert(false);
					taskCompetion.set(SteamLoginResult::CreateErrorSteamLoginResult(L"Unknown Error", L"xyz", L"."));
				}
			}
		}).then([taskCompetion](pplx::task<void> task)
		{
			try
			{
				task.get();
			}
			catch (...)
			{
				taskCompetion.set_exception(std::current_exception());
			}
		});

		//TODO: set_exception on task completion event
		return concurrency::create_task(taskCompetion);
	}

	void SteamConnection::NotifyConnectionStateChanged(EConnectionStatus status)
	{
		m_ConnectionChangeEvent(this, status);
	}

	bool SteamConnection::VerifyLoggedIn()
	{
		bool fLoginDetails = !m_PlayerSteamID.empty();// && !m_wstAccessToken.empty();
		assert(fLoginDetails);
		return fLoginDetails;
	}

	concurrency::task<UserDataList_t> SteamConnection::GetUsersDataAsync(const FriendsList_t & rgSteamIds)
	{
		AddPendingUserLookup(rgSteamIds);
		auto & lookupSet = GetPendingUserLookupSet();
		std::wstring wstFriendIds = ConcatenateSteamIds(begin(lookupSet), end(lookupSet));

		VerifyLoggedIn();

		auto reqTask = m_dataConnection->GetUsersData(wstFriendIds.c_str());
		auto thisPtr = shared_from_this();
		concurrency::task_completion_event<UserDataList_t> taskCompetion;

		reqTask.then( [taskCompetion, thisPtr, rgSteamIds](http::http_response response)
		{
			if (response.status_code() == http::status_codes::OK)
			{
				http::json::value jsonResponse = response.extract_json().get();
				UserDataList_t rgUserData;
				rgUserData.reserve(rgSteamIds.size());

				http::json::value & friendsValue = jsonResponse[L"players"];

				PrivateSteamConnectionPtr connectionPtr = thisPtr;

				auto players = friendsValue.elements();

				// Store all results in our user map
				for (auto& player : players)
				{
					PrivateSteamUserPtr steamUser = SteamUser::CreateSteamUser(player, connectionPtr);
					thisPtr->m_UserMap[steamUser->GetSteamID()] = std::move(steamUser);
				}

				// Fulfill the friend data request from values in our user map
				for (auto& steamID : rgSteamIds)
				{
					rgUserData.push_back(thisPtr->m_UserMap[steamID]);
				}

				taskCompetion.set(rgUserData);
			}
		}).then([taskCompetion](pplx::task<void> task)
		{
			try
			{
				task.get();
			}
			catch (...)
			{
				taskCompetion.set_exception(std::current_exception());
			}
		});

		return concurrency::create_task(taskCompetion);
	}

	PrivateSteamUserPtr SteamConnection::GetUserPrivate(const SteamID_t & steamID)
	{
		auto iter = m_UserMap.find(steamID);
		if (iter != end(m_UserMap))
			return iter->second;
		else
			return nullptr;
	}


	concurrency::task<FriendsList_t> SteamConnection::GetFriendsListAsync()
	{
		VerifyLoggedIn();
		return GetFriendsListAsync(m_PlayerSteamID);
	}

	concurrency::task<FriendsList_t> SteamConnection::GetFriendsListAsync(const SteamID_t & pwszSteamId)
	{
		auto reqTask = m_dataConnection->GetFriendsList(pwszSteamId.c_str());
		auto thisPtr = shared_from_this();
		concurrency::task_completion_event<FriendsList_t> taskCompetion;

		reqTask.then( [taskCompetion, thisPtr](http::http_response response)
		{
			if (response.status_code() == http::status_codes::OK)
			{
				http::json::value & jsonResponse = response.extract_json().get();
				taskCompetion.set(ParseGetFriendsListJson(jsonResponse));
			}
			else
			{
				taskCompetion.set(FriendsList_t());
			}
		}).then([taskCompetion](pplx::task<void> task)
		{
			try
			{
				task.get();
			}
			catch (...)
			{
				taskCompetion.set_exception(std::current_exception());
			}
		});

		//TODO: set_exception on task completion event
		return concurrency::create_task(taskCompetion);
	}


	SteamUserPtr SteamConnection::GetLoggedInUser()
	{
		assert(!m_PlayerSteamID.empty());
		auto iter = m_UserMap.find(m_PlayerSteamID);
		assert(iter != end(m_UserMap));

		if (iter != end(m_UserMap))
			return m_UserMap[m_PlayerSteamID];
		return nullptr;
	}

	/*static*/ FriendsList_t SteamConnection::ParseGetFriendsListJson(http::json::value & jsonResponse)
	{
		FriendsList_t        rgFriendIds;
		http::json::value&  friendsValue = jsonResponse[L"friends"];

		auto players = friendsValue.elements();
		for (auto player : players)
		{
			rgFriendIds.push_back(player[L"steamid"].as_string());
		}
		return std::move(rgFriendIds);
	}


	void SteamUser::SetCurrentGame(const wchar_t *pwzGameID, const wchar_t *pwzGameName)
	{
		m_CurrentGameID = pwzGameID;
		if (!m_CurrentGameID.empty())
			m_CurrentGameName = pwzGameName;
		else
			m_CurrentGameName.clear();
	}


	/*static*/ ISteamUser::OnlineStatus SteamUser::ConvertIntToStatus(int nStatus)
	{
		if (nStatus >= Offline && nStatus <= LookingToPlay)
			return (OnlineStatus)nStatus;
		else
			throw std::invalid_argument("");
	}

	/*static*/ PrivateSteamUserPtr SteamUser::CreateSteamUser(http::json::value & jsonPlayer, PrivateSteamConnectionPtr & steamConnection)
	{
		auto steamUser = std::make_shared<SteamUser>(steamConnection);

		steamUser->SetSteamID(jsonPlayer[L"steamid"].as_string().data());
		steamUser->SetPersonaName(jsonPlayer[L"personaname"].as_string().data());
		//steamUser->SetProfileURI(jsonPlayer[L"profileurl"].as_string().data());
		steamUser->SetAvatarImageURI(jsonPlayer[L"avatarmedium"].as_string().data());

		if (JsonHasValue(jsonPlayer, L"gameid"))
			steamUser->SetCurrentGame(jsonPlayer[L"gameid"].as_string().data(), JsonStringGetValueWithDefault(jsonPlayer, L"gameextrainfo", L"").data());

		steamUser->SetOnlineStatus(steamUser->ConvertIntToStatus((int)jsonPlayer[L"personastate"].as_integer() ));
		steamUser->SetLastLogOffTime((time_t) (int)jsonPlayer[L"lastlogoff"].as_integer() );

		return steamUser;
	}

	SteamConversation::SteamConversation(const std::shared_ptr<SteamUser> & user)
		: m_User(user)
	{
		//m_MessagePollCallback = std::make_shared<concurrency::call<int>>([this](int v){CreateChangeEvent(v); });
		//m_MessagePollTimer = std::make_shared<concurrency::timer<int>>(5000, 0, m_MessagePollCallback.get(), true);

		//m_MessagePollTimer->start();

		RefreshMessages();
	}

	void SteamConversation::CreateChangeEvent(int x)
	{
		std::wstring stwMessage = L"Blah";
		SteamAPI::SteamMessagePtr msg = SteamAPI::CreateSteamUserMessageX(m_User, stwMessage.data(), false);

		int insertIndex = m_ConversationItems.size();
		m_ConversationItems.push_back(msg);

		m_ConversationChangedEvent(this, insertIndex);
	}

	SteamConversation_t SteamConversation::GetMessages()
	{
		return SteamConversation_t();
	}

	IEvent<ISteamConversation, int>& SteamConversation::GetConversationChangedEvent()
	{
		return m_ConversationChangedEvent;
	}

	int SteamConversation::GetCount()
	{
		return m_ConversationItems.size();
	}

	SteamMessagePtr SteamConversation::GetMessage(int index)
	{
		return m_ConversationItems[index];
	}

	void SteamConversation::SyncMessage(const wchar_t * pwzMessage, bool fromMe)
	{
		SteamAPI::SteamMessagePtr msg = SteamAPI::CreateSteamUserMessageX(m_User, pwzMessage, fromMe);

		int insertIndex = m_ConversationItems.size();
		m_ConversationItems.push_back(msg);

		m_ConversationChangedEvent(this, insertIndex);
	}

	void SteamConversation::RefreshMessages()
	{
		SteamConversation_t rgConversationHistory;

		CSQLite_Connection & db = CSteamDBConnection::GetInstance();

		std::string strQuery = FormatStr("SELECT SteamID, FromMe, Message FROM ConversationHistory WHERE SteamID=\"%S\" ORDER BY Sent ASC ", m_User->GetSteamID());
		CSQLite_Results results = db.FRunQuery(strQuery.data());

		for (int iRow = 0; iRow != results.NRows(); ++iRow)
		{
			std::wstring stwMessage = StrMultiToWide(results[iRow][2]);
			std::string strFromMe = results[iRow][1];

			SteamUserPtr userPtr = m_User;
			SteamMessagePtr msg = SteamUserMessage::CreateSteamUserMessage(userPtr, stwMessage.data(), strFromMe == "True");
			rgConversationHistory.push_back(msg);
		}

		std::swap(m_ConversationItems, rgConversationHistory);
	}

	std::wstring SteamConnection::LookupSteamGuard(const wchar_t *pwzUserName) 
	{
		std::string strQuery = FormatStr("SELECT SteamGuard FROM UserTable WHERE SteamID=\"%S\"", pwzUserName);

		CSQLite_Connection & db = CSteamDBConnection::GetInstance();
		CSQLite_Results results = db.FRunQuery(strQuery.data());

		if (results.NRows() >= 1)
			return StrMultiToWide(results[0][0]);
		else
			return std::wstring();
	}

	void SteamConnection::SaveSteamGuard(const wchar_t *pwzUserName, const wchar_t *pwzSteamGuard)
	{
		std::string strQuery;

		if (LookupSteamGuard(pwzUserName) != L"")
			strQuery = FormatStr("UPDATE UserTable SET SteamGuard='%S' WHERE SteamID='%S'", pwzSteamGuard, pwzUserName);
		else
			strQuery = FormatStr("INSERT INTO UserTable VALUES ('%S', '%S')", pwzUserName, pwzSteamGuard);

		CSQLite_Connection & db = CSteamDBConnection::GetInstance();
		CSQLite_Results results = db.FRunQuery(strQuery.data());
	}


}  // End namespace SteamAPI
