#include "DirEncryptor.hpp"

#define ENCRYPT_BTN_ID 200
#define LIST_VIEW_ID   300
#define IDC_FILELIST   101
#define IDS_PATHTOFILL 102

static HWND hTreeView;
static HWND hButton;
TCHAR       DraggedFileName[MAX_PATH];

static DirTreeRoot TreeRoot;

LRESULT CALLBACK WindowProcRoutine(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE: {
        {
            HFONT hfDefault;
            hTreeView = CreateWindowEx(
                0, WC_TREEVIEW, L"Tree View",
                WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_CHILD | WS_BORDER | ES_AUTOVSCROLL |
                    ES_AUTOHSCROLL | TVS_HASLINES,
                0, 0, WNDWIDTH - 20, WNDHEIGHT - 100, hwnd, (HMENU)LIST_VIEW_ID,
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL
            );
            if (hTreeView == NULL)
                MessageBox(hwnd, L"Could not create edit box.", L"Error", MB_OK | MB_ICONERROR);
            hfDefault = (HFONT)GetStockObject(OEM_FIXED_FONT);
            SendMessage(hTreeView, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
        }
        {
            hButton = CreateWindowEx(
                0,                                                     // Optional window styles.
                L"BUTTON",                                             // Predefined class; Button.
                L"Encrypt",                                            // Button text.
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Styles.
                10,                                                    // x position.
                WNDHEIGHT - 95,                                        // y position.
                100,                                                   // Button width.
                50,                                                    // Button height.
                hwnd,                                                  // Parent window.
                (HMENU)ENCRYPT_BTN_ID,                                 // Button ID.
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                NULL
            ); // Pointer not needed.

            if (hButton == NULL)
                MessageBox(hwnd, L"Could not create button.", L"Error", MB_OK | MB_ICONERROR);
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
            WCHAR       dest[MAX_PATH];
            BROWSEINFOW dialogInfo = {0};
            dialogInfo.hwndOwner   = hwnd;
            dialogInfo.lpszTitle   = L"Select Output folder";
            dialogInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_RETURNFSANCESTORS;
            ITEMIDLIST* pidl   = SHBrowseForFolder(&dialogInfo);
            if (pidl == NULL)
                break;
            SHGetPathFromIDList(pidl, dest);
            DirEncryptor dirEncryptor(L"123", hwnd);
            if (dirEncryptor.isReady() && dirEncryptor.encryptTree(dest, TreeRoot))
                MessageBox(hwnd, L"Encryption success.", PROGNAME, MB_OK);
            break;
        }
        }
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC         hdc = BeginPaint(hwnd, &ps);

        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 2));
        ps.rcPaint.top    = 0;
        ps.rcPaint.bottom = 100;
        ps.rcPaint.left   = 0;
        ps.rcPaint.right  = 100;
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_HIGHLIGHT));

        EndPaint(hwnd, &ps);
        break;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}