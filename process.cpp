#include "process.hpp"
#include <TlHelp32.h>
#include <Psapi.h>

#pragma comment(lib, "Psapi.lib")

namespace process {
    uintptr_t get_base_address(DWORD pid) {
        HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (!hProc) return 0;

        HMODULE modules[1024];
        DWORD needed = 0;
        if (EnumProcessModules(hProc, modules, sizeof(modules), &needed) && needed >= sizeof(HMODULE)) {
            CloseHandle(hProc);
            return reinterpret_cast<uintptr_t>(modules[0]);
        }
        CloseHandle(hProc);
        return 0;
    }

    std::string get_name(DWORD pid) {
        HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (!hProc) return {};

        HMODULE first_module = nullptr;
        DWORD needed = 0;
        char name_buffer[MAX_PATH] = { 0 };
        if (EnumProcessModules(hProc, &first_module, sizeof(first_module), &needed)) {
            GetModuleBaseNameA(hProc, first_module, name_buffer, sizeof(name_buffer));
        }
        CloseHandle(hProc);
        return std::string(name_buffer);
    }
}
