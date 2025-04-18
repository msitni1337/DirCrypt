#include "DirEncryptor.hpp"

#define ENCRYPT_BTN_ID 200
#define LIST_VIEW_ID   300
#define IDC_FILELIST   101
#define IDS_PATHTOFILL 102

static HWND hTreeView;
static HWND hButton;
static HWND hwndEdit;
static HWND hwndPB;
static HWND hwndCR;

WCHAR SelectedItemFolder[MAX_PATH];
TCHAR DraggedFileName[MAX_PATH];

static DirTreeRoot TreeRoot;

DWORD WINAPI EncryptionThread(LPVOID hwnd)
{
    SendMessage(hwndPB, PBM_SETPOS, 0, 0);
    long cr = 0;
    {
        WCHAR buff[256];
        buff[GetWindowText(hwndCR, buff, 256)] = 0;
        cr                                     = std::wcstol(buff, NULL, 10);
        if (cr < 0)
        {
            MessageBox((HWND)hwnd, L"Enter credit more than 0", PROGNAME, MB_OK);
            return 0;
        }
    }
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
    DirEncryptor dirEncryptor(buff, SelectedItemFolder, (UINT)cr, (HWND)hwnd);
    if (dirEncryptor.isReady() && dirEncryptor.encryptTree(dest_path, TreeRoot))
    {
        DirTree dirTree(DraggedFileName, hTreeView);
        TreeRoot = dirTree.getTreeRoot();
        MessageBox((HWND)hwnd, L"Encryption success.", PROGNAME, MB_OK);
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
            HWND handle = CreateWindowEx(
                0, L"Static", L"Credit:", WS_CHILD | WS_VISIBLE, 185, WNDHEIGHT - 95, 75, 50, hwnd,
                0, g_hInst, NULL
            );
            SendMessage(handle, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
        }
        {
            hwndCR = CreateWindowEx(
                WS_EX_LEFT | WS_EX_CLIENTEDGE | WS_EX_CONTEXTHELP, // Extended window styles.
                WC_EDIT, NULL,
                WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER // Window styles.
                    | ES_NUMBER | ES_LEFT,              // Edit control styles.
                185, WNDHEIGHT - 72, 52, 23, hwnd, NULL, g_hInst, NULL
            );
            SendMessage(hwndCR, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
            HWND hControl = CreateWindowEx(
                WS_EX_LEFT | WS_EX_LTRREADING, UPDOWN_CLASS, NULL,
                WS_CHILDWINDOW | WS_VISIBLE | UDS_AUTOBUDDY | UDS_SETBUDDYINT | UDS_ALIGNRIGHT |
                    UDS_ARROWKEYS | UDS_HOTTRACK,
                0, 0, 0, 0, hwnd, NULL, g_hInst, NULL
            );
            SendMessage(hControl, UDM_SETRANGE, 0, MAKELPARAM(UD_MAXVAL, 1));
        }
        {
            hButton = CreateWindowEx(
                0,                                                     // Optional window styles.
                L"BUTTON",                                             // Predefined class; Button.
                L"Encrypt",                                            // Button text.
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Styles.
                270,                                                   // x position.
                WNDHEIGHT - 95,                                        // y position.
                100,                                                   // Button width.
                50,                                                    // Button height.
                hwnd,                                                  // Parent window.
                (HMENU)ENCRYPT_BTN_ID,                                 // Button ID.
                g_hInst, NULL
            );
            SendMessage(hButton, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
        }
        {
            hwndPB = CreateWindowEx(
                0, PROGRESS_CLASS, (LPTSTR)NULL, WS_CHILD | WS_VISIBLE, 380, WNDHEIGHT - 85,
                WNDWIDTH - 410, 25, hwnd, (HMENU)0, g_hInst, NULL
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
            DragQueryFile(hDrop, i, DraggedFileName, MAX_PATH);
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
        case ENCRYPT_BTN_ID: {
            HTREEITEM selection = TreeView_GetSelection(hTreeView);
            if (selection == NULL)
            {
                MessageBox((HWND)hwnd, L"Select License output folder", PROGNAME, MB_OK);
                break;
            }
            TVITEM item;
            item.hItem      = selection;
            item.mask       = TVIF_TEXT;
            item.pszText    = SelectedItemFolder;
            item.cchTextMax = MAX_PATH;
            if (!TreeView_GetItem(hTreeView, &item))
            {
                MessageBox((HWND)hwnd, L"Select License output folder", PROGNAME, MB_OK);
                break;
            }
            DWORD dwAttrib = GetFileAttributes(SelectedItemFolder);
            if (dwAttrib == INVALID_FILE_ATTRIBUTES || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
            {
                MessageBox((HWND)hwnd, L"Select License output folder", PROGNAME, MB_OK);
                break;
            }
            DWORD  ThreadId;
            HANDLE Thread = CreateThread(
                NULL,             // default security attributes
                0,                // use default stack size
                EncryptionThread, // thread function name
                hwnd,             // argument to thread function
                0,                // use default creation flags
                &ThreadId
            );
            if (Thread == NULL)
                break;
            CloseHandle(Thread);
            break;
        }
        case ENCRYPT_NOTIF_ID: {
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