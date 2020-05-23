/*!
 * \file gnfa_test.cpp
 */
#include <doctest/doctest.h>

#include "../../src/regular_lang/gnfa.hpp"

using namespace cyy::computation;
TEST_CASE("DFA to regex") {

  DFA dfa({0, 1}, "ab_set", 0,
          {
              {{0, 'a'}, 0},
              {{0, 'b'}, 1},

              {{1, 'a'}, 1},
              {{1, 'b'}, 1},
          },
          {1});

  auto regex = GNFA(dfa).to_regex();
  auto regex_str = regex->to_string();
  auto res = (regex_str == symbol_string(U"a*b(a|b)*") ||
              regex_str == symbol_string(U"a*b(b|a)*"));
  CHECK(res);
}
