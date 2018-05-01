/*!
 * \file canonical_lr_grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <functional>
#include <set>
#include <unordered_set>

#include "lang.hpp"
#include "lr_grammar.hpp"

namespace cyy::lang {

class canonical_LR_grammar : public LR_grammar {

public:
  canonical_LR_grammar(const std::string &alphabet_name,
              const nonterminal_type &start_symbol_,
              std::map<nonterminal_type, std::vector<production_body_type>>
                  &productions_)
      : LR_grammar(alphabet_name, start_symbol_, productions_),
        new_start_symbol(get_new_head(start_symbol)),first_sets(first()) {
    construct_parsing_table();
  }

  std::pair<std::vector<LR_1_item_set>,
            std::map<std::pair<uint64_t, grammar_symbol_type>, uint64_t>>

  canonical_collection() const;

private:
  LR_1_item_set closure(LR_1_item_set set) const;
  LR_1_item_set GOTO(LR_1_item_set set,
                     const grammar_symbol_type &symbol) const;
  void construct_parsing_table() override;

private:
  nonterminal_type new_start_symbol;
  std::map<nonterminal_type, std::set<terminal_type>> first_sets;
};
} // namespace cyy::lang
