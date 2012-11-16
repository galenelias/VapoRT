#include "pch.h"

#include <ctime>
#include <set>


#include "SteamAPI.h"
#include "SteamAPI_private.h"
#include "StandardUtils.h"

namespace SteamAPI
{
	const wchar_t HTTP_JSON_CONTENT_TYPE[]                = L"application/json";

	class FakeSteamDataConnection : public ISteamDataConnection
	{
	public:
		FakeSteamDataConnection();

		virtual pplx::task<http::http_response> Login(const char *pszUserName, const char *pszPassword, const char *pszAuthCode) override;
		virtual pplx::task<http::http_response> SendUserMessage(const wchar_t *pwzSteamID, const wchar_t * pwzMessage) override;
		virtual pplx::task<http::http_response> ChatLogin() override;
		virtual pplx::task<http::http_response> GetMessages(int baseMessage, const wchar_t * pwzSteamID, bool fSecure) override;
		virtual pplx::task<http::http_response> GetUsersData(const wchar_t * pwzSteamIds) override;
		virtual pplx::task<http::http_response> GetFriendsList(const wchar_t *pwzSteamID) override;

		virtual void SetAccessToken(const wchar_t * pwzDataAccess) override {}
		virtual void SetUMQID(int umqid) override {}

		static SteamDataConnectionPtr CreateFakeDataConnection();

	private:
		struct fake_friend_t
		{ 
			const wchar_t *pwzSteamID;
			const wchar_t *pwzPersona;
			const wchar_t *pwszAvatarUrl;
			bool           isFriend;
			int            personaState;
			time_t         friendSince;
		};
		static fake_friend_t friends[];

		class FakeFriend
		{
		public:
			std::wstring wstSteamId;
			std::wstring wstPersonaName;
			std::wstring wstAvatarUrl;
			bool         isFriend;
			int          personaState;
			time_t       friendSince;
		};

		std::unordered_map<std::wstring, FakeFriend>  m_userMap;
		int                                           m_pollMessage;

		void LoadFriendData();

		pplx::task<http::http_response> MakeFakeResponse(const wchar_t *pwzResponse, DWORD msDelay = 0);
	};

	FakeSteamDataConnection::FakeSteamDataConnection()
		: m_pollMessage(0)
	{
		LoadFriendData();
	}

