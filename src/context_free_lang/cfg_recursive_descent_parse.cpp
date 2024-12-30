/*!
 * \file grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include <algorithm>
#include <cassert>
#include <optional>
#include <ranges>
#include <stack>
#include <utility>

#include "../exception.hpp"
#include "cfg.hpp"

namespace cyy::computation {
  namespace {

    class recursive_descent_parse_node;
    using recursive_descent_parse_node_ptr =
        std::shared_ptr<recursive_descent_parse_node>;
    class recursive_descent_parse_node {
    public:
      using iter_type = CFG::production_set_type::mapped_type::const_iterator;

      recursive_descent_parse_node(const CFG &cfg_,
                                   grammar_symbol_type grammar_symbol_)
          : grammar_symbol(std::move(grammar_symbol_)), cfg(cfg_) {
        if (grammar_symbol.is_nonterminal()) {
          auto const &bodies = cfg.get_bodies(grammar_symbol.get_nonterminal());
          begin_body_it_opt = bodies.begin();
          end_body_it_opt = bodies.end();
          reset_iter();
        }
      }

      bool use_next_body() {
        if (cur_body_it_opt == end_body_it_opt) {
          return false;
        }
        if (!children.empty()) {
          for (auto it = children.rbegin(); it != children.rend(); ++it) {
            if ((*it)->use_next_body()) {
              for (auto it2 = children.rbegin(); it2 < it; ++it2) {
                (*it2)->reset_iter();
              }
              return true;
            }
          }
        }
        return use_next_body_of_root();
      }
      bool use_next_body_of_root() {
        if (cur_body_it_opt == end_body_it_opt) {
          return false;
        }
        assert(grammar_symbol.is_nonterminal());
        cur_body_it_opt = ++(*cur_body_it_opt);
        children.clear();
        if (cur_body_it_opt != end_body_it_opt) {
          set_children();
          return true;
        }
        return false;
      }

      std::pair<bool, cyy::algorithm::symbol_string_view>
      match_nonterminal(cyy::algorithm::symbol_string_view view,
                        bool match_all = false) {
        assert(cur_body_it_opt);
        assert(end_body_it_opt);
        while (cur_body_it_opt != end_body_it_opt) {
          auto const &body = *(*cur_body_it_opt);
          if (body.empty()) {
            if (match_all) {
              if (view.empty()) {
                return {true, view};
              }
              use_next_body_of_root();
              continue;
            }
            return {true, view};
          }

          if (static_cast<size_t>(std::ranges::count_if(
                  body, [](auto const &g) { return g.is_terminal(); })) >
              view.size()) {
            use_next_body_of_root();
            continue;
          }

          std::stack<cyy::algorithm::symbol_string_view> view_stack;
          view_stack.push(view);
          size_t i = 0;
          bool backtrack_succ = true;
          while (i < body.size()) {
            assert(view_stack.size() == i + 1);
            auto cur_view = view_stack.top();
            auto const &grammal_symbol = body[i];
            if (grammal_symbol.is_terminal()) {
              if (!cur_view.empty() &&
                  grammal_symbol.get_terminal() == cur_view[0]) {
                cur_view.remove_prefix(1);
                view_stack.push(cur_view);
                i++;
                continue;
              }
            } else {
              auto [result, remain_view] =
                  children[i]->match_nonterminal(cur_view);
              if (result) {
                view_stack.push(remain_view);
                i++;
                continue;
              }
            }
            // backtrack
            backtrack_succ = false;
            while (true) {
              if (children[i]->use_next_body()) {
                backtrack_succ = true;
                for (auto j = i + 1; j < children.size(); j++) {
                  children[j]->reset_iter();
                }
                break;
              }
              if (i == 0) {
                break;
              }
              view_stack.pop();
              i--;
            }
            if (!backtrack_succ) {
              use_next_body_of_root();
              break;
            }
          }
          if (backtrack_succ) {
            return {true, view_stack.top()};
          }
        }
        return {false, view};
      }

    private:
      grammar_symbol_type grammar_symbol;
      std::vector<recursive_descent_parse_node_ptr> children;
      const CFG &cfg;
      std::optional<iter_type> begin_body_it_opt;
      std::optional<iter_type> cur_body_it_opt;
      std::optional<iter_type> end_body_it_opt;

      void reset_iter() {
        if (grammar_symbol.is_nonterminal()) {
          cur_body_it_opt = begin_body_it_opt;
          set_children();
        }
      }
      void set_children() {
        assert(cur_body_it_opt != end_body_it_opt);
        auto const &body = *(*cur_body_it_opt);
        children.clear();
        for (auto const &s : body) {
          children.push_back(
              std::make_shared<recursive_descent_parse_node>(cfg, s));
        }
      }
    };

  } // namespace

  bool CFG::recursive_descent_parse(symbol_string_view view) const {
    if (has_left_recursion()) {
      throw exception::left_recursion_CFG("");
    }
    auto terminals = get_terminals();
    for (auto s : view) {
      if (!terminals.contains(s)) {
        return false;
      }
    }
    recursive_descent_parse_node node(*this, start_symbol);
    while (true) {
      auto [res, remain_view] = node.match_nonterminal(view, true);
      if (!res) {
        return false;
      }
      if (remain_view.empty()) {
        return true;
      }

      if (!node.use_next_body()) {
        break;
      }
    }
    return false;
  }

} // namespace cyy::computation
