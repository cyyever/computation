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

#include "../../src/contex_free_lang/canonical_lr_grammar.hpp"
#include "../../src/lang/common_tokens.hpp"

using namespace cyy::computation;

TEST_CASE("canonical_collection") {
  std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
      productions;
  auto endmarker = ALPHABET::get("common_tokens")->get_endmarker();
  productions["S"] = {
      {"C", "C"},
  };
  productions["C"] = {
      {'c', "C"},
      {'d'},
  };

  canonical_LR_grammar grammar("common_tokens", "S", productions);

  std::unordered_set<LR_1_item_set> sets;
  {
    LR_1_item_set set;

    set.add_kernel_item(
        grammar, LR_0_item{CFG::production_type{"S'", {"S"}}, 0}, {endmarker});
    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(
        grammar, LR_0_item{CFG::production_type{"S'", {"S"}}, 1}, {endmarker});
    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG::production_type{"S", {"C", "C"}}, 1},
                        {endmarker});
    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG::production_type{"C", {'c', "C"}}, 1},
                        {'c', 'd'});

    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG::production_type{"C", {'d'}}, 1},
                        {'c', 'd'});

    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG::production_type{"S", {"C", "C"}}, 2},
                        {endmarker});
    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG::production_type{"C", {'c', "C"}}, 1},
                        {endmarker});

    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG::production_type{"C", {'d'}}, 1},
                        {endmarker});

    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG::production_type{"C", {'c', "C"}}, 2},
                        {'c', 'd'});
    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG::production_type{"C", {'c', "C"}}, 2},
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

    std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
        productions;
    auto epsilon = ALPHABET::get("common_tokens")->get_epsilon();
    auto id = static_cast<CFG::terminal_type>(common_token::id);
    productions["E"] = {
        {"T", "E'"},
    };
    productions["E'"] = {
        {'+', "T", "E'"},
        {epsilon},
    };
    productions["T"] = {
        {"F", "T'"},
    };
    productions["T'"] = {
        {'*', "F", "T'"},
        {epsilon},
    };
    productions["F"] = {
        {'(', "E", ')'}, {id} // i for id
    };

    canonical_LR_grammar grammar("common_tokens", "E", productions);

    auto parse_tree =
        grammar.get_parse_tree(symbol_string{id, '+', id, '*', id});
    REQUIRE(parse_tree);
    CHECK(parse_tree->children.size() == 2);
  }

  SUBCASE("parse grammar with epsilon production") {

    std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
        productions;
    auto epsilon = ALPHABET::get("common_tokens")->get_epsilon();
    productions["E"] = {
        {'a', "E"},
        {epsilon},
    };

    canonical_LR_grammar grammar("common_tokens", "E", productions);

    auto parse_tree = grammar.get_parse_tree(symbol_string{});
    REQUIRE(parse_tree);
    CHECK(parse_tree->children.size() == 1);

    parse_tree = grammar.get_parse_tree(symbol_string{'a'});
    REQUIRE(parse_tree);
    CHECK(parse_tree->children.size() == 2);
    parse_tree = grammar.get_parse_tree(symbol_string{'a', 'a'});
    REQUIRE(parse_tree);
    CHECK(parse_tree->children.size() == 2);
  }
}
