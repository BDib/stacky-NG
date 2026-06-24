#pragma once
#include <windows.h>
#include "Utils.h"

#define WM_TRAYICON (WM_USER + 1)

void CreateTrayIcon(HWND hwnd);
void RemoveTrayIcon(HWND hwnd);
void ShowContextMenu(HWND hwnd);