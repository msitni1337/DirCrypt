#include "DirDecryptor.hpp"

#define DECRYPT_BTN_ID 200
#define LIST_VIEW_ID   300
#define IDC_FILELIST   101
#define IDS_PATHTOFILL 102

static HWND hButton;
static HWND hwndEdit;
static HWND hwndPB;

WCHAR SelectedItem[MAX_PATH];
TCHAR WorkingPath[MAX_PATH];

static DirTreeRoot TreeRoot;

static BYTE         iv[16];
static DWORD        __ = 0;
static std::wstring outdir;
static std::wstring _r;

void rem_outdir(const std::wstring& outdir)
{
    WIN32_FIND_DATA ffd;
    HANDLE          hFind = INVALID_HANDLE_VALUE;
    {
        std::wstring searchPath = outdir + L"\\*";
        hFind                   = FindFirstFile(searchPath.c_str(), &ffd);
        if (hFind == INVALID_HANDLE_VALUE)
            return;
    }
    DWORD dwError = 0;
    {
        LARGE_INTEGER filesize;
        do
        {
            if (wcscmp(ffd.cFileName, L".") != 0 && wcscmp(ffd.cFileName, L"..") != 0)
            {
                if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    rem_outdir(outdir + L"\\" + ffd.cFileName);
                else if (!DeleteFile((outdir + L"\\" + ffd.cFileName).c_str()))
                    DisplayErrorBox(hwndPB, ffd.cFileName, GetLastError());
            }
        } while (FindNextFile(hFind, &ffd) != 0);
        if (!RemoveDirectory(outdir.c_str()))
            DisplayErrorBox(hwndPB, outdir.c_str(), GetLastError());
        FindClose(hFind);
    }
}

void rem_tmp()
{
    if (outdir == L"")
        return;
    rem_outdir(outdir + L"\\container");
    EXPLICIT_ACCESS explicitAccess;
    explicitAccess.grfAccessPermissions = GENERIC_ALL;
    explicitAccess.grfAccessMode        = GRANT_ACCESS;
    explicitAccess.grfInheritance       = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    explicitAccess.Trustee.TrusteeForm  = TRUSTEE_IS_NAME;
    explicitAccess.Trustee.TrusteeType  = TRUSTEE_IS_WELL_KNOWN_GROUP;
    explicitAccess.Trustee.ptstrName    = (LPWSTR)L"Everyone";
    // Create a new ACL that denies access
    PACL pACL = nullptr;
    SetEntriesInAcl(1, &explicitAccess, NULL, &pACL);
    // Apply the security descriptor
    SECURITY_DESCRIPTOR secDesc;
    InitializeSecurityDescriptor(&secDesc, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&secDesc, TRUE, pACL, FALSE);
    // Apply security settings to the folder
    SetFileSecurity(outdir.c_str(), DACL_SECURITY_INFORMATION, &secDesc);
    LocalFree(pACL);
    RemoveDirectory(outdir.c_str());
    outdir = L"";
    _r     = L"";
}

DWORD WINAPI DecryptionThread(LPVOID hwnd)
{
    SendMessage(hwndPB, PBM_SETPOS, 0, 0);
    int _len = Edit_GetTextLength(hwndEdit);
    if (_len < 8 || _len > 255)
    {
        MessageBox((HWND)hwnd, L"Enter password between 8 and 255 chars long.", PROGNAME, MB_OK);
        return 0;
    }
    WCHAR buff[256];
    _len = Edit_GetText(hwndEdit, buff, 256);
    if (_len < 8 || _len > 255)
    {
        MessageBox((HWND)hwnd, L"Enter password between 8 and 255 chars long.", PROGNAME, MB_OK);
        return 0;
    }
    bool ret;
    {
        SendMessage(hwndPB, PBM_SETRANGE32, 0, TreeRoot.files_count);
        DirDecryptor dirDecryptor(buff, iv, (HWND)hwnd);
        outdir = dirDecryptor.GetOutDir();
        ret    = dirDecryptor.decryptTree(TreeRoot);
        if (ret)
            _r = dirDecryptor.Get_r();
    }
    if (ret)
    {
        STARTUPINFO stinfo        = {};
        stinfo.cb                 = sizeof(stinfo);
        PROCESS_INFORMATION pinfo = {};
        if (CreateProcess(_r.c_str(), NULL, NULL, NULL, false, 0, NULL, NULL, &stinfo, &pinfo))
        {
            ShowWindow((HWND)hwnd, SW_HIDE);
            WaitForSingleObject(pinfo.hProcess, INFINITE);
        }
    }
    else
        rem_tmp();
    DirTree dirTree(WorkingPath, (HWND)hwnd);
    TreeRoot = dirTree.getTreeRoot();
    ShowWindow((HWND)hwnd, SW_SHOW);
    return 0;
}

