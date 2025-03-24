#include "DirDecryptor.hpp"

#define DECRYPT_BTN_ID 200
#define LIST_VIEW_ID   300
#define IDC_FILELIST   101
#define IDS_PATHTOFILL 102

static HWND hTreeView;
static HWND hButton;
static HWND hwndEdit;
static HWND hwndPB;

TCHAR DraggedFileName[MAX_PATH];

static DirTreeRoot TreeRoot;

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
    BROWSEINFOW dialogInfo = {0};
    dialogInfo.hwndOwner   = (HWND)hwnd;
    dialogInfo.lpszTitle   = L"Select Output folder";
    dialogInfo.ulFlags     = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_RETURNFSANCESTORS;
    ITEMIDLIST* pidl       = SHBrowseForFolder(&dialogInfo);
    if (pidl == NULL)
        return 0;
    WCHAR dest_path[MAX_PATH];
    SHGetPathFromIDList(pidl, dest_path);
    DirDecryptor dirDecryptor(buff, (HWND)hwnd);
    if (dirDecryptor.isReady() && dirDecryptor.decryptTree(dest_path, TreeRoot))
        MessageBox((HWND)hwnd, L"Decryption success.", PROGNAME, MB_OK);
    return 0;
}

LRESULT CALLBACK WindowProcRoutine(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE: {
        HFONT hfDefault;
        hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        {
            hTreeView = CreateWindowEx(
                0, WC_TREEVIEW, L"Tree View",
                WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_CHILD | WS_BORDER | ES_AUTOVSCROLL |
                    ES_AUTOHSCROLL | TVS_HASLINES,
                0, 0, WNDWIDTH - 16, WNDHEIGHT - 100, hwnd, (HMENU)LIST_VIEW_ID,
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL
            );
            if (hTreeView == NULL)
                MessageBox(hwnd, L"Could not create edit box.", L"Error", MB_OK | MB_ICONERROR);
            SendMessage(hTreeView, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
        }
        {
            HWND handle = CreateWindowEx(
                0, L"Static", L"Password:", WS_CHILD | WS_VISIBLE, 5, WNDHEIGHT - 95, 180, 50, hwnd,
                0, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL
            );
            SendMessage(handle, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
        }
        {
            hwndEdit = CreateWindowEx(
                0, L"EDIT", // predefined class
                NULL,       // no window title
                WS_CHILD | WS_VISIBLE | ES_LEFT | ES_PASSWORD, 10, WNDHEIGHT - 72, 170,
                25,       // set size in WM_SIZE message
                hwnd,     // parent window
                (HMENU)0, // edit control ID
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL
            );
            SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
        }
        {
            hButton = CreateWindowEx(
                0,                                                     // Optional window styles.
                L"BUTTON",                                             // Predefined class; Button.
                L"Decrypt",                                            // Button text.
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Styles.
                200,                                                   // x position.
                WNDHEIGHT - 95,                                        // y position.
                100,                                                   // Button width.
                50,                                                    // Button height.
                hwnd,                                                  // Parent window.
                (HMENU)DECRYPT_BTN_ID,                                 // Button ID.
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL
            );
            SendMessage(hButton, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
        }
        {
            hwndPB = CreateWindowEx(
                0, PROGRESS_CLASS, (LPTSTR)NULL, WS_CHILD | WS_VISIBLE, 310, WNDHEIGHT - 85,
                WNDWIDTH - 360, 25, hwnd, (HMENU)0,
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL
            );
            SendMessage(hwndPB, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
            SendMessage(hwndPB, PBM_SETSTEP, 1, 0);
        }
        break;
    }
    case WM_DROPFILES: {
        HDROP hDrop = (HDROP)wParam;
        int   count = DragQueryFile(hDrop, 0xFFFFFFFF, DraggedFileName, MAX_PATH);
        for (int i = 0; i < count; i++)
        {
            DragQueryFile(hDrop, i, DraggedFileName, MAX_PATH);
            // MessageBox(GetForegroundWindow(), DraggedFileName, L"Current file received", MB_OK);
        }
        DragFinish(hDrop);
        DirTree dirTree(DraggedFileName, hTreeView);
        TreeRoot = dirTree.getTreeRoot();
        SendMessage(hwndPB, PBM_SETRANGE32, 0, TreeRoot.files_count);
        break;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }
    case WM_COMMAND: {
        switch (LOWORD(wParam))
        {
        case DECRYPT_BTN_ID: {
            DWORD  ThreadId;
            HANDLE Thread = CreateThread(
                NULL,             // default security attributes
                0,                // use default stack size
                DecryptionThread, // thread function name
                hwnd,             // argument to thread function
                0,                // use default creation flags
                &ThreadId
            );
            if (Thread == NULL)
                break;
            CloseHandle(Thread);
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

        // FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 2));
        // ps.rcPaint.top    = 0;
        // ps.rcPaint.bottom = 100;
        // ps.rcPaint.left   = 0;
        // ps.rcPaint.right  = 100;
        // FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_HIGHLIGHT));

        EndPaint(hwnd, &ps);
        break;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}