/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */
#include <doctest/doctest.h>

#include "alphabet/common_tokens.hpp"
#include "context_free_lang/cfg.hpp"
#include "context_free_lang/model_transform.hpp"

using namespace cyy::computation;
TEST_CASE("eliminate_useless_symbols") {
  SUBCASE("have productions after eliminate") {
    CFG::production_set_type productions;
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
    productions["C"] = {{"S"}, {"C"}};

    CFG::production_set_type reduced_productions;
    reduced_productions["S"] = {
        {'0'},
        {"C"},
    };
    reduced_productions["C"] = {{"S"}};

    CHECK_EQ(CFG("common_tokens", "S", productions),
             CFG("common_tokens", "S", reduced_productions));
  }

  SUBCASE("no production after eliminate") {
    CFG::production_set_type productions;

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
  CFG::production_set_type productions;
  auto id = static_cast<CFG::terminal_type>(common_token::id);
  productions["E"] = {
      {"T", "E'"},
  };
  productions["E'"] = {
      {'+', "T", "E'"},
      {},
  };
  productions["T"] = {
      {"F", "T'"},
  };
  productions["T'"] = {
      {'*', "F", "T'"},
      {},
  };
  productions["F"] = {{'(', "E", ')'}, {id}};

  CFG cfg("common_tokens", "E", productions);
  CHECK(cfg.get_terminals() == CFG::terminal_set_type{'+', '*', ')', '(', id});
}

TEST_CASE("eliminate_left_recursion") {
  CFG::production_set_type productions;
  productions["S"] = {
      {"A", 'a'},
      {'b'},
  };
  productions["A"] = {
      {"A", 'c'},
      {"S", 'd'},
      {},
  };

  CFG cfg("common_tokens", "S", productions);
  cfg.eliminate_left_recursion({"S", "A"});

  CFG::production_set_type reduced_productions;
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
      {},
  };

  CHECK_EQ(cfg, CFG("common_tokens", "S", reduced_productions));
}

TEST_CASE("left_factoring") {
  CFG::production_set_type productions;
  productions["S"] = {
      {'i', "E", 't', "S"},
      {'i', "E", 't', "S", 'e', "S"},
      {'a'},
  };
  productions["E"] = {{'b'}};

  CFG cfg("common_tokens", "S", productions);
  cfg.left_factoring();
  CFG::production_set_type reduced_productions;
  reduced_productions["S"] = {
      {'i', "E", 't', "S", "S'"},
      {'a'},
  };
  reduced_productions["S'"] = {
      {'e', "S"},
      {},
  };
  reduced_productions["E"] = {{'b'}};
  CHECK_EQ(cfg, CFG("common_tokens", "S", reduced_productions));
}

TEST_CASE("recursive_descent_parse") {
  CFG::production_set_type productions;
  productions["S"] = {
      {'a', "S", 'a'},
      {'a', 'a'},
  };

  CFG cfg("common_tokens", "S", productions);
  auto terminals = U"aaaa";
  CHECK(cfg.recursive_descent_parse(terminals));
  terminals = U"aaaaaa";
  CHECK(cfg.recursive_descent_parse(terminals));
}

TEST_CASE("first_and_follow") {
  CFG::production_set_type productions;
  auto endmarker = ALPHABET::endmarker;
  auto id = static_cast<CFG::terminal_type>(common_token::id);
  productions["E"] = {
      {"T", "E'"},
  };
  productions["E'"] = {
      {'+', "T", "E'"},
      {},
  };
  productions["T"] = {
      {"F", "T'"},
  };
  productions["T'"] = {
      {'*', "F", "T'"},
      {},
  };
  productions["F"] = {{'(', "E", ')'}, {id}};

  CFG cfg("common_tokens", "E", productions);
  auto first_sets = cfg.first();

  CHECK(first_sets["F"].first == CFG::terminal_set_type{'(', id});
  CHECK(first_sets["T"].first == CFG::terminal_set_type{'(', id});
  CHECK(first_sets["E"].first == CFG::terminal_set_type{'(', id});
  CHECK(first_sets["E'"].first == CFG::terminal_set_type{'+'});
  CHECK(first_sets["T'"].first == CFG::terminal_set_type{'*'});
  CHECK(first_sets["E'"].second);
  CHECK(first_sets["T'"].second);
  auto follow_sets = cfg.follow();

  CHECK(follow_sets["E"] == CFG::terminal_set_type{')', endmarker});
  CHECK(follow_sets["E'"] == CFG::terminal_set_type{')', endmarker});

  CHECK(follow_sets["T"] == CFG::terminal_set_type{'+', ')', endmarker});
  CHECK(follow_sets["T'"] == CFG::terminal_set_type{'+', ')', endmarker});
  CHECK(follow_sets["F"] == CFG::terminal_set_type{'+', '*', ')', endmarker});
}

TEST_CASE("to_PDA") {
  CFG::production_set_type productions;
  productions["S"] = {{'a', "T", 'b'}, {'c', 'd'}, {'b'}};
  productions["T"] = {
      {"T", 'a'},
      {},
  };

  CFG cfg("common_tokens", "S", productions);
  auto pda = CFG_to_PDA(cfg);
  SUBCASE("aab") {
    symbol_string str = U"aab";
    CHECK(pda.recognize(str));
  }
  SUBCASE("cd") {
    symbol_string str = U"cd";
    CHECK(pda.recognize(str));
  }
  SUBCASE("ab") {
    symbol_string str = U"ab";
    CHECK(pda.recognize(str));
  }

  SUBCASE("b") {
    symbol_string str = U"b";
    CHECK(pda.recognize(str));
  }
  SUBCASE("a") {
    symbol_string str = U"a";
    CHECK(!pda.recognize(str));
  }
}
TEST_CASE("MMA_draw") {
  CFG::production_set_type productions;
  auto id = static_cast<CFG::terminal_type>(common_token::id);
  productions["E"] = {
      {"T", "E'"},
  };
  productions["E'"] = {
      {'+', "T", "E'"},
      {},
  };
  productions["T"] = {
      {"F", "T'"},
  };
  productions["T'"] = {
      {'*', "F", "T'"},
      {},
  };
  productions["F"] = {{'(', "E", ')'}, {id}};

  CFG cfg("common_tokens", "E", productions);
  std::cout << cfg.MMA_draw() << std::endl;
}
