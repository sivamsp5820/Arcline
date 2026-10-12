#pragma once

#include <iostream>
#include <cstdlib>

#define ARCLINE_TEST_ASSERT(expr) \
    do { \
        if (!(expr)) { \
            std::cerr << "\n[TEST FAILED] Assertion failed: " #expr "\n  File: " << __FILE__ << ":" << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (false)

#define SONORA_TEST_ASSERT(expr) ARCLINE_TEST_ASSERT(expr)

