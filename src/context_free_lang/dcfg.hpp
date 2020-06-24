/*!
 * \file dcfg.hpp
 *
 */

#pragma once

#include "cfg.hpp"
#include "dpda.hpp"
#include <optional>

namespace cyy::computation {

  class DCFG : public CFG {

  public:
    DCFG(std::shared_ptr<ALPHABET> alphabet, nonterminal_type start_symbol_,
         production_set_type productions_)
        : CFG(alphabet, start_symbol_, std::move(productions_)),
          new_start_symbol{get_new_head(start_symbol)} {
      if (!DK_test()) {
        throw exception::no_DCFG("DK test failed");
      }
    }

    DPDA to_DPDA() const;

  private:
    bool DK_test() const;

  private:
    mutable std::optional<DFA> dk_opt;
    mutable std::unordered_map<nonterminal_type, symbol_type>
        nonterminal_to_symbol;
    mutable std::unordered_map<DFA::state_type, new_LR_0_item_set>
        state_to_LR_0_item_set;
    nonterminal_type new_start_symbol;
  };
} // namespace cyy::computation
