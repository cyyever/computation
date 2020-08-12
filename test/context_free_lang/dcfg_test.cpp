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
  SUBCASE("DK") {
    CFG cfg(ALPHABET::get("parentheses", true), "S", productions);
   auto [dk,_,__,state_to_item_set]= cfg.get_DK();
   for (auto const&[_,set]:state_to_item_set) {
     std::cout<< set.MMA_draw(cfg.get_alphabet())<<std::endl;
   }
  }
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
