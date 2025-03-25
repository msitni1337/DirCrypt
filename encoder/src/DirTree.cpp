#include "DirTree.hpp"

DirTree::DirTree(const std::wstring root, HWND TreeViewUIHandler)
    : _TreeViewUIHandler(TreeViewUIHandler)
{
    TreeView_DeleteAllItems(TreeViewUIHandler);
    _DirTreeRoot.directory_path = root;
    _DirTreeRoot.files_count    = 0;
    if (RecurseDirSearch(root, _DirTreeRoot) == false)
        InsertToTreeView(L"Analyzing path: [" + root + L"] failed.", TVI_ROOT, TVI_FIRST, -1);
    else
        PopulateTreeView(_DirTreeRoot, TVI_ROOT);
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
                _TreeViewUIHandler, L"FindFirstFile() failed, please provide valid directory.",
                GetLastError()
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
                    dirTreeRoot.files.push_back(ffd.cFileName);
                    _DirTreeRoot.files_count++;
                }
            }
        } while (FindNextFile(hFind, &ffd) != 0);
        dwError = GetLastError();
        FindClose(hFind);
    }
    if (dwError != ERROR_NO_MORE_FILES)
    {
        DisplayErrorBox(_TreeViewUIHandler, L"FindFirstFile() failed", dwError);
        return false;
    }
    return true;
}

void DirTree::PopulateTreeView(const DirTreeRoot& dirTreeRoot, HTREEITEM parent)
{
    HTREEITEM prev = TVI_FIRST;
    for (size_t i = 0; i < dirTreeRoot.directories.size(); i++)
    {
        prev = InsertToTreeView(dirTreeRoot.directories[i].directory_path, parent, prev, 0);
        if (dirTreeRoot.directories[i].directories.size() ||
            dirTreeRoot.directories[i].files.size())
            PopulateTreeView(dirTreeRoot.directories[i], prev);
    }
    for (size_t i = 0; i < dirTreeRoot.files.size(); i++)
        prev = InsertToTreeView(dirTreeRoot.files[i], parent, prev, -1);
}

HTREEITEM DirTree::InsertToTreeView(
    const std::wstring& text, HTREEITEM parent, HTREEITEM prev, int img
)
{
    TVITEM         tvi;
    TVINSERTSTRUCT tvins;
    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    // Set the text of the item.
    tvi.pszText        = (LPWSTR)text.c_str();
    tvi.cchTextMax     = sizeof(tvi.pszText) / sizeof(tvi.pszText[0]);
    tvi.iImage         = -1;
    tvi.iSelectedImage = img;
    // Save the heading level in the item's application-defined
    // data area.
    tvins.item         = tvi;
    tvins.hInsertAfter = prev;
    tvins.hParent      = parent;
    // Set the parent item based on the specified level.
    // tvins.hParent = TVI_ROOT;
    return (HTREEITEM
    )SendMessage(_TreeViewUIHandler, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);
}