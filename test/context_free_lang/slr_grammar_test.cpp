/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */
#if __has_include(<CppCoreCheck\Warnings.h>)
#include <CppCoreCheck\Warnings.h>
#pragma warning(disable : ALL_CPPCORECHECK_WARNINGS)
#endif
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <iostream>

#include "../../src/context_free_lang/slr_grammar.hpp"
#include "../../src/lang/common_tokens.hpp"

using namespace cyy::computation;

TEST_CASE("canonical_collection") {
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

  std::unordered_set<LR_0_item_set> sets;

  {
    LR_0_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"E'", {"E"}}, 0});

    sets.emplace(std::move(set));
  }

  {
    LR_0_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"E'", {"E"}}, 1});

    set.add_kernel_item(grammar,
                        LR_0_item{CFG_production{"E", {"E", U'+', "T"}}, 1});
    sets.emplace(std::move(set));
  }

  {
    LR_0_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"E", {"T"}}, 1});
    set.add_kernel_item(grammar,
                        LR_0_item{CFG_production{"T", {"T", U'*', "F"}}, 1});
    sets.emplace(std::move(set));
  }

  {
    LR_0_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"T", {"F"}}, 1});
    sets.emplace(std::move(set));
  }

  {
    LR_0_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG_production{"F", {U'(', "E", U')'}}, 1});
    sets.emplace(std::move(set));
  }

  {
    LR_0_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"F", {id}}, 1});
    sets.emplace(std::move(set));
  }

  {
    LR_0_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG_production{"E", {"E", U'+', "T"}}, 2});
    sets.emplace(std::move(set));
  }

  {
    LR_0_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG_production{"T", {"T", U'*', "F"}}, 2});
    sets.emplace(std::move(set));
  }

  {
    LR_0_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG_production{"E", {"E", U'+', "T"}}, 1});
    set.add_kernel_item(grammar,
                        LR_0_item{CFG_production{"F", {U'(', "E", U')'}}, 2});
    sets.emplace(std::move(set));
  }

  {
    LR_0_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG_production{"E", {"E", U'+', "T"}}, 3});
    set.add_kernel_item(grammar,
                        LR_0_item{CFG_production{"T", {"T", U'*', "F"}}, 1});
    sets.emplace(std::move(set));
  }

  {
    LR_0_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG_production{"T", {"T", U'*', "F"}}, 3});
    sets.emplace(std::move(set));
  }

  {
    LR_0_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG_production{"F", {U'(', "E", U')'}}, 3});
    sets.emplace(std::move(set));
  }

  std::unordered_set<LR_0_item_set> collection;
  for (auto &[set, _] : grammar.canonical_collection().first) {
    collection.emplace(set);
  }

  CHECK(sets == collection);
}

TEST_CASE("SLR(1) parse") {
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

    SLR_grammar grammar("common_tokens", "E", productions);

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

    SLR_grammar grammar("common_tokens", "E", productions);

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
