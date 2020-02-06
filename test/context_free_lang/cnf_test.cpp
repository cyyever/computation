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

#include "../../src/context_free_lang/cfg.hpp"
#include "../../src/lang/common_tokens.hpp"

using namespace cyy::computation;

TEST_CASE("eliminate_epsilon_productions") {
  std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
      productions;
  productions["S"] = {
      {U'a', "S", U'b', "S"},
      {U'b', "S", U'a', "S"},
      {},
  };
  CFG cfg("ab_set", "S", productions);
  SUBCASE("nullable") {
    auto nullable_nonterminals = cfg.nullable();
    CHECK(nullable_nonterminals == std::set<CFG::nonterminal_type>{"S"});
  }
  SUBCASE("eliminate_epsilon_productions") {
    cfg.eliminate_epsilon_productions();
    std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
        new_productions;
    new_productions["S"] = {
        {U'a', "S", "S'"},
        {U'a', "S'"},
        {U'b', "S", "S''"},
        {U'b', "S''"},
    };
    new_productions["S'"] = {
        {U'b', "S"},
        {U'b'},
    };
    new_productions["S''"] = {
        {U'a', "S"},
        {U'a'},
    };
    CHECK(cfg == CFG("ab_set", "S", new_productions));
  }
}

TEST_CASE("eliminate_single_productions") {

  std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
      productions;
  auto id = static_cast<CFG::terminal_type>(common_token::id);
  productions["E"] = {{"E", U'+', "T"}, {"T"}};
  productions["T"] = {{"T", U'*', "F"}, {"F"}};
  productions["F"] = {
      {U'(', "E", U')'}, {id} // i for id
  };

  CFG cfg("common_tokens", "E", productions);

  cfg.eliminate_single_productions();

  std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
      new_productions;

  new_productions["E"] = {
      {"E", "E'"},
      {"F", "E'"},
      {"T", "E'"},
  };
  new_productions["E'"] = {
      {U'+', "F"},
      {U'+', "T"},
  };
  new_productions["F"] = {
      {U'(', "F", "F'"},
      {U'(', "E", "F'"},
      {U'(', "T", "F'"},
      {id},
  };
  new_productions["F'"] = {{U')'}};
  new_productions["T"] = {
      {"F", "T'"},
      {"T", "T'"},
  };
  new_productions["T'"] = {{U'*', "F"}};
  CHECK(cfg == CFG("common_tokens", "E", new_productions));
}

TEST_CASE("to_CNF") {
  std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
      productions;
  productions["S"] = {
      {U'a', "S", U'b', "S"},
      {U'b', "S", U'a', "S"},
      {},
  };
  CFG cfg("ab_set", "S", productions);
  cfg.to_CNF();

  CHECK(cfg.is_CNF());
}
