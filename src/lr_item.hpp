/*!
 * \file lr_item.hpp
 *
 * \brief
 * \author cyy
 * \date 2018-04-21
 */

#pragma once

#include <functional>
#include <set>
#include <unordered_set>

#include "cfg.hpp"

namespace cyy::lang {

struct LR_0_item {
  CFG::nonterminal_type head;
  CFG::production_body_type body;
  size_t dot_pos;
  bool operator==(const LR_0_item &rhs) const {
    return head == rhs.head && body == rhs.body && dot_pos == rhs.dot_pos;
  }
};

} // namespace cyy::lang

namespace std {
template <> struct hash<cyy::lang::LR_0_item> {
  size_t operator()(const cyy::lang::LR_0_item &x) const {
    auto hash_value = ::std::hash<cyy::lang::CFG::nonterminal_type>()(x.head) ^

                      ::std::hash<decltype(x.body.size())>()(x.body.size()) ^
                      ::std::hash<decltype(x.dot_pos)>()(x.dot_pos);
    return hash_value;
  }
};
} // namespace std

namespace cyy::lang {

struct LR_0_item_set {
  std::set<CFG::nonterminal_type> nonkernel_items;
  std::unordered_set<LR_0_item> kernel_items;
};
} // namespace cyy::lang
