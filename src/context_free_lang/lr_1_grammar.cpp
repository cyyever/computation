/*!
 * \file lr_1_grammar.cpp
 *
 */

#include <iostream>
#include <sstream>

#include "../exception.hpp"
#include "lr_1_grammar.hpp"

namespace cyy::computation {
  void LR_1_grammar::construct_parsing_table() const {
    const_cast<LR_1_grammar *>(this)->normalize_start_symbol();
    auto [collection, goto_transitions] = get_collection();

    for (auto const &[p, next_state] : goto_transitions) {
      auto ptr = p.second.get_nonterminal_ptr();
      if (ptr) {
        goto_table[{p.first, *ptr}] = next_state;
      } else {
        action_table[{p.first, p.second.get_terminal()}] = next_state;
      }
    }
    for (auto const &[state, set] : collection) {
      for (const auto &[kernel_item, lookahead_set] : set.get_kernel_items()) {
        if (!kernel_item.completed()) {
          continue;
        }

        if (kernel_item.get_head() == start_symbol) {
          action_table[{state, ALPHABET::endmarker}] = true;
          continue;
        }

        for (const auto &lookahead : lookahead_set) {
          // conflict
          if (action_table.contains({state, lookahead})) {
            std::ostringstream os;
            os << "state " << state << " conflict with follow terminal"
               << static_cast<int>(lookahead) << std::endl;
            throw cyy::computation::exception::no_LR_1_grammar(os.str());
          }
          action_table[{state, lookahead}] = kernel_item.get_production();
        }
      }
    }
    const_cast<LR_1_grammar *>(this)->remove_head(start_symbol,
                                                  old_start_symbol);
  }
} // namespace cyy::computation
