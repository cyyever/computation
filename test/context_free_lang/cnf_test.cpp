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

#include "../../src/context_free_lang/cnf.hpp"
#include "../../src/lang/common_tokens.hpp"

using namespace cyy::computation;

TEST_CASE("eliminate_epsilon_productions") {
  CFG::production_set_type productions;
  productions["S'"] = {{"S"}};
  productions["S"] = {
      {'a', "S", 'b', "S"},
      {'b', "S", 'a', "S"},
      {},
  };
  CFG cfg("ab_set", "S'", productions);
  SUBCASE("nullable") {
    auto nullable_nonterminals = cfg.nullable();
    CHECK_EQ(nullable_nonterminals, std::set<CFG::nonterminal_type>{"S", "S'"});
  }
  SUBCASE("eliminate_epsilon_productions") {
    CFG::production_set_type new_productions;
    new_productions["S'"] = {{"S"}, {}};
    new_productions["S"] = {
        {'a', "S", 'b', "S"}, {'a', 'b', "S"}, {'a', "S", 'b'}, {'a', 'b'},
        {'b', "S", 'a', "S"}, {'b', 'a', "S"}, {'b', "S", 'a'}, {'b', 'a'},
    };
    cfg.eliminate_epsilon_productions();
    CHECK_EQ(cfg, CFG("ab_set", "S'", new_productions));
  }
}

TEST_CASE("eliminate_single_productions") {

  std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
      productions;
  auto id = static_cast<CFG::terminal_type>(common_token::id);
  productions["E"] = {{"E", '+', "T"}, {"T"}};
  productions["T"] = {{"T", '*', "F"}, {"F"}};
  productions["F"] = {{'(', "E", ')'}, {id}};

  CFG cfg("common_tokens", "E", productions);

  cfg.eliminate_single_productions();

  CFG::production_set_type new_productions;
  new_productions["E"] = {
      {"E", '+', "T"}, {"T", '*', "F"}, {'(', "E", ')'}, {id}};
  new_productions["F"] = {{'(', "E", ')'}, {id}};
  new_productions["T"] = {{"T", '*', "F"}, {'(', "E", ')'}, {id}};
  CHECK_EQ(cfg, CFG("common_tokens", "E", new_productions));
}

TEST_CASE("to_CNF") {
  std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
      productions;
  productions["S"] = {
      {'a', "S", 'b', "S"},
      {'b', "S", 'a', "S"},
      {},
  };
  CFG cfg("ab_set", "S", productions);
  cfg.to_CNF();
  CNF cnf(cfg);
}
