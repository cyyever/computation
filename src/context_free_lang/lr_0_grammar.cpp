/*!
 * \file lr_1_grammar.cpp
 *
 */

#include "lr_0_grammar.hpp"

#include <iostream>
#include <sstream>

#include "../exception.hpp"

namespace cyy::computation {
  void LR_0_grammar::construct_parsing_table() const {
    collection_type collection;
    std::tie(collection, goto_table) = get_collection();

    for (auto const &[state, set] : collection) {
      for (const auto &item : set.get_completed_items()) {
        // conflict
        auto it = action_table.find(state);
        if (it != action_table.end()) {
          std::ostringstream os;
          os << "state " << state << " with production ";
          it->second.print(os, get_alphabet());
          os << " conflict with " << item.get_head();
          throw cyy::computation::exception::no_LR_grammar(os.str());
        }
        action_table.emplace(state, item.get_production());
        if (item.get_head() == get_start_symbol()) {
          final_states.insert(state);
        }
      }
    }
  }

  bool
  LR_0_grammar::parse(symbol_string_view view,
                      const std::function<void(terminal_type)> &shift_callback,
                      const std::function<void(const CFG_production &)>
                          &reduction_callback) const {
    if (action_table.empty() || goto_table.empty()) {
      construct_parsing_table();
    }

    std::vector<state_type> stack{0};

    auto endmarked_view = endmarked_symbol_string(view);
    bool parsing_succ = false;
    for (auto terminal_it = endmarked_view.begin();
         terminal_it < endmarked_view.end();) {
      auto terminal = *terminal_it;
      auto it = action_table.find(stack.back());
      // reduce
      if (it != action_table.end()) {
        auto const &production = it->second;
        auto const &head = production.get_head();
        reduction_callback(production);

        if (terminal == ALPHABET::endmarker &&
            final_states.contains(stack.back())) {
          parsing_succ = true;
        }
        stack.resize(stack.size() - production.get_body().size());
        auto it2 = goto_table.find({stack.back(), head});
        if (it2 == goto_table.end()) {
          break;
        }
        stack.push_back(it2->second);
        continue;
      }

      if (terminal == ALPHABET::endmarker) {
        break;
      }
      auto it2 = goto_table.find({stack.back(), terminal});
      if (it2 == goto_table.end()) {
        std::cerr << "no action for state " << stack.back() << " and terminal "
                  << alphabet->to_string(terminal) << std::endl;
        return false;
      }
      stack.push_back(it2->second);
      shift_callback(terminal);
      terminal_it++;
    }
    if (parsing_succ) {
      return true;
    }
    std::cerr << "parsing ended at state" << stack.back() << std::endl;
    return false;
  }
} // namespace cyy::computation
