/*!
 * \file lr_item.hpp
 *
 * \author cyy
 * \date 2018-04-21
 */

#pragma once

#include <memory>
#include <ranges>
#include <unordered_set>

#include "cfg.hpp"
#include "cfg_production.hpp"
#include "regular_lang/dfa.hpp"

namespace cyy::computation {
  class LR_0_item {
  public:
    LR_0_item(CFG_production production, size_t dot_pos_ = 0)
        : production_ptr{std::make_shared<CFG_production>(
              std::move(production))},
          dot_pos{dot_pos_} {}
    LR_0_item(CFG_production::head_type head, CFG_production::body_type body,
              size_t dot_pos_ = 0)
        : LR_0_item(CFG_production{std::move(head), std::move(body)},
                    dot_pos_) {}
    LR_0_item(const LR_0_item &) = default;
    LR_0_item &operator=(const LR_0_item &) = default;
    LR_0_item(LR_0_item &&) = default;
    LR_0_item &operator=(LR_0_item &&) = default;
    bool operator==(const LR_0_item &rhs) const {
      return dot_pos == rhs.dot_pos && (production_ptr == rhs.production_ptr ||
                                        *production_ptr == *rhs.production_ptr);
    }
    auto const &get_head() const noexcept { return production_ptr->get_head(); }
    auto const &get_body() const noexcept { return production_ptr->get_body(); }
    size_t get_dot_pos() const { return dot_pos; }
    bool completed() const noexcept { return dot_pos >= get_body().size(); }
    void go() {
      if (completed()) {
        throw exception::invalid_operation("move completed item");
      }
      dot_pos++;
    }
    [[deprecated]] auto prefix() const {
      return grammar_symbol_const_span_type(get_body()).subspan(dot_pos);
    }
    auto const &get_grammar_symbal() const {
      if (completed()) {
        throw exception::invalid_operation("no grammar_symbol under dot");
      }
      return get_body()[dot_pos];
    }
    std::string MMA_draw(const CFG &cfg) const;

    const CFG_production &get_production() const { return *production_ptr; }

  private:
    std::shared_ptr<CFG_production> production_ptr;
    size_t dot_pos;
  };

} // namespace cyy::computation
namespace std {
  template <> struct hash<cyy::computation::LR_0_item> {
    size_t operator()(const cyy::computation::LR_0_item &x) const noexcept {
      return ::std::hash<std::string>()(x.get_head()) ^
             ::std::hash<cyy::computation::CFG_production::body_type>()(
                 x.get_body()) ^
             ::std::hash<size_t>()(x.get_dot_pos());
    }
  };
} // namespace std
namespace cyy::computation {
  class LR_0_item_set {
  public:
    void add_item(LR_0_item item) {
      if (item.get_dot_pos() == 0 && !item.completed()) {
        nonkernel_items.insert(item.get_head());
        return;
      }
      kernel_items.emplace(std::move(item));
    }

    auto const &get_kernel_items() const { return kernel_items; }
    auto get_completed_items() const {
      return kernel_items |
             std::views::filter([](auto const &p) { return p.completed(); });
    }
    bool has_completed_items() const {
      return std::ranges::any_of(kernel_items,
                                 [](auto const &p) { return p.completed(); });
    }

    std::unordered_set<LR_0_item> expand_nonkernel_items(const CFG &cfg) const;

    bool operator==(const LR_0_item_set &rhs) const = default;
    bool empty() const noexcept {
      return kernel_items.empty() && nonkernel_items.empty();
    }
    std::string MMA_draw(const CFG &cfg) const;

  private:
    std::unordered_set<LR_0_item> kernel_items;
    std::unordered_set<CFG_production::head_type> nonkernel_items;
  };
} // namespace cyy::computation
