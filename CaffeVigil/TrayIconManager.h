// Last modified: 2024/08/04
#pragma once
#include "pch.h"

#include <windows.h>
#include <shellapi.h>
#include <iostream>

#define TRAY_ICON_UID	1001
#define WM_TRAYICON (WM_USER + 1)

class TrayIconManager {
public:
	TrayIconManager(HINSTANCE hInstance, HWND hwnd);
	~TrayIconManager();
	void ShowContextMenu();
	void SetAwakeInterval(int minutes, int command);
	void RemoveAwakeInterval();

private:
	HINSTANCE hInst;
	HWND hwnd;
	NOTIFYICONDATA nid;
	HMENU hMenu;
	HMENU hTimeMenu;
	UINT_PTR activeTimerID;

	void AddTrayIcon();
	void RemoveTrayIcon();
	void SetExecutionStateNormal();


};

