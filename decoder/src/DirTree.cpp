#include "DirTree.hpp"

DirTree::DirTree(const std::wstring root, HWND hwnd) : _hwnd(hwnd)
{
    _DirTreeRoot.directory_path = root;
    _DirTreeRoot.files_count    = 0;
    if (RecurseDirSearch(root, _DirTreeRoot) == false)
        DisplayErrorBox(_hwnd, L"Failed to read directory.", GetLastError());
    else
        _DirTreeRoot.directory_path = root;
}

DirTree::~DirTree() {}

const DirTreeRoot& DirTree::getTreeRoot() const
{
    return _DirTreeRoot;
}

bool DirTree::RecurseDirSearch(const std::wstring& root, DirTreeRoot& dirTreeRoot)
{
    WIN32_FIND_DATA ffd;
    HANDLE          hFind = INVALID_HANDLE_VALUE;
    {
        std::wstring searchPath = root + L"\\*";
        hFind                   = FindFirstFile(searchPath.c_str(), &ffd);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            DisplayErrorBox(
                _hwnd, L"FindFirstFile() failed, please provide valid directory.", GetLastError()
            );
            return false;
        }
    }
    DWORD dwError = 0;
    {
        LARGE_INTEGER filesize;
        do
        {
            if (wcscmp(ffd.cFileName, L".") != 0 && wcscmp(ffd.cFileName, L"..") != 0)
            {
                if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    dirTreeRoot.directories.push_back({});
                    dirTreeRoot.directories.back().directory_name = ffd.cFileName;
                    dirTreeRoot.directories.back().directory_path = root + L"\\" + ffd.cFileName;
                    if (RecurseDirSearch(
                            dirTreeRoot.directories.back().directory_path,
                            dirTreeRoot.directories.back()
                        ) == false)
                    {
                        FindClose(hFind);
                        return false;
                    }
                }
                else
                {
                    dirTreeRoot.files.push_back({root + L"\\" + ffd.cFileName, ffd.cFileName});
                    _DirTreeRoot.files_count++;
                }
            }
        } while (FindNextFile(hFind, &ffd) != 0);
        dwError = GetLastError();
        FindClose(hFind);
    }
    if (dwError != ERROR_NO_MORE_FILES)
    {
        DisplayErrorBox(_hwnd, L"FindFirstFile() failed", dwError);
        return false;
    }
    return true;
}
