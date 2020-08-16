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
  SUBCASE("parse") {
    CFG::production_set_type productions;
    /* productions["S"] = { */
    /*   {"T"}, */
    /* }; */
    productions["S"] = {
      {'(', "S", ')'},
      {},
    };
    DCFG dcfg(ALPHABET::get("parentheses", true), "S", productions);
    auto parse_tree=dcfg.get_parse_tree(U"()");
    CHECK(parse_tree);
  }
  SUBCASE("DPDA") {
    auto endmarker = ALPHABET::endmarker;

    CFG::production_set_type productions;
    productions["S"] = {
      {"T",endmarker},
    };
    productions["T"] = {
      {"T", '(', "T", ')'},
      {},
    };
    DCFG dcfg(ALPHABET::get("parentheses", true), "S", productions);
    auto dpda = dcfg.to_DPDA();
    symbol_string str = U"()";
    str.push_back(endmarker);
    CHECK(dpda.recognize(str));
  }
}
