// C++ MsgPack without DOM
//
// C++17(11*) DOM-less parsing and generating MsgPack messages.
//
// Author: Yurii Blok
// License: BSL-1.0
// https://github.com/yurablok/cpp-msgpack-without-dom
// History:
// v0.1 2025-Jun-02     WIP

#pragma once
#ifndef CPP_MSGPACK_WITHOUT_DOM_HPP
#define CPP_MSGPACK_WITHOUT_DOM_HPP
//#include <string>
#include <functional>
#include <type_traits>
#include <vector>

#if defined(_MSVC_LANG) && _MSVC_LANG >= 201703L
#   define CPPMPWD_CPP_LIB_CHARCONV
#elif __cplusplus >= 201703L
#   if defined(__GNUG__)
#       if __GNUC__ >= 8 && __GNUC_MINOR__ >= 1
#           define CPPMPWD_CPP_LIB_CHARCONV
#       endif
#   else
#       define CPPMPWD_CPP_LIB_CHARCONV
#   endif
#endif

#if defined(CPPMPWD_CPP_LIB_CHARCONV)
#   include <string_view>
#else
#   include "string_view.hpp" // https://github.com/martinmoene/string-view-lite
namespace std {
    using string_view = nonstd::string_view;
}
#endif


class msgpack_reader {
public:
    ~msgpack_reader();

    void assign(const void* data, const uint32_t size_b);
    struct value_t;
    void parse(std::function<void(value_t value)> handler);

    struct value_t {
        value_t(const void* obj) : m_obj(obj) {}
        bool is_null() const;
        bool is_bool() const;
        bool as_bool() const;
        bool is_uint() const;
        uint64_t as_uint() const;
        bool is_int() const;
        int64_t as_int() const;
        bool is_float() const;
        bool is_float32() const;
        bool is_float64() const;
        double as_float() const;
        bool is_number() const;
        bool is_string() const;
        std::string_view as_string() const;
        bool is_array() const;
        uint32_t array_size() const;
        void array(std::function<void(uint32_t idx, value_t value)> handler) const;
        bool is_map() const;
        uint32_t map_size() const;
        void map(std::function<void(value_t key, value_t value)> handler) const;
        bool is_raw() const;
        uint32_t raw_size_b() const;
        const uint8_t* raw() const;
        bool is_ext() const;
        int8_t ext_type() const;
        uint32_t ext_size_b() const;
        const uint8_t* ext() const;
    private:
        const void* m_obj;
    };

private:
    alignas(uintptr_t) uint8_t m_unpacked[sizeof(uint64_t) * 4] = {};
    const char* m_data = nullptr;
    size_t m_size_b = 0;
    size_t m_offset_b = 0;
};

class msgpack_writer {
public:
    std::vector<uint8_t> buffer;
    const uint8_t* data() const;
    uint32_t size_b() const;
    void clear();

    msgpack_writer& value(const int64_t number);
    msgpack_writer& value(const uint64_t number);
    msgpack_writer& value(const float number);
    msgpack_writer& value(const double number);
    template <typename bool_t,
        typename = typename std::enable_if<std::is_same<bool_t, bool>::value>::type>
    msgpack_writer& value(const bool_t boolean) { return value_boolean(boolean); }
    msgpack_writer& value(std::nullptr_t = nullptr);
    msgpack_writer& value(const std::string_view string);
# ifdef __cpp_lib_char8_t
    msgpack_writer& value(const std::u8string_view string);
# endif // __cpp_lib_char8_t
    //msgpack_writer& value(const DateTime dateTime);
    msgpack_writer& array(const uint32_t size);
    msgpack_writer& map(const uint32_t size);
    //void raw(const void* data, const uint32_t size_b);
    //void ext(const uint8_t type, const void* data, const uint32_t size_b);

    msgpack_writer& key(const int64_t number);
    msgpack_writer& key(const uint64_t number);
    msgpack_writer& key(const std::string_view string);
# ifdef __cpp_lib_char8_t
    msgpack_writer& key(const std::u8string_view string);
# endif // __cpp_lib_char8_t

private:
    msgpack_writer& value_boolean(const bool boolean);
    static int32_t msgpack_write(void* buffer, const char* data, size_t size_b);
};

#endif // CPP_MSGPACK_WITHOUT_DOM_HPP
