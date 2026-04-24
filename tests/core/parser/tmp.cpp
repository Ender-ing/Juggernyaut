/**
 * @brief
 * Parser tests
**/

#include "../../gtest.hpp"

TEST(ParserTest, Temporary1) {
    EXPECT_EQ(1, 1);
}

TEST(ParserTest, Temporary1_) {
    EXPECT_EQ(1, 0);
}

TEST(ParserTest, Temporary2) {
    EXPECT_NE(1, 0);
}

TEST(ParserTest, Temporary2_) {
    EXPECT_NE(1, 1);
}
