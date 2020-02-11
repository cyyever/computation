/*!
 * \file grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include <cassert>
#include <optional>
#include <unordered_map>
#include <utility>

#include "cfg.hpp"

namespace cyy::computation {
  namespace {

    struct recursive_descent_parse_node;
    using recursive_descent_parse_node_ptr =
        std::shared_ptr<recursive_descent_parse_node>;
    struct recursive_descent_parse_node {
      using iter_type = CFG::production_set_type::mapped_type::const_iterator;

      recursive_descent_parse_node(const CFG &cfg_,
                                   grammar_symbol_type grammar_symbol_)
          : grammar_symbol(std::move(grammar_symbol_)), cfg(cfg_) {
        if (grammar_symbol.is_nonterminal()) {
          auto it =
              cfg.get_productions().find(grammar_symbol.get_nonterminal());
          assert(it != cfg.get_productions().end());
          begin_body_it_opt = it->second.begin();
          cur_body_it_opt = it->second.begin();
          end_body_it_opt = it->second.end();
          auto const &body = *(*cur_body_it_opt);
          for (auto const &s : body) {
            children.emplace_back(
                std::make_shared<recursive_descent_parse_node>(cfg, s));
          }
        }
      }

      bool use_next_body() {
        if (!children.empty()) {
          size_t i = children.size() - 1;
          while (true) {
            if (children[i]->use_next_body()) {
              for (auto j = i + 1; j < children.size(); j++) {
                children[j]->reset_iter();
              }
              return true;
            }
            if (i == 0) {
              break;
            }
            i--;
          }
        }
        if (cur_body_it_opt.has_value() &&
            *cur_body_it_opt != *end_body_it_opt) {
          cur_body_it_opt = (*cur_body_it_opt) + 1;
          auto const &body = *(*cur_body_it_opt);
          children.clear();
          for (auto const &s : body) {
            children.push_back(
                std::make_shared<recursive_descent_parse_node>(cfg, s));
          }
          return true;
        }
        return false;
      }

      std::pair<bool, symbol_string_view>
      match_nonterminal(symbol_string_view view) {
        assert(cur_body_it_opt);
        assert(end_body_it_opt);
        while (cur_body_it_opt != end_body_it_opt) {
          auto cur_body_it = *cur_body_it_opt;
          auto end_body_it = *end_body_it_opt;

          auto const &body = *cur_body_it;
          if (body.empty() && view.empty()) {
            return {true, view};
          }

          std::vector<symbol_string_view> view_stack{view};
          size_t i = 0;
          bool backtrack_succ = true;
          while (i < body.size()) {
            backtrack_succ = true;
            assert(!view_stack.empty());
            auto cur_view = view_stack.back();
            auto const &grammal_symbol = body[i];
            if (grammal_symbol.is_terminal()) {
              if (!cur_view.empty() &&
                  grammal_symbol.get_terminal() == cur_view[0]) {
                cur_view.remove_prefix(1);
                view_stack.push_back(cur_view);
                i++;
                continue;
              }
            } else {
              auto [result, remain_view] =
                  children[i]->match_nonterminal(cur_view);
              if (result) {
                view_stack.push_back(remain_view);
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
              view_stack.pop_back();
              i--;
            }
            if (!backtrack_succ) {
              use_next_body();
              break;
            }
          }
          if (backtrack_succ) {
            return {true, view_stack.back()};
          }
        }
        return {false, view};
      }

      grammar_symbol_type grammar_symbol;

      std::vector<recursive_descent_parse_node_ptr> children;

    private:
      const CFG &cfg;
      std::optional<iter_type> begin_body_it_opt;
      std::optional<iter_type> cur_body_it_opt;
      std::optional<iter_type> end_body_it_opt;

    private:
      void reset_iter() {
        cur_body_it_opt = begin_body_it_opt;
        for (const auto &child : children) {
          child->reset_iter();
        }
      }
    };

  } // namespace

  bool CFG::recursive_descent_parse(symbol_string_view view) const {
    auto node =
        std::make_shared<recursive_descent_parse_node>(*this, start_symbol);
    while (true) {
      auto [res, remain_view] = node->match_nonterminal(view);
      if (!res) {
        return false;
      }
      if (remain_view.empty()) {
        return true;
      }
      if (!node->use_next_body()) {
        break;
      }
    }
    return false;
  }

} // namespace cyy::computation