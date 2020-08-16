/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */
#include <doctest/doctest.h>

#include "../../src/context_free_lang/lalr_grammar.hpp"
#include "../../src/lang/common_tokens.hpp"

using namespace cyy::computation;

TEST_CASE("canonical_collection") {
  CFG::production_set_type productions;
  auto endmarker = ALPHABET::endmarker;
  auto id = static_cast<CFG::terminal_type>(common_token::id);
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
  grammar.normalize_start_symbol();

  std::unordered_set<LR_1_item_set> sets;
  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"S'", {"S"}}, 0},
                        {endmarker});
    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"S'", {"S"}}, 1},
                        {endmarker});
    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG_production{"S", {{"L", U'=', "R"}}}, 1},
                        {endmarker});
    set.add_kernel_item(grammar, LR_0_item{CFG_production{"R", {{"L"}}}, 1},
                        {endmarker});
    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"S", {"R"}}, 1},
                        {endmarker});

    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"L", {U'*', "R"}}, 1},
                        {U'=', endmarker});

    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"L", {id}}, 1},
                        {U'=', endmarker});
    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG_production{"S", {{"L", U'=', "R"}}}, 2},
                        {endmarker});

    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"L", {U'*', "R"}}, 2},
                        {U'=', endmarker});

    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"R", {"L"}}, 1},
                        {U'=', endmarker});
    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG_production{"S", {{"L", U'=', "R"}}}, 3},
                        {endmarker});
    sets.emplace(std::move(set));
  }

  std::unordered_set<LR_1_item_set> collection;
  for (auto &[_, set] : grammar.get_collection().first) {
    collection.emplace(set);
  }

  CHECK(sets == collection);
}
TEST_CASE("LALR(1) parse") {
  SUBCASE("parse expression grammar") {
    CFG::production_set_type productions;
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

    LALR_grammar grammar("common_tokens", "E", productions);

    auto parse_tree =
        grammar.get_parse_tree(symbol_string{id, U'+', id, U'*', id});
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

    LALR_grammar grammar("common_tokens", "E", productions);

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
