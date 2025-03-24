#include "dircrypt.hpp"

bool DisplayErrorBox(HWND hwnd, std::wstring message, DWORD error)
{
    wchar_t* error_msg = NULL;
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