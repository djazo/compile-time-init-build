#pragma once

#include <concepts>
#include <type_traits>

namespace match {
template <typename T>
concept matcher = requires { typename std::remove_cvref_t<T>::is_matcher; };

template <typename T, typename Event>
concept matcher_for = matcher<T> and requires(T const &t, Event const &e) {
    { t(e) } -> std::convertible_to<bool>;
    t.describe();
    t.describe_match(e);
};
} // namespace match
