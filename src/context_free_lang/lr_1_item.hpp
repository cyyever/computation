/*!
 * \file lr_item.hpp
 *
 * \author cyy
 * \date 2018-04-21
 */

#pragma once

#include <memory>
#include <set>
#include <unordered_map>

#include "cfg.hpp"
#include "lr_0_item.hpp"

namespace cyy::computation {
  class LR_1_item_set {

  public:
    auto const &get_kernel_items() const { return kernel_items; }
    auto const &get_nonkernel_items() const { return nonkernel_items; }
    auto get_completed_items() const {
      return get_kernel_items() | std::views::filter([](auto const &p) {
               return p.first.completed();
             });
    }
    void add_kernel_item(const CFG &cfg, const LR_0_item &kernel_item,
                         std::set<CFG::terminal_type> lookahead_set);

    bool operator==(const LR_1_item_set &rhs) const {
      return kernel_items == rhs.kernel_items;
    }
    bool empty() const noexcept {
      return kernel_items.empty() && nonkernel_items.empty();
    }
    std::string MMA_draw(const ALPHABET &alphabet) const;

  private:
    void add_nonkernel_item(const CFG &cfg, grammar_symbol_const_span_type view,
                            const std::set<CFG::terminal_type> &lookahead_set);

  private:
    std::unordered_map<LR_0_item, std::set<CFG::terminal_type>> kernel_items;
    std::unordered_map<CFG::nonterminal_type, std::set<CFG::terminal_type>>
        nonkernel_items;
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
