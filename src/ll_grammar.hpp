/*!
 * \file ll_grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include "grammar.hpp"
#include "lang.hpp"

namespace cyy::lang {

class LL_grammar : public CFG {

public:
  explicit LL_grammar(CFG rhs) : CFG(std::move(rhs)) {
    if (!is_LL1()) {
      throw std::invalid_argument("not a LL1 grammar");
    }
  }

  LL_grammar(const std::string &alphabet_name,
             const nonterminal_type &start_symbol_,
             std::map<nonterminal_type, std::vector<production_body_type>>
                 &productions_)
      : CFG(alphabet_name, start_symbol_, productions_) {
    if (!is_LL1()) {
      throw std::invalid_argument("not a LL1 grammar");
    }
  }

  parse_node_ptr parse(symbol_string_view view) const;

private:
  bool is_LL1() const;

  bool is_LL1(const std::map<nonterminal_type, std::set<terminal_type>>
                  &nonterminal_first_sets,
              const std::map<nonterminal_type, std::set<terminal_type>>
                  &follow_sets) const;
};
} // namespace cyy::lang
