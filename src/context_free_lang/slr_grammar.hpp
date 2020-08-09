/*!
 * \file slr_grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <unordered_map>

#include "../hash.hpp"
#include "lr_grammar.hpp"

namespace cyy::computation {

  class SLR_grammar final : public LR_grammar {

  public:
    using LR_grammar::LR_grammar;

    using collection_type = std::unordered_map<state_type, new_LR_0_item_set>;
    using goto_transition_set_type =
        std::unordered_map<std::pair<state_type, grammar_symbol_type>,
                           state_type>;

    std::pair<collection_type, goto_transition_set_type>
    canonical_collection() const;

  private:
    void construct_parsing_table() const override;
  };
} // namespace cyy::computation
