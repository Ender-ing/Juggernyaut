/**
 * @brief
 * Compiler Flag tests
**/

#include "../../gtest.hpp"

TEST(CompilerFlags, Temporary1) {
    EXPECT_EQ(1, 1);
}

TEST(CompilerFlags, Temporary2) {
    EXPECT_NE(1, 0);
}
