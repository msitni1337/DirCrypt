#pragma once
#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <CommCtrl.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <string>
#pragma comment(lib, "comctl32")
#pragma comment(lib, "Gdi32")
#pragma comment(lib, "user32")
#pragma comment(lib, "Shell32")

#define PROGNAME L"DirDec"
#define PROGCREDIT L"coded by @msitni"
#define WNDWIDTH 1000
#define WNDHEIGHT 600

void DisplayErrorBox(HWND hwnd, LPTSTR lpszFunction);
int list_dir(TCHAR *argv, HWND hEdit);
LRESULT CALLBACK WindowProcRoutine(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
