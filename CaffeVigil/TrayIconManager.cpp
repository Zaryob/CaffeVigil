#include "Resource.h"
#include "TrayIconManager.h"

TrayIconManager::TrayIconManager(HINSTANCE hInstance, HWND hwnd)
	: hInst(hInstance), hwnd(hwnd) {
	ZeroMemory(&nid, sizeof(nid));
	AddTrayIcon();
}

TrayIconManager::~TrayIconManager() {
	RemoveTrayIcon();
}

void TrayIconManager::ShowContextMenu() {
	POINT pt;
	GetCursorPos(&pt);
	SetForegroundWindow(hwnd);

	TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
	PostMessage(hwnd, WM_NULL, 0, 0);
}

void TrayIconManager::AddTrayIcon() {
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = TRAY_ICON_UID;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcscpy_s(nid.szTip, sizeof(nid.szTip) / sizeof(nid.szTip[0]), L"Prevent Sleep");

    Shell_NotifyIcon(NIM_ADD, &nid);

    hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, IDR_MAINFRAME, L"Open Main Window");
    AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);

    // Add a new menu item
    AppendMenu(hMenu, MF_UNCHECKED, SLP_FOREVER, L"Prevent Sleep forever.");

    hTimeMenu = CreatePopupMenu();

    AppendMenu(hTimeMenu, MF_UNCHECKED, SLP_FIVE, L"Prevent Sleep for 5 minutes.");
    AppendMenu(hTimeMenu, MF_UNCHECKED, SLP_TEN, L"Prevent Sleep for 10 minutes.");
    AppendMenu(hTimeMenu, MF_UNCHECKED, SLP_THIRTY, L"Prevent Sleep for 30 minutes.");
    AppendMenu(hTimeMenu, MF_UNCHECKED, SLP_HOUR, L"Prevent Sleep for 1 hours.");

    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hTimeMenu, L"Prevent Sleep for a period of time.");

    AppendMenu(hMenu, MF_STRING, IDM_EXIT, L"Exit");
}

void TrayIconManager::RemoveTrayIcon() {
	Shell_NotifyIcon(NIM_DELETE, &nid);
}

void TrayIconManager::RemoveAwakeInterval() {
    if (activeTimerID != 0) {
        KillTimer(hwnd, activeTimerID);
    }
    SetExecutionStateNormal();
}

void TrayIconManager::SetExecutionStateNormal() {
	SetThreadExecutionState(ES_CONTINUOUS);
    // Update menu items to reflect the current selection
    CheckMenuItem(hMenu, SLP_FOREVER, MF_UNCHECKED);
    CheckMenuItem(hTimeMenu, SLP_FIVE, MF_UNCHECKED);
    CheckMenuItem(hTimeMenu, SLP_TEN, MF_UNCHECKED);
    CheckMenuItem(hTimeMenu, SLP_THIRTY, MF_UNCHECKED);
    CheckMenuItem(hTimeMenu, SLP_HOUR, MF_UNCHECKED);
}

void TrayIconManager::SetAwakeInterval(int minutes, int command) {
    // Clear any previous state
    SetExecutionStateNormal();

    // Set the execution state to keep awake for the specified interval
    SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);

   
    if(command == SLP_FOREVER)
        CheckMenuItem(hMenu, SLP_FOREVER, MF_CHECKED);
    else
        CheckMenuItem(hTimeMenu, command, MF_CHECKED);

    // Kill any existing timer
    if (activeTimerID != 0 && minutes>0) {
        KillTimer(hwnd, activeTimerID);
    }

    // Set a new timer to allow sleep after the interval
    activeTimerID = SetTimer(hwnd, TRAY_ICON_UID, /*minutes * 60*/5 * 1000, nullptr);
}