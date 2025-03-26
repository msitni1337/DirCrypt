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
    SendMessage(hwndPB, PBM_SETRANGE32, 0, TreeRoot.files_count);
    DirDecryptor dirDecryptor(buff, (HWND)hwnd);
    dirDecryptor.decryptTree(TreeRoot);
    DirTree dirTree(WorkingPath, (HWND)hwnd);
    TreeRoot = dirTree.getTreeRoot();
    return 0;
}

LRESULT CALLBACK WindowProcRoutine(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE: {
        ChangeWindowMessageFilterEx(hwnd, WM_DROPFILES, MSGFLT_ALLOW, NULL);
        ChangeWindowMessageFilterEx(hwnd, WM_COPYDATA, MSGFLT_ALLOW, NULL);
        DragAcceptFiles(hwnd, true);
        HINSTANCE g_hInst   = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        HFONT     hfDefault = CreateFont(
            14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial"
        );
        {
            HWND handle = CreateWindowEx(
                0, L"Static", L"Password:", WS_CHILD | WS_VISIBLE, 5, 15, 180, 50, hwnd, 0, g_hInst,
                NULL
            );
            SendMessage(handle, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
        }
        {
            hwndEdit = CreateWindowEx(
                0, L"EDIT", // predefined class
                NULL,       // no window title
                WS_CHILD | WS_VISIBLE | ES_LEFT | ES_PASSWORD | ES_CENTER, 10, 32, 170,
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
                L"START",                                            // Button text.
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Styles.
                200,                                                   // x position.
                15,                                                     // y position.
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
                0, PROGRESS_CLASS, (LPTSTR)NULL, WS_CHILD | WS_VISIBLE, 310, 25, WNDWIDTH - 340, 25,
                hwnd, (HMENU)0, g_hInst, NULL
            );
            SendMessage(hwndPB, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
            SendMessage(hwndPB, PBM_SETSTEP, 1, 0);
        }
        GetCurrentDirectory(MAX_PATH, WorkingPath);
        DirTree dirTree(WorkingPath, hwnd);
        TreeRoot = dirTree.getTreeRoot();
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