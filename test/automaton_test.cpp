/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <iostream>

#include "../src/automaton.hpp"
#include "../src/set_alphabet.hpp"

using namespace cyy::lang;
TEST_CASE("simulate DFA") {

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

  SUBCASE("abb") {
    symbol_string str = {'a', 'b', 'b'};
    CHECK(dfa.simulate(str));
  }

  SUBCASE("aabb") {
    symbol_string str = {'a', 'a', 'b', 'b'};
    CHECK(dfa.simulate(str));
  }
  SUBCASE("babb") {
    symbol_string str = {'b', 'a', 'b', 'b'};
    CHECK(dfa.simulate(str));
  }

  SUBCASE("bab") {
    symbol_string str = {'b', 'a', 'b'};
    CHECK(!dfa.simulate(str));
  }
}

TEST_CASE("simulate NFA") {
  auto epsilon = ALPHABET::get("ab_set")->get_epsilon();

  NFA nfa({0, 1, 2, 3, 4}, "ab_set", 0,
          {

              {{0, epsilon}, {1, 3}},

              {{1, 'a'}, {2}},
              {{2, 'a'}, {2}},

              {{3, 'b'}, {4}},
              {{4, 'b'}, {4}},
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

              {{0, epsilon}, {1, 7}},
              {{1, epsilon}, {2, 4}},
              {{2, 'a'}, {3}},
              {{3, epsilon}, {6}},
              {{4, 'b'}, {5}},
              {{5, epsilon}, {6}},
              {{6, epsilon}, {1, 7}},
              {{7, 'a'}, {8}},
              {{8, 'b'}, {9}},
              {{9, 'b'}, {10}},
          },
          {10});

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
TEST_CASE("minimize DFA") {
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

  DFA minimized_dfa({0, 1, 2, 3}, "ab_set", 0,
          {

              {{0, 'a'}, 1},
              {{0, 'b'}, 0},

              {{1, 'a'}, 1},
              {{1, 'b'}, 3},
              {{2, 'a'}, 1},
              {{2, 'b'}, 3},
              {{3, 'a'}, 1},
              {{3, 'b'}, 0},
          },
          {3});

 // CHECK(dfa.minimize()==minimized_dfa);
}