	FakeSteamDataConnection::fake_friend_t FakeSteamDataConnection::friends[] = {
		//steamid               personaname,        avatar_url                                                                                                                                      fr?  S  friends_since
		{ L"76561198066311334", L"Corg"           , L"http:\\/\\/media.steampowered.com\\/steamcommunity\\/public\\/images\\/avatars\\/db\\/db165c0b3816930a3a15a5141aa79e096413987e_medium.jpg",  true, 4, 0}, 
		{ L"76561197995070795", L"Dahllm"         , L"http:\\/\\/media.steampowered.com\\/steamcommunity\\/public\\/images\\/avatars\\/59\\/59b25482ce56f2c0a91d050b259d1201ed94a433_medium.jpg",  true, 4, 1345265840}, 
		{ L"76561198039269170", L"loggum"         , L"http:\\/\\/media.steampowered.com\\/steamcommunity\\/public\\/images\\/avatars\\/7a\\/7ae537b6b7b7b09e68f52e76cdb8f0727f5cb270_medium.jpg",  true, 4, 1349595448}, 
		{ L"76561197972392315", L"Shaedyn"        , L"http:\\/\\/media.steampowered.com\\/steamcommunity\\/public\\/images\\/avatars\\/30\\/30cfd0a11450b1f137b1d7b60e6846ae5173773e_medium.jpg",  true, 0, 1333826881}, 
		{ L"76561198054881341", L"charles.elias"  , L"http:\\/\\/media.steampowered.com\\/steamcommunity\\/public\\/images\\/avatars\\/fe\\/fef49e7fa7e1997310d705b2a6158ff8dc1cdfeb_medium.jpg",  true, 4, 1331024236}, 
		{ L"76561197969962114", L"paradochs"      , L"http:\\/\\/media.steampowered.com\\/steamcommunity\\/public\\/images\\/avatars\\/0a\\/0af1d8c7e6db2c70694c51f2046432933727ae39_medium.jpg",  true, 4, 1346654501}, 
		{ L"76561198066347005", L"jofus101"       , L"http:\\/\\/media.steampowered.com\\/steamcommunity\\/public\\/images\\/avatars\\/fe\\/fef49e7fa7e1997310d705b2a6158ff8dc1cdfeb_medium.jpg",  true, 4, 1332219399}, 
		{ L"76561198037564684", L"Nahyo"          , L"http:\\/\\/media.steampowered.com\\/steamcommunity\\/public\\/images\\/avatars\\/27\\/27fa43fb17913a539e647b5db583b8a1987c048a_medium.jpg",  true, 4, 1341386230}, 
		{ L"76561198044516212", L"Anbu"           , L"http:\\/\\/media.steampowered.com\\/steamcommunity\\/public\\/images\\/avatars\\/42\\/428f9ad44653c6c9e9d00aee1c8fb66865819c6a_medium.jpg",  true, 1, 1331863922}, 
		{ L"76561197993369072", L"Fzsh!"          , L"http:\\/\\/media.steampowered.com\\/steamcommunity\\/public\\/images\\/avatars\\/4d\\/4dca26def1d6a06327903a4ee9af7940710d98e2_medium.jpg",  true, 4, 1324423924}, 
		{ L"76561198061854334", L"Darkhog"        , L"http:\\/\\/media.steampowered.com\\/steamcommunity\\/public\\/images\\/avatars\\/13\\/130bfce7199640313273d5c3f18e160f7b5689fa_medium.jpg",  true, 4, 1334372150}, 
		{ L"76561197992964399", L"blindai"        , L"http:\\/\\/media.steampowered.com\\/steamcommunity\\/public\\/images\\/avatars\\/63\\/639a3459ff856c7d93815838bc9ea7ae14d6edfd_medium.jpg",  true, 3, 1340496107}, 
		{ L"76561198066311335", L"slan85"         , L"http:\\/\\/media.steampowered.com\\/steamcommunity\\/public\\/images\\/avatars\\/e5\\/e58f219992893871d2bdd51252e492d6113f5709_medium.jpg",  true, 0, 1341117371}, 
		{ L"76561198062988124", L"TruffleShuffle" , L"http:\\/\\/media.steampowered.com\\/steamcommunity\\/public\\/images\\/avatars\\/68\\/689ca4bf7a99ac8ef2ca8c4a8a503fa483753cb3_medium.jpg",  true, 0, 1341382666}, 
		{ L"76561198001848589", L"Davian"         , L"http:\\/\\/media.steampowered.com\\/steamcommunity\\/public\\/images\\/avatars\\/3f\\/3f1404bc15d161db5746c9f3afd02f037ec77971_medium.jpg", false, 0, 0},
	};

	void FakeSteamDataConnection::LoadFriendData()
	{
		if (!m_userMap.empty())
			return;

		for (auto iter : friends)
		{
			FakeFriend newFriend;
			newFriend.wstSteamId     = iter.pwzSteamID;
			newFriend.wstPersonaName = iter.pwzPersona;
			newFriend.wstAvatarUrl   = iter.pwszAvatarUrl;
			newFriend.personaState   = iter.personaState;
			newFriend.friendSince    = iter.friendSince;
			newFriend.isFriend       = iter.isFriend;
			m_userMap[newFriend.wstSteamId] = newFriend;
		}
	}

	pplx::task<http::http_response> FakeSteamDataConnection::MakeFakeResponse(const wchar_t *pwzResponse, DWORD msDelay)
	{
		std::wstring wstResponse = pwzResponse;
		return pplx::create_task([wstResponse, msDelay]()
		{
			if (msDelay)
				WaitForSingleObjectEx(GetCurrentThread(), msDelay, TRUE);

			http::http_response httpResponse(http::status_codes::OK);
			httpResponse.set_body(wstResponse, HTTP_JSON_CONTENT_TYPE);
			return httpResponse;
		});

	}

