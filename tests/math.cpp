#include <gtest/gtest.h>
#include <cmath>
#include <limits>

// Pretend utility functions we're testing
double safe_sqrt(double x) {
    if (x < 0) {
        throw std::domain_error("Cannot take square root of negative number");
    }
    return std::sqrt(x);
}

bool approximately_equal(double a, double b, double epsilon = 1e-9) {
    return std::abs(a - b) < epsilon;
}

namespace {

    TEST(MathUtils, SquareRootBasic)
    {
        EXPECT_DOUBLE_EQ(safe_sqrt(16.0), 4.0);
        EXPECT_DOUBLE_EQ(safe_sqrt(0.0), 0.0);
        EXPECT_NEAR(safe_sqrt(2.0), 1.41421356237, 1e-10);
    }

    TEST(MathUtils, SquareRootThrowsOnNegative)
    {
        EXPECT_THROW(safe_sqrt(-1.0), std::domain_error);
        EXPECT_THROW(safe_sqrt(-0.0001), std::domain_error);
    }

    TEST(MathUtils, FloatingPointComparison)
    {
        double a = 0.1 + 0.2;
        double b = 0.3;

        // This will FAIL (classic floating point surprise)
        // EXPECT_DOUBLE_EQ(a, b);

        // These are the correct ways:
        EXPECT_NEAR(a, b, 1e-10);
        EXPECT_TRUE(approximately_equal(a, b));
    }

    TEST(MathUtils, SpecialValues)
    {
        EXPECT_TRUE(std::isinf(std::numeric_limits<double>::infinity()));
        EXPECT_TRUE(std::isnan(std::sqrt(-1.0)));
        EXPECT_FALSE(std::isfinite(std::numeric_limits<double>::quiet_NaN()));
    }

    TEST(MathUtils, DISABLED_ThisTestIsDisabledExample)
    {
        // Tests that are temporarily disabled often use DISABLED_ prefix
        FAIL() << "This test should be fixed later";
    }

} // namespace