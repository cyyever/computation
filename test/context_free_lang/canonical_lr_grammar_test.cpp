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

#include "../../src/context_free_lang/canonical_lr_grammar.hpp"
#include "../../src/lang/common_tokens.hpp"

using namespace cyy::computation;

TEST_CASE("canonical_collection") {
  std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
      productions;
  auto endmarker = ALPHABET::endmarker;
  productions["S"] = {
      {"C", "C"},
  };
  productions["C"] = {
      {U'c', "C"},
      {U'd'},
  };

  canonical_LR_grammar grammar("common_tokens", "S", productions);

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

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"S", {"C", "C"}}, 1},
                        {endmarker});
    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"C", {U'c', "C"}}, 1},
                        {U'c', U'd'});

    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"C", {U'd'}}, 1},
                        {U'c', U'd'});

    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"S", {"C", "C"}}, 2},
                        {endmarker});
    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"C", {U'c', "C"}}, 1},
                        {endmarker});

    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"C", {U'd'}}, 1},
                        {endmarker});

    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"C", {U'c', "C"}}, 2},
                        {U'c', U'd'});
    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"C", {U'c', "C"}}, 2},
                        {endmarker});
    sets.emplace(std::move(set));
  }

  std::unordered_set<LR_1_item_set> collection;
  for (auto &[set, _] : grammar.canonical_collection().first) {
    collection.emplace(set);
  }

  CHECK(sets == collection);
}

TEST_CASE("canonical_LR(1) parse") {
  SUBCASE("parse expression grammar") {

    std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
        productions;
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

    std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
        productions;
    productions["E"] = {
        {U'a', "E"},
        {},
    };

    canonical_LR_grammar grammar("common_tokens", "E", productions);

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
