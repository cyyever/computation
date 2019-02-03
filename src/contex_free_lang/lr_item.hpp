/*!
 * \file lr_item.hpp
 *
 * \author cyy
 * \date 2018-04-21
 */

#pragma once

#include <functional>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "cfg.hpp"

namespace cyy::computation {

struct LR_0_item {
  CFG_production production;
  size_t dot_pos{0};
  bool operator==(const LR_0_item &rhs) const noexcept {
    return std::tie(production, dot_pos) ==
           std::tie(rhs.production, rhs.dot_pos);
  }
};

} // namespace cyy::computation

namespace std {
template <> struct hash<cyy::computation::LR_0_item> {
  size_t operator()(const cyy::computation::LR_0_item &x) const noexcept {
    return ::std::hash<cyy::computation::CFG_production::head_type>()(
               x.production.get_head()) ^
           ::std::hash<decltype(x.production.get_body().size())>()(
               x.production.get_body().size()) ^
           ::std::hash<decltype(x.dot_pos)>()(x.dot_pos);
  }
};

template <> struct less<cyy::computation::LR_0_item> {
  bool operator()(const cyy::computation::LR_0_item &lhs,
                  const cyy::computation::LR_0_item &rhs) const noexcept {
    return std::tie(lhs.dot_pos, lhs.production) <
           std::tie(rhs.dot_pos, rhs.production);
  }
};

} // namespace std

namespace cyy::computation {

class LR_0_item_set {

public:
  auto get_kernel_items() const noexcept -> const auto & {
    return kernel_items;
  }

  auto get_nonkernel_items() const noexcept -> const auto & {
    return nonkernel_items;
  }
  void add_kernel_item(const CFG &cfg, LR_0_item kernel_item);
  bool empty() const noexcept { return kernel_items.empty(); }

  bool operator==(const LR_0_item_set &rhs) const {
    return kernel_items == rhs.kernel_items;
  }

private:
  std::set<LR_0_item> kernel_items;
  std::unordered_set<CFG::nonterminal_type> nonkernel_items;
};
} // namespace cyy::computation

namespace std {
template <> struct hash<cyy::computation::LR_0_item_set> {
  size_t operator()(const cyy::computation::LR_0_item_set &x) const noexcept {
    return ::std::hash<decltype(x.get_kernel_items().size())>()(
        x.get_kernel_items().size());
  }
};
} // namespace std

namespace cyy::computation {
class LR_1_item_set {

public:
  auto get_kernel_items() const noexcept -> const auto & {
    return kernel_items;
  }

  auto get_nonkernel_items() const noexcept -> const auto & {
    return nonkernel_items;
  }
  void add_kernel_item(const CFG &cfg, const LR_0_item &kernel_item,
                       std::set<CFG::terminal_type> lookahead_set);

  bool operator==(const LR_1_item_set &rhs) const {
    return kernel_items == rhs.kernel_items;
  }
  bool empty() const noexcept { return kernel_items.empty(); }

private:
  void add_nonkernel_item(const CFG &cfg, grammar_symbol_const_span_type view,
                          const std::set<CFG::terminal_type> &lookahead_set);

private:
  std::map<LR_0_item, std::set<CFG::terminal_type>> kernel_items;
  std::map<CFG::nonterminal_type, std::set<CFG::terminal_type>> nonkernel_items;
};

} // namespace cyy::computation

namespace std {
template <> struct hash<cyy::computation::LR_1_item_set> {
  size_t operator()(const cyy::computation::LR_1_item_set &x) const {
    const auto size = x.get_kernel_items().size();

    if (size >= 1) {
      return ::std::hash<cyy::computation::LR_0_item>()(
          x.get_kernel_items().begin()->first);
    }
    return 0;
  }
};
} // namespace std
