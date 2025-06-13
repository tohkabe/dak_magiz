#include <windows.h>
#include <stdio.h>
#include <dbghelp.h>   

#pragma comment(lib, "dbghelp.lib") 
#define TARGET_FILE L"C:\\Users\\vanld5\\Downloads\\VCS\\1.txt"

// Hàm gốc
typedef HANDLE(WINAPI* CreateFileW_t)(
    LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE
);

CreateFileW_t originalCreateFileW = NULL;

// Hàm hook
HANDLE WINAPI HookedCreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
) {
    if (lpFileName && _wcsicmp(lpFileName, TARGET_FILE) == 0) {
        MessageBoxW(NULL, L"File 1.txt vừa được mở!", L"Alert!", MB_OK | MB_ICONINFORMATION);
    }

    // Gọi hàm gốc, mọi lời gọi khác đến CreateFileW() vẫn hoạt động bình thường
    return originalCreateFileW(
        lpFileName,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile
    );
}

// Hook IAT
void HookIAT() {
    HMODULE hModule = GetModuleHandle(NULL); // dll được load trong process nào
    if (!hModule) return;

    ULONG size;
    PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(
        hModule, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &size);

    if (!pImportDesc) return;

    for (; pImportDesc->Name; pImportDesc++) { // duyệt đến IAT để tìm module KERNEL32.dll vì CreateFileW nằm trong đó.
        LPCSTR pszModName = (LPCSTR)((PBYTE)hModule + pImportDesc->Name);
        if (_stricmp(pszModName, "KERNEL32.dll") != 0) continue;

        PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)((PBYTE)hModule + pImportDesc->FirstThunk);
        PIMAGE_THUNK_DATA pOrigThunk = (PIMAGE_THUNK_DATA)((PBYTE)hModule + pImportDesc->OriginalFirstThunk);

        for (; pOrigThunk->u1.Function; pOrigThunk++, pThunk++) { // tiếp tục duyệt danh sách các hàm được import
            PROC* ppfn = (PROC*)&pThunk->u1.Function;
            PROC pfn = *ppfn;

            PIMAGE_IMPORT_BY_NAME pImport = (PIMAGE_IMPORT_BY_NAME)((PBYTE)hModule + pOrigThunk->u1.AddressOfData);
            if (_stricmp((char*)pImport->Name, "CreateFileW") == 0) {
                DWORD oldProtect;
                VirtualProtect(ppfn, sizeof(PROC), PAGE_EXECUTE_READWRITE, &oldProtect); // // Cho phép ghi vào vùng nhớ
                originalCreateFileW = (CreateFileW_t)pfn;
                *ppfn = (PROC)HookedCreateFileW; // Ghi đè địa chỉ
                VirtualProtect(ppfn, sizeof(PROC), oldProtect, &oldProtect);
                return;
            }
        }
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        HookIAT(); // Gọi hook khi DLL được nạp
    }
    return TRUE;
}
