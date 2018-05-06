/*!
 * \file lr_item.hpp
 *
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
    return std::tie(production, dot_pos) ==
           std::tie(rhs.production, rhs.dot_pos);
  }
};

} // namespace cyy::lang

namespace std {
template <> struct hash<cyy::lang::LR_0_item> {
  size_t operator()(const cyy::lang::LR_0_item &x) const {
    auto hash_value =
        ::std::hash<decltype(x.production.first)>()(x.production.first) ^

        ::std::hash<decltype(x.production.second.size())>()(
            x.production.second.size()) ^
        ::std::hash<decltype(x.dot_pos)>()(x.dot_pos);
    return hash_value;
  }
};
} // namespace std

namespace cyy::lang {

class LR_0_item_set {

public:
  auto get_kernel_items() const -> const auto & { return kernel_items; }

  auto get_nonkernel_items() const -> const auto & { return nonkernel_items; }
  void add_kernel_item(const CFG &cfg, LR_0_item kernel_item);

  bool operator==(const LR_0_item_set &rhs) const {
    return kernel_items == rhs.kernel_items;
  }
  bool empty() const { return kernel_items.empty(); }

private:
  std::unordered_set<LR_0_item> kernel_items;
  std::set<CFG::nonterminal_type> nonkernel_items;
};
} // namespace cyy::lang

namespace std {
template <> struct hash<cyy::lang::LR_0_item_set> {
  size_t operator()(const cyy::lang::LR_0_item_set &x) const {
    auto hash_value = ::std::hash<decltype(x.get_kernel_items().size())>()(
        x.get_kernel_items().size());
    return hash_value;
  }
};
} // namespace std

namespace cyy::lang {
struct LR_1_item {
  LR_0_item item;
  CFG::terminal_type lookahead;
  bool operator==(const LR_1_item &rhs) const {
    return std::tie(item, lookahead) == std::tie(rhs.item, rhs.lookahead);
  }
};

} // namespace cyy::lang

namespace std {
template <> struct hash<cyy::lang::LR_1_item> {
  size_t operator()(const cyy::lang::LR_1_item &x) const {
    auto hash_value = ::std::hash<decltype(x.item)>()(x.item) ^
                      ::std::hash<decltype(x.lookahead)>()(x.lookahead);
    return hash_value;
  }
};
} // namespace std

namespace cyy::lang {
class LR_1_item_set {

public:
  auto get_kernel_items() const -> const auto & { return kernel_items; }

  auto get_nonkernel_items() const -> const auto & { return nonkernel_items; }
  void add_kernel_item(const CFG &cfg, const LR_0_item &kernel_item,
                       std::set<CFG::terminal_type> lookahead_set);

  bool operator==(const LR_1_item_set &rhs) const {
    return kernel_items == rhs.kernel_items;
  }
  bool empty() const { return kernel_items.empty(); }

private:
  void add_nonkernel_item(const CFG &cfg, CFG::grammar_symbol_string_view view,

                          std::set<CFG::terminal_type> lookahead_set);

private:
  std::unordered_map<LR_0_item, std::set<CFG::terminal_type>> kernel_items;
  std::map<CFG::nonterminal_type, std::set<CFG::terminal_type>> nonkernel_items;
};

} // namespace cyy::lang

namespace std {
template <> struct hash<cyy::lang::LR_1_item_set> {
  size_t operator()(const cyy::lang::LR_1_item_set &x) const {
    auto hash_value = ::std::hash<decltype(x.get_kernel_items().size())>()(
        x.get_kernel_items().size());
    return hash_value;
  }
};
} // namespace std
