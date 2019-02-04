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

#include "../../src/contex_free_lang/ll_grammar.hpp"
#include "../../src/lang/common_tokens.hpp"

using namespace cyy::computation;

TEST_CASE("LL(1) parse tree") {
  std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
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

  LL_grammar grammar("common_tokens", "E", productions);

  auto parse_tree = grammar.get_parse_tree(symbol_string{id, '+', id, '*', id});
  REQUIRE(parse_tree);
  CHECK(parse_tree->children.size() == 2);
}

TEST_CASE("LL(1) parse") {
  std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
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

  LL_grammar grammar("common_tokens", "E", productions);

  auto parse_res = grammar.parse2(symbol_string{id, '+', id, '*', id},
                                  [](const auto &production, auto pos) {});
  REQUIRE(parse_res);
}
