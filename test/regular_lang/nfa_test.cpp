/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */
#include <doctest/doctest.h>

#include "context_free_lang/ll_grammar.hpp"
#include "context_free_lang/model_transform.hpp"
#include "regular_lang/nfa.hpp"

using namespace cyy::computation;
TEST_CASE("recognize NFA") {

  NFA nfa({0, 1, 2, 3, 4}, "ab_set", 0,
          {
              {{1, 'a'}, {2}},
              {{2, 'a'}, {2}},
              {{3, 'b'}, {4}},
              {{4, 'b'}, {4}},
          },
          {2, 4}, {{0, {1, 3}}});

  SUBCASE("a") { CHECK(nfa.recognize(U"a")); }

  SUBCASE("aa") { CHECK(nfa.recognize(U"aa")); }

  SUBCASE("b") { CHECK(nfa.recognize(U"b")); }

  SUBCASE("bb") { CHECK(nfa.recognize(U"bb")); }
  SUBCASE("ab") { CHECK(!nfa.recognize(U"ab")); }
}
TEST_CASE("NFA to DFA") {
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

  DFA dfa({0, 1, 2, 3, 4}, "ab_set", 0,
          {

              {{0, 'a'}, 1},
              {{0, 'b'}, 2},
              {{1, 'a'}, 1},
              {{1, 'b'}, 3},
              {{2, 'a'}, 1},
              {{2, 'b'}, 2},
              {{3, 'a'}, 1},
              {{3, 'b'}, 4},
              {{4, 'a'}, 1},
              {{4, 'b'}, 2},
          },
          {4});

  CHECK(dfa.equivalent_with(nfa.to_DFA()));
}

TEST_CASE("NFA to CFG") {
  NFA nfa({0, 1, 2, 3, 4}, "ab_set", 0,
          {
              {{1, 'a'}, {2}},
              {{2, 'a'}, {2}},
              {{3, 'b'}, {4}},
              {{4, 'b'}, {4}},
          },
          {2, 4}, {{0, {1, 3}}});
  auto cfg = NFA_to_CFG(nfa);
  // CHECK(cfg.recursive_descent_parse(U"a"));
}
TEST_CASE("draw") {
  NFA nfa({0, 1, 2, 3, 4}, "ab_set", 0,
          {
              {{1, 'a'}, {2}},
              {{2, 'a'}, {2}},
              {{3, 'b'}, {4}},
              {{4, 'b'}, {4}},
          },
          {2, 4}, {{0, {1, 3}}});
  std::cout << nfa.MMA_draw() << std::endl;
}
