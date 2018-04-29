/*!
 * \file slr_grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <functional>
#include <set>
#include <unordered_set>

#include "lr_grammar.hpp"
#include "lang.hpp"
#include "lr_item.hpp"

namespace cyy::lang {

class SLR_grammar : public LR_grammar {

public:
  SLR_grammar(const std::string &alphabet_name,
              const nonterminal_type &start_symbol_,
              std::map<nonterminal_type, std::vector<production_body_type>>
                  &productions_)
      : LR_grammar(alphabet_name, start_symbol_, productions_) {}

  std::pair<std::vector<LR_0_item_set>,
            std::map<std::pair<uint64_t, grammar_symbol_type>, uint64_t>>
  canonical_collection() const;

private:
  LR_0_item_set closure(LR_0_item_set set) const;
  LR_0_item_set GOTO(LR_0_item_set set,
                     const grammar_symbol_type &symbol) const;
  void construct_parsing_table();

private:
  std::map<std::pair<uint64_t, terminal_type>,
           std::variant<uint64_t, production_type>>
      action_table;
  std::map<std::pair<uint64_t, nonterminal_type>, uint64_t> goto_table;
};
} // namespace cyy::lang
