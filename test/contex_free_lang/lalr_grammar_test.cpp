/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS
#include <doctest.h>
#include <iostream>

#include "../../src/lang/common_tokens.hpp"
#include "../../src/contex_free_lang/lalr_grammar.hpp"

using namespace cyy::computation;

TEST_CASE("canonical_collection") {
  std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
      productions;
  auto endmarker = ALPHABET::get("common_tokens")->get_endmarker();
  auto id = static_cast<CFG::terminal_type>(common_tokens::token::id);
  productions["S"] = {
      {"L", '=',"R"},
      {"R"},
  };

  productions["L"] = {
      {'*',"R"},
      {id},
  };
  productions["R"] = {
      {"L"},
  };

  LALR_grammar grammar("common_tokens", "S", productions);

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
                        LR_0_item{CFG::production_type{"S", {  {"L", '=',"R"}}}, 1},
                        {endmarker});
    set.add_kernel_item(grammar,
                        LR_0_item{CFG::production_type{"R", {  {"L"}}}, 1},
                        {endmarker});
    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG::production_type{"S", {"R"}}, 1},
                        {endmarker});

    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG::production_type{"L", {'*',"R"}}, 1},
                        {'=', endmarker});

    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG::production_type{"L", {id}}, 1},
                        {'=',endmarker});
    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG::production_type{"S", {  {"L", '=',"R"}}}, 2},
                        {endmarker});

    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar, LR_0_item{CFG::production_type{"L", {'*',"R"}}, 2},
                        {'=',endmarker});

    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG::production_type{"R", {"L"}}, 1},
                        {'=',endmarker});
    sets.emplace(std::move(set));
  }

  {
    LR_1_item_set set;

    set.add_kernel_item(grammar,
                        LR_0_item{CFG::production_type{"S", {  {"L", '=',"R"}}}, 3},
                        {endmarker});
    sets.emplace(std::move(set));
  }

  std::unordered_set<LR_1_item_set> collection;
  for (auto &[set, _] : grammar.canonical_collection().first) {
    collection.emplace(std::move(set));
  }

  CHECK(sets == collection);
}

TEST_CASE("LALR(1) parse") {
  SUBCASE("parse expression grammar") {

    std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
        productions;
    auto epsilon = ALPHABET::get("common_tokens")->get_epsilon();
    auto id = static_cast<CFG::terminal_type>(common_tokens::token::id);
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

    LALR_grammar grammar("common_tokens", "E", productions);

    auto parse_tree = grammar.parse(symbol_string{id, '+', id, '*', id});
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

    LALR_grammar grammar("common_tokens", "E", productions);

    auto parse_tree = grammar.parse(symbol_string{});
    REQUIRE(parse_tree);
    CHECK(parse_tree->children.size() == 1);

    parse_tree = grammar.parse(symbol_string{'a'});
    REQUIRE(parse_tree);
    CHECK(parse_tree->children.size() == 2);
    parse_tree = grammar.parse(symbol_string{'a', 'a'});
    REQUIRE(parse_tree);
    CHECK(parse_tree->children.size() == 2);
  }
}
