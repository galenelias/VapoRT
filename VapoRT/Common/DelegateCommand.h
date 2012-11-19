#pragma once

namespace VapoRT
{
	namespace Common
	{
		delegate void ExecuteDelegate(Platform::Object^ parameter);
		delegate bool CanExecuteDelegate(Platform::Object^ parameter);

		public ref class DelegateCommand sealed : public Windows::UI::Xaml::Input::ICommand
		{
			ExecuteDelegate^ _executeDelegate;
			CanExecuteDelegate^ _canExecuteDelegate;

		internal:
			DelegateCommand(ExecuteDelegate^ executeDelegate, 
				CanExecuteDelegate^ canExecuteDelegate = nullptr)
			{
				_executeDelegate = executeDelegate;
				_canExecuteDelegate = canExecuteDelegate;
			}

		public:
			virtual event Windows::Foundation::EventHandler<Object^>^ CanExecuteChanged;

			void RaiseCanExecuteChanged()
			{
				CanExecuteChanged(this, nullptr);
			}

			virtual void Execute(Object^ parameter)
			{
				_executeDelegate(parameter);
			}

			virtual bool CanExecute(Object^ parameter)
			{
				return _canExecuteDelegate == nullptr || _canExecuteDelegate(parameter);
			}
		};

	}
}