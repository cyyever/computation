/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <iostream>

#include "../src/grammar.hpp"

using namespace cyy::lang;
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

    CHECK(CFG("ASCII", "S", productions) ==
          CFG("ASCII", "S", reduced_productions));
  }

  SUBCASE("no production after eliminate") {
    std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
        productions;

    productions["S"] = {{"A"}};
    productions["A"] = {{"S"}};

    bool has_exception = false;
    try {
      CFG("ASCII", "S", productions);
    } catch (...) {
      has_exception = true;
    }
    CHECK(has_exception);
  }
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
      {make_alphabet("ASCII")->get_epsilon()},
  };

  CFG cfg("ASCII", "S", productions);
  cfg.eliminate_left_recursion();

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
      {make_alphabet("ASCII")->get_epsilon()},
  };

  CHECK(cfg == CFG("ASCII", "S", reduced_productions));
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

  CFG cfg("ASCII", "S", productions);
  cfg.left_factoring();
  std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
      reduced_productions;
  reduced_productions["S"] = {
      {'i', "E", 't', "S", "S'"},
      {'a'},
  };
  reduced_productions["S'"] = {
      {'e', "S"},
      {make_alphabet("ASCII")->get_epsilon()},
  };
  reduced_productions["E"] = {{'b'}};
  CHECK(cfg == CFG("ASCII", "S", reduced_productions));
}
