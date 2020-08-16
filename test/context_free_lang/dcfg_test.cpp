/*!
 * \file dcfg_test.cpp
 *
 * \brief
 */
#include <doctest/doctest.h>
#include <iostream>

#include "../../src/context_free_lang/dcfg.hpp"
#include "../../src/lang/alphabet.hpp"

using namespace cyy::computation;
TEST_CASE("DCFG") {
  auto endmarker = ALPHABET::endmarker;

  CFG::production_set_type productions;
  productions["S"] = {
      {"T", endmarker},
  };
  productions["T"] = {
      {"T", '(', "T", ')'},
      {},
  };
  SUBCASE("DCFG") {
    DCFG dcfg(ALPHABET::get("parentheses", true), "S", productions);
  }
  SUBCASE("DPDA") {
    DCFG dcfg(ALPHABET::get("parentheses", true), "S", productions);
    auto dpda = dcfg.to_DPDA();
    symbol_string str = U"()";
    str.push_back(endmarker);
    CHECK(dpda.recognize(str));
  }
}
