#include "dircrypt.hpp"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    InitCommonControls();
    WNDCLASSEX wc    = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WindowProcRoutine;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_SHIELD);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = PROGNAME L"_ClassWindow";
    // wc.hIconSm       = LoadIcon(NULL, IDI_WINLOGO);
    RegisterClassEx(&wc);
    // Create the window.
    HWND hwnd = CreateWindowEx(
        WS_EX_ACCEPTFILES,                                          // Optional window styles.
        PROGNAME L"_ClassWindow",                                   // Window class
        PROGNAME L" | " PROGCREDIT,                                 // Window text
        (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX), // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, WNDWIDTH, WNDHEIGHT,

        NULL,      // Parent window
        NULL,      // Menu
        hInstance, // Instance handle
        NULL       // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}