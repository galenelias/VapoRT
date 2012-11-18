#include "pch.h"

#include "StayScrolledToBottom.h"
#include "XamlHelpers.h"

using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Interop;
using namespace Platform;

namespace VapoRT
{
	void ScrollableHeightChanged(Windows::UI::Xaml::DependencyObject^ d, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e)
	{
		auto newV = e->NewValue;
		auto oldV = e->OldValue;

		if (e->NewValue)
		{
			ScrollViewer^ scrollViewer = safe_cast<ScrollViewer^>(d);

			ScrollBar^ scrollBar = FindXamlElement<ScrollBar^>(scrollViewer, [](ScrollBar^ bar) { return bar->Orientation == Orientation::Vertical; });
			Platform::IBox<double>^ boxedDouble = dynamic_cast<Platform::IBox<double>^>(e->OldValue);
			if (boxedDouble != nullptr && boxedDouble->Value == scrollBar->Value)
				scrollViewer->ScrollToVerticalOffset(safe_cast<double>(e->NewValue)); //Scroll to bottom
		}
		else
		{
			//assert(false);
		}
	}

	void RegisterForScrollableHeightChangeNotification(FrameworkElement^ element)
	{
		TypeName objectType = Object::typeid;
		TypeName scrollViewerOwnerType = {ScrollViewer::typeid->FullName, TypeKind::Metadata };
		PropertyMetadata^ sourcePropertyMetadata = ref new PropertyMetadata(false, ref new PropertyChangedCallback(&ScrollableHeightChanged));

		static DependencyProperty^ _ScrollableHeightProperty = DependencyProperty::RegisterAttached("ScrollableHeightAttached", objectType, scrollViewerOwnerType, sourcePropertyMetadata);

		Binding^ binding = ref new Binding();
		binding->Source = element;
		binding->Path = ref new PropertyPath(L"ScrollableHeight");
		element->SetBinding(_ScrollableHeightProperty, binding);
	}

	TypeName StayScrolledToBottom::boolType = { "bool", TypeKind::Primitive };
	TypeName StayScrolledToBottom::ownerType = {StayScrolledToBottom::typeid->FullName, TypeKind::Metadata };
	PropertyMetadata^ StayScrolledToBottom::enabledPropertyMetadata = ref new PropertyMetadata(false, ref new PropertyChangedCallback(&StayScrolledToBottom::RegisterForScrollHeightChanged));
	DependencyProperty^ StayScrolledToBottom::_IsEnabledProperty = DependencyProperty::RegisterAttached("IsEnabled", boolType, ownerType, enabledPropertyMetadata);


	void StayScrolledToBottom::RegisterForScrollHeightChanged(Windows::UI::Xaml::DependencyObject^ d, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e)
	{
		if (e->NewValue)
		{
			ScrollViewer^ scrollViewer = FindXamlElement<ScrollViewer^>(d);
			if (scrollViewer)
				RegisterForScrollableHeightChangeNotification(scrollViewer);
			else
			{
				FrameworkElement^ fe = safe_cast<FrameworkElement^>(d);
				fe->Loaded += ref new RoutedEventHandler(StayScrolledToBottom::ScrollViewerLoaded);
			}
		}
		else
		{
			//assert(false);
		}
	}

	void StayScrolledToBottom::ScrollViewerLoaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
	{
		DependencyObject^ depObj = safe_cast<DependencyObject^>(sender);
		ScrollViewer^ scrollViewer = FindXamlElement<ScrollViewer^>(depObj);
		if (scrollViewer)
			RegisterForScrollableHeightChangeNotification(scrollViewer);
	}

}