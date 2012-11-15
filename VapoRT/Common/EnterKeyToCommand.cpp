#include "pch.h"
#include "EnterKeyToCommand.h"
#include "EventRegistrationTokenMap.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;

namespace VapoRT
{
	TypeName EnterKeyToCommand::commandType = ICommand::typeid;
	TypeName EnterKeyToCommand::ownerType = {EnterKeyToCommand::typeid->FullName, TypeKind::Metadata };

	PropertyMetadata^ EnterKeyToCommand::commandPropertyMetadata = ref new PropertyMetadata(false, ref new PropertyChangedCallback(&EnterKeyToCommand::RegisterForKeyDown));

	DependencyProperty^ EnterKeyToCommand::_EnterKeyCommandProperty = DependencyProperty::RegisterAttached("EnterKeyCommand", commandType, ownerType, commandPropertyMetadata);


	void EnterKeyToCommand::RegisterForKeyDown(Windows::UI::Xaml::DependencyObject^ d, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e)
	{
		TextBox^ tb = safe_cast<TextBox^>(d);
		String^ eventName = L"EnterKeyToCommand_KeyDown";

		if (e->NewValue)
		{
			EventRegistrationToken tokenIgnored;
			if (!LookupEventToken(d, eventName, false, &tokenIgnored))
			{
				EventRegistrationToken token = tb->KeyDown += ref new KeyEventHandler(EnterKeyToCommand::KeyDownEvent);
				RecordEventToken(d, eventName, token);
			}
		}
		else
		{
			EventRegistrationToken token;
			if (LookupEventToken(d, eventName, true, &token))
			{
				tb->KeyDown -= token;
			}
		}
	}

	void EnterKeyToCommand::KeyDownEvent(Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ e)
	{
		if (e->Key == Windows::System::VirtualKey::Enter && e->KeyStatus.RepeatCount != 0)
		{
			DependencyObject^ dependencyObject = dynamic_cast<DependencyObject^>(sender);

			if (dependencyObject)
			{
				ICommand^ cmd = (ICommand^)dependencyObject->GetValue(EnterKeyToCommand::EnterKeyCommandProperty);
				cmd->Execute(sender);
			}
		}
	}
}