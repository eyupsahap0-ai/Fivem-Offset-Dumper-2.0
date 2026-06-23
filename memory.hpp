#pragma once
#include <windows.h>
#include <cstdint>
#include <string>

namespace memory {
    uintptr_t pattern_scan(HANDLE process_handle, uintptr_t base_address, const std::string& pattern);
    uintptr_t get_rip_relative_address(HANDLE process_handle, uintptr_t instruction_address);
    uintptr_t dereference_pointer(HANDLE process_handle, uintptr_t address);
}
