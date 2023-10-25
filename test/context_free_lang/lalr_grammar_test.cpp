/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */
#include <doctest/doctest.h>

#include "alphabet/common_tokens.hpp"
#include "context_free_lang/lalr_grammar.hpp"

using namespace cyy::computation;

TEST_CASE("LALR(1) parse") {
  SUBCASE("parse expression grammar") {
    CFG::production_set_type productions;
    auto id = static_cast<CFG::terminal_type>(cyy::algorithm::common_token::id);
    productions["S"] = {
        {"L", U'=', "R"},
        {"R"},
    };
    productions["L"] = {
        {U'*', "R"},
        {id},
    };
    productions["R"] = {
        {"L"},
    };

    LALR_grammar grammar("common_tokens", "S", productions);

    auto parse_tree = grammar.get_parse_tree(symbol_string{id, U'=', id});
    REQUIRE(parse_tree);
    CHECK_EQ(parse_tree->grammar_symbol.get_nonterminal(), "S");
  }
}
