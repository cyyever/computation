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

#include "../../src/regular_lang/dfa.hpp"

using namespace cyy::computation;
TEST_CASE("simulate DFA") {

  DFA dfa({0, 1, 2, 3}, "ab_set", 0,
          {

              {{'a', 0}, 1},
              {{'b', 0}, 0},

              {{'a', 1}, 1},
              {{'b', 1}, 2},
              {{'a', 2}, 1},
              {{'b', 2}, 3},
              {{'a', 3}, 1},
              {{'b', 3}, 0},
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

TEST_CASE("minimize DFA") {
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

  DFA minimized_dfa({0, 1, 2, 3}, "ab_set", 0,
                    {

                        {{'a', 0}, 1},
                        {{'b', 0}, 0},

                        {{'a', 1}, 1},
                        {{'b', 1}, 3},
                        {{'a', 2}, 1},
                        {{'b', 2}, 3},
                        {{'a', 3}, 1},
                        {{'b', 3}, 0},
                    },
                    {3});

  CHECK(dfa.minimize() == minimized_dfa);
}
