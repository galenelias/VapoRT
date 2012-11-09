#pragma once

namespace MyFirstApp
{
	public ref class EnterKeyToCommand sealed : public Windows::UI::Xaml::DependencyObject
	{
	private:
		static Windows::UI::Xaml::Interop::TypeName commandType;
		static Windows::UI::Xaml::Interop::TypeName ownerType;
		static Windows::UI::Xaml::PropertyMetadata^ propertyMetadata;
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


	public ref class UpdateSourceHelper sealed : public Windows::UI::Xaml::DependencyObject
	{
	private:
		static Windows::UI::Xaml::Interop::TypeName boolType;
		static Windows::UI::Xaml::Interop::TypeName stringType;
		static Windows::UI::Xaml::Interop::TypeName ownerType;
		static Windows::UI::Xaml::PropertyMetadata^ enabledPropertyMetadata;
		static Windows::UI::Xaml::PropertyMetadata^ sourcePropertyMetadata;
		static Windows::UI::Xaml::DependencyProperty^ _IsEnabledProperty;
		static Windows::UI::Xaml::DependencyProperty^ _UpdateSourceTextProperty;

	public:
		static property Windows::UI::Xaml::DependencyProperty^ IsEnabledProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get()
			{
				return _IsEnabledProperty;
			}
		}

		static bool UpdateSourceHelper::GetIsEnabled(Windows::UI::Xaml::UIElement^ element)
		{
			return (bool)element->GetValue(_IsEnabledProperty);
		};
		static void UpdateSourceHelper::SetIsEnabled(Windows::UI::Xaml::UIElement^ element, bool value)
		{
			element->SetValue(_IsEnabledProperty, value);
		};

		static void RegisterForTextChanged(Windows::UI::Xaml::DependencyObject^ d, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e);
		static void AttachedTextBoxTextChanged2(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);

		static property Windows::UI::Xaml::DependencyProperty^ UpdateSourceTextProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get()
			{
				return _UpdateSourceTextProperty;
			}
		}

		static String^ GetUpdateSourceText(Windows::UI::Xaml::DependencyObject^ obj)
		{
			return (String^)obj->GetValue(_UpdateSourceTextProperty);
		}

		static void SetUpdateSourceText(Windows::UI::Xaml::DependencyObject^ obj, String^ value)
		{
			obj->SetValue(_UpdateSourceTextProperty, value);
		}
	};
}