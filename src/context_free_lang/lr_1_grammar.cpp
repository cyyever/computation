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
      assert(collection.contains(p.first));
      if (collection[p.first].empty()) {
        continue;
      }
      if (p.second.is_terminal()) {
        assert(p.second.get_terminal() != ALPHABET::endmarker);
        if (collection[next_state].empty()) {
          continue;
        }
        action_table[{p.first, p.second.get_terminal()}] = next_state;
      }
    }
    for (auto const &[state, set] : collection) {
      for (const auto &item : set.get_completed_items()) {
        for (const auto &lookahead : item.get_lookahead_symbols()) {

          // conflict
          auto it = action_table.find({state, lookahead});
          if (it != action_table.end()) {
            std::ostringstream os;
            os << "state " << state << " with head " << item.get_head()
               << " conflict with follow terminal "
               << alphabet->to_string(lookahead) << " and action index "
               << it->second.index();
            throw cyy::computation::exception::no_LR_1_grammar(os.str());
          }
          if (item.get_head() == get_start_symbol()) {
            assert(item.get_lookahead_symbols().size() == 1);
            assert(lookahead == ALPHABET::endmarker);
            action_table[{state, lookahead}] = true;
          } else {
            action_table[{state, lookahead}] = item.get_production();
          }
        }
      }
    }
    const_cast<LR_1_grammar *>(this)->remove_head(get_start_symbol());
  }
  bool
  LR_1_grammar::parse(symbol_string_view view,
                      const std::function<void(terminal_type)> &shift_callback,
                      const std::function<void(const CFG_production &)>
                          &reduction_callback) const {
    if (action_table.empty() || goto_table.empty()) {
      construct_parsing_table();
    }

    std::vector<state_type> stack{0};

    auto endmarked_view = endmarked_symbol_string(view);
    auto terminal_it = endmarked_view.begin();
    while (true) {
      auto terminal = *terminal_it;

      auto it = action_table.find({stack.back(), terminal});
      if (it == action_table.end()) {
        std::cerr << "no action for state " << stack.back() << " and terminal "
                  << alphabet->to_string(terminal) << std::endl;
        return false;
      }

      if (std::holds_alternative<bool>(it->second)) {
        terminal_it++;
        break;
      }

      if (std::holds_alternative<state_type>(it->second)) {
        // shift
        stack.push_back(std::get<state_type>(it->second));
        shift_callback(terminal);
        terminal_it++;
        continue;
      }

      auto const &production = std::get<CFG_production>(it->second);
      // reduce
      auto const &head = std::get<CFG_production>(it->second).get_head();
      auto const &body = std::get<CFG_production>(it->second).get_body();
      reduction_callback(production);

      stack.resize(stack.size() - body.size());
      auto it2 = goto_table.find({stack.back(), head});
      if (it2 == goto_table.end()) {
        std::cerr << "goto table no find for head:" << head << std::endl;
        return false;
      }
      stack.push_back(it2->second);
    }
    assert(terminal_it == endmarked_view.end());
    return true;
  }
} // namespace cyy::computation
