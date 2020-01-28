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

#include "../../src/regular_lang/gnfa.hpp"

using namespace cyy::computation;
TEST_CASE("DFA to regex") {

  DFA dfa({0, 1}, "ab_set", 0,
          {

              {{'a', 0}, 0},
              {{'b', 0}, 1},

              {{'a', 1}, 1},
              {{'b', 1}, 1},
          },
          {1});

  auto regex = GNFA(dfa).to_regex();
  CHECK_EQ(regex->to_string(), symbol_string(U"a*b(a|b)*"));
}
