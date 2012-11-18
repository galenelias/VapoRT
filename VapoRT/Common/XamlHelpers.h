#pragma once

namespace VapoRT
{

	template <class T>
	T FindXamlElement(Windows::UI::Xaml::DependencyObject^ obj, std::function<bool(T)> predicate)
	{
		std::vector<Windows::UI::Xaml::DependencyObject^> rgElements;
		rgElements.push_back(std::move(obj));

		while (!rgElements.empty())
		{
			Windows::UI::Xaml::DependencyObject^ cur = rgElements.back();
			rgElements.erase(rgElements.end()-1);
			T t = dynamic_cast<T>(cur);
			if (t && (!predicate || predicate(t)))
				return t;
			else
			{
				int childCount = Windows::UI::Xaml::Media::VisualTreeHelper::GetChildrenCount(cur);
				for (int i = 0; i != childCount; ++i)
				{
					auto ctrl = Windows::UI::Xaml::Media::VisualTreeHelper::GetChild(cur, i);
					rgElements.push_back(std::move(ctrl));
				}
			}
		}
		return nullptr;
	}

	template <class T>
	T FindXamlElement(Windows::UI::Xaml::DependencyObject^ obj)
	{
		return FindXamlElement(obj, std::function<bool(T)>());
	}

}