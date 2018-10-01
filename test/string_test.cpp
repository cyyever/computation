/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */

#if __has_include(<CppCoreCheck\Warnings.h>)
#include <CppCoreCheck\Warnings.h>
#pragma warning(disable: ALL_CPPCORECHECK_WARNINGS)
#endif

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <iostream>

#include "../src/string.hpp"

using namespace cyy::computation;
TEST_CASE("KMP") {
  SUBCASE("match") {

    auto str = "abababaab";
    auto word = "ababaa";

    auto p = KMP<char>(word, str);
    REQUIRE(!p.empty());
    CHECK(p == word);
  }

  SUBCASE("dismatch") {
    auto str = "abababbaa";
    auto word = "ababaa";
    auto p = KMP<char>(word, str);
    CHECK(p.empty());
  }
}

TEST_CASE("Aho_Corasick") {
  SUBCASE("dismatch") {
    auto str = "abababaab";
    std::vector<std::string_view> words = {"aaa", "abaaa", "ababaaa"};

    CHECK(Aho_Corasick<char>(words, str).empty());
  }

  SUBCASE("match") {
    auto str = "abaa";
    std::vector<std::string_view> words = {"aa", "abaaa"};

    auto p = Aho_Corasick<char>(words, str);
    REQUIRE(!p.empty());
    CHECK(p == words[0]);
  }
}
