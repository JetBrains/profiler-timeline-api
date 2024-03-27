#include "../include/etw_provider_api.hpp"
#include "../include/debug_output.hpp"
#include <string>

using namespace events;
using namespace events::etw;

template <class T>
bool check_result(const result_t<T> & result, char ** error) noexcept {
    try {
        if(const error_t * err = result.try_get_error()) {
            std::string str = std::string(err->message) + ": " + err->value.message();
            char * message = new char[str.size() + 1];
            std::memcpy(message, str.c_str(), str.size() + 1);
            return false;
        }
    }
    catch(...) { return false; }
    return true;
}

/////////////
//// API ////
/////////////

extern "C" __declspec(dllexport)
void ReleaseString(char * error)
{
    delete [] error;
}

using OnProviderEnableCallback = void(*)(bool isEnabled, uint64_t matchAnyKeyword, uint64_t matchAllKeyword);

void __stdcall OnProviderEnableTrampoline(bool isEnabled, uint64_t matchAnyKeyword, uint64_t matchAllKeyword, void * onEnableCallback)
{
    reinterpret_cast<OnProviderEnableCallback>(onEnableCallback)(isEnabled, matchAnyKeyword, matchAllKeyword);
}

extern "C" __declspec(dllexport)
uint64_t __cdecl RegisterProvider(uint64_t onEnableCallback, char ** error)
{
    result_t<uint64_t> handle = register_provider<&OnProviderEnableTrampoline>(JetBrains_Common_DebugOutput::guid, reinterpret_cast<void *>(onEnableCallback));
    return check_result(handle, error) ? handle : 0;
}

extern "C" __declspec(dllexport)
void __cdecl UnregisterProvider(uint64_t providerHandle, char ** error)
{
    check_result(unregister_provider(providerHandle), error);
}

extern "C" __declspec(dllexport)
void WriteDebugOutput(uint64_t providerHandle, const char * str, char ** error)
{
    using dbg_output = JetBrains_Common_DebugOutput::DebugOutput;
    check_result(write_event(providerHandle, { dbg_output::id, dbg_output::version }, dbg_output::data::to_tuple({ static_cast<uint32_t>(GetCurrentProcessId()), str })), error);
}
