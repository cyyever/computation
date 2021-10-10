/*!
 * \file dcfg_test.cpp
 *
 * \brief
 */
#include <iostream>

#include <doctest/doctest.h>

#include "context_free_lang/dk_1.hpp"
#include "alphabet/alphabet.hpp"

using namespace cyy::computation;
TEST_CASE("DK_1") {
  auto endmarker = ALPHABET::endmarker;

  CFG::production_set_type productions;
  productions["S"] = {
      {"E", endmarker},
  };
  productions["E"] = {
      {"E", '+', "T"},
      {"T"},
  };
  productions["T"] = {
      {"T", '*', 'a'},
      {'a'},
  };
  CFG cfg(ALPHABET::get("ASCII", true), "S", productions);
  auto dk = DK_1_DFA(cfg);
  std::cout << dk.MMA_draw(cfg) << std::endl;
}
