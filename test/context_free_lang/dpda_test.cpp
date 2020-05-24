/*!
 * \file dpda_test.cpp
 *
 * \brief
 */
#include <doctest/doctest.h>

#include "../../src/context_free_lang/dpda.hpp"
#include "../../src/lang/alphabet.hpp"
#include "../../src/lang/set_alphabet.hpp"

using namespace cyy::computation;
TEST_CASE("simulate DPDA") {

  auto endmarker = U'1';
  DPDA dpda({0, 1, 2, 3, 4}, "01_set", "01_set", 0,
            {{0,
              {
                  {{}, {1, endmarker}},
              }},
             {1,
              {
                  {{U'0'}, {1, U'0'}},
                  {{U'1', U'0'}, {2}},
                  {{U'1', endmarker}, {4}},
              }},
             {2,
              {
                  {{U'1', U'0'}, {2}},
                  {{{}, endmarker}, {3}},
                  {{U'0', U'0'}, {4}},
              }},
             {3,
              {
                  {{}, {4}},
              }},
             {4,
              {
                  {{U'0'}, {4}},
                  {{U'1'}, {4}},
              }}},
            {0, 3});

  SUBCASE("") {
    symbol_string str = U"";
    CHECK(dpda.simulate(str));
  }
  SUBCASE("0") {
    symbol_string str = U"0";
    CHECK(!dpda.simulate(str));
  }
  SUBCASE("1") {
    symbol_string str = U"1";
    CHECK(!dpda.simulate(str));
  }

  SUBCASE("01") {
    symbol_string str = U"01";
    CHECK(dpda.simulate(str));
  }
  SUBCASE("10") {
    symbol_string str = U"10";
    CHECK(!dpda.simulate(str));
  }
  SUBCASE("0011") {
    symbol_string str = U"0011";
    CHECK(dpda.simulate(str));
  }
  SUBCASE("normalize") {
    symbol_string str = U"0011";
    dpda.normalize();
    CHECK(dpda.simulate(str));
  }
}
