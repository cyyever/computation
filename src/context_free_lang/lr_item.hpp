/*!
 * \file lr_item.hpp
 *
 * \author cyy
 * \date 2018-04-21
 */

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "cfg.hpp"
#include "cfg_production.hpp"

namespace cyy::computation {
  class LR_0_item {
  public:
    LR_0_item(CFG_production production, size_t dot_pos_ = 0)
        : production_ptr{std::make_shared<CFG_production>(
              std::move(production))},
          dot_pos{dot_pos_} {}
    bool operator==(const LR_0_item &rhs) const {
      return *production_ptr == *rhs.production_ptr && dot_pos == rhs.dot_pos;
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
    auto prefix() const {
      return grammar_symbol_const_span_type(get_body()).subspan(dot_pos);
    }
    auto const &get_grammar_symbal() const {
      if (completed()) {
        throw exception::invalid_operation("no grammar_symbol under dot");
      }
      return get_body()[dot_pos];
    }

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
    std::unordered_set<LR_0_item> kernel_items;
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
