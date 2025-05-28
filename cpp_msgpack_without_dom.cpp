#include "cpp_msgpack_without_dom.hpp"

#include "msgpack.h"
#include <limits>



msgpack_reader::~msgpack_reader() {
    static_assert(sizeof(m_unpacked) >= sizeof(msgpack_unpacked), "");

    auto unpacked = reinterpret_cast<msgpack_unpacked*>(m_unpacked);
    msgpack_unpacked_destroy(unpacked);
}

void msgpack_reader::assign(const void* data, const uint32_t size_b) {
    auto unpacked = reinterpret_cast<msgpack_unpacked*>(m_unpacked);
    msgpack_unpacked_destroy(unpacked);
    msgpack_unpacked_init(unpacked);
    m_data = static_cast<const char*>(data);
    m_size_b = size_b;
    m_offset_b = 0;
}

void msgpack_reader::parse(std::function<void(value_t value)> handler) {
    auto unpacked = reinterpret_cast<msgpack_unpacked*>(m_unpacked);
    while (true) {
        auto rc = msgpack_unpack_next(unpacked, m_data, m_size_b, &m_offset_b);
        if (rc != MSGPACK_UNPACK_SUCCESS) {
            break;
        }
        value_t value(&unpacked->data);
        handler(value);
    }
}

bool msgpack_reader::value_t::is_null() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return obj->type == MSGPACK_OBJECT_NIL;
}
bool msgpack_reader::value_t::is_bool() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return obj->type == MSGPACK_OBJECT_BOOLEAN;
}
bool msgpack_reader::value_t::as_bool() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return obj->via.boolean;
}
bool msgpack_reader::value_t::is_uint() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return obj->type == MSGPACK_OBJECT_POSITIVE_INTEGER;
}
uint64_t msgpack_reader::value_t::as_uint() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    switch (obj->type) {
    case MSGPACK_OBJECT_POSITIVE_INTEGER:
        return obj->via.u64;
    case MSGPACK_OBJECT_NEGATIVE_INTEGER:
        return static_cast<uint64_t>(obj->via.i64);
    case MSGPACK_OBJECT_FLOAT32:
    case MSGPACK_OBJECT_FLOAT64:
        return static_cast<uint64_t>(obj->via.f64);
    default:
        assert(false);
        return UINT64_MAX;
    }
}
bool msgpack_reader::value_t::is_int() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    if (obj->type == MSGPACK_OBJECT_POSITIVE_INTEGER
            and obj->via.u64 <= INT64_MAX) {
        return true;
    }
    return obj->type == MSGPACK_OBJECT_NEGATIVE_INTEGER;
}
int64_t msgpack_reader::value_t::as_int() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    switch (obj->type) {
    case MSGPACK_OBJECT_POSITIVE_INTEGER:
        return static_cast<int64_t>(obj->via.u64);
    case MSGPACK_OBJECT_NEGATIVE_INTEGER:
        return obj->via.i64;
    case MSGPACK_OBJECT_FLOAT32:
    case MSGPACK_OBJECT_FLOAT64:
        return static_cast<int64_t>(obj->via.f64);
    default:
        assert(false);
        return INT64_MAX;
    }
}
bool msgpack_reader::value_t::is_float() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return obj->type == MSGPACK_OBJECT_FLOAT32
        or obj->type == MSGPACK_OBJECT_FLOAT64;
}
bool msgpack_reader::value_t::is_float32() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return obj->type == MSGPACK_OBJECT_FLOAT32;
}
bool msgpack_reader::value_t::is_float64() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return obj->type == MSGPACK_OBJECT_FLOAT64;
}
double msgpack_reader::value_t::as_float() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    switch (obj->type) {
    case MSGPACK_OBJECT_POSITIVE_INTEGER:
        return static_cast<double>(obj->via.u64);
    case MSGPACK_OBJECT_NEGATIVE_INTEGER:
        return static_cast<double>(obj->via.i64);
    case MSGPACK_OBJECT_FLOAT32:
    case MSGPACK_OBJECT_FLOAT64:
        return obj->via.f64;
    default:
        assert(false);
        return std::numeric_limits<double>::quiet_NaN();
    }
}
bool msgpack_reader::value_t::is_number() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return obj->type == MSGPACK_OBJECT_POSITIVE_INTEGER
        or obj->type == MSGPACK_OBJECT_NEGATIVE_INTEGER
        or obj->type == MSGPACK_OBJECT_FLOAT32
        or obj->type == MSGPACK_OBJECT_FLOAT64;
}
bool msgpack_reader::value_t::is_string() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return obj->type == MSGPACK_OBJECT_STR;
}
std::string_view msgpack_reader::value_t::as_string() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return std::string_view(obj->via.str.ptr, obj->via.str.size);
}
bool msgpack_reader::value_t::is_array() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return obj->type == MSGPACK_OBJECT_ARRAY;
}
uint32_t msgpack_reader::value_t::array_size() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return obj->via.array.size;
}
void msgpack_reader::value_t::array(
        std::function<void(uint32_t idx, value_t value)> handler) const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    for (uint32_t idx = 0; idx < obj->via.array.size; ++idx) {
        handler(idx, value_t(&obj->via.array.ptr[idx]));
    }
}
bool msgpack_reader::value_t::is_map() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return obj->type == MSGPACK_OBJECT_MAP;
}
uint32_t msgpack_reader::value_t::map_size() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return obj->via.map.size;
}
void msgpack_reader::value_t::map(
        std::function<void(value_t key, value_t value)> handler) const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    for (uint32_t idx = 0; idx < obj->via.array.size; ++idx) {
        const auto& entry = obj->via.map.ptr[idx];
        handler(value_t(&entry.key), value_t(&entry.val));
    }
}
bool msgpack_reader::value_t::is_raw() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return obj->type == MSGPACK_OBJECT_BIN;
}
uint32_t msgpack_reader::value_t::raw_size_b() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return obj->via.bin.size;
}
const uint8_t* msgpack_reader::value_t::raw() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return reinterpret_cast<const uint8_t*>(obj->via.bin.ptr);
}
bool msgpack_reader::value_t::is_ext() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return obj->type == MSGPACK_OBJECT_EXT;
}
int8_t msgpack_reader::value_t::ext_type() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return obj->via.ext.type;
}
uint32_t msgpack_reader::value_t::ext_size_b() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return obj->via.ext.size;
}
const uint8_t* msgpack_reader::value_t::ext() const {
    auto obj = reinterpret_cast<const msgpack_object*>(m_obj);
    return reinterpret_cast<const uint8_t*>(obj->via.ext.ptr);
}



