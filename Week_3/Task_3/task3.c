#include <windows.h>
#include <stdio.h>

typedef HANDLE (WINAPI *CreateFileW_t)(
    LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);

CreateFileW_t originalCreateFileW = NULL;
BYTE originalBytes[5]; // Lưu 5 byte đầu tiên của CreateFileW

// Hàm hook
HANDLE WINAPI HookedCreateFileW(
    LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    // Tạm thời khôi phục 5 byte gốc để tránh recursion
    DWORD oldProtect;
    VirtualProtect((LPVOID)originalCreateFileW, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy((void*)originalCreateFileW, originalBytes, 5);
    VirtualProtect((LPVOID)originalCreateFileW, 5, oldProtect, &oldProtect);

    if (lpFileName && wcsstr(lpFileName, L"1.txt")) {
        MessageBoxW(NULL, L"Notepad is opening 1.txt", L"Alert!", MB_OK);
    }

    // Gọi lại CreateFileW gốc
    HANDLE hFile = originalCreateFileW(
        lpFileName, dwDesiredAccess, dwShareMode,
        lpSecurityAttributes, dwCreationDisposition,
        dwFlagsAndAttributes, hTemplateFile);

    // Hook lại sau khi gọi xong
    VirtualProtect((LPVOID)originalCreateFileW, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
    DWORD relative = (DWORD)((BYTE*)&HookedCreateFileW - (BYTE*)originalCreateFileW - 5);
    BYTE patch[5] = { 0xE9, 0, 0, 0, 0 };
    memcpy(patch + 1, &relative, 4);
    memcpy((void*)originalCreateFileW, patch, 5);
    VirtualProtect((LPVOID)originalCreateFileW, 5, oldProtect, &oldProtect);

    return hFile;
}

// Thực hiện hook
void HookCreateFileW()
{
    HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    if (!hKernel32) return;

    FARPROC pCreateFileW = GetProcAddress(hKernel32, "CreateFileW");
    if (!pCreateFileW) return;

    originalCreateFileW = (CreateFileW_t)pCreateFileW;

    // Lưu 5 byte gốc
    memcpy(originalBytes, (const void*)pCreateFileW, 5);

    // Tính toán JMP offset
    DWORD relative = (DWORD)((BYTE*)&HookedCreateFileW - (BYTE*)pCreateFileW - 5); // vì JMP là 5 byte, cần trừ đi 5 để đúng khoảng cách từ cuối lệnh JMP
    BYTE patch[5] = { 0xE9, 0, 0, 0, 0 }; // E9 là opcode của lệnh JMP
    memcpy(patch + 1, &relative, 4); // copy 4 byte offset vào phần còn lại tạo thành JMP hợp lệ

    DWORD oldProtect;
    VirtualProtect((LPVOID)pCreateFileW, 5, PAGE_EXECUTE_READWRITE, &oldProtect); // mở khóa ghi
    memcpy((void*)pCreateFileW, patch, 5); // ghi lệnh JMP vào đầu hàm
    VirtualProtect((LPVOID)pCreateFileW, 5, oldProtect, &oldProtect); // trả lại trạng thái bảo vệ ban đầu để tránh lỗi bảo mật hoặc truy cập bất thường
}

// DLL entrypoint
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        HookCreateFileW();
    }
    return TRUE;
}
