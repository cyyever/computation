/*!
 * \file lr_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "lr_grammar.hpp"
#include <iostream>

namespace cyy::computation {

  LR_grammar::parse_node_ptr
  LR_grammar::get_parse_tree(symbol_string_view view) const {
    parse_node_ptr parent;
    std::vector<parse_node_ptr> viable_prefix;

    if (parse(
            view,
            [&viable_prefix](auto terminal) {
              viable_prefix.push_back(std::make_shared<parse_node>(terminal));
            },

            [&viable_prefix, &parent](auto const &production) {
              parent = std::make_shared<parse_node>(production.get_head());

              if (!production.is_epsilon()) {
                const auto body_size = production.get_body().size();
                parent->children = {
                    std::move_iterator(viable_prefix.end() -
                                       static_cast<std::ptrdiff_t>(body_size)),
                    std::move_iterator(viable_prefix.end())};
                viable_prefix.resize(viable_prefix.size() - body_size);
              }
              viable_prefix.push_back(parent);
            })) {
      return parent;
    }
    return {};
  }

  bool
  LR_grammar::parse(symbol_string_view view,
                    const std::function<void(terminal_type)> &shift_callback,
                    const std::function<void(const CFG_production &)>
                        &reduction_callback) const {
    if (action_table.empty() || goto_table.empty()) {
      construct_parsing_table();
    }

    std::vector<state_type> stack{0};

    const auto endmarker = alphabet->get_endmarker();
    while (true) {
      auto terminal = view.empty() ? endmarker : view.front();

      auto it = action_table.find({stack.back(), terminal});
      if (it == action_table.end()) {
        std::cerr << "no action for ";
        alphabet->to_string(terminal);
        std::cerr << std::endl;
        return false;
      }

      if (std::holds_alternative<bool>(it->second)) {
        break;
      }

      if (std::holds_alternative<state_type>(it->second)) {
        // shift
        stack.push_back(std::get<state_type>(it->second));
        shift_callback(terminal);
        view.remove_prefix(1);
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
    return true;
  }
} // namespace cyy::computation
