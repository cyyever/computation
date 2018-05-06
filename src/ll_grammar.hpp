/*!
 * \file ll_grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include "cfg.hpp"
#include "lang.hpp"

namespace cyy::lang {

class LL_grammar : public CFG {

public:
  explicit LL_grammar(CFG rhs) : CFG(std::move(rhs)) {
    construct_parsing_table();
  }

  LL_grammar(const std::string &alphabet_name,
             const nonterminal_type &start_symbol_,
             std::map<nonterminal_type, std::vector<production_body_type>>
                 &productions_)
      : CFG(alphabet_name, start_symbol_, productions_) {
    construct_parsing_table();
  }

  parse_node_ptr parse(symbol_string_view view) const;

private:
  void construct_parsing_table();

private:
  std::map<std::pair<CFG::terminal_type, CFG::nonterminal_type>,
           const production_body_type &>
      parsing_table;
};
} // namespace cyy::lang
