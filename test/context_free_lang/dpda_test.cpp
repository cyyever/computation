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

  SUBCASE("") { CHECK(dpda.simulate(U"")); }
  SUBCASE("0") { CHECK(!dpda.simulate(U"0")); }
  SUBCASE("1") { CHECK(!dpda.simulate(U"1")); }

  SUBCASE("01") { CHECK(dpda.simulate(U"01")); }
  SUBCASE("10") { CHECK(!dpda.simulate(U"10")); }
  SUBCASE("0011") { CHECK(dpda.simulate(U"0011")); }
  SUBCASE("normalize") {
    auto new_dpda = dpda;
    new_dpda.normalize();
    CHECK(new_dpda.simulate(U""));
    CHECK(new_dpda.simulate(U"01"));
    CHECK(new_dpda.simulate(U"0011"));
  }
  SUBCASE("complement") {
    auto dpda_complement = dpda.complement();
    CHECK(!dpda_complement.simulate(U"0011"));
    CHECK(dpda_complement.simulate(U"10"));
  }
}
