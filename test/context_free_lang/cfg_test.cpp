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

#include "../../src/context_free_lang/cfg.hpp"
#include "../../src/context_free_lang/model_transform.hpp"
#include "../../src/lang/common_tokens.hpp"

using namespace cyy::computation;
TEST_CASE("eliminate_useless_symbols") {
  SUBCASE("have productions after eliminate") {
    std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
        productions;
    productions["S"] = {
        {"A"},
        {"C"},
        {U'0'},
    };
    productions["A"] = {
        {"A"},
        {"b"},
    };
    productions["B"] = {{U'1'}};
    productions["C"] = {{"S"}};

    std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
        reduced_productions;
    reduced_productions["S"] = {
        {U'0'},
        {"C"},
    };
    reduced_productions["C"] = {{"S"}};

    CHECK(CFG("common_tokens", "S", productions) ==
          CFG("common_tokens", "S", reduced_productions));
  }

  SUBCASE("no production after eliminate") {
    std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
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
  std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
      productions;
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

  CFG cfg("common_tokens", "E", productions);
  CHECK(cfg.get_terminals() ==
        std::set<CFG::terminal_type>{U'+', U'*', U')', U'(', id});
}

TEST_CASE("eliminate_left_recursion") {
  std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
      productions;
  productions["S"] = {
      {"A", U'a'},
      {U'b'},
  };
  productions["A"] = {
      {"A", U'c'},
      {"S", U'd'},
      {},
  };

  CFG cfg("common_tokens", "S", productions);
  cfg.eliminate_left_recursion({"S", "A"});

  std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
      reduced_productions;
  reduced_productions["S"] = {
      {"A", U'a'},
      {U'b'},
  };
  reduced_productions["A"] = {
      {U'b', U'd', "A'"},
      {"A'"},
  };
  reduced_productions["A'"] = {
      {U'c', "A'"},
      {U'a', U'd', "A'"},
      {},
  };

  CHECK_EQ(cfg, CFG("common_tokens", "S", reduced_productions));
}

TEST_CASE("left_factoring") {
  std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
      productions;
  productions["S"] = {
      {'i', "E", 't', "S"},
      {'i', "E", 't', "S", 'e', "S"},
      {'a'},
  };
  productions["E"] = {{U'b'}};

  CFG cfg("common_tokens", "S", productions);
  cfg.left_factoring();
  std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
      reduced_productions;
  reduced_productions["S"] = {
      {U'i', "E", U't', "S", "S'"},
      {U'a'},
  };
  reduced_productions["S'"] = {
      {U'e', "S"},
      {},
  };
  reduced_productions["E"] = {{U'b'}};
  CHECK_EQ(cfg, CFG("common_tokens", "S", reduced_productions));
}

TEST_CASE("recursive_descent_parse") {
  std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
      productions;
  productions["S"] = {
      {U'a', "S", U'a'},
      {U'a', U'a'},
  };

  CFG cfg("common_tokens", "S", productions);
  std::vector<symbol_type> terminals(4, U'a');
  CHECK(cfg.recursive_descent_parse({terminals.data(), terminals.size()}));
}

TEST_CASE("first_and_follow") {
  std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
      productions;
  auto endmarker = ALPHABET::get("common_tokens")->get_endmarker();
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

  CFG cfg("common_tokens", "E", productions);
  auto first_sets = cfg.first();

  CHECK(first_sets["F"].first == std::set<CFG::terminal_type>{U'(', id});
  CHECK(first_sets["T"].first == std::set<CFG::terminal_type>{U'(', id});
  CHECK(first_sets["E"].first == std::set<CFG::terminal_type>{U'(', id});
  CHECK(first_sets["E'"].first == std::set<CFG::terminal_type>{U'+'});
  CHECK(first_sets["T'"].first == std::set<CFG::terminal_type>{U'*'});
  CHECK(first_sets["E'"].second);
  CHECK(first_sets["T'"].second);
  auto follow_sets = cfg.follow();

  CHECK(follow_sets["E"] == std::set<CFG::terminal_type>{U')', endmarker});
  CHECK(follow_sets["E'"] == std::set<CFG::terminal_type>{U')', endmarker});
  CHECK(follow_sets["T"] ==
        std::set<CFG::terminal_type>{U'+', U')', endmarker});
  CHECK(follow_sets["T'"] ==
        std::set<CFG::terminal_type>{U'+', U')', endmarker});
  CHECK(follow_sets["F"] ==
        std::set<CFG::terminal_type>{U'+', U'*', U')', endmarker});
}

TEST_CASE("to_PDA") {
  std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
      productions;
  productions["S"] = {{U'a', "T", U'b'}, {U'c', U'd'}, {U'b'}};
  productions["T"] = {
      {"T", U'a'},
      {},
  };

  CFG cfg("common_tokens", "S", productions);
  auto pda = CFG_to_PDA(cfg);
  SUBCASE("aab") {
    symbol_string str = U"aab";
    CHECK(pda.simulate(str));
  }
  SUBCASE("cd") {
    symbol_string str = U"cd";
    CHECK(pda.simulate(str));
  }
  SUBCASE("ab") {
    symbol_string str = U"ab";
    CHECK(pda.simulate(str));
  }

  SUBCASE("b") {
    symbol_string str = U"b";
    CHECK(pda.simulate(str));
  }
  SUBCASE("a") {
    symbol_string str = U"a";
    CHECK(!pda.simulate(str));
  }
}
