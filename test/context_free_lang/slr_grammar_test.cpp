/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */

#include <doctest/doctest.h>

#include "alphabet/common_tokens.hpp"
#include "context_free_lang/common_grammar.hpp"

using namespace cyy::computation;

TEST_CASE("SLR(1) parse") {
  SUBCASE("parse expression grammar") {
    auto id = static_cast<CFG::terminal_type>(cyy::algorithm::common_token::id);
    auto grammar = get_expression_grammar();
    auto parse_tree =
        grammar->get_parse_tree(symbol_string{id, U'+', id, U'*', id});
    REQUIRE(parse_tree);
    CHECK_EQ(parse_tree->grammar_symbol.get_nonterminal(), "E");
    CHECK(parse_tree->children.size() == 3);
    CHECK(parse_tree->children[0]->children.size() == 1);
  }

  SUBCASE("parse grammar with epsilon production") {
    CFG::production_set_type productions;
    productions["E"] = {
        {U'a', "E"},
        {},
    };

    SLR_grammar grammar("printable-ASCII", "E", productions);

    auto parse_tree = grammar.get_parse_tree(symbol_string{});
    REQUIRE(parse_tree);
    CHECK(parse_tree->children.size() == 0);

    parse_tree = grammar.get_parse_tree(symbol_string{U'a'});
    REQUIRE(parse_tree);
    CHECK(parse_tree->children.size() == 2);
    parse_tree = grammar.get_parse_tree(symbol_string{U'a', U'a'});
    REQUIRE(parse_tree);
    CHECK(parse_tree->children.size() == 2);
  }
}
