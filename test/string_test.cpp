/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <iostream>

#include "../src/string.hpp"

using namespace cyy::lang;
TEST_CASE("KMP") {
  SUBCASE("KMP match") {

    auto str="abababaab";
    auto word="ababaa";

    auto p=KMP<char>(word,str);
    CHECK(!p.empty());
    CHECK(p==word);
  }

  SUBCASE("KMP mismatch") {
    auto str="abababbaa";
    auto word="ababaa";
    auto p=KMP<char>(word,str);
    CHECK(p.empty());
}
}

