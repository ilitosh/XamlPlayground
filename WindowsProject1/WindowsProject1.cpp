#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.system.h>
#include <winrt/windows.ui.xaml.hosting.h>
#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>
#include <winrt/windows.ui.xaml.input.h>
#include <winrt/windows.ui.xaml.controls.h>
#include <winrt/windows.ui.core.h>
#include <winrt/windows.ui.xaml.controls.primitives.h>
#include <winrt/Windows.ui.xaml.media.h>
#include <tchar.h>

using namespace std;
using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Hosting;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::Foundation::Numerics;
using namespace Windows::Foundation;
using namespace Windows::System::Diagnostics;

namespace winrt {
	using namespace Windows::Foundation;
	using namespace Windows::UI;
	using namespace Windows::UI::Xaml;
	using namespace Windows::UI::Xaml::Hosting;
	using namespace Windows::UI::Xaml::Input;
	using namespace Windows::UI::Xaml::Controls;
	using namespace Windows::UI::Xaml::Controls::Primitives;
	using namespace Windows::UI::Xaml::Media;
	using namespace Windows::UI::Core;
} // namespace winrt

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

HWND _hWnd;
HWND _childhWnd;
HINSTANCE _hInstance;

// ex: DebugOut(_T("TryMoveFocus result = %d\n"), result);
void WINAPIV DebugOut(const TCHAR* fmt, ...) {
	TCHAR s[1025];
	va_list args;
	va_start(args, fmt);
	wvsprintf(s, fmt, args);
	va_end(args);
	OutputDebugString(s);
}

bool IsElementChildOf(winrt::DependencyObject element, winrt::DependencyObject parentToCheck) {
	winrt::DependencyObject parent = element;
	do {
		parent = winrt::VisualTreeHelper::GetParent(parent);
		if (parent == parentToCheck) {
			return true;
		}
	} while (parent != nullptr);
	return false;
}

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	_hInstance = hInstance;

	// The main window class name.
	const wchar_t szWindowClass[] = L"Win32DesktopApp";
	WNDCLASSEX windowClass = { };

	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = szWindowClass;
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	windowClass.hIconSm = LoadIcon(windowClass.hInstance, IDI_APPLICATION);

	if (RegisterClassEx(&windowClass) == NULL)
	{
		MessageBox(NULL, L"Windows registration failed!", L"Error", NULL);
		return 0;
	}

	_hWnd = CreateWindow(
		szWindowClass,
		L"Windows c++ Win32 Desktop App",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	if (_hWnd == NULL)
	{
		MessageBox(NULL, L"Call to CreateWindow failed!", L"Error", NULL);
		return 0;
	}


	// Begin XAML Island section.

	// The call to winrt::init_apartment initializes COM; by default, in a multithreaded apartment.
	init_apartment(apartment_type::single_threaded);

	// Initialize the XAML framework's core window for the current thread.
	WindowsXamlManager winxamlmanager = WindowsXamlManager::InitializeForCurrentThread();

	// This DesktopWindowXamlSource is the object that enables a non-UWP desktop application 
	// to host WinRT XAML controls in any UI element that is associated with a window handle (HWND).
	DesktopWindowXamlSource desktopSource;

	// Get handle to the core window.
	auto interop = desktopSource.as<IDesktopWindowXamlSourceNative>();

	// Parent the DesktopWindowXamlSource object to the current window.
	check_hresult(interop->AttachToWindow(_hWnd));

	// This HWND will be the window handler for the XAML Island: A child window that contains XAML.  
	HWND hWndXamlIsland = nullptr;

	// Get the new child window's HWND. 
	interop->get_WindowHandle(&hWndXamlIsland);

	// Update the XAML Island window size because initially it is 0,0.
	SetWindowPos(hWndXamlIsland, 0, 200, 100, 800, 200, SWP_SHOWWINDOW);

	// Create the XAML content.
	Windows::UI::Xaml::Controls::StackPanel stackPanel;


	auto makeButtonWithFlyout = [](bool shouldConstrainToRootBounds, bool useClickEvent) {
		Button button;
		std::wostringstream wostringstream;
		wostringstream << L"Open Flyout (shouldConstrainToRootBounds=" << shouldConstrainToRootBounds << L",useClickEvent=" << useClickEvent << L")";
		button.Content(winrt::box_value(wostringstream.str()));

		StackPanel sp2;
		Button b2;
		b2.Content(winrt::box_value(L"b2"));
		sp2.Children().Append(b2);
		Flyout flyout;
		flyout.ShouldConstrainToRootBounds(shouldConstrainToRootBounds);
		flyout.Content(sp2);
		if (useClickEvent) {
			button.Click([=](auto const &...) {
				flyout.ShowAt(button);
				});
		}
		else {
			button.Flyout(flyout);
		}
		return button;
	};
	for (auto shouldConstrainToRootBounds : { false, true }) {
		for (auto useClickEvent : { false, true }) {
			stackPanel.Children().Append(makeButtonWithFlyout(shouldConstrainToRootBounds, useClickEvent));
		}
	}

	desktopSource.Content(stackPanel);

	/////////
	

	// For dark mode:
	//xamlContainer.RequestedTheme(Windows::UI::Xaml::ElementTheme::Dark);
	//xamlContainer.Background(winrt::Media::SolidColorBrush(winrt::Colors::Black()));

	auto takeFocusRevoker = desktopSource.TakeFocusRequested(winrt::auto_revoke,
		[=](auto, auto& args) {
			auto reason = args.Request().Reason();
			if (reason == winrt::XamlSourceFocusNavigationReason::First) {
				if (auto firstFocusableElement = winrt::FocusManager::FindFirstFocusableElement(desktopSource.Content())
					.try_as<winrt::DependencyObject>()) {
					winrt::FocusManager::TryFocusAsync(firstFocusableElement, winrt::FocusState::Programmatic);
				}
			}
			else if (reason == winrt::XamlSourceFocusNavigationReason::Last) {
				if (auto lastFocusableElement = winrt::FocusManager::FindLastFocusableElement(desktopSource.Content())
					.try_as<winrt::DependencyObject>()) {
					winrt::FocusManager::TryFocusAsync(lastFocusableElement, winrt::FocusState::Programmatic);
				}
			}

		});


	// End XAML Island section.

	ShowWindow(_hWnd, nCmdShow);
	UpdateWindow(_hWnd);

	//Message loop:
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT messageCode, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	wchar_t greeting[] = L"Hello World in Win32!";
	RECT rcClient;

	switch (messageCode)
	{
	case WM_PAINT:
		if (hWnd == _hWnd)
		{
			hdc = BeginPaint(hWnd, &ps);
			TextOut(hdc, 300, 5, greeting, wcslen(greeting));
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

		// Create main window
	case WM_CREATE:
		_childhWnd = CreateWindowEx(0, L"ChildWClass", NULL, WS_CHILD | WS_BORDER, 0, 0, 0, 0, hWnd, NULL, _hInstance, NULL);
		return 0;

		// Main window changed size
	case WM_SIZE:
		// Get the dimensions of the main window's client
		// area, and enumerate the child windows. Pass the
		// dimensions to the child windows during enumeration.
		GetClientRect(hWnd, &rcClient);
		MoveWindow(_childhWnd, 200, 200, 400, 500, TRUE);
		ShowWindow(_childhWnd, SW_SHOW);

		return 0;

		// Process other messages.

	default:
		return DefWindowProc(hWnd, messageCode, wParam, lParam);
		break;
	}

	return 0;
}