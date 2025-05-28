#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

DWORD FindProcessId(const char* processName) {
    PROCESSENTRY32 pe32; // Khởi tạo một struct chứa thông tin về từng process.
    HANDLE hSnapshot = NULL;
    pe32.dwSize = sizeof(PROCESSENTRY32); // Báo cho Windows biết kích thước của PROCESSENTRY32 là bao nhiêu bytes để nó có thể điền dữ liệu đúng cách vào cấu trúc này
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // Tạo snapshot để list toàn bộ process đang chạy.

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (_stricmp(pe32.szExeFile, processName) == 0) {
                DWORD pid = pe32.th32ProcessID;
                CloseHandle(hSnapshot);
                return pid;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
    return 0;
}

int main() {
    const char* dllPath = "C:\\Users\\vanld5\\Downloads\\VCS\\loaddll.dll"; 
    DWORD pid = FindProcessId("notepad.exe");

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    LPVOID remoteAddr = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1,
                                       MEM_COMMIT, PAGE_READWRITE); // Cấp phát chỗ trống đủ để chứa đường dẫn DLL.

    WriteProcessMemory(hProcess, remoteAddr, dllPath, strlen(dllPath) + 1, NULL);

    LPTHREAD_START_ROUTINE loadLibraryAddr = 
        (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");// Lấy địa chỉ LoadLibraryA trong kernel32.dll

    CreateRemoteThread(hProcess, NULL, 0, loadLibraryAddr, remoteAddr, 0, NULL); // Tạo thread từ notepad.exe để gọi LoadLibraryA(dllPath)
    CloseHandle(hProcess);

    return 0;
}
