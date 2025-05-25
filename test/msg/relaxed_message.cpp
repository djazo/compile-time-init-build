#include <msg/message.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using namespace msg;

using fixed_f =
    field<"fixed", std::uint32_t>::located<at{0_dw, 31_msb, 24_lsb}>;
using auto_f1 = field<"auto1", std::uint32_t>;
using auto_f2 = field<"auto2", std::uint8_t>;

} // namespace

TEST_CASE("message with automatically packed fields", "[relaxed_message]") {
    using defn = relaxed_message<"msg", auto_f1, auto_f2>;
    using expected_defn =
        msg::message<"msg", auto_f1::located<at{0_dw, 31_msb, 0_lsb}>,
                     auto_f2::located<at{1_dw, 7_msb, 0_lsb}>>;
    STATIC_REQUIRE(std::is_same_v<defn, expected_defn>);
}

TEST_CASE("automatically packed fields are sorted by size",
          "[relaxed_message]") {
    using defn = relaxed_message<"msg", auto_f2, auto_f1>;
    using expected_defn =
        msg::message<"msg", auto_f1::located<at{0_dw, 31_msb, 0_lsb}>,
                     auto_f2::located<at{1_dw, 7_msb, 0_lsb}>>;
    STATIC_REQUIRE(std::is_same_v<defn, expected_defn>);
}

TEST_CASE("message with mixed fixed and automatically packed fields",
          "[relaxed_message]") {
    using defn = relaxed_message<"msg", auto_f2, fixed_f, auto_f1>;
    using expected_defn =
        msg::message<"msg", fixed_f, auto_f1::located<at{1_dw, 31_msb, 0_lsb}>,
                     auto_f2::located<at{2_dw, 7_msb, 0_lsb}>>;
    STATIC_REQUIRE(std::is_same_v<defn, expected_defn>);
}

TEST_CASE("message with no automatically packed fields", "[relaxed_message]") {
    using defn = relaxed_message<"msg", fixed_f>;
    using expected_defn = msg::message<"msg", fixed_f>;
    STATIC_REQUIRE(std::is_same_v<defn, expected_defn>);
}

TEST_CASE("auto field with default value", "[relaxed_message]") {
    using defn = relaxed_message<"msg", auto_f1::with_default<42>>;
    using expected_defn = msg::message<
        "msg", auto_f1::located<at{0_dw, 31_msb, 0_lsb}>::with_default<42>>;
    STATIC_REQUIRE(std::is_same_v<defn, expected_defn>);
}

TEST_CASE("auto field with const default value", "[relaxed_message]") {
    using defn = relaxed_message<"msg", auto_f1::with_const_default<42>>;
    using expected_defn =
        msg::message<"msg", auto_f1::located<at{
                                0_dw, 31_msb, 0_lsb}>::with_const_default<42>>;
    STATIC_REQUIRE(std::is_same_v<defn, expected_defn>);
}

TEST_CASE("auto field without default value", "[relaxed_message]") {
    using defn = relaxed_message<"msg", auto_f1::without_default>;
    using expected_defn = msg::message<
        "msg", auto_f1::located<at{0_dw, 31_msb, 0_lsb}>::without_default>;
    STATIC_REQUIRE(std::is_same_v<defn, expected_defn>);
}

namespace {
[[maybe_unused]] constexpr inline struct custom_t {
    template <typename T>
        requires true // more constrained
    CONSTEVAL auto operator()(T &&t) const
        noexcept(noexcept(std::forward<T>(t).query(std::declval<custom_t>())))
            -> decltype(std::forward<T>(t).query(*this)) {
        return std::forward<T>(t).query(*this);
    }

    CONSTEVAL auto operator()(auto &&) const { return 42; }
} custom;
} // namespace

TEST_CASE("message with default empty environment", "[relaxed_message]") {
    using defn = relaxed_message<"msg", auto_f1, auto_f2>;
    STATIC_REQUIRE(custom(defn::env_t{}) == 42);
}

TEST_CASE("message with defined environment", "[message]") {
    using env_t = stdx::make_env_t<custom, 17>;
    using defn = relaxed_message<"msg", env_t, auto_f1, auto_f2>;
    STATIC_REQUIRE(custom(defn::env_t{}) == 17);
}
