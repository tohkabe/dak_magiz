#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

DWORD GetProcessIdByName(const wchar_t* processName) {
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return 0;

    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            if (_wcsicmp(pe32.szExeFile, processName) == 0) {
                DWORD pid = pe32.th32ProcessID;
                CloseHandle(hSnapshot);
                return pid;
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return 0;
}

int main() {
    const wchar_t* targetProcess = L"notepad.exe";
    const char* dllPath = "C:\\Users\\vanld5\\Downloads\\VCS\\task3.dll";

    DWORD pid = GetProcessIdByName(targetProcess);
    if (pid == 0) {
        printf("Không tìm thấy tiến trình %ls\n", targetProcess);
        return 1;
    }

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProc) {
        printf("Không thể mở tiến trình PID %lu\n", pid);
        return 1;
    }

    LPVOID addr = VirtualAllocEx(hProc, NULL, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!addr) {
        printf("VirtualAllocEx thất bại.\n");
        CloseHandle(hProc);
        return 1;
    }

    SIZE_T written;
    if (!WriteProcessMemory(hProc, addr, dllPath, strlen(dllPath) + 1, &written)) {
        printf("WriteProcessMemory thất bại.\n");
        VirtualFreeEx(hProc, addr, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return 1;
    }

    HANDLE hThread = CreateRemoteThread(hProc, NULL, 0,
        (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"),
        addr, 0, NULL);

    if (!hThread) {
        printf("CreateRemoteThread thất bại.\n");
        VirtualFreeEx(hProc, addr, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return 1;
    }

    WaitForSingleObject(hThread, INFINITE);
    printf("Đã inject DLL thành công vào Notepad (PID: %lu)\n", pid);

    VirtualFreeEx(hProc, addr, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProc);

    return 0;
}
