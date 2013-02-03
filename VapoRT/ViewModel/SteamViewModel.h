#pragma once

#include "Common\DelegateCommand.h"
#include "Common\ImmersiveUtils.h"

#define MAKE_PROP(type, var)  \
	private: \
	type _##var; \
	public: \
	property type var \
	{ \
		type get() \
		{ return _##var; } \
		void set(type value) \
		{ _##var = value; OnPropertyChanged(#var); 	} \
	}


namespace VapoRT
{
	[Windows::UI::Xaml::Data::Bindable]
	public ref class ConversationItemVM sealed
	{
	public:
		ConversationItemVM();

		property Platform::String^  Message;
		property Platform::String^  From;
		property bool     FromMe;
		property Windows::Foundation::DateTime SentTime;
		property Windows::Foundation::Uri^ FromPic;
	};

	[Windows::UI::Xaml::Data::Bindable]
	public ref class SteamConnectionVM sealed : public Common::BindableBase
	{
		Platform::String^ m_UserName;
		Platform::String^ m_Password;
		Platform::String^ m_SteamGuard;
		Platform::String^ m_Status;

	public:
		SteamConnectionVM() {}  // For Design Time Data

		MAKE_PROP(bool, FConnected);
		MAKE_PROP(bool, FConnecting);
		MAKE_PROP(bool, FInputSteamGuard);

		property Platform::String^ UserName
		{
			Platform::String^ get() { return m_UserName; }
			void set(Platform::String^ value)
			{
				m_UserName = value;
				OnPropertyChanged("UserName");
			}
		}
		property Platform::String^ Password
		{
			Platform::String^ get() { return m_Password; }
			void set(Platform::String^ value)
			{
				m_Password = value;
				OnPropertyChanged("Password");
			}
		}
		property Platform::String^ SteamGuard
		{
			Platform::String^ get() { return m_SteamGuard; }
			void set(Platform::String^ value)
			{
				m_SteamGuard = value;
				OnPropertyChanged("SteamGuard");
			}
		}
		property Platform::String^ Status
		{
			Platform::String^ get() { return m_Status; }
			void set(Platform::String^ value)
			{
				m_Status = value;
				OnPropertyChanged("Status");
			}
		}

		property bool LoginButtonEnabled
		{
			bool get() { return !FConnecting; }
		}

		property Common::DelegateCommand^ DoLoginCommand;
	
	internal:
		SteamConnectionVM(SteamAPI::SteamConnectionPtr connection);

		void LoginAsync(Platform::String^ UserName, Platform::String^ Password);

	private:
		void LoadSavedCredentials();
		void ClearSavedCredentials();
		void SaveCredentials();

		SteamAPI::SteamConnectionPtr m_model;
	};


	[Windows::UI::Xaml::Data::Bindable]
	public ref class SteamUserDesignVM sealed : public Common::BindableBase
	{
	public:
		SteamUserDesignVM()
		{
			ConversationHistory = ref new Platform::Collections::Vector<ConversationItemVM^>();
		}

		property Platform::String^ SteamID;
		property Platform::String^ PersonaName;
		property Windows::Foundation::DateTime LastLogOffTime;
		property Platform::String^ StatusString;
		property int OnlineStatus;
		property bool InGame;
		property Windows::UI::Xaml::Media::Brush^ StatusColor;
		property Windows::Foundation::Uri^ AvatarImageURI;
		property Windows::Foundation::Uri^ ProfileURI;

		property Windows::Foundation::Collections::IVector<ConversationItemVM^>^ ConversationHistory;
	};

	[Windows::UI::Xaml::Data::Bindable]
	public ref class ConversationDataTemplateSelector sealed : Windows::UI::Xaml::Controls::DataTemplateSelector
	{
	public:
		ConversationDataTemplateSelector() {}

		property Windows::UI::Xaml::DataTemplate^ ToMeTemplate;
		property Windows::UI::Xaml::DataTemplate^ FromMeTemplate;

		virtual Windows::UI::Xaml::DataTemplate^ SelectTemplateCore(Object^ item, Windows::UI::Xaml::DependencyObject^ container) override
		{
			ConversationItemVM^ convItem = safe_cast<ConversationItemVM^>(item);
			Windows::UI::Xaml::DataTemplate^ dt = convItem->FromMe ? FromMeTemplate : ToMeTemplate; //: this.CustTemplate;
			return dt;
		}
	};

    [Windows::UI::Xaml::Data::Bindable]
    public ref class SteamUserVM sealed : public Common::BindableBase
    {
	private:
		Platform::String^ _CurrentMessage;
    public:
        property Platform::String^ SteamID
		{	Platform::String^ get() { return ref new Platform::String(m_model->GetSteamID()); } }

		property Platform::String^ PersonaName
		{	Platform::String^ get() { return ref new Platform::String(m_model->GetPersonaName()); } }

		//property Windows::Foundation::Uri^ ProfileURI
		//{	Windows::Foundation::Uri^ get() { return ref new Windows::Foundation::Uri(ref new String(m_model->GetProfileURI())); } }

		property Windows::Foundation::Uri^ AvatarImageURI
		{	Windows::Foundation::Uri^ get() { return ref new Windows::Foundation::Uri(ref new Platform::String(m_model->GetAvatarImageURI())); } }

		property Windows::Foundation::DateTime LastLogOffTime
		{
			Windows::Foundation::DateTime get() { 
				return UnixTimeToDateTime(m_model->GetLastLogOffTime());
			}
		}

		property Platform::String^ StatusString
		{	Platform::String^ get(); };

		property int OnlineStatus
		{	int get() { return m_model->GetOnlineStatus(); } 	};

		property bool InGame
		{	bool get() { return std::get<0>(m_model->GetCurrentGame());	} }

		property Windows::UI::Xaml::Media::Brush^ StatusColor {
			Windows::UI::Xaml::Media::Brush^ get();
		};

		property Windows::UI::Xaml::Interop::IBindableObservableVector^ ConversationHistory
		{
			Windows::UI::Xaml::Interop::IBindableObservableVector^ get();
		}


		property Platform::String^ CurrentMessage
		{
			Platform::String^ get() { return _CurrentMessage; }
			void set(Platform::String^ value)
			{
				_CurrentMessage = value;
				OnPropertyChanged(L"CurrentMessage");

				// Ability to send current message depends on whether the Current Message is empty or not - must manually raise event to signal a change in CanExecute state
				SendCurrentMessage->RaiseCanExecuteChanged();  
			}
		}

		// Send message
		property Common::DelegateCommand^ SendCurrentMessage;
		MAKE_PROP(bool, SendingMessage);

	internal:
		SteamUserVM(SteamAPI::SteamUserPtr user);
		SteamAPI::SteamUserPtr GetModel() { return m_model; }

		static wchar_t * GetStatusString(SteamAPI::ISteamUser::OnlineStatus status);
		void SendConversationMessage(Platform::String^ message);

	private:
		bool CanSendMessage();

		SteamAPI::SteamUserPtr m_model;
		Windows::UI::Xaml::Interop::IBindableObservableVector^ _ConversationHistory;
		AutoEventListener<SteamAPI::ISteamUser, int> m_StatusChangedEventListener;

	};

	ref class ConversationVectorChangedEventArgs sealed : Windows::Foundation::Collections::IVectorChangedEventArgs
	{
	public:
		ConversationVectorChangedEventArgs(Windows::Foundation::Collections::CollectionChange change, unsigned int index) : _change(change), _index(index) {}

		property Windows::Foundation::Collections::CollectionChange CollectionChange { virtual Windows::Foundation::Collections::CollectionChange get() { return _change; } }
		property unsigned int Index { virtual unsigned int get() { return _index; } }

	private:
		Windows::Foundation::Collections::CollectionChange _change;
		unsigned int _index;
	};

	[Windows::UI::Xaml::Data::Bindable]
	public ref class VirtualConversationList sealed : public Windows::UI::Xaml::Interop::IBindableObservableVector
	{
	public:
		// IEnumerable
		virtual Windows::UI::Xaml::Interop::IBindableIterator^ First()      { throw std::exception("NYI"); return nullptr; }

		// IVector - not implemented
		virtual void Append(Platform::Object^ value)                         { throw std::exception("NYI"); }
		virtual void Clear()                                                 { throw std::exception("NYI"); }
		virtual Windows::UI::Xaml::Interop::IBindableVectorView^ GetView()   { throw std::exception("NYI"); }
		virtual bool IndexOf(Platform::Object^ value, unsigned int *index)   { throw std::exception("NYI"); return false; }
		virtual void InsertAt(unsigned int index, Platform::Object^ value)   { throw std::exception("NYI"); }
		virtual void RemoveAt(unsigned int index)                            { throw std::exception("NYI"); }
		virtual void RemoveAtEnd()                                           { throw std::exception("NYI"); }
		virtual void SetAt(unsigned int index, Platform::Object^ value)      { throw std::exception("NYI"); }
		
		// IVector - implemented
		virtual property unsigned int Size { unsigned int get(); }
		virtual Platform::Object^ GetAt(unsigned int index);

		// IObservableVector - implemented
		virtual event Windows::UI::Xaml::Interop::BindableVectorChangedEventHandler^ VectorChanged;

	internal:
		VirtualConversationList(const SteamAPI::SteamUserPtr & user, const SteamAPI::SteamConversationPtr & conversation);

	private:
		SteamAPI::SteamUserPtr         m_User;
		SteamAPI::SteamConversationPtr m_ChatConversation;
		Windows::Foundation::Uri^      m_LoggedInAvatarURI;
		AutoEventListener<SteamAPI::ISteamConversation, int> m_ConversationChangedEventListener;
	};

	[Windows::UI::Xaml::Data::Bindable]
	public ref class SteamDataDesignVM sealed
	{
	public:
		SteamDataDesignVM(void)
		{
			Items = ref new Platform::Collections::Vector<SteamUserDesignVM^>();
		}

		property Platform::String^ Title;
		property Windows::Foundation::Collections::IVector<SteamUserDesignVM^>^ Items;
		property SteamUserDesignVM^ SelectedItem;
	};


	[Windows::UI::Xaml::Data::Bindable]
	public ref class SteamDataVM sealed : public Common::BindableBase
	{
	public:
		SteamDataVM(void)
		{
			m_items = ref new Platform::Collections::Vector<SteamUserVM^>();
		}

		property Windows::Foundation::Collections::IVector<SteamUserVM^>^ Items
		{
			Windows::Foundation::Collections::IVector<SteamUserVM^>^ get() {return m_items; }
		}

		MAKE_PROP(SteamUserVM^, SelectedItem);
		//property SteamUserVM^ SelectedItem;

	internal:
		void SortByStatus();
		void AddFriend(SteamAPI::SteamUserPtr & user);

	private:
		~SteamDataVM(void){}

		Platform::Collections::Vector<SteamUserVM^>^ m_items;
		AutoEventListener<SteamAPI::ISteamUser, int> m_UserStatusChangedEventListener;
	};

	SteamUserVM^ CreateUserVMFromUser(SteamAPI::SteamUserPtr & user);
}

