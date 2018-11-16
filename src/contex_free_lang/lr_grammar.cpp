/*!
 * \file lr_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "lr_grammar.hpp"

namespace cyy::computation {

LR_grammar::parse_node_ptr
LR_grammar::get_parse_tree(symbol_string_view view) const {
  parse_node_ptr parent;
  std::vector<parse_node_ptr> viable_prefix;

  const auto epsilon = alphabet->get_epsilon();
  if (parse(view,
            [&viable_prefix](auto terminal) {
              viable_prefix.push_back(std::make_shared<parse_node>(terminal));
            },

            [&viable_prefix, &parent, this, epsilon](auto const &head,
                                                     auto const &body) {
              parent = std::make_shared<parse_node>(head);

              if (is_epsilon(body)) {
                parent->children.push_back(
                    std::make_shared<parse_node>(epsilon));
              } else {
                const auto body_size = body.size();
                parent->children = {
                    std::move_iterator(viable_prefix.end() - body_size),
                    std::move_iterator(viable_prefix.end())};
                viable_prefix.resize(viable_prefix.size() - body_size);
              }
              viable_prefix.push_back(parent);
            })) {
    return parent;
  }
  return {};
}

bool LR_grammar::parse(symbol_string_view view,
                       const std::function<void(terminal_type)> &shift_callback,
                       const std::function<void(const nonterminal_type &,
                                                const production_body_type &)>
                           &reduction_callback) const {
  if (action_table.empty() || goto_table.empty()) {
    construct_parsing_table();
  }

  std::vector<uint64_t> stack{0};

  const auto endmarker = alphabet->get_endmarker();
  while (true) {
    auto terminal = view.empty() ? endmarker : view.front();

    auto it = action_table.find({stack.back(), terminal});
    if (it == action_table.end()) {
      std::cerr << "no action for ";
      alphabet->print(std::cerr, terminal);
      std::cerr << std::endl;
      return false;
    }

    if (std::holds_alternative<bool>(it->second)) {
      break;
    }

    if (std::holds_alternative<uint64_t>(it->second)) {
      // shift
      stack.push_back(std::get<uint64_t>(it->second));
      shift_callback(terminal);
      view.remove_prefix(1);
      continue;
    }

    // reduce
    auto const &[head, body] = std::get<production_type>(it->second);
    reduction_callback(head, body);

    if (!is_epsilon(body)) {
      stack.resize(stack.size() - body.size());
    }
    auto it2 = goto_table.find({stack.back(), head});
    if (it2 == goto_table.end()) {
      std::cerr << "goto table no find for head:" << head << std::endl;
      return false;
    }
    stack.push_back(it2->second);
  }

  if (!view.empty()) {
    std::cerr << "have symbols remain after parse:";
    for (auto const &terminal : view) {
      print(std::cerr, terminal);
    }
    std::cerr << std::endl;
    return false;
  }
  return true;
}
} // namespace cyy::computation
