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
  CFG::production_type production;
  size_t dot_pos;
  bool operator==(const LR_0_item &rhs) const {
    //return head == rhs.head && body == rhs.body && dot_pos == rhs.dot_pos;
    return  production== rhs.production && dot_pos == rhs.dot_pos;
  }
};

} // namespace cyy::lang

namespace std {
template <> struct hash<cyy::lang::LR_0_item> {
  size_t operator()(const cyy::lang::LR_0_item &x) const {
    auto hash_value = ::std::hash<decltype(x.production.first)>()(x.production.first) ^

                      ::std::hash<decltype(x.production.second.size())>()(x.production.second.size()) ^
                      ::std::hash<decltype(x.dot_pos)>()(x.dot_pos);
    return hash_value;
  }
};
} // namespace std

namespace cyy::lang {

struct LR_0_item_set {
  std::set<CFG::nonterminal_type> nonkernel_items;
  std::unordered_set<LR_0_item> kernel_items;

  bool operator==(const LR_0_item_set &rhs) const { return nonkernel_items==rhs.nonkernel_items && kernel_items==rhs.kernel_items;}
  bool empty() const { return nonkernel_items.empty() && kernel_items.empty();}
};
} // namespace cyy::lang
