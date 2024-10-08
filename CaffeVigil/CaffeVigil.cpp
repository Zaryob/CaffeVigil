// CaffeVigil.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "CaffeVigil.h"
#include "TrayIconManager.h" 
#include "NotificationManager.h"

#include <functional>
#include <winrt/Windows.Data.Xml.Dom.h>
#include <winrt/Windows.UI.Notifications.h>
#include <conio.h>

using namespace winrt;
using namespace Windows::Data::Xml::Dom;
using namespace Windows::UI::Notifications;

bool _hasStarted;

void sendToast(std::wstring message);

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

HWND hStatic = nullptr;
ULONGLONG endTime = 0;

static TrayIconManager* trayIconManager = nullptr;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_CAFFEVIGIL, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, SW_HIDE))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CAFFEVIGIL));

	NotificationManager::Register(L"io.zaryob.CaffeVigil", szTitle, L"IDI_CAFFEVIGIL");

	NotificationManager::OnActivated([](NotificationActivatedEventArgsCompat e)
		{
			if (e.Argument()._Starts_with(L"action=viewWindow")) {
				NotificationManager::History().Clear();
				PostMessage(FindWindow(szWindowClass, szTitle), WM_COMMAND, IDR_MAINFRAME, 0);
			}
			else if (e.Argument()._Starts_with(L"action=ok"))
			{
				NotificationManager::History().Clear();
			}
			// action=cancel
			else
			{
				NotificationManager::History().Clear();
				// send msg to CALLBACK WndProc
				PostMessage(FindWindow(szWindowClass, szTitle), WM_TIMER, trayIconManager->GetTimerId(), 0);

			}
		});


	MSG msg;


	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

