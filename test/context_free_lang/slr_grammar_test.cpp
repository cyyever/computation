/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */
#if __has_include(<CppCoreCheck\Warnings.h>)
#include <CppCoreCheck\Warnings.h>
#pragma warning(disable : ALL_CPPCORECHECK_WARNINGS)
#endif
#include <doctest/doctest.h>
#include <iostream>

#include "../../src/context_free_lang/slr_grammar.hpp"
#include "../../src/lang/common_tokens.hpp"

using namespace cyy::computation;

TEST_CASE("SLR(1) parse") {
  SUBCASE("parse expression grammar") {

    std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
        productions;
    auto id = static_cast<CFG::terminal_type>(common_token::id);
    productions["E"] = {
        {"E", U'+', "T"},
        {"T"},
    };
    productions["T"] = {
        {"T", U'*', "F"},
        {"F"},
    };
    productions["F"] = {
        {U'(', "E", U')'}, {id} // i for id
    };

    SLR_grammar grammar("common_tokens", "E", productions);

    auto parse_tree =
        grammar.get_parse_tree(symbol_string{id, U'+', id, U'*', id});
    REQUIRE(parse_tree);
    CHECK_EQ(parse_tree->grammar_symbol.get_nonterminal(), "E");
    CHECK(parse_tree->children.size() == 3);
    CHECK(parse_tree->children[0]->children.size() == 1);
  }

  SUBCASE("parse grammar with epsilon production") {
    std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
        productions;
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
