/*!
 * \file canonical_lr_grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once
#include "lr_1_grammar.hpp"

namespace cyy::computation {

  class canonical_LR_grammar : public LR_1_grammar {

  public:
    using LR_1_grammar::LR_1_grammar;

    std::pair<collection_type, goto_transition_map_type>
    get_collection() const override;

  private:
    std::unordered_map<grammar_symbol_type, LR_1_item_set>
    GOTO(const LR_1_item_set &set) const;
  };
} // namespace cyy::computation
