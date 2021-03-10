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
#include "hash.hpp"
#include "lr_0_item.hpp"

namespace cyy::computation {

  class LR_1_item : public LR_0_item {
  public:
    LR_1_item(LR_0_item item) : LR_0_item(std::move(item)) {}
    LR_1_item(LR_0_item item, CFG::terminal_type lookahead_symbol)
        : LR_0_item(std::move(item)), lookahead_symbols{lookahead_symbol} {}
    LR_1_item(LR_0_item item, CFG::terminal_set_type lookahead_symbols_)
        : LR_0_item(std::move(item)), lookahead_symbols{
                                          std::move(lookahead_symbols_)} {}
    LR_1_item(const LR_1_item &) = default;
    LR_1_item &operator=(const LR_1_item &) = default;
    LR_1_item(LR_1_item &&) = default;
    LR_1_item &operator=(LR_1_item &&) = default;
    bool operator==(const LR_1_item &rhs) const = default;

    CFG::terminal_set_type follow_of_dot(const CFG &cfg) const;

    const auto &get_lookahead_symbols() const & { return lookahead_symbols; }
    auto &get_lookahead_symbols() && { return lookahead_symbols; }

    void add_lookahead_symbol(CFG::terminal_type lookahead_symbol);
    bool contain_lookahead_symbol(CFG::terminal_type lookahead_symbol) const {
      return lookahead_symbols.contains(lookahead_symbol);
    }

    void add_lookahead_symbols(CFG::terminal_set_type lookahead_symbols_);
    std::string [[nodiscard]] MMA_draw(const CFG &cfg) const;

  private:
    CFG::terminal_set_type lookahead_symbols;
  };
} // namespace cyy::computation
namespace std {
  template <> struct hash<cyy::computation::LR_1_item> {
    size_t operator()(const cyy::computation::LR_1_item &x) const noexcept {
      return ::std::hash<cyy::computation::LR_0_item>()(x) ^
             ::std::hash<cyy::computation::CFG::terminal_set_type>()(
                 x.get_lookahead_symbols());
    }
  };
} // namespace std
namespace cyy::computation {
  class LR_1_item_set {
  public:
    bool operator==(const LR_1_item_set &rhs) const = default;
    void add_item(LR_1_item item) {
      if (item.get_dot_pos() == 0 && !item.completed()) {
        nonkernel_items[item.get_head()].merge(
            std::move(item).get_lookahead_symbols());
        return;
      }
      kernel_items.emplace(std::move(item));
    }
    auto const &get_kernel_items() const { return kernel_items; }
    auto const &get_nonkernel_items() const { return nonkernel_items; }
    bool empty() const noexcept {
      return kernel_items.empty() && nonkernel_items.empty();
    }

    bool has_completed_items() const {
      return std::ranges::any_of(kernel_items,
                                 [](auto const &p) { return p.completed(); });
    }
    auto get_completed_items() const {
      return kernel_items | std::ranges::views::filter(
                                [](auto const &p) { return p.completed(); });
    }

    void add_nonkernel_item(CFG::nonterminal_type head,
                            CFG::terminal_set_type lookahead_symbols = {}) {
      nonkernel_items.try_emplace(std::move(head),
                                  std::move(lookahead_symbols));
    }
    std::unordered_set<LR_1_item> expand_nonkernel_items(const CFG &cfg) const;

    const LR_1_item *
    get_completed_item(CFG::terminal_type lookahead_symbol) const;
    std::string [[nodiscard]] MMA_draw(const CFG &cfg) const;

    void merge_lookahead_symbols(LR_1_item_set &item_set) {
      kernel_items.merge(item_set.kernel_items);
      for (auto &[head, lookahead_symbols] : item_set.nonkernel_items) {
        nonkernel_items[head].merge(lookahead_symbols);
      }
    }
    LR_0_item_set get_lr_0_item_set() const {
      LR_0_item_set item_set;
      for (auto const &item : kernel_items) {
        item_set.add_item(item);
      }
      for (auto const &[head, _] : nonkernel_items) {
        item_set.add_nonkernel_item(head);
      }
      return item_set;
    }

  private:
    std::unordered_set<LR_1_item> kernel_items;
    std::unordered_map<CFG::nonterminal_type, CFG::terminal_set_type>
        nonkernel_items;
  };

} // namespace cyy::computation

/* namespace std { */
/*   template <> struct hash<cyy::computation::LR_1_item_set> { */
/*     size_t operator()(const cyy::computation::LR_1_item_set &x) const {
 */
/*       return 0; */
/*       /1* auto hash_value = *1/ */
/*       /1*     ::std::hash<std::unordered_set<cyy::computation::LR_1_item>>()(
 * *1/ */
/*       /1*         x.get_kernel_items()); *1/ */
/*       /1* return hash_value; *1/ */
/*     } */
/*   }; */
/* } // namespace std *1/ */
