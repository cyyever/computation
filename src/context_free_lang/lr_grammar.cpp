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

} // namespace cyy::computation
