#include "pch.h"
#include "EventRegistrationTokenMap.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;

namespace VapoRT
{
	public ref class EventTokenAssociator sealed : public Windows::UI::Xaml::DependencyObject
	{
	private:
		static Windows::UI::Xaml::Interop::TypeName eventTokenMapType;
		static Windows::UI::Xaml::Interop::TypeName ownerType;
		static Windows::UI::Xaml::PropertyMetadata^ eventTokenMapPropertyMetadata;
		static Windows::UI::Xaml::DependencyProperty^ _EventTokenMapProperty;

	public:
		typedef Windows::Foundation::Collections::IMap<String^, Windows::Foundation::Collections::IVector<EventRegistrationToken>^> TokenMap_t;

		static property Windows::UI::Xaml::DependencyProperty^ EventTokenMapProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get()
			{
				return _EventTokenMapProperty;
			}
		}

		static Windows::UI::Xaml::Input::ICommand^ EventTokenAssociator::GetEventTokenMap(Windows::UI::Xaml::UIElement^ element)
		{
			return (Windows::UI::Xaml::Input::ICommand^)element->GetValue(_EventTokenMapProperty);
		};
		static void EventTokenAssociator::SetEventTokenMap(Windows::UI::Xaml::UIElement^ element, Windows::UI::Xaml::Input::ICommand^ value)
		{
			element->SetValue(_EventTokenMapProperty, value);
		};
	};

	// Map dependency object
	TypeName EventTokenAssociator::eventTokenMapType = TokenMap_t::typeid;
	TypeName EventTokenAssociator::ownerType = {EventTokenAssociator::typeid->FullName, TypeKind::Metadata };
	PropertyMetadata^ EventTokenAssociator::eventTokenMapPropertyMetadata = ref new PropertyMetadata(false);
	DependencyProperty^ EventTokenAssociator::_EventTokenMapProperty = DependencyProperty::RegisterAttached("EventTokenMap", eventTokenMapType, ownerType, eventTokenMapPropertyMetadata);

	struct EventTokenEq
	{
		bool operator()(EventRegistrationToken token1, EventRegistrationToken token2) const
		{
			return token1.Value == token2.Value;
		}
	};


	EventTokenAssociator::TokenMap_t^ EnsureEventTokenMap(Windows::UI::Xaml::DependencyObject^ obj)
	{
		EventTokenAssociator::TokenMap_t^ map = dynamic_cast<EventTokenAssociator::TokenMap_t^>(obj->GetValue(EventTokenAssociator::EventTokenMapProperty));

		if (!map)
		{
			map = ref new Platform::Collections::Map<String^, Windows::Foundation::Collections::IVector<EventRegistrationToken>^>();
			obj->SetValue(EventTokenAssociator::EventTokenMapProperty, map);
		}
		return map;
	}

	void RecordEventToken(Windows::UI::Xaml::DependencyObject^ obj, String^ eventName, EventRegistrationToken token)
	{
		EventTokenAssociator::TokenMap_t^ map = EnsureEventTokenMap(obj);

		if (map->HasKey(eventName))
			map->Lookup(eventName)->Append(token);
		else
		{
			auto newVec = ref new Platform::Collections::Vector<EventRegistrationToken, EventTokenEq>();
			newVec->Append(token);
			map->Insert(eventName, newVec);
		}
	}

	bool LookupEventToken(Windows::UI::Xaml::DependencyObject^ obj, String^ eventName, bool fRemove, EventRegistrationToken *pToken)
	{
		EventTokenAssociator::TokenMap_t^ map = EnsureEventTokenMap(obj);

		if (!map->HasKey(eventName))
			return false;
		else
		{
			auto tokenVector = map->Lookup(eventName);

			if (!tokenVector->Size)
				return false;

			*pToken = tokenVector->GetAt(tokenVector->Size - 1);
			
			if (fRemove)
				tokenVector->RemoveAtEnd();

			return true;
		}
	}
}