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

  class LR_1_item {
  public:
    LR_1_item(LR_0_item item) : lr_0_item(std::move(item)) {}
    LR_1_item(LR_0_item item, CFG::terminal_type lookahead_symbol)
        : lr_0_item(std::move(item)), lookahead_symbols{lookahead_symbol} {}
    LR_1_item(LR_0_item item, CFG::terminal_set_type lookahead_symbols_)
        : lr_0_item(std::move(item)), lookahead_symbols{
                                          std::move(lookahead_symbols_)} {}
    LR_1_item(const LR_1_item &) = default;
    LR_1_item &operator=(const LR_1_item &) = default;
    LR_1_item(LR_1_item &&) = default;
    LR_1_item &operator=(LR_1_item &&) = default;
    bool operator==(const LR_1_item &rhs) const = default;

    void go() { lr_0_item.go(); }

    auto completed() const { return lr_0_item.completed(); }

    CFG::terminal_set_type follow_of_dot(const CFG &cfg) const;

    const auto &get_lr_0_item() const { return lr_0_item; }
    const auto &get_lookahead_symbols() const & { return lookahead_symbols; }
    auto &get_lookahead_symbols() && { return lookahead_symbols; }

    void add_lookahead_symbol(CFG::terminal_type lookahead_symbol);
    void add_lookahead_symbols(CFG::terminal_set_type lookahead_symbols_);
    std::string MMA_draw(const ALPHABET &alphabet) const;

  private:
    LR_0_item lr_0_item;
    CFG::terminal_set_type lookahead_symbols;
  };
} // namespace cyy::computation
namespace std {
  template <> struct hash<cyy::computation::LR_1_item> {
    size_t operator()(const cyy::computation::LR_1_item &x) const noexcept {
      return ::std::hash<cyy::computation::LR_0_item>()(x.get_lr_0_item()) ^
             ::std::hash<cyy::computation::CFG::terminal_set_type>()(
                 x.get_lookahead_symbols());
    }
  };
} // namespace std
namespace cyy::computation {
  class new_LR_1_item_set {
  public:
    void add_item(LR_1_item item) {
      auto const &lr_0_item = item.get_lr_0_item();

      if (lr_0_item.get_dot_pos() == 0 && !lr_0_item.completed()) {
        nonkernel_items[lr_0_item.get_head()].merge(
            std::move(item).get_lookahead_symbols());
        return;
      }
      kernel_items.emplace(std::move(item));
    }
    bool operator==(const new_LR_1_item_set &rhs) const = default;
    bool empty() const noexcept {
      return kernel_items.empty() && nonkernel_items.empty();
    }

    std::unordered_set<LR_1_item> expand_nonkernel_items(const CFG &cfg) const;

    std::string MMA_draw(const CFG &cfg) const;

  private:
    std::unordered_set<LR_1_item> kernel_items;
    std::unordered_map<CFG::nonterminal_type, CFG::terminal_set_type>
        nonkernel_items;
  };

} // namespace cyy::computation

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
                         CFG::terminal_set_type lookahead_set);

    bool operator==(const LR_1_item_set &rhs) const = default;
    bool empty() const noexcept {
      return kernel_items.empty() && nonkernel_items.empty();
    }

    std::unordered_map<grammar_symbol_type, LR_1_item_set>
    go(const CFG &cfg) const;

    std::string MMA_draw(const ALPHABET &alphabet) const;

  private:
    void add_nonkernel_item(const CFG &cfg, grammar_symbol_const_span_type view,
                            const CFG::terminal_set_type &lookahead_set);

  private:
    std::unordered_map<LR_0_item, CFG::terminal_set_type> kernel_items;
    std::unordered_map<CFG::nonterminal_type, CFG::terminal_set_type>
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
