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
#define DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS
#include <doctest/doctest.h>
#include <iostream>

#include "../../src/contex_free_lang/lalr_grammar.hpp"
#include "../../src/lang/common_tokens.hpp"

using namespace cyy::computation;

TEST_CASE("canonical_collection") {
  std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
      productions;
  auto endmarker = ALPHABET::get("common_tokens")->get_endmarker();
  auto id = static_cast<CFG::terminal_type>(common_token::id);
  productions["S"] = {
      {"L", '=', "R"},
      {"R"},
  };

  productions["L"] = {
      {'*', "R"},
      {id},
  };
  productions["R"] = {
      {"L"},
  };

  LALR_grammar grammar("common_tokens", "S", productions);

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
                        LR_0_item{CFG_production{"S", {{"L", '=', "R"}}}, 1},
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

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"L", {'*', "R"}}, 1},
                        {'=', endmarker});

    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"L", {id}}, 1},
                        {'=', endmarker});
    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG_production{"S", {{"L", '=', "R"}}}, 2},
                        {endmarker});

    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"L", {'*', "R"}}, 2},
                        {'=', endmarker});

    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG_production{"R", {"L"}}, 1},
                        {'=', endmarker});
    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG_production{"S", {{"L", '=', "R"}}}, 3},
                        {endmarker});
    sets.emplace(std::move(set));
  }

  std::unordered_set<LR_1_item_set> collection;
  for (auto &[set, _] : grammar.canonical_collection().first) {
    collection.emplace(set);
  }

  CHECK(sets == collection);
}

TEST_CASE("LALR(1) parse") {
  SUBCASE("parse expression grammar") {

    std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
        productions;
    auto id = static_cast<CFG::terminal_type>(common_token::id);
    productions["E"] = {
        {"T", "E'"},
    };
    productions["E'"] = {
        {'+', "T", "E'"},
        {},
    };
    productions["T"] = {
        {"F", "T'"},
    };
    productions["T'"] = {
        {'*', "F", "T'"},
        {},
    };
    productions["F"] = {
        {'(', "E", ')'}, {id} // i for id
    };

    LALR_grammar grammar("common_tokens", "E", productions);

    auto parse_tree =
        grammar.get_parse_tree(symbol_string{id, '+', id, '*', id});
    REQUIRE(parse_tree);
    CHECK(parse_tree->children.size() == 2);
  }

  SUBCASE("parse grammar with epsilon production") {

    std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
        productions;
    productions["E"] = {
        {'a', "E"},
        {},
    };

    LALR_grammar grammar("common_tokens", "E", productions);

    auto parse_tree = grammar.get_parse_tree(symbol_string{});
    REQUIRE(parse_tree);
    CHECK(parse_tree->children.size() == 0);

    parse_tree = grammar.get_parse_tree(symbol_string{'a'});
    REQUIRE(parse_tree);
    CHECK(parse_tree->children.size() == 2);
    parse_tree = grammar.get_parse_tree(symbol_string{'a', 'a'});
    REQUIRE(parse_tree);
    CHECK(parse_tree->children.size() == 2);
  }
}
