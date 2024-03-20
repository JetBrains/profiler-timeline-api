#pragma once

#include "utility.hpp"
#include <tuple>
#include <type_traits>
#include <utility>
#include <windows.h>
#include <evntprov.h>
#include <winerror.h>

#include <string>


namespace events::etw {

    //////////////////////////////////
    //// Provider Enable Callback ////
    //////////////////////////////////

    using provider_enabled_callback = void(*)(bool, uint64_t match_any_keyword, uint64_t match_all_keyword, void * context);

    namespace detail {

        template <provider_enabled_callback provider_enabled_cb>
        inline void __stdcall on_provider_enabled(LPCGUID, ULONG is_enabled, UCHAR, ULONGLONG match_any_keyword, ULONGLONG match_all_keyword, PEVENT_FILTER_DESCRIPTOR, PVOID callback_context) noexcept {
            try {
                provider_enabled_cb(!!is_enabled, match_any_keyword, match_all_keyword, callback_context);
            }
            catch(...) {}
        }

        template <typename...>
        inline constexpr bool false_v = false;
    }

    /////////////////
    //// ETW API ////
    /////////////////
    
    template <provider_enabled_callback provider_enabled_cb = nullptr>
    inline result_t<uint64_t> register_provider(const GUID & provider_guid, void * cb_context = nullptr) noexcept {
        static_assert(sizeof(REGHANDLE) <= sizeof(uint64_t));

        REGHANDLE reg_handle = 0;
        ULONG status = provider_enabled_cb != nullptr
            ? EventRegister(
                &provider_guid,
                &detail::on_provider_enabled<provider_enabled_cb>,
                &cb_context, &reg_handle)
            : EventRegister(&provider_guid, nullptr, nullptr, &reg_handle);

        if(status != ERROR_SUCCESS)
            return { static_cast<uint32_t>(status), "Unable to register provider" };
        return static_cast<uint64_t>(reg_handle);
    }

    inline result_t<> unregister_provider(uint64_t provider_handle) noexcept {
        return static_cast<uint32_t>(EventUnregister(static_cast<REGHANDLE>(provider_handle)));
    }

    struct event_descriptor {
        uint16_t id;
        uint8_t version;
        uint64_t keyword = 0;
        uint8_t channel = 0;
        uint8_t level = 0;
        uint8_t opcode = 0;
        uint16_t task = 0;
    };

    struct event_data_field {
        const uint8_t * data;
        uint32_t size;
    };

    template <uint32_t data_sz>
    inline result_t<> write_event(uint64_t provider_handle, const event_descriptor & descriptor, event_data_field (&data)[data_sz]) noexcept {
        EVENT_DATA_DESCRIPTOR descrs[data_sz];
        for(uint32_t i = 0; i < data_sz; ++i)
            EventDataDescCreate(&descrs[i], data[i].data, static_cast<ULONG>(data[i].size));

        EVENT_DESCRIPTOR ed{
            descriptor.id,
            descriptor.version,
            descriptor.channel,
            descriptor.level,
            descriptor.opcode,
            descriptor.task,
            descriptor.keyword };

        return EventWrite(static_cast<REGHANDLE>(provider_handle), &ed, static_cast<ULONG>(data_sz), descrs);
    }

    template <class... Args>
    inline result_t<> write_event(uint64_t provider_handle, const event_descriptor & descriptor, const std::tuple<Args...> & data) noexcept {
        auto visitor = [](const auto & arg) mutable -> event_data_field {
            using type = std::decay_t<decltype(arg)>;
            if constexpr(std::disjunction_v<std::is_same<type, const char *>, std::is_same<type, const wchar_t *>, std::is_same<type, const char16_t *>, std::is_same<type, const char32_t *>>) {
                using char_type = std::remove_const_t<std::remove_pointer_t<type>>;
                uint32_t size = static_cast<uint32_t>(std::char_traits<char_type>::length(arg) + 1) * sizeof(char_type);
                return { reinterpret_cast<const uint8_t *>(arg), size };
            }
            else if constexpr(std::is_trivial_v<type> && std::is_standard_layout_v<type>)
                return { reinterpret_cast<const uint8_t *>(&arg), sizeof(type) };
            else {
                // std::string a = arg;
                static_assert(detail::false_v<type>, "Unsupported type: only trivial types and C-strings are supported");
                return {};
            }
        };
        return std::apply([&](const auto &... args) {
            event_data_field result[] = { visitor(args)... };
            return write_event(provider_handle, descriptor, result);
        }, data);
    }

}
