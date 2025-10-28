// test_palindrome.cpp
#include <gtest/gtest.h>
#include <string>
#include <optional>
using std::string;

enum class CheckErr { Ok, InvalidInput };
struct CheckResult { bool is_palindrome{}; CheckErr err{ CheckErr::Ok }; };
std::optional<std::string> parse_number_string(std::string_view raw);
bool is_palindrome_digits(std::string_view d);
CheckResult check_number_palindrome(std::string_view user_input);

// Базовые позитивные кейсы
TEST(Palindrome, SimplePositive) {
    EXPECT_TRUE(check_number_palindrome("0").is_palindrome);
    EXPECT_TRUE(check_number_palindrome("5").is_palindrome);
    EXPECT_TRUE(check_number_palindrome("11").is_palindrome);
    EXPECT_TRUE(check_number_palindrome("121").is_palindrome);
    EXPECT_TRUE(check_number_palindrome("+1221").is_palindrome);
    EXPECT_TRUE(check_number_palindrome("-1221").is_palindrome); // знак игнорируется
    EXPECT_TRUE(check_number_palindrome("000").is_palindrome);   // ведущие нули допустимы
}

// негативный, позитивный
TEST(Palindrome, SimpleNegative) {
    EXPECT_FALSE(check_number_palindrome("12").is_palindrome);
    EXPECT_TRUE(check_number_palindrome("12021").is_palindrome);
}

// Валидация ввода
TEST(Input, Invalid) {
    EXPECT_EQ(check_number_palindrome("").err, CheckErr::InvalidInput);
    EXPECT_EQ(check_number_palindrome("+").err, CheckErr::InvalidInput);
    EXPECT_EQ(check_number_palindrome("  -   ").err, CheckErr::InvalidInput);
    EXPECT_EQ(check_number_palindrome("12a21").err, CheckErr::InvalidInput);
    EXPECT_EQ(check_number_palindrome("++11").err, CheckErr::InvalidInput);
}

// наличие пробелов
TEST(Input, TrimAndSign) {
    auto r = check_number_palindrome("   -0123210   ");
    ASSERT_EQ(r.err, CheckErr::Ok);
    EXPECT_TRUE(r.is_palindrome);
}

// число большой длины
TEST(Palindrome, LongNumber) {
    std::string big(10000, '9');           // 10k девяток — палиндром
    EXPECT_TRUE(check_number_palindrome(big).is_palindrome);
}