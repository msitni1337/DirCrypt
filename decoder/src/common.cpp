#include "dircrypt.hpp"

bool DisplayErrorBox(HWND hwnd, std::wstring message, DWORD error)
{
    LPTSTR error_msg = NULL;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), error_msg, 0, NULL
    );
    message += L"\nfailed with error: " + std::to_wstring(error) + L": ";
    if (error_msg != NULL)
        message += error_msg;
    MessageBox(hwnd, (LPCTSTR)message.c_str(), L"Error", MB_OK);
    if (error_msg != NULL)
        LocalFree(error_msg);
    return false;
}

bool IsPathOutsideAnother(HWND hwnd, const std::wstring& parent, const std::wstring& child)
{
    wchar_t fullParent[MAX_PATH];
    wchar_t fullChild[MAX_PATH];
    if (!GetFullPathNameW(parent.c_str(), MAX_PATH, fullParent, nullptr) ||
        !GetFullPathNameW(child.c_str(), MAX_PATH, fullChild, nullptr))
        return DisplayErrorBox(hwnd, L"Error", GetLastError());
    if (PathCchAppend(fullParent, MAX_PATH, L"") != S_OK ||
        PathCchAppend(fullChild, MAX_PATH, L"") != S_OK)
        return DisplayErrorBox(hwnd, L"Error", GetLastError());
    return (StrStrIW(fullChild, fullParent) == NULL);
}