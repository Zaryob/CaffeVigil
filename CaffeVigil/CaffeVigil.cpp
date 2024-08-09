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

	NotificationManager::Register(L"io.zaryob.CaffeVigil", szTitle, L"C:\\MyIcon.png");

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
				PostMessage(FindWindow(szWindowClass, szTitle), WM_TIMER, WM_TIMER, 0);

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

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

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
	static TrayIconManager* trayIconManager = nullptr;

	switch (message) {
	case WM_CREATE:
		trayIconManager = new TrayIconManager((HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), hWnd);
		break;
	case WM_TRAYICON:
		if (LOWORD(lParam) == WM_RBUTTONUP) {
			trayIconManager->ShowContextMenu();
		}
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
			break;
		case SLP_FIVE:
			//MessageBox(hWnd, L"Prevent Sleep for 5 minutes.", L"Prevent Sleep", MB_OK);
			sendToast(L"5 mins.");
			trayIconManager->SetAwakeInterval(5, SLP_FIVE);
			break;
		case SLP_TEN:
			//MessageBox(hWnd, L"Prevent Sleep for 10 minutes.", L"Prevent Sleep", MB_OK);
			sendToast(L"10 mins.");
			trayIconManager->SetAwakeInterval(10, SLP_TEN);
			break;
		case SLP_THIRTY:
			//MessageBox(hWnd, L"Prevent Sleep for 30 minutes.", L"Prevent Sleep", MB_OK);
			sendToast(L"30 mins.");
			trayIconManager->SetAwakeInterval(30, SLP_THIRTY);
			break;
		case SLP_HOUR:
			//MessageBox(hWnd, L"Prevent Sleep for 1 hour.", L"Prevent Sleep", MB_OK);
			sendToast(L"1 hour.");
			trayIconManager->SetAwakeInterval(30, SLP_HOUR);
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
		trayIconManager->RemoveAwakeInterval();
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


