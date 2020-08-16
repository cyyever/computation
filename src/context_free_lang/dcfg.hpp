/*!
 * \file dcfg.hpp
 *
 */

#pragma once

#include <optional>

#include "dpda.hpp"
#include "lr_1_grammar.hpp"

namespace cyy::computation {

  class DCFG : public LR_grammar {

  public:
    DCFG(std::shared_ptr<ALPHABET> alphabet_, nonterminal_type start_symbol_,
         production_set_type productions_);

    DPDA to_DPDA() const;

  private:
    bool DK_test() const;
    void construct_parsing_table() const override;

  private:
    mutable std::optional<DFA> dk_opt;
    mutable std::unordered_map<nonterminal_type, symbol_type>
        nonterminal_to_symbol;
    mutable std::unordered_map<DFA::state_type, LR_0_item_set>
        state_to_LR_0_item_set;
  };
} // namespace cyy::computation