const uint8_t* msgpack_writer::data() const {
    return buffer.data();
}
uint32_t msgpack_writer::size_b() const {
    return static_cast<uint32_t>(buffer.size());
}
void msgpack_writer::clear() {
    buffer.clear();
}

msgpack_writer& msgpack_writer::value(const int64_t number) {
    msgpack_packer packer;
    msgpack_packer_init(&packer, &buffer, msgpack_writer::msgpack_write);
    msgpack_pack_int64(&packer, number);
    return *this;
}

msgpack_writer& msgpack_writer::value(const uint64_t number) {
    msgpack_packer packer;
    msgpack_packer_init(&packer, &buffer, msgpack_writer::msgpack_write);
    msgpack_pack_uint64(&packer, number);
    return *this;
}

msgpack_writer& msgpack_writer::value(const float number) {
    msgpack_packer packer;
    msgpack_packer_init(&packer, &buffer, msgpack_writer::msgpack_write);
    msgpack_pack_float(&packer, number);
    return *this;
}

msgpack_writer& msgpack_writer::value(const double number) {
    msgpack_packer packer;
    msgpack_packer_init(&packer, &buffer, msgpack_writer::msgpack_write);
    msgpack_pack_double(&packer, number);
    return *this;
}

msgpack_writer& msgpack_writer::value(std::nullptr_t) {
    msgpack_packer packer;
    msgpack_packer_init(&packer, &buffer, msgpack_writer::msgpack_write);
    msgpack_pack_nil(&packer);
    return *this;
}

msgpack_writer& msgpack_writer::value(const std::string_view string) {
    msgpack_packer packer;
    msgpack_packer_init(&packer, &buffer, msgpack_writer::msgpack_write);
    msgpack_pack_str_with_body(&packer, string.data(), string.size());
    return *this;
}

#ifdef __cpp_lib_char8_t
msgpack_writer& msgpack_writer::value(const std::u8string_view string) {
    msgpack_packer packer;
    msgpack_packer_init(&packer, &buffer, msgpack_writer::msgpack_write);
    msgpack_pack_str_with_body(&packer, string.data(), string.size());
    return *this;
}
#endif // __cpp_lib_char8_t

msgpack_writer& msgpack_writer::array(const uint32_t size) {
    msgpack_packer packer;
    msgpack_packer_init(&packer, &buffer, msgpack_writer::msgpack_write);
    msgpack_pack_array(&packer, size);
    return *this;
}

msgpack_writer& msgpack_writer::map(const uint32_t size) {
    msgpack_packer packer;
    msgpack_packer_init(&packer, &buffer, msgpack_writer::msgpack_write);
    msgpack_pack_map(&packer, size);
    return *this;
}

msgpack_writer& msgpack_writer::key(const int64_t number) {
    msgpack_packer packer;
    msgpack_packer_init(&packer, &buffer, msgpack_writer::msgpack_write);
    msgpack_pack_int64(&packer, number);
    return *this;
}

msgpack_writer& msgpack_writer::key(const uint64_t number) {
    msgpack_packer packer;
    msgpack_packer_init(&packer, &buffer, msgpack_writer::msgpack_write);
    msgpack_pack_uint64(&packer, number);
    return *this;
}

msgpack_writer& msgpack_writer::key(const std::string_view string) {
    msgpack_packer packer;
    msgpack_packer_init(&packer, &buffer, msgpack_writer::msgpack_write);
    msgpack_pack_str_with_body(&packer, string.data(), string.size());
    return *this;
}

#ifdef __cpp_lib_char8_t
msgpack_writer& msgpack_writer::key(const std::u8string_view string) {
    msgpack_packer packer;
    msgpack_packer_init(&packer, &buffer, msgpack_writer::msgpack_write);
    msgpack_pack_str_with_body(&packer, string.data(), string.size());
    return *this;
}
#endif // __cpp_lib_char8_t

msgpack_writer& msgpack_writer::value_boolean(const bool boolean) {
    msgpack_packer packer;
    msgpack_packer_init(&packer, &buffer, msgpack_writer::msgpack_write);
    if (boolean) {
        msgpack_pack_true(&packer);
    }
    else {
        msgpack_pack_false(&packer);
    }
    return *this;
}

int32_t msgpack_writer::msgpack_write(void* buffer, const char* data, size_t size_b) {
    if (data == nullptr or size_b == 0) {
        return 0;
    }
    try {
        auto& buf = *static_cast<decltype(msgpack_writer::buffer)*>(buffer);
        if (buf.capacity() == 0) {
            buf.reserve(MSGPACK_SBUFFER_INIT_SIZE / 2);
        }
        buf.insert(buf.end(), data, data + size_b);
    }
    catch (...) {
        return -1;
    }
    return 0;
}
