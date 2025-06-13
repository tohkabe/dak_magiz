#define UNICODE
#define _UNICODE
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

DWORD FindProcessId(const wchar_t* processName) {
    PROCESSENTRY32W entry = { 0 };
    entry.dwSize = sizeof(PROCESSENTRY32W);
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, processName) == 0) {
                pid = entry.th32ProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return pid;
}


BOOL InjectDLL(DWORD pid, const char* dllPath) {
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProc) return FALSE;

    LPVOID pRemote = VirtualAllocEx(hProc, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    WriteProcessMemory(hProc, pRemote, dllPath, strlen(dllPath) + 1, NULL);

    HANDLE hThread = CreateRemoteThread(hProc, NULL, 0,
        (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryA"),
        pRemote, 0, NULL);

    if (hThread) {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
        VirtualFreeEx(hProc, pRemote, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return TRUE;
    }

    VirtualFreeEx(hProc, pRemote, 0, MEM_RELEASE);
    CloseHandle(hProc);
    return FALSE;
}

int main() {
    DWORD pid = FindProcessId(L"notepad.exe");
    if (!pid)
        return 1;

    InjectDLL(pid, "C:\\Users\\vanld5\\Downloads\\VCS\\task1.dll");
    return 0;
}
