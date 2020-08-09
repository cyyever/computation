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
TEST_CASE("endmarked DPDA") {
  auto endmarker = ALPHABET::endmarker;
  DPDA dpda({0, 1, 2}, "01_set", "01_set", 0,
            {{0,
              {
                  {{U'0'}, {1, U'0'}},
                  {{U'1'}, {1, U'1'}},
              }},
             {1,
              {
                  {{U'0'}, {2}},
                  {{U'1'}, {2}},
              }},
             {2,
              {
                  {{U'0'}, {2}},
                  {{U'1'}, {2}},
              }}},
            {1});

  SUBCASE("test DPDA") {
    SUBCASE("recognize") {
      for (auto str : {U"0", U"1"}) {
        CHECK(dpda.recognize(str));
      }
    }

    SUBCASE("can't recognize") {
      for (auto str : {U"", U"01", U"00", U"10", U"11"}) {
        CHECK(!dpda.recognize(str));
      }
    }
  }

  SUBCASE("endmarked_dpda") {
    endmarked_DPDA endmarked_dpda(dpda);

    auto endmarker = ALPHABET::endmarker;
    std::u32string endmarked_str;

    SUBCASE("recognize") {
      for (auto str : {U"0", U"1"}) {
        endmarked_str = str;
        endmarked_str.push_back(endmarker);
        CHECK(endmarked_dpda.recognize(endmarked_str));
      }
    }

    SUBCASE("can't recognize") {
      for (auto str : {U"", U"01", U"00", U"10", U"11"}) {
        endmarked_str = str;
        endmarked_str.push_back(endmarker);
        CHECK(!endmarked_dpda.recognize(endmarked_str));
      }
    }

    SUBCASE("to DPDA") { auto reverted_dpda = endmarked_dpda.to_DPDA(); }
  }
}
