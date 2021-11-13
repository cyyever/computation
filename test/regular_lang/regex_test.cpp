/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */
#if __has_include(<CppCoreCheck\Warnings.h>)
#include <CppCoreCheck\Warnings.h>
#pragma warning(disable : ALL_CPPCORECHECK_WARNINGS)
#endif
#include <doctest/doctest.h>

#include "../../src/exception.hpp"
#include "../../src/regular_lang/regex.hpp"

using namespace cyy::computation;

TEST_CASE("parse regex and to NFA") {

  SUBCASE("empty string") {
    symbol_string expr = U"";
    regex reg("ab_set", expr);
    NFA nfa({0, 1}, "ab_set", 0,
            {
                {{0, 'a'}, {1}},
            },
            {0});
    reg.to_CFG();
  }

  SUBCASE("one symbol") {
    symbol_string expr = U"a";
    regex reg("ab_set", expr);

    NFA nfa({0, 1}, "ab_set", 0,
            {
                {{0, 'a'}, {1}},
            },
            {1});

    CHECK(nfa == reg.to_NFA());
    reg.to_CFG();
  }

  SUBCASE("union") {
    symbol_string expr = U"a|b";
    regex reg("ab_set", expr);

    NFA nfa({0, 1, 2, 3, 4, 5}, "ab_set", 0,
            {
                {{1, 'a'}, {2}},
                {{3, 'b'}, {4}},
            },
            {5},
            {
                {0, {1, 3}},
                {2, {5}},
                {4, {5}},
            });

    CHECK(nfa == reg.to_NFA());
    reg.to_CFG();
  }

  SUBCASE("kleene_closure_node") {
    symbol_string expr = U"(a|b)*";
    regex reg("ab_set", expr);

    NFA nfa({0, 1, 2, 3, 4, 5, 6, 7}, "ab_set", 0,
            {
                {{2, 'a'}, {3}},
                {{4, 'b'}, {5}},
            },
            {7},
            {
                {0, {1, 7}},
                {1, {2, 4}},
                {3, {6}},
                {5, {6}},
                {6, {1, 7}},
            });

    CHECK(nfa == reg.to_NFA());
    reg.to_CFG();
  }

  SUBCASE("all together") {

    symbol_string expr = U"(a|b)*abb";
    regex reg("ab_set", expr);

    NFA nfa({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, "ab_set", 0,
            {

                {{2, 'a'}, {3}},
                {{4, 'b'}, {5}},
                {{7, 'a'}, {8}},
                {{8, 'b'}, {9}},
                {{9, 'b'}, {10}},
            },
            {10},
            {
                {0, {1, 7}},
                {1, {2, 4}},
                {3, {6}},
                {5, {6}},
                {6, {1, 7}},
            });

    CHECK(nfa == reg.to_NFA());
  }
}

TEST_CASE("parse regex and to DFA") {
  symbol_string expr = U"(a|b)*abb";
  regex reg("ab_set", expr);

  DFA dfa({0, 1, 2, 3}, "ab_set", 0,
          {
              {{0, 'a'}, 1},
              {{0, 'b'}, 0},
              {{1, 'a'}, 1},
              {{1, 'b'}, 2},
              {{2, 'a'}, 1},
              {{2, 'b'}, 3},
              {{3, 'a'}, 1},
              {{3, 'b'}, 0},
          },
          {3});
  CHECK(dfa == reg.to_DFA());

  CHECK(dfa.recognize(U"abb"));
  CHECK(dfa.recognize(U"aabb"));
  CHECK(dfa.recognize(U"babb"));
  CHECK(!dfa.recognize(U"bb"));
  CHECK(!dfa.recognize(U"ab"));
}

TEST_CASE("parse extended regex and to NFA") {

  SUBCASE("*") {
    symbol_string expr = U"a*";
    regex reg("ab_set", expr);

    auto nfa = reg.to_NFA();
    auto dfa = reg.to_DFA().minimize().first;

    for (auto const &test_string : {U"", U"a", U"aa"}) {
      CHECK(nfa.recognize(test_string));
      CHECK(dfa.recognize(test_string));
    }
  }

  SUBCASE("+") {
    symbol_string expr = U"a+";
    regex reg("ab_set", expr);

    auto nfa = reg.to_NFA();
    auto dfa = reg.to_DFA().minimize().first;

    CHECK(!nfa.recognize(U""));
    CHECK(!dfa.recognize(U""));
    for (auto const &test_string : {U"a", U"aa"}) {
      CHECK(nfa.recognize(test_string));
      CHECK(dfa.recognize(test_string));
    }
  }

  SUBCASE("?") {
    symbol_string expr = U"a?";
    regex reg("ab_set", expr);

    auto nfa = reg.to_NFA();
    auto dfa = reg.to_DFA().minimize().first;

    for (auto const &test_string : {U"", U"a"}) {
      CHECK(nfa.recognize(test_string));
      CHECK(dfa.recognize(test_string));
    }
    CHECK(!nfa.recognize(U"aa"));
    CHECK(!dfa.recognize(U"aa"));
  }

  SUBCASE(".") {
    symbol_string expr = U".";
    regex reg("printable-ASCII", expr);

    auto nfa = reg.to_NFA();
    auto dfa = reg.to_DFA().minimize().first;

    CHECK(nfa.recognize(U"a"));
    CHECK(dfa.recognize(U"a"));
    CHECK(nfa.recognize(U"b"));
    CHECK(dfa.recognize(U"b"));
    CHECK(!nfa.recognize(U"\n"));
    CHECK(!dfa.recognize(U"\n"));
    CHECK(!nfa.recognize(U"\r"));
    CHECK(!dfa.recognize(U"\r"));
  }

  SUBCASE("[^a]") {
    symbol_string expr = U"[^a]";
    regex reg("ab_set", expr);

    auto nfa = reg.to_NFA();
    auto dfa = reg.to_DFA().minimize().first;

    CHECK(nfa.recognize(U"b"));
    CHECK(dfa.recognize(U"b"));
  }

  SUBCASE("[^]") {
    symbol_string expr = U"[^]";
    CHECK_THROWS_AS(regex("ab_set", expr),
                    const cyy::computation::exception::no_regular_expression &);
  }

  SUBCASE("[-]") {
    symbol_string expr = U"[\\-]";
    regex reg("printable-ASCII", expr);

    auto nfa = reg.to_NFA();
    auto dfa = reg.to_DFA().minimize().first;

    CHECK(nfa.recognize(U"-"));
    CHECK(dfa.recognize(U"-"));

    CHECK_THROWS_AS(regex("ab_set", expr),
                    const cyy::computation::exception::no_regular_expression &);
  }

  SUBCASE("[^-]") {
    symbol_string expr = U"[^\\-]";
    regex reg("printable-ASCII", expr);

    auto nfa = reg.to_NFA();
    auto dfa = reg.to_DFA().minimize().first;

    CHECK(!nfa.recognize(U"-"));
    CHECK(!dfa.recognize(U"-"));
    CHECK(nfa.recognize(U"a"));
    CHECK(dfa.recognize(U"a"));
    CHECK_THROWS_AS(regex("ab_set", expr),
                    const cyy::computation::exception::no_regular_expression &);
  }

  SUBCASE("[a-c]") {
    symbol_string expr = U"[a-c]";
    regex reg("printable-ASCII", expr);

    auto nfa = reg.to_NFA();
    auto dfa = reg.to_DFA().minimize().first;

    CHECK(nfa.recognize(U"a"));
    CHECK(dfa.recognize(U"a"));
    CHECK(nfa.recognize(U"b"));
    CHECK(dfa.recognize(U"b"));
    CHECK(nfa.recognize(U"c"));
    CHECK(dfa.recognize(U"c"));
    CHECK(!nfa.recognize(U"d"));
    CHECK(!dfa.recognize(U"d"));
    CHECK(!nfa.recognize(U"-"));
    CHECK(!dfa.recognize(U"-"));
  }
  SUBCASE("[a-b]") {
    symbol_string expr = U"[a-b]";
    regex reg("ab_set", expr);

    auto nfa = reg.to_NFA();
    auto dfa = reg.to_DFA().minimize().first;

    CHECK(nfa.recognize(U"a"));
    CHECK(dfa.recognize(U"a"));
    CHECK(nfa.recognize(U"b"));
    CHECK(dfa.recognize(U"b"));
    CHECK(!nfa.recognize(U"c"));
    CHECK(!dfa.recognize(U"c"));
  }

  SUBCASE("[a-c\\-z]") {
    symbol_string expr = U"[a-c\\-z]";
    regex reg("printable-ASCII", expr);

    auto nfa = reg.to_NFA();
    auto dfa = reg.to_DFA().minimize().first;

    CHECK(nfa.recognize(U"a"));
    CHECK(dfa.recognize(U"a"));
    CHECK(nfa.recognize(U"b"));
    CHECK(dfa.recognize(U"b"));
    CHECK(nfa.recognize(U"c"));
    CHECK(dfa.recognize(U"c"));
    CHECK(!nfa.recognize(U"d"));
    CHECK(!dfa.recognize(U"d"));
    CHECK(nfa.recognize(U"-"));
    CHECK(dfa.recognize(U"-"));
    CHECK(nfa.recognize(U"z"));
    CHECK(dfa.recognize(U"z"));
    CHECK(!nfa.recognize(U"y"));
    CHECK(!dfa.recognize(U"y"));
  }
}
