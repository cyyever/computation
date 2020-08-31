/*!
 * \file dpda_test.cpp
 *
 * \brief
 */
#include <doctest/doctest.h>

#include "../../src/context_free_lang/dpda.hpp"
#include "../../src/context_free_lang/endmarked_dpda.hpp"
#include "../../src/lang/alphabet.hpp"

using namespace cyy::computation;
TEST_CASE("recognize DPDA") {
  auto endmarker = U'1';
  finite_automata dpda_automata(

      {0, 1, 2, 3, 4}, "01_set", 0, {0, 3});
  DPDA dpda(dpda_automata, "01_set",
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
              }}});

  SUBCASE("") { CHECK(dpda.recognize(U"")); }
  SUBCASE("0") { CHECK(!dpda.recognize(U"0")); }
  SUBCASE("1") { CHECK(!dpda.recognize(U"1")); }

  SUBCASE("01") { CHECK(dpda.recognize(U"01")); }
  SUBCASE("10") { CHECK(!dpda.recognize(U"10")); }
  SUBCASE("0011") { CHECK(dpda.recognize(U"0011")); }
  SUBCASE("normalize") {
    auto new_dpda = dpda;
    new_dpda.normalize();
    CHECK(new_dpda.recognize(U""));
    CHECK(new_dpda.recognize(U"01"));
    CHECK(new_dpda.recognize(U"0011"));
  }

  SUBCASE("complement") {
    auto dpda_complement = dpda.complement();
    CHECK(!dpda_complement.recognize(U"0011"));
    CHECK(dpda_complement.recognize(U"10"));
  }
  SUBCASE("draw") { std::cout << dpda.MMA_draw() << std::endl; }
}
