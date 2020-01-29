/*!
 * \file regex.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <map>
#include <set>
#include <string_view>

#include "../context_free_lang/ll_grammar.hpp"
#include "dfa.hpp"
#include "nfa.hpp"

namespace cyy::computation {

  class regex {

  public:
    class syntax_node {
    public:
      syntax_node() = default;
      virtual ~syntax_node() = default;
      virtual NFA to_NFA(std::string_view alphabet_name,
                         NFA::state_type start_state) const = 0;
      virtual bool is_empty_set_node() const = 0;
      virtual bool is_epsilon_node() const = 0;
      virtual bool nullable() const = 0;
      virtual void
      assign_position(std::map<uint64_t, symbol_type> &position_to_symbol) = 0;
      virtual std::set<uint64_t> first_pos() const = 0;
      virtual std::set<uint64_t> last_pos() const = 0;
      virtual std::map<uint64_t, std::set<uint64_t>> follow_pos() const = 0;
      virtual std::shared_ptr<syntax_node> simplify() const { return {}; }
      virtual symbol_string to_string() const = 0;
    };

    class empty_set_node final : public syntax_node {
    public:
      explicit empty_set_node() = default;
      NFA to_NFA(std::string_view alphabet_name,
                 NFA::state_type start_state) const override;
      bool is_empty_set_node() const override { return true; }
      bool is_epsilon_node() const override { return false; }
      bool nullable() const noexcept override { return true; }
      void assign_position(
          std::map<uint64_t, symbol_type> &position_to_symbol) override;
      std::set<uint64_t> first_pos() const override;
      std::set<uint64_t> last_pos() const override;
      std::map<uint64_t, std::set<uint64_t>> follow_pos() const override {
        return {};
      }
      symbol_string to_string() const override { return {}; }
    };

    class epsilon_node final : public syntax_node {
    public:
      explicit epsilon_node() = default;
      NFA to_NFA(std::string_view alphabet_name,
                 NFA::state_type start_state) const override;
      bool nullable() const noexcept override { return true; }
      bool is_empty_set_node() const override { return false; }
      bool is_epsilon_node() const override { return true; }
      void assign_position(std::map<uint64_t, symbol_type>
                               &position_to_symbol) noexcept override;
      std::set<uint64_t> first_pos() const override;
      std::set<uint64_t> last_pos() const override;
      std::map<uint64_t, std::set<uint64_t>> follow_pos() const override {
        return {};
      }
      symbol_string to_string() const override { return {}; }
    };

    class basic_node final : public syntax_node {
    public:
      explicit basic_node(symbol_type symbol_) noexcept : symbol(symbol_) {}
      NFA to_NFA(std::string_view alphabet_name,
                 NFA::state_type start_state) const override;
      bool is_empty_set_node() const override { return false; }
      bool is_epsilon_node() const override { return false; }
      bool nullable() const noexcept override { return false; }
      void assign_position(
          std::map<uint64_t, symbol_type> &position_to_symbol) override;
      std::set<uint64_t> first_pos() const override;
      std::set<uint64_t> last_pos() const override;
      std::map<uint64_t, std::set<uint64_t>> follow_pos() const override {
        return {};
      }
      symbol_string to_string() const override { return {symbol}; }

    private:
      symbol_type symbol;
      uint64_t position{0};
    };
    class union_node final : public syntax_node {
    public:
      explicit union_node(
          const std::shared_ptr<syntax_node> &left_node_,
          const std::shared_ptr<syntax_node> &right_node_) noexcept
          : left_node(left_node_), right_node(right_node_) {}
      NFA to_NFA(std::string_view alphabet_name,
                 NFA::state_type start_state) const override;
      bool nullable() const override {
        return left_node->nullable() || right_node->nullable();
      }
      void assign_position(
          std::map<uint64_t, symbol_type> &position_to_symbol) override;
      std::set<uint64_t> first_pos() const override;
      std::set<uint64_t> last_pos() const override;
      std::map<uint64_t, std::set<uint64_t>> follow_pos() const override;
      bool is_empty_set_node() const override;
      bool is_epsilon_node() const override;
      std::shared_ptr<syntax_node> simplify() const override;
      symbol_string to_string() const override {
        symbol_string str;
        auto left_str = left_node->to_string();
        if (left_str.size() > 1) {
          str += '(';
          str += left_str;
          str += ')';
        } else {
          str += left_str;
        }
        str += '|';
        auto right_str = right_node->to_string();
        if (right_str.size() > 1) {
          str += '(';
          str += right_str;
          str += ')';
        } else {
          str += right_str;
        }
        return str;
      }

    private:
      std::shared_ptr<syntax_node> left_node, right_node;
    };
    class concat_node final : public syntax_node {
    public:
      explicit concat_node(
          const std::shared_ptr<syntax_node> &left_node_,
          const std::shared_ptr<syntax_node> &right_node_) noexcept
          : left_node(left_node_), right_node(right_node_) {}
      NFA to_NFA(std::string_view alphabet_name,
                 NFA::state_type start_state) const override;
      bool nullable() const override {
        return left_node->nullable() && right_node->nullable();
      }
      void assign_position(
          std::map<uint64_t, symbol_type> &position_to_symbol) override;
      std::set<uint64_t> first_pos() const override;
      std::set<uint64_t> last_pos() const override;
      std::map<uint64_t, std::set<uint64_t>> follow_pos() const override;
      bool is_empty_set_node() const override;
      bool is_epsilon_node() const override;
      std::shared_ptr<syntax_node> simplify() const override;
      symbol_string to_string() const override {
        return left_node->to_string() + right_node->to_string();
      }

    private:
      std::shared_ptr<syntax_node> left_node, right_node;
    };
    class kleene_closure_node final : public syntax_node {
    public:
      explicit kleene_closure_node(
          const std::shared_ptr<syntax_node> &inner_node_)
          : inner_node(inner_node_) {}
      NFA to_NFA(std::string_view alphabet_name,
                 NFA::state_type start_state) const override;
      bool nullable() const noexcept override { return true; }
      void assign_position(
          std::map<uint64_t, symbol_type> &position_to_symbol) override;
      std::set<uint64_t> first_pos() const override;
      std::set<uint64_t> last_pos() const override;
      bool is_empty_set_node() const override;
      bool is_epsilon_node() const override;
      std::shared_ptr<syntax_node> simplify() const override;
      symbol_string to_string() const override {
        symbol_string str;
        auto inner_string = inner_node->to_string();
        if (inner_string.size() > 1) {
          str += '(';
          str += inner_string;
          str += ')';
        } else {
          str = inner_string;
        }
        str += '*';
        return str;
      }

    private:
      std::shared_ptr<syntax_node> inner_node;
      std::map<uint64_t, std::set<uint64_t>> follow_pos() const override;
    };

  public:
    regex(const std::string &alphabet_name, symbol_string_view view)
        : alphabet(ALPHABET::get(alphabet_name)) {
      syntax_tree = parse(view);
    }
    regex(std::string_view alphabet_name,
          std::shared_ptr<regex::syntax_node> syntax_tree_)
        : alphabet(ALPHABET::get(alphabet_name)), syntax_tree(syntax_tree_) {}

    NFA to_NFA(NFA::state_type start_state = 0) const {
      return syntax_tree->to_NFA(alphabet->get_name(), start_state);
    }

    //基于McNaughton-Yamada算法
    DFA to_DFA() const;

  private:
    std::shared_ptr<syntax_node> parse(symbol_string_view view) const;
    std::shared_ptr<syntax_node>
    make_character_class(const std::set<symbol_type> &symbol_set) const;
    std::shared_ptr<syntax_node> make_complemented_character_class(
        const std::set<symbol_type> &symbol_set) const;

    const LL_grammar &get_grammar(const std::string &alphabet_name) const;

  private:
    std::shared_ptr<ALPHABET> alphabet;
    mutable std::shared_ptr<regex::syntax_node> syntax_tree;
  };
} // namespace cyy::computation
