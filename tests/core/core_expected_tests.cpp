#include <drogular/expected.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace {

using drogular::Expected;
using drogular::Unexpected;

TEST(CoreExpected, SuccessAndError) {
    Expected<int, std::string> ok{42};
    EXPECT_TRUE(ok.has_value());
    EXPECT_EQ(*ok, 42);
    EXPECT_EQ(ok.value_or(3), 42);
    EXPECT_THROW(ok.error(), std::logic_error);

    Expected<int, std::string> bad{Unexpected{std::string{"invalid"}}};
    EXPECT_FALSE(bad);
    EXPECT_EQ(bad.error(), "invalid");
    EXPECT_EQ(bad.value_or(3), 3);
    EXPECT_THROW(bad.value(), std::logic_error);
}

TEST(CoreExpected, SameValueAndErrorTypes) {
    Expected<std::string, std::string> ok{std::string{"ok"}};
    Expected<std::string, std::string> bad{Unexpected{std::string{"error"}}};
    EXPECT_EQ(ok.value(), "ok");
    EXPECT_EQ(bad.error(), "error");
}

TEST(CoreExpected, MoveOnlyAndInPlace) {
    Expected<std::unique_ptr<int>, std::string> ok{std::make_unique<int>(7)};
    EXPECT_EQ(**ok, 7);
    auto ptr = std::move(ok).value();
    EXPECT_EQ(*ptr, 7);

    Expected<std::string, int> inPlace{std::in_place, 3, 'a'};
    EXPECT_EQ(inPlace.value(), "aaa");
}

TEST(CoreExpected, VoidResult) {
    Expected<void, std::string> ok;
    EXPECT_TRUE(ok);
    EXPECT_NO_THROW(ok.value());
    EXPECT_THROW(ok.error(), std::logic_error);

    Expected<void, std::string> bad{Unexpected{std::string{"failed"}}};
    EXPECT_FALSE(bad);
    EXPECT_EQ(bad.error(), "failed");
    EXPECT_THROW(bad.value(), std::logic_error);
}

TEST(CoreExpected, CopyAndMove) {
    Expected<std::string, int> first{std::string{"hello"}};
    auto second = first;
    EXPECT_EQ(second.value(), "hello");
    second = Expected<std::string, int>{Unexpected{9}};
    EXPECT_EQ(second.error(), 9);
    second = std::move(first);
    EXPECT_EQ(second.value(), "hello");
}

static_assert(!std::is_copy_constructible_v<Expected<std::unique_ptr<int>, int>>);
static_assert(std::is_move_constructible_v<Expected<std::unique_ptr<int>, int>>);

} // namespace