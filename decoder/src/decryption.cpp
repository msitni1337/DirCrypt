#include "dirdec.hpp"

void ListFilesInDirectory(HWND hwnd, const std::wstring &directory, std::wstring &EditText, std::wstring Indent)
{
    WIN32_FIND_DATA ffd;
    LARGE_INTEGER filesize;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;
    {
        std::wstring searchPath = directory + L"\\*";
        hFind = FindFirstFile(searchPath.c_str(), &ffd);
    }
    if (INVALID_HANDLE_VALUE == hFind)
    {
        DisplayErrorBox(hwnd, L"FindFirstFile() failed, please provide valid directory.");
        return;
    }
    {
        std::wstring Files;
        do
        {
            if (wcscmp(ffd.cFileName, L".") != 0 && wcscmp(ffd.cFileName, L"..") != 0)
            {
                if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    EditText += Indent;
                    EditText += L"|<dir> [";
                    EditText += ffd.cFileName;
                    EditText += L"]\r\n";
                    ListFilesInDirectory(hwnd, directory + L"\\" + ffd.cFileName, EditText, Indent + L"|   ");
                }
                else
                {
                    filesize.LowPart = ffd.nFileSizeLow;
                    filesize.HighPart = ffd.nFileSizeHigh;
                    Files += Indent;
                    Files += L"| ";
                    Files += L"file> ";
                    Files += ffd.cFileName;
                    Files += L" Size: ";
                    Files += std::to_wstring(filesize.QuadPart);
                    Files += L" bytes\r\n";
                }
            }
        } while (FindNextFile(hFind, &ffd) != 0);
        EditText += Files;
    }
    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
    {
        DisplayErrorBox(hwnd, L"FindFirstFile() failed");
    }
    FindClose(hFind);
}

int list_dir(TCHAR *argv, HWND hEdit)
{
    std::wstring EditText = L"Analyzing path: ";
    EditText += argv;
    EditText += L"\r\n";

    ListFilesInDirectory(hEdit, argv, EditText, L"");

    SetWindowText(hEdit, EditText.c_str());
    return 0;
}