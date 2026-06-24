#pragma once
#include <windows.h>
#include <string>

class NotificationManager {
public:
    static void ShowNotification(const std::wstring& title, const std::wstring& message, HWND hWndFallback = NULL);
};
