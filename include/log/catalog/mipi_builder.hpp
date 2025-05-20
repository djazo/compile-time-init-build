#pragma once

#include <log/catalog/catalog.hpp>
#include <log/catalog/mipi_messages.hpp>

#include <stdx/compiler.hpp>
#include <stdx/type_traits.hpp>
#include <stdx/utility.hpp>

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>

namespace logging::mipi {
template <typename T>
concept signed_packable = std::signed_integral<stdx::underlying_type_t<T>> and
                          sizeof(T) <= sizeof(std::int64_t);

template <typename T>
concept unsigned_packable =
    std::unsigned_integral<stdx::underlying_type_t<T>> and
    sizeof(T) <= sizeof(std::int64_t);

template <typename T>
concept packable = signed_packable<T> or unsigned_packable<T>;

template <typename T> struct encoding;

template <signed_packable T> struct encoding<T> {
    using encode_t = stdx::conditional_t<sizeof(T) <= sizeof(std::int32_t),
                                         encode_32<T>, encode_64<T>>;
    using pack_t = stdx::conditional_t<sizeof(T) <= sizeof(std::int32_t),
                                       std::int32_t, std::int64_t>;
};

template <unsigned_packable T> struct encoding<T> {
    using encode_t = stdx::conditional_t<sizeof(T) <= sizeof(std::int32_t),
                                         encode_u32<T>, encode_u64<T>>;
    using pack_t = stdx::conditional_t<sizeof(T) <= sizeof(std::uint32_t),
                                       std::uint32_t, std::uint64_t>;
};

template <packable T> using pack_as_t = typename encoding<T>::pack_t;
template <packable T> using encode_as_t = typename encoding<T>::encode_t;

template <typename> struct builder;

template <> struct builder<defn::short32_msg_t> {
    template <auto Level> static auto build(string_id id, module_id) {
        using namespace msg;
        return owning<defn::short32_msg_t>{"payload"_field = id};
    }
};

template <typename Storage> struct catalog_builder {
    template <auto Level, packable... Ts>
    static auto build(string_id id, module_id m, Ts... args) {
        using namespace msg;
        defn::catalog_msg_t::owner_t<Storage> message{"severity"_field = Level,
                                                      "module_id"_field = m};

        using V = typename Storage::value_type;
        constexpr auto header_size = defn::catalog_msg_t::size<V>::value;

        auto const pack_arg = []<typename T>(V *p, T arg) -> V * {
            auto const packed = stdx::to_le(stdx::as_unsigned(
                static_cast<pack_as_t<T>>(stdx::to_underlying(arg))));
            std::memcpy(p, &packed, sizeof(packed));
            return p + stdx::sized8{sizeof(packed)}.in<V>();
        };

        auto dest = &message.data()[header_size];
        dest = pack_arg(dest, stdx::to_le(id));
        ((dest = pack_arg(dest, args)), ...);

        return message;
    }
};

template <> struct builder<defn::catalog_msg_t> {
    template <auto Level, typename... Ts>
    static auto build(string_id id, module_id m, Ts... args) {
        using namespace msg;
        if constexpr ((0 + ... + sizeof(Ts)) <= sizeof(std::uint32_t) * 2) {
            constexpr auto header_size =
                defn::catalog_msg_t::size<std::uint32_t>::value;
            constexpr auto payload_size =
                stdx::sized8{(sizeof(id) + ... + sizeof(pack_as_t<Ts>))}
                    .in<std::uint32_t>();
            using storage_t =
                std::array<std::uint32_t, header_size + payload_size>;
            return catalog_builder<storage_t>{}.template build<Level>(id, m,
                                                                      args...);
        } else {
            constexpr auto header_size =
                defn::catalog_msg_t::size<std::uint8_t>::value;
            constexpr auto payload_size =
                (sizeof(id) + ... + sizeof(pack_as_t<Ts>));
            using storage_t =
                std::array<std::uint8_t, header_size + payload_size>;
            return catalog_builder<storage_t>{}.template build<Level>(id, m,
                                                                      args...);
        }
    }
};

template <> struct builder<defn::compact32_build_msg_t> {
    template <auto Version> static auto build() {
        using namespace msg;
        return owning<defn::compact32_build_msg_t>{"build_id"_field = Version};
    }
};

template <> struct builder<defn::compact64_build_msg_t> {
    template <auto Version> static auto build() {
        using namespace msg;
        return owning<defn::compact64_build_msg_t>{"build_id"_field = Version};
    }
};

template <> struct builder<defn::normal_build_msg_t> {
    template <auto Version, stdx::ct_string S> static auto build() {
        using namespace msg;
        constexpr auto header_size =
            defn::normal_build_msg_t::size<std::uint8_t>::value;
        constexpr auto payload_len = S.size() + sizeof(std::uint64_t);
        using storage_t = std::array<std::uint8_t, header_size + payload_len>;

        defn::normal_build_msg_t::owner_t<storage_t> message{
            "payload_len"_field = payload_len};
        auto dest = &message.data()[header_size];

        auto const ver = stdx::to_le(static_cast<std::uint64_t>(Version));
        std::memcpy(dest, &ver, sizeof(std::uint64_t));
        dest += sizeof(std::uint64_t);
        std::copy_n(std::cbegin(S.value), S.size(), dest);
        return message;
    }
};

struct default_builder {
    template <auto Level, packable... Ts>
    static auto build(string_id id, module_id m, Ts... args) {
        if constexpr (sizeof...(Ts) == 0u) {
            return builder<defn::short32_msg_t>{}.template build<Level>(id, m);
        } else {
            return builder<defn::catalog_msg_t>{}.template build<Level>(
                id, m, args...);
        }
    }

    template <auto Version, stdx::ct_string S = ""> auto build_version() {
        using namespace msg;
        if constexpr (S.empty() and stdx::bit_width(Version) <= 22) {
            return builder<defn::compact32_build_msg_t>{}
                .template build<Version>();
        } else if constexpr (S.empty() and stdx::bit_width(Version) <= 54) {
            return builder<defn::compact64_build_msg_t>{}
                .template build<Version>();
        } else {
            return builder<defn::normal_build_msg_t>{}
                .template build<Version, S>();
        }
    }

    template <template <typename...> typename F, typename... Args>
    using convert_args = F<encode_as_t<Args>...>;
};
} // namespace logging::mipi
