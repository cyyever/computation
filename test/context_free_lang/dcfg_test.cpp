/*!
 * \file dcfg_test.cpp
 *
 * \brief
 */
#include <iostream>

#include <doctest/doctest.h>

#include "alphabet/alphabet.hpp"
#include "context_free_lang/dcfg.hpp"
#include "context_free_lang/dk.hpp"

using namespace cyy::computation;
TEST_CASE("DCFG") {
  SUBCASE("parse") {
    CFG::production_set_type productions;
    productions["S"] = {
        {"S", '(', "S", ')'},
        {},
    };
    DCFG dcfg(ALPHABET::get("parentheses"), "S", productions);
    auto parse_tree = dcfg.get_parse_tree(U"()");
    CHECK(parse_tree);
    std::cout << parse_tree->MMA_draw(dcfg.get_alphabet()) << std::endl;
  }
  SUBCASE("DPDA") {
    auto endmarker = ALPHABET::endmarker;

    CFG::production_set_type productions;
    productions["S"] = {
        {"T", endmarker},
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
    str.clear();
    str.push_back(endmarker);
    CHECK(dpda.recognize(str));
  }
}
