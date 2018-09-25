/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <iostream>

#include "../../src/contex_free_lang/ll_grammar.hpp"
#include "../../src/lang/common_tokens.hpp"

using namespace cyy::computation;

TEST_CASE("LL(1) parse") {
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

  LL_grammar grammar("common_tokens", "E", productions);

  auto parse_tree = grammar.parse(symbol_string{id, '+', id, '*', id});
  REQUIRE(parse_tree);
  CHECK(parse_tree->children.size() == 2);
}
