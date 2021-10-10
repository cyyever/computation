/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */

#include <doctest/doctest.h>

#include "alphabet/common_tokens.hpp"
#include "context_free_lang/canonical_lr_grammar.hpp"

using namespace cyy::computation;

TEST_CASE("canonical_LR(1) parse") {
  SUBCASE("parse expression grammar") {
    CFG::production_set_type productions;
    auto id = static_cast<CFG::terminal_type>(common_token::id);
    productions["E"] = {
        {"T", "E'"},
    };
    productions["E'"] = {
        {U'+', "T", "E'"},
        {},
    };
    productions["T"] = {
        {"F", "T'"},
    };
    productions["T'"] = {
        {U'*', "F", "T'"},
        {},
    };
    productions["F"] = {
        {U'(', "E", U')'}, {id} // i for id
    };

    canonical_LR_grammar grammar("common_tokens", "E", productions);

    auto parse_tree =
        grammar.get_parse_tree(symbol_string{id, U'+', id, U'*', id});
    REQUIRE(parse_tree);
    CHECK(parse_tree->children.size() == 2);
  }

  SUBCASE("parse grammar with epsilon production") {
    CFG::production_set_type productions;
    productions["E"] = {
        {U'a', "E"},
        {},
    };

    canonical_LR_grammar grammar("ab_set", "E", productions);

    auto parse_tree = grammar.get_parse_tree(U"");
    REQUIRE(parse_tree);
    CHECK(parse_tree->children.size() == 0);

    parse_tree = grammar.get_parse_tree(U"a");
    REQUIRE(parse_tree);
    CHECK(parse_tree->children.size() == 2);
    parse_tree = grammar.get_parse_tree(U"aa");
    REQUIRE(parse_tree);
    CHECK(parse_tree->children.size() == 2);
    auto dpda = grammar.to_DPDA();
    std::u32string endmarked_str;
    for (auto str : {U"", U"a", U"aa"}) {
      endmarked_str = str;
      endmarked_str.push_back(ALPHABET::endmarker);

      REQUIRE(dpda.recognize(endmarked_str));
    }
  }
}
