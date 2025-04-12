#pragma once
#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <aclapi.h>
#include <combaseapi.h>
#include <fstream>
#include <pathcch.h>
#include <shlobj_core.h>
#include <shlwapi.h>
#include <stdio.h>
#include <string>
#include <strsafe.h>
#include <tchar.h>
#include <wincrypt.h>
#include "resource.h"

#pragma comment(lib, "Ole32")
#pragma comment(lib, "pathcch")
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "comctl32")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "user32")
#pragma comment(lib, "shell32")
#pragma comment(lib, "advapi32")

#define PROGNAME   L"DirCrypt decryptor free unlimited"
#define PROGCREDIT L"coded by @msitni"
#define WNDWIDTH   500
#define WNDHEIGHT  150

#define DECRYPT_NOTIF_ID 100
#define HEXSTR           L"0123456789abcdef"

bool DisplayErrorBox(HWND hwnd, std::wstring message, DWORD error);
bool IsPathOutsideAnother(HWND hwnd, const std::wstring& parent, const std::wstring& child);
LRESULT CALLBACK WindowProcRoutine(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
std::wstring     readAutorunInf(const std::wstring& filePath);