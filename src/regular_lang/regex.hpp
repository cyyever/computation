/*!
 * \file regex.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include "context_free_lang/ll_grammar.hpp"
#include "dfa.hpp"
#include "exception.hpp"
#include "nfa.hpp"

namespace cyy::computation {

  class regex {

  public:
    class syntax_node {
    public:
      virtual ~syntax_node() = default;
      virtual NFA to_NFA(const ALPHABET_ptr &alphabet,
                         NFA::state_type start_state) const = 0;
      virtual CFG to_CFG(const ALPHABET_ptr &alphabet,
                         const CFG::nonterminal_type &start_symbol) const = 0;
      virtual bool is_empty_set_node() const = 0;
      virtual bool is_epsilon_node() const = 0;
      virtual bool nullable() const = 0;
      virtual void assign_position(
          std::unordered_map<uint64_t, symbol_type> &position_to_symbol) = 0;
      virtual std::unordered_set<uint64_t> first_pos() const = 0;
      virtual std::unordered_set<uint64_t> last_pos() const = 0;
      virtual std::unordered_map<uint64_t, std::unordered_set<uint64_t>>
      follow_pos() const = 0;
      virtual std::shared_ptr<syntax_node> simplify() const = 0;
      virtual symbol_string to_string() const = 0;
    };

    class empty_set_node final : public syntax_node {
    public:
      explicit empty_set_node() = default;
      NFA to_NFA(const ALPHABET_ptr &alphabet,
                 NFA::state_type start_state) const override;
      CFG to_CFG(const ALPHABET_ptr &alphabet,
                 const CFG::nonterminal_type &start_symbol) const override;
      bool is_empty_set_node() const override { return true; }
      bool is_epsilon_node() const override { return false; }
      bool nullable() const noexcept override { return true; }
      void assign_position(std::unordered_map<uint64_t, symbol_type>
                               &position_to_symbol) override;
      std::unordered_set<uint64_t> first_pos() const override;
      std::unordered_set<uint64_t> last_pos() const override;
      std::unordered_map<uint64_t, std::unordered_set<uint64_t>>
      follow_pos() const override {
        return {};
      }
      std::shared_ptr<syntax_node> simplify() const noexcept override {
        return {};
      }
      symbol_string to_string() const override { return {}; }
    };

    class epsilon_node final : public syntax_node {
    public:
      explicit epsilon_node() = default;
      NFA to_NFA(const ALPHABET_ptr &alphabet,
                 NFA::state_type start_state) const override;
      CFG to_CFG(const ALPHABET_ptr &alphabet,
                 const CFG::nonterminal_type &start_symbol) const override;
      bool nullable() const noexcept override { return true; }
      bool is_empty_set_node() const override { return false; }
      bool is_epsilon_node() const override { return true; }
      void assign_position(std::unordered_map<uint64_t, symbol_type>
                               &position_to_symbol) noexcept override;
      std::unordered_set<uint64_t> first_pos() const override;
      std::unordered_set<uint64_t> last_pos() const override;
      std::unordered_map<uint64_t, std::unordered_set<uint64_t>>
      follow_pos() const override {
        return {};
      }
      std::shared_ptr<syntax_node> simplify() const override { return {}; }
      symbol_string to_string() const override { return {}; }
    };

    class basic_node final : public syntax_node {
    public:
      explicit basic_node(symbol_type symbol_) noexcept : symbol(symbol_) {}
      NFA to_NFA(const ALPHABET_ptr &alphabet,
                 NFA::state_type start_state) const override;
      CFG to_CFG(const ALPHABET_ptr &alphabet,
                 const CFG::nonterminal_type &start_symbol) const override;
      bool is_empty_set_node() const override { return false; }
      bool is_epsilon_node() const override { return false; }
      bool nullable() const noexcept override { return false; }
      void assign_position(std::unordered_map<uint64_t, symbol_type>
                               &position_to_symbol) override;
      std::unordered_set<uint64_t> first_pos() const override;
      std::unordered_set<uint64_t> last_pos() const override;
      std::unordered_map<uint64_t, std::unordered_set<uint64_t>>
      follow_pos() const override {
        return {};
      }
      std::shared_ptr<syntax_node> simplify() const override { return {}; }
      symbol_string to_string() const override { return {symbol}; }

    private:
      symbol_type symbol;
      uint64_t position{0};
    };
    class union_node final : public syntax_node {
    public:
      union_node(const std::shared_ptr<syntax_node> &left_node_,
                 const std::shared_ptr<syntax_node> &right_node_)
          : left_node(left_node_), right_node(right_node_) {
        if (!left_node) {
          throw exception::empty_syntax_tree("left tree is empty");
        }
        if (!right_node) {
          throw exception::empty_syntax_tree("right tree is empty");
        }
      }
      NFA to_NFA(const ALPHABET_ptr &alphabet,
                 NFA::state_type start_state) const override;
      CFG to_CFG(const ALPHABET_ptr &alphabet,
                 const CFG::nonterminal_type &start_symbol) const override;
      bool nullable() const override {
        return left_node->nullable() || right_node->nullable();
      }
      void assign_position(std::unordered_map<uint64_t, symbol_type>
                               &position_to_symbol) override;
      std::unordered_set<uint64_t> first_pos() const override;
      std::unordered_set<uint64_t> last_pos() const override;
      std::unordered_map<uint64_t, std::unordered_set<uint64_t>>
      follow_pos() const override;
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
      concat_node(const std::shared_ptr<syntax_node> &left_node_,
                  const std::shared_ptr<syntax_node> &right_node_)
          : left_node(left_node_), right_node(right_node_) {

        if (!left_node) {
          throw exception::empty_syntax_tree("left tree is empty");
        }
        if (!right_node) {
          throw exception::empty_syntax_tree("right tree is empty");
        }
      }
      NFA to_NFA(const ALPHABET_ptr &alphabet,
                 NFA::state_type start_state) const override;
      CFG to_CFG(const ALPHABET_ptr &alphabet,
                 const CFG::nonterminal_type &start_symbol) const override;
      bool nullable() const override {
        return left_node->nullable() && right_node->nullable();
      }
      void assign_position(std::unordered_map<uint64_t, symbol_type>
                               &position_to_symbol) override;
      std::unordered_set<uint64_t> first_pos() const override;
      std::unordered_set<uint64_t> last_pos() const override;
      std::unordered_map<uint64_t, std::unordered_set<uint64_t>>
      follow_pos() const override;
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
          : inner_node(inner_node_) {

        if (!inner_node) {
          throw exception::empty_syntax_tree("inner tree is empty");
        }
      }
      NFA to_NFA(const ALPHABET_ptr &alphabet,
                 NFA::state_type start_state) const override;
      CFG to_CFG(const ALPHABET_ptr &alphabet,
                 const CFG::nonterminal_type &start_symbol) const override;
      bool nullable() const noexcept override { return true; }
      void assign_position(std::unordered_map<uint64_t, symbol_type>
                               &position_to_symbol) override;
      std::unordered_set<uint64_t> first_pos() const override;
      std::unordered_set<uint64_t> last_pos() const override;
      bool is_empty_set_node() const override;
      bool is_epsilon_node() const override;
      std::shared_ptr<syntax_node> simplify() const override;
      symbol_string to_string() const override {
        auto inner_string = inner_node->to_string();
        if (inner_string.size() > 1) {
          inner_string.insert(inner_string.begin(), '(');
          inner_string.push_back(')');
        }
        inner_string.push_back('*');
        return inner_string;
      }

    private:
      std::shared_ptr<syntax_node> inner_node;
      std::unordered_map<uint64_t, std::unordered_set<uint64_t>>
      follow_pos() const override;
    };

    regex(ALPHABET_ptr alphabet_, symbol_string_view view)
        : alphabet(std::move(alphabet_)) {
      syntax_tree = parse(view);
    }
    regex(ALPHABET_ptr alphabet_,
          std::shared_ptr<regex::syntax_node> syntax_tree_)
        : alphabet(std::move(alphabet_)), syntax_tree(std::move(syntax_tree_)) {
    }

    NFA to_NFA(NFA::state_type start_state = 0) const {
      return syntax_tree->to_NFA(alphabet, start_state);
    }
    CFG to_CFG() const { return syntax_tree->to_CFG(alphabet, "S"); }
    const auto &get_syntax_tree() const { return syntax_tree; }

    // 基于McNaughton-Yamada算法
    DFA to_DFA() const;

  private:
    symbol_type escape_symbol(symbol_type symbol) const;
    std::shared_ptr<syntax_node> parse(symbol_string_view view) const;
    static std::shared_ptr<syntax_node>
    make_character_class(const symbol_set_type &symbol_set);
    std::shared_ptr<syntax_node>
    make_complemented_character_class(const symbol_set_type &symbol_set) const;

    const LL_grammar &get_grammar() const;

    ALPHABET_ptr alphabet;
    mutable std::shared_ptr<regex::syntax_node> syntax_tree;
  };
} // namespace cyy::computation
