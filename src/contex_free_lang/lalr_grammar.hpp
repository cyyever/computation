/*!
 * \file lalr_grammar.hpp
 *
 * \author cyy
 * \date 2018-09-24
 */

#pragma once

#include "../lang/lang.hpp"
#include "canonical_lr_grammar.hpp"
#include "lr_item.hpp"
#include <functional>
#include <set>

namespace cyy::computation {

class LALR_grammar final : public canonical_LR_grammar {

public:
  using canonical_LR_grammar::canonical_LR_grammar;

  std::pair<std::unordered_map<LR_1_item_set, uint64_t>,
            std::map<std::pair<uint64_t, grammar_symbol_type>, uint64_t>>
  canonical_collection() const override;

private:
  std::map<grammar_symbol_type,
           std::map<production_type, std::pair<bool, std::set<terminal_type>>>>
  check_lookahead(const LR_0_item &item) const;
  void construct_parsing_table() const override;
};
} // namespace cyy::computation
