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
TEST_CASE("parse regex and to NFA") {

  SUBCASE("basic case") {
    symbol_string expr = U"a";
    regex reg("ab_set", expr);

    NFA nfa({0, 1}, "ab_set", 0,
            {

                {{0, 'a'}, {1}},
            },
            {1});

    CHECK(nfa == reg.to_NFA());
  }

  SUBCASE("union") {

    symbol_string expr = U"a|b";
    regex reg("ab_set", expr);

    auto epsilon = ALPHABET::get("ab_set")->get_epsilon();
    NFA nfa({0, 1, 2, 3, 4, 5}, "ab_set", 0,
            {

                {{0, epsilon}, {1, 3}},
                {{1, 'a'}, {2}},
                {{2, epsilon}, {5}},
                {{3, 'b'}, {4}},
                {{4, epsilon}, {5}},
            },
            {5});

    CHECK(nfa == reg.to_NFA());
  }

  SUBCASE("kleene_closure_node") {

    symbol_string expr = U"(a|b)*";
    regex reg("ab_set", expr);

    auto epsilon = ALPHABET::get("ab_set")->get_epsilon();
    NFA nfa({0, 1, 2, 3, 4, 5, 6, 7}, "ab_set", 0,
            {

                {{0, epsilon}, {1, 7}},
                {{1, epsilon}, {2, 4}},
                {{2, 'a'}, {3}},
                {{3, epsilon}, {6}},
                {{4, 'b'}, {5}},
                {{5, epsilon}, {6}},
                {{6, epsilon}, {1, 7}},
            },
            {7});

    CHECK(nfa == reg.to_NFA());
  }

  SUBCASE("all together") {

    symbol_string expr = U"(a|b)*abb";
    regex reg("ab_set", expr);

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

  CHECK(dfa.simulate(U"abb"));
  CHECK(dfa.simulate(U"aabb"));
  CHECK(dfa.simulate(U"babb"));
  CHECK(!dfa.simulate(U"bb"));
  CHECK(!dfa.simulate(U"ab"));
}

TEST_CASE("parse extended regex and to NFA") {

  SUBCASE("*") {
    symbol_string expr = U"a*";
    regex reg("ab_set", expr);

    auto nfa = reg.to_NFA();
    auto dfa = reg.to_DFA().minimize();

    for (auto const &test_string : {U"", U"a", U"aa"}) {
      CHECK(nfa.simulate(test_string));
      CHECK(dfa.simulate(test_string));
    }
  }

  SUBCASE("+") {
    symbol_string expr = U"a+";
    regex reg("ab_set", expr);

    auto nfa = reg.to_NFA();
    auto dfa = reg.to_DFA().minimize();

    CHECK(!nfa.simulate(U""));
    CHECK(!dfa.simulate(U""));
    for (auto const &test_string : {U"a", U"aa"}) {
      CHECK(nfa.simulate(test_string));
      CHECK(dfa.simulate(test_string));
    }
  }

  SUBCASE("?") {
    symbol_string expr = U"a?";
    regex reg("ab_set", expr);

    auto nfa = reg.to_NFA();
    auto dfa = reg.to_DFA().minimize();

    for (auto const &test_string : {U"", U"a"}) {
      CHECK(nfa.simulate(test_string));
      CHECK(dfa.simulate(test_string));
    }
    CHECK(!nfa.simulate(U"aa"));
    CHECK(!dfa.simulate(U"aa"));
  }
}
