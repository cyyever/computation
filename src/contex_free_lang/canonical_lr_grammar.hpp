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

#include "../lang/lang.hpp"
#include "lr_grammar.hpp"
#include "lr_item.hpp"

namespace cyy::computation {

class canonical_LR_grammar : public LR_grammar {

public:
  canonical_LR_grammar(
      const std::string &alphabet_name, const nonterminal_type &start_symbol_,
      std::map<nonterminal_type, std::vector<production_body_type>> productions)
      : LR_grammar(alphabet_name, start_symbol_, productions),
        new_start_symbol(get_new_head(start_symbol)) {}

  virtual std::pair<
      std::unordered_map<LR_1_item_set, uint64_t>,
      std::map<std::pair<uint64_t, grammar_symbol_type>, uint64_t>>
  canonical_collection() const;

protected:
  std::map<grammar_symbol_type, LR_1_item_set>
  GOTO(const LR_1_item_set &set) const;
  void construct_parsing_table() const override;

protected:
  nonterminal_type new_start_symbol;
};
} // namespace cyy::computation
