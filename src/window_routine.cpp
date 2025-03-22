#include "direnc.hpp"
#define ENC_BTN 200
#define EDT_BOX 300

static HWND hEdit;
static HWND hButton;

LRESULT CALLBACK WindowProcRoutine(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        {
            HFONT hfDefault;
            hEdit = CreateWindowEx(
                WS_EX_CLIENTEDGE, // Optional window styles.
                L"EDIT",          // Predefined class; Edit.
                L"Drag and drop a folder",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                0, 0, WNDWIDTH, WNDHEIGHT - 100,
                hwnd,
                (HMENU)EDT_BOX,
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                NULL);
            if (hEdit == NULL)
                MessageBox(hwnd, L"Could not create edit box.", L"Error", MB_OK | MB_ICONERROR);

            hfDefault = (HFONT)GetStockObject(OEM_FIXED_FONT);
            SendMessage(hEdit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
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
                (HMENU)ENC_BTN,                                        // Button ID.
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                NULL); // Pointer not needed.

            if (hButton == NULL)
                MessageBox(hwnd, L"Could not create button.", L"Error", MB_OK | MB_ICONERROR);
        }
        break;
    }
    case WM_DROPFILES:
    {
        TCHAR DraggedFileName[MAX_PATH];
        HDROP hDrop = (HDROP)wParam;
        int count = DragQueryFile(hDrop, 0xFFFFFFFF, DraggedFileName, MAX_PATH);
        for (int i = 0; i < count; i++)
        {
            DragQueryFile(hDrop, i, DraggedFileName, MAX_PATH);
            // MessageBox(GetForegroundWindow(), DraggedFileName, L"Current file received", MB_OK);
        }
        DragFinish(hDrop);
        list_dir(DraggedFileName, hEdit);
        break;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        break;
    }
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case ENC_BTN:
            MessageBox(hwnd, L"NOT IMPLEMENTED", L"Encryption", MB_OK);
            break;
        }
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 2));
        ps.rcPaint.top = 0;
        ps.rcPaint.bottom = 100;
        ps.rcPaint.left = 0;
        ps.rcPaint.right = 100;
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_HIGHLIGHT));

        EndPaint(hwnd, &ps);
        break;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}