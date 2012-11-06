#include "pch.h"

#include <ctime>
#include <set>


#include "SteamAPI.h"
#include "SteamAPI_private.h"
#include "StandardUtils.h"

using namespace Util;

namespace SteamAPI
{
	const wchar_t STEAM_GET_OAUTH_FRIENDS_LIST_URI_FMT[] = L"https://api.steampowered.com/ISteamUserOAuth/GetFriendList/v0001/?access_token=%s&steamid=%s";
	const wchar_t STEAM_GET_OAUTH_SUMARRIES_URI_FMT[]    = L"https://api.steampowered.com/ISteamUserOAuth/GetUserSummaries/v0001/?access_token=%s&steamids=%s";

	const wchar_t STEAM_POST_OAUTH_LOGIN_URI[]           = L"https://api.steampowered.com/ISteamOAuth2/GetTokenWithCredentials/v0001/";
	const wchar_t STEAM_POST_OAUTH_CHAT_LOGON_URI[]      = L"https://api.steampowered.com/ISteamWebUserPresenceOAuth/Logon/v0001/";
	const wchar_t STEAM_POST_OAUTH_POLL_URI[]            = L"https://api.steampowered.com/ISteamWebUserPresenceOAuth/Poll/v0001/";
	const wchar_t STEAM_POST_OAUTH_SEND_MESSAGE_URI[]    = L"https://api.steampowered.com/ISteamWebUserPresenceOAuth/Message/v0001/";
	const wchar_t STEAM_POST_OAUTH_POLLSTATUS_URI[]      = L"http://api.steampowered.com/ISteamWebUserPresenceOAuth/PollStatus/v0001/";


	const char STEAM_LOGIN_POST_DATA_FMT_SZ[]             = "client_id=DE45CD61&grant_type=password&username=%s&password=%s&scope=read_profile%%20write_profile%%20read_client%%20write_client&x_emailauthcode=%s";  //JHQ8D
	const char STEAM_CHAT_LOGIN_POST_DATA_FMT_SZ[]        = "access_token=%S&umqid=%d";
	const char STEAM_CHAT_POLL_POST_DATA_FMT_SZ[]         = "access_token=%S&steamid=%S&umqid=%d&message=%d";
	const char STEAM_CHAT_POLLSTATUS_POST_DATA_FMT_SZ[]   = "steamid=%S&umqid=%d&message=%d";
	const char STEAM_CHAT_SEND_MESSAGE_POST_DATA_FMT_SZ[] = "access_token=%S&umqid=%d&type=%s&text=%S&steamid_dst=%S";

	class RealSteamDataConnection : public ISteamDataConnection
	{
	public:
		RealSteamDataConnection();

		virtual pplx::task<http::http_response> Login(const char *pszUserName, const char *pszPassword, const char *pszAuthCode) override;
		virtual pplx::task<http::http_response> SendUserMessage(const wchar_t *pwzSteamID, const wchar_t * pwzMessage) override;
		virtual pplx::task<http::http_response> ChatLogin() override;
		virtual pplx::task<http::http_response> GetMessages(int baseMessage, const wchar_t * pwzSteamID, bool fSecure) override;
		virtual pplx::task<http::http_response> GetUsersData(const wchar_t * pwzSteamIds) override;
		virtual pplx::task<http::http_response> GetFriendsList(const wchar_t *pwzSteamID) override;

		virtual void SetAccessToken(const wchar_t * pwzDataAccess) override;
		virtual void SetUMQID(int umqid) override;

	private:
		http::http_request CreateHttpPostRequest(const std::string & stPostData);
		static void        SetSteamAPIHeaders(http::http_headers & headers);
		const wchar_t *    GetAccessToken();
		int                GetUMQID();

	private:
		std::wstring  m_wstAccessToken;
		int           m_umqid;
	};


	RealSteamDataConnection::RealSteamDataConnection()
		: m_umqid(-1)
	{

	}

	void RealSteamDataConnection::SetAccessToken(const wchar_t * pwzDataAccess)
	{
		assert(m_wstAccessToken.empty()); // We shouldn't set this twice
		m_wstAccessToken = pwzDataAccess;
	}

	const wchar_t * RealSteamDataConnection::GetAccessToken()
	{
		assert(!m_wstAccessToken.empty()); // We better have an access token
		return m_wstAccessToken.c_str();
	}

	void RealSteamDataConnection::SetUMQID(int umqid)
	{
		m_umqid = umqid;
	}

	int RealSteamDataConnection::GetUMQID()
	{
		return m_umqid;
	}

	http::http_request RealSteamDataConnection::CreateHttpPostRequest(const std::string & stPostData)
	{
		http::http_request request(http::methods::POST);
		SetSteamAPIHeaders(request.headers());
		request.set_body_utf8(stPostData);

		return request;
	}

	// Free floating utility functions:
	void RealSteamDataConnection::SetSteamAPIHeaders(http::http_headers & headers)
	{
		headers.add(L"User-Agent", L"Steam 1291812 / iPhone");
		headers.add(L"Content-Type", L"application/x-www-form-urlencoded");
	}


	pplx::task<http::http_response> RealSteamDataConnection::Login(const char *pszUserName, const char *pszPassword, const char *pszAuthCode)
	{
		std::string strPostData = FormatStr(STEAM_LOGIN_POST_DATA_FMT_SZ, pszUserName, pszPassword, pszAuthCode);

		http::client::http_client  client(STEAM_POST_OAUTH_LOGIN_URI);
		http::http_request         request = CreateHttpPostRequest(strPostData);

		// Successful response:      { "access_token": "5c9eeeab5e9e04fc1add09a45eab1d38", "token_type": "steam", "x_steamid": "76561198001848589", "x_webcookie": "9F5FDF4BE28D831B4E49B502C6E7DE8161F08236" }
		// Possible error response:  { "error": "access_denied", "error_description": "Incorrect login", "x_errorcode": "incorrect_login" }
		// Steamguard err response:  { "error": "access_denied", "error_description": "Invalid Steam guard authentication code", "x_errorcode": "invalid_steamguard_code" }
		//                           { "error": "access_denied", "error_description": "Steam guard email authentication code required", "x_errorcode": "steamguard_code_required" }

		return client.request(request);
	}

	pplx::task<http::http_response> RealSteamDataConnection::SendUserMessage(const wchar_t *pwzSteamID, const wchar_t * pwzMessage)
	{
		std::string strPostData;
		const char szSendMessageType[] = "saytext";

		strPostData = FormatStr(STEAM_CHAT_SEND_MESSAGE_POST_DATA_FMT_SZ, GetAccessToken(), GetUMQID(), szSendMessageType, UrlEncodeString(pwzMessage).c_str(), pwzSteamID);

		http::client::http_client  client(STEAM_POST_OAUTH_SEND_MESSAGE_URI);
		http::http_request         request = CreateHttpPostRequest(strPostData);

		return client.request(request);
	}

	pplx::task<http::http_response> RealSteamDataConnection::ChatLogin()
	{
		std::string strPostData = FormatStr(STEAM_CHAT_LOGIN_POST_DATA_FMT_SZ, GetAccessToken(), GetUMQID());

		http::client::http_client  client(STEAM_POST_OAUTH_CHAT_LOGON_URI);
		http::http_request         request = CreateHttpPostRequest(strPostData);

		return client.request(request);
	}

	pplx::task<http::http_response> RealSteamDataConnection::GetMessages(int baseMessage, const wchar_t * pwzSteamID, bool fSecure)
	{
		std::string strPostData;

		if (fSecure)
			strPostData = FormatStr(STEAM_CHAT_POLL_POST_DATA_FMT_SZ, GetAccessToken(), pwzSteamID, GetUMQID(), baseMessage);
		else
			strPostData = FormatStr(STEAM_CHAT_POLLSTATUS_POST_DATA_FMT_SZ, pwzSteamID, GetUMQID(), baseMessage);

		http::client::http_client  client(fSecure ? STEAM_POST_OAUTH_POLL_URI : STEAM_POST_OAUTH_POLLSTATUS_URI);
		http::http_request         request = CreateHttpPostRequest(strPostData);

		return client.request(request);
	}

	pplx::task<http::http_response> RealSteamDataConnection::GetUsersData(const wchar_t * pwzSteamIds)
	{
		std::wstring wstUri = FormatWstr(SteamAPI::STEAM_GET_OAUTH_SUMARRIES_URI_FMT, GetAccessToken(), pwzSteamIds);

		http::client::http_client client(wstUri);
		http::http_request request(http::methods::GET);
		SetSteamAPIHeaders(request.headers());

		return client.request(request);
	}

	pplx::task<http::http_response> RealSteamDataConnection::GetFriendsList(const wchar_t *pwzSteamID)
	{
		std::wstring wstUri = FormatWstr(STEAM_GET_OAUTH_FRIENDS_LIST_URI_FMT, GetAccessToken(), pwzSteamID);

		http::client::http_client client(wstUri);
		http::http_request request(http::methods::GET);
		SetSteamAPIHeaders(request.headers());

		return client.request(request);
	}


	// Forward declarations:
	SteamDataConnectionPtr CreateFakeDataConnection();

	SteamDataConnectionPtr MakeSteamDataConnection(bool fFake)
	{

		if (fFake)
			return CreateFakeDataConnection();
		else
			return std::make_shared<RealSteamDataConnection>();

	}

}