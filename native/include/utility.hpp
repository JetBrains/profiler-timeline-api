#pragma once

#include <cstdint>
#include <system_error>
#include <type_traits>

namespace events {

    //////////////////////
    //// Result types ////
    /////////////////////

    struct error_t {
        std::error_code value;
        const char * message;
    };

    template <class T = void>
    class result_t {
        union {
            T result;
            error_t error;
        };
        bool is_error;
    public:
        result_t() : result_t(0, "Unknown error") {}
        result_t(T result) : result{ std::move(result) }, is_error{ false } {}
        result_t(std::error_code error, const char * message)
            : error{ error, message }
            , is_error{ true }
        {}
        result_t(uint32_t error, const char * message) : result_t{ { static_cast<int>(error), std::system_category() }, message }
        {}

        explicit operator bool() const noexcept { return !is_error; }

        const T & get() const {
            if(is_error)
                throw std::system_error(error.value, error.message);
            return result;
        }

        operator const T & () const { return get(); }

        template <class Func>
        auto then(Func && func) && -> std::invoke_result_t<std::decay_t<Func>, T> {
            if(this->operator bool())
                return std::invoke(std::forward<Func>(func), std::move(result));
            return { std::move(error.value), error.message };
        }
    };

    template <>
    class result_t<void> {
        error_t error;
    public:
        result_t(uint32_t error = 0, const char * message = nullptr) : error{ { static_cast<int>(error), std::system_category() }, message } {}
        explicit operator bool() const noexcept { return error.message == nullptr && error.value.value() == 0; }
    };

}
