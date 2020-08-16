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
    collection_type collection;
    std::tie(collection, goto_table) = get_collection();

    for (auto const &[p, next_state] : goto_table) {
      if (p.second.is_terminal()) {
        assert(p.second.get_terminal() != ALPHABET::endmarker);
        action_table[{p.first, p.second.get_terminal()}] = next_state;
      }
    }
    for (auto const &[state, set] : collection) {
      for (const auto &[kernel_item, lookahead_set] :
           set.get_completed_items()) {

        for (const auto &lookahead : lookahead_set) {
          // conflict
          auto it = action_table.find({state, lookahead});
          if (it != action_table.end()) {
            std::ostringstream os;
            os << "state " << state << " with head " << kernel_item.get_head()
               << " conflict with follow terminal "
               << alphabet->to_string(lookahead) << " and action index "
               << it->second.index();
            throw cyy::computation::exception::no_LR_1_grammar(os.str());
          }
          if (lookahead == ALPHABET::endmarker &&
              kernel_item.get_head() == get_start_symbol()) {
            assert(lookahead_set.size() == 1);
            action_table[{state, lookahead}] = true;
          } else {
            action_table[{state, lookahead}] = kernel_item.get_production();
          }
        }
      }
    }
    const_cast<LR_1_grammar *>(this)->remove_head(get_start_symbol());
  }
} // namespace cyy::computation
