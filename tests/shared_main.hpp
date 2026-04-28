/**
 * @brief
 * Shared tests initiation
**/

#pragma once

#include "gtest.hpp"
#include <iostream>

#define TESTS_MAIN(STAGE_NAME, BEFORE, AFTER) \
int main(int argc, char **argv) { \
    std::cout << "[JUG TESTS] [" << STAGE_NAME << "] Starting Test Suite..." << std::endl; \
\
    BEFORE \
\
    ::testing::InitGoogleTest(&argc, argv); \
    int result = RUN_ALL_TESTS(); \
\
    AFTER \
\
    std::cout << "[JUG TESTS] [" << STAGE_NAME << "] Finished running tests." << std::endl; \
\
    return result; \
}
