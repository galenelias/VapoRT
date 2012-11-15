#pragma once

namespace VapoRT
{
	public ref class EnterKeyToCommand sealed : public Windows::UI::Xaml::DependencyObject
	{
	private:
		static Windows::UI::Xaml::Interop::TypeName commandType;
		static Windows::UI::Xaml::Interop::TypeName ownerType;
		static Windows::UI::Xaml::PropertyMetadata^ commandPropertyMetadata;
		static Windows::UI::Xaml::DependencyProperty^ _EnterKeyCommandProperty;

	public:
		static property Windows::UI::Xaml::DependencyProperty^ EnterKeyCommandProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get()
			{
				return _EnterKeyCommandProperty;
			}
		}

		static Windows::UI::Xaml::Input::ICommand^ EnterKeyToCommand::GetEnterKeyCommand(Windows::UI::Xaml::UIElement^ element)
		{
			return (Windows::UI::Xaml::Input::ICommand^)element->GetValue(_EnterKeyCommandProperty);
		};
		static void EnterKeyToCommand::SetEnterKeyCommand(Windows::UI::Xaml::UIElement^ element, Windows::UI::Xaml::Input::ICommand^ value)
		{
			element->SetValue(_EnterKeyCommandProperty, value);
		};
		static void RegisterForKeyDown(Windows::UI::Xaml::DependencyObject^ d, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e);
		static void KeyDownEvent(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ e);

	};
}