#include "DirDecryptor.hpp"

#define DECRYPT_BTN_ID 200
#define LIST_VIEW_ID   300
#define IDC_FILELIST   101
#define IDS_PATHTOFILL 102

static HWND hTreeView;
static HWND hButton;
static HWND hwndEdit;
static HWND hwndPB;

WCHAR SelectedItem[MAX_PATH];
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
    DirDecryptor dirDecryptor(buff, SelectedItem, (HWND)hwnd);
    if (dirDecryptor.isReady() && dirDecryptor.decryptTree(TreeRoot))
    {
        DirTree dirTree(DraggedFileName, hTreeView);
        TreeRoot = dirTree.getTreeRoot();
        MessageBox((HWND)hwnd, L"Decryption success.", PROGNAME, MB_OK);
        return 0;
    }
    DirTree dirTree(DraggedFileName, hTreeView);
    TreeRoot = dirTree.getTreeRoot();
    return 0;
}

LRESULT CALLBACK WindowProcRoutine(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE: {
        HINSTANCE g_hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        HFONT     hfDefault;
        hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        {
            hTreeView = CreateWindowEx(
                0, WC_TREEVIEW, L"Tree View",
                WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_CHILD | WS_BORDER | ES_AUTOVSCROLL |
                    ES_AUTOHSCROLL | TVS_HASLINES,
                0, 0, WNDWIDTH - 16, WNDHEIGHT - 100, hwnd, (HMENU)LIST_VIEW_ID, g_hInst, NULL
            );
            if (hTreeView == NULL)
                MessageBox(hwnd, L"Could not create edit box.", L"Error", MB_OK | MB_ICONERROR);
            SendMessage(hTreeView, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
            HIMAGELIST himl; // handle to image list
            if ((himl = ImageList_Create(20, 20, FALSE, 3, 0)) != NULL)
            {
                ImageList_AddIcon(himl, LoadIcon(NULL, IDI_SHIELD));
                TreeView_SetImageList(hTreeView, himl, TVSIL_NORMAL);
            }
        }
        {
            HWND handle = CreateWindowEx(
                0, L"Static", L"Password:", WS_CHILD | WS_VISIBLE, 5, WNDHEIGHT - 95, 180, 50, hwnd,
                0, g_hInst, NULL
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
                g_hInst, NULL
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
                g_hInst, NULL
            );
            SendMessage(hButton, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
        }
        {
            hwndPB = CreateWindowEx(
                0, PROGRESS_CLASS, (LPTSTR)NULL, WS_CHILD | WS_VISIBLE, 310, WNDHEIGHT - 85,
                WNDWIDTH - 340, 25, hwnd, (HMENU)0, g_hInst, NULL
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
        if (!IsPathOutsideAnother(hwnd, DraggedFileName, L"."))
        {
            MessageBox((HWND)hwnd, PROGNAME L" MUST NOT be in the directory", PROGNAME, MB_OK);
            break;
        }
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
            HTREEITEM selection = TreeView_GetSelection(hTreeView);
            if (selection == NULL)
            {
                MessageBox((HWND)hwnd, L"Select Application to run", PROGNAME, MB_OK);
                break;
            }
            TVITEM item;
            item.hItem      = selection;
            item.mask       = TVIF_TEXT;
            item.pszText    = SelectedItem;
            item.cchTextMax = MAX_PATH;
            if (!TreeView_GetItem(hTreeView, &item))
            {
                MessageBox((HWND)hwnd, L"Select Application to run", PROGNAME, MB_OK);
                break;
            }
            DWORD dwAttrib = GetFileAttributes(SelectedItem);
            if (dwAttrib == INVALID_FILE_ATTRIBUTES || dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
            {
                MessageBox((HWND)hwnd, L"Select Application to run", PROGNAME, MB_OK);
                break;
            }
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