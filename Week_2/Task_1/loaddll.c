#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        MessageBoxA(NULL, "PwN!", "vanld5_heck", MB_OK);
    }
    return TRUE;
}