void sendToast(std::wstring message)
{
	// Construct the toast template
	XmlDocument doc;
	doc.LoadXml(
		L"<toast>\
    <visual>\
        <binding template=\"ToastGeneric\">\
            <text></text>\
        </binding>\
    </visual>\
    <actions>\
        <action\
            content=\"Ok\"\
            activationType=\"background\"/>\
        <action\
            content=\"Cancel\"\
            activationType=\"background\"/>\
    </actions>\
</toast>");

	// Populate with text and values
	doc.SelectSingleNode(L"//toast").as<XmlElement>().SetAttribute(L"duration", L"Default");

	doc.DocumentElement().SetAttribute(L"launch", L"action=viewWindow&conversationId=9813");
	doc.SelectSingleNode(L"//text[1]").InnerText(L"Sleep prevention set to " + message);
	doc.SelectSingleNode(L"//action[1]").as<XmlElement>().SetAttribute(L"arguments", L"action=ok&conversationId=9813");
	doc.SelectSingleNode(L"//action[2]").as<XmlElement>().SetAttribute(L"arguments", L"action=cancel&conversationId=9813");

	// Construct the notification
	ToastNotification notif{ doc };

	// And send it!
	NotificationManager::CreateToastNotifier().Show(notif);

}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CAFFEVIGIL));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CAFFEVIGIL);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	int windowWidth = 400;
	int windowHeight = 300;

	HWND hWnd = CreateWindowW(
		szWindowClass,
		szTitle,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,  // Window style
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowWidth,
		windowHeight,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	if (!hWnd)
	{
		return FALSE;
	}

	// Create a static control to display the countdown timer
	hStatic = CreateWindowW(
		L"STATIC", L"Time Left: 00:00",
		WS_CHILD | WS_VISIBLE | SS_CENTER,
		50, 50, 300, 50,
		hWnd, nullptr, hInstance, nullptr
	);
	    // Create buttons for different timer options
    CreateWindowW(
        L"BUTTON", L"5 Minutes",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        50, 100, 120, 30,
        hWnd, (HMENU)SLP_FIVE, hInstance, nullptr
    );

    CreateWindowW(
        L"BUTTON", L"10 Minutes",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        200, 100, 120, 30,
        hWnd, (HMENU)SLP_TEN, hInstance, nullptr
    );

    CreateWindowW(
        L"BUTTON", L"30 Minutes",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        50, 150, 120, 30,
        hWnd, (HMENU)SLP_THIRTY, hInstance, nullptr
    );

    CreateWindowW(
        L"BUTTON", L"1 Hour",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        200, 150, 120, 30,
        hWnd, (HMENU)SLP_HOUR, hInstance, nullptr
    );

    CreateWindowW(
        L"BUTTON", L"Forever",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        125, 200, 120, 30,
        hWnd, (HMENU)SLP_FOREVER, hInstance, nullptr
    );

	// Store the handle of the static control
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)hStatic);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	SetTimer(hWnd, 1, 1000, nullptr); // Timer fires every second

	return TRUE;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message) {
	case WM_CREATE:
		trayIconManager = new TrayIconManager((HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), hWnd);
		break;
	case WM_TRAYICON:
		if (LOWORD(lParam) == WM_RBUTTONUP) {
			trayIconManager->ShowContextMenu();
		}
		break;
		break;

	case WM_COMMAND:
	{
		// Parse the menu selections:
		switch (LOWORD(wParam))
		{
		case SLP_FOREVER:
			// MessageBox(hWnd, L"Prevent Sleep forever.", L"Prevent Sleep", MB_OK);
			sendToast(L"forever");
			trayIconManager->SetAwakeInterval(-1, SLP_FOREVER);
			endTime = 2; // forever

			break;
		case SLP_FIVE:
			//MessageBox(hWnd, L"Prevent Sleep for 5 minutes.", L"Prevent Sleep", MB_OK);
			sendToast(L"5 mins.");
			trayIconManager->SetAwakeInterval(5, SLP_FIVE);
			endTime = GetTickCount64() + 5 * 60 * 1000; // 300 seconds from now

			break;
		case SLP_TEN:
			//MessageBox(hWnd, L"Prevent Sleep for 10 minutes.", L"Prevent Sleep", MB_OK);
			sendToast(L"10 mins.");
			trayIconManager->SetAwakeInterval(10, SLP_TEN);
			endTime = GetTickCount64() + 10 * 60 * 1000; // 6000 seconds from now

			break;
		case SLP_THIRTY:
			//MessageBox(hWnd, L"Prevent Sleep for 30 minutes.", L"Prevent Sleep", MB_OK);
			sendToast(L"30 mins.");
			trayIconManager->SetAwakeInterval(30, SLP_THIRTY);
			endTime = GetTickCount64() + 30 * 60 * 1000; // 18000 seconds from now

			break;
		case SLP_HOUR:
			//MessageBox(hWnd, L"Prevent Sleep for 1 hour.", L"Prevent Sleep", MB_OK);
			sendToast(L"1 hour.");
			trayIconManager->SetAwakeInterval(30, SLP_HOUR);
			endTime = GetTickCount64() + 60 * 60 * 1000; // 3600 seconds from now
			break;
		case IDR_MAINFRAME:
			ShowWindow(hWnd, SW_SHOW);
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			delete trayIconManager;
			DestroyWindow(hWnd);
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_TIMER:
		// Clear the execution state to allow sleep after the timer interval
		if (wParam == trayIconManager->GetTimerId()) {
			trayIconManager->RemoveAwakeInterval();
			KillTimer(hWnd, trayIconManager->GetTimerId());
			endTime = 0;
		}
		else {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			ULONGLONG currentTime = GetTickCount64();
			if (endTime > currentTime) {
				int timeLeft = (int)((endTime - currentTime) / 1000); // Time left in seconds

				if (timeLeft > 0)
				{
					int minutes = timeLeft / 60;
					int seconds = timeLeft % 60;

					wchar_t buffer[50];
					swprintf(buffer, 50, L"Time Left: %02d:%02d", minutes, seconds);
					SetWindowText(hStatic, buffer);
				}
				else
				{
					SetWindowText(hStatic, L"Time Left: 00:00");
				}
			}
			else if (endTime == 0) {
				SetWindowText(hStatic, L"Timer not set!");
			}
			else if (endTime == 1) {
				SetWindowText(hStatic, L"Timer set forever!");
			}
			else
			{
				SetWindowText(hStatic, L"Time is up!");
			}
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;

}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


