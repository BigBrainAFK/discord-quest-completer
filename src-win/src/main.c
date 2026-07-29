#include <windows.h>
#include <shellapi.h>

#define ID_BTN_GITHUB   1002
#define WM_TRAY_MESSAGE (WM_USER + 1)
#define ID_TRAY_EXIT    1003
#define ID_TRAY_TOGGLE  1004

typedef BOOL            (WINAPI *P_Shell_NotifyIconW)(DWORD, PNOTIFYICONDATAW);
typedef HINSTANCE       (WINAPI *P_ShellExecuteW)(HWND, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, INT);

static P_Shell_NotifyIconW  f_Shell_NotifyIconW;
static P_ShellExecuteW      f_ShellExecuteW;

static wchar_t              g_szGameName[256] = L"Discord Quest Completer";
static const wchar_t        *GITHUB_URL = L"https://github.com/markterence/discord-quest-completer";
static NOTIFYICONDATAW      nid;
static HFONT                hFontTitle, hFontText;

/*
 * with /NODEFAULTLIB the compiler may still emit a call to
 * memset for some god forsaken reason. 
 * If it is never referenced, /OPT:REF strips it,
 * so it costs nothing in that case.
 */
#pragma function(memset)
void *memset(void *dest, int c, size_t count) {
    char *b = (char *)dest;
    while (count--) *b++ = (char)c;
    return dest;
}

static const wchar_t *FindStringW(const wchar_t *str, const wchar_t *substr) {
    for (; *str; str++) {
        const wchar_t *h = str, *n = substr;
        while (*h && *n && *h == *n) { h++; n++; }
        if (!*n) return str;
    }
    return NULL;
}

static void ToggleWindow(HWND hWnd) {
    if (IsWindowVisible(hWnd)) {
        ShowWindow(hWnd, SW_HIDE);
    } else {
        ShowWindow(hWnd, SW_RESTORE);
        SetForegroundWindow(hWnd);
    }
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        HWND h1, h2, h3, h4;

        hFontTitle = CreateFontW(22, 0, 0, 0, FW_BOLD,   0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"Segoe UI");
        hFontText  = CreateFontW(18, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"Segoe UI");

        h1 = CreateWindowExW(0, L"STATIC", L"Discord Quest Completer",
                             WS_VISIBLE | WS_CHILD | SS_CENTER, 0, 20, 400, 25, hWnd, 0, 0, 0);
        h2 = CreateWindowExW(0, L"STATIC", g_szGameName,
                             WS_VISIBLE | WS_CHILD | SS_CENTER, 0, 55, 400, 25, hWnd, 0, 0, 0);
        h3 = CreateWindowExW(0, L"STATIC", L"This program is part of the Discord Quest Completer",
                             WS_VISIBLE | WS_CHILD | SS_CENTER, 10, 100, 380, 40, hWnd, 0, 0, 0);
        h4 = CreateWindowExW(0, L"BUTTON", L"View on GitHub",
                             WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 125, 150, 150, 35,
                             hWnd, (HMENU)ID_BTN_GITHUB, 0, 0);

        SendMessageW(h1, WM_SETFONT, (WPARAM)hFontTitle, TRUE);
        SendMessageW(h2, WM_SETFONT, (WPARAM)hFontText,  TRUE);
        SendMessageW(h3, WM_SETFONT, (WPARAM)hFontText,  TRUE);
        SendMessageW(h4, WM_SETFONT, (WPARAM)hFontText,  TRUE);

        nid.cbSize           = sizeof nid;
        nid.hWnd             = hWnd;
        nid.uID              = 1;
        nid.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_TRAY_MESSAGE;
        nid.hIcon            = LoadIconW(0, IDI_APPLICATION);
        lstrcpynW(nid.szTip, g_szGameName, 128);
        f_Shell_NotifyIconW(NIM_ADD, &nid);
    } break;

    case WM_TRAY_MESSAGE:
        if (lParam == WM_RBUTTONUP) {
            POINT cur;
            HMENU m;
            GetCursorPos(&cur);
            m = CreatePopupMenu();
            InsertMenuW(m, 0, MF_BYPOSITION | MF_STRING, ID_TRAY_TOGGLE,
                        IsWindowVisible(hWnd) ? L"Hide" : L"Show");
            InsertMenuW(m, 2, MF_SEPARATOR, 0, 0);
            InsertMenuW(m, 3, MF_BYPOSITION | MF_STRING, ID_TRAY_EXIT, L"Exit");
            SetForegroundWindow(hWnd);
            TrackPopupMenu(m, TPM_BOTTOMALIGN | TPM_LEFTALIGN, cur.x, cur.y, 0, hWnd, 0);
            DestroyMenu(m);
        } else if (lParam == WM_LBUTTONDBLCLK) {
            ToggleWindow(hWnd);
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_BTN_GITHUB:  f_ShellExecuteW(0, L"open", GITHUB_URL, 0, 0, SW_SHOWNORMAL); break;
        case ID_TRAY_EXIT:   DestroyWindow(hWnd); break;
        case ID_TRAY_TOGGLE: ToggleWindow(hWnd);  break;
        }
        break;

    case WM_CTLCOLORSTATIC:
        SetBkMode((HDC)wParam, TRANSPARENT);
        return (LRESULT)GetStockObject(WHITE_BRUSH);

    case WM_DESTROY:
        f_Shell_NotifyIconW(NIM_DELETE, &nid);
        DeleteObject(hFontTitle);
        DeleteObject(hFontText);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    return 0;
}

static void ParseTitle(void) {
    const wchar_t *p = FindStringW(GetCommandLineW(), L"--title");
    int i = 0;
    if (!p) return;
    p += 7;                       /* skip "--title" */
    while (*p == L' ') p++;
    if (*p == L'"') {
        for (p++; *p && *p != L'"' && i < 255; ) g_szGameName[i++] = *p++;
    } else {
        for (; *p && *p != L' ' && i < 255; ) g_szGameName[i++] = *p++;
    }
    g_szGameName[i] = 0;
}

void mainEntryPoint(void) {
    HINSTANCE hInst = GetModuleHandleW(NULL);
    HMODULE   hShell32;
    WNDCLASSW wc;
    HWND      hWnd;
    MSG       msg;

    hShell32            = LoadLibraryW(L"shell32.dll");
    f_Shell_NotifyIconW = (P_Shell_NotifyIconW)GetProcAddress(hShell32, "Shell_NotifyIconW");
    f_ShellExecuteW     = (P_ShellExecuteW)    GetProcAddress(hShell32, "ShellExecuteW");

    ParseTitle();

    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInst;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursorW(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName  = 0;
    wc.lpszClassName = L"DQCTray";
    RegisterClassW(&wc);

    hWnd = CreateWindowExW(0, wc.lpszClassName, g_szGameName,
                           WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                           CW_USEDEFAULT, CW_USEDEFAULT, 415, 240, 0, 0, hInst, 0);
    ShowWindow(hWnd, SW_SHOWNORMAL);

    while (GetMessageW(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    ExitProcess(0);
}
