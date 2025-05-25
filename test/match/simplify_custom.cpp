#include "test_matcher.hpp"

#include <match/ops.hpp>

#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <type_traits>

using namespace match;

TEST_CASE("custom matcher simplifies (NOT)", "[match simplify]") {
    constexpr auto e = not rel_matcher<std::less<>, 5>{};
    STATIC_REQUIRE(std::is_same_v<decltype(e),
                                  rel_matcher<std::greater_equal<>, 5> const>);
}

TEST_CASE("custom matcher simplifies (AND negative terms)",
          "[match simplify]") {
    constexpr auto e = rel_matcher<std::less<>, 5>{} and
                       rel_matcher<std::greater_equal<>, 5>{};
    STATIC_REQUIRE(std::is_same_v<decltype(e), never_t const>);
}

TEST_CASE("custom matcher simplifies (AND subsumptive terms)",
          "[match simplify]") {
    constexpr auto e =
        rel_matcher<std::less<>, 5>{} and rel_matcher<std::less<>, 3>{};
    STATIC_REQUIRE(
        std::is_same_v<decltype(e), rel_matcher<std::less<>, 3> const>);
}

TEST_CASE("custom matcher simplifies (AND exclusive terms)",
          "[match simplify]") {
    constexpr auto e =
        rel_matcher<std::less<>, 5>{} and rel_matcher<std::greater<>, 4>{};
    STATIC_REQUIRE(std::is_same_v<decltype(e), match::never_t const>);
}

TEST_CASE("custom matcher simplifies (exclusive terms inside AND)",
          "[match simplify]") {
    constexpr auto e = rel_matcher<std::less<>, 5>{} and
                       rel_matcher<std::less<>, 10>{} and
                       rel_matcher<std::greater<>, 4>{};
    STATIC_REQUIRE(std::is_same_v<decltype(e), match::never_t const>);
}

TEST_CASE("custom matcher simplifies (OR)", "[match simplify]") {
    constexpr auto e =
        rel_matcher<std::less<>, 5>{} or rel_matcher<std::greater_equal<>, 5>{};
    STATIC_REQUIRE(std::is_same_v<decltype(e), always_t const>);
}

TEST_CASE("custom matcher simplifies (OR negative terms)", "[match simplify]") {
    constexpr auto e =
        rel_matcher<std::less<>, 5>{} or rel_matcher<std::greater_equal<>, 5>{};
    STATIC_REQUIRE(std::is_same_v<decltype(e), match::always_t const>);
}

TEST_CASE("custom matcher simplifies (OR subsumptive terms)",
          "[match simplify]") {
    constexpr auto e =
        rel_matcher<std::less<>, 5>{} or rel_matcher<std::less<>, 3>{};
    STATIC_REQUIRE(
        std::is_same_v<decltype(e), rel_matcher<std::less<>, 5> const>);
}

TEST_CASE("custom matcher simplifies (OR overlapping terms)",
          "[match simplify]") {
    constexpr auto e =
        rel_matcher<std::less<>, 5>{} or rel_matcher<std::greater<>, 4>{};
    STATIC_REQUIRE(std::is_same_v<decltype(e), match::always_t const>);
}

TEST_CASE("custom matcher simplifies (overlapping terms inside OR)",
          "[match simplify]") {
    constexpr auto e = rel_matcher<std::less<>, 5>{} or
                       rel_matcher<std::less<>, 10>{} or
                       rel_matcher<std::greater<>, 4>{};
    STATIC_REQUIRE(std::is_same_v<decltype(e), match::always_t const>);
}