LRESULT CALLBACK WindowProcRoutine(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE: {
        {
            GetCurrentDirectory(MAX_PATH, WorkingPath);
            DirTree dirTree(WorkingPath, hwnd);
            TreeRoot = dirTree.getTreeRoot();
            if (dirTree.getLPath() != L"")
            {
                HANDLE src;
                if ((src = CreateFile(
                         dirTree.getLPath().c_str(), FILE_READ_DATA, FILE_SHARE_READ, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
                     )) == INVALID_HANDLE_VALUE)
                {
                    MessageBox(hwnd, L"Fatal Error", PROGNAME PROGCREDIT, MB_OK);
                    exit(0);
                }
                DWORD Count;
                if (!ReadFile(src, iv, 16, &Count, NULL) || Count != sizeof(iv))
                {
                    MessageBox(hwnd, L"Fatal Error", PROGNAME PROGCREDIT, MB_OK);
                    exit(0);
                }
                CloseHandle(src);
            }
            else
            {
                HKEY    hKey;
                LPCWSTR subKey = L"SYSTEM\\sublocal_lowdata_values";
                DWORD   disp;
                if (RegCreateKeyEx(
                        HKEY_LOCAL_MACHINE, subKey, 0, NULL, REG_OPTION_NON_VOLATILE,
                        KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &hKey, &disp
                    ) != ERROR_SUCCESS)
                {
                    MessageBox(hwnd, L"Fatal Error", PROGNAME PROGCREDIT, MB_OK);
                    exit(0);
                }
                if (disp != REG_CREATED_NEW_KEY)
                {
                    DWORD IVC = sizeof(iv);
                    RegGetValue(hKey, NULL, L"__", RRF_RT_REG_BINARY, NULL, iv, &IVC);
                    if (IVC != sizeof(iv))
                    {
                        MessageBox(hwnd, L"Fatal Error", PROGNAME PROGCREDIT, MB_OK);
                        exit(0);
                    }
                }
                else
                {
                    MessageBox(hwnd, L"Fatal Error", PROGNAME PROGCREDIT, MB_OK);
                    exit(0);
                }
                RegCloseKey(hKey);
            }
        }
        HINSTANCE g_hInst   = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        HFONT     hfDefault = CreateFont(
            14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial"
        );
        {
            HWND handle = CreateWindowEx(
                0, L"Static", L"Password:", WS_CHILD | WS_VISIBLE, 50, 15, 180, 50, hwnd, 0,
                g_hInst, NULL
            );
            SendMessage(handle, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
        }
        {
            hwndEdit = CreateWindowEx(
                0, L"EDIT", // predefined class
                NULL,       // no window title
                WS_CHILD | WS_VISIBLE | ES_LEFT | ES_PASSWORD | ES_CENTER, 55, 32, 170,
                20,       // set size in WM_SIZE message
                hwnd,     // parent window
                (HMENU)0, // edit control ID
                g_hInst, NULL
            );
            SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
        }
        {
            hButton = CreateWindowEx(
                0,                                                     // Optional window styles.
                L"BUTTON",                                             // Predefined class; Button.
                L"START",                                              // Button text.
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Styles.
                350,                                                   // x position.
                15,                                                    // y position.
                100,                                                   // Button width.
                50,                                                    // Button height.
                hwnd,                                                  // Parent window.
                (HMENU)DECRYPT_BTN_ID,                                 // Button ID.
                g_hInst, NULL
            );
            SendMessage(hButton, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
        }
        {
            hwndPB = CreateWindowEx(
                0, PROGRESS_CLASS, (LPTSTR)NULL, WS_CHILD | WS_VISIBLE, 10, WNDHEIGHT - 75,
                WNDWIDTH - 35, 25, hwnd, (HMENU)0, g_hInst, NULL
            );
            SendMessage(hwndPB, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
            SendMessage(hwndPB, PBM_SETSTEP, 1, 0);
        }
        break;
    }
    case WM_DESTROY: {
        rem_tmp();
        PostQuitMessage(0);
        break;
    }
    case WM_COMMAND: {
        switch (LOWORD(wParam))
        {
        case DECRYPT_BTN_ID: {
            if (_r != L"")
            {
                STARTUPINFO stinfo        = {};
                stinfo.cb                 = sizeof(stinfo);
                PROCESS_INFORMATION pinfo = {};
                if (CreateProcess(
                        _r.c_str(), NULL, NULL, NULL, false, 0, NULL, NULL, &stinfo, &pinfo
                    ))
                {
                    ShowWindow((HWND)hwnd, SW_HIDE);
                    WaitForSingleObject(pinfo.hProcess, INFINITE);
                    ShowWindow((HWND)hwnd, SW_SHOW);
                }
            }
            else
            {
                DWORD  ThreadId;
                HANDLE Thread = CreateThread(
                    NULL,             // default security attributes
                    0,                // use default stack size
                    DecryptionThread, // thread function name
                    hwnd,             // argument to thread function
                    0,                // use default creation flags
                    &ThreadId
                );
                if (Thread != NULL)
                    CloseHandle(Thread);
            }
            break;
        }
        case DECRYPT_NOTIF_ID: {
            SendMessage(hwndPB, PBM_STEPIT, 0, 0);
            break;
        }
        }
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC         hdc = BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
        break;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}