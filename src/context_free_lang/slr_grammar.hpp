/*!
 * \file slr_grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <functional>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "../hash.hpp"
#include "../lang/alphabet.hpp"
#include "lr_grammar.hpp"
#include "lr_item.hpp"

namespace cyy::computation {

  class SLR_grammar final : public LR_grammar {

  public:
    using LR_grammar::LR_grammar;
    using collection_type = std::unordered_map<LR_0_item_set, state_type>;
    using goto_transition_set_type =
        std::unordered_map<std::pair<state_type, grammar_symbol_type>,
                           state_type>;

    std::pair<collection_type, goto_transition_set_type>
    canonical_collection() const;

  private:
    std::unordered_map<grammar_symbol_type, LR_0_item_set>
    GOTO(const LR_0_item_set &set) const;

    void construct_parsing_table() const override;

  private:
    nonterminal_type new_start_symbol{get_new_head(start_symbol)};
  };
} // namespace cyy::computation
