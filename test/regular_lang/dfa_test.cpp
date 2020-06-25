/*!
 * \file cfg_test.cpp
 *
 * \brief 测试dfa
 */
#include <doctest/doctest.h>

#include "../../src/regular_lang/dfa.hpp"

using namespace cyy::computation;
TEST_CASE("recognize DFA") {

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

  SUBCASE("abb") { CHECK(dfa.recognize(U"abb")); }

  SUBCASE("aabb") { CHECK(dfa.recognize(U"aabb")); }
  SUBCASE("babb") { CHECK(dfa.recognize(U"babb")); }

  SUBCASE("bab") { CHECK(!dfa.recognize(U"bab")); }
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
  CHECK(dfa.recognize(U"a"));
  CHECK(dfa.recognize(U"baa"));
  CHECK(dfa2.recognize(U"a"));
  auto dfa3 = dfa.intersect(dfa2);
  CHECK(dfa3.recognize(U"a"));
  CHECK(!dfa3.recognize(U"baa"));
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
  CHECK(!dfa.recognize(U"a"));
  CHECK(dfa.recognize(U"b"));
  CHECK(dfa.recognize(U"ab"));
}

TEST_CASE("draw") {
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
  std::cout<< dfa.MMA_draw()<<std::endl;
}