	pplx::task<http::http_response> FakeSteamDataConnection::Login(const char *pszUserName, const char *pszPassword, const char *pszAuthCode)
	{
		return MakeFakeResponse( L"{ \"access_token\": \"5c9eeeab5e9e04fc1add09a45eab1d38\", \"token_type\": \"steam\", \"x_steamid\": \"76561198001848589\", \"x_webcookie\": \"5D59E310A52072405D7D8846D5D3E3C5F0F3B83E\" }" );
	}

	pplx::task<http::http_response> FakeSteamDataConnection::SendUserMessage(const wchar_t *pwzSteamID, const wchar_t * pwzMessage)
	{
		return MakeFakeResponse( L"{ }" );
	}

	pplx::task<http::http_response> FakeSteamDataConnection::ChatLogin()
	{
		return MakeFakeResponse( L"{ \"steamid\": \"76561198001848589\", \"error\": \"OK\", \"umqid\": \"9543\", \"timestamp\": 129536441, \"message\": 0, \"push\": 0}" );
	}

	pplx::task<http::http_response> FakeSteamDataConnection::GetMessages(int baseMessage, const wchar_t * pwzSteamID, bool fSecure)
	{
		http::json::value response  = http::json::value::object();
		response[L"pollid"]         = http::json::value::number(0);
		response[L"messagelast"]    = http::json::value::number(baseMessage);
		response[L"timestamp"]      = http::json::value::number(129537522);
		response[L"messagebase"]    = http::json::value::number(baseMessage);
		response[L"sectimeout"]     = http::json::value::number(1);
		response[L"error"]          = http::json::value::string(L"OK");

		http::json::value::element_vector messagesArray;
		//for (auto iFriend : m_userMap)
		{
			auto message          = http::json::value::object();
			message[L"type"]         = http::json::value::string(L"personastate");
			message[L"relationship"] = http::json::value::string(L"friend");
			message[L"steamid_from"] = http::json::value::string(L"76561197969962114");
			message[L"persona_name"] = http::json::value::string(L"Paradochs");
			message[L"persona_state"]= http::json::value::number(1);
			message[L"timestamp"]    = http::json::value::number(295315435);
			messagesArray.push_back(message);
		}
		response[L"messages"]          = http::json::value::array(messagesArray);

		m_pollMessage = baseMessage;

		return MakeFakeResponse( response.to_string().c_str(), 20000);
	}

	pplx::task<http::http_response> FakeSteamDataConnection::GetUsersData(const wchar_t * pwzSteamIds)
	{
		int i = 0;

		http::json::value friendArray = http::json::value::array(_countof(friends));
		for (auto iFriend : m_userMap)
		{
			friendArray[i] = http::json::value::object();
			friendArray[i][L"steamid"]      = http::json::value::string(iFriend.second.wstSteamId);
			friendArray[i][L"personaname"]  = http::json::value::string(iFriend.second.wstPersonaName);
			friendArray[i][L"avatarmedium"] = http::json::value::string(iFriend.second.wstAvatarUrl);
			friendArray[i][L"personastate"] = http::json::value::number(iFriend.second.personaState);
			friendArray[i][L"lastlogoff"]   = http::json::value::number((int)iFriend.second.friendSince);
			++i;
		}

		http::json::value response = http::json::value::object();
		response[L"players"] = friendArray;

		return MakeFakeResponse( response.to_string().c_str() );
	}

	pplx::task<http::http_response> FakeSteamDataConnection::GetFriendsList(const wchar_t *pwzSteamID)
	{
		http::json::value::element_vector friendArray;
		for (auto iFriend : m_userMap)
		{
			if (!iFriend.second.isFriend)
				continue;

			auto user = http::json::value::object();
			user[L"steamid"]      = http::json::value::string(casablanca::string_t(iFriend.second.wstSteamId));
			user[L"relationship"] = http::json::value::string(L"friend");
			user[L"friend_since"] = http::json::value::number((int)iFriend.second.friendSince);
			friendArray.push_back(user);
		}

		http::json::value response = http::json::value::object();
		response[L"friends"] = http::json::value::array(friendArray); //friendArray;

		return MakeFakeResponse( response.to_string().c_str() );
	}

	SteamDataConnectionPtr CreateFakeDataConnection()
	{
		return std::make_shared<FakeSteamDataConnection>();
	}

}