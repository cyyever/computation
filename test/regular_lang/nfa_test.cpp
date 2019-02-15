/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */
#if __has_include(<CppCoreCheck\Warnings.h>)
#include <CppCoreCheck\Warnings.h>
#pragma warning(disable : ALL_CPPCORECHECK_WARNINGS)
#endif
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "../../src/regular_lang/nfa.hpp"

using namespace cyy::computation;
TEST_CASE("simulate NFA") {
  auto epsilon = ALPHABET::get("ab_set")->get_epsilon();

  NFA nfa({0, 1, 2, 3, 4}, "ab_set", 0,
          {

              {{epsilon, 0}, {1, 3}},

              {{'a', 1}, {2}},
              {{'a', 2}, {2}},

              {{'b', 3}, {4}},
              {{'b', 4}, {4}},
          },
          {2, 4});

  SUBCASE("a") {
    symbol_string str = {'a'};
    CHECK(nfa.simulate(str));
  }

  SUBCASE("aa") {
    symbol_string str = {'a', 'a'};
    CHECK(nfa.simulate(str));
  }

  SUBCASE("b") {
    symbol_string str = {'b'};
    CHECK(nfa.simulate(str));
  }

  SUBCASE("bb") {
    symbol_string str = {'b', 'b'};
    CHECK(nfa.simulate(str));
  }
  SUBCASE("ab") {
    symbol_string str = {'a', 'b'};
    CHECK(!nfa.simulate(str));
  }
}

TEST_CASE("NFA to DFA") {
  auto epsilon = ALPHABET::get("ab_set")->get_epsilon();

  NFA nfa({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, "ab_set", 0,
          {

              {{
                   epsilon,
                   0,
               },
               {1, 7}},
              {{
                   epsilon,
                   1,
               },
               {2, 4}},
              {{'a', 2}, {3}},
              {{
                   epsilon,
                   3,
               },
               {6}},
              {{'b', 4}, {5}},
              {{
                   epsilon,
                   5,
               },
               {6}},
              {{
                   epsilon,
                   6,
               },
               {1, 7}},
              {{'a', 7}, {8}},
              {{'b', 8}, {9}},
              {{'b', 9}, {10}},
          },
          {10});

  DFA dfa({0, 1, 2, 3, 4}, "ab_set", 0,
          {

              {{'a', 0}, 1},
              {{'b', 0}, 2},

              {{'a', 1}, 1},
              {{'b', 1}, 3},
              {{'a', 2}, 1},
              {{'b', 2}, 2},
              {{'a', 3}, 1},
              {{'b', 3}, 4},
              {{'a', 4}, 1},
              {{'b', 4}, 2},
          },
          {4});

  CHECK(dfa.equivalent_with(nfa.to_DFA()));
}
