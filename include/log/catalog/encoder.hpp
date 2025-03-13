#pragma once

#include <log/catalog/builder.hpp>
#include <log/catalog/catalog.hpp>
#include <log/log.hpp>
#include <log/module.hpp>

#include <stdx/ct_string.hpp>
#include <stdx/span.hpp>
#include <stdx/tuple.hpp>
#include <stdx/utility.hpp>

#include <conc/concurrency.hpp>

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <utility>

namespace logging::binary {
namespace detail {
template <typename S, typename... Args> constexpr static auto to_message() {
    constexpr auto s = S::value;
    using char_t = typename std::remove_cv_t<decltype(s)>::value_type;
    return [&]<std::size_t... Is>(std::integer_sequence<std::size_t, Is...>) {
        return sc::message<
            sc::undefined<sc::args<Args...>, char_t, s[Is]...>>{};
    }(std::make_integer_sequence<std::size_t, std::size(s)>{});
}

template <stdx::ct_string S> constexpr static auto to_module() {
    constexpr auto s = std::string_view{S};
    return [&]<std::size_t... Is>(std::integer_sequence<std::size_t, Is...>) {
        return sc::module_string<sc::undefined<void, char, s[Is]...>>{};
    }(std::make_integer_sequence<std::size_t, std::size(s)>{});
}

template <typename S> struct to_message_t {
    template <typename... Args> using fn = decltype(to_message<S, Args...>());
};
} // namespace detail

template <typename Destinations> struct log_writer {
    template <std::size_t N>
    auto operator()(stdx::span<std::uint8_t const, N> msg) -> void {
        stdx::for_each(
            [&]<typename Dest>(Dest &dest) {
                conc::call_in_critical_section<Dest>(
                    [&] { dest.log_by_buf(msg); });
            },
            dests);
    }

    template <std::size_t N>
    auto operator()(stdx::span<std::uint32_t const, N> msg) -> void {
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            stdx::for_each(
                [&]<typename Dest>(Dest &dest) {
                    conc::call_in_critical_section<Dest>(
                        [&] { dest.log_by_args(msg[Is]...); });
                },
                dests);
        }(std::make_index_sequence<N>{});
    }

    auto operator()(auto const &msg) -> void {
        this->operator()(msg.as_const_view().data());
    }

    Destinations dests;
};
template <typename T> log_writer(T) -> log_writer<T>;

template <typename Writer> struct log_handler {
    template <typename Env, typename FilenameStringType,
              typename LineNumberType, typename MsgType>
    auto log(FilenameStringType, LineNumberType, MsgType const &msg) -> void {
        log_msg<Env>(msg);
    }

    template <typename Env, typename Msg> auto log_msg(Msg msg) -> void {
        msg.apply([&]<typename S, typename... Args>(S, Args... args) {
            auto builder = get_builder(Env{});
            constexpr auto L = stdx::to_underlying(get_level(Env{}));
            using Message = typename decltype(builder)::template convert_args<
                detail::to_message_t<S>::template fn, Args...>;
            using Module = decltype(detail::to_module<get_module(Env{})>());
            w(builder.template build<L>(catalog<Message>(), module<Module>(),
                                        args...));
        });
    }

    template <typename Env, auto Version, stdx::ct_string S = "">
    auto log_version() -> void {
        auto builder = get_builder(Env{});
        w(builder.template build_version<Version, S>());
    }

    Writer w;
};

template <typename... TDestinations> struct config {
    using destinations_tuple_t = stdx::tuple<TDestinations...>;
    constexpr explicit config(TDestinations... dests)
        : logger{log_writer{stdx::tuple{std::move(dests)...}}} {}

    log_handler<log_writer<destinations_tuple_t>> logger;
};
template <typename... Ts> config(Ts...) -> config<Ts...>;
} // namespace logging::binary
