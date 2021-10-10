/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */
#include <doctest/doctest.h>

#include "alphabet/common_tokens.hpp"
#include "context_free_lang/ll_grammar.hpp"

using namespace cyy::computation;

TEST_CASE("LL(1) parse tree") {
  CFG::production_set_type productions;
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

  LL_grammar grammar("common_tokens", "E", productions);

  auto parse_tree = grammar.get_parse_tree(symbol_string{id, '+', id, '*', id});
  REQUIRE(parse_tree);
  std::cout << parse_tree->MMA_draw(grammar.get_alphabet()) << std::endl;
  CHECK(parse_tree->children.size() == 2);
}

TEST_CASE("LL(1) parse") {
  CFG::production_set_type productions;
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

  LL_grammar grammar("common_tokens", "E", productions);

  auto parse_res = grammar.parse(symbol_string{id, '+', id, '*', id},
                                 [](const auto &, auto) {});
  REQUIRE(parse_res);
}
