#pragma once
#include <windows.h>
#include <string>

namespace process {
    uintptr_t get_base_address(DWORD pid);
    std::string get_name(DWORD pid);
}
