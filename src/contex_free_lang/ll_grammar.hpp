/*!
 * \file ll_grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <functional>
#include <iostream>

#include "cfg.hpp"

namespace cyy::computation {

  class LL_grammar final : public CFG {
  public:
    explicit LL_grammar(CFG rhs) : CFG(std::move(rhs)) {
      construct_parsing_table();
    }

    LL_grammar(const std::string &alphabet_name,
               const nonterminal_type &start_symbol_,
               std::map<nonterminal_type,
                        std::vector<CFG_production::body_type>> &productions_)
        : CFG(alphabet_name, start_symbol_, productions_) {
      construct_parsing_table();
    }

    bool parse(symbol_string_view view,
               const std::function<void(const CFG_production &, size_t pos)>
                   &match_callback) const;

    parse_node_ptr get_parse_tree(symbol_string_view view) const;

  private:
    void construct_parsing_table();

  private:
    std::map<std::pair<CFG::terminal_type, CFG::nonterminal_type>,
             const CFG_production::body_type &>
        parsing_table;
  };
} // namespace cyy::computation
