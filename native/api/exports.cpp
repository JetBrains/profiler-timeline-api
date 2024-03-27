#include "../include/etw_provider_api.hpp"
#include "../include/debug_output.hpp"
#include <string>

using namespace events;
using namespace events::etw;

#pragma pack(push, 1)
struct error_info {
    int32_t code;
    const char * message;

    alignas(std::string) const std::string priv_message;

    error_info(int32_t c, std::string m) noexcept : code(c), message(nullptr), priv_message(std::move(m)) {
        message = priv_message.c_str();
    }
};
#pragma pack(pop)

template <class T>
bool check_result(const result_t<T> & result, error_info ** error) {
    if(const error_t * err = result.try_get_error()) {
        *error = new error_info{ err->value.value(), err->message };
        return false;
    }
    return true;
}

/////////////
//// API ////
/////////////

extern "C" __declspec(dllexport)
void ReleaseErrorInfo(error_info * error)
{
    delete error;
}

using OnProviderEnableCallback = void(*)(bool isEnabled, uint64_t matchAnyKeyword, uint64_t matchAllKeyword);

void __stdcall OnProviderEnableTrampoline(bool isEnabled, uint64_t matchAnyKeyword, uint64_t matchAllKeyword, void * onEnableCallback)
{
    reinterpret_cast<OnProviderEnableCallback>(onEnableCallback)(isEnabled, matchAnyKeyword, matchAllKeyword);
}

extern "C" __declspec(dllexport)
uint64_t __cdecl RegisterProvider(GUID providerGuid, uint64_t onEnableCallback, error_info ** error)
{
    result_t<uint64_t> handle = register_provider<&OnProviderEnableTrampoline>(providerGuid, reinterpret_cast<void *>(onEnableCallback));
    return check_result(handle, error) ? handle : 0;
}

extern "C" __declspec(dllexport)
bool __cdecl UnregisterProvider(uint64_t providerHandle, error_info ** error)
{
    return check_result(unregister_provider(providerHandle), error);
}

extern "C" __declspec(dllexport)
bool WriteDebugOutput(uint64_t providerHandle, const char * str, error_info ** error)
{
    using dbg_output = JetBrains_Common_DebugOutput::DebugOutput;
    return check_result(write_event(providerHandle, { dbg_output::id, dbg_output::version }, dbg_output::data::to_tuple({ static_cast<uint32_t>(GetCurrentProcessId()), str })), error);
}
