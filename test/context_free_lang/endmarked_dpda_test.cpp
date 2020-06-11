/*!
 * \file dpda_test.cpp
 *
 * \brief
 */
#include <doctest/doctest.h>

#include "../../src/context_free_lang/dpda.hpp"
#include "../../src/context_free_lang/endmarkered_dpda.hpp"
#include "../../src/lang/alphabet.hpp"

using namespace cyy::computation;
TEST_CASE("endmarkered DPDA") {
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

  SUBCASE("endmarkered_dpda") {
    endmarkered_DPDA endmarkered_dpda(dpda);

    auto endmarker = ALPHABET::endmarker;
    std::u32string endmarkered_str;

    SUBCASE("recognize") {
      for (auto str : {U"0", U"1"}) {
        endmarkered_str = str;
        endmarkered_str.push_back(endmarker);
        CHECK(endmarkered_dpda.recognize(endmarkered_str));
      }
    }

    SUBCASE("can't recognize") {
      for (auto str : {U"", U"01", U"00", U"10", U"11"}) {
        endmarkered_str = str;
        endmarkered_str.push_back(endmarker);
        CHECK(!endmarkered_dpda.recognize(endmarkered_str));
      }
    }

    SUBCASE("to DPDA") {
      auto reverted_dpda = endmarkered_dpda.to_DPDA();
    }
  }
}
