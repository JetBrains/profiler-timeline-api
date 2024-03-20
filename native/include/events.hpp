#pragma once

#include <cstdint>
#include <tuple>

namespace events {

////////////////////////
//// Event manifest ////
////////////////////////

#define PROVIDER_BEGIN(_name, ...) \
    struct _name { \
        static inline constexpr const char * name = #_name; \
        static inline constexpr GUID guid##__VA_ARGS__;

#define PROVIDER_END };

#define EVENTS struct events

#define EVENT(_name, _id, _version, ...) \
    struct _name {        \
        static constexpr uint16_t id = _id; \
        static constexpr uint8_t version = _version; \
        struct data { \
            __VA_ARGS__ \
        }; \
    };

#define TO_TUPLE(...) \
    static constexpr auto to_tuple(const data & d) { \
        const auto &[__VA_ARGS__] = d; \
        return std::make_tuple(__VA_ARGS__); \
    }

}
