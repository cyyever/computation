/*!
 * \file dcfg_test.cpp
 *
 * \brief
 */
#include <doctest/doctest.h>
#include <iostream>

#include "../../src/context_free_lang/dcfg.hpp"
#include "../../src/context_free_lang/dk.hpp"
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
  SUBCASE("DK") {
    CFG cfg(ALPHABET::get("parentheses", true), "S", productions);
    auto dk = DK_DFA(cfg);
    std::cout << dk.MMA_draw(cfg) << std::endl;
  }
}
