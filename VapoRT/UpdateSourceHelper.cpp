#include "pch.h"

#include "UpdateSourceHelper.h"

namespace MyFirstApp
{
	using namespace Windows::Foundation;
	using namespace Windows::UI::Xaml;
	using namespace Windows::UI::Xaml::Controls;
	using namespace Windows::UI::Xaml::Input;
	using namespace Windows::UI::Xaml::Interop;

	TypeName EnterKeyToCommand::commandType = ICommand::typeid;
	TypeName EnterKeyToCommand::ownerType = {EnterKeyToCommand::typeid->FullName, TypeKind::Metadata };

	PropertyMetadata^ EnterKeyToCommand::propertyMetadata = ref new PropertyMetadata(false, ref new PropertyChangedCallback(&EnterKeyToCommand::RegisterForKeyDown));

	DependencyProperty^ EnterKeyToCommand::_EnterKeyCommandProperty = DependencyProperty::RegisterAttached("EnterKeyCommand", commandType, ownerType, propertyMetadata);


	void EnterKeyToCommand::RegisterForKeyDown(Windows::UI::Xaml::DependencyObject^ d, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e)
	{
		TextBox^ tb = safe_cast<TextBox^>(d);
		if (e->NewValue)
		{
			tb->KeyDown += ref new KeyEventHandler(EnterKeyToCommand::KeyDownEvent);
		}
	}

	void EnterKeyToCommand::KeyDownEvent(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ e)
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