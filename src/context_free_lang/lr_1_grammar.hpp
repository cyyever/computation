/*!
 * \file lr_1_grammar.hpp
 *
 */

#pragma once

#include "lr_1_item.hpp"
#include "lr_grammar.hpp"

namespace cyy::computation {

  class LR_1_grammar : public LR_grammar {

  public:
    using LR_grammar::LR_grammar;
    using collection_type = std::unordered_map<state_type, LR_1_item_set>;
    using goto_transition_map_type =
        std::unordered_map<std::pair<state_type, grammar_symbol_type>,
                           state_type>;

    virtual std::pair<collection_type, goto_transition_map_type>
    get_collection() const = 0;

  private:
    void construct_parsing_table() const override;
  };
} // namespace cyy::computation
