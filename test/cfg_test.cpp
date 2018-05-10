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

using namespace cyy::computation;
TEST_CASE("eliminate_useless_symbols") {
  SUBCASE("have productions after eliminate") {
    std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
        productions;
    productions["S"] = {
        {"A"},
        {"C"},
        {'0'},
    };
    productions["A"] = {
        {"A"},
        {"b"},
    };
    productions["B"] = {{'1'}};
    productions["C"] = {{"S"}};

    std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
        reduced_productions;
    reduced_productions["S"] = {
        {'0'},
        {"C"},
    };
    reduced_productions["C"] = {{"S"}};

    CHECK(CFG("common_tokens", "S", productions) ==
          CFG("common_tokens", "S", reduced_productions));
  }

  SUBCASE("no production after eliminate") {
    std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
        productions;

    productions["S"] = {{"A"}};
    productions["A"] = {{"S"}};

    bool has_exception = false;
    try {
      CFG("common_tokens", "S", productions);
    } catch (...) {
      has_exception = true;
    }
    CHECK(has_exception);
  }
}

TEST_CASE("get_terminals") {
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

  CFG cfg("common_tokens", "E", productions);
  CHECK(cfg.get_terminals() ==
        std::set<CFG::terminal_type>{'+', '*', ')', '(', id});
}

TEST_CASE("eliminate_left_recursion") {
  std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
      productions;
  productions["S"] = {
      {"A", 'a'},
      {'b'},
  };
  productions["A"] = {
      {"A", 'c'},
      {"S", 'd'},
      {ALPHABET::get("common_tokens")->get_epsilon()},
  };

  CFG cfg("common_tokens", "S", productions);
  cfg.eliminate_left_recursion({"S", "A"});

  std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
      reduced_productions;
  reduced_productions["S"] = {
      {"A", 'a'},
      {'b'},
  };
  reduced_productions["A"] = {
      {'b', 'd', "A'"},
      {"A'"},
  };
  reduced_productions["A'"] = {
      {'c', "A'"},
      {'a', 'd', "A'"},
      {ALPHABET::get("common_tokens")->get_epsilon()},
  };

  CHECK(cfg == CFG("common_tokens", "S", reduced_productions));
}

TEST_CASE("left_factoring") {
  std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
      productions;
  productions["S"] = {
      {'i', "E", 't', "S"},
      {'i', "E", 't', "S", 'e', "S"},
      {'a'},
  };
  productions["E"] = {{'b'}};

  CFG cfg("common_tokens", "S", productions);
  cfg.left_factoring();
  std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
      reduced_productions;
  reduced_productions["S"] = {
      {'i', "E", 't', "S", "S'"},
      {'a'},
  };
  reduced_productions["S'"] = {
      {'e', "S"},
      {ALPHABET::get("common_tokens")->get_epsilon()},
  };
  reduced_productions["E"] = {{'b'}};
  CHECK(cfg == CFG("common_tokens", "S", reduced_productions));
}

TEST_CASE("recursive_descent_parse") {
  std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
      productions;
  productions["S"] = {
      {'a', "S", 'a'},
      {'a', 'a'},
  };

  CFG cfg("common_tokens", "S", productions);
  std::vector<symbol_type> terminals(4, 'a');
  CHECK(cfg.recursive_descent_parse({terminals.data(), terminals.size()}));
}

TEST_CASE("first_and_follow") {
  std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
      productions;
  auto epsilon = ALPHABET::get("common_tokens")->get_epsilon();
  auto endmarker = ALPHABET::get("common_tokens")->get_endmarker();
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

  CFG cfg("common_tokens", "E", productions);
  auto first_sets = cfg.first();

  CHECK(first_sets["F"] == std::set<CFG::terminal_type>{'(', id});
  CHECK(first_sets["T"] == std::set<CFG::terminal_type>{'(', id});
  CHECK(first_sets["E"] == std::set<CFG::terminal_type>{'(', id});
  CHECK(first_sets["E'"] == std::set<CFG::terminal_type>{'+', epsilon});
  CHECK(first_sets["T'"] == std::set<CFG::terminal_type>{'*', epsilon});
  auto follow_sets = cfg.follow();

  CHECK(follow_sets["E"] == std::set<CFG::terminal_type>{')', endmarker});
  CHECK(follow_sets["E'"] == std::set<CFG::terminal_type>{')', endmarker});
  CHECK(follow_sets["T"] == std::set<CFG::terminal_type>{'+', ')', endmarker});
  CHECK(follow_sets["T'"] == std::set<CFG::terminal_type>{'+', ')', endmarker});
  CHECK(follow_sets["F"] ==
        std::set<CFG::terminal_type>{'+', '*', ')', endmarker});
}
