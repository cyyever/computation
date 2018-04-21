/*!
 * \file slr_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "slr_grammar.hpp"

namespace cyy::lang {

LR_0_item_set SLR_grammar::closure(LR_0_item_set set) const {
  while (true) {
    bool has_added = false;
    for (const auto &kernel_item : set.kernel_items) {
      if (kernel_item.dot_pos < kernel_item.body.size()) {
        auto const &symbol = kernel_item.body[kernel_item.dot_pos];
        if (!std::holds_alternative<nonterminal_type>(symbol)) {
          continue;
        }
        if (set.nonkernel_items.insert(std::get<nonterminal_type>(symbol))
                .second) {
          has_added = true;
        }
      }
    }

    for (const auto &nonkernel_item : set.nonkernel_items) {
      auto it = productions.find(nonkernel_item);

      for (auto const &body : it->second) {
        auto const &symbol = body[0];
        if (!std::holds_alternative<nonterminal_type>(symbol)) {
          continue;
        }
        if (set.nonkernel_items.insert(std::get<nonterminal_type>(symbol))
                .second) {
          has_added = true;
        }
      }
    }
    if (!has_added) {
      break;
    }
  }
  return set;
}

LR_0_item_set SLR_grammar::GOTO(LR_0_item_set set,
                                const grammar_symbol_type &symbol) const {
  LR_0_item_set res;
  while (!set.kernel_items.empty()) {
    LR_0_item kernel_item =
        std::move(set.kernel_items.extract(set.kernel_items.begin()).value());
    if (kernel_item.dot_pos < kernel_item.body.size()) {
      if (kernel_item.body[kernel_item.dot_pos] != symbol) {
        continue;
      }
      kernel_item.dot_pos++;
      res.kernel_items.emplace(std::move(kernel_item));
    }
  }

  while (!set.nonkernel_items.empty()) {
    nonterminal_type nonkernel_item = std::move(
        set.nonkernel_items.extract(set.nonkernel_items.begin()).value());
    auto it = productions.find(nonkernel_item);

    for (auto const &body : it->second) {
      if (body[0] == symbol) {

        LR_0_item new_kernel_item;
        new_kernel_item.head = nonkernel_item;
        new_kernel_item.body = body;
        new_kernel_item.dot_pos = 1;
        res.kernel_items.emplace(std::move(new_kernel_item));
      }
    }
  }

  return closure(std::move(res));
}
} // namespace cyy::lang
