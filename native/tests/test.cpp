#include "../include/etw_provider_api.hpp"
#include "debug_output.hpp"
#include <iostream>
#include <string>

void on_provider_enable(bool is_enabled, uint64_t match_any_keyword, uint64_t match_all_keyword, void *) {
    std::cout << "Provider enable callback: enabled: " << std::boolalpha << is_enabled << ", match_any_keyword: " << match_any_keyword << ", match_all_keyword: " << match_all_keyword << "\n";
};

int main()
{
    using namespace events;
    using namespace events::etw;

    uint64_t handle = 0;

    try {
        std::cout << "Register provider...\n";
        handle = register_provider<on_provider_enable>(JetBrains_Common_DebugOutput::guid);
        std::cout << "Provider is registered\n";
        std::cout << "Write string and then Enter to send it to DebugOutput:\n";

        using dbg_output = JetBrains_Common_DebugOutput::DebugOutput;

        for(std::string str; std::getline(std::cin, str); )
            write_event(handle, { dbg_output::id, dbg_output::version }, dbg_output::data::to_tuple({ 1, str.c_str() }));
    }
    catch(const std::exception & ex) {
        std::cout << "Error occurred: " << ex.what() << "\n\n";
    }

    if(handle != 0)
        unregister_provider(handle);

    return 0;
}