/*!
 * \file dk_test.cpp
 */

#include <doctest/doctest.h>

#include "alphabet/alphabet.hpp"
#include "context_free_lang/dk.hpp"
import std;

using namespace cyy::computation;
TEST_CASE("DK") {
  auto endmarker = ALPHABET::endmarker;

  CFG::production_set_type productions;
  productions["S"] = {
      {"T", endmarker},
  };
  productions["T"] = {
      {"T", '(', "T", ')'},
      {},
  };
  CFG cfg(ALPHABET::get("parentheses", true), "S", productions);
  auto dk = DK_DFA(cfg);
  std::cout << dk.MMA_draw(cfg) << std::endl;
}
