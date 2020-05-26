/*!
 * \file cfg_test.cpp
 *
 * \brief 测试dfa
 */
#include <doctest/doctest.h>

#include "../../src/regular_lang/dfa.hpp"

using namespace cyy::computation;
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

  SUBCASE("abb") { CHECK(dfa.simulate(U"abb")); }

  SUBCASE("aabb") { CHECK(dfa.simulate(U"aabb")); }
  SUBCASE("babb") { CHECK(dfa.simulate(U"babb")); }

  SUBCASE("bab") { CHECK(!dfa.simulate(U"bab")); }
}

TEST_CASE("intesection") {
  DFA dfa(
      {
          0,
          1,
      },
      "ab_set", 0,
      {
          {{0, 'a'}, 1},
          {{0, 'b'}, 0},
          {{1, 'a'}, 1},
          {{1, 'b'}, 1},
      },
      {1});
  DFA dfa2(
      {
          0,
          1,
      },
      "ab_set", 0,
      {
          {{0, 'a'}, 1},
          {{0, 'b'}, 0},
          {{1, 'a'}, 0},
          {{1, 'b'}, 0},
      },
      {1});
  CHECK(dfa.simulate(U"a"));
  CHECK(dfa.simulate(U"baa"));
  CHECK(dfa2.simulate(U"a"));
  auto dfa3 = dfa.intersect(dfa2);
  CHECK(dfa3.simulate(U"a"));
  CHECK(!dfa3.simulate(U"baa"));
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

  CHECK(dfa.minimize() == minimized_dfa);
}

TEST_CASE("complement") {
  DFA dfa(
      {
          0,
          1,
      },
      "ab_set", 0,
      {
          {{0, 'a'}, 1},
          {{0, 'b'}, 0},
          {{1, 'a'}, 0},
          {{1, 'b'}, 0},
      },
      {1});
  dfa = dfa.complement();
  CHECK(!dfa.simulate(U"a"));
  CHECK(dfa.simulate(U"b"));
  CHECK(dfa.simulate(U"ab"));
}
