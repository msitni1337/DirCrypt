#pragma once
#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <shlobj_core.h>
#include <stdio.h>
#include <string>
#include <strsafe.h>
#include <tchar.h>
#include <wincrypt.h>
#include<shlwapi.h>

#pragma comment(lib, "shlwapi")
#pragma comment(lib, "comctl32")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "user32")
#pragma comment(lib, "shell32")
#pragma comment(lib, "advapi32")

#define PROGNAME   L"DirCrypt - Decoder"
#define PROGCREDIT L"coded by @msitni"
#define WNDWIDTH   1000
#define WNDHEIGHT  600

#define DECRYPT_NOTIF_ID 100

bool DisplayErrorBox(HWND hwnd, std::wstring message, DWORD error);
LRESULT CALLBACK WindowProcRoutine(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
