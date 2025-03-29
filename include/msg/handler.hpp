#pragma once

#include <log/log.hpp>
#include <msg/handler_interface.hpp>

#include <stdx/tuple_algorithms.hpp>
#include <stdx/utility.hpp>

namespace msg {

template <typename Callbacks, typename MsgBase, typename... ExtraCallbackArgs>
struct handler : handler_interface<MsgBase, ExtraCallbackArgs...> {
    Callbacks callbacks{};

    constexpr explicit handler(Callbacks new_callbacks)
        : callbacks{new_callbacks} {}

    auto is_match(MsgBase const &msg) const -> bool final {
        return stdx::any_of(
            [&](auto &callback) { return callback.is_match(msg); }, callbacks);
    }

    auto handle(MsgBase const &msg, ExtraCallbackArgs... args) const
        -> bool final {
        auto const found_valid_callback = stdx::apply(
            [&](auto &...cbs) -> bool {
                return (0u | ... | cbs.handle(msg, args...));
            },
            callbacks);
        if (!found_valid_callback) {
            CIB_ERROR(
                "None of the registered callbacks ({}) claimed this message:",
                stdx::ct<stdx::tuple_size_v<Callbacks>>());
            stdx::for_each([&](auto &callback) { callback.log_mismatch(msg); },
                           callbacks);
        }
        return found_valid_callback;
    }
};

} // namespace msg
