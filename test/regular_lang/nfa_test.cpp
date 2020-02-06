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

#include "../../src/context_free_lang/ll_grammar.hpp"
#include "../../src/context_free_lang/model_transform.hpp"
#include "../../src/regular_lang/nfa.hpp"

using namespace cyy::computation;
TEST_CASE("simulate NFA") {

  NFA nfa({0, 1, 2, 3, 4}, "ab_set", 0,
          {
              {{'a', 1}, {2}},
              {{'a', 2}, {2}},

              {{'b', 3}, {4}},
              {{'b', 4}, {4}},
          },
          {2, 4}, {{0, {1, 3}}});

  SUBCASE("a") { CHECK(nfa.simulate(U"a")); }

  SUBCASE("aa") { CHECK(nfa.simulate(U"aa")); }

  SUBCASE("b") { CHECK(nfa.simulate(U"b")); }

  SUBCASE("bb") { CHECK(nfa.simulate(U"bb")); }
  SUBCASE("ab") { CHECK(!nfa.simulate(U"ab")); }
}

TEST_CASE("NFA to DFA") {
  NFA nfa({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, "ab_set", 0,
          {

              {{'a', 2}, {3}},
              {{'b', 4}, {5}},
              {{'a', 7}, {8}},
              {{'b', 8}, {9}},
              {{'b', 9}, {10}},
          },
          {10},

          {

              {0, {1, 7}},
              {1, {2, 4}},

              {3, {6}},
              {5, {6}},
              {6, {1, 7}},
          });

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

TEST_CASE("NFA to CFG") {
  NFA nfa({0, 1, 2, 3, 4}, "ab_set", 0,
          {
              {{'a', 1}, {2}},
              {{'a', 2}, {2}},

              {{'b', 3}, {4}},
              {{'b', 4}, {4}},
          },
          {2, 4}, {{0, {1, 3}}});
  auto cfg = NFA_to_CFG(nfa);
  CHECK(cfg.recursive_descent_parse(U"a"));
}
