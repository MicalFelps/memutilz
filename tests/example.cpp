#include <gtest/gtest.h>
#include <string>
#include <vector>

// Simple helper function we're going to test (pretend it's in your project)
std::string reverse_string(const std::string& s) {
    std::string result = s;
    std::reverse(result.begin(), result.end());
    return result;
}

namespace {

    // Basic positive cases
    TEST(StringUtils, ReverseNormalString)
    {
        EXPECT_EQ(reverse_string("hello"), "olleh");
        EXPECT_EQ(reverse_string("CMake is fun!"), "!nuf si ebmaC");
    }

    // Edge cases
    TEST(StringUtils, ReverseEmptyString)
    {
        EXPECT_EQ(reverse_string(""), "");
    }

    TEST(StringUtils, ReverseSingleCharacter)
    {
        EXPECT_EQ(reverse_string("x"), "x");
    }

    // More interesting case with unicode (even though it's very simple)
    TEST(StringUtils, ReverseUnicode)
    {
        // Note: this is a very basic check - real unicode reversing is more complex
        std::string input = "café";
        EXPECT_EQ(reverse_string(input), "éfac");
    }

    // Demonstrate EXPECT vs ASSERT
    TEST(StringUtils, DemonstrateAssertions)
    {
        std::vector<int> v{ 1, 2, 3 };

        EXPECT_EQ(v.size(), 3);          // continues even if fails
        ASSERT_EQ(v.size(), 3);          // stops this test if fails
        EXPECT_TRUE(v[0] == 1);          // never reached if previous ASSERT failed
    }

} // namespace