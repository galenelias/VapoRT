#include "pch.h"
#include "UpdateSourceHelper.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;

namespace VapoRT
{
	TypeName UpdateSourceHelper::boolType = { "bool", TypeKind::Primitive };
	TypeName UpdateSourceHelper::stringType = String::typeid;
	TypeName UpdateSourceHelper::ownerType = {UpdateSourceHelper::typeid->FullName, TypeKind::Metadata };

	PropertyMetadata^ UpdateSourceHelper::enabledPropertyMetadata = ref new PropertyMetadata(false, ref new PropertyChangedCallback(&UpdateSourceHelper::RegisterForTextChanged));
	PropertyMetadata^ UpdateSourceHelper::sourcePropertyMetadata = ref new PropertyMetadata(false);

	DependencyProperty^ UpdateSourceHelper::_IsEnabledProperty = DependencyProperty::RegisterAttached("IsEnabled", boolType, ownerType, enabledPropertyMetadata);
	DependencyProperty^ UpdateSourceHelper::_UpdateSourceTextProperty = DependencyProperty::RegisterAttached("UpdateSourceText", stringType, ownerType, sourcePropertyMetadata);

	void UpdateSourceHelper::RegisterForTextChanged(Windows::UI::Xaml::DependencyObject^ d, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e)
	{
		if (e->NewValue)
		{
			TextBox^ tb = safe_cast<TextBox^>(d);
			tb->TextChanged += ref new TextChangedEventHandler(&UpdateSourceHelper::AttachedTextBoxTextChanged2);
		}
		else
		{
			// We don't save off our event subscription token, so can't unsubscribe
			assert(false);
		}
	}

	void UpdateSourceHelper::AttachedTextBoxTextChanged2(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
	{
		auto typee = sender->GetType();
		TextBox^ tb = dynamic_cast<TextBox^>(sender);
		if (tb)
		{
			tb->SetValue(UpdateSourceHelper::UpdateSourceTextProperty, tb->Text);
		}
	}
}