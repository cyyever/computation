/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <iostream>

#include "../src/common_tokens.hpp"
#include "../src/grammar.hpp"

using namespace cyy::lang;

TEST_CASE("LL1 parse") {
  std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
      productions;
  auto epsilon = ALPHABET::get("common_tokens")->get_epsilon();
  auto id = common_tokens::token::id;
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

  CFG cfg("common_tokens", "E", productions);

  auto parse_tree = cfg.LL1_parse(symbol_string{id, '+', id, '*', id});
  CHECK(parse_tree);
  CHECK(parse_tree->children.size() == 2);
}
