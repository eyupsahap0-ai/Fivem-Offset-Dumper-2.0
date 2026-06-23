#include <iostream>
#include <thread>
#include <sstream>
#include "utils.hpp"
#include "process.hpp"
#include "memory.hpp"

struct pattern_struct {
    const char* name;
    const char* signature;
    bool should_dereference;
};

int main() {
    SetConsoleTitleA("FiveM Offset Dumper");
    system("mode con:cols=80 lines=30");

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    const int padding = 5;
    std::cout << std::string(padding, ' ');
    std::cout << "FiveM Offset Dumper" << std::endl;
    std::cout << "" << std::endl;

    logger::log("FiveM bekleniyor...", logger::info);
    HWND window_handle = nullptr;
    while (!(window_handle = FindWindowA("grcWindow", NULL))) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    DWORD pid;
    GetWindowThreadProcessId(window_handle, &pid);
    if (!pid) {
        logger::log("PID alinamadi!", logger::fail);
        std::cin.get(); return 1;
    }

    std::string process_name = process::get_name(pid);
    int build = 0;
    size_t b_pos = process_name.find("_b");
    if (b_pos != std::string::npos) {
        std::string build_str = process_name.substr(b_pos + 2);
        try {
            build = std::stoi(build_str);
            logger::log("FiveM bulundu! (b" + std::to_string(build) + ")", logger::success);
        } catch (...) {
            logger::log("FiveM bulundu! (Build: " + process_name + ")", logger::success);
        }
    } else {
        logger::log("FiveM bulundu! (" + process_name + ")", logger::success);
    }

    HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!process_handle) {
        logger::log("Process acilamadi! Yonetici olarak calistir.", logger::fail);
        std::cin.get(); return 1;
    }

    uintptr_t base_address = process::get_base_address(pid);
    if (!base_address) {
        logger::log("Base adres alinamadi!", logger::fail);
        CloseHandle(process_handle);
        std::cin.get(); return 1;
    }

    std::cout << "" << std::endl;

    pattern_struct patterns[] = {
        {"world",                "48 8B 05 ? ? ? ? 33 D2 48 8B 40 08 8A CA 48 85 C0 74 16 48 8B", true},
        {"replay_interface",     "48 8D 0D ? ? ? ? 48 ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 8A D8 E8 ? ? ? ? 84 DB 75 13 48 8D 0D ? ? ? ? 48 8B D7 E8 ? ? ? ? 84 C0 74 BC 8B 8F", false},
        {"viewport",             "48 8B 15 ? ? ? ? 48 8D 2D ? ? ? ? 48 8B CD", true},
        {"blip_list",            "4C 8D 05 ? ? ? ? 0F B7 C1", false},
        {"camera",               "4C 8B 35 ? ? ? ? 33 FF 32 DB", true},
        {"bullet",               "F3 41 0F 10 19 F3 41 0F 10 41 04", true},
        {"aim_cped",             "48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 8A CB", true},
        {"set_ped_in_to_vehicle","48 8B C4 44 89 48 ? 44 89 40 ? 48 89 50 ? 48 89 48 ? 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 83 BA", true},
        {"c_sky_settings",       "48 8D 0D ? ? ? ? E8 ? ? ? ? 83 25 ? ? ? ? 00 48 8D 0D ? ? ? ? F3", true}
    };

    std::cout << std::string(padding, ' ');
    std::cout << "--- OFFSETLER ---" << std::endl;

    std::stringstream dump;
    dump << "// FiveM Offset Dump" << std::endl;
    dump << "// Build: b" << build << std::endl;
    dump << "// Base: 0x" << std::hex << base_address << std::dec << std::endl;
    dump << std::endl;

    int foundCount = 0;

    for (const auto& pattern : patterns) {
        uintptr_t instruction_address = memory::pattern_scan(process_handle, base_address, pattern.signature);
        if (!instruction_address) {
            logger::log(std::string(pattern.name) + ": BULUNAMADI", logger::fail);
            dump << "// " << pattern.name << ": NOT FOUND" << std::endl;
            continue;
        }

        uintptr_t resolved_address = memory::get_rip_relative_address(process_handle, instruction_address);
        uintptr_t offset_fix = resolved_address - base_address;
        uintptr_t final_address = pattern.should_dereference
            ? memory::dereference_pointer(process_handle, resolved_address)
            : resolved_address;

        std::stringstream ss;
        ss << "0x" << std::hex << offset_fix;

        logger::log(std::string(pattern.name) + ": " + ss.str(), logger::success);
        dump << "#define " << pattern.name << " " << ss.str() << std::endl;
        foundCount++;
    }

    CloseHandle(process_handle);

    // Dosyaya yaz
    std::string fname = "fivem_offsets_b" + std::to_string(build) + ".h";
    FILE* f = fopen(fname.c_str(), "w");
    if (f) {
        fputs(dump.str().c_str(), f);
        fclose(f);
        std::cout << "" << std::endl;
        logger::log("Dosyaya kaydedildi: " + fname, logger::success);
    }

    std::cout << "" << std::endl;
    std::stringstream summary;
    summary << foundCount << " / 9 offset bulundu";
    logger::log(summary.str(), foundCount > 5 ? logger::success : logger::fail);
    std::cout << "" << std::endl;
    std::cin.get();
    return 0;
}
