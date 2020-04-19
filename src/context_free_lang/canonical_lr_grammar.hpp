/*!
 * \file canonical_lr_grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include "../hash.hpp"
#include "lr_grammar.hpp"
#include "lr_item.hpp"

namespace cyy::computation {

  class canonical_LR_grammar : public LR_grammar {

  public:
    using LR_grammar::LR_grammar;
    using collection_type = std::unordered_map<LR_1_item_set, state_type>;
    using goto_transition_set_type =
        std::unordered_map<std::pair<state_type, grammar_symbol_type>,
                           state_type>;

    virtual std::pair<collection_type, goto_transition_set_type>
    canonical_collection() const;

  protected:
    std::unordered_map<grammar_symbol_type, LR_1_item_set>
    GOTO(const LR_1_item_set &set) const;
    void construct_parsing_table() const override;

  protected:
    nonterminal_type new_start_symbol{get_new_head(start_symbol)};
  };
} // namespace cyy::computation
