/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <iostream>

#include "../src/regex.hpp"
#include "../src/set_alphabet.hpp"

using namespace cyy::lang;
TEST_CASE("simulate regex") {
    symbol_string expr = {'a', '*'};
  regex reg("ASCII", expr);
}
