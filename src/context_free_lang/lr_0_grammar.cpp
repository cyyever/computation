/*!
 * \file lr_1_grammar.cpp
 */

#include "lr_0_grammar.hpp"

#include <fmt/format.h>

#include "exception.hpp"

namespace cyy::computation {
  void LR_0_grammar::construct_parsing_table() const {
    collection_type collection;
    std::tie(collection, goto_table) = get_collection();

    reduction_table.clear();
    for (auto const &[state, set] : collection) {
      for (const auto &item : set.get_completed_items()) {
        // conflict
        auto it = reduction_table.find(state);
        if (it != reduction_table.end()) {
          throw cyy::computation::exception::no_LR_grammar(fmt::format(
              "state {} with production {} conflict with {}", state,
              it->second.to_string(get_alphabet()), item.get_head()));
        }
        reduction_table.emplace(state, item.get_production());
      }
    }
  }

  bool
  LR_0_grammar::parse(symbol_string_view view,
                      const std::function<void(terminal_type)> &shift_callback,
                      const std::function<void(const CFG_production &)>
                          &reduction_callback) const {
    if (reduction_table.empty() || goto_table.empty()) {
      construct_parsing_table();
    }

    std::vector<state_type> stack{0};

    auto endmarked_view = endmarked_symbol_string(view);
    bool parsing_succ = false;
    auto terminal_it = endmarked_view.begin();
    while (terminal_it < endmarked_view.end()) {
      auto terminal = *terminal_it;
      auto it = reduction_table.find(stack.back());
      // reduce
      if (it != reduction_table.end()) {
        auto const &production = it->second;
        auto const &head = production.get_head();
        reduction_callback(production);

        stack.resize(stack.size() - production.get_body().size());
        if (stack.size() == 1) {
          assert(stack[0] == 0);
          if (terminal == ALPHABET::endmarker) {
            parsing_succ = true;
            break;
          }
        }
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

  const LR_0_grammar::goto_table_type &LR_0_grammar::get_goto_table() const {
    if (goto_table.empty()) {
      goto_table = get_collection().second;
    }
    return goto_table;
  }
} // namespace cyy::computation
