/*!
 * \file dpda_test.cpp
 *
 * \brief
 */
#include <doctest/doctest.h>
#define private public

#include "context_free_lang/cnf.hpp"
#include "context_free_lang/dpda.hpp"
#include "context_free_lang/endmarked_dpda.hpp"
#include "context_free_lang/model_transform.hpp"
#include "lang/alphabet.hpp"

using namespace cyy::computation;
TEST_CASE("endmarked DPDA") {
  finite_automaton dpda_automata({0, 1, 2}, "01_set", 0, {1});
  DPDA dpda(dpda_automata, "01_set",
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
              }}});

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

  SUBCASE("test endmarked_DPDA") {
    endmarked_DPDA endmarked_dpda(dpda);

    std::u32string endmarked_str;

    SUBCASE("recognize") {
      for (auto str : {U"0", U"1"}) {
        endmarked_str = str;
        endmarked_str.push_back(ALPHABET::endmarker);
        CHECK(endmarked_dpda.recognize(endmarked_str));
      }
    }

    SUBCASE("can't recognize") {
      for (auto str : {U"", U"01", U"00", U"10", U"11"}) {
        endmarked_str = str;
        endmarked_str.push_back(ALPHABET::endmarker);
        CHECK(!endmarked_dpda.recognize(endmarked_str));
      }
    }

    SUBCASE("prepare_CFG_conversion") {
      endmarked_dpda.prepare_DCFG_conversion();
      SUBCASE("recognize") {
        for (auto str : {U"0", U"1"}) {
          endmarked_str = str;
          endmarked_str.push_back(ALPHABET::endmarker);
          CHECK(endmarked_dpda.recognize(endmarked_str));
        }
      }

      SUBCASE("can't recognize") {
        for (auto str : {U"", U"01", U"00", U"10", U"11"}) {
          endmarked_str = str;
          endmarked_str.push_back(ALPHABET::endmarker);
          CHECK(!endmarked_dpda.recognize(endmarked_str));
        }
      }
    }
    SUBCASE("to DCFG") {
      auto dcfg = DPDA_to_DCFG(endmarked_dpda);
      dcfg.to_CNF();
      CNF cnf(dcfg);
      SUBCASE("recognize") {
        for (auto str : {U"0", U"1"}) {
          endmarked_str = str;
          endmarked_str.push_back(ALPHABET::endmarker);
          CHECK(cnf.parse(endmarked_str));
        }
      }

      SUBCASE("can't recognize") {
        for (auto str : {U"", U"01", U"00", U"10", U"11"}) {
          endmarked_str = str;
          endmarked_str.push_back(ALPHABET::endmarker);
          CHECK(!cnf.parse(endmarked_str));
        }
      }
    }

    SUBCASE("to DPDA") {
      SUBCASE("normalize_functions") {
        endmarked_dpda.normalize_transitions();
        SUBCASE("recognize") {
          for (auto str : {U"0", U"1"}) {
            endmarked_str = str;
            endmarked_str.push_back(ALPHABET::endmarker);
            CHECK(endmarked_dpda.recognize(endmarked_str));
          }
        }

        SUBCASE("can't recognize") {
          for (auto str : {U"", U"01", U"00", U"10", U"11"}) {
            endmarked_str = str;
            endmarked_str.push_back(ALPHABET::endmarker);
            CHECK(!endmarked_dpda.recognize(endmarked_str));
          }
        }
      }
      auto reverted_dpda = endmarked_dpda.to_DPDA();
      SUBCASE("recognize") {
        for (auto str : {U"0", U"1"}) {
          CHECK(reverted_dpda.recognize(str));
        }
      }

      /*
      SUBCASE("can't recognize") {
        for (auto str : {U"", U"01", U"00", U"10", U"11"}) {
          CHECK(!reverted_dpda.recognize(str));
        }
      }
      */
    }
  }
}
