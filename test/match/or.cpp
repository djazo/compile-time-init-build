#include "test_matcher.hpp"

#include <match/ops.hpp>

#include <stdx/ct_format.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

TEST_CASE("OR fulfils matcher concept", "[match or]") {
    using T = match::or_t<test_matcher, test_matcher>;
    STATIC_REQUIRE(match::matcher<T>);
    STATIC_REQUIRE(match::matcher_for<T, int>);
}

TEST_CASE("OR describes itself", "[match or]") {
    constexpr auto e = test_m<0>{} or test_m<1>{};
    STATIC_REQUIRE(e.describe() ==
                   stdx::ct_format<"({}) or ({})">(test_m<0>{}.describe(),
                                                   test_m<1>{}.describe()));
}

TEST_CASE("OR description flattens", "[match or]") {
    constexpr auto e = test_m<0>{} or test_m<1>{} or test_m<2>{};
    STATIC_REQUIRE(e.describe() == stdx::ct_format<"({}) or ({}) or ({})">(
                                       test_m<0>{}.describe(),
                                       test_m<1>{}.describe(),
                                       test_m<2>{}.describe()));
}

TEST_CASE("OR describes a match", "[match or]") {
    constexpr auto e = test_m<0>{} or test_m<1>{};
    STATIC_REQUIRE(e.describe_match(1) == stdx::ct_format<"({}) or ({})">(
                                              test_m<0>{}.describe_match(1),
                                              test_m<1>{}.describe_match(1)));
}

TEST_CASE("OR match description flattens", "[match or]") {
    constexpr auto e = test_m<0>{} or test_m<1>{} or test_m<2>{};
    STATIC_REQUIRE(
        e.describe_match(1) ==
        stdx::ct_format<"({}) or ({}) or ({})">(test_m<0>{}.describe_match(1),
                                                test_m<1>{}.describe_match(1),
                                                test_m<2>{}.describe_match(1)));
}

TEST_CASE("OR matches correctly", "[match or]") {
    constexpr auto e = test_m<0>{} or test_m<1>{};
    STATIC_REQUIRE(
        std::is_same_v<decltype(e), match::or_t<test_m<0>, test_m<1>> const>);
    STATIC_REQUIRE(e(1));
    STATIC_REQUIRE(not e(0));
}

TEST_CASE("OR simplifies correctly", "[match or]") {
    constexpr auto e = test_matcher{} and test_matcher{};
    STATIC_REQUIRE(std::is_same_v<decltype(e), test_matcher const>);
}

TEST_CASE("any expression simplifies", "[match or]") {
    constexpr auto m = match::any(test_m<0>{}, test_m<1>{}, match::never);
    STATIC_REQUIRE(
        std::is_same_v<decltype(m), match::or_t<test_m<0>, test_m<1>> const>);
}
