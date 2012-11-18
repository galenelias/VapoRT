#pragma once

namespace VapoRT
{
	public ref class StayScrolledToBottom sealed : public Windows::UI::Xaml::DependencyObject
	{
	private:
		static Windows::UI::Xaml::Interop::TypeName boolType;
		static Windows::UI::Xaml::Interop::TypeName ownerType;
		static Windows::UI::Xaml::PropertyMetadata^ enabledPropertyMetadata;
		static Windows::UI::Xaml::DependencyProperty^ _IsEnabledProperty;

	public:
		static property Windows::UI::Xaml::DependencyProperty^ IsEnabledProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get()
			{
				return _IsEnabledProperty;
			}
		}

		static bool StayScrolledToBottom::GetIsEnabled(Windows::UI::Xaml::UIElement^ element)
		{
			return (bool)element->GetValue(_IsEnabledProperty);
		};
		static void StayScrolledToBottom::SetIsEnabled(Windows::UI::Xaml::UIElement^ element, bool value)
		{
			element->SetValue(_IsEnabledProperty, value);
		};

		static void RegisterForScrollHeightChanged(Windows::UI::Xaml::DependencyObject^ d, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e);
		static void ScrollViewerLoaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};

}