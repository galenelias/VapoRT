#pragma once

namespace VapoRT
{

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

		static Platform::String^ GetUpdateSourceText(Windows::UI::Xaml::DependencyObject^ obj)
		{
			return (Platform::String^)obj->GetValue(_UpdateSourceTextProperty);
		}

		static void SetUpdateSourceText(Windows::UI::Xaml::DependencyObject^ obj, Platform::String^ value)
		{
			obj->SetValue(_UpdateSourceTextProperty, value);
		}
	};
}