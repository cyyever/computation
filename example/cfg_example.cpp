/*!
 * \file hardware_example.cpp
 *
 * \brief 硬件相关函数例子
 * \author cyy
 */

#include <cassert>
#include <iostream>

#include "../src/grammar.hpp"
#include "../src/common_tokens.hpp"

int main() {
using namespace cyy::lang;

  std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
      productions;
  productions["rexpr"] = {
      {"rexpr", '+',"rterm"},
      {"rterm"},
  };
  productions["rterm"] = {
      {"rterm", "rfactor"},
      {"rfactor"},
  };
  productions["rfactor"] = {
      {"rfactor",'*'},
      {"rprimary"},
  };
  productions["rprimary"] = {
      {'(',"rexpr",')'},
      {common_tokens::token::ascii_char},
      {common_tokens::token::escape_sequence},
  };

  CFG cfg("common_tokens", "rexpr", productions);
  cfg.eliminate_left_recursion();
  if(cfg.is_LL1()) {
    std::cout<<"regex grammar is LL(1) grammar\n";
  } else {
    std::cout<<"regex grammar is not LL(1) grammar\n";
  }

  cfg.print(std::cout);
  return 0;
}
