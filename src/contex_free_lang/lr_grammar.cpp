/*!
 * \file lr_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "lr_grammar.hpp"

namespace cyy::computation {

LR_grammar::parse_node_ptr LR_grammar::parse(symbol_string_view view) {
  std::vector<parse_node_ptr> viable_prefix;
  std::vector<uint64_t> stack{0};
  auto endmarker = alphabet->get_endmarker();

  if (action_table.empty() || goto_table.empty()) {
    construct_parsing_table();
  }

  while (true) {
    auto terminal = view.empty() ? endmarker : view.front();

    auto it = action_table.find({stack.back(), terminal});
    if (it == action_table.end()) {
      std::cout << "no action for " << static_cast<int>(terminal) << std::endl;

      return {};
    }

    if (std::holds_alternative<bool>(it->second)) {
      break;
    }

    if (std::holds_alternative<uint64_t>(it->second)) {
      // shift
      stack.push_back(std::get<uint64_t>(it->second));
      viable_prefix.push_back(std::make_shared<parse_node>(terminal));
      view.remove_prefix(1);
    } else {
      // reduce

      auto const &production = std::get<production_type>(it->second);

      auto body_size = production.second.size();
      if (body_size == 1 && is_epsilon(production.second[0])) {
        body_size = 0;
      }

      auto parent = std::make_shared<parse_node>(production.first);
      if (body_size == 0) {
        parent->children.push_back(
            std::make_shared<parse_node>(alphabet->get_epsilon()));
      } else {
        parent->children = {std::move_iterator(viable_prefix.end() - body_size),
                            std::move_iterator(viable_prefix.end())};
        stack.resize(stack.size() - body_size);
        viable_prefix.resize(viable_prefix.size() - body_size);
      }

      auto it2 = goto_table.find({stack.back(), production.first});
      if (it2 == goto_table.end()) {
        puts("goto table no find");
        return {};
      }

      stack.push_back(it2->second);
      viable_prefix.push_back(parent);
    }
  }

  if (!view.empty()) {
    puts("no empty view");
    return {};
  }

  return viable_prefix.at(0);
}
} // namespace cyy::computation